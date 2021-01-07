package com.foxconn.lau.myapplication;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

public class  LauPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("lau-player");
    }

    private native void native_prepare(String dataSource);
    private native void native_start();
    private native void native_set_surface(Surface surface);

    private SurfaceHolder surfaceHolder;
    private String dataSource;

    public void setSurfaceView(SurfaceView surfaceView)  {
        if (null != this.surfaceHolder) {
            this.surfaceHolder.removeCallback(this);
        }
        this.surfaceHolder = surfaceView.getHolder();
        this.surfaceHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        native_set_surface(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }

    public void setDataSource(String absolutePath) {
        this.dataSource = dataSource;

    }

    public void prepare() {
        native_prepare(dataSource);
    }

    public void onPrepare(){
        if (null != onPrepareListener) {
            onPrepareListener.onPrepared();
        }
    }
    public void onProgress(int progress){
        if (null != onProgressListener) {
            onProgressListener.onProgress(progress);
        }
    }

    public void onError(int errorCode){
        if (null != onErrorListener) {
            onErrorListener.onError(errorCode);
        }
    }

    private OnErrorListener onErrorListener;
    private OnPrepareListener onPrepareListener;
    private OnProgressListener onProgressListener;

    public void start() {
        native_start();
    }

    public interface OnPrepareListener{
        void onPrepared();
    }

    public interface OnErrorListener{
        void onError(int error);
    }
    public interface OnProgressListener{
        void onProgress(int progress);
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void setOnPrepareListener(OnPrepareListener onPrepareListener) {
        this.onPrepareListener = onPrepareListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }
}
