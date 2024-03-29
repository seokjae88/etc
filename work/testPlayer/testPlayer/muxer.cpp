#include "muxer.h"


bool muxer::run() {
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
