#ifndef __CameraPreview__
#define __CameraPreview__

#include <queue>
#include <GLES2/gl2.h>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "Thread.h"

typedef enum AVCaptureDevicePosition {
    AVCaptureDevicePositionBack,
    AVCaptureDevicePositionFront
}AVCapptureDevicePosition_T;

typedef enum AVCaptureVideoOrientation {
    AVCaptureVideoOrientationPortrait = 1,
    AVCaptureVideoOrientationPortraitUpsideDown,
    AVCaptureVideoOrientationLandscapeRight,
    AVCaptureVideoOrientationLandscapeLeft
}AVCaptureVideoOrientation_T;

class CameraPreview : public Thread
{
public:
    void Run();
    static CameraPreview* GetThe();
    void setupDevice();
    void setSize( float, float );
    const bool getIsRunning() { return IsRunning(); }
    const int start();
    const int stop();
    bool drawFrame();
    const float getAspectRatio() const { return myAspectRatio; }
    AVCaptureVideoOrientation defaultAVCaptureVideoOrientation;
    void SetCameraPosition(AVCaptureDevicePosition newPos);
    const AVCaptureDevicePosition getCameraPosition() const { return AVDevicePosition; }
    void toggleDirection();
    void toggleLight();
    const bool getTorchOn() const;
    void cleanUp();
    float FPS;
    bool isReady;
    void useTestImage();
    const cv::Mat * getDrawTexture() const { return &drawing_frame; }
    bool didSetup;

private:
    CameraPreview();
    ~CameraPreview();
    cv::Size calc_optimal_camera_resolution(const char* supported, int width, int height);
    int ID;
    cv::VideoCapture vidCapture;
    bool goingAway;
    std::queue<int64> time_queue;
    cv::Mat drawing_frame;
    float myAspectRatio;
    GLuint captureTexture;
    GLuint quadShaderProgram;
    GLuint PositionHandle;
    GLuint TexCoordHandle;
    GLuint SamplerHandle;
    int testImageWidth;
    int testImageHeight;
    GLubyte* testTextureData;
    int screenWidth;
    int screenHeight;
    int currentFlashMode;
    pthread_mutex_t m_mutex;
    AVCaptureDevicePosition AVDevicePosition;
};

#endif