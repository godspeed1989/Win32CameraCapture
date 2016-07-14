#include "CameraCapture.h"
#include "CaptureVideo.h"

CaptureVideo *capvid[MAX_CAMERA_INDEX];

#ifdef __cplusplus
extern "C" {
#endif

// 说明：初始化摄像驱动
// 参数：idx，设备编号
// 返回：无
void cm_init(UINT idx)
{
	capvid[idx] = new CaptureVideo();
}

// 说明：清理摄像驱动
// 参数：idx，设备编号
// 返回：无
void cm_exit(UINT idx)
{
	if (capvid)
	{
		delete capvid[idx];
		capvid[idx] = 0;
	}
}

// 说明：打印摄像驱动列表
// 参数：无
// 返回：摄像驱动个数
UINT cm_print_driver_list()
{
	// 枚举设备
	return CaptureVideo::EnumAllDevices();
}

// 说明：打开摄像设备
// 参数：idx，设备编号
// 返回：TRUE，打开成功；FALSE，打开失败
BOOL cm_open(UINT idx)
{
	capvid[idx]->OpenDevice(idx);
	if (capvid[idx]->m_bConnected == TRUE)
	{
		// 开启摄像头
		capvid[idx]->StartCapture();
		return TRUE;
	}
	return FALSE;
}

// 说明：关闭摄像设备
// 参数：idx，设备编号
// 返回：无
void cm_close(UINT idx)
{
	if (capvid[idx]->m_bConnected == TRUE)
	{	
		// 关闭摄像头
		capvid[idx]->StopCapture();
	}
	// 关闭设备[0]
	capvid[idx]->CloseDevice();
}

// 说明：从摄像设备截取一张图片到BMP文件
// 参数：idx，设备编号；file，文件名（如a.bmp）
// 返回：TRUE，截取成功；FALSE，截取失败
BOOL cm_grab_to_bmp(UINT idx, WCHAR *file)
{
	capvid[idx]->GrabOneFrame(file);
	return TRUE;
}

// 说明：开始录制视频
// 参数：idx，设备编号；file，文件名（如a.h264）
// 返回：TRUE，开始录制；FALSE，录制失败
BOOL cm_start_record(UINT idx, const char *file)
{
	capvid[idx]->StartGrabVideo(file);
	return 0;
}

// 说明：停止录制视频
// 参数：idx，设备编号
// 返回：无
void cm_stop_record(UINT idx)
{
	capvid[idx]->StopGrabVideo();
}

// 说明：获得摄像设备参数
// 参数：idx，设备编号；width，图像宽度；height，图像高度
// 返回：无
BOOL cm_get_param(UINT idx, int *width, int *height)
{
	if (capvid[idx]->m_bConnected == TRUE)
	{
		*width = (int)capvid[idx]->m_lWidth;
		*height = (int)capvid[idx]->m_lHeight;
	}
	return FALSE;
}

#ifdef __cplusplus
}
#endif
