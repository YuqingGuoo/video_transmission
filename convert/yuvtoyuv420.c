#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <convert_manager.h>


static int yuv_to_yuv420(unsigned char *pSrc, unsigned char *pDest,int iWidth, int iHeight)
{

    int i, j;
    unsigned char *u = pDest + (iWidth * iHeight);
    unsigned char *v = u + (iWidth * iHeight) / 4;

    for (i = 0; i < iHeight/2; i++)
    {

        unsigned char *src_l1 = pSrc + iWidth*2*2*i;
        unsigned char *src_l2 = src_l1 + iWidth*2;
        unsigned char *y_l1 = pDest + iWidth*2*i;
        unsigned char *y_l2 = y_l1 + iWidth;
        for (j = 0; j < iWidth/2; j++)
        {
            // two pels in one go
            *y_l1++ = src_l1[0];
            *u++ = src_l1[1];
            *y_l1++ = src_l1[2];
            *v++ = src_l1[3];
            *y_l2++ = src_l2[0];
            *y_l2++ = src_l2[2];
            src_l1 += 4;
            src_l2 += 4;
        }
    }
    return 0;
}


static int is_support_yuv_to_yuv420(void)
{
    return 0;
}



static int yuv_to_yuv420_convert(PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut)
{
    PT_PixelDatas ptPixelDatasIn  = &ptVideoBufIn->tPixelDatas;
    PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas;
    ptPixelDatasOut->iWidth  = ptPixelDatasIn->iWidth;
    ptPixelDatasOut->iHeight = ptPixelDatasIn->iHeight;
    ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iWidth*ptPixelDatasOut->iHeight*1.5;
    if (!ptPixelDatasOut->aucPixelDatas)
    {
        ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);
    }
    printf("ptPixelDatasIn = %p,ptPixelDatasOut = %p\n",ptPixelDatasIn->aucPixelDatas,ptPixelDatasOut->aucPixelDatas);
    yuv_to_yuv420(ptPixelDatasIn->aucPixelDatas,ptPixelDatasOut->aucPixelDatas,ptPixelDatasOut->iWidth, ptPixelDatasOut->iHeight);
    return 0;
}


static int yuv_to_yuv420_convert_exit(PT_VideoBuf ptVideoBufOut) 
{

	if (ptVideoBufOut->tPixelDatas.aucPixelDatas)
    {
        free(ptVideoBufOut->tPixelDatas.aucPixelDatas);
        ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
    }
    return 0;
}



static T_VideoConvert g_tYuv2Yuv420Convert = {
    .name        = "yuvtoyuv420",
    .isSupport   = is_support_yuv_to_yuv420,
    .Convert     = yuv_to_yuv420_convert,
    .ConvertExit = yuv_to_yuv420_convert_exit,
};


int yuv_to_yuv420_init(void)
{
    return register_video_convert(&g_tYuv2Yuv420Convert);
}


