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

int iError;
int fd;
T_VideoDevice tVideoDevice;
T_VideoBuf tVideoBuf;
T_VideoBuf tConvertBuf;
PT_VideoConvert ptVideoConvert;
pthread_cond_t captureOK = PTHREAD_COND_INITIALIZER; /*线程采集满一个时的标志*/
pthread_cond_t encodeOK = PTHREAD_COND_INITIALIZER; /*线程编码完一个的标志*/
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;     /*互斥锁*/
pthread_t CaptureThread;
pthread_t EncodetTread;

static void init_fun(void)
{

	pthread_mutex_init(&lock,NULL);     /* 以动态方式创建互斥锁 */
	pthread_cond_init(&captureOK,NULL); /* 初始化captureOK条件变量 */
	pthread_cond_init(&encodeOK,NULL);  /* 初始化encodeOK条件变量 */
	 /* 申请帧内存,和转码帧内存 */
	memset(&tVideoBuf, 0, sizeof(tVideoBuf)); 
	memset(&tConvertBuf, 0, sizeof(tConvertBuf));

}

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
        /* 读入摄像头数据保存到帧内存 */
        /*注意pthread_mutex_lock时以下的资源将被锁定不能访问,所以pthread_mutex_lock不能放在
         *GetFrame的前面
         */
		if(tVideoDevice.ptOPr->GetFrame(&tVideoDevice, &tVideoBuf)==0)//采集一帧数据
		{
		    printf("GetFrame is ok\n");
		}
		iError = tVideoDevice.ptOPr->PutFrame(&tVideoDevice, &tVideoBuf);
        if(iError)
        {
        	printf("PutFrame for error!\n");
        }
        pthread_mutex_lock(&lock); /* 获取互斥锁 */
        pthread_cond_signal(&captureOK); /*设置状态信号*/
		pthread_mutex_unlock(&lock);     /*释放互斥锁*/
	}
}

void *video_Encode_Thread(void *arg)
{
	while(1)
	{	
	    pthread_mutex_lock(&lock); /*获取互斥锁*/
    	pthread_cond_wait(&captureOK,&lock);
    	pthread_mutex_unlock(&lock);/*释放互斥锁*/	
        int h264_length = 0;
    	h264_length = ptVideoConvert->Convert(&tVideoBuf, &tConvertBuf);/*H.264压缩视频*/
    	if (h264_length > 0) 
    	{
    		printf("h264_length = %d\n",h264_length);
    		//写h264文件
            if (write(fd,tConvertBuf.tPixelDatas.aucPixelDatas,tConvertBuf.tPixelDatas.iTotalBytes)!=tConvertBuf.tPixelDatas.iTotalBytes)
            {
               printf("write error\n");
            }
    	}
    	/*H.264压缩视频*/
	}
}


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

   if ((fd = open("test.h264",O_WRONLY|O_CREAT))<0)
    {
        printf("open error:\n");
        return -1;
    }
    thread_create();
    thread_wait();
	return 0;
}




























