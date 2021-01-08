//
// Created by dell on 2021/1/5.
//


#include "LauFFmpeg.h"
#include "macro.h"
#include "VideoPlay.h"
#include "AudioPlay.h"

void *prepareFFmpeg_(void *args){
    LauFFmpeg *lauFFmpeg = static_cast<LauFFmpeg *>(args);
    lauFFmpeg->prepareFFmpeg();
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
            audioPlay = new AudioPlay(i, javaCallHelper, codecContext);
        } else if (codep->codec_type == AVMEDIA_TYPE_VIDEO) {
            //视频
            videoPlay = new VideoPlay(0, nullptr, nullptr, AVRational(), i, javaCallHelper,
                                      codecContext);
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

