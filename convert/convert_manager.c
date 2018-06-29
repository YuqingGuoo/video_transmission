#include <convert_manager.h>
#include <string.h>
#include <stdio.h>

static PT_VideoConvert g_ptVideoConvertHead = NULL;


int register_video_convert(PT_VideoConvert ptVideoConvert)
{
	PT_VideoConvert ptTmp;

	if (!g_ptVideoConvertHead)
	{
		g_ptVideoConvertHead   = ptVideoConvert;
		ptVideoConvert->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptVideoConvertHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext     = ptVideoConvert;
		ptVideoConvert->ptNext = NULL;
	}
    printf("register_video_convert success\n");
	return 0;
}


void show_video_convert(void)
{
	int i = 0;
	PT_VideoConvert ptTmp = g_ptVideoConvertHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


PT_VideoConvert get_video_convert(char *pcName)
{
	PT_VideoConvert ptTmp = g_ptVideoConvertHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


PT_VideoConvert get_video_convert_for_formats(void)
 {
 	PT_VideoConvert ptTmp = g_ptVideoConvertHead;
 	
 	while (ptTmp)
 	{
         if (ptTmp->isSupport())
         {
             printf("get_video_convert success\n");
             return ptTmp;
         }
 		ptTmp = ptTmp->ptNext;
 	}
 	return NULL;
 }


int video_convert_init(void)
{
	int iError;
    //iError = yuv_to_h264_init();
    //iError = yuv_to_yuv420_init();
    iError |= omx_yuv_to_h264_init();
	return iError;
}


