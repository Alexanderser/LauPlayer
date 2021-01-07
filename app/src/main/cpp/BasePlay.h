//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_BASEPLAY_H
#define MY_APPLICATION_BASEPLAY_H

#include "safe_queue.h"
#include "include/libavcodec/avcodec.h"
#include "include/libavutil/frame.h"
#include "JavaCallHelper.h"

class BasePlay{
public:
    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *avCodecContext;
    JavaCallHelper *javaCallHelper;
};

#endif //MY_APPLICATION_BASEPLAY_H
