#pragma once

#include <bfc/platform/types.h>
#include <stdio.h>

// IDs
// these are slightly different from the matroska spec because we specify
// values after vint decoding and they specify before
const uint32_t mkv_segment_tags = 0x254c367;
const uint32_t mkv_tags_tag = 0x3373;
const uint32_t mkv_tags_tag_target = 0x23c0;
const uint32_t mkv_tags_tag_target_targettypevalue = 0x28ca;
const uint32_t mkv_tags_tag_simpletag = 0x27c8;
const uint32_t mkv_tags_tag_simpletag_tagname = 0x5a3;
const uint32_t mkv_tags_tag_simpletag_tagstring = 0x487;
const uint32_t mkv_tags_tag_simpletag_taglanguage = 0x47a;
const uint32_t mkv_tags_tag_simpletag_tagdefault = 0x484;

namespace nsmkv
{

	class Tags
	{
	public:
		Tags(void);
		~Tags(void);
	};

	// returns bytes read.  0 means EOF
	uint64_t ReadTags(FILE *f, uint64_t size, nsmkv::Tags &tags);

};
