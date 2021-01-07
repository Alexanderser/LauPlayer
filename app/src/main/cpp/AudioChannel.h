//
// Created by dell on 2020/12/29.
//

#ifndef MY_APPLICATION_AUDIOCHANNEL_H
#define MY_APPLICATION_AUDIOCHANNEL_H

#include <sys/types.h>
#include <jni.h>
#include "faac.h"
#include "librtmp/rtmp.h"

class AudioChannel{

    typedef void (*AudioCallback)(RTMPPacket *packet);

public:
    void encodeData(signed char *data);
    void setAudioEncInfo(int samplesInHZ, int channels);
    jint getInputSamples() const;

    void setCallback(AudioCallback audioCallback);
    RTMPPacket *getAudioTag();

private:
    int mChannels;
    faacEncHandle audioCodec;
    u_long inputSamples;
    u_long maxOutputBytes;
    u_char *buffer = 0;
    AudioCallback callback;
};


#endif //MY_APPLICATION_AUDIOCHANNEL_H
