//
// Created by dell on 2021/1/7.
//

#include "AudioPlay.h"
#include "JavaCallHelper.h"

AudioPlay::AudioPlay(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,
                     AVRational base)
        : BasePlay(id, javaCallHelper, avCodecContext, base) {

}

void AudioPlay::play() {

}

void AudioPlay::stop() {
}

AudioPlay::~AudioPlay() = default;

