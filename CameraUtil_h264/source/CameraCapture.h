#ifndef _CAMERACAPTURE_H_
#define _CAMERACAPTURE_H_
#include <windows.h>

#ifdef DLL_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CAMERA_INDEX 10

// 说明：初始化摄像驱动
// 参数：无
// 返回：无
DLLEXPORT void cm_init();

// 说明：清理摄像驱动
// 参数：无
// 返回：无
DLLEXPORT void cm_exit();

// 说明：打印摄像驱动列表
// 参数：无
// 返回：摄像驱动个数
DLLEXPORT UINT cm_print_driver_list();

// 说明：打开摄像设备
// 参数：设备编号
// 返回：TRUE，打开成功；FALSE，打开失败
DLLEXPORT BOOL cm_open(UINT idx);

// 说明：关闭摄像设备
// 参数：idx，设备编号
// 返回：无
DLLEXPORT void cm_close(UINT idx);

// 说明：从摄像设备截取一张图片到BMP文件
// 参数：idx，设备编号；file，文件名（如a.bmp）
// 返回：TRUE，截取成功；FALSE，截取失败
DLLEXPORT BOOL cm_grab_to_bmp(UINT idx, WCHAR *file);

// 说明：从摄像设备截取一张图片到剪贴板
// 参数：idx，设备编号
// 返回：TRUE，截取成功；FALSE，截取失败
DLLEXPORT BOOL cm_grab_to_clipboard(UINT idx);

// 说明：开始录制视频
// 参数：idx，设备编号；file，文件名（如a.h264）
// 返回：TRUE，开始录制；FALSE，录制失败
DLLEXPORT BOOL cm_start_record(UINT idx, const char *file);

// 说明：停止录制视频
// 参数：idx，设备编号
// 返回：无
DLLEXPORT void cm_stop_record(UINT idx);

// 说明：获得摄像设备参数
// 参数：idx，设备编号；width，图像宽度；height，图像高度
// 返回：无
DLLEXPORT BOOL cm_get_param(UINT idx, int *width, int *height);

#ifdef __cplusplus
}
#endif

#endif
