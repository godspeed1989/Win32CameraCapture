#include "CameraCapture.h"

#ifdef __cplusplus
extern "C" {
#endif

static HWND m_hCapWnd[MAX_CAMERA_INDEX];
static CAPDRIVERCAPS m_CapDrvCap[MAX_CAMERA_INDEX];
static CAPSTATUS m_CapStatus[MAX_CAMERA_INDEX];
static CAPTUREPARMS m_Parms[MAX_CAMERA_INDEX];

UINT cm_print_driver_list()
{
	UINT wIndex, wCameraCnt;
	WCHAR szDriverName[80];
	WCHAR szDriverVersion[80];
	wCameraCnt = 0;
	for (wIndex = 0; wIndex < MAX_CAMERA_INDEX; wIndex++)
	{
		if( TRUE == capGetDriverDescription (wIndex,
					szDriverName, sizeof(szDriverName),
					szDriverVersion, sizeof(szDriverVersion)) )
		{
			wprintf(L"[%d] %s - %s\n", wIndex, szDriverName, szDriverVersion);
			wCameraCnt++;
		}
	}
	return wCameraCnt;
}

static BOOL cm_open_internal(UINT idx)
{
	// 创建窗口句柄
	m_hCapWnd[idx] = capCreateCaptureWindow(NULL, 0, 0, 0, 200, 200, NULL, 0);
	if (!m_hCapWnd)
	{
		printf("m_hCapWnd failed !!!\n");
		return FALSE;
	}
	// 连接第idx号驱动器
	else if (TRUE == capDriverConnect(m_hCapWnd[idx], idx))
	{
		capOverlay(m_hCapWnd[idx], FALSE);
		capPreview(m_hCapWnd[idx], FALSE);
		// 得到驱动器的性能
		memset(&m_CapDrvCap[idx], 0, sizeof(CAPDRIVERCAPS));
		if (FALSE == capDriverGetCaps(m_hCapWnd[idx], &m_CapDrvCap[idx], sizeof(CAPDRIVERCAPS)))
		{
			printf("capDriverGetCaps failed !!!\n");
		}
		else if (m_CapDrvCap[idx].fCaptureInitialized)
		{
			// 初始化成功
			return TRUE;
		}
		else
		{
			printf("fCaptureInitialized failed !!!\n");
		}
	}
	else
	{
		printf("connect failed !!!\n");
	}
	capCaptureStop(m_hCapWnd[idx]); // 停止捕捉
	capDriverDisconnect(m_hCapWnd[idx]);
	DestroyWindow(m_hCapWnd[idx]);
	return FALSE;
}

void cm_close(UINT idx)
{
	capCaptureStop(m_hCapWnd[idx]);
	capCaptureAbort(m_hCapWnd[idx]); // 停止捕获
	capDriverDisconnect(m_hCapWnd[idx]); // 断开捕捉器与驱动器的连接
	DestroyWindow(m_hCapWnd[idx]);
}

BOOL cm_open(UINT idx)
{
	BOOL ret = TRUE;
	int n = 0;
	do
	{
		printf("%d try to open %d\n", n, idx);
		Sleep(500);
		ret = cm_open_internal(idx);
		n++;
	} while(FALSE == ret && n < 5);
	if (FALSE == ret)
	{
		printf("open failed !!!\n");
	}
	return ret;
}

BOOL cm_get_param(UINT idx, int *width, int *height, int *frame_rate)
{
	BOOL ret = TRUE;
	// 得到设置参数
	ret = capCaptureGetSetup(m_hCapWnd[idx], &m_Parms[idx], sizeof(CAPTUREPARMS));
	if (FALSE == ret)
	{
		printf("capCaptureGetSetup failed !!!\n");
		return ret;
	}
	// 得到驱动器状态
	ret = capGetStatus(m_hCapWnd[idx], &m_CapStatus[idx], sizeof(CAPSTATUS));
	if (FALSE == ret)
	{
		printf("capGetStatus failed !!!\n");
		return ret;
	}
	*width = (int)m_CapStatus[idx].uiImageHeight;
	*height = (int)m_CapStatus[idx].uiImageWidth;
	*frame_rate = (int)1000000 / (int)m_Parms[idx].dwRequestMicroSecPerFrame;

	return ret;
}

BOOL cm_grab_to_bmp(UINT idx, WCHAR *file)
{
	BOOL ret = TRUE;
	// 截获当前图像
	ret = capGrabFrame(m_hCapWnd[idx]);
	if (FALSE == ret)
	{
		printf("capGrabFrame failed !!!\n");
		return ret;
	}
	// 保存到文件
	ret = capFileSaveDIB(m_hCapWnd[idx], file);
	if (FALSE == ret)
	{
		printf("capFileSaveDIB failed !!!\n");
		return ret;
	}
	return ret;
}

BOOL cm_grab_to_clipboard(UINT idx)
{
	BOOL ret = TRUE;
	// 截获当前图像
	ret = capGrabFrame(m_hCapWnd[idx]);
	if (FALSE == ret)
	{
		printf("capGrabFrame failed !!!\n");
		return ret;
	}
	// 保存到剪贴板
	ret = capEditCopy(m_hCapWnd[idx]);
	if (FALSE == ret)
	{
		printf("capEditCopy failed !!!\n");
		return ret;
	}
	return ret;
}

//LRESULT CALLBACK StreamCallbackProc(HWND hCapWnd, LPVIDEOHDR lphdr)
//{
//	return (LRESULT)TRUE;
//}

BOOL cm_start_record(UINT idx, WCHAR *file)
{
	BOOL ret = TRUE;
	// 视频流回调函数
	//ret = capSetCallbackOnVideoStream(m_hCapWnd[idx], StreamCallbackProc);
	//if (FALSE == ret)
	//{
	//	printf("capSetCallbackOnVideoStream failed !!!\n");
	//	return ret;
	//}
	// 设置捕获参数
	capCaptureGetSetup(m_hCapWnd[idx], &m_Parms[idx], sizeof(CAPTUREPARMS));
	m_Parms[idx].fYield = TRUE; // 后台运行
	m_Parms[idx].fCaptureAudio = FALSE; // 无声音
	m_Parms[idx].wTimeLimit = FALSE; // 无时间限制
	capCaptureSetSetup(m_hCapWnd[idx], &m_Parms[idx], sizeof(CAPTUREPARMS));
	// 设置捕获视频的文件
	capFileSetCaptureFile(m_hCapWnd[idx], file);
	// 开始录像线程
	ret = capCaptureSequence(m_hCapWnd[idx]);
	if (FALSE == ret)
	{
		printf("capCaptureSequence failed !!!\n");
		return ret;
	}
	return ret;
}

void cm_stop_record(UINT idx)
{
	//capSetCallbackOnVideoStream(m_hCapWnd[idx], NULL);
	capCaptureStop(m_hCapWnd[idx]);
	capCaptureAbort(m_hCapWnd[idx]);
}

#ifdef __cplusplus
}
#endif
