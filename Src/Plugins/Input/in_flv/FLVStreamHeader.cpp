#include "FLVStreamHeader.h"
#include "FLVUtil.h"
/*
(c) 2006 Nullsoft, Inc.
Author: Ben Allison benski@nullsoft.com 
*/

/*
PreviousTagSize  - uint32   - total size of previous tag, presumably for stream continuity checking.  big endian
Type             - uint8    - what does this frame contain?  0x12=meta, 0x8=audio, 0x9=video
BodyLength       - uint24   - size of the data following this header.  big endian
Timestamp        - uint24   - timestamp (milliseconds).  big endian
Timestamp High   - uint8    - high 8 bits of timestamp (to form 32bit timestamp)
Stream ID        - uint24   - always zero
*/

bool FLVStreamHeader::Read(uint8_t *data, size_t size)
{
	if (size < 15)
		return false; // header size too small

	previousSize = FLV::Read32(&data[0]);
	type = data[4];
	dataSize = FLV::Read24(&data[5]);
	timestamp = FLV::Read24(&data[8]);
	uint8_t timestampHigh = FLV::Read8(&data[11]);
	timestamp |= (timestampHigh << 24);
	streamID = FLV::Read24(&data[12]);

	if (streamID != 0)
		return false;

	return true;


}