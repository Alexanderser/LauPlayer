//
// Created by dell on 2021/1/4.
//
#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include "LauFFmpeg.h"
#include "JavaCallHelper.h"
extern "C"{
#include "libavcodec/avcodec.h"
}
JavaCallHelper *javaCallHelper;

ANativeWindow *window = 0;
LauFFmpeg *lauFFmpeg;
JavaVM *javaVm = NULL;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVm = vm;
    return JNI_VERSION_1_4;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LauPlayer_native_1prepare(JNIEnv *env, jobject thiz,
                                                             jstring data_source_) {
    const char *dataSource = env->GetStringUTFChars(data_source_, 0);
    javaCallHelper = new JavaCallHelper(javaVm, env, thiz);
    lauFFmpeg= new LauFFmpeg(javaCallHelper, dataSource);

    env->ReleaseStringUTFChars(data_source_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LauPlayer_native_1start(JNIEnv *env, jobject thiz) {
    if (lauFFmpeg) {
        lauFFmpeg.start();
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LauPlayer_native_1set_1surface(JNIEnv *env, jobject thiz,
                                                                  jobject surface) {


}


