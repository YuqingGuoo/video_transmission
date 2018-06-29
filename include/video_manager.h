#ifndef _VIDEO_MANAGER_H
#define _VIDEO_MANAGER_H

#include <linux/videodev2.h>

#define NB_BUFFER 4
#define WIDTH 640
#define HEIGHT 480
//#define WIDTH 1280
//#define HEIGHT 960
//#define WIDTH 1920
//#define HEIGHT 1080



struct VideoOpr;
typedef struct VideoOpr T_VideoOpr, *PT_VideoOpr;

typedef struct VideoDevice
{
    int iFd;
    int iPixelFormat; //像素格式
    int iWidth;
    int iHeight;
    int iFps;         //帧率
    int iVideoBufCnt; //视频数据长度
    int iVideoBufMaxLen;
    int iVideoBufCurIndex;
    unsigned char *pucVideBuf[NB_BUFFER];
    /* 函数 */
    PT_VideoOpr ptOPr;
}T_VideoDevice,*PT_VideoDevice;


typedef struct PixelDatas
{
    int iWidth;                   /* 宽度: 一行有多少个象素 */
    int iHeight;                  /* 高度: 一列有多少个象素 */
    int iBpp;                     /* 一个象素用多少位来表示 */
    int iLineBytes;               /* 一行数据有多少字节 */
    int iTotalBytes;              /* 所有字节数 */
    unsigned char *aucPixelDatas; /* 象素数据存储的地方 */
}T_PixelDatas, *PT_PixelDatas;

typedef struct VideoBuf
{
    T_PixelDatas tPixelDatas;
    int iPixelFormat;
}T_VideoBuf, *PT_VideoBuf;

struct VideoOpr
{
    char *name;
    int (*InitDevice)(char *strDevName, PT_VideoDevice ptVideoDevice);
    int (*ExitDevice)(PT_VideoDevice ptVideoDevice);
    int (*GetFrame)(PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf);
    int (*GetFormat)(PT_VideoDevice ptVideoDevice);
    int (*PutFrame)(PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf);
    int (*StartDevice)(PT_VideoDevice ptVideoDevice);
    int (*StopDevice)(PT_VideoDevice ptVideoDevice);
    struct VideoOpr *ptNext;
};

int register_video_opr(PT_VideoOpr ptVideoOpr);
void show_video_opr(void);
PT_VideoOpr get_video_opr(char *pcName);
int video_device_init(char *strDevName, PT_VideoDevice ptVideoDevice);
int video_init(void);
int V4l2Init(void);



#endif
