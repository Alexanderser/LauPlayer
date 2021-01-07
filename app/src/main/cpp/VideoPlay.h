//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_VIDEOPLAY_H
#define MY_APPLICATION_VIDEOPLAY_H

#include <libavcodec/avcodec.h>
#include "JavaCallHelper.h"

class VideoPlay{

public:
    VideoPlay(int id, JavaCallHelper *pHelper, AVCodecContext *pContext);
    void play();
};

#endif //MY_APPLICATION_VIDEOPLAY_H
