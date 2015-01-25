#include "CameraPreview.h"
#include "GLESUtils.h"
#include <android/log.h>

#define  LOG_TAG    "CameraPreview"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

static const char gVertexShader[] = "attribute vec4 a_position;   \n"
  "attribute vec2 a_texCoord;   \n"
  "varying vec2 v_texCoord;     \n"
  "void main()                  \n"
  "{                            \n"
  "   gl_Position = a_position; \n"
  "   v_texCoord = a_texCoord;  \n"
  "}                            \n";

static const char gFragmentShader[] = "precision mediump float;   \n"
  "varying vec2 v_texCoord;                            \n"
  "uniform sampler2D s_texture;                        \n"
  "void main()                                         \n"
  "{                                                   \n"
  "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
 "}                                                   \n";

const GLfloat Vertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, //pos0 //bottom left
        0.0f, 1.0f, //tex0
        1.0f, -1.0f, 0.0f, 1.0f, //pos1  //bottom right
        0.0f, 0.0f, //tex1
        -1.0f, 1.0f, 0.0f, 1.0f, //pos2 //top left
        1.0f, 1.0f, //tex2
        1.0f, 1.0f, 0.0f, 1.0f, //pos3 //top right
        1.0f, 0.0f  //tex3
    };
const GLsizei stride = 6 * sizeof(GLfloat); // 4 for position, 2 for texture

CameraPreview* CameraPreview::GetThe() {
    static CameraPreview The;
    return &The;
}

CameraPreview::CameraPreview()
: AVDevicePosition(AVCaptureDevicePositionBack)
, defaultAVCaptureVideoOrientation(AVCaptureVideoOrientationPortrait)
, FPS(0)
, isReady(false)
, vidCapture(AVCaptureDevicePositionBack)
, goingAway(false)
, captureTexture(0)
, currentFlashMode(cv::CAP_ANDROID_FLASH_MODE_OFF)
, didSetup(false)
{
    vidCapture.set(cv::CAP_PROP_ANDROID_FLASH_MODE, currentFlashMode);
    pthread_mutex_init(&m_mutex, 0);
}

CameraPreview::~CameraPreview()
{
    LOGD("Deconstructor");
    cleanUp();
    pthread_mutex_destroy(&m_mutex);
}

void CameraPreview::cleanUp()
{
    LOGD("Cleanup");
    goingAway = true;
    pthread_mutex_lock(&m_mutex);
    vidCapture.release();
    glDeleteTextures(1, &captureTexture);
    pthread_mutex_unlock(&m_mutex);
 }

void CameraPreview::setupDevice()
{
    LOGD("Setting up device");
    if(goingAway)
    {
        vidCapture = cv::VideoCapture(AVCaptureDevicePositionBack);
        vidCapture.set(cv::CAP_PROP_ANDROID_FLASH_MODE, currentFlashMode);
    }
    LOGD("OpenCV Ready");
    quadShaderProgram = createProgram(gVertexShader, gFragmentShader);
    if( !quadShaderProgram )
    {
        LOGD("can't create program");
        assert(0);
    }
    LOGD("Created program");

    PositionHandle = glGetAttribLocation(quadShaderProgram, "a_position");
    TexCoordHandle = glGetAttribLocation(quadShaderProgram, "a_texCoord");
    SamplerHandle = glGetUniformLocation(quadShaderProgram, "s_texture");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &captureTexture);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glUseProgram(quadShaderProgram);
    LOGD("OPENGL Ready");
}

void CameraPreview::setSize( float view_width, float view_height )
{
    LOGD("setSize");
    union {double prop; const char* name;} u;
    u.prop = vidCapture.get(cv::CAP_PROP_ANDROID_PREVIEW_SIZES_STRING);

    cv::Size camera_resolution;
    if(u.name)
        camera_resolution = calc_optimal_camera_resolution(u.name, 640,480);
    else
    {
        LOGD("Cannot get supported camera camera_resolutions");
        camera_resolution = cv::Size(640, 480);
    }

    if((camera_resolution.width != 0) && (camera_resolution.height != 0))
    {
        vidCapture.set(cv::CAP_PROP_FRAME_WIDTH, camera_resolution.width);
        vidCapture.set(cv::CAP_PROP_FRAME_HEIGHT, camera_resolution.height);
    }
    myAspectRatio = std::min((float) view_width / camera_resolution.width, (float) view_height / camera_resolution.height);
    screenWidth = view_width;
    screenHeight = view_height;
    glViewport(0, 0, screenWidth, screenHeight);
    didSetup = true;
    LOGD("SetSizeFinished");
}

const int CameraPreview::start()
{
    LOGD("Start");
    Start();
    goingAway = false;
    return 0;
}

const int CameraPreview::stop()
{
    LOGD("Stop");
    Quit();
    Cancel();
    cleanUp();
    goingAway = true;
    return 0;
}

cv::Size CameraPreview::calc_optimal_camera_resolution(const char* supported, int width, int height)
{
    int frame_width = 0;
    int frame_height = 0;

    size_t prev_idx = 0;
    size_t idx = 0;
    float min_diff = FLT_MAX;

    do
    {
        int tmp_width;
        int tmp_height;

        prev_idx = idx;
        while ((supported[idx] != '\0') && (supported[idx] != ','))
            idx++;

        sscanf(&supported[prev_idx], "%dx%d", &tmp_width, &tmp_height);

        int w_diff = width - tmp_width;
        int h_diff = height - tmp_height;
        if ((h_diff >= 0) && (w_diff >= 0))
        {
            if ((h_diff <= min_diff) && (tmp_height <= 720))
            {
                frame_width = tmp_width;
                frame_height = tmp_height;
                min_diff = h_diff;
            }
        }

        idx++; // to skip comma symbol

    } while(supported[idx-1] != '\0');

    return cv::Size(frame_width, frame_height);
}

bool CameraPreview::drawFrame()
{
    LOGD("CameraPreview DrawFrame");
    if(!goingAway && IsRunning())
    {
        if(isReady)
        {
            captureTexture = createSimpleTexture2D( captureTexture, drawing_frame.ptr(), GL_TEXTURE_2D, drawing_frame.cols, drawing_frame.rows, drawing_frame.channels());
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glVertexAttribPointer(PositionHandle, 4, GL_FLOAT, GL_FALSE, stride, Vertices);
        glVertexAttribPointer(TexCoordHandle, 2, GL_FLOAT, GL_FALSE, stride, &Vertices[4]);
        glEnableVertexAttribArray(PositionHandle);
        glEnableVertexAttribArray(TexCoordHandle);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, captureTexture);
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        glUniform1i(SamplerHandle, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        isReady = false;
        return true;
    }
    return false;
}

void CameraPreview::Run()
{
    while (!goingAway)
    {
        if(!IsRunning())
        {
            break;
        }
        pthread_mutex_lock(&m_mutex);
        if(goingAway)
            break;
        int64 then = 0;
        int64 now = cv::getTickCount();
        time_queue.push(now);

        // Capture frame from camera and draw it
        if (vidCapture.grab())
        {
            vidCapture.retrieve(drawing_frame, cv::CAP_ANDROID_COLOR_FRAME_RGBA);
            LOGD("%s", "Display performance: %dx%d @ %.3f", drawing_frame.cols, drawing_frame.rows, FPS);
            isReady = true;
        }

        if (time_queue.size() >= 2)
            then = time_queue.front();
        if (time_queue.size() >= 25)
            time_queue.pop();
        FPS = time_queue.size() * (float)cv::getTickFrequency() / (now-then);
        pthread_mutex_unlock(&m_mutex);
//        LOGD("currentFPS[%.3f]", FPS);
    }
}
