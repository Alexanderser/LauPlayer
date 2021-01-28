//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_AUDIOPLAY_H
#define MY_APPLICATION_AUDIOPLAY_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
};

#include "JavaCallHelper.h"
#include "BasePlay.h"
#include <SLES/OpenSLES_Android.h>


class AudioPlay : public BasePlay{

public:
    AudioPlay(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
              AVRational base);

    ~AudioPlay();
    void play();
    void stop();

    void initOpenSL();

    void decode();
    int getPcm();

    uint8_t *buffer{};

private:
    pthread_t pid_audio_play{};
    pthread_t pid_audio_decode{};
    SwrContext *swrContext = nullptr;
    int out_channels{};
    int out_sample_size{};
    int out_sample_rate{};
};

#endif //MY_APPLICATION_AUDIOPLAY_H
