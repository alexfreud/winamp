#pragma once
#include "bfc/platform/types.h"
#include "avi_reader.h"


namespace nsavi
{
	class avi_reader;
#pragma pack(push, 4)
	struct riff_chunk
	{
		uint32_t id;
		uint32_t size;
		uint32_t type; // if id is LIST or RIFF, this will be set
	};
#pragma pack(pop)

	
	enum ParseState
	{
		NOT_READ = 0,
		PARSED = 1,
		NOT_FOUND = 2,
		PARSE_ERROR = 3,
		FOUND = 4, // we know where it is, but we havn't read it
		PARSE_RESYNC = 5, // read was aborted (return code < 0). need to resync inside the avi_reader

	};

	#define nsaviFOURCC( ch0, ch1, ch2, ch3 )	((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8 ) | ((uint32_t)(uint8_t)(ch2) << 16 ) | ( (uint32_t)(uint8_t)(ch3) << 24 ))

		// negative return codes are 'pass-thru' from the the avi_reader object
	// interpret accordingly (e.g. in_avi might abort a long network i/o on stop or seek)
	int read_riff_chunk(nsavi::avi_reader *reader, riff_chunk *chunk, uint32_t *bytes_read=0);
	int skip_chunk(nsavi::avi_reader *reader, const nsavi::riff_chunk *chunk, uint32_t *out_bytes_read=0);


}