#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/pixdesc.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
#include <libavdevice/avdevice.h>
#include "libavutil/time.h"
};

class EncodeVideo
{
public:
	EncodeVideo();
	~EncodeVideo();
private:
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;
	int64_t nPts;
	FILE* _packetFile;
public:
	bool InitEncoder(int width, int height, int frameRate, AVCodecID codecID, AVPixelFormat fmt);
	bool GetPacketData(AVFrame* pFrame, AVPacket* pPacket);
};

