#include "FLVVideoHeader.h"

/*
(c) 2006 Nullsoft, Inc.
Author: Ben Allison benski@nullsoft.com 
*/

/*
codecID  - 4 bits - 2: Sorensen H.263, 3: Screen video, 4: On2 VP6
frameType - 4 bits - 1: keyframe, 2: inter frame, 3: disposable inter frame
*/

bool FLVVideoHeader::Read(unsigned __int8 *data, size_t size)
{
	if (size < 1)
		return false; // header size too small

	unsigned __int8 byte = data[0];

	format =  (byte & 0x0f) >> 0;
	frameType = (byte & 0xf0) >> 4;
	
	return true;
}