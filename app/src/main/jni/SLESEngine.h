#ifndef SLESENGINE_H
#define SLESENGINE_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdint.h>
#include <list>
#include <vector>

#define AUDIO_BUFFER_SIZE 256

class SoundInfo {
public:
    int16_t* mData;
    uint32_t mLength;
    SoundInfo(const int16_t * data, size_t length);
    ~SoundInfo();
};


class SLESEngine {
public:
    SLESEngine();
    ~SLESEngine();
    void startSoundPlayer();
    void stopSoundPlayer();
    static void PlayerCallback( SLAndroidSimpleBufferQueueItf bq, void *context );
    bool checkSlESError(SLuint32 result, int line);
    void sendSoundBuffer();
    void prepareSoundBuffer();
    void playBuffer(const int16_t* const dataIn, size_t length);
    void stop();
private:
    // mixer
    int16_t mTmpSoundBuffer[AUDIO_BUFFER_SIZE * 2];
    // mixer channels
    std::list<SoundInfo*> mSounds;
    std::vector<int16_t> BufferQueue;

    // engine interfaces
    SLObjectItf m_EngineObject;
    SLEngineItf m_EngineEngine;
    SLObjectItf m_OutputMixObject;
    // buffer queue player interfaces
    SLObjectItf m_PlayerObject;
    SLPlayItf m_PlayerPlay;
    SLAndroidSimpleBufferQueueItf m_PlayerBufferQueue;
    SLMuteSoloItf m_PlayerMuteSolo;
    SLVolumeItf m_PlayerVolume;
    // Double buffering.
    int16_t m_buffer[2][AUDIO_BUFFER_SIZE];
    int m_curBuffer;
};

#endif