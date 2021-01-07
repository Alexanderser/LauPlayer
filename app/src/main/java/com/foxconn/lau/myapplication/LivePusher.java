package com.foxconn.lau.myapplication;

import android.app.Activity;
import android.view.SurfaceHolder;

import com.foxconn.lau.myapplication.meida.AudioChannel;
import com.foxconn.lau.myapplication.meida.VideoChannel;


public class LivePusher {
    private AudioChannel audioChannel;
    private VideoChannel videoChannel;
    static {
        System.loadLibrary("native-lib");
    }

    public LivePusher(Activity activity, int width, int height, int bitrate,
                      int fps, int cameraId) {
        native_init();
        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel(this);
    }
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }
    public void switchCamera() {
        videoChannel.switchCamera();
    }

    private native void native_init();

    public native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);

    public void startLive(String path) {
        native_start(path);
        videoChannel.startLive();
//        audioChannel.startLive();
    }

    private native void native_start(String path);

    public native void native_pushVideo(byte[] data);

    public native void native_pushAudio(byte[] bytes);

    public native void native_setAudioEncInfo(int i, int channels);

    public native int getInputSamples() ;
}
