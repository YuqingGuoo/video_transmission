#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <convert_manager.h>
#include <omx_encode.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>


struct enc_handle *enchandle = NULL;
struct enc_param encp;

void *hd_buf, *enc_buf;
int  hd_len, enc_len;
enum pic_t ptype;

#define FRAMERATE 40


static int convert_init(void)
{
    encp.src_picwidth = WIDTH;
	encp.src_picheight = HEIGHT;
	encp.enc_picwidth = WIDTH;
	encp.enc_picheight = HEIGHT;
	encp.chroma_interleave = 0;
	encp.fps = FRAMERATE;
	encp.gop = 30;
	encp.bitrate = 800;
    enchandle = encode_open(encp);   
	if (!enchandle)
	{
		printf("--- Open encode failed\n");
		return -1;
	}
	return 0;
}

static int omx_is_support_yuv_to_h264(void)
{
    return 1;
}

static int omx_yuv_to_h264_convert_exit(PT_VideoBuf ptVideoBufOut)
{
    
	encode_close(enchandle);
	if (ptVideoBufOut->tPixelDatas.aucPixelDatas)
    {
        free(ptVideoBufOut->tPixelDatas.aucPixelDatas);
        ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
    }
    return 0;
	
}


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


static int omx_yuv_to_h264_convert(PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut)
{
    int ret;
    PT_PixelDatas ptPixelDatasIn  = &ptVideoBufIn->tPixelDatas;
    PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas;
    if ( ptVideoBufIn->iPixelFormat == V4L2_PIX_FMT_YUV420 )
    {
        if (!ptPixelDatasOut->aucPixelDatas)
        {
            ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);
        }
        ptPixelDatasOut->iWidth  = ptPixelDatasIn->iWidth;
        ptPixelDatasOut->iHeight = ptPixelDatasIn->iHeight;
        ptPixelDatasOut->iTotalBytes = ptPixelDatasIn->iTotalBytes;   
    	while ((ret = encode_get_headers(enchandle, &hd_buf, &hd_len, &ptype))== 1);
    	ret = encode_do(enchandle, ptPixelDatasIn->aucPixelDatas, ptPixelDatasIn->iTotalBytes,&enc_buf, &enc_len,&ptype);
    	if (ret < 0)
    	{
    		printf(" encode_do failed\n");
            return -1;
    	}
    	
    	ptPixelDatasOut->aucPixelDatas = (unsigned char *)enc_buf;
    	ptPixelDatasOut->iTotalBytes = enc_len;
    	return ptPixelDatasOut->iTotalBytes;
    }
    else if ( ptVideoBufIn->iPixelFormat == V4L2_PIX_FMT_YUYV )
    {
        PT_PixelDatas ptPixelDatasTmp = NULL;
        if (ptPixelDatasTmp == NULL)
        {
            ptPixelDatasTmp = malloc(sizeof(T_PixelDatas));
            //printf("ptPixelDatasTmp = %p\n",ptPixelDatasTmp);
        }
        ptPixelDatasTmp->iWidth  = ptPixelDatasIn->iWidth;
        ptPixelDatasTmp->iHeight = ptPixelDatasIn->iHeight;
        ptPixelDatasTmp->iTotalBytes = ptPixelDatasIn->iWidth*ptPixelDatasIn->iHeight*1.5;
        if (!ptPixelDatasTmp->aucPixelDatas)
        {
            ptPixelDatasTmp->aucPixelDatas = malloc(ptPixelDatasTmp->iTotalBytes);
            printf("ptPixelDatasTmp->aucPixelDatas = %p\n",ptPixelDatasTmp->aucPixelDatas);
            printf("ptPixelDatasIn->aucPixelDatas = %p\n",ptPixelDatasIn->aucPixelDatas);
        }
        //memset(ptPixelDatasTmp->aucPixelDatas,0)
        yuv_to_yuv420(ptPixelDatasIn->aucPixelDatas,ptPixelDatasTmp->aucPixelDatas,ptPixelDatasTmp->iWidth, ptPixelDatasTmp->iHeight);
    	while ((ret = encode_get_headers(enchandle, &hd_buf, &hd_len, &ptype))== 1);
    	ret = encode_do(enchandle, ptPixelDatasTmp->aucPixelDatas, ptPixelDatasTmp->iTotalBytes,&enc_buf, &enc_len,&ptype);
    	if (ret < 0)
    	{
    		printf(" encode_do failed\n");
            return -1;
    	}
    	
    	ptPixelDatasOut->aucPixelDatas = (unsigned char *)enc_buf;
    	ptPixelDatasOut->iTotalBytes = enc_len;
    	//free(ptPixelDatasTmp->aucPixelDatas);
    	free(ptPixelDatasTmp);
    	return 1;
    }

}


static T_VideoConvert g_tYuv2H264Convert = {
    .name        = "yuvtoh264",
    .isSupport   = omx_is_support_yuv_to_h264,
    .Convert     = omx_yuv_to_h264_convert,
    .ConvertExit = omx_yuv_to_h264_convert_exit,
};


int omx_yuv_to_h264_init(void)
{
    convert_init();
    return register_video_convert(&g_tYuv2H264Convert);
}


