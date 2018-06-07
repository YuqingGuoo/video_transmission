#ifndef _VIDEO_MANAGER_H
#define _VIDEO_MANAGER_H

#define NB_BUFFER 4

struct VideoDevice;
struct VideoOpr;
typedef struct VideoDevice T_VideoDevice, *PT_VideoDevice;
typedef struct VideoOpr T_VideoOpr, *PT_VideoOpr;


struct VideoDevice
{
    int iFd;
    int iPixelFormat; //像素格式
    int iWidth;
    int iHeight;
    int iVideoBufCnt; //视频数据长度
    int iVideoBufMaxLen;
    int iVideoBufCurIndex;
    unsigned char *pucVideBuf[NB_BUFFER];
    /* 函数 */
    PT_VideoOpr ptOPr;
}T_VideoDevice,*PT_VideoDevice;

struct PixelDatas
{
    int iWidth;                   /* 宽度: 一行有多少个象素 */
    int iHeight;                  /* 高度: 一列有多少个象素 */
    int iBpp;                     /* 一个象素用多少位来表示 */
    int iLineBytes;               /* 一行数据有多少字节 */
    int iTotalBytes;              /* 所有字节数 */
    unsigned char *aucPixelDatas; /* 象素数据存储的地方 */
}T_PixelDatas, *PT_PixelDatas;

struct VideoBuf
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
}T_VideoOpr, *PT_VideoOpr;

int register_video_opr(PT_VideoOpr ptVideoOpr);
void show_video_opr(void);
PT_VideoOpr get_video_opr(char *pcName);
int video_device_init(char *strDevName, PT_VideoDevice ptVideoDevice);

#endif
