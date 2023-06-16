#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include "HCNetSDK.h"
#include "PlayM4.h"
#include <Windows.h>
#include "EncodeVideo.h"

using namespace std;

struct VideoInfo {
	int iWidth;
	int iHeight;
	float iFrameRate;
};

struct AudioInfo {
	int iChannel;
	int iSampleBit;
	int iSampleRate;
};

class DeviceHk
{
public:
	DeviceHk();
	~DeviceHk();
private:
	int64_t m_i64LoginId;
	int64_t m_i64PreviewHandle;
	LONG m_iPlayHandle;
	bool m_haveVideo;
	bool m_haveAudio;
	VideoInfo m_videoInfo;
	AudioInfo m_audioInfo;
	EncodeVideo m_encodeVideo;

	FILE* m_YuvFile;
	FILE* m_PcmFile;
private:
	void onPreview(DWORD dwDataType, BYTE* pBuffer, DWORD dwBufSize);
	void onDecData(char* pBuf, long nSize, FRAME_INFO* pFrameInfo);

public:
	bool Login(string ip, uint16_t port, string userName, string pwd);
	void LogOut();
	bool StartPlay(int channel);
};

