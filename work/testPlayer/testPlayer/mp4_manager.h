#pragma once

#include <iostream>
#include <map>

#pragma warning(disable : 4996)

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swscale.lib")
}

#define USE_H264BSF (0)
#define USE_AACBSF (0)

#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

#define SCALE_FLAGS SWS_BICUBIC

// a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream* st;
    AVCodecContext* enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame* frame;
    AVFrame* tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext* sws_ctx;
    struct SwrContext* swr_ctx;
} OutputStream;

class muxer {
private:
    std::string filename;
    std::string audioFile;
    std::string videoFile;
    AVFormatContext* outFmtCtx;
    AVFormatContext* inAudioFmtCtx;
    AVFormatContext* inVideoFmtCtx;

public:
    muxer(const std::string& filename);
    ~muxer();
    static AVFrame* alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
    static AVFrame* get_video_frame(OutputStream* ost);
    static bool open_video(AVFormatContext* oc, AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg);
    static bool write_video_frame(AVFormatContext* oc, OutputStream* ost);

    static AVFrame* alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, 
        int nb_samples);
    static bool open_audio(AVFormatContext* oc, AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg);
    static AVFrame* get_audio_frame(OutputStream* ost);
    static bool write_audio_frame(AVFormatContext* oc, OutputStream* ost);
    
    
    
    static void fill_yuv_image(AVFrame* pict, int frame_index, int width, int height);    
    static bool write_frame(AVFormatContext* fmt_ctx, AVCodecContext* c, AVStream* st, AVFrame* frame);
    static bool add_stream(OutputStream* ost, AVFormatContext* oc, AVCodec** codec, enum AVCodecID codec_id);
    static void close_stream(AVFormatContext* oc, OutputStream* ost);

    bool run();
};

