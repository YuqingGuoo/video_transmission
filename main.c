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
#include <pthread.h>


int main(int argc, char *argv[])
{
    T_VideoDevice tVideoDevice;
    T_VideoBuf tVideoBuf;
    T_VideoBuf tConvertBuf;
    PT_VideoConvert ptVideoConvert;
    int iError;
    int h264_length;
    if (argc != 2)
    {
 		printf("Usage:\n");
		printf("%s </dev/video0,1,...>\n", argv[0]);
		return -1;
    }
	video_init();
    iError = video_device_init(argv[1], &tVideoDevice);
    if (iError)
    {
        printf("VideoDeviceInit for %s error!\n", argv[1]);
        return -1;
    }
    video_convert_init();
    ptVideoConvert = get_video_convert_for_formats();
    if (iError)
    {
      printf("VideoDeviceInit for %s error!\n", argv[1]);
      return -1;
    }

    iError = tVideoDevice.ptOPr->StartDevice(&tVideoDevice);
	if(iError)
	{
		printf("StartDevice error!\n");
	}
	
    memset(&tVideoBuf, 0, sizeof(tVideoBuf));
	memset(&tConvertBuf, 0, sizeof(tConvertBuf));
	
    while ( 1 )
    {
        if(tVideoDevice.ptOPr->GetFrame(&tVideoDevice, &tVideoBuf)!=0)//采集一帧数据
		{
		    printf("GetFrame is error\n");
		}
		while ( 1 )
		{
            h264_length = ptVideoConvert->Convert(&tVideoBuf, &tConvertBuf);
            if (h264_length > 0)
            {
                //printf("Convert is ok\n");
                break;
            }
		}
		iError = tVideoDevice.ptOPr->PutFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("PutFrame for error!\n");
        }
    }    
    ptVideoConvert->ConvertExit(&tConvertBuf);
	return 0;;
}




























