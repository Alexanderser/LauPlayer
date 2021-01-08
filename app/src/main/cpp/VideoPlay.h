//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_VIDEOPLAY_H
#define MY_APPLICATION_VIDEOPLAY_H
extern "C"{
#include <libavcodec/avcodec.h>
};
#include "JavaCallHelper.h"
#include "BasePlay.h"

typedef void(*RenderFrame)(uint8_t *, int, int, int);
class VideoPlay : public BasePlay {

public:
    VideoPlay(int id1, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
              AVRational base) ;

    void play();
    void stop();
    void decodePacket();

    void synchronizeFrame();
    void setRenderCallBack(RenderFrame renderFrame);

private:
    pthread_t pid_video_play{};
    pthread_t pid_synchronize{};
    RenderFrame renderFrame{};
};

#endif //MY_APPLICATION_VIDEOPLAY_H
