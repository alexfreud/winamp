#pragma once
#include "AudioSample.h"
#include "../Winamp/wa_ipc.h"
void Video_Init();
void Video_Close();

extern IVideoOutput *videoOutput;
extern VideoSample *video_sample;
extern HANDLE video_start_flushing, video_flush, video_flush_done, video_resume;
extern MP4SampleId nextVideoSampleId; // set in conjunction with video_flush