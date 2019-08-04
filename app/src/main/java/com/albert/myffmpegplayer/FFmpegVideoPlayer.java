package com.albert.myffmpegplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class FFmpegVideoPlayer implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("myffmpegplayer");
    }

//    private Surface surface;
    private SurfaceHolder mHolder;

    public void start(String path) {
        native_start(path, mHolder.getSurface());
    }

    //绘制NDK path surfaceView
    public void setSurfaceView(SurfaceView surfaceView) {
        if (mHolder != null) {
            this.mHolder.removeCallback(this);
        }
        this.mHolder = surfaceView.getHolder();
        this.mHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        this.mHolder = holder;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public native void native_start(String path, Surface surface);
}
