#pragma once
#include "mkv_reader.h"
#include <bfc/platform/types.h>
// IDs
// these are slightly different from the matroska spec because we specify
// values after vint decoding and they specify before
const uint32_t mkv_void=0x6C;
const uint32_t mkv_crc=0x3F;

namespace nsmkv
{
	// doesn't really do anything but fseek, but will output unknown values in debug mode
	uint64_t ReadGlobal(nsmkv::MKVReader *reader, uint64_t id, uint64_t size);
	uint64_t SkipNode(nsmkv::MKVReader *reader, uint64_t id, uint64_t size); // same thing as ReadGlobal but doesn't display unknown nodes
}