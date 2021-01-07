//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_AUDIOPLAY_H
#define MY_APPLICATION_AUDIOPLAY_H

#include <libavcodec/avcodec.h>
#include "JavaCallHelper.h"

class AudioPlay{

public:
    AudioPlay(int i, JavaCallHelper *pHelper, AVCodecContext *pContext);
    void play();
};

#endif //MY_APPLICATION_AUDIOPLAY_H
