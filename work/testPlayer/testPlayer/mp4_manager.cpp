
#include "mp4_manager.h"

muxer::muxer(const std::string& file) {
    this->filename = file;
}
/* Prepare a dummy image. */
void muxer::fill_yuv_image(AVFrame* pict, int frame_index, int width, int height)
{
    int x, y, i;
    i = frame_index;
    /* Y */
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;

    /* Cb and Cr */
    for (y = 0; y < height / 2; y++) {
        for (x = 0; x < width / 2; x++) {
            pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
            pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
        }
    }
}

AVFrame* muxer::get_video_frame(OutputStream* ost)
{
    AVCodecContext* c = ost->enc;

    /* check if we want to generate more frames */
    AVRational rational;
    rational.den = 1;
    rational.num = 1;
    if (av_compare_ts(ost->next_pts, c->time_base, STREAM_DURATION, rational) > 0) {
        return nullptr;
    }

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally; make sure we do not overwrite it here */
    if (av_frame_make_writable(ost->frame) < 0) {
        return nullptr;
    }

    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        /* as we only generate a YUV420P picture, we must convert it
         * to the codec pixel format if needed */
        if (!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(c->width, c->height, AV_PIX_FMT_YUV420P, c->width, c->height, c->pix_fmt,
                SCALE_FLAGS, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                printf("Could not initialize the conversion context\n");
                return nullptr;
            }
        }
        muxer::fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);
        sws_scale(ost->sws_ctx, (const uint8_t* const*)ost->tmp_frame->data, ost->tmp_frame->linesize, 0, 
            c->height, ost->frame->data, ost->frame->linesize);
    }
    else {
        muxer::fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);
    }

    ost->frame->pts = ost->next_pts++;

    return ost->frame;
}

bool muxer::write_frame(AVFormatContext* fmt_ctx, AVCodecContext* c, AVStream* st, AVFrame* frame)
{
    int ret;

    // send the frame to the encoder
    ret = avcodec_send_frame(c, frame);
    if (ret < 0) {
        printf("Error sending a frame to the encoder: %d\n", ret);
        return false;
    }

    while (ret >= 0) {
        AVPacket pkt = { 0 };

        ret = avcodec_receive_packet(c, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            printf("Error encoding a frame: %d\n", ret);
            return false;
        }

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(&pkt, c->time_base, st->time_base);
        pkt.stream_index = st->index;

        /* Write the compressed frame to the media file. */
        //log_packet(fmt_ctx, &pkt);
        ret = av_interleaved_write_frame(fmt_ctx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0) {
            printf("Error while writing output packet: %d\n", ret);
            return false;
        }
    }

    return ret == AVERROR_EOF ? true : false;
}


AVFrame* muxer::alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, 
    int nb_samples)
{
    AVFrame* frame = av_frame_alloc();
    int ret;

    if (!frame) {
        printf("Error allocating an audio frame\n");
        return nullptr;
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            printf("Error allocating an audio buffer\n");
            return nullptr;
        }
    }

    return frame;
}
bool muxer::open_audio(AVFormatContext* oc, AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg) {
    AVCodecContext* c;
    int nb_samples;
    int ret;
    AVDictionary* opt = NULL;

    c = ost->enc;

    /* open it */
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        printf("Could not open audio codec: %d\n", ret);
        return false;
    }

    /* init signal generator */
    ost->t = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;

    ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);
    if (!ost->frame) {
        return false;
    }
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout, c->sample_rate, nb_samples);

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        printf("Could not copy the stream parameters\n");
        return false;
    }

    /* create resampler context */
    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx) {
        printf("Could not allocate resampler context\n");
        return false;
    }

    /* set options */
    av_opt_set_int(ost->swr_ctx, "in_channel_count", c->channels, 0);
    av_opt_set_int(ost->swr_ctx, "in_sample_rate", c->sample_rate, 0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(ost->swr_ctx, "out_channel_count", c->channels, 0);
    av_opt_set_int(ost->swr_ctx, "out_sample_rate", c->sample_rate, 0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);

    /* initialize the resampling context */
    if ((ret = swr_init(ost->swr_ctx)) < 0) {
        printf("Failed to initialize the resampling context\n");
        return false;
    }
}

AVFrame* muxer::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame* picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return nullptr;

    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 0);
    if (ret < 0) {
        printf("Could not allocate frame data.\n");
        return nullptr;
    }

    return picture;
}

bool muxer::open_video(AVFormatContext* oc, AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg)
{
    int ret;
    AVCodecContext* c = ost->enc;
    AVDictionary* opt = nullptr;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        printf("Could not open video codec: %d\n", ret);
        return false;
    }

    /* allocate and init a re-usable frame */
    ost->frame = muxer::alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        printf("Could not allocate video frame\n");
        return false;
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = muxer::alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            printf("Could not allocate temporary picture\n");
            return false;
        }
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        printf("Could not copy the stream parameters\n");
        return false;
    }

    return true;
}
bool muxer::write_video_frame(AVFormatContext* oc, OutputStream* ost)
{
    return muxer::write_frame(oc, ost->enc, ost->st, get_video_frame(ost));
}

/* Add an output stream. */
bool muxer::add_stream(OutputStream* ost, AVFormatContext* oc, AVCodec** codec, enum AVCodecID codec_id) {
    AVCodecContext* c;
    int i;

    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        printf("Could not find encoder for '%s'\n", avcodec_get_name(codec_id));
        return false;
    }

    ost->st = avformat_new_stream(oc, nullptr);
    if (!ost->st) {
        printf("Could not allocate stream\n");
        return false;
    }
    ost->st->id = oc->nb_streams - 1;
    c = avcodec_alloc_context3(*codec);
    if (!c) {
        printf("Could not alloc an encoding context\n");
        return false;
    }
    ost->enc = c;

    switch ((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt = (*codec)->sample_fmts ?
            (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate = 64000;
        c->sample_rate = 44100;
        if ((*codec)->supported_samplerates) {
            c->sample_rate = (*codec)->supported_samplerates[0];
            for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                if ((*codec)->supported_samplerates[i] == 44100)
                    c->sample_rate = 44100;
            }
        }
        c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        if ((*codec)->channel_layouts) {
            c->channel_layout = (*codec)->channel_layouts[0];
            for (i = 0; (*codec)->channel_layouts[i]; i++) {
                if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
        ost->st->time_base = (AVRational){ 1, c->sample_rate };
        break;

    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;

        c->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        c->width = 352;
        c->height = 288;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        ost->st->time_base.num = 1;
        ost->st->time_base.den = STREAM_FRAME_RATE;
        c->time_base = ost->st->time_base;

        c->gop_size = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt = STREAM_PIX_FMT;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B-frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
        break;

    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER) {
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    return true;
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
AVFrame* muxer::get_audio_frame(OutputStream* ost)
{
    AVFrame* frame = ost->tmp_frame;
    int j, i, v;
    int16_t* q = (int16_t*)frame->data[0];

    /* check if we want to generate more frames */
    if (av_compare_ts(ost->next_pts, ost->enc->time_base,
        STREAM_DURATION, (AVRational) { 1, 1 }) > 0)
        return nullptr;

    for (j = 0; j < frame->nb_samples; j++) {
        v = (int)(sin(ost->t) * 10000);
        for (i = 0; i < ost->enc->channels; i++)
            *q++ = v;
        ost->t += ost->tincr;
        ost->tincr += ost->tincr2;
    }

    frame->pts = ost->next_pts;
    ost->next_pts += frame->nb_samples;

    return frame;
}
/*
 * encode one audio frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
bool muxer::write_audio_frame(AVFormatContext* oc, OutputStream* ost)
{
    AVCodecContext* c;
    AVFrame* frame;
    int ret;
    int dst_nb_samples;

    c = ost->enc;

    frame = muxer::get_audio_frame(ost);

    if (frame) {
        /* convert samples from native format to destination codec format, using the resampler */
        /* compute destination number of samples */
        dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
            c->sample_rate, c->sample_rate, AV_ROUND_UP);
        //av_assert0(dst_nb_samples == frame->nb_samples);

        /* when we pass a frame to the encoder, it may keep a reference to it
         * internally;
         * make sure we do not overwrite it here
         */
        ret = av_frame_make_writable(ost->frame);
        if (ret < 0) {
            return false;
        }

        /* convert to destination format */
        ret = swr_convert(ost->swr_ctx,
            ost->frame->data, dst_nb_samples,
            (const uint8_t**)frame->data, frame->nb_samples);
        if (ret < 0) {
            printf("Error while converting\n");
            return false;
        }
        frame = ost->frame;

        AVRational rational;
        rational.num = 1;
        rational.den = c->sample_rate;
        frame->pts = av_rescale_q(ost->samples_count, rational, c->time_base);
        ost->samples_count += dst_nb_samples;
    }

    return muxer::write_frame(oc, c, ost->st, frame);
}

void muxer::close_stream(AVFormatContext* oc, OutputStream* ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}

bool muxer::run() {
    /* allocate the output media context */
    avformat_alloc_output_context2(&this->outFmtCtx, NULL, NULL, this->filename.c_str());
    if (!this->outFmtCtx) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&this->outFmtCtx, NULL, "mpeg", this->filename.c_str());
    }
    if (!this->outFmtCtx) {
        return false;
    }
    AVOutputFormat* fmt = this->outFmtCtx->oformat;

    if (avformat_open_input(&this->inVideoFmtCtx, this->videoFile.c_str(), 0, 0) < 0) {
        printf("Could not open input file.\n");
        return false;
    }
    if (avformat_find_stream_info(this->inVideoFmtCtx, 0) < 0) {
        printf("Failed to retrieve input stream information\n");
        return false;
    }

    if (avformat_open_input(&this->inAudioFmtCtx, this-> audioFile.c_str(), 0, 0) < 0) {
        printf("Could not open input file.\n");
        return false;
    }
    if (avformat_find_stream_info(this->inAudioFmtCtx, 0) < 0) {
        printf("Failed to retrieve input stream information\n");
        return false;
    }
    av_dump_format(this->inVideoFmtCtx, 0, this->videoFile.c_str(), 0);
    av_dump_format(this->inAudioFmtCtx, 0, this->audioFile.c_str(), 0);

    OutputStream video_st = { 0 }, audio_st = { 0 };
    AVCodec* audio_codec, * video_codec;
    int have_video = 0, have_audio = 0;
    int encode_video = 0, encode_audio = 0;
    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        if (muxer::add_stream(&video_st, this->outFmtCtx, &video_codec, fmt->video_codec) == false) {
            return false;
        }        
        have_video = 1;
        encode_video = 1;
    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE) {
        if (muxer::add_stream(&audio_st, this->outFmtCtx, &audio_codec, fmt->audio_codec) == false) {
            return false;
        }
        have_audio = 1;
        encode_audio = 1;
    }


    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    AVDictionary* opt = nullptr;
    av_dict_set(&opt, "", "", 0);
    if (have_video) {
        muxer::open_video(this->outFmtCtx, video_codec, &video_st, opt);
    }
    if (have_audio) {
        muxer::open_audio(this->outFmtCtx, audio_codec, &audio_st, opt);
    }
        
    av_dump_format(this->outFmtCtx, 0, this->filename.c_str(), 1);

    int ret = 0;
    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&this->outFmtCtx->pb, this->filename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("Could not open '%s': %d\n", filename.c_str(), ret);
            return false;
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(this->outFmtCtx, &opt);
    if (ret < 0) {
        printf("Error occurred when opening output file: %d\n", ret);
        return false;
    }

    while (encode_video || encode_audio) {
        /* select the stream to encode */
        if (encode_video &&
            (!encode_audio || av_compare_ts(video_st.next_pts, video_st.enc->time_base,
                audio_st.next_pts, audio_st.enc->time_base) <= 0)) {
            encode_video = !muxer::write_video_frame(this->outFmtCtx, &video_st);
        }
        else {
            encode_audio = !write_audio_frame(this->outFmtCtx, &audio_st);
        }
    }

    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    av_write_trailer(this->outFmtCtx);

    /* Close each codec. */
    if (have_video)
        muxer::close_stream(this->outFmtCtx, &video_st);
    if (have_audio)
        muxer::close_stream(this->outFmtCtx, &audio_st);

    if (!(fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_closep(&this->outFmtCtx->pb);

    /* free the stream */
    avformat_free_context(this->outFmtCtx);

    return true;
}


bool mp4Manager::run() {
    AVOutputFormat* ofmt = NULL;
    AVFormatContext* ifmt_ctx_v = NULL, * ifmt_ctx_a = NULL, * ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;

    std::string in_filename_v = "D:/jsj/2. 개발업무/test/file3_track1.h264";
    std::string in_filename_a = "D:/jsj/2. 개발업무/test/file3_track2.aac";
    std::string out_filename = "D:/jsj/2. 개발업무/test/test_out.mp4";

    av_register_all();

    // Input
    if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v.c_str(), 0, 0)) < 0) {
        printf("Could not open input file.\n");
        return false;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
        printf("Failed to retrieve input stream information\n");
        return false;
    }

    if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a.c_str(), 0, 0)) < 0) {
        printf("Could not open input file.\n");
        return false;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
        printf("Failed to retrieve input stream information\n");
        return false;
    }
    printf("Input Information=====================\n");
    av_dump_format(ifmt_ctx_v, 0, in_filename_v.c_str(), 0);
    av_dump_format(ifmt_ctx_a, 0, in_filename_a.c_str(), 0);
    printf("======================================\n");

    // Output
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename.c_str());
    if (!ofmt_ctx) {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        return false;
    }

    ofmt = ofmt_ctx->oformat;
    int videoindex_v = -1, videoindex_out = -1;
    for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
        // Create output AVStream according to input AVStream
        if (ifmt_ctx_v->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex_v = i;
            AVStream* in_stream = ifmt_ctx_v->streams[i];
            AVStream* out_stream = avformat_new_stream(ofmt_ctx, (AVCodec*)in_stream->codec->codec);
            if (!out_stream) {
                printf("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                return false;
            }
            videoindex_out = out_stream->index;
            // Copy the settings of AVCodecContext
            if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                printf("Failed to copy context from input to output stream codec context\n");
                return false;
            }
            out_stream->codec->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            break;
        }
    }

    int audioindex_a = -1, audioindex_out = -1;
    for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
        // Create output AVStream according to input AVStream
        if (ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioindex_a = i;
            AVStream* in_stream = ifmt_ctx_a->streams[i];
            AVStream* out_stream = avformat_new_stream(ofmt_ctx, (AVCodec*)in_stream->codec->codec);
            if (!out_stream) {
                printf("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                return false;
            }
            audioindex_out = out_stream->index;
            // Copy the settings of AVCodecContext
            if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                printf("Failed to copy context from input to output stream codec context\n");
                return false;
            }
            out_stream->codec->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            break;
        }
    }

    printf("Output Information====================\n");
    av_dump_format(ofmt_ctx, 0, out_filename.c_str(), 1);
    printf("======================================\n");

    // Open output file
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx->pb, out_filename.c_str(), AVIO_FLAG_WRITE) < 0) {
            printf("Could not open output file '%s'\n", out_filename.c_str());
            return false;
        }
    }
    // Write file header
    if (avformat_write_header(ofmt_ctx, NULL) < 0) {
        printf("Error occurred when opening output file\n");
        return false;
    }
    int frame_index = 0;
    int64_t cur_pts_v = 0, cur_pts_a = 0;

    //FIX
#if USE_H264BSF
    AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif
#if USE_AACBSF
    AVBitStreamFilterContext* aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
#endif

    while (1) {
        AVFormatContext* ifmt_ctx;
        int stream_index = 0;
        AVStream* in_stream, * out_stream;

        // Get an AVPacket
        if (av_compare_ts(cur_pts_v, ifmt_ctx_v->streams[videoindex_v]->time_base, cur_pts_a, ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0) {
            ifmt_ctx = ifmt_ctx_v;
            stream_index = videoindex_out;

            if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
                do {
                    if (pkt.stream_index == videoindex_v) {
                        cur_pts_v = pkt.pts;
                        break;
                    }
                } while (av_read_frame(ifmt_ctx, &pkt) >= 0);
            }
            else {
                break;
            }
        }
        else {
            ifmt_ctx = ifmt_ctx_a;
            stream_index = audioindex_out;
            if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
                do {
                    if (pkt.stream_index == audioindex_a) {
                        cur_pts_a = pkt.pts;
                        break;
                    }
                } while (av_read_frame(ifmt_ctx, &pkt) >= 0);
            }
            else {
                break;
            }

        }

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[stream_index];
        //FIX
#if USE_H264BSF
        av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
        av_bitstream_filter_filter(aacbsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
        //FIX：No PTS (Example: Raw H.264)
        //Simple Write PTS
        if (pkt.pts == AV_NOPTS_VALUE) {
            //Write PTS
            AVRational time_base1 = in_stream->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
            //Parameters
            pkt.pts = (double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
            pkt.dts = pkt.pts;
            pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
            frame_index++;
        }
        /* copy packet */
        // Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = stream_index;

        //printf("Write 1 Packet. size:%5d\tpts:%8d\n", pkt.size, pkt.pts);
        // Write
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            printf("Error muxing packet\n");
            break;
        }
        av_free_packet(&pkt);

    }
    // Write file trailer
    av_write_trailer(ofmt_ctx);

#if USE_H264BSF
    av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
    av_bitstream_filter_close(aacbsfc);
#endif

end:
    avformat_close_input(&ifmt_ctx_v);
    avformat_close_input(&ifmt_ctx_a);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        printf("Error occurred.\n");
        return false;
    }
    return true;
}