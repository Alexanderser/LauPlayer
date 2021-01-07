package com.foxconn.lau.myapplication.meida;


import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import com.foxconn.lau.myapplication.LivePusher;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {
    private LivePusher mLivePusher;
    private AudioRecord audioRecord;
    private int channels = 2;
    private int channelConfig;
    private ExecutorService executor;
    private boolean isLiving;
    private int minBufferSize;
    private int inputSamples;

    public AudioChannel(LivePusher livePusher) {
        executor = Executors.newSingleThreadExecutor();
        mLivePusher = livePusher;
        if (channels == 2) {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_IN_MONO;
        }
        mLivePusher.native_setAudioEncInfo(44100,channels);
        Log.e("TAG", "AudioChannel: "+mLivePusher.getInputSamples() );
        inputSamples = mLivePusher.getInputSamples() * 2;

        minBufferSize = AudioRecord.getMinBufferSize(44100, channelConfig,
                AudioFormat.ENCODING_PCM_16BIT) * 2;
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 44100,
                channelConfig, AudioFormat.ENCODING_PCM_16BIT, Math.min(minBufferSize, inputSamples)
        );
    }

    public void startLive(){
        isLiving = true;
        executor.submit(new AudioTeask());
    }

    public void setChannels(int channels) {
        this.channels = channels;
    }

    private class AudioTeask implements Runnable {
        @Override
        public void run() {
            audioRecord.startRecording();
            byte[] bytes = new byte[inputSamples];
            while (isLiving) {
                audioRecord.read(bytes, 0, bytes.length);
                mLivePusher.native_pushAudio(bytes);
            }
        }
    }
}
