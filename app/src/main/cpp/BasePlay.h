//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_BASEPLAY_H
#define MY_APPLICATION_BASEPLAY_H

#include "safe_queue.h"
#include "JavaCallHelper.h"
extern "C"{
#include "libavutil/frame.h"
};

class BasePlay{
public:
    BasePlay(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
             AVRational base)
            : channelId(id), javaCallHelper(javaCallHelper), avCodecContext(avCodecContext) {
    }

    static void releaseAvPacket(AVPacket *&packet){
        if (packet) {
            av_packet_free(&packet);
            packet = 0;
        }
    };

    static void releaseAvFrame(AVFrame *&frame){
        if (frame) {
            av_frame_free(&frame);
            frame = 0;
        }
    };
    virtual ~BasePlay(){
        if (avCodecContext) {
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = 0;
        }
        pkt_queue.clear();
        frame_queue.clear();
    }
    virtual void play(){};
    virtual void stop(){};

    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext *avCodecContext;
    JavaCallHelper *javaCallHelper;
};

#endif //MY_APPLICATION_BASEPLAY_Hd
