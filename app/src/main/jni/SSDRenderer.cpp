#include "CameraPreview.h"
#include "SSDRenderer.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <android/log.h>

#define  LOG_TAG    "SSDRenderer"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

SSDRenderer * SSDRenderer::GetThe()
{
    static SSDRenderer The;
    return &The;
}

SSDRenderer::SSDRenderer()
 : ns(2l * (long)(0.5f*FS*1.05f))
 , m(ns / HEIGHT)
 , mWidth(WIDTH)
 , mHeight(HEIGHT)
 , newFrameReady(false)
 , goingAway(false)
 , dt(1.0 / FS)
 , scale(0.5f/int(sqrtf(float(WIDTH))))
 , SoundEngine()
{
   for (int i=0; i < mWidth; i++) {
//     w[i] = 6.283185307179586476925287 * 500 + 6.283185307179586476925287 * (5000-500)   *i / (mHeight - 1); // linear distribution
       w[i] = 6.283185307179586476925287 * 500 * powf(1.0f* 5000/500,1.0f * i / (mHeight - 1)); // exponential distribution
       phi0[i] = 6.283185307179586476925287 * rand();
   }
   for ( int k=0; k<ns; k++ )
   {
        double t = k * dt;
        for(int i = 0; i < mHeight; i++)
           sinTable[i][k] = sin(w[i] * t + phi0[i]);
   }
   tau1 = 0.5 / w[WIDTH-1];
   tau2 = 0.25 * tau1*tau1;
   pthread_mutex_init(&m_mutex, 0);
}

SSDRenderer::~SSDRenderer()
{
    pthread_mutex_destroy(&m_mutex);
}

const int SSDRenderer::start()
{
    LOGD("Start");
    SoundEngine.startSoundPlayer();
    Start();
    stoppedEngine = false;
    return 0;
}

const int SSDRenderer::stop()
{
    LOGD("Stop");
    pthread_mutex_lock(&m_mutex);
    newFrameReady = false;
    stoppedEngine = true;
    goingAway = true;
    Quit();
    Cancel();
    SoundEngine.stopSoundPlayer();
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

void SSDRenderer::Run()
{
    while (!goingAway)
    {
        if(!IsRunning())
            break;
        if(!newFrameReady)
            continue;
        pthread_mutex_lock(&m_mutex);
        if(goingAway)
            break;
        int64 then = 0;
        int64 now = cv::getTickCount();
        time_queue.push(now);

        //running it here
        ProcessMono();
        newFrameReady = false;

        //Done running this pass
        if (time_queue.size() >= 2)
            then = time_queue.front();
        if (time_queue.size() >= 25)
            time_queue.pop();
        float FPS = time_queue.size() * (float)cv::getTickFrequency() / (now-then);
        pthread_mutex_unlock(&m_mutex);
        LOGD("\n\n\ncurrent SSD Renders per Second[%.3f]\n\n\n", FPS);
    }
}

void SSDRenderer::generateSoundScape(const cv::Mat * drawFrame)
{
    if(pthread_mutex_trylock(&m_mutex) == 0)
    {
        LOGD("Generating SoundScape");
        cv::resize(*drawFrame, RenderFrame, cv::Size(mWidth, mHeight));
        cvtColor(RenderFrame,RenderFrame,CV_BGR2GRAY);
        newFrameReady = true;
        pthread_mutex_unlock(&m_mutex);
    }
}

void SSDRenderer::ProcessMono()
{
    float yp=0,y=0,z=0;
    double s=0;
    int16_t outBuffer[ ns ];
    memset(outBuffer, 0 , sizeof(int16_t) * ns);

    for ( int k=0; k<ns; k++ )
    {
        s = 0;
        int j = k * 1 / m;
        if (j>WIDTH-1) j=WIDTH-1;

        if (k < ns * 1/(5*WIDTH))
        {
            s = (2.0*rand()-1.0) * 1 / scale;  // "click"
        }
        else
        {
            for (int i=0; i<WIDTH; i++)
            {
                s += powf(10.0,float(RenderFrame.at<uchar>(i,j)) * 1/255.0f) * sinTable[i][k];
            }
        }
        yp = y;
        y = tau1 * 1/dt + tau2 * 1/(dt*dt);
        y = (s + y * yp + tau2/dt * z) * 1 / (1.0 + y);
        z = (y - yp) * 1 / dt;
        int b  = (int) (scale * 128 * y); // y = 2nd order filtered s
        if (b > SHRT_MAX)  b = SHRT_MAX;
        if (b < SHRT_MIN)  b = SHRT_MIN;
        outBuffer[k] = b;
    }
    SoundEngine.playBuffer(outBuffer, ns);
}