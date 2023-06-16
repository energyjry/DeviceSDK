#include "DeviceHk.h"

DeviceHk::DeviceHk()
{
	NET_DVR_Init();
	m_i64LoginId = -1;
	m_i64PreviewHandle = -1;
	m_iPlayHandle = -1;
	m_haveVideo = false;
	m_haveAudio = false;
	m_videoInfo = {};
	m_audioInfo = {};
	m_YuvFile = fopen("hk.yuv", "wb");
	m_PcmFile = fopen("hk.pcm", "wb");
}

DeviceHk::~DeviceHk()
{
	LogOut();
	if (m_iPlayHandle >= 0) {
		PlayM4_Stop(m_iPlayHandle);
		PlayM4_CloseStream(m_iPlayHandle);
		PlayM4_FreePort(m_iPlayHandle);
	}
	NET_DVR_Cleanup();
}

void DeviceHk::onPreview(DWORD dwDataType, BYTE* pBuffer, DWORD dwBufSize)
{
	switch (dwDataType) {
	case NET_DVR_SYSHEAD: { //系统头数据
		//printf("系统头\n");
		if (m_iPlayHandle >= 0)
		{
			break;  //该通道取流之前已经获取到句柄，后续接口不需要再调用
		}
		if (!PlayM4_GetPort(&m_iPlayHandle)) {  //获取播放库未使用的通道号
			printf("PlayM4_GetPort:%d\n", NET_DVR_GetLastError());
			break;
		}
		if (dwBufSize > 0) {
			if (!PlayM4_SetStreamOpenMode(m_iPlayHandle, STREAME_REALTIME)) { //设置实时流播放模式
				printf("PlayM4_SetStreamOpenMode:%d\n", NET_DVR_GetLastError());
				break;
			}

			if (!PlayM4_OpenStream(m_iPlayHandle, pBuffer, dwBufSize, 1024 * 1024)) {  //打开流接口
				printf("PlayM4_OpenStream:%d\n", NET_DVR_GetLastError());
				break;
			}


			PlayM4_SetDecCallBackMend(m_iPlayHandle, [](long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, void* nUser, void* nReserved2) {
				DeviceHk* self = (DeviceHk*)(nUser);
				if (nPort != self->m_iPlayHandle) {
					return;
				}
				self->onDecData(pBuf, nSize, pFrameInfo);
				}, this);


			if (!PlayM4_Play(m_iPlayHandle, nullptr)) //播放开始
			{
				break;
			}
			printf("设置解码器成功\n");

			//打开音频解码, 需要码流是复合流
			if (!PlayM4_PlaySoundShare(m_iPlayHandle)) {
				printf("PlayM4_PlaySound:%d\n", NET_DVR_GetLastError());
				break;
			}
		}
		break;
	}
	case NET_DVR_STREAMDATA: { //流数据（包括复合流或音视频分开的视频流数据）
		//printf("数据流\n");
		if (dwBufSize > 0 && m_iPlayHandle != -1) {
			if (!PlayM4_InputData(m_iPlayHandle, pBuffer, dwBufSize)) {
				printf("PlayM4_InputData:%d", NET_DVR_GetLastError());
				break;
			}
		}
		break;
	}
	case NET_DVR_AUDIOSTREAMDATA: { //音频数据
		printf("音频数据\n");
		break;
	}
	case NET_DVR_PRIVATE_DATA: { //私有数据,包括智能信息
		printf("私有数据\n");
		break;
	}
	default:
		break;
	}
}

void DeviceHk::onDecData(char* pBuf, long nSize, FRAME_INFO* pFrameInfo)
{
	switch (pFrameInfo->nType)
	{
	case T_YV12: {
		if (!m_haveVideo) {
			m_haveVideo = true;
			m_videoInfo.iWidth = pFrameInfo->nWidth;
			m_videoInfo.iHeight = pFrameInfo->nHeight;
			m_videoInfo.iFrameRate = pFrameInfo->nFrameRate;
			m_encodeVideo.InitEncoder(m_videoInfo.iWidth, m_videoInfo.iHeight, m_videoInfo.iFrameRate, AV_CODEC_ID_H264, AV_PIX_FMT_YUV420P);
		}

		// YV12 -->  YUV420
		// YV12:		Y亮度（行×列） + V（行×列/4) + U（行×列/4）
		// YUV420P:		Y亮度（行×列） + U（行×列/4) + V（行×列/4）
		int dwOffset_Y = pFrameInfo->nWidth * pFrameInfo->nHeight;
		char* yuv420p = new char[nSize];
		memcpy(yuv420p, pBuf, dwOffset_Y);
		memcpy(yuv420p + dwOffset_Y, pBuf + dwOffset_Y + dwOffset_Y / 4, dwOffset_Y / 4);
		memcpy(yuv420p + dwOffset_Y + dwOffset_Y / 4, pBuf + dwOffset_Y, dwOffset_Y / 4);
		printf("yuv size:%d\n", nSize);
		fwrite(yuv420p, 1, nSize, m_YuvFile);
		delete[] yuv420p;
		break;
	}
	case T_AUDIO16: {
		if (!m_haveAudio) {
			m_haveAudio = true;
			m_audioInfo.iChannel = pFrameInfo->nWidth;
			m_audioInfo.iSampleBit = pFrameInfo->nHeight;
			m_audioInfo.iSampleRate = pFrameInfo->nFrameRate;
		}
		printf("pcm size:%d\n", nSize);
		fwrite(pBuf, 1, nSize, m_PcmFile);
		break;
	}
	default:
		break;
	}
}

bool DeviceHk::Login(string ip, uint16_t port, string userName, string pwd)
{
	NET_DVR_USER_LOGIN_INFO loginInfo = { 0 };
	NET_DVR_DEVICEINFO_V40 loginResult = { 0 };

	strcpy(loginInfo.sDeviceAddress, ip.c_str());
	loginInfo.wPort = port;
	strcpy(loginInfo.sUserName, userName.c_str());
	strcpy(loginInfo.sPassword, pwd.c_str());
	loginInfo.bUseAsynLogin = false;
	NET_DVR_SetConnectTime(3 * 1000, 3);
	m_i64LoginId = NET_DVR_Login_V40(&loginInfo, &loginResult);
	return m_i64LoginId >= 0;
}

void DeviceHk::LogOut()
{
	if (m_i64PreviewHandle >= 0) {
		NET_DVR_StopRealPlay(m_i64PreviewHandle);
	}
	if (m_i64LoginId >= 0) {
		NET_DVR_Logout(m_i64LoginId);
	}
}

bool DeviceHk::StartPlay(int channel)
{
	NET_DVR_PREVIEWINFO previewInfo;
	previewInfo.lChannel = channel; //通道号
	previewInfo.dwStreamType = 0; // 码流类型，0-主码流，1-子码流，2-码流3，3-码流4 等以此类推
	previewInfo.dwLinkMode = 1; // 0：TCP方式,1：UDP方式,2：多播方式,3 - RTP方式，4-RTP/RTSP,5-RSTP/HTTP
	previewInfo.hPlayWnd = 0; //播放窗口的句柄,为NULL表示不播放图象
	previewInfo.byProtoType = 0; //应用层取流协议，0-私有协议，1-RTSP协议
	previewInfo.dwDisplayBufNum = 1; //播放库播放缓冲区最大缓冲帧数，范围1-50，置0时默认为1
	previewInfo.bBlocked = 0;
	m_i64PreviewHandle = NET_DVR_RealPlay_V40(m_i64LoginId,
		&previewInfo,
		[](LONG lPlayHandle, DWORD dwDataType, BYTE* pBuffer, DWORD dwBufSize, void* pUser) {
			DeviceHk* self = (DeviceHk*)(pUser);
			if (lPlayHandle != self->m_i64PreviewHandle) {
				return;
			}
			self->onPreview(dwDataType, pBuffer, dwBufSize);
		}, this);
	if (m_i64PreviewHandle < 0) {
		return false;
	}
	return true;
}
