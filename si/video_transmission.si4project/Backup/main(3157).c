#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <video_manager.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


int main(int argc, char *argv[])
{
	/* code */
	int iError;
	int iPixelFormatOfVideo;
	int iPixelFormatOfDisp;
	T_VideoDevice tVideoDevice;
	T_VideoBuf tVideoBuf;

    if (argc != 2)
    {
 		printf("Usage:\n");
		printf("%s </dev/video0,1,...>\n", argv[0]);
		return -1;
    }

    //iPixelFormatOfDisp = V4L2_PIX_FMT_YUV420;
    
    video_init();
    
    iError = video_device_init(argv[1], &tVideoDevice);
    if (iError)
    {
        printf("VideoDeviceInit for %s error!\n", argv[1]);
        return -1;
    }
    iPixelFormatOfVideo = tVideoDevice.ptOPr->GetFormat(&tVideoDevice);

	/* 启动摄像头设备 */
	iError = tVideoDevice.ptOPr->StartDevice(&tVideoDevice);
	if(iError)
	{
		printf("StartDevice for %s error!\n", argv[1]);
		return -1;
	}
    
    /* 申请帧内存 */
	memset(&tVideoBuf, 0, sizeof(tVideoBuf)); 
	
    while (1)
    {
        /* 读入摄像头数据保存到帧内存 */
        iError = tVideoDevice.ptOPr->GetFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("GetFrame for %s error!\n", argv[1]);
        	return -1;
        }
        iError = tVideoDevice.ptOPr->PutFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("PutFrame for %s error!\n", argv[1]);
        	return -1;
        }
    }
    
	return 0;
}




























