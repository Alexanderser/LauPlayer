//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_AUDIOPLAY_H
#define MY_APPLICATION_AUDIOPLAY_H

extern "C"{
#include <libavcodec/avcodec.h>
};

#include "JavaCallHelper.h"
#include "BasePlay.h"

class AudioPlay : public BasePlay{

public:
    AudioPlay(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
              AVRational base);

    ~AudioPlay();
    void play();
    void stop();
};

#endif //MY_APPLICATION_AUDIOPLAY_H
