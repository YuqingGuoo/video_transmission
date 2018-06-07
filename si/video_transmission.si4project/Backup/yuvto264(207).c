#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <convert_manager.h>
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
	pParam->i_csp = X264_CSP_I422;

	if( x264_param_apply_profile( pParam, "high422") < 0 )  //订制编码器性能
	{
	    printf("set x264 param profile is error\n");
	    return -1;
	}

    x264_picture_alloc(pPic_in, X264_CSP_I422, pParam->i_width, pParam->i_height);
	
	if ((pHandle = x264_encoder_open(pParam)) == 0)
	{   
	    printf("compress begin error\n");
		return -1;
	}

	//pPic_in->img.i_csp = X264_CSP_I420;
	//pPic_in->img.i_plane = 3;
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
    int iNal   = 0;  	
	int result = 0;
	int i,j = 0;
	unsigned char *p422;
	unsigned char *y = pPic_in->img.plane[0];
	unsigned char *u = pPic_in->img.plane[1];
	unsigned char *v = pPic_in->img.plane[2];

	#if 0
	
    int widthStep422 = pParam->i_width * 2;
    
    for(i = 0; i < pParam->i_height; i += 2)
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
	#endif

	#if 0
    uint8_t *u = ptPixelDatasIn->aucPixelDatas + (pParam->i_width * pParam->i_height);
    uint8_t *v = u + (pParam->i_width * pParam->i_height) / 4;

    for (i = 0; i < pParam->i_height/2; i++)
    {
        /*采取的策略是:在外层循环里面，取两个相邻的行*/    
        uint8_t *src_l1 = pSrc + pParam->i_width*2*2*i; //因为4:2:2的原因，所以占用内存，相当一个像素占2个字节，2个像素合成4个字节
        uint8_t *src_l2 = src_l1 + pParam->i_width*2;   //YUY2的偶数行下一行
        uint8_t *y_l1 = pDest + pParam->i_width*2*i;    //偶数行
        uint8_t *y_l2 = y_l1 + pParam->i_width;         //偶数行的下一行
        for (j = 0; j < pParam->i_width/2; j++)         //内层循环
        {
            // two pels in one go//一次合成两个像素
            //偶数行，取完整像素;Y,U,V;偶数行的下一行，只取Y
            *y_l1++ = src_l1[0];   //Y
            *u++ = src_l1[1];      //U
            *y_l1++ = src_l1[2];   //Y
            *v++ = src_l1[3];      //V
            //这里只有取Y
            *y_l2++ = src_l2[0];
            *y_l2++ = src_l2[2];
            //YUY2,4个像素为一组
            src_l1 += 4;
            src_l2 += 4;
        }
    }

	#endif

	pPic_in->i_type = X264_TYPE_AUTO;
	if ( x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out) < 0) 
	{
	    printf("encode error\n");
		return -1;
	}
	pPic_in->i_pts++;
    if (iNal > 0)
    {
    	for (i = 0; i < iNal; i++) 
    	{
    	    ptPixelDatasOut->aucPixelDatas = pNals[i].p_payload;
        	//ptPixelDatasOut->aucPixelDatas += pNals[i].i_payload;
        	result += pNals[i].i_payload;
        	//printf("iTotalBytes = %d\n",ptPixelDatasOut->iTotalBytes);
    	}
    }
	return result;
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


