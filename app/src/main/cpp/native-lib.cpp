#include <jni.h>
#include <string>
#include <iostream>
#include"android/log.h"
#include "safe_queue.h"
using namespace std;

#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"lau",FORMAT,##__VA_ARGS__);

#define MAX_AUDIO_FRME_SIZE 48000*4
extern "C" {
//封装格式
#include "libavformat//avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"
#include "x264.h"
#include "librtmp/rtmp.h"
#include "VideoChannel.h"
//bspatch
extern int main(int argc, const char *argv[]);
#include "AudioChannel.h"
}



extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LauPlayer_sound(JNIEnv *env, jobject thiz, jstring input_,
                                                   jstring output_) {


    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);

    cout << input << endl;
    LOGE("%s",input);
    avformat_network_init();
    //总上下文
    AVFormatContext *formatContext = avformat_alloc_context();
    //打开音频文件
    if (avformat_open_input(&formatContext, input, NULL, NULL) != 0) {
        LOGE("%s", "无法打开音频文件");
        return;
    }
    //获取输入文件信息
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("%s", "无法获取输入文件信息")
        return;
    }
    int audio_stream_idx = -1;
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            break;
        }
    }
    AVCodecParameters *codecpar = formatContext->streams[audio_stream_idx]->codecpar;
    //找到解码器
    AVCodec *avCodec = avcodec_find_decoder(codecpar->codec_id);
    //创建解码器上下文
    AVCodecContext *codecContext = avcodec_alloc_context3(avCodec);
    avcodec_parameters_to_context(codecContext, codecpar);
    avcodec_open2(codecContext, avCodec, NULL);
    //音频转换器上下文
    SwrContext *swrContext = swr_alloc();
    //输入参数
    AVSampleFormat in_sample = codecContext->sample_fmt;
    //输入采样率
    int in_sample_rate = codecContext->sample_rate;
    //输入声道布局
    uint64_t in_ch_layout = codecContext->channel_layout;
    //输出参数 是固定的
    AVSampleFormat out_sample = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    //设置转换器的输入参数和输出参数
    swr_alloc_set_opts(swrContext, out_ch_layout, out_sample, out_sample_rate,
                       in_ch_layout, in_sample, in_sample_rate, 0, NULL);
    //初始化转换器其他默认参数
    swr_init(swrContext);

    uint8_t *out_buffer = (uint8_t *) (av_malloc(2 * 44100));
    FILE *fp_pcm = fopen(output, "wb");

    //读取包
    AVPacket *packet = av_packet_alloc();
    int count = 0;
    AVFrame *frame = av_frame_alloc();
    while (av_read_frame(formatContext, packet) >= 0) {
        avcodec_send_packet(codecContext, packet);
        int ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            LOGE("AVERROR(EAGAIN)>>%d", -11)
            continue;
        } else if (ret < 0) {
            LOGE("解码完成")
            break;
        } else {
            LOGE("ret>>%d", ret)
        }

        if (packet->stream_index != audio_stream_idx) {
            LOGE("继续")
            continue;
        }
        LOGE("正在解码%d", count++);
        //将frame转成统一格式
        swr_convert(swrContext, &out_buffer, 2 * 44100, (const uint8_t **) frame->data,
                    frame->nb_samples);
        //获取布局声道数
        int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
        //缓冲区大小
        int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples, out_sample, 1);
        fwrite(out_buffer, 1, out_buffer_size, fp_pcm);

    }
    LOGE("结束")
    fclose(fp_pcm);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);



    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_BsPatch_BsPatcher(JNIEnv *env, jobject thiz, jstring old_apk,
                                                     jstring patch_, jstring output_) {
    const char *oldApk = env->GetStringUTFChars(old_apk, 0);
    const char *patch = env->GetStringUTFChars(patch_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);

    const char *argv[] = {"", oldApk, output, patch};
    main(4, argv);

    env->ReleaseStringUTFChars(old_apk, oldApk);
    env->ReleaseStringUTFChars(patch_, patch);
    env->ReleaseStringUTFChars(output_, output);
}

VideoChannel *videoChannel;
int isStart =0;
pthread_t pid;
uint32_t start_time;
int readyPushing = 0;
SafeQueue<RTMPPacket *> packets;

AudioChannel *audioChannel;

void callback(RTMPPacket *packet){
    if (packet) {
        //设置时间戳
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        //加入队列
        packets.put(packet);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LivePusher_native_1init(JNIEnv *env, jobject thiz) {
    videoChannel=new VideoChannel;
    videoChannel->setVideoCallback(callback);
    audioChannel = new AudioChannel;
    audioChannel->setCallback(callback);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject thiz,
                                                                      jint width, jint height,
                                                                      jint fps, jint bitrate) {
    if (!videoChannel) {
        return;
    }
    videoChannel->setVideoEncInfo(width, height, fps, bitrate);
}


void releasePackets(RTMPPacket *&packet){
    if (packet) {
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }
}

void *start(void *args){
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    rtmp = RTMP_Alloc();
    if (!rtmp) {
        LOGE("%s", "alloc rtmp 失败")
        return NULL;
    }
    RTMP_Init(rtmp);
    int ret = RTMP_SetupURL(rtmp, url);
    if (!ret) {
        LOGE("%s%s", "设置地址失败:", url)
        return NULL;
    }
    rtmp->Link.timeout = 5;
    RTMP_EnableWrite(rtmp);
    ret = RTMP_Connect(rtmp, 0);
    if (!ret) {
        LOGE("%s%s", "连接服务器:", url)
        return NULL;
    }
    ret = RTMP_ConnectStream(rtmp, 0);
    if (!ret) {
        LOGE("连接流:%s", url)
        return NULL;
    }
    start_time = RTMP_GetTime();
    //开始推流
    readyPushing = 1;
    packets.setWork(1);
    RTMPPacket *packet = 0;
    callback(audioChannel->getAudioTag());
    while (readyPushing) {
        packets.get(packet);
        LOGE("%s","取出一帧数据")
        if (!readyPushing) {
            break;
        }
        if (!packet) {
            continue;
        }
        //设置音频流还是视频流
        packet->m_nInfoField2 = rtmp->m_stream_id;
        ret = RTMP_SendPacket(rtmp, packet, 1);
        releasePackets(packet);
    }
    isStart = 0;
    readyPushing = 0;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    delete (url);
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LivePusher_native_1start(JNIEnv *env, jobject thiz,
                                                            jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    if (isStart) {
        return;
    }
    isStart = 1;
    char *url = new char[strlen(path) + 1];
    strcpy(url, path);
    pthread_create(&pid, 0, start, url);
    env->ReleaseStringUTFChars(path_, path);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LivePusher_native_1pushVideo(JNIEnv *env, jobject thiz,
                                                                jbyteArray data_) {

    if (!videoChannel || !readyPushing) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, NULL);

    videoChannel->encodeData(data);

    env->ReleaseByteArrayElements(data_, data, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LivePusher_native_1pushAudio(JNIEnv *env, jobject thiz,
                                                                jbyteArray bytes_) {
    jbyte *data = env->GetByteArrayElements(bytes_, NULL);
    if (!audioChannel || !readyPushing) {
        return;
    }
    audioChannel->encodeData(data);

    env->ReleaseByteArrayElements(bytes_, data, 0);
}extern "C"
JNIEXPORT void JNICALL
Java_com_foxconn_lau_myapplication_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject thiz,
                                                                      jint i, jint channels) {
    if(audioChannel) {
        audioChannel->setAudioEncInfo(i, channels);
    }
}extern "C"
JNIEXPORT jint JNICALL
Java_com_foxconn_lau_myapplication_LivePusher_getInputSamples(JNIEnv *env, jobject thiz) {

    if (audioChannel) {
        return audioChannel->getInputSamples();
    }
    return -1;

}