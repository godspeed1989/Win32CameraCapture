#include "../source/CameraCapture.h"

#include <stdio.h>

void run(UINT cm_idx)
{
	int width, height;
	// 初始化摄像驱动
	cm_init(cm_idx);
	// 打开摄像设备
	cm_open(cm_idx);

	// 获得摄像设备参数
	cm_get_param(cm_idx, &width, &height);
	printf("%d X %d\n", width, height);

	// 从摄像设备截取一张图片到BMP文件
	Sleep(500);
	cm_grab_to_bmp(cm_idx, L"aaa.bmp");
	Sleep(500);
	cm_grab_to_bmp(cm_idx, L"bbb.bmp");
	
	// 开始录制视频
	cm_start_record(cm_idx, "Cam.h264");
	printf("录制视频开始，按回车停止录制。。。\n");
	getchar();
	// 停止录制视频
	cm_stop_record(cm_idx);
	printf("视频录制结束\n");

	// 关闭摄像设备
	cm_close(cm_idx);
	// 清理摄像驱动
	cm_exit(cm_idx);
}

int main()
{
	UINT cm_idx, cm_cnt;

	// 打印摄像设备列表
	cm_cnt = cm_print_driver_list();
	run(0);
	run(1);

	system("pause");
	return 0;
}
