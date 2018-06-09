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

#define DelayTime 30


int iError;
T_VideoDevice tVideoDevice;
T_VideoBuf tVideoBuf;
T_VideoBuf tConvertBuf;
PT_VideoConvert ptVideoConvert;
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
	    /* 设置相对时间，30ms读取一次摄像头 */
		long long absmsec;
        struct timeval now;  
        struct timespec outtime;  
    	gettimeofday(&now, NULL);
    	absmsec = now.tv_sec * 1000ll + now.tv_usec / 1000ll;
    	absmsec += DelayTime;
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
		    //printf("GetFrame is ok\n");
		}
		
		iError = tVideoDevice.ptOPr->PutFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("PutFrame for error!\n");
        }
        
		pthread_mutex_unlock(&lock);     /*释放互斥锁*/
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
    while (1)
	{
	    pthread_mutex_lock(&lock); /* 获取互斥锁 */
        if(tVideoBuf.tPixelDatas.aucPixelDatas != NULL)
        {
            
            pthread_cond_wait(&captureOK,&lock);
        	if (ptVideoConvert->Convert(&tVideoBuf, &tConvertBuf) > 0) 
        	{
        	    
        	}
        	//printf("Encode_Thread end\n");
    	}
    	pthread_mutex_unlock(&lock);/*释放互斥锁*/
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



int main(int argc, char *argv[])
{
    if (argc != 2)
    {
 		printf("Usage:\n");
		printf("%s </dev/video0,1,...>\n", argv[0]);
		return -1;
    }
    init_fun();
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
	
	thread_create();
    thread_wait();

    ptVideoConvert->ConvertExit(&tConvertBuf);
	return 0;;
}




























