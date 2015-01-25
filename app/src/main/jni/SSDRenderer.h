#ifndef SSDRENDERER
#define SSDRENDERER

#define WIDTH 64
#define HEIGHT 64
#define FS 22050

#include <queue>
#include <opencv2/core.hpp>
#include "Thread.h"
#include "SLESEngine.h"

class SSDRenderer : public Thread
{
public:
    static SSDRenderer * GetThe();
    void Run();
    const bool getIsRunning() { return IsRunning(); }
    const int start();
    const int stop();
    void generateSoundScape(const cv::Mat * drawFrame);
private:
    SSDRenderer();
    ~SSDRenderer();
    void ProcessMono();
    float w[WIDTH];
    float phi0[WIDTH];
    float tau1;
    float tau2;
    float dt;
    float scale;
    double sinTable[WIDTH][2l * (long)(0.5f*FS*1.05f)];
    long ns;
    long m;
    int mWidth;
    int mHeight;
    cv::Mat RenderFrame;
    bool newFrameReady;
    bool goingAway;
    std::queue<int64> time_queue;
    pthread_mutex_t m_mutex;
    std::vector<int16_t> m_audioBuffer;
    SLESEngine SoundEngine;
    bool stoppedEngine;
};

#endif