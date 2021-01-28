//
// Created by dell on 2021/1/7.
//

#include "AudioPlay.h"
#include "JavaCallHelper.h"
#include"android/log.h"
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"lau",FORMAT,##__VA_ARGS__);
extern "C"{
#include <libavutil/time.h>
}

AudioPlay::AudioPlay(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                     AVRational base)
        : BasePlay(id, javaCallHelper, avCodecContext, base) {
    LOGE("audio")
    if (avCodecContext == nullptr){
        LOGE("avCodecContext is null")
    } else {
        LOGE("avCodecContext not null")
    }
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    buffer = static_cast<uint8_t *>(malloc(out_sample_rate * out_sample_size * out_channels));
}

void *audioPlay(void *args){
    auto *audio = static_cast<AudioPlay *>(args);
    audio->initOpenSL();
    return nullptr;
}

void *audioDecode(void *args){
    auto *audio = static_cast<AudioPlay *>(args);
    audio->decode();
    return nullptr;
}

void AudioPlay::play() {
    swrContext = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_STEREO,
            AV_SAMPLE_FMT_S16,
                                    out_sample_rate,
                                    avCodecContext->channel_layout, avCodecContext->sample_fmt,
                                    avCodecContext->sample_rate,
                                    0, nullptr);
    swr_init(swrContext);
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true;
    //创建初始化OpenSL ES的线程
    pthread_create(&pid_audio_play, nullptr, audioPlay, this);
    //创建初始化音频解码线程
    pthread_create(&pid_audio_decode, nullptr, audioDecode, this);
}

void AudioPlay::stop() {
}

void bqPlayerCallBack(SLAndroidSimpleBufferQueueItf caller,
                       void *pContext){
    auto *audio = static_cast<AudioPlay *>(pContext);
    int dataLen = audio->getPcm();
    if (dataLen > 0) {
        (*caller)->Enqueue(caller, audio->buffer, dataLen);
    }
}

void AudioPlay::initOpenSL() {
    //音频引擎
    SLEngineItf engineInterface = nullptr;
    //音频对象
    SLObjectItf engineObject = nullptr;
    //混音器
    SLObjectItf outputMixObject = nullptr;
    //播放器
    SLObjectItf bqPlayerObject = nullptr;
    //回调接口
    SLPlayItf bqPlayerInterface = nullptr;
    //缓冲队列
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = nullptr;
    //1,初始化播放引擎
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //音频接口 相当于surfaceholder
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //2,初始化混音器
    (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, nullptr, nullptr);
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //3,初始化播放器
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm={SL_DATAFORMAT_PCM//播放pcm格式数据
                          ,2//两个声道(立体声)
                          ,SL_SAMPLINGRATE_44_1//44100HZ的频率
                          ,SL_PCMSAMPLEFORMAT_FIXED_16//采样位数16
                          ,SL_PCMSAMPLEFORMAT_FIXED_16//和位数一样
                          ,SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT//立体声 (前左前右)
                          ,SL_BYTEORDER_LITTLEENDIAN//小端模式
                           };

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioMix = {&outputMix, nullptr};
    SLDataSource slDataSource = {&android_queue, &pcm};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject//播放器,
            , &slDataSource//播放器参数 播放缓冲队列 播放格式
            , &audioMix //播放缓冲区
            , 1 //播放接口回调个数
            , ids//设置播放器队列ID
            , req //是否采用内置播放队列
             );
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    //得到接口后调用 获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);
    //获得播放器接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    //设置播放回调
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallBack, this);
    //设置播放状态
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);
    bqPlayerCallBack(bqPlayerBufferQueue, this);
    LOGE("手动调用播放%d",this->pkt_queue.size())


}

void AudioPlay::decode() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        int ret = pkt_queue.get(packet);
        if (!isPlaying) {
            return;
        }
        if (!ret) {
            //失败
            break;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        LOGE("%d%s%d" ,ret,"---",AVERROR(EAGAIN))
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            break;
        }
        while (frame_queue.size() > 100 && isPlaying) {
            av_usleep(10 * 1000);
       }
        frame_queue.put(frame);
    }
}

int AudioPlay::getPcm() {
    AVFrame *frame = nullptr;
    int data_size = 0;
    while (isPlaying) {
        int ret = frame_queue.get(frame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        uint64_t dst_nb_samples = av_rescale_rnd(swr_get_delay(swrContext, frame->sample_rate) +
                                                 frame->nb_samples, out_sample_rate,
                                                 frame->sample_rate, AV_ROUND_UP);
        //转换,返回值为转换后的sample个数
        int nb = swr_convert(swrContext, &buffer, dst_nb_samples, (const uint8_t **) frame->data,
                             frame->nb_samples);
        //转换后的数据 buffer大小
        data_size = nb * out_channels * out_sample_size;

        clock = frame->pts * av_q2d(time_base);

        break;
    }
    releaseAvFrame(frame);
    return data_size;
}

AudioPlay::~AudioPlay() = default;

