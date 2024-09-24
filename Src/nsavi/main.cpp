#include <stdio.h>
#include <stdlib.h>
#include <windows.h> // evil, i know
#include <bfc/platform/types.h>
#include "avi_header.h"
#include "file_avi_reader.h"
#include "read.h"
#include "Info.h"

using namespace nsavi;

void printf_riff_chunk(const riff_chunk *chunk, int indent)
{
	char cc[5];
	memcpy(cc, &chunk->id, 4);
	cc[4]=0;
	if (chunk->type)
	{
		char type[5];
		memcpy(type, &chunk->type, 4);
		type[4]=0;
		printf("%*sID: %4s%*sSIZE: %10u   TYPE: %s\r\n", indent, "", cc,8-indent,   "",chunk->size, type);
	}
	else
		printf("%*sID: %4s%*sSIZE: %10u\r\n", indent, "", cc, 8-indent, "", chunk->size);
}



uint32_t ParseLIST(nsavi::avi_reader *reader, const riff_chunk *parse_chunk, int indent);
uint32_t ParseHDRL(nsavi::avi_reader *reader, uint32_t chunk_size, int indent)
{
	uint32_t total_bytes_read=0;
	uint32_t bytes_read=0;

	riff_chunk chunk;
	while ((chunk_size - total_bytes_read) >= 8
		&& total_bytes_read < chunk_size // seems redundant, but since the above is unsigned math, we won't get negative values
		&& (read_riff_chunk(reader, &chunk, &bytes_read) == READ_OK))
	{
		total_bytes_read += bytes_read;
		printf_riff_chunk(&chunk, indent);
		if (chunk.id == 'TSIL')
		{
			bytes_read = ParseLIST(reader, &chunk, indent+1);
			if (!bytes_read)
				return 0;
			total_bytes_read += bytes_read;
		}
		else if (chunk.id == 'hiva')
		{
			nsavi::AVIH *header = (nsavi::AVIH *)malloc(chunk.size + sizeof(uint32_t));
			if (header)
			{
				reader->Read(((uint8_t *)header) + sizeof(uint32_t), chunk.size, &bytes_read);
				if (bytes_read != chunk.size)
					return 0;
				total_bytes_read+=bytes_read;
				header->size_bytes = chunk.size;
			}
			else
				return 0;
		}
		else
		{
			if (skip_chunk(reader, &chunk, &bytes_read) != READ_OK)
				return 0;

			total_bytes_read += bytes_read;
		}

	}
	return total_bytes_read;
}

uint32_t ParseSTRL(nsavi::avi_reader *reader, uint32_t chunk_size, int indent)
{
	uint32_t total_bytes_read=0;
	uint32_t bytes_read=0;

	riff_chunk chunk;
	while ((chunk_size - total_bytes_read) >= 8
		&& total_bytes_read < chunk_size // seems redundant, but since the above is unsigned math, we won't get negative values
		&& (read_riff_chunk(reader, &chunk, &bytes_read) == READ_OK))
	{
		total_bytes_read += bytes_read;
		printf_riff_chunk(&chunk, indent);
		if (chunk.id == 'TSIL')
		{
			bytes_read = ParseLIST(reader, &chunk, indent+1);
			if (!bytes_read)
				return 0;
						total_bytes_read += bytes_read;
		}
		else if (chunk.id == 'hrts')
		{
			nsavi::STRH *header = (nsavi::STRH *)malloc(chunk.size + sizeof(uint32_t));
			if (header)
			{
				reader->Read(((uint8_t *)header) + sizeof(uint32_t), chunk.size, &bytes_read);
				if (bytes_read != chunk.size)
					return 0;
				total_bytes_read+=bytes_read;
				header->size_bytes = chunk.size;
			}
			else
				return 0;
		}
		else if (chunk.id == 'frts')
		{
			nsavi::STRF *header = (nsavi::STRF *)malloc(chunk.size + sizeof(uint32_t));
			if (header)
			{
				reader->Read(((uint8_t *)header) + sizeof(uint32_t), chunk.size, &bytes_read);
				if (bytes_read != chunk.size)
					return 0;
				total_bytes_read+=bytes_read;
				header->size_bytes = chunk.size;
			}
			else  
				return 0;
		}
		else if (chunk.id == 'xdni')
		{
			nsavi::INDX *index = (nsavi::INDX *)malloc(chunk.size + sizeof(uint32_t));
			if (index)
			{
				reader->Read(&index->entry_size, chunk.size, &bytes_read);
				if (bytes_read != chunk.size)
					return 0;
				total_bytes_read+=bytes_read;
				index->size_bytes = chunk.size;
			}
			else  
				return 0;
		}
		else
		{
			if (skip_chunk(reader, &chunk, &bytes_read) != READ_OK)
				return 0;

			total_bytes_read += bytes_read;
		}

	}
	return total_bytes_read;
}

uint32_t ParseGeneric(nsavi::avi_reader *reader, uint32_t chunk_size, int indent)
{
	uint32_t total_bytes_read=0;
	uint32_t bytes_read=0;

	riff_chunk chunk;
	while ((chunk_size - total_bytes_read) >= 8
		&& total_bytes_read < chunk_size // seems redundant, but since the above is unsigned math, we won't get negative values
		&& (read_riff_chunk(reader, &chunk, &bytes_read) == READ_OK))
	{
		total_bytes_read += bytes_read;
		printf_riff_chunk(&chunk, indent);
		if (chunk.id == 'TSIL')
		{
			bytes_read = ParseLIST(reader, &chunk, indent+1);
			if (!bytes_read)
				return 0;
						total_bytes_read += bytes_read;
		}
		else
		{
			if (skip_chunk(reader, &chunk, &bytes_read) != READ_OK)
				return 0;

			total_bytes_read += bytes_read;
		}

	}
	return total_bytes_read;
}

uint32_t ParseLIST(nsavi::avi_reader *reader, const riff_chunk *parse_chunk, int indent)
{
	uint32_t total_bytes_read=0;
	uint32_t bytes_read=0;

	if (parse_chunk->type == 'lrdh')
		ParseHDRL(reader, parse_chunk->size, indent);
	else if (parse_chunk->type == 'lrts')
		ParseSTRL(reader, parse_chunk->size, indent);
	else if (parse_chunk->type == 'lmdo')
		ParseSTRL(reader, parse_chunk->size, indent);
	else if (parse_chunk->type == 'OFNI')
	{
		Info *info = new Info;
		info->Read(reader, parse_chunk->size);
		info = info;
	}
	//else if (parse_chunk->type == 'ivom')
	 //ParseGeneric(reader, parse_chunk->size, indent);
	//else if (parse_chunk->type == ' cer')
	 //ParseGeneric(reader, parse_chunk->size, indent);
	else
		reader->Skip(parse_chunk->size);

	if (parse_chunk->size & 1)
		reader->Skip(1);


	return parse_chunk->size;
}

uint32_t ParseRIFF(nsavi::avi_reader *reader, uint32_t chunk_size, int indent)
{
	uint32_t total_bytes_read=0;
	uint32_t bytes_read=0;

	riff_chunk chunk;
	while ((chunk_size - total_bytes_read) >= 8
		&& total_bytes_read < chunk_size // seems redundant, but since the above is unsigned math, we won't get negative values
		&& (read_riff_chunk(reader, &chunk, &bytes_read) == READ_OK))
	{
		total_bytes_read += bytes_read;
		printf_riff_chunk(&chunk, indent);
		if (chunk.id == 'TSIL')
		{
			bytes_read = ParseLIST(reader, &chunk, indent+1);
			if (!bytes_read)
				return 0;
			total_bytes_read += bytes_read;
		}
		else if (chunk.id == '1xdi')
		{
			nsavi::IDX1 *index = (nsavi::IDX1 *)malloc(chunk.size + sizeof(uint32_t));
			if (index)
			{
				reader->Read(((uint8_t *)index) + sizeof(uint32_t), chunk.size, &bytes_read);
				if (bytes_read != chunk.size)
					return 0;
				total_bytes_read+=bytes_read;
				index->index_count = chunk.size / sizeof(IDX1_INDEX);
			}
		}
		else
		{
			if (skip_chunk(reader, &chunk, &bytes_read) != READ_OK)
				return 0;
			total_bytes_read += bytes_read;
		}

	}
	if (chunk_size & 1)
		reader->Skip(1);

	return total_bytes_read;
}

int main()
{
	AVIReaderFILE reader(L"//o2d2/ftp/usr/nullsoft/test media/20bit/Track 1.wav");
	riff_chunk chunk;
	while (read_riff_chunk(&reader, &chunk) == READ_OK)
	{
		printf_riff_chunk(&chunk, 0);
		if (chunk.id == 'FFIR')
			ParseRIFF(&reader, chunk.size, 1);
		else
			skip_chunk(&reader, &chunk);
	}	

}