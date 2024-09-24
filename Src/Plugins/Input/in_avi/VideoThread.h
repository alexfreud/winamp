#pragma once
#include <bfc/platform/types.h>
#include "../nsavi/demuxer.h"
extern uint64_t video_total_time;

bool OnVideo(uint16_t type, void *data, size_t length);
void Video_Stop();
void Video_Close();
void Video_Flush();
void Video_Init();
void Video_Break();
bool CreateVideoReaderThread(nsavi::Demuxer *demuxer, nsavi::avi_reader *video_reader);