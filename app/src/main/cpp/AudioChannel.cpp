
#include "AudioChannel.h"
#include <cstring>
#include <opencl-c-base.h>
#include "faac.h"
#include "librtmp/rtmp.h"
#include"android/log.h"
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"lau",FORMAT,##__VA_ARGS__);

RTMPPacket  *AudioChannel::getAudioTag(){
    u_char *buf;
    u_long byteLen;
    faacEncGetDecoderSpecificInfo(audioCodec, &buf, &byteLen);
    int bodySize = 2 + byteLen;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);
    packet->m_body[0] = 0xAF;
    packet->m_body[1] = 0x01;
    memcpy(&packet->m_body[2], buffer, byteLen);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x12;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE ;
    return packet;
}

//不断调用编码
void AudioChannel::encodeData(int8_t *data) {
    int byteLen = faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), inputSamples, buffer,
                                maxOutputBytes);
    if (byteLen > 0) {
        LOGE("取一频次音频")
        RTMPPacket *packet = new RTMPPacket;
        int bodySize = 2 + byteLen;
        RTMPPacket_Alloc(packet, bodySize);
        packet->m_body[0] = 0xAF;
        packet->m_body[1] = 0x01;
        memcpy(&packet->m_body[2], buffer, byteLen);

        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x12;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE ;
        callback(packet);
    }
}

void AudioChannel::setAudioEncInfo(int samplesInHZ, int channels) {
    audioCodec = faacEncOpen(samplesInHZ, channels, &inputSamples, &maxOutputBytes);
    //给编码器设置参数
    faacEncConfigurationPtr ptr = faacEncGetCurrentConfiguration(audioCodec);
    ptr->mpegVersion = MPEG4;
    ptr->aacObjectType = LOW;
    ptr->inputFormat = FAAC_INPUT_16BIT;
    ptr->outputFormat = 0;
    faacEncSetConfiguration(audioCodec, ptr);
    buffer = new u_char[maxOutputBytes];
}

int AudioChannel::getInputSamples() const {
    return inputSamples;
}

void AudioChannel::setCallback(AudioChannel::AudioCallback audioCallback) {
    this->callback = audioCallback;
}
