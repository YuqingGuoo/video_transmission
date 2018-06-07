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

int iNal   = 0;  
x264_nal_t* pNals = NULL;  
x264_t* pHandle   = NULL;  
x264_picture_t* pPic_in;
x264_picture_t* pPic_out;
x264_param_t* pParam;


void compress_begin(void) 
{
    pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
    pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
    pParam = (x264_param_t*)malloc(sizeof(x264_param_t));
	if( x264_param_default_preset( pParam, "medium", NULL ) < 0 )
	{
	    printf("set x264 param default is error\n");
	    return -1;
	}
	//en->param->rc.i_rc_method = X264_RC_CQP;
	// en->param->i_log_level = X264_LOG_NONE;

	pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
	pParam->i_width = WIDTH;   //set frame width
	pParam->i_height = HEIGHT; //set frame height
	//en->param->i_frame_total = 0;
	//en->param->i_keyint_max = 10;
	pParam->rc.i_lookahead = 0; 
	//en->param->i_bframe = 5; 

	//en->param->b_open_gop = 0;
	//en->param->i_bframe_pyramid = 0;
	//en->param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

	//en->param->rc.i_bitrate = 1024 * 10;//rate 10 kbps
	pParam->i_fps_num = 30; 
	pParam->i_fps_den = 1;
	pParam->i_csp = X264_CSP_I422;

	x264_param_apply_profile(pParam, x264_profile_names[4]); 

	if ((pHandle = x264_encoder_open(pParam)) == 0)
	{   
	    printf("set x264 encoder open is error\n");
		return -1 ;
	}
	
	/* Create a new pic */
	x264_picture_alloc(pPic_in, X264_CSP_I422, pParam->i_width,
			pParam->i_height);
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
    printf("\r\n |yuv_to_h264_convert() entry--- ");
    PT_PixelDatas ptPixelDatasIn  = &ptVideoBufIn->tPixelDatas;
    PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas;
	int index_y, index_u, index_v;
	int num;
	int nNal = 0;
	int result = 0;
	int i = 0;
	static long int pts = 0;
	unsigned char *y = pPic_in->img.plane[0];   
	unsigned char *u = pPic_in->img.plane[1];   
	unsigned char *v = pPic_in->img.plane[2];   
	char * ptr;

	index_y = 0;
    index_u = 0;
    index_v = 0;

    num = ptPixelDatasIn->iWidth * ptPixelDatasIn->iHeight * 2 - 4  ;

    for(i=0; i<num; i=i+4)
    {
            *(y + (index_y++)) = *(ptPixelDatasIn->aucPixelDatas + i);
            *(u + (index_u++)) = *(ptPixelDatasIn->aucPixelDatas + i + 1);
            *(y + (index_y++)) = *(ptPixelDatasIn->aucPixelDatas + i + 2);
            *(v + (index_v++)) = *(ptPixelDatasIn->aucPixelDatas + i + 3);
    }

	pPic_in->i_type = X264_TYPE_AUTO;
	pPic_in->i_pts = pts++;
	if (x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out) < 0) 
	{
		printf("\r\n |yuv_to_h264_convert() exit---: 1 ");
		return -1;
	}
    ptPixelDatasOut->iTotalBytes = pNals[i].i_payload;
	for (i = 0; i < nNal; i++) {
		memcpy(ptPixelDatasOut->aucPixelDatas, pNals[i].p_payload, pNals[i].i_payload);   
		ptPixelDatasOut->aucPixelDatas += pNals[i].i_payload;								 
		result += pNals[i].i_payload;
	}
	printf("\r\n |yuv_to_h264_convert() exit---: 2 ");
	return result;
}


static T_VideoConvert g_tYuv2H264Convert = {
    .name        = "uv2H264",
    .Convert     = yuv_to_h264_convert,
    .ConvertExit = yuv_to_h264_convert_exit,
};


int yuv_to_h264_init(void)
{
    compress_begin();
    return register_video_convert(&g_tYuv2H264Convert);
}


