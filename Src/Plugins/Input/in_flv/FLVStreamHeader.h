#ifndef NULLSOFT_FLVSTREAMHEADER_H
#define NULLSOFT_FLVSTREAMHEADER_H

#include <bfc/platform/types.h>
namespace FLV
{
 enum
 {
	 FRAME_TYPE_AUDIO = 0x8,
	 FRAME_TYPE_VIDEO = 0x9,
	 FRAME_TYPE_METADATA = 0x12,
 };
}
class FLVStreamHeader
{
public:
	bool Read(unsigned __int8 *data, size_t size); // size must be >=15, returns "true" if this was a valid header

	// attributes, consider these read-only
	uint32_t previousSize;
	uint8_t type;
	uint32_t dataSize;
	uint32_t timestamp;
	uint32_t streamID;
};

#endif