//
// Created by dell on 2021/1/6.
//

#ifndef MY_APPLICATION_JAVACALLHELPER_H
#define MY_APPLICATION_JAVACALLHELPER_H


#include "../../../../../../sdk/ndk/21.3.6528147/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/jni.h"

class JavaCallHelper{

public:
    JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj);
    ~JavaCallHelper();

    void onError(int thread, int code);

    void onPrepare(int thread);

    void onProgress(int thread, int progress);

private:
    JavaVM *javaVm;
    JNIEnv *env;
    jobject jobj;
    jmethodID jmid_prepare;
    jmethodID jmid_error;
    jmethodID jmid_progress;
};

#endif //MY_APPLICATION_JAVACALLHELPER_H
