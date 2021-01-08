package com.foxconn.lau.myapplication;

import androidx.appcompat.app.AppCompatActivity;

import android.hardware.Camera;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "lau";

    private LivePusher livePusher;
    private LauPlayer lauPlayer;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        //直播
//        livePusher = new LivePusher(this,1920, 1080, 800_000, 10, Camera.CameraInfo.CAMERA_FACING_BACK);
////          设置摄像头预览的界面
//        livePusher.setPreviewDisplay(surfaceView.getHolder());

        //拉流播放
        lauPlayer = new LauPlayer();
        lauPlayer.setSurfaceView(surfaceView);
        File file = new File(Environment.getExternalStorageDirectory(), "Movies/Screen Recorder/20-10-03-09-03-09.mp4");
        Log.e(TAG, "onCreate: "+file.exists() );
        lauPlayer.setDataSource(file.getAbsolutePath());
        lauPlayer.setOnPrepareListener(() -> {
            Log.e(TAG, "onCreate: prepare" );
            lauPlayer.start();
        });
    }

    public void switchCamera(View view) {
    }

    public void startLive(View view) {
//        livePusher.startLive("rtmp://106.53.24.225:1935/myapp/");
//        livePusher.startLive("rtmp://192.168.1.153:1935/live/12345");
        lauPlayer.prepare();
    }
//    public void startLive(View view) {
//        livePusher.startLive("rtmp://192.168.1.153:1935/myapp/");
//    }

    public void stopLive(View view) {
    }

    public void play(View view) {
//        LauPlayer lauPlayer = new LauPlayer();
//        String input = new File(Environment.getExternalStorageDirectory(), "input.mp3").getAbsolutePath();
//        String output = new File(Environment.getExternalStorageDirectory(), "output.pcm").getAbsolutePath();
//        Log.e(TAG, "play: " + input);
//        lauPlayer.sound(input, output);

//        String oldApk = "/sdcard/zhy/origin/old.apk";
//        String patch = "/sdcard/zhy/origin/patch";
//       new Thread(){
//           @Override
//           public void run() {
//               super.run();
//               new BsPatch().BsPatcher(oldApk,patch,"/sdcard/zhy/origin/new.apk");
//           }
//       }.start();
    }

    public void prepare(View view) {
        lauPlayer.prepare();
    }
}