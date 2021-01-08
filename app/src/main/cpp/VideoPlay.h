//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_VIDEOPLAY_H
#define MY_APPLICATION_VIDEOPLAY_H

#include <libavcodec/avcodec.h>
#include "JavaCallHelper.h"
#include "BasePlay.h"
class VideoPlay : public BasePlay {

public:
    VideoPlay(int id1, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
              AVRational base, int id, JavaCallHelper *pHelper,
              AVCodecContext *pContext);

    void play();
    void stop();
    void decodePacket();
private:
    pthread_t pid_video_play;
    pthread_t pid_synchronize;

};

#endif //MY_APPLICATION_VIDEOPLAY_H
