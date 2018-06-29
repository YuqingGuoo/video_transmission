//
// Created by Liming Shao on 2018/5/10.
//

#ifndef RTPSERVER_AVC_H
#define RTPSERVER_AVC_H

#include <stdint.h>

/* copy from FFmpeg libavformat/acv.c */
const unsigned char *ff_avc_find_startcode(const unsigned char *p, const unsigned char *end);

#endif //RTPSERVER_AVC_H
