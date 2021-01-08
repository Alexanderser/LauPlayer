//
// Created by dell on 2021/1/7.
//

#include "VideoPlay.h"
#include "JavaCallHelper.h"

VideoPlay::VideoPlay(int id1, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                     AVRational base, int id, JavaCallHelper *pHelper,
                     AVCodecContext *pContext) : BasePlay(id1, javaCallHelper, avCodecContext, base) {

}

void *decode(void *args){

};

void VideoPlay::play() {
    BasePlay::play();
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    pthread_create(&pid_video_play, NULL, decode, this);
}

void VideoPlay::stop() {
    BasePlay::stop();
}

void VideoPlay::decodePacket() {
    //子线程
    AVPacket *packet = 0;
    while (isPlaying) {

    }
}

