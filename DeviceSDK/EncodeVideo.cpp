#include "EncodeVideo.h"
#include <iostream>

EncodeVideo::EncodeVideo()
{
	pCodecCtx = nullptr;
	pCodec = nullptr;
	nPts = 0;
	_packetFile = nullptr;
}

EncodeVideo::~EncodeVideo()
{
	if (pCodecCtx != nullptr)
	{
		avcodec_close(pCodecCtx);
	}
	if (_packetFile != nullptr)
	{
		fclose(_packetFile);
	}
}

bool EncodeVideo::InitEncoder(int width, int height, int frameRate, AVCodecID codecID, AVPixelFormat fmt)
{
	pCodec = avcodec_find_encoder(codecID);
	if (!pCodec)
	{
		printf("H264Encoder:Codec not found libx264 encoder!\n");
		return false;
	}
	// 编码器Context设置参数
	pCodecCtx = avcodec_alloc_context3(pCodec);
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = fmt;
	pCodecCtx->width = width;
	pCodecCtx->height = height;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = frameRate;
	pCodecCtx->bit_rate = 900000;
	//调整视频编码质量
	//极优_最接近原图(转换前后视频大小1:4)
	/*c->qmin = 10;
	c->qmax = 20;*/
	//优(转换前后视频大小1:2)
	/*c->qmin = 10;
	c->qmax = 25;*/
	//良(转换前后视频大小1:0.8)
	pCodecCtx->qmin = 10;
	pCodecCtx->qmax = 30;
	//一般(转换前后视频大小1:0.5)
	/*c->qmin = 10;
	c->qmax = 35;*/
	// 打开编码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("H264Encoder:Could not open H264 codec!\n");
		return false;
	}
	if (codecID == AV_CODEC_ID_H264) {
		_packetFile = fopen("encode.h264", "wb");
	}
	else if (codecID == AV_CODEC_ID_H265) {
		_packetFile = fopen("encode.h265", "wb");
	}
	
	return true;
}

bool EncodeVideo::GetPacketData(AVFrame* pFrame, AVPacket* pPacket)
{
	pFrame->pts = nPts++;
	int ret = avcodec_send_frame(pCodecCtx, pFrame);
	if (ret < 0)
	{
		printf("H264Encoder:Error sending a frame for encoding!\n");
		return false;
	}
	while (ret >= 0)
	{
		ret = avcodec_receive_packet(pCodecCtx, pPacket);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return false;
		}
		else if (ret < 0)
		{
			printf("H264Encoder:Error during encoding!\n");
			return false;
		}
		//std::cout << "encode size:" << pPacket->size << std::endl;
		if (_packetFile != nullptr) {
			fwrite((char*)pPacket->data, 1, pPacket->size, _packetFile);
		}
		return true;
	}
	return true;
}
