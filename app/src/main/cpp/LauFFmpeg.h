//
// Created by dell on 2021/1/5.
//

#ifndef MY_APPLICATION_LAUFFMPEG_H
#define MY_APPLICATION_LAUFFMPEG_H
#include <pthread.h>
#include <android/native_window_jni.h>
#include "JavaCallHelper.h"
#include "VideoPlay.h"
#include "AudioPlay.h"

extern "C"{
#include "libavformat/avformat.h"
#include "libavutil/time.h"
};

//控制层
class LauFFmpeg{


public:
    LauFFmpeg(JavaCallHelper *javaCallHelper, const char *dataSource);
    ~LauFFmpeg();

    void prepare();

    void prepareFFmpeg();
    void start();
private:
    //线程引用
    pthread_t pid_prepare;
    AVFormatContext *formatContext;
    char *url;
    JavaCallHelper *javaCallHelper;
    VideoPlay *videoPlay;
    AudioPlay *audioPlay;
    bool isPlaying;
};

#endif //MY_APPLICATION_LAUFFMPEG_H
