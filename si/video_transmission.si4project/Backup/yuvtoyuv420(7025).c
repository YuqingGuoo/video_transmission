#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <convert_manager.h>


static int is_support_yuv_to_yuv420(void)
{
    return 1;
}



static int yuv_to_yuv420_convert(PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut)
{
    PT_PixelDatas ptPixelDatasIn  = &ptVideoBufIn->tPixelDatas;
    PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas;
    ptPixelDatasOut->iTotalBytes = ptVideoBufIn->tPixelDatas.iWidth * ptVideoBufIn->tPixelDatas.iHeight * 1.5;
    ptPixelDatasIn->aucPixelDatas = (unsigned char *)malloc(ptPixelDatasIn->iTotalBytes * sizeof(char));
    ptPixelDatasOut->aucPixelDatas = (unsigned char *)malloc(ptPixelDatasOut->iTotalBytes * sizeof(char));
    memset(ptPixelDatasOut->aucPixelDatas, 0, ptPixelDatasOut->iTotalBytes);
    int i, j;
    unsigned char *Y, *U, *V;
    unsigned char *Y2, *U2, *V2;
    Y = ptPixelDatasIn->aucPixelDatas;
    U = Y + ptVideoBufIn->tPixelDatas.iWidth * ptVideoBufIn->tPixelDatas.iHeight;
    V = U + (ptVideoBufIn->tPixelDatas.iWidth * ptVideoBufIn->tPixelDatas.iHeight >> 1); //Y  U  V  =4 : 2 ; 2
    
    Y2 = ptPixelDatasOut->aucPixelDatas;
    U2 = Y2 + ptVideoBufIn->tPixelDatas.iWidth * ptVideoBufIn->tPixelDatas.iHeight;
    V2 = U2 + (ptVideoBufIn->tPixelDatas.iWidth * ptVideoBufIn->tPixelDatas.iHeight >> 2);
    printf("1\n");
    for (i = 0; i < ptVideoBufIn->tPixelDatas.iHeight; ++i)
    {
        for (j = 0; j < (ptVideoBufIn->tPixelDatas.iHeight >> 1); ++j)
        {
            Y[j * 2] = ptPixelDatasIn->aucPixelDatas[4 * j];
            U[j] = ptPixelDatasIn->aucPixelDatas[4 * j + 1];
            Y[j * 2 + 1] = ptPixelDatasIn->aucPixelDatas[4 * j + 2];
            V[j] = ptPixelDatasIn->aucPixelDatas[4 * j + 3];
        }
        ptPixelDatasIn->aucPixelDatas = ptPixelDatasIn->aucPixelDatas + ptVideoBufIn->tPixelDatas.iHeight * 2;

        Y = Y + ptVideoBufIn->tPixelDatas.iWidth;
        U = U + (ptVideoBufIn->tPixelDatas.iWidth >> 1);
        V = V + (ptVideoBufIn->tPixelDatas.iWidth >> 1);
    }

    Y = ptPixelDatasIn->aucPixelDatas;
    U = Y + ptVideoBufIn->tPixelDatas.iWidth * ptVideoBufIn->tPixelDatas.iHeight;
    V = U + (ptVideoBufIn->tPixelDatas.iWidth * ptVideoBufIn->tPixelDatas.iHeight >> 1); //Y  U  V  =4 : 2 ; 2

    int l;
    for (l = 0; l < ptVideoBufIn->tPixelDatas.iHeight / 2; ++l)
    {
        memcpy(U2, U, ptVideoBufIn->tPixelDatas.iWidth >> 1);
        memcpy(V2, V, ptVideoBufIn->tPixelDatas.iWidth >> 1);

        U2 = U2 + (ptVideoBufIn->tPixelDatas.iWidth >> 1);
        V2 = V2 + (ptVideoBufIn->tPixelDatas.iWidth >> 1);

        U = U + (ptVideoBufIn->tPixelDatas.iWidth);
        V = V + (ptVideoBufIn->tPixelDatas.iWidth);
    }
    memcpy(Y2, Y, ptVideoBufIn->tPixelDatas.iWidth * ptVideoBufIn->tPixelDatas.iWidth);
    free(ptPixelDatasIn->aucPixelDatas);
    free(ptPixelDatasOut->aucPixelDatas);
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


