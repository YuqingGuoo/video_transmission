#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <convert_manager.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>


/* 
    av_register_all()：注册所有的编解码器，复用/解复用器等等组件。其中调用了avcodec_register_all()注册所有编解码器相关的组件。
    avformat_alloc_context()：创建AVFormatContext结构体。
    avformat_alloc_output_context2()：初始化一个输出流。
    avio_open()：打开输出文件。
    avformat_new_stream()：创建AVStream结构体。avformat_new_stream()中会调用avcodec_alloc_context3()
    创建AVCodecContext结构体。
    avformat_write_header()：写文件头。
    av_write_frame()：写编码后的文件帧。
    av_write_trailer()：写文件尾。
    （2）	新增了如下几个函数
    avcodec_register_all()：只注册编解码器有关的组件。
    avcodec_alloc_context3()：创建AVCodecContext结构体。 
*/

AVFrame *pFrame;
AVPacket pkt;
AVCodec *pCodec;
AVCodecContext *pCodecCtx = NULL;

int convert_init(void)
{
    int ret;
    enum AVCodecID codec_id = AV_CODEC_ID_H264;
    /* 只注册编解码器有关的组件 */
    avcodec_register_all();
    
    pCodec = avcodec_find_encoder(codec_id);
    if (!pCodec)
    {
        printf("Codec not found\n");
        return -1;
    }
    /* 创建AVCodecContext结构体 */
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx)
    {
        printf("Could not allocate video codec context\n");
        return -1;
    }
    pCodecCtx->bit_rate = 400000;
    pCodecCtx->width = WIDTH;
    pCodecCtx->height = HEIGHT;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->gop_size = 10;
    pCodecCtx->max_b_frames = 1;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(pCodecCtx->priv_data, "preset", "faster", 0);

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Could not open codec\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    if (!pFrame)
    {
        printf("Could not allocate video frame\n");
        return -1;
    }
    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;

    ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height,
                         pCodecCtx->pix_fmt, 16);
    if (ret < 0)
    {
        printf("Could not allocate raw picture buffer\n");
        return -1;
    }
	return 0;
}


static int is_support_yuv_to_h264(void)
{
    return 1;
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


static int yuv_to_h264_convert_exit(PT_VideoBuf ptVideoBufOut) 
{
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&pFrame->data[0]);
    av_frame_free(&pFrame);

	if (ptVideoBufOut->tPixelDatas.aucPixelDatas)
    {
        free(ptVideoBufOut->tPixelDatas.aucPixelDatas);
        ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
    }
    return 0;
	
}



static int yuv_to_h264_convert(PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut)
{
    int iTotalBytes,got_output,ret;
    //unsigned char *ptTmp = NULL;
    unsigned char *p422 = NULL;
    unsigned char *y = pFrame->data[0];
	unsigned char *u = pFrame->data[1];
	unsigned char *v = pFrame->data[2];
	int i,j = 0;
    //Read raw YUV data
    PT_PixelDatas ptPixelDatasIn  = &ptVideoBufIn->tPixelDatas;
    PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas; 
    #if  0
    iTotalBytes = ptPixelDatasIn->iWidth * ptPixelDatasIn->iHeight*1.5;
    
    if (!ptTmp)
    {
        ptTmp = malloc(iTotalBytes);
    }
    
    yuv_to_yuv420(ptPixelDatasIn->aucPixelDatas,ptTmp,ptPixelDatasIn->iWidth, ptPixelDatasIn->iHeight);
    
    memcpy(y,ptTmp,307200);
    memcpy(u,ptTmp+307200,76800);
    memcpy(v,ptTmp+384000,76800);
    #endif /* #if 0 */
    #if  1
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
    #endif /* #if 1 */
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
    pFrame->pts ++;
    /* encode the image */
    ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
    if (ret < 0) 
    {
        printf("Error encoding frame\n");
        return -1;
    }
    if (got_output) 
    {
        printf("Succeed to encode frame:tsize:%5d\n",pkt.size);
        ptPixelDatasOut->aucPixelDatas = pkt.data;
        ptPixelDatasOut->iTotalBytes = pkt.size;
        av_free_packet(&pkt);
        free(ptTmp);
	}
	return ptPixelDatasOut->iTotalBytes;
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


