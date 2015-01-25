#include <jni.h>
#include <android/log.h>

#include "SSDRenderer.h"
#include "CameraPreview.h"
#include "Trilateration.h"

#define  LOG_TAG    "SSDRenderer"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

extern "C"
{
JavaVM* gJavaVM = 0;

jint JNI_OnLoad(JavaVM* aVm, void* aReserved)
{
    LOGD("JNI_ONLOAD");
    // cache java VM
    gJavaVM = aVm;

    JNIEnv* env;
    if (aVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        LOGE("Failed to get the environment");
        return -1;
    }
    LOGD("JNI_ONLOAD FINISHED");
    return JNI_VERSION_1_6;
}

void Java_com_elasmotech_givingthroughglass_lexapp_MainActivity_SSDStart(JNIEnv * env, jobject javaObj)
{
    if(!SSDRenderer::GetThe()->getIsRunning())
        SSDRenderer::GetThe()->start();
}

void Java_com_elasmotech_givingthroughglass_lexapp_MainActivity_SSDStop(JNIEnv * env, jobject javaObj)
{
    if(SSDRenderer::GetThe()->getIsRunning())
        SSDRenderer::GetThe()->stop();
}

void Java_com_elasmotech_givingthroughglass_lexapp_MainActivity_generateSSDFromMat(JNIEnv * env, jobject javaObj, jlong matPtr)
{
    cv::Mat * inputMat = (cv::Mat*)matPtr;
    if(inputMat != 0)
        SSDRenderer::GetThe()->generateSoundScape(inputMat);
}

void Java_com_elasmotech_givingthroughglass_lexapp_BeaconFinder_addMeasurementDevice(JNIEnv * env, jobject javaObj, jstring javaName, jint RSSI, jint Distance)
{
    LOGD("AddMeasurementDevice");
    if(!javaName)
        return;
    std::string name;
    const char *s = env->GetStringUTFChars(javaName,0);
    name = s;
    env->ReleaseStringUTFChars(javaName,s);
    std::vector<BeaconMeas> beaconMeasurements;
    BeaconMeas beacon;
    beacon.setBeaconId(name);
    beacon.setRssi(RSSI);
    beacon.setDistance(Distance);
    beaconMeasurements.push_back(beacon);
    Trilateration::GetThe()->updateMeasurements(beaconMeasurements);
}

void Java_com_elasmotech_givingthroughglass_lexapp_BeaconFinder_addDevice(JNIEnv * env, jobject javaObj, jstring javaName, jstring javaBeaconID, jstring javaSubLoc, jint X, jint Y )
{
    LOGD("AddDevice");
    if(!javaName || !javaBeaconID || !javaSubLoc)
        return;
    std::string name, beaconId, subLoc;
    const char *s = env->GetStringUTFChars(javaName, 0);
    name = s;
    env->ReleaseStringUTFChars(javaName, s);
    s = env->GetStringUTFChars(javaBeaconID, 0);
    beaconId = s;
    env->ReleaseStringUTFChars(javaBeaconID, s);
    s = env->GetStringUTFChars(javaSubLoc, 0);
    subLoc = s;
    env->ReleaseStringUTFChars(javaSubLoc, s);
    std::vector<Beacon> beaconVec;
    Beacon beacon;
    beacon.fillData(X, Y, name, beaconId, subLoc);
    beaconVec.push_back(beacon);
    Trilateration::GetThe()->fillLocationBeacons(beaconVec);
}

} // extern "C"