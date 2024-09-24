#ifndef _MEMINPUT_H
#define _MEMINPUT_H
#pragma once

#include "nalucommon.h"
#include <bfc/platform/types.h>

typedef struct memory_input_struct 
{
	const uint8_t *user_buffer;
	size_t user_buffer_size;
	size_t user_buffer_read;

  uint8_t *Buf;
	int resetting;
	int skip_b_frames;
} memory_input_t;

int  GetMemoryNALU (VideoParameters *p_Vid, NALU_t *nalu);
void OpenMemory(VideoParameters *p_Vid, const char *fn);
void CloseMemory(VideoParameters *p_Vid);
void malloc_mem_input(VideoParameters *p_Vid);
void free_mem_input(VideoParameters *p_Vid);

#endif

