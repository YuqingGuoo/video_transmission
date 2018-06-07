
#ifndef _CONVERT_MANAGER_H
#define _CONVERT_MANAGER_H

#include <video_manager.h>
#include <linux/videodev2.h>



typedef struct VideoConvert {
    char *name;
    int (*isSupport)(void);
    int (*Convert)(PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut);
    int (*ConvertExit)(PT_VideoBuf ptVideoBufOut);
    struct VideoConvert *ptNext;
}T_VideoConvert, *PT_VideoConvert;

int video_convert_init(void);
int register_video_convert(PT_VideoConvert ptVideoConvert);
PT_VideoConvert get_video_convert(char *pcName);
void show_video_convert(void);
int yuv_to_h264_init(void);
PT_VideoConvert get_video_convert_for_formats(void);





#endif /* _CONVERT_MANAGER_H */

