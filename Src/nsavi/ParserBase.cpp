#include "ParserBase.h"

nsavi::ParserBase::ParserBase(nsavi::avi_reader *_reader)
{
	reader = _reader;
	riff_parsed = NOT_READ;
	header_list_parsed = NOT_READ;
	riff_start = 0;
	avi_header = 0;
	stream_list = 0;
	stream_list_size = 0;
	odml_header = 0;
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

int nsavi::ParserBase::GetRIFFType(uint32_t *type)
{
	if (riff_parsed == PARSE_RESYNC)
		reader->Seek(0); // seek to the beginning if we need to

	if (riff_parsed == NOT_READ)
	{
		uint32_t bytes_read;
		// assume we are at the beginning of the file
		int ret = ReadChunk(reader, &riff_header, riff_parsed, &bytes_read);
		if (ret)
			return ret;

		if (!riff_header.type)
		{
			riff_parsed = PARSE_ERROR;
			return READ_INVALID_DATA;
		}

		riff_start = reader->Tell();
		riff_parsed = PARSED;
	}

	if (riff_parsed == PARSED)
	{
		*type = riff_header.type;
		return READ_OK;
	}

	// we'll only get here if GetRIFFType was called a second time after an initial failure
	return READ_INVALID_CALL;
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


int nsavi::ParserBase::ParseStreamList(uint32_t chunk_size, STRL *stream, uint32_t *out_bytes_read)
{
	uint32_t bytes_available = chunk_size;
	uint32_t stream_number = 0;
	while (bytes_available)
	{
		if (bytes_available < 8)
		{
			header_list_parsed = PARSE_ERROR;
			return READ_INVALID_DATA;
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
		case 'hrts': // strh

			free(stream->stream_header);
			stream->stream_header = (nsavi::STRH *)malloc(chunk.size + sizeof(uint32_t));
			if (stream->stream_header)
			{
				ret = Read(reader, ((uint8_t *)stream->stream_header) + sizeof(uint32_t), chunk.size, header_list_parsed, &bytes_read);
				if (ret)
					return ret;
				bytes_available-=bytes_read;
				stream->stream_header->size_bytes = chunk.size;
				if ((chunk.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
			}
			else
			{
				return READ_OUT_OF_MEMORY;
			}
			break;
		case 'frts': // strf
			free(stream->stream_format);
			stream->stream_format = (nsavi::STRF *)malloc(chunk.size + sizeof(uint32_t));
			if (stream->stream_format)
			{
				ret = Read(reader, ((uint8_t *)stream->stream_format) + sizeof(uint32_t), chunk.size, header_list_parsed, &bytes_read);
				if (ret)
					return ret;
				bytes_available-=bytes_read;
				stream->stream_format->size_bytes = chunk.size;
				if ((chunk.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
			}
			else
			{
				return READ_OUT_OF_MEMORY;
			}
			break;
		case 'drts': // strd
			free(stream->stream_data);
			stream->stream_data = (nsavi::STRD *)malloc(chunk.size + sizeof(uint32_t));
			if (stream->stream_data)
			{
				ret = Read(reader, ((uint8_t *)stream->stream_data) + sizeof(uint32_t), chunk.size, header_list_parsed, &bytes_read);
				if (ret)
					return ret;
				bytes_available-=bytes_read;
				stream->stream_data->size_bytes = chunk.size;
				if ((chunk.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
			}
			else
			{
				return READ_OUT_OF_MEMORY;
			}
			break;
		case 'nrts': // strn
			free(stream->stream_name);
			stream->stream_name = (nsavi::STRN *)malloc(chunk.size + sizeof(uint32_t));
			if (stream->stream_name)
			{
				ret = Read(reader, ((uint8_t *)stream->stream_name) + sizeof(uint32_t), chunk.size, header_list_parsed, &bytes_read);
				if (ret)
					return ret;
				bytes_available-=bytes_read;
				stream->stream_name->size_bytes = chunk.size;
				if ((chunk.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
			}
			else
			{
				return READ_OUT_OF_MEMORY;
			}
			break;
		case 'xdni': // indx
			free(stream->stream_index);
			stream->stream_index = (nsavi::INDX *)malloc(chunk.size + sizeof(uint32_t));
			if (stream->stream_index)
			{
				ret = Read(reader, &stream->stream_index->entry_size, chunk.size, header_list_parsed, &bytes_read);
				if (ret)
					return ret;
				bytes_available-=bytes_read;
				stream->stream_index->size_bytes = chunk.size;
				if ((chunk.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
			}
			else
			{
				return READ_OUT_OF_MEMORY;
			}
			break;
		case nsaviFOURCC('v','p','r','p'):
			free(stream->video_properties);
			stream->video_properties = (nsavi::VPRP *)malloc(chunk.size + sizeof(uint32_t));
			if (stream->video_properties)
			{
				ret = Read(reader, &stream->video_properties->video_format_token, chunk.size, header_list_parsed, &bytes_read);
				if (ret)
					return ret;
				bytes_available-=bytes_read;
				stream->video_properties->size_bytes = chunk.size;
				if ((chunk.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
			}
			else
			{
				return READ_OUT_OF_MEMORY;
			}
			break;
		default:
			ret = SkipChunk(reader, &chunk, header_list_parsed, &bytes_read);
			if (ret)
				return ret;
			bytes_available -= bytes_read;
			break;
		}
	}

	if ((chunk_size & 1) && bytes_available)
	{
		bytes_available--;
		reader->Skip(1);
	}
	*out_bytes_read = chunk_size - bytes_available;

	// TODO: see what we managed to collect and return an error code accordingly
	return READ_OK;
}

int nsavi::ParserBase::ParseODML(uint32_t chunk_size, uint32_t *out_bytes_read)
{
	uint32_t bytes_available = chunk_size;
	uint32_t stream_number = 0;
	while (bytes_available)
	{
		if (bytes_available < 8)
		{
			header_list_parsed = PARSE_ERROR;
			return READ_INVALID_DATA;
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
		case 'hlmd': // dmlh

			free(odml_header);
			odml_header = (nsavi::DMLH *)malloc(chunk.size + sizeof(uint32_t));
			if (odml_header)
			{
				ret = Read(reader, ((uint8_t *)odml_header) + sizeof(uint32_t), chunk.size, header_list_parsed, &bytes_read);
				if (ret)
					return ret;

				bytes_available-=bytes_read;
				odml_header->size_bytes = chunk.size;
				if ((chunk.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
			}
			else
			{
				return READ_OUT_OF_MEMORY;
			}
			break;
		default:
			ret = SkipChunk(reader, &chunk, header_list_parsed, &bytes_read);
			if (ret)
				return ret;
			bytes_available -= bytes_read;
			break;
		}
	}

	if ((chunk_size & 1) && bytes_available)
	{
		bytes_available--;
		reader->Skip(1);
	}
	*out_bytes_read = chunk_size - bytes_available;

	// TODO: see what we managed to collect and return an error code accordingly
	return READ_OK;
}

int nsavi::ParserBase::ParseHeaderList(uint32_t chunk_size, uint32_t *out_bytes_read)
{
	chunk_size = (chunk_size+1) & ~1;
	uint32_t bytes_available = chunk_size;
	uint32_t stream_number = 0;
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
		case 'hiva': // avih
			free(avi_header);
			avi_header = (nsavi::AVIH *)malloc(chunk.size + sizeof(uint32_t));
			if (avi_header)
			{
				ret = Read(reader, ((uint8_t *)avi_header) + sizeof(uint32_t), chunk.size, header_list_parsed, &bytes_read);
				if (ret)
					return ret;

				bytes_available-=bytes_read;
				avi_header->size_bytes = chunk.size;
				if ((chunk.size & 1) && bytes_available)
				{
					bytes_available--;
					reader->Skip(1);
				}
				if (avi_header->streams && !stream_list)
				{
					// if we fail to allocate, no major worry (maybe avi_header->streams was incorrect and some huge value
					// we'll just dynamically allocate as needed

					stream_list_size = 0;
					if (avi_header->streams < 65536) /* set a reasonable upper bound */
					{
						stream_list = (STRL *)calloc(avi_header->streams, sizeof(STRL));
						if (stream_list)
						{
							stream_list_size = avi_header->streams;
						}
					}
				}
			}
			else
			{
				header_list_parsed = PARSE_ERROR;
				return READ_OUT_OF_MEMORY;
			}
			break;
		case 'TSIL':
			switch(chunk.type)
			{
			case 'lrts':
				{
					if (stream_list_size <= stream_number)
					{
						stream_list = (STRL *)realloc(stream_list, (stream_number+1) * sizeof(STRL));
						if (!stream_list)
						{
							header_list_parsed = PARSE_ERROR;
							return READ_OUT_OF_MEMORY;
						}
						stream_list_size = stream_number+1;
					}

					STRL &stream = stream_list[stream_number];
					memset(&stream, 0, sizeof(STRL));
					ret = ParseStreamList(chunk.size, &stream, &bytes_read);
					if (ret)
						return ret;
					stream_number++;
					bytes_available-=bytes_read;
					if ((chunk.size & 1) && bytes_available)
					{
						bytes_available--;
						reader->Skip(1);
					}
				}
				break;
			case 'lmdo':
				ret = ParseODML(chunk.size, &bytes_read);
				if (ret)
					return ret;
				bytes_available -= bytes_read;
				break;
			default:
				ret = SkipChunk(reader, &chunk, header_list_parsed, &bytes_read);
				if (ret)
					return ret;
				bytes_available -= bytes_read;
				break;
			}
			break;

		default:
			ret = SkipChunk(reader, &chunk, header_list_parsed, &bytes_read);
			if (ret)
				return ret;
			bytes_available -= bytes_read;
			break;
		}
	}


	if ((chunk_size & 1) && bytes_available)
	{
		bytes_available--;
		reader->Skip(1);
	}
	stream_list_size = stream_number;
	*out_bytes_read = chunk_size - bytes_available;
	return READ_OK;
	// TODO: see what we managed to collect and return an error code accordingly
}
