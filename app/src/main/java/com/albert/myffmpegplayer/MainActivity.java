package com.albert.myffmpegplayer;

import android.app.Activity;
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
    private FFmpegVideoPlayer player;

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
        button.setOnClickListener(this);

        button = (Button) findViewById(R.id.btn_audio_play);
        button.setOnClickListener(this);
        TextView textView = (TextView) findViewById(R.id.text);
//        textView.setText(stringFromJNI());


    }

    public native String stringFromJNI();

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_video_play:
                player = new FFmpegVideoPlayer();
                player.setSurfaceView(surfaceView);
                File file = new File("/sdcard/input.mp4");
                if (file.exists()) {
                    player.start(file.getAbsolutePath());
                } else {
                    Log.i(TAG, "文件不存在！");
                }
                break;
            case R.id.btn_audio_play:
                FFmpegAudioPlayer player = new FFmpegAudioPlayer();
                String input = new File("/sdcard/input.mp3").getAbsolutePath();
                String output = new File("/sdcard/output.pcm").getAbsolutePath();
                player.sound(input, output);
                break;
        }
    }
}
