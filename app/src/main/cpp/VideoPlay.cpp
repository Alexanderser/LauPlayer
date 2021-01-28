//
// Created by dell on 2021/1/7.
//

#include "VideoPlay.h"
#include "JavaCallHelper.h"
#include"android/log.h"
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"lau",FORMAT,##__VA_ARGS__);
extern "C"{
#include <libavutil/time.h>
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
}

void dropPacket(queue<AVPacket *> &queue){
    while (!queue.empty()) {
        LOGE("丢视频帧....")
        AVPacket *pkt = queue.front();
        if (pkt->flags != AV_PKT_FLAG_KEY) {
            queue.pop();
            BasePlay::releaseAvPacket(pkt);
        } else {
            break;
        }
    }
};

void *decode(void *args){
    auto *videoPlay = static_cast<VideoPlay *>(args);
    videoPlay->decodePacket();
    return nullptr;
}

void *synchronize(void *args) {
    auto *videoPlay = static_cast<VideoPlay *>(args);
    videoPlay->synchronizeFrame();
    return nullptr;
}

void VideoPlay::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    pthread_create(&pid_video_play, nullptr, decode, this);
    pthread_create(&pid_synchronize, nullptr, synchronize, this);
}

void VideoPlay::stop() {
}

void VideoPlay::decodePacket() {
    //子线程
    AVPacket *packet = nullptr;
    while (isPlaying) {
        int ret = pkt_queue.get(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            //失败
            break;
        }
        AVFrame *frame = av_frame_alloc();
        avcodec_receive_frame(avCodecContext, frame);
        frame_queue.put(frame);
        while (frame_queue.size() > 100 && isPlaying) {
            av_usleep(10 * 1000);
       }
    }
    releaseAvPacket(packet);
}

void VideoPlay::synchronizeFrame() {
    LOGE("%d",222222)
    LOGE("%d", avCodecContext->width)
    LOGE("%d",avCodecContext->height)
    if (avCodecContext == nullptr) {
        LOGE("avCodecContext is null")
    } else {
        LOGE("avCodecContext not null")
    }
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                            avCodecContext->pix_fmt, avCodecContext->width,
                                            avCodecContext->height, AV_PIX_FMT_RGBA,
                                            SWS_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t *dst_data[4];
    int dst_linesize[4];
    av_image_alloc(dst_data, dst_linesize, avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);
    AVFrame *frame = nullptr;
    while (isPlaying) {
        int ret = frame_queue.get(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        sws_scale(swsContext, reinterpret_cast<const uint8_t *const *>(frame->data), frame->linesize,
                  0,
                  frame->height, dst_data, dst_linesize);
        //数据回调,将RGBA数据传到lauffmpeg进行播放
        renderFrame(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
        av_usleep(16 * 1000);
//        clock = frame->pts * av_q2d(time_base);
////        double frame_delays = 1.0 / fps; //大概16ms
//        double audioClock = audioPlay->clock;
//        double diff = clock - audioClock;
//        //把解码时间算进去
//        double delay = frame->repeat_pict;
//        //将解码需要时间算进去,因为配置差的手机,解码耗时需要多一点
//        double extra_delay = frame->repeat_pict / (2 * fps);
//        if (clock > audioClock) {
//            //视频超前
//            if (diff > 1) {
//                av_usleep((int) (2*delay) * 1000000);
//            } else {
//                av_usleep((int) (delay + diff) * 1000000);
//            }
//        } else {
//            //视频延后
//            if (diff > 1) {
//                //不休眠
//            } else if (diff >= 0.05) {
//                //丢帧
////                releaseAvFrame(frame);
//            } else {
//
//            }
//        }


        releaseAvFrame(frame);
    }
    av_freep(&dst_data[0]);
    isPlaying = false;
    releaseAvFrame(frame);
    sws_freeContext(swsContext);

}

void VideoPlay::setRenderCallBack(RenderFrame renderFrame1) {
    this->renderFrame = renderFrame1;
}

void dropFrame(queue<AVFrame *> &queue){
    if (!queue.empty()) {
        AVFrame *frame = queue.front();
        queue.pop();
        BasePlay::releaseAvFrame(frame);
    }
};

VideoPlay::VideoPlay(int id1, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                     AVRational base) : BasePlay(id1, javaCallHelper, avCodecContext, base) {
    if (avCodecContext == nullptr){
        LOGE("avCodecContext is null")
    } else {
        LOGE("avCodecContext not null")
        LOGE("%d",avCodecContext->width)
        LOGE("%d",avCodecContext->height)
    }
//    frame_queue.setReleaseHandle(releaseAvFrame);
//    frame_queue.setSyncHandle(dropFrame);

}

void VideoPlay::setFps(int fps1) {
    this->fps = fps1;
}




