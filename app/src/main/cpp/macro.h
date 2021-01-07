//
// Created by dell on 2021/1/7.
//

#ifndef MY_APPLICATION_MACRO_H
#define MY_APPLICATION_MACRO_H

#define THREAD_MAIN 1
#define THREAD_CHILD 2

//错误代码
//打不开视频
#define FFMPEG_CANNOT_OPEN_URL 1
//找不到流媒体
#define FFMPEG_CANNOT_FIND_STREAMS 2
//找不到解码器
#define FFMPEG_FIND_DECODER_FAIL 3
//无法根据解码器创建上下文
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
//根据流信息,配置上下文参数失败
#define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL 6
//打开解码器失败
#define FFMPEG_OPEN_DECODER_FAIL 7
//没有音视频
#define FFMPEG_NOMEDIA 8

#endif //MY_APPLICATION_MACRO_H
