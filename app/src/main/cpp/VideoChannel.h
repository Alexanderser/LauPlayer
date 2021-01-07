//
// Created by dell on 2020/12/25.
//

#ifndef MY_APPLICATION_VIDEOCHANNEL_H
#define MY_APPLICATION_VIDEOCHANNEL_H

#include <jni.h>
#include "librtmp/rtmp.h"

class VideoChannel{
    typedef void (*VideoCallBack)(RTMPPacket *packet);

public:
    void setVideoEncInfo(jint width, jint height, jint fps, jint bitrate);

    void setVideoCallback(VideoCallBack videoCallBack);

    void encodeData(int8_t *data);

private:
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int ySize;
    int uvSize;
    x264_t *videoCodec;
    x264_picture_t *pic_in;
    VideoCallBack videoCallBack;

    void  sendFrame(int type, uint8_t *payload, int i_payload);
    void sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len);
};

#endif //MY_APPLICATION_VIDEOCHANNEL_H
