#include "read.h"


/* helper macro to do a read, check error code and number of bytes read, and return from the function if necessary 
  has to be a macro because of the return */

#define NSAVI_READ(reader, buffer, size, bytes_read) { int ret = reader->Read(buffer, size, &bytes_read); if (ret) return ret; if (bytes_read != size) return READ_EOF; }

int nsavi::read_riff_chunk(nsavi::avi_reader *reader, nsavi::riff_chunk *chunk, uint32_t *out_bytes_read)
{
	uint32_t total_bytes_read=0;
	uint32_t bytes_read;
	NSAVI_READ(reader, chunk, 8, bytes_read); // read id and size
	total_bytes_read += bytes_read;

	if (chunk->id == 'FFIR' || chunk->id == 'TSIL')
	{
		if (chunk->size < 4)
			return READ_INVALID_DATA;
		NSAVI_READ(reader, &chunk->type, 4, bytes_read);
		total_bytes_read += bytes_read;
		chunk->size -= 4;
	}
	else
		chunk->type = 0;

	if (out_bytes_read)
		*out_bytes_read = total_bytes_read;
	return READ_OK;
}

// we pass riff_chunk instead of size
// to avoid any confusion about who is responsible for adding the padding byte
// (this function is responsible)
int nsavi::skip_chunk(nsavi::avi_reader *reader, const nsavi::riff_chunk *chunk, uint32_t *out_bytes_read)
{
	uint32_t chunk_size = chunk->size;
	if (chunk_size & 1)
	{
		chunk_size ++;  // odd chunk sizes must be padded by one
		if (chunk_size == 0) // check for overflow
			return READ_INVALID_DATA;
	}

	int ret = reader->Skip(chunk_size);
	if (ret)
		return ret;

	if (out_bytes_read)
		*out_bytes_read = chunk_size;
	return READ_OK;
}

