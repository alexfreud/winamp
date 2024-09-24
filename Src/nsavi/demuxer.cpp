#include "demuxer.h"
#include "read.h"
#include "avi_reader.h"

static int GetStreamNumber(uint32_t id)
{
	char *stream_data = (char *)(&id); 
	if (!isxdigit(stream_data[0]) || !isxdigit(stream_data[1]))
		return -1;

	stream_data[2] = 0;
	int stream_number = strtoul(stream_data, 0, 16);
	return stream_number;
}

nsavi::Demuxer::Demuxer(nsavi::avi_reader *_reader) : ParserBase(_reader)
{
	movie_found = NOT_READ;
	idx1_found = NOT_READ;
	info_found = NOT_READ;
	movie_start = 0;
	index = 0;
	info = 0;
}

// reads a chunk and updates parse state variable on error
static int ReadChunk(nsavi::avi_reader *reader, nsavi::riff_chunk *chunk, nsavi::ParseState &state, uint32_t *bytes_read)
{
	int ret = nsavi::read_riff_chunk(reader, chunk, bytes_read);
	if (ret == nsavi::READ_EOF)
	{
		state = nsavi::NOT_FOUND;
		return nsavi::READ_NOT_FOUND;
	}
	else if (ret > nsavi::READ_OK)
	{
		state = nsavi::PARSE_ERROR;
		return ret;
	}
	else if (ret < nsavi::READ_OK)
	{ // pass-thru return value from avi_reader
		state = nsavi::PARSE_RESYNC;
		return ret;
	}

	return nsavi::READ_OK;
}

// skips a chunk and updates a parser state variable on error
static int SkipChunk(nsavi::avi_reader *reader, const nsavi::riff_chunk *chunk, nsavi::ParseState &state, uint32_t *bytes_read)
{
	int ret = nsavi::skip_chunk(reader, chunk, bytes_read);
	if (ret == nsavi::READ_EOF)
	{
		state = nsavi::NOT_FOUND;
		return nsavi::READ_NOT_FOUND;
	}
	else if (ret > nsavi::READ_OK)
	{
		state = nsavi::PARSE_ERROR;
		return ret;
	}
	else if (ret < nsavi::READ_OK)
	{ // pass-thru return value from avi_reader
		state = nsavi::PARSE_RESYNC;
		return ret;
	}

	return nsavi::READ_OK;
}

static int Read(nsavi::avi_reader *reader, void *buffer, uint32_t size, nsavi::ParseState &state, uint32_t *out_bytes_read)
{
	uint32_t bytes_read;
	int ret = reader->Read(buffer, size, &bytes_read);
	if (ret > nsavi::READ_OK)
	{
		state = nsavi::PARSE_ERROR;
		return ret;
	}
	else if (ret < nsavi::READ_OK)
	{ // pass-thru return value from avi_reader
		state = nsavi::PARSE_RESYNC;
		return ret;
	}
	else if (bytes_read != size)
	{
		state = nsavi::PARSE_ERROR;
		return nsavi::READ_EOF;
	}
	*out_bytes_read = bytes_read;
	return nsavi::READ_OK;
}

int nsavi::Demuxer::GetHeaderList(HeaderList *header_list)
{
	if (riff_parsed != PARSED)
		return READ_INVALID_CALL;

	if (riff_parsed == PARSE_RESYNC)
		reader->Seek(riff_start);

	if (header_list_parsed == NOT_READ)
	{
		// first, see how far we are into the file to properly bound our reads
		uint64_t start = reader->Tell();
		uint32_t bytes_available = riff_header.size;
		bytes_available -= (uint32_t)(start - riff_start);

		while (bytes_available)
		{
			if (bytes_available < 8)
			{
				header_list_parsed = NOT_FOUND;
				return READ_NOT_FOUND;
			}
			uint32_t bytes_read;
			riff_chunk chunk;
			int ret = ReadChunk(reader, &chunk, header_list_parsed, &bytes_read);
			if (ret)
				return ret;

			bytes_available -= bytes_read;
			if (bytes_available < chunk.size)
			{
				header_list_parsed = PARSE_ERROR;
				return READ_INVALID_DATA;
			}
			switch(chunk.id)
			{
			case 'TSIL': // list chunk
				switch(chunk.type)
				{
				case 'lrdh': // this is what we're looking for
					ret = ParseHeaderList(chunk.size, &bytes_read);
					if (ret == READ_OK)
					{
						header_list->avi_header = avi_header;
						header_list->stream_list = stream_list;
						header_list->stream_list_size = stream_list_size;
						header_list->odml_header = odml_header;
					}
					return ret;
				case 'OFNI': // INFO
					if (!info)
					{
						info = new nsavi::Info();
						if (!info)
						{
							header_list_parsed = PARSE_ERROR;
							return READ_OUT_OF_MEMORY;
						}
						ret = info->Read(reader, chunk.size);
						if (ret)
						{
							header_list_parsed = PARSE_ERROR;
							return ret;
						}
						break;
					}
					// fall through
				default: // skip anything we don't understand
					ret = SkipChunk(reader, &chunk, header_list_parsed, &bytes_read);
					if (ret)
						return ret;
					bytes_available -= bytes_read;
					break;
				}

				break;
			default: // skip anything we don't understand
			case 'KNUJ': // skip junk chunks
				ret = SkipChunk(reader, &chunk, header_list_parsed, &bytes_read);
				if (ret)
					return ret;
				bytes_available -= bytes_read;
				break;
				// TODO; case '1xdi': break;
			}
		}
	}


	if (header_list_parsed == PARSED)
	{
		header_list->avi_header = avi_header;
		header_list->stream_list = stream_list;
		header_list->stream_list_size = stream_list_size;
		header_list->odml_header = odml_header;
		return READ_OK;
	}

	return READ_INVALID_CALL;
}

int nsavi::Demuxer::FindMovieChunk()
{
	if (riff_parsed != PARSED)
		return READ_INVALID_CALL;

	if (header_list_parsed != READ_OK)
		return READ_INVALID_CALL;

	if (movie_found == PARSED)
		return READ_OK;

	if (movie_found == NOT_READ)
	{
		// first, see how far we are into the file to properly bound our reads
		uint64_t start = reader->Tell();
		uint32_t bytes_available = riff_header.size;
		bytes_available -= (uint32_t)(start - riff_start);
		while (movie_found == NOT_READ)
		{
			if (bytes_available < 8)
			{
				header_list_parsed = NOT_FOUND;
				return READ_NOT_FOUND;
			}
			uint32_t bytes_read;
			int ret = ReadChunk(reader, &movi_header, movie_found, &bytes_read);
			if (ret)
				return ret;

			bytes_available -= bytes_read;
			if (bytes_available < movi_header.size)
			{
				movie_found = PARSE_ERROR;
				return READ_INVALID_DATA;
			}
			switch(movi_header.id)
			{
				// TODO: parse any other interesting chunks along the way
			case 'TSIL': // list chunk
				switch(movi_header.type)
				{
				case 'ivom':
					{
						movie_found = PARSED;
						movie_start = reader->Tell();
						return READ_OK;
					}
					break;
				case '1xdi': // index v1 chunk
					if (!index)
					{
						index = (nsavi::IDX1 *)malloc(idx1_header.size + sizeof(uint32_t));
						if (index)
						{
							ret = Read(reader, ((uint8_t *)index) + sizeof(uint32_t), idx1_header.size, idx1_found, &bytes_read);
							if (ret)
								return ret;

							bytes_available-=bytes_read;
							index->index_count = idx1_header.size / sizeof(IDX1_INDEX);
							if ((idx1_header.size & 1) && bytes_available)
							{
								bytes_available--;
								reader->Skip(1);
							}
							idx1_found = PARSED;
						}
						else
						{
							return READ_OUT_OF_MEMORY;
						}
					}
					else
					{
						ret = SkipChunk(reader, &movi_header, movie_found, &bytes_read);
						if (ret)
							return ret;
						bytes_available -= bytes_read;
					}
					break;
				case 'OFNI': // INFO
					if (!info)
					{
									info = new nsavi::Info();
						if (!info)
						{
							movie_found = PARSE_ERROR;
							return READ_OUT_OF_MEMORY;
						}

						ret = info->Read(reader, movi_header.size);
						if (ret)
						{
							movie_found = PARSE_ERROR;
							return ret;
						}
						break;
					}
					// fall through
				default: // skip anything we don't understand
					ret = SkipChunk(reader, &movi_header, movie_found, &bytes_read);
					if (ret)
						return ret;
					bytes_available -= bytes_read;
					break;
				}
				break;

			default: // skip anything we don't understand
			case 'KNUJ': // skip junk chunks
				ret = SkipChunk(reader, &movi_header, movie_found, &bytes_read);
				if (ret)
					return ret;
				bytes_available -= bytes_read;
				break;
			}
		}
	}
	return nsavi::READ_NOT_FOUND; // TODO: not sure about this
}
int nsavi::Demuxer::SeekToMovieChunk(nsavi::avi_reader *reader)
{
	return reader->Seek(movie_start);
}

int nsavi::Demuxer::GetNextMovieChunk(nsavi::avi_reader *reader, void **data, uint32_t *chunk_size, uint32_t *chunk_type, int limit_stream_num)
{
		ParseState no_state;
	if (movie_found == PARSED)
	{
		uint64_t start = reader->Tell();
		uint32_t bytes_available = movi_header.size;
		bytes_available -= (uint32_t)(start - movie_start);

		uint32_t bytes_read;
		riff_chunk chunk;
again:
		int ret = ReadChunk(reader, &chunk, no_state, &bytes_read);
		if (ret)
			return ret;

		if (chunk.id == 'TSIL' || chunk.id == 'FFIR')
		{
			goto again; // skip 'rec' chunk headers
		}
		if (chunk.id == 'KNUJ' || chunk.id == '1xdi') 
		{
			SkipChunk(reader, &chunk, no_state, &bytes_read);
			goto again;

		}
		if (limit_stream_num != 65536)
		{
			if (limit_stream_num != GetStreamNumber(chunk.id))
			{
				SkipChunk(reader, &chunk, no_state, &bytes_read);
				goto again;
			}
		}

		*data = malloc(chunk.size);
		if (!*data)
			return READ_OUT_OF_MEMORY;
		*chunk_size = chunk.size;
		*chunk_type = chunk.id;


		ret = Read(reader, *data, chunk.size, no_state, &bytes_read);
		if (ret)
			return ret;

		if ((chunk.size & 1))
		{
			bytes_available--;
			reader->Skip(1);
		}
		return READ_OK;
	}
	else
		return READ_FAILED;

}

int nsavi::Demuxer::GetSeekTable(nsavi::IDX1 **out_index)
{
	if (idx1_found == PARSED)
	{
		*out_index = index;
		return READ_OK;
	}

	if (idx1_found == NOT_FOUND)
	{
		return READ_NOT_FOUND;
	}

	if (idx1_found != NOT_READ)
		return READ_FAILED;

	uint64_t old_position = reader->Tell();

	if (movie_found == PARSED)
		reader->Seek(movie_start+movi_header.size);
	else
		reader->Seek(riff_start);

	uint64_t start = reader->Tell();
	uint32_t bytes_available = riff_header.size;
	bytes_available -= (uint32_t)(start - riff_start);

	while (idx1_found == NOT_READ)
	{
		if (bytes_available < 8)
		{
			idx1_found = NOT_FOUND;
			reader->Seek(old_position);
			return READ_NOT_FOUND;
		}
		uint32_t bytes_read;
		int ret = ReadChunk(reader, &idx1_header, idx1_found, &bytes_read);
		if (ret)
			return ret;

		bytes_available -= bytes_read;
		if (bytes_available == (idx1_header.size - 12)) // some stupid program has this bug
		{
			idx1_header.size-=12;
		}
		if (bytes_available < idx1_header.size)
		{
			idx1_found = PARSE_ERROR;
			reader->Seek(old_position);
			return READ_INVALID_DATA;
		}
		switch(idx1_header.id)
		{
			// TODO: parse any other interesting chunks along the way
		case '1xdi': // index v1 chunk
			index = (nsavi::IDX1 *)malloc(idx1_header.size + sizeof(uint32_t));
			if (index)
			{
				ret = Read(reader, ((uint8_t *)index) + sizeof(uint32_t), idx1_header.size, idx1_found, &bytes_read);
				if (ret)
				{
					reader->Seek(old_position);
					return ret;
				}

				bytes_available-=bytes_read;
				index->index_count = idx1_header.size / sizeof(IDX1_INDEX);
				if ((idx1_header.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
				idx1_found = PARSED;
			}
			else
			{
				reader->Seek(old_position);
				return READ_OUT_OF_MEMORY;
			}

			break;
		default: // skip anything we don't understand
		case 'KNUJ': // skip junk chunks
			ret = SkipChunk(reader, &idx1_header, idx1_found, &bytes_read);
			if (ret)
				return ret;
			bytes_available -= bytes_read;
			break;
		}
	}

	*out_index = index;
	reader->Seek(old_position);
	return READ_OK;
}

int nsavi::Demuxer::GetIndexChunk(nsavi::INDX **out_index, uint64_t offset)
{
	nsavi::INDX *index = 0;
	uint64_t old_position = reader->Tell();
	reader->Seek(offset);
	ParseState dummy;
	uint32_t bytes_read;
	riff_chunk chunk;
	int ret = ReadChunk(reader, &chunk, dummy, &bytes_read);
	if (ret)
		return ret;
	index = (nsavi::INDX *)malloc(sizeof(uint32_t) + chunk.size);
	if (index)
	{
		ret = Read(reader, ((uint8_t *)index) + sizeof(uint32_t), chunk.size, dummy, &bytes_read);
		if (ret)
		{
			reader->Seek(old_position);
			return ret;
		}
		index->size_bytes=chunk.size;
	}
	else
	{
		reader->Seek(old_position);
		return READ_OUT_OF_MEMORY;
	}

	*out_index = index;
	reader->Seek(old_position);
	return READ_OK;
}

static bool IsCodecChunk(uint32_t header)
{
	char *blah = (char *)&header;
	if (blah[0] != 'i' && !isxdigit(blah[0]))
		return false;
	if (blah[1] != 'x' && !isxdigit(blah[1]))
		return false;

	return true;
}

int nsavi::Demuxer::Seek(uint64_t offset, bool absolute, nsavi::avi_reader *reader)
{
	/* verify index by reading the riff chunk and comparing position->chunk_id and position->size with the read chunk
	if it fails, we'll try the two following things
	1) try again without the -4
	2) try from the start of the file
	3) try from riff_start
	*/
	uint32_t bytes_read;
	uint32_t chunk_header=0;
	if (!reader)
		reader = this->reader;
	if (absolute)
	{
		reader->Seek(offset - 8);
		reader->Peek(&chunk_header, 4, &bytes_read);
		if (!IsCodecChunk(chunk_header))
		{
			reader->Skip(4);
			reader->Peek(&chunk_header, 4, &bytes_read);
			if (!IsCodecChunk(chunk_header))
			{
				reader->Skip(4);
			}
		}
	}
	else
	{
		reader->Seek(movie_start+offset - 4);
		reader->Peek(&chunk_header, 4, &bytes_read);
		if (!IsCodecChunk(chunk_header))
		{
			reader->Seek(offset);
		}
	}


	
	/*
	riff_chunk test;
	ParseState blah;
	uint32_t bytes_read;
	ReadChunk(f, &test, blah, &bytes_read);
	fseek64(f, movie_start+position->offset - 4, SEEK_SET);
	*/
	return READ_OK;
}
