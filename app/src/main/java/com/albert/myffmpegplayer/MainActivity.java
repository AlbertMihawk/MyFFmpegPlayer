package com.albert.myffmpegplayer;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends Activity implements View.OnClickListener {


    private static final String TAG = MainActivity.class.getCanonicalName();


    private SurfaceView surfaceView;
    private SeekBar seekBar;
    private Button button;
    private FFmpegVideoPlayer fFmpegVideoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //设置窗体全屏
//        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        //设置窗体始终点亮
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


        surfaceView = (SurfaceView) findViewById(R.id.surface_view);
//        seekBar = (SeekBar) findViewById(R.id.seek_bar);
        button = (Button) findViewById(R.id.btn_video_play);
        fFmpegVideoPlayer = new FFmpegVideoPlayer();
        fFmpegVideoPlayer.setSurfaceView(surfaceView);
        button.setOnClickListener(this);
        findViewById(R.id.btn_audio_play).setOnClickListener(this);
        TextView textView = (TextView) findViewById(R.id.text);
//        textView.setText(stringFromJNI());

        checkPermission();
    }

    public void checkPermission() {
        boolean isGranted = true;
        if (android.os.Build.VERSION.SDK_INT >= 23) {
            if (this.checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                //如果没有写sd卡权限
                isGranted = false;
            }
            if (this.checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                isGranted = false;
            }
            Log.i("cbs", "isGranted == " + isGranted);
            if (!isGranted) {
                this.requestPermissions(
                        new String[]{Manifest.permission.ACCESS_COARSE_LOCATION, Manifest.permission
                                .ACCESS_FINE_LOCATION,
                                Manifest.permission.READ_EXTERNAL_STORAGE,
                                Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        102);
            }
        }
    }

    public native String stringFromJNI();

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_video_play:
                File file = new File(Environment.getExternalStorageDirectory(), "input.mp4");
                if (file.exists()) {
                    fFmpegVideoPlayer.start(file.getAbsolutePath());
                } else {
                    Log.i(TAG, "文件不存在！");
                }
                break;
            case R.id.btn_audio_play:
                FFmpegAudioPlayer player = new FFmpegAudioPlayer();
                String input = new File(Environment.getExternalStorageDirectory(), "wumingzhibei.mp3").getAbsolutePath();
                String output = new File(Environment.getExternalStorageDirectory(), "output.pcm").getAbsolutePath();
                player.sound(input, output);
                break;
        }
    }
}
