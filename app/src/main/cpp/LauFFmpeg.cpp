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
    auto *lauFFmpeg = static_cast<LauFFmpeg *>(args);
    lauFFmpeg->prepareFFmpeg();
    return nullptr;
}

LauFFmpeg::LauFFmpeg(JavaCallHelper *javaCallHelper, const char *dataSource) {
    url = new char[strlen(dataSource) + 1];
    this->javaCallHelper = javaCallHelper;
    strcpy(url, dataSource);
}

LauFFmpeg::~LauFFmpeg() = default;

void LauFFmpeg::prepare() {
    pthread_create(&pid_prepare, nullptr, prepareFFmpeg_, this);
}

void LauFFmpeg::prepareFFmpeg() {
    avformat_network_init();
    //代表一个视频中所有信息
    formatContext = avformat_alloc_context();
    //1,打开url
    AVDictionary *opts = nullptr;
    //设置超时3秒
    av_dict_set(&opts, "timeout", "3000000", 0);
    //强制指定AVFormatContext中AVInputFormat的.这个参数可一设置为null,这样ffmpeg会自动检测AVInputFormat
    //输入文件封装格式
//    av_find_input_format("avi")
    int ret = avformat_open_input(&formatContext, url, nullptr, &opts);
    if (ret != 0) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CANNOT_OPEN_URL);
        return;
    }
    //2,查找流
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CANNOT_FIND_STREAMS);
        }
        return;
    }
    //遍历流
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVCodecParameters *codep = formatContext->streams[i]->codecpar;
        AVStream *stream = formatContext->streams[i];
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
        //复制参数
        if (avcodec_parameters_to_context(codecContext, codep)) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }
        //打开解码器
        if (avcodec_open2(codecContext, dec, nullptr) != 0) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }
        if (codep->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频
            audioPlay = new AudioPlay(i, javaCallHelper, codecContext,stream->time_base);
        } else if (codep->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVRational frame_rate = stream->avg_frame_rate;
//            int fps = frame_rate.num / frame_rate.den;
            //视频
            LOGE("%s%d","lauffmpeg",codecContext->width)
            LOGE("%s%d","lauffmpeg",codecContext->height)
            videoPlay = new VideoPlay(i, javaCallHelper, codecContext,stream->time_base);
            videoPlay->setRenderCallBack(renderFrame);
        }


        if(audioPlay&&videoPlay) {
            if (javaCallHelper) {
                javaCallHelper->onPrepare(THREAD_CHILD);
            }
        }
    }
    if (!audioPlay && !videoPlay) {
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        }
        return;
    }
    videoPlay->audioPlay = audioPlay;

}

void *startThread(void *args){
    auto *lauFFmpeg = static_cast<LauFFmpeg *>(args);
    lauFFmpeg->play();
    return nullptr;
}

void LauFFmpeg::start() {
    isPlaying = true;
    if (audioPlay) {
        audioPlay->play();
    }
    if (videoPlay){
        videoPlay->play();
    }
    pthread_create(&pid_play, nullptr, startThread, this);
}

void LauFFmpeg::play() {
    int ret ;
    while (isPlaying) {
        if (audioPlay && audioPlay->pkt_queue.size() > 100) {
            //思想:队列 生产者的生产速度远远大于消费者的速度  休眠10ms
            av_usleep(1000 * 10);
            continue;
        }
        if (videoPlay && videoPlay->pkt_queue.size() > 100) {
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
//                LOGE("取一帧音频")
            } else if (videoPlay && avPacket->stream_index == videoPlay->channelId) {
                videoPlay->pkt_queue.put(avPacket);
//                LOGE("取一帧视频")
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

    isPlaying = false;
    audioPlay->stop();
    videoPlay->stop();
}

void LauFFmpeg::setRenderCallback(RenderFrame renderFrame1) {
    this->renderFrame = renderFrame1;
}

