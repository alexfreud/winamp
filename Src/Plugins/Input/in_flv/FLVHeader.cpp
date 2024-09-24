#include "FLVHeader.h"
#include "FLVUtil.h"
/*
(c) 2006 Nullsoft, Inc.
Author: Ben Allison benski@nullsoft.com 
*/

#define FLV_BITMASK_AUDIO 0x4
#define FLV_BITMASK_VIDEO 0x1

/*
FLV Header spec

Signature - uint8[3] - must equal "FLV"
Version   - uint8    - only known version is 1
Flags     - uint8    - bitmask, 4 is audio, 1 is video
Offset    - uint32 	 - total size of header (9), big endian
*/




bool FLVHeader::Read(uint8_t *data, size_t size)
{
	if (size < 9)
		return false; // too small to be an FLV header

	if (data[0] != 'F' || data[1] != 'L' || data[2] != 'V')
		return false; // invalid signature

	version = data[3];

	hasAudio = !!(data[4] & FLV_BITMASK_AUDIO);
	hasVideo = data[4] & FLV_BITMASK_VIDEO;

	headerSize = FLV::Read32(&data[5]);
	if (headerSize != 9)
		return false;

	return true;
}