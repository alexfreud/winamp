#ifndef NULLSOFT_IN_FLV_VIDEOTHREAD_H
#define NULLSOFT_IN_FLV_VIDEOTHREAD_H

#include "../nsv/dec_if.h"
#include "../Winamp/wa_ipc.h"
#include "svc_flvdecoder.h"
#include "ifc_flvvideodecoder.h"

bool OnVideo(void *data, size_t length, int type, unsigned __int32 timestamp);
void VideoFlush();
extern int width, height;
extern IVideoOutput *videoOutput;
extern bool video_opened;

void Video_Init();
void Video_Stop();
void Video_Close();
bool Video_IsSupported(int type);
bool Video_DecoderReady();

#endif