#pragma once
#include "read.h"
#include "avi_header.h"
#include "avi_reader.h"
#include "info.h"

namespace nsavi
{


	struct HeaderList
	{
		const AVIH *avi_header;
		const STRL *stream_list;
		size_t stream_list_size;
		const DMLH *odml_header;
	};

	class ParserBase
	{
	public:
		ParserBase(nsavi::avi_reader *_reader);
		int GetRIFFType(uint32_t *type);
		

	protected:
		int ParseHeaderList(uint32_t chunk_size, uint32_t *out_bytes_read);
		int ParseStreamList(uint32_t chunk_size, STRL *stream, uint32_t *out_bytes_read);
		int ParseODML(uint32_t chunk_size, uint32_t *out_bytes_read);

		nsavi::avi_reader *reader;

		/* RIFF header (12 bytes at start of file) */
		ParseState riff_parsed;
		riff_chunk riff_header;
		uint64_t riff_start; // should normally be 12

		/* header list */
		ParseState header_list_parsed;
		AVIH *avi_header;
		STRL *stream_list;
		size_t stream_list_size;
		DMLH *odml_header;
	};
}