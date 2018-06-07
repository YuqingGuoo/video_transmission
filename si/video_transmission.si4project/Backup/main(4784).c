#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <video_manager.h>
#include <convert_manager.h>
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
	T_VideoBuf tConvertBuf;
	PT_VideoConvert ptVideoConvert;
	int fd;

    if (argc != 2)
    {
 		printf("Usage:\n");
		printf("%s </dev/video0,1,...>\n", argv[0]);
		printf("%s </dir>\n",argv[0]);
		return -1;
    }
    video_init();
    video_convert_init();
    
    iError = video_device_init(argv[1], &tVideoDevice);
    if (iError)
    {
        printf("VideoDeviceInit for %s error!\n", argv[1]);
        return -1;
    }

//   if ((fd = open(argv[2],O_WRONLY|O_CREAT))<0)
//    {
//        printf("open error:\n");
//        return -1;
//    }
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
	memset(&tConvertBuf, 0, sizeof(tConvertBuf));
	
    while (1)
    {
        /* 读入摄像头数据保存到帧内存 */
        iError = tVideoDevice.ptOPr->GetFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("GetFrame for %s error!\n", argv[1]);
        	return -1;
        }
        ptVideoConvert->Convert(&tVideoBuf, &tConvertBuf);
//		if(iError)
//		{
//			printf("Convert for %s error!\n", argv[1]);
//			return -1;
//		}
        
        iError = tVideoDevice.ptOPr->PutFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("PutFrame for %s error!\n", argv[1]);
        	return -1;
        }

//        if (write(fd,tConvertBuf.tPixelDatas.aucPixelDatas,tConvertBuf.tPixelDatas.iTotalBytes)!=tConvertBuf.tPixelDatas.iTotalBytes)
//        {
//            printf("write error\n");
//            return -1;
//        }
    }
	return 0;
}




























