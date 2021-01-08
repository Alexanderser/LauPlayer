cmake_minimum_required(VERSION 3.4.1)
file(GLOB bzip ${CMAKE_SOURCE_DIR}/src/main/cpp/bzip/*.c)
file(GLOB SOURCE src/main/cpp/*.cpp)
#将所有.cpp文件生成ffmpeg
#file(GLOB SOURCE src/main/cpp/*.cpp)
add_library( # Sets the name of the library.
        native-lib
        SHARED
        ${SOURCE}
        src/main/cpp/bspatch.c
        ${bzip})

#set(distribution_DIR libs)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/")

include_directories(src/main/cpp/bzip)
include_directories(src/main/cpp/include src/main/cpp)
add_subdirectory(src/main/cpp/librtmp)

add_library(avformat SHARED IMPORTED )
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavformat.so)

add_library(avcodec SHARED IMPORTED )
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavcodec.so)

add_library(avfilter SHARED IMPORTED )
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavfilter.so)

add_library(avutil SHARED IMPORTED )
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavutil.so)

add_library(swresample SHARED IMPORTED )
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libswresample.so)

add_library(swscale SHARED IMPORTED )
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libswscale.so)

add_library(x264 STATIC IMPORTED )
set_target_properties(x264 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libx264.a)

add_library(faac STATIC IMPORTED )
set_target_properties(faac PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libfaac.a)

find_library( # Sets the name of the path variable.
        log-lib
        log )

target_link_libraries( # Specifies the target library.
        native-lib
        avformat
        avcodec
        avfilter
        avutil
        swresample
        swscale
        android
        ${log-lib}
        z
        OpenSLES
        x264
        rtmp
        faac
        log
        )
