#include "Metadata.h"

nsavi::Metadata::Metadata(nsavi::avi_reader *_reader) : ParserBase(_reader)
{
	info_found = NOT_READ;
	info = 0;
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

int nsavi::Metadata::GetDuration(int *time_ms)
{
	uint32_t riff_type;
	int ret = GetRIFFType(&riff_type);
	if (ret)
		return ret;

	if (riff_type != ' IVA')
		return READ_INVALID_DATA;

	nsavi::HeaderList header_list;
	ret = GetHeaderList(&header_list);
	if (ret)
		return ret;

	int duration=-1;
	for (uint32_t i=0;i!=header_list.stream_list_size;i++)
	{
		const nsavi::STRL &stream = header_list.stream_list[i];
		if (stream.stream_header)
		{
			if (stream.stream_header->stream_type == nsavi::stream_type_audio)
			{
				if (stream.stream_header->length && !stream.stream_header->sample_size && stream.stream_header->rate)
					duration = (uint64_t)stream.stream_header->length * (uint64_t)stream.stream_header->scale * 1000ULL / (uint64_t)stream.stream_header->rate;
			}
			else if (stream.stream_header->stream_type == nsavi::stream_type_video && stream.stream_header->rate)
			{
				if (duration == -1)
					duration = (uint64_t)stream.stream_header->length * (uint64_t)stream.stream_header->scale * 1000ULL / (uint64_t)stream.stream_header->rate;
			}
		}
	}

	*time_ms = duration;

	return nsavi::READ_OK;
}

int nsavi::Metadata::GetHeaderList(HeaderList *header_list)
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

						info_found = PARSED;
					}
					else
					{
						ret = SkipChunk(reader, &chunk, header_list_parsed, &bytes_read);
						if (ret)
							return ret;
						bytes_available -= bytes_read;
					}
					break;
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

int nsavi::Metadata::GetInfo(nsavi::Info **out_info)
{
if (riff_parsed != PARSED)
		return READ_INVALID_CALL;

	if (riff_parsed == PARSE_RESYNC)
		reader->Seek(riff_start);

	if (info_found == NOT_READ)
	{
		// first, see how far we are into the file to properly bound our reads
		uint64_t start = reader->Tell();
		uint32_t bytes_available = riff_header.size;
		bytes_available -= (uint32_t)(start - riff_start);

		while (bytes_available)
		{
			if (bytes_available < 8)
			{
				info_found = NOT_FOUND;
				return READ_NOT_FOUND;
			}
			uint32_t bytes_read;
			riff_chunk chunk;
			int ret = ReadChunk(reader, &chunk, info_found, &bytes_read);
			if (ret)
				return ret;

			bytes_available -= bytes_read;
			if (bytes_available < chunk.size)
			{
				info_found = PARSE_ERROR;
				return READ_INVALID_DATA;
			}
			switch(chunk.id)
			{
			case 'TSIL': // list chunk
				switch(chunk.type)
				{
				case 'lrdh': // parse this if we havn't already
					if (header_list_parsed != PARSED)
					{
					ret = ParseHeaderList(chunk.size, &bytes_read);
					if (ret)
						return ret;
					bytes_available -= bytes_read;
					header_list_parsed = PARSED;
					}
					else
					{
				ret = SkipChunk(reader, &chunk, info_found, &bytes_read);
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
							info_found = PARSE_ERROR;
							return READ_OUT_OF_MEMORY;
						}

						ret = info->Read(reader, chunk.size);
						if (ret)
						{
							header_list_parsed = PARSE_ERROR;
							return ret;
						}

						info_found = PARSED;
						*out_info = info;
						return nsavi::READ_OK;
					}
					else
					{
						ret = SkipChunk(reader, &chunk, info_found, &bytes_read);
						if (ret)
							return ret;
						bytes_available -= bytes_read;
					}
					break;
				default: // skip anything we don't understand
					ret = SkipChunk(reader, &chunk, info_found, &bytes_read);
					if (ret)
						return ret;
					bytes_available -= bytes_read;
					break;
				}

				break;
			default: // skip anything we don't understand
			case 'KNUJ': // skip junk chunks
				ret = SkipChunk(reader, &chunk, info_found, &bytes_read);
				if (ret)
					return ret;
				bytes_available -= bytes_read;
				break;
			}
		}
	}

	if (info_found == PARSED)
	{
		*out_info = info;
		return READ_OK;
	}

	return READ_INVALID_CALL;
}