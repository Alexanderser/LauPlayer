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

ANativeWindow *window = nullptr;
LauFFmpeg *lauFFmpeg;
JavaVM *javaVm = nullptr;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVm = vm;
    return JNI_VERSION_1_4;
}

void renderFrame(uint8_t *data, int line_size, int w, int h) {
    //渲染
    ANativeWindow_setBuffersGeometry(window, w, h, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;
    if (ANativeWindow_lock(window, &windowBuffer, nullptr)) {
        ANativeWindow_release(window);
        window = nullptr;
        return;
    }

    //window中的缓冲区数据
    auto *dst_data = static_cast<uint8_t *>(windowBuffer.bits);
    //一行像素的字节数   stride 一行像素个数   rgba 4个字节 ,所以大小为 stride84
    int window_linesize = windowBuffer.stride * 4;
    //数据源
    uint8_t *src_data = data;
    for (int i = 0; i < windowBuffer.height; ++i) {
        memcpy(dst_data + i * window_linesize, src_data + i * line_size, window_linesize);
    }
    ANativeWindow_unlockAndPost(window);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LauPlayer_native_1prepare(JNIEnv *env, jobject thiz,
                                                             jstring data_source_) {
    const char *dataSource = env->GetStringUTFChars(data_source_, nullptr);
    javaCallHelper = new JavaCallHelper(javaVm, env, thiz);
    lauFFmpeg= new LauFFmpeg(javaCallHelper, dataSource);
    lauFFmpeg->setRenderCallback(renderFrame);

    lauFFmpeg->prepare();
    env->ReleaseStringUTFChars(data_source_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LauPlayer_native_1start(JNIEnv *env, jobject thiz) {
    if (lauFFmpeg) {
        lauFFmpeg->start();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LauPlayer_native_1set_1surface(JNIEnv *env, jobject thiz,
                                                                  jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
    window = ANativeWindow_fromSurface(env, surface);

}


