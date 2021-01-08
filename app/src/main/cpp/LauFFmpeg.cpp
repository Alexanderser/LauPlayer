//
// Created by dell on 2021/1/5.
//


#include "LauFFmpeg.h"
#include "macro.h"
#include "VideoPlay.h"
#include "AudioPlay.h"
#include"android/log.h"
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"lau",FORMAT,##__VA_ARGS__);

void *prepareFFmpeg_(void *args){
    LauFFmpeg *lauFFmpeg = static_cast<LauFFmpeg *>(args);
    lauFFmpeg->prepareFFmpeg();
    return 0;
}

LauFFmpeg::LauFFmpeg(JavaCallHelper *javaCallHelper, const char *dataSource) {
    url = new char[strlen(dataSource) + 1];
    this->javaCallHelper = javaCallHelper;
    strcpy(url, dataSource);
}

LauFFmpeg::~LauFFmpeg() {

}

void LauFFmpeg::prepare() {
    pthread_create(&pid_prepare, NULL, prepareFFmpeg_, this);
}

void LauFFmpeg::prepareFFmpeg() {
    avformat_network_init();
    //代表一个视频中所有信息
    formatContext = avformat_alloc_context();
    //1,打开url
    AVDictionary *opts = NULL;
    //设置超时3秒
    av_dict_set(&opts, "timeout", "3000000", 0);
    //强制指定AVFormatContext中AVInputFormat的.这个参数可一设置为null,这样ffmpeg会自动检测AVInputFormat
    //输入文件封装格式
//    av_find_input_format("avi")
    int ret = avformat_open_input(&formatContext, url, NULL, &opts);
    if (ret != 0) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CANNOT_OPEN_URL);
        return;
    }
    //2,查找流
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CANNOT_FIND_STREAMS);
        }
        return;
    }
    //遍历流
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVCodecParameters *codep = formatContext->streams[i]->codecpar;
        //找到解码器
        AVCodec *dec = avcodec_find_decoder(codep->codec_id);
        if (!dec) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }
        //创建上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(dec);
        if (!codecContext) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }
        //打开解码器
        if (avcodec_open2(codecContext, dec, 0) != 0) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }
        if (codep->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频
            audioPlay = new AudioPlay(i, javaCallHelper, codecContext, AVRational());
        } else if (codep->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频
            videoPlay = new VideoPlay(i, javaCallHelper, codecContext, AVRational());
            videoPlay->setRenderCallBack(renderFrame);
        }

        if (!audioPlay && !videoPlay) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
            }
            return;
        }
        if (javaCallHelper) {
            javaCallHelper->onPrepare(THREAD_CHILD);
        }
    }
}

void *startThread(void *args){
    LauFFmpeg *lauFFmpeg = static_cast<LauFFmpeg *>(args);
    lauFFmpeg->play();
    return 0;
};

void LauFFmpeg::start() {
    isPlaying = true;
    if (audioPlay) {
        audioPlay->play();
    }
    if (videoPlay){
        videoPlay->play();
    }
    pthread_create(&pid_play, NULL, startThread, this);
}

void LauFFmpeg::play() {
    int ret = 0;
    while (isPlaying) {
        if (audioPlay && audioPlay->pkt_queue.size() > 100) {
            //思想:队列 生产者的生产速度远远大于消费者的速度  休眠10ms
            av_usleep(1000 * 10);
            continue;
        }
        if (videoPlay && videoPlay->pkt_queue.size() > 100) {
            //思想:队列 生产者的生产速度远远大于消费者的速度  休眠10ms
            av_usleep(1000 * 10);
            continue;
        }
        //读取包
        AVPacket *avPacket = av_packet_alloc();
        //从媒体读取音频,视频包
        ret = av_read_frame(formatContext, avPacket);
        if (ret == 0) {
            if (audioPlay && avPacket->stream_index == audioPlay->channelId) {
                audioPlay->pkt_queue.put(avPacket);
            } else if (videoPlay && avPacket->stream_index == videoPlay->channelId) {
                videoPlay->pkt_queue.put(avPacket);
            }
        } else if (ret == AVERROR_EOF) {
            //读取完毕
            if (videoPlay->pkt_queue.empty() && videoPlay->frame_queue.empty() &&
                audioPlay->pkt_queue.empty() && audioPlay->frame_queue.empty()) {
                LOGE("播放完毕...")
                break;
            }
            //因为seek的存在,就算读取完毕,也要循环执行av_read_frame(否则seek无效
        } else {
            break;
        }
    }

    isPlaying = 0;
    audioPlay->stop();
    videoPlay->stop();
}

void LauFFmpeg::setRenderCallback(RenderFrame renderFrame1) {
    this->renderFrame = renderFrame1;
}

