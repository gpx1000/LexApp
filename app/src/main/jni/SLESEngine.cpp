#include "SLESEngine.h"
#include <android/log.h>

#define checkSlError(x) checkSlESError(x,__LINE__)
#define  LOG_TAG    "SLESEngine"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

SoundInfo::SoundInfo(const int16_t * data, size_t length)
 : mLength(length)
{
    mData = new int16_t[length];
    memcpy(mData, data, length);
}

SoundInfo::~SoundInfo()
{
    delete mData;
}

SLESEngine::SLESEngine()
 : m_EngineObject(0)
 , m_EngineEngine(0)
 , m_OutputMixObject(0)
 , m_PlayerObject(0)
 , m_PlayerPlay(0)
 , m_PlayerBufferQueue(0)
 , m_PlayerMuteSolo(0)
 , m_PlayerVolume(0)
 , m_curBuffer(0)
{
    // engine
    const SLInterfaceID engineMixIIDs[] = {SL_IID_ENGINE};
    const SLboolean engineMixReqs[] = {SL_BOOLEAN_TRUE};
    // create engine
    SLuint32 result = slCreateEngine(&m_EngineObject, 0, 0, 1, engineMixIIDs, engineMixReqs);
    if(checkSlError(result)) return;
    // realize
    result = (*m_EngineObject)->Realize(m_EngineObject, SL_BOOLEAN_FALSE);
    if(checkSlError(result)) return;
    // get interfaces
    result = (*m_EngineObject)->GetInterface(m_EngineObject, SL_IID_ENGINE, &m_EngineEngine);
    if(checkSlError(result)) return;

    // create output
    result = (*m_EngineEngine)->CreateOutputMix(m_EngineEngine, &m_OutputMixObject, 0, 0, 0);
    if(checkSlError(result)) return;
    result = (*m_OutputMixObject)->Realize(m_OutputMixObject, SL_BOOLEAN_FALSE);
    if(checkSlError(result)) return;
}

SLESEngine::~SLESEngine()
{
    // destroy sound player
    stopSoundPlayer();
    std::list<SoundInfo*>::const_iterator citer = mSounds.begin();
    for(; citer != mSounds.end(); citer++)
    {
        if(*citer != 0)
            delete (*citer);
    }
    mSounds.clear();

    if (m_OutputMixObject != 0)
    {
        (*m_OutputMixObject)->Destroy(m_OutputMixObject);
        m_OutputMixObject = 0;
    }

    if (m_EngineObject != 0)
    {
        (*m_EngineObject)->Destroy(m_EngineObject);
        m_EngineObject = 0;
        m_EngineEngine = 0;
    }
}

void SLESEngine::startSoundPlayer()
{
    // INPUT
    LOGD("Starting Audio Player");
    SLDataLocator_AndroidSimpleBufferQueue dataLocatorInput;
    dataLocatorInput.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    dataLocatorInput.numBuffers = 1;

    // format of data
    SLDataFormat_PCM dataFormat;
    dataFormat.formatType = SL_DATAFORMAT_PCM;
    dataFormat.numChannels = 1;
    dataFormat.samplesPerSec = SL_SAMPLINGRATE_22_05;
    dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
    dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

    // combine location and format into source
    SLDataSource dataSource;
    dataSource.pLocator = &dataLocatorInput;
    dataSource.pFormat = &dataFormat;
    // OUTPUT
    SLDataLocator_OutputMix dataLocatorOut;
    dataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    dataLocatorOut.outputMix = m_OutputMixObject;

    SLDataSink dataSink;
    dataSink.pLocator = &dataLocatorOut;
    dataSink.pFormat = NULL;
    // create sound player
    const SLuint32 soundPlayerIIDCount = 3;
    const SLInterfaceID soundPlayerIIDs[] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean soundPlayerReqs[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    SLuint32 result =(*m_EngineEngine)->CreateAudioPlayer(m_EngineEngine, &m_PlayerObject, &dataSource, &dataSink, soundPlayerIIDCount, soundPlayerIIDs, soundPlayerReqs);
    if(checkSlError(result)) return;
    result = (*m_PlayerObject)->Realize(m_PlayerObject, SL_BOOLEAN_FALSE);
    if(checkSlError(result)) return;
    // get interfaces
    result = (*m_PlayerObject)->GetInterface(m_PlayerObject, SL_IID_PLAY, &m_PlayerPlay);
    if(checkSlError(result)) return;
    result = (*m_PlayerObject)->GetInterface(m_PlayerObject, SL_IID_BUFFERQUEUE, &m_PlayerBufferQueue);
    if(checkSlError(result)) return;
    result = (*m_PlayerObject)->GetInterface(m_PlayerObject, SL_IID_VOLUME, &m_PlayerVolume);
    if(checkSlError(result)) return;
    result = (*m_PlayerBufferQueue)->RegisterCallback(m_PlayerBufferQueue, PlayerCallback, this);
    if(checkSlError(result)) return;
    SLmillibel MaxVolume = SL_MILLIBEL_MIN;
    result = (*m_PlayerVolume)->GetMaxVolumeLevel(m_PlayerVolume, &MaxVolume);
    if(checkSlError(result)) return;
    result - (*m_PlayerVolume)->SetVolumeLevel(m_PlayerVolume, MaxVolume);
    if(checkSlError(result)) return;
    sendSoundBuffer();
}

void SLESEngine::stopSoundPlayer()
{
    if (m_PlayerObject != 0)
    {
        SLuint32 soundPlayerState;
        (*m_PlayerObject)->GetState(m_PlayerObject, &soundPlayerState);

        if (soundPlayerState == SL_OBJECT_STATE_REALIZED)
        {
            (*m_PlayerBufferQueue)->Clear(m_PlayerBufferQueue);
            (*m_PlayerObject)->AbortAsyncOperation(m_PlayerObject);
            (*m_PlayerObject)->Destroy(m_PlayerObject);
            m_PlayerObject = 0;
            m_PlayerPlay = 0;
            m_PlayerBufferQueue = 0;
            m_PlayerVolume = 0;
        }
    }
    std::list<SoundInfo*>::const_iterator citer = mSounds.begin();
    for(; citer != mSounds.end(); citer++)
    {
        if(*citer != 0)
            delete (*citer);
    }
    mSounds.clear();
}

void SLESEngine::PlayerCallback( SLAndroidSimpleBufferQueueItf bq, void *context )
{
    ((SLESEngine*) context)->sendSoundBuffer();
}

bool SLESEngine::checkSlESError(SLuint32 result, int line)
{
    if (result != SL_RESULT_SUCCESS)
    {
        LOGE("0x%x error code encountered at line %d, exiting\n", result, line);
        stop();
        return true;
    }
    return false;
}

void SLESEngine::sendSoundBuffer()
{
    prepareSoundBuffer();
    SLuint32 result = (*m_PlayerBufferQueue)->Enqueue(m_PlayerBufferQueue, &BufferQueue[0], sizeof(uint16_t) * BufferQueue.size());
    if(checkSlError(result)) return;
}

void SLESEngine::prepareSoundBuffer()
{
    BufferQueue.clear();
    std::list<SoundInfo*>::iterator iter = mSounds.begin();
    for(; iter != mSounds.end(); iter++)
    {
        BufferQueue.insert(BufferQueue.end(), (*iter)->mData, (*iter)->mData + (*iter)->mLength);
        delete (*iter);
    }
    mSounds.clear();
    if(BufferQueue.empty())
    {
        BufferQueue.assign(AUDIO_BUFFER_SIZE, 0);
        LOGW("Buffer was empty!!!!");
    }
}

void SLESEngine::playBuffer(const int16_t* const dataIn, size_t length)
{
    mSounds.push_back(new SoundInfo(dataIn, length * sizeof(int16_t)));
    SLuint32 state;
    SLuint32 result = (*m_PlayerPlay)->GetPlayState(m_PlayerPlay, &state );
    if(checkSlError(result)) return;
    if(state != SL_PLAYSTATE_PLAYING)
        result = (*m_PlayerPlay)->SetPlayState(m_PlayerPlay, SL_PLAYSTATE_PLAYING);
    if(checkSlError(result)) return;
}

void SLESEngine::stop()
{
    SLuint32 state;
    SLuint32 result = (*m_PlayerPlay)->GetPlayState(m_PlayerPlay, &state );
    if(checkSlError(result)) return;
    if(state != SL_PLAYSTATE_STOPPED)
        result = (*m_PlayerPlay)->SetPlayState(m_PlayerPlay, SL_PLAYSTATE_STOPPED);
    if(checkSlError(result)) return;
}
