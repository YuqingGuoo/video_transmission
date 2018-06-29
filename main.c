#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <video_manager.h>
#include <convert_manager.h>
#include <tran_manager.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/time.h>
#include <RTPEnc.h>


int fd;
struct timeval startTime,endTime;
struct pac_handle *pachandle = NULL;

float Timeuse;
int iError;
void *pac_buf;
int pac_len;
int iDelayTime;

T_VideoDevice tVideoDevice;
T_VideoBuf tVideoBuf;
T_VideoBuf tConvertBuf;
PT_VideoConvert ptVideoConvert;
T_ContextDevice tContextDevice;
RTPMuxContext rtpMuxContext;



pthread_cond_t captureOK; /*线程采集满一个时的标志*/
pthread_cond_t encodeOK; /*线程编码完一个的标志*/
pthread_mutex_t lock;     /*互斥锁*/
pthread_t CaptureThread;
pthread_t EncodetTread;

/*****************************************************************************
 函 数 名  : init_fun
 功能描述  : 初始化函数
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月9日
    作    者   : ganlin
    修改内容   : 新生成函数

*****************************************************************************/



static void init_fun(void)
{

	pthread_mutex_init(&lock,NULL);     /* 以动态方式创建互斥锁 */
	pthread_cond_init(&captureOK,NULL); /* 初始化captureOK条件变量 */
	pthread_cond_init(&encodeOK,NULL);  /* 初始化encodeOK条件变量 */
	 /* 申请帧内存,和转码帧内存 */
	memset(&tVideoBuf, 0, sizeof(tVideoBuf)); 
	memset(&tConvertBuf, 0, sizeof(tConvertBuf));
}

/*****************************************************************************
 函 数 名  : video_Capture_Thread
 功能描述  : 摄像头线程
 输入参数  : void *arg  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月9日
    作    者   : ganlin
    修改内容   : 新生成函数

*****************************************************************************/
void *video_Capture_Thread(void *arg)
{
    /* 启动摄像头设备 */
    iError = tVideoDevice.ptOPr->StartDevice(&tVideoDevice);
	if(iError)
	{
		printf("StartDevice error!\n");
	}
	while(1)
	{   
	    #if  1
	    /* 设置相对时间，30ms读取一次摄像头 */
		long long absmsec;
        struct timeval now;  
        struct timespec outtime;  
    	gettimeofday(&now, NULL);
    	absmsec = now.tv_sec * 1000ll + now.tv_usec / 1000ll;
    	absmsec += iDelayTime;
    	outtime.tv_sec = absmsec / 1000ll;
    	outtime.tv_nsec = absmsec % 1000ll * 1000000ll;    	
    	pthread_mutex_lock(&lock); /*获取互斥锁*/
    	
        /* 读入摄像头数据保存到帧内存 */
        /*注意pthread_mutex_lock时以下的资源将被锁定不能访问,所以pthread_mutex_lock单独不能放在
         *GetFrame的前面
         */
        /* 每30ms循环一次 */
        pthread_cond_timedwait(&encodeOK,&lock,&outtime);
		if(tVideoDevice.ptOPr->GetFrame(&tVideoDevice, &tVideoBuf)==0)//采集一帧数据
		{
		    pthread_cond_signal(&captureOK);
		}
		iError = tVideoDevice.ptOPr->PutFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("PutFrame for error!\n");
        }
        
		pthread_mutex_unlock(&lock);     /*释放互斥锁*/
    	#endif /* #if 0 */
		#if  0
		if(tVideoDevice.ptOPr->GetFrame(&tVideoDevice, &tVideoBuf)==0)//采集一帧数据
		{
		    
		}
		iError = tVideoDevice.ptOPr->PutFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("PutFrame for error!\n");
        }
    	pthread_mutex_lock(&lock); /*获取互斥锁*/
        pthread_cond_signal(&captureOK);
        pthread_mutex_unlock(&lock);     /*释放互斥锁*/       
        #endif /* #if 0 */
	}
}

/*****************************************************************************
 函 数 名  : video_Encode_Thread
 功能描述  : 转码线程
 输入参数  : void *arg  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月9日
    作    者   : ganlin
    修改内容   : 新生成函数

*****************************************************************************/
void *video_Encode_Thread(void *arg)
{
    struct timeval startTime,endTime;
    float Timeuse;
    int ret;
    int cnt = 0;
    while (1)
	{
	    gettimeofday(&startTime,NULL);
	    pthread_mutex_lock(&lock); /* 获取互斥锁 */
	    pthread_cond_wait(&captureOK,&lock);
	    pthread_mutex_unlock(&lock);/*释放互斥锁*/
        if(tVideoBuf.tPixelDatas.aucPixelDatas != NULL)
        {
        	if (ptVideoConvert->Convert(&tVideoBuf, &tConvertBuf) > 0) 
        	{   
        	    cnt++;
        	    if(cnt >= 20)
        	    {
                    //printf("iTotalBytes = %d\n",tConvertBuf.tPixelDatas.iTotalBytes);
                    //write(fd,tConvertBuf.tPixelDatas.aucPixelDatas,tConvertBuf.tPixelDatas.iTotalBytes);
                    rtpSendH264HEVC(&rtpMuxContext, &tContextDevice, tConvertBuf.tPixelDatas.aucPixelDatas, tConvertBuf.tPixelDatas.iTotalBytes);
                    cnt = 20;
        	    }
        	}
    	}
    	gettimeofday(&endTime,NULL);
    	Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    	Timeuse /= 1000000;
    	printf("Timeuse = %f\n",Timeuse);
	}
}

/*****************************************************************************
 函 数 名  : thread_create
 功能描述  : 创建线程
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月9日
    作    者   : ganlin
    修改内容   : 新生成函数

*****************************************************************************/
void thread_create(void)
{
        int temp;
        memset(&CaptureThread, 0, sizeof(CaptureThread));
        memset(&EncodetTread, 0, sizeof(EncodetTread));
		/*创建线程*/
        if((temp = pthread_create(&CaptureThread, NULL, video_Capture_Thread, NULL)) != 0)   
                printf("video_Capture_Thread create fail!\n");
        if((temp = pthread_create(&EncodetTread, NULL, video_Encode_Thread, NULL)) != 0)  
                printf("video_Encode_Thread create fail!\n");
}

void thread_wait(void)
{
    /*等待线程结束*/
    if(CaptureThread !=0) {  
            pthread_join(CaptureThread,NULL);
    }
    if(EncodetTread !=0) {   
            pthread_join(EncodetTread,NULL);
    }
}

#if  1
int main(int argc, char *argv[])
{

    if (argc < 6)
    {
 		printf("Usage:\n");
		printf("</dev/video0,yuyv,640,480,30>\n");
		return -1;
    }
    #if  0
    if ((fd = open("Sample.h264",O_WRONLY|O_CREAT))<0)
    
    {
        printf("open error:\n");
        return -1;
    }
    #endif /* #if 0 */
    if (!strcmp(argv[2],"YUYV"))
    {
        tVideoDevice.iPixelFormat = V4L2_PIX_FMT_YUYV;
    }
    else if (!strcmp(argv[2],"MJPEG"))
    {
        tVideoDevice.iPixelFormat = V4L2_PIX_FMT_MJPEG;
    }
    else if (!strcmp(argv[2],"YUY2"))
    {
        tVideoDevice.iPixelFormat = V4L2_PIX_FMT_YUV420;
    }

    tVideoDevice.iWidth = atoi(argv[3]);
    tVideoDevice.iHeight = atoi(argv[4]);
    tVideoDevice.iFps = atoi(argv[5]);
    iDelayTime = atoi(argv[5]);
    if (argv[6] == NULL)
    {
        tContextDevice.dstIp = "127.0.0.1";
    }
    else
    {
        tContextDevice.dstIp = argv[6];
    }
    if (argv[6] == NULL)
    {
        tContextDevice.dstPort = 1234;
    }
    else
    {
        tContextDevice.dstPort = atoi(argv[7]);
    }
    init_fun();
	tran_init();
	tran_chanel_init(&tContextDevice);
	video_init();
    initRTPMuxContext(&rtpMuxContext);
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
      printf("VideoDeviceConvert is error!\n");
      return -1;
    }

    iError = tVideoDevice.ptOPr->StartDevice(&tVideoDevice);
	if(iError)
	{
		printf("StartDevice error!\n");
	}
	
	thread_create();
    thread_wait();

    ptVideoConvert->ConvertExit(&tConvertBuf);
	return 0;;
}
#endif /* #if 0 */

#if  0

int main(int argc, char *argv[])
{
    struct timeval startTime,endTime;
    float Timeuse;
    if ((fd = open("Sample.h264",O_WRONLY|O_CREAT))<0)
    {
        printf("open error:\n");
        return -1;
    }
    if (argc != 6)
    {
 		printf("Usage:\n");
		printf("</dev/video0,yuyv,640,480,30>\n");
		return -1;
    }
    if (!strcmp(argv[2],"YUYV"))
    {
        tVideoDevice.iPixelFormat = V4L2_PIX_FMT_YUYV;
    }
    else if (!strcmp(argv[2],"MJPEG"))
    {
        tVideoDevice.iPixelFormat = V4L2_PIX_FMT_MJPEG;
    }
    else if (!strcmp(argv[2],"YUY2"))
    {
        tVideoDevice.iPixelFormat = V4L2_PIX_FMT_YUV420;
    }
    
    tVideoDevice.iWidth = atoi(argv[3]);
    tVideoDevice.iHeight = atoi(argv[4]);
    tVideoDevice.iFps = atoi(argv[5]);
    if (argv[6] == NULL)
    {
        tContextDevice.dstIp = "127.0.0.1";
    }
    else
    {
        tContextDevice.dstIp = argv[6];
    }
    if (argv[6] == NULL)
    {
        tContextDevice.dstPort = 1234;
    }
    else
    {
        tContextDevice.dstPort = atoi(argv[7]);
    }
    tran_init();
	tran_chanel_init(&tContextDevice);
	video_init();
	initRTPMuxContext(&rtpMuxContext);
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
    while (1)
    {
    
        gettimeofday(&startTime,NULL);
        iError = tVideoDevice.ptOPr->GetFrame(&tVideoDevice, &tVideoBuf);
        if (iError)
        {
            printf("GetFrame for %s error!\n", argv[1]);
            return -1;
        }
		iError = tVideoDevice.ptOPr->PutFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("PutFrame for error!\n");
        }
    	while (ptVideoConvert->Convert(&tVideoBuf, &tConvertBuf) > 0) 
    	{       	
            printf("iTotalBytes = %d\n",tConvertBuf.tPixelDatas.iTotalBytes);
            write(fd,tConvertBuf.tPixelDatas.aucPixelDatas,tConvertBuf.tPixelDatas.iTotalBytes);
            rtpSendH264HEVC(&rtpMuxContext, &tContextDevice, tConvertBuf.tPixelDatas.aucPixelDatas, tConvertBuf.tPixelDatas.iTotalBytes);
    	}
        gettimeofday(&endTime,NULL);
    	Timeuse = 1000000*(endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec);
    	Timeuse /= 1000000;
    	printf("Timeuse = %f\n",Timeuse);
    }
	return 0;
}
#endif /* #if 0 */



























