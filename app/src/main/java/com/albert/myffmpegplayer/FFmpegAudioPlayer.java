package com.albert.myffmpegplayer;

public class FFmpegAudioPlayer {

    static {
        System.loadLibrary("myffmpegplayer");
    }

    public native void sound(String input, String output);
}
