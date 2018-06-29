#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <convert_manager.h>
#include <fcntl.h>
#include <unistd.h>

#include <x264.h>

/* x264_param_default()：设置参数集结构体x264_param_t的缺省值。
   
   x264_picture_alloc()：为图像结构体x264_picture_t分配内存。
   x264_encoder_open()：打开编码器。
   x264_encoder_encode()：编码一帧图像。
   x264_encoder_close()：关闭编码器。
   x264_picture_clean()：释放x264_picture_alloc()申请的资源。
   
   存储数据的结构体如下所示。
   x264_picture_t：存储压缩编码前的像素数据。
   x264_nal_t：存储压缩编码后的码流数据。 
 */

x264_nal_t* pNals = NULL;  
x264_t* pHandle   = NULL;  
x264_picture_t* pPic_in;
x264_picture_t* pPic_out;
x264_param_t* pParam;
int fd;

int convert_init(void)
{
    pPic_in  = (x264_picture_t*)malloc(sizeof(x264_picture_t));
    pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
    pParam   = (x264_param_t*)malloc(sizeof(x264_param_t));
    if( x264_param_default_preset( pParam, "ultrafast", NULL ) < 0 ) //编码器默认设置
    {
        printf("set x264 param default is error\n");
        return -1;
    }

	pParam->i_width = WIDTH;   //set frame width
	pParam->i_height = HEIGHT; //set frame height
	pParam->i_csp = X264_CSP_I420;


	if( x264_param_apply_profile( pParam, "high") < 0 )  //订制编码器性能
	{
	    printf("set x264 param profile is error\n");
	    return -1;
	}

    x264_picture_alloc(pPic_in, X264_CSP_I420, pParam->i_width, pParam->i_height);
	
	if ((pHandle = x264_encoder_open(pParam)) == 0)
	{   
	    printf("compress begin error\n");
		return -1;
	}

    if ((fd = open("test.h264",O_WRONLY|O_CREAT))<0)
    {
        printf("open error:\n");
        return -1;
    }
	return 0;
}


static int is_support_yuv_to_h264(void)
{
    return 1;
}

static int yuv_to_h264_convert_exit(PT_VideoBuf ptVideoBufOut) {
	if (pHandle) 
	{
		x264_encoder_close(pHandle);
	}
	if (pPic_in) 
	{
		x264_picture_clean(pPic_in);
		free(pPic_in);
		pPic_in = NULL;
	}
	if (pPic_out) 
	{
		x264_picture_clean(pPic_out);
		free(pPic_out);
		pPic_out = NULL;
	}
	if (pParam) 
	{
		free(pParam);
		pParam = NULL;
	}
	    if (ptVideoBufOut->tPixelDatas.aucPixelDatas)
    {
        free(ptVideoBufOut->tPixelDatas.aucPixelDatas);
        ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
    }
    return 0;
	
}



static int yuv_to_h264_convert(PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut)
{
    PT_PixelDatas ptPixelDatasIn  = &ptVideoBufIn->tPixelDatas;
    PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas;

    if (!ptPixelDatasOut->aucPixelDatas)
    {
        ptPixelDatasOut->aucPixelDatas = malloc(800*1024);
    }
    int iNal= 0;  	
	int i,j = 0;
    int i_frame_size;
    unsigned char *pout;
	unsigned char *p422;
	unsigned char *y = pPic_in->img.plane[0];
	unsigned char *u = pPic_in->img.plane[1];
	unsigned char *v = pPic_in->img.plane[2];

    int widthStep422 = ptPixelDatasIn->iWidth * 2;
    
    for(i = 0; i < ptPixelDatasIn->iHeight; i += 2)
    
	{
		p422 = ptPixelDatasIn->aucPixelDatas + i * widthStep422;

		for(j = 0; j < widthStep422; j+=4)
		{
			*(y++) = p422[j];
			*(u++) = p422[j+1];
			*(y++) = p422[j+2];
		}

		p422 += widthStep422;

		for(j = 0; j < widthStep422; j+=4)
		{
			*(y++) = p422[j];
			*(v++) = p422[j+3];
			*(y++) = p422[j+2];
		}
	}
	pout = ptPixelDatasOut->aucPixelDatas;
	pPic_in->i_type = X264_TYPE_AUTO;
	pPic_in->i_pts++;
	i_frame_size = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
	if ( i_frame_size < 0) 
	{
	    printf("encode error\n");
		return -1;
	}
    for (i = 0; i < iNal; i++) 
    {  
        memcpy(pout, pNals[i].p_payload, pNals[i].i_payload);     
        pout += pNals[i].i_payload;
    }
	return i_frame_size;
}


static T_VideoConvert g_tYuv2H264Convert = {
    .name        = "yuvtoh264",
    .isSupport   = is_support_yuv_to_h264,
    .Convert     = yuv_to_h264_convert,
    .ConvertExit = yuv_to_h264_convert_exit,
};


int yuv_to_h264_init(void)
{
    convert_init();
    return register_video_convert(&g_tYuv2H264Convert);
}


