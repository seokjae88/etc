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

class muxer {
private:

public:
	bool run();
};

