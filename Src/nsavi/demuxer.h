#pragma once
/* this parser is meant for actual playback */
#include "read.h"
#include "avi_header.h"
#include "avi_reader.h"
#include "info.h"
#include "ParserBase.h"
namespace nsavi
{
	class Demuxer : public ParserBase
	{
	public:
		Demuxer(nsavi::avi_reader *_reader);
		int GetNextMovieChunk(nsavi::avi_reader *reader, void **data, uint32_t *chunk_size, uint32_t *chunk_type, int limit_stream_num=65536);
		int GetSeekTable(nsavi::IDX1 **out_index); // get the idx1 chunk
		int GetIndexChunk(nsavi::INDX **out_index, uint64_t offset); // get the INDX/##ix/##ix chunk at the given position
		int Seek(uint64_t offset, bool absolute, nsavi::avi_reader *reader);
		int GetHeaderList(HeaderList *header_list);
		int FindMovieChunk();
		int SeekToMovieChunk(nsavi::avi_reader *reader);
	private:
		/* movie chunk */
		ParseState movie_found;
		riff_chunk movi_header;
		uint64_t movie_start;

		/* idx1 seektable */
		ParseState idx1_found;
		riff_chunk idx1_header; // dunno if we really need it
		nsavi::IDX1 *index;

		/* INFO */
		Info *info;
		ParseState info_found;
	};
}