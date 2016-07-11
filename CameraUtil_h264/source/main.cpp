#include "CaptureVideo.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	CaptureVideo capvid;

	// 枚举设备
	HRESULT r = capvid.EnumAllDevices();
	for (int i = 0; i < capvid.m_nCaptureDeviceNumber; i++)
	{
		wprintf(L"[%d] %s\n", i, capvid.m_pCapDeviceName[i]);
	}
	// 打开设备[0]
	capvid.OpenDevice(0);
	if (capvid.m_bConnected == TRUE)
	{
		// 开启摄像头
		capvid.StartCapture();
		// 捕获一张图片
		Sleep(500);
		capvid.GrabOneFrame(L"aaa.bmp");
		// 捕获视频文件
		capvid.StartGrabVideo("vvv.h264");
		Sleep(10000);
		capvid.StopGrabVideo();
		// 关闭摄像头
		capvid.StopCapture();
	}
	// 关闭设备[0]
	capvid.CloseDevice();

	system("pause");
	return 0;
}
