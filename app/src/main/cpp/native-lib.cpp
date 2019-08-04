#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <zconf.h>
#include <iostream>
#include <android/log.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);

#define MAX_AUDIO_FRAME_SIZE 48000 * 4


extern "C" {
//封装格式
#include <libavformat/avformat.h>
//解码
#include <libavcodec/avcodec.h>
//缩放
#include <libswscale/swscale.h>
//重采样
#include<libswresample/swresample.h>
//
#include <libavutil/imgutils.h>
};

extern "C" JNIEXPORT jstring JNICALL
Java_com_albert_myffmpegplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    std::string mytext = av_version_info();
    return env->NewStringUTF(mytext.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_albert_myffmpegplayer_FFmpegVideoPlayer_native_1start(JNIEnv *env, jobject instance,
                                                               jstring path_, jobject surface) {


    const char *path = env->GetStringUTFChars(path_, 0);

    //FFmpeg 视频绘制  音频绘制 音视频同步

    //初始化网络模块
    avformat_network_init();

    //格式 总上下文
    AVFormatContext *avFmtCtx = avformat_alloc_context();

    if (avFmtCtx == NULL) {
        return;
    }
    std::cout << "avFmtCtx is not NULL" << std::endl;
    //创建字典
    AVDictionary *opts = NULL;
    av_dict_set(&opts, "timeout", "3000000", 0);
    //强制指定AVFormatContext中AVInputFormat的。这个参数一般情况下可以设置为NULL，这样FFmpeg可以自动检测AVInputFormat。
    //输入文件的封装格式
    //av_find_input_format("avi")
    //ret为零 表示成功
    int ret;
    avformat_open_input(&avFmtCtx, path, NULL, &opts);

    /*Retrieve the stream information, APP CRASH RIGHT HERE*/
    if(avformat_find_stream_info(avFmtCtx, nullptr)<0){
        LOGI("Couldn't retrieve stream information");
        avformat_free_context(avFmtCtx);
        return ; // Couldn’t find stream information
    }

    int video_stream_idx = -1;
    for (int i = 0; i < avFmtCtx->nb_streams; ++i) {
        if (avFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    //视频流索引video_stream_idx
    AVCodecParameters *avCodecPar = avFmtCtx->streams[video_stream_idx]->codecpar;

    //解码器 H264 java 策略模式 key id
    //code_id与解码器实例，一一对应的，通过不同的id获得解码器
    AVCodec *avCodec = avcodec_find_decoder(avCodecPar->codec_id);

    //解码器上下文 ffmpeg 1.0-4.0
    AVCodecContext *avCodecCtx = avcodec_alloc_context3(avCodec);
    //将解码器参数拷贝到解码器上下文
    avcodec_parameters_to_context(avCodecCtx, avCodecPar);
    //打开解码器
    avcodec_open2(avCodecCtx, avCodec, NULL);
    //解码 yuv数据
    //AVPacket 以前malloc() new AVPacket
    AVPacket *avPkt = av_packet_alloc();
    //avCodecCtx->pix_fmt
    /**
    * SWS 转化器
    * 缩小：
    * 重视速度：fast_bilinear, point
    * 重视质量：cubic, spline, lanczos
    * 重视锐度：gauss, bilinear
    */
    //swsContext转换上下文
    SwsContext *swsCtx = sws_getContext(avCodecCtx->width, avCodecCtx->height,
                                        avCodecCtx->pix_fmt,
                                        avCodecCtx->width, avCodecCtx->height, AV_PIX_FMT_RGBA,
                                        SWS_BILINEAR, 0, 0,
                                        0);
    //创建AVNativeWindow
    ANativeWindow *aNativeWindow = ANativeWindow_fromSurface(env, surface);
    //视频缓冲区
    ANativeWindow_Buffer aNativeWinBuf;
    //设置缓冲区大小
    int frameCount = 0;
    ANativeWindow_setBuffersGeometry(aNativeWindow, avCodecCtx->width, avCodecCtx->height,
                                     WINDOW_FORMAT_RGBA_8888);

    //从视频流中读取数据包
    while (av_read_frame(avFmtCtx, avPkt) >= 0) {
        //编解码器获取数据包
        avcodec_send_packet(avCodecCtx, avPkt);
        //ffmpeg malloc()
        AVFrame *avFrame = av_frame_alloc();
        //从编解码器中取出帧画面
        ret = avcodec_receive_frame(avCodecCtx, avFrame);
        if (ret == AVERROR(EAGAIN)) {
            //需要更多数据
            continue;
        } else if (ret < 0) {
            break;
        }

        //接收容器
        uint8_t *dst_data[0];
        //每一行的首地址
        int dst_linesize[0];
        //创建规定格式的图片容器对象
        av_image_alloc(dst_data, dst_linesize, avCodecCtx->width, avCodecCtx->height,
                       AV_PIX_FMT_RGBA, 1);

        if (avPkt->stream_index == video_stream_idx) {
            //非零 正在解码
            if (ret == 0) {
                //绘制之前 篇日志一些信息 比如宽高 格式
                //锁住缓冲区
                ANativeWindow_lock(aNativeWindow, &aNativeWinBuf, NULL);

                //转化数据 slice切片部分 stride步幅
                sws_scale(swsCtx, reinterpret_cast<const uint8_t *const *>(avFrame->data),
                          avFrame->linesize, 0, avFrame->height, dst_data,
                          dst_linesize);
                //rgb_frame是有画面数据
//                uint8_t *dst = (uint8_t *) aNativeWinBuf.bits;
                //拿到一行有多少个字节RGBA
                int destStride = aNativeWinBuf.stride * 4;
                //输入源（RGB）的
                uint8_t *src_data = dst_data[0];
                int src_linesize = dst_linesize[0];
                //渲染
                uint8_t *firstWindow = static_cast<uint8_t *>(aNativeWinBuf.bits);
                for (int i = 0; i < aNativeWinBuf.height; ++i) {
                    //通过内存拷贝，进行渲染
                    memcpy(firstWindow + i * destStride, src_data + i * src_linesize, destStride);
                }

                ANativeWindow_unlockAndPost(aNativeWindow);
                usleep(1000 * 16);
                av_frame_free(&avFrame);

                //AvFrame yuv ---> image 变现为dst_data数组 ---> 渲染surfaceView
            }
        }
    }
//    回收资源和关闭上下文
    ANativeWindow_release(aNativeWindow);
    avcodec_close(avCodecCtx);
    avcodec_free_context(&avCodecCtx);
    avformat_free_context(avFmtCtx);
    env->ReleaseStringUTFChars(path_, path);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_albert_myffmpegplayer_FFmpegAudioPlayer_sound(JNIEnv *env, jobject instance,
                                                       jstring input_, jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);

    avformat_network_init();
    //总上下文
    AVFormatContext *avFmtCtx = avformat_alloc_context();

    //打开音频文件
    if(avformat_open_input(&avFmtCtx,input,NULL,NULL) != 0){
        LOGI("%s","无法打开音频文件");
        return;
    }

    //获取输入文件信息
    if(avformat_find_stream_info(avFmtCtx,NULL) < 0){
        LOGI("%s","无法获取输入文件信息");
        return;
    }
    //视频时长（单位：微秒us，转换为秒需要除以1000000）
    int audio_stream_idx=-1;
    for (int i = 0; i < avFmtCtx->nb_streams; ++i) {
        if (avFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx=i;
            break;
        }
    }

    AVCodecParameters *codecpar = avFmtCtx->streams[audio_stream_idx]->codecpar;
    //找到解码器
    AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);
    //创建上下文
    AVCodecContext *codecContext = avcodec_alloc_context3(dec);


    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}