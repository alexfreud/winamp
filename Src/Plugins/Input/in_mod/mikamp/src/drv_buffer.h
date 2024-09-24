#pragma once
#include "mikmod.h"
struct DecodeInfo
{
	uint    mode;
	uint    mixspeed;
	uint    channels;

	void *buffer;
	size_t buffersize;

	int bits;
	int frame_size; // cached channels*bits/8
	int error;
	size_t bytesWritten;
};

//extern MD_DEVICE drv_buffer;
