//
// Created by dell on 2021/1/7.
//

#include "VideoPlay.h"
#include "JavaCallHelper.h"
extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
}

void *decode(void *args){
    auto *videoPlay = static_cast<VideoPlay *>(args);
    videoPlay->decodePacket();
    return 0;
};

void *synchronize(void *args) {
    auto *videoPlay = static_cast<VideoPlay *>(args);
    videoPlay->synchronizeFrame();
    return 0;
};

void VideoPlay::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    pthread_create(&pid_video_play, NULL, decode, this);
    pthread_create(&pid_synchronize, NULL, synchronize, this);
}

void VideoPlay::stop() {
}

void VideoPlay::decodePacket() {
    //子线程
    AVPacket *packet = 0;
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
            continue;
        }
    }
    releaseAvPacket(packet);
}

void VideoPlay::synchronizeFrame() {
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                            avCodecContext->pix_fmt, avCodecContext->width,
                                            avCodecContext->height, AV_PIX_FMT_RGBA,
                                            SWS_BILINEAR, 0, 0, 0);
    uint8_t *dst_data[4];
    int dst_linesize[4];
    av_image_alloc(dst_data, dst_linesize, avCodecContext->width, avCodecContext->height,
                   AV_PIX_FMT_RGBA, 1);
    AVFrame *frame = 0;
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
        releaseAvFrame(frame);
    }
    av_freep(&dst_data[0]);
    isPlaying = false;
    releaseAvFrame(frame);
    sws_freeContext(swsContext);

}

void VideoPlay::setRenderCallBack(RenderFrame renderFrame) {
    this->renderFrame = renderFrame;
}

VideoPlay::VideoPlay(int id1, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                     AVRational base) : BasePlay(id1, javaCallHelper, avCodecContext, base) {
    this->javaCallHelper = javaCallHelper;
    this->avCodecContext = avCodecContext;
}




