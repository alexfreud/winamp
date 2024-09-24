#pragma once

#include <bfc/platform/types.h>
#include <stdio.h>

// IDs
// these are slightly different from the matroska spec because we specify
// values after vint decoding and they specify before
const uint32_t mkv_segment_chapters = 0x43a770;

namespace nsmkv
{
	class Chapters
	{
	public:
		Chapters(void);
		~Chapters(void);
	};

	// returns bytes read.  0 means EOF
	uint64_t ReadChaptersInfo(FILE *f, uint64_t size, nsmkv::Chapters &chapters);
};
