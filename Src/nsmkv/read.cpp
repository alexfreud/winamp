#include "read.h"
#include "vint.h"
#include "ebml_float.h"
#include "ebml_unsigned.h"
#include "ebml_signed.h"
#include <limits.h>
#ifdef WA_VALIDATE
extern uint64_t max_id_length;
extern uint64_t max_size_length;
#endif

// returns bytes read.  0 means EOF
uint64_t read_vint(nsmkv::MKVReader *reader, uint64_t *val)
{
	uint8_t data[9] = {0};
	size_t bytes_read = 0;
	reader->Read(data, 1, &bytes_read);

	if (bytes_read != 1)
		return 0;
	uint8_t length = vint_get_number_bytes(data[0]);
	reader->Read(data+1, length, &bytes_read);
	if (bytes_read != length)
		return 0;

	*val = vint_read_ptr(data);
	return bytes_read+1;
}


// returns bytes read.  0 means EOF
uint64_t read_ebml_node(nsmkv::MKVReader *reader, ebml_node *node)
{
	uint64_t bytes_read = read_vint(reader, &node->id);
	if (!bytes_read)
		return 0;

	bytes_read += read_vint(reader, &node->size);

	return bytes_read;
}

uint64_t read_utf8(nsmkv::MKVReader *reader, uint64_t size, char **utf8)
{
	if (utf8)
	{
		if (size == SIZE_MAX) // prevent integer overflow
			return 0;

		char *&val = *utf8;
		val = (char *)calloc((size_t)size + 1, sizeof(char));
		if (val)
		{
			val[size]=0;
			size_t bytes_read;
			reader->Read(val, (size_t)size, &bytes_read);
			if (bytes_read != (size_t)size)
			{
				free(val);
				return 0;
			}

			return size;
		}
		return 0; // actually, out of memory and not EOF, but still we should abort ASAP
	}
	else
	{
		reader->Skip(size);
		return size;
	}
}

#if 0
int fseek64(nsmkv::MKVReader *reader, int64_t pos, int whence)
{
	switch(whence)
	{
	case SEEK_SET:
		return fsetpos(f, &pos);
	case SEEK_CUR:
		{
			fpos_t curpos=0;
			int ret = fgetpos(f, &curpos);
			if (ret != 0)
				return ret;
			pos+=curpos;
			return fsetpos(f, &pos);
		}
	case SEEK_END:
		{
			return _fseeki64(f, pos, SEEK_END);
		}
	}
	return 1;
}

int64_t ftell64(nsmkv::MKVReader *reader)
{
	fpos_t pos;
	if (fgetpos(f, &pos) == 0)
		return pos;
	else
		return -1L;
}
#endif
uint64_t read_unsigned(nsmkv::MKVReader *reader, uint64_t size, uint64_t *val)
{
	uint8_t data[8] = {0};
	if (size == 0 || size > 8)
		return 0;

	size_t bytes_read = 0;
	reader->Read(data, (size_t)size, &bytes_read);
	if (bytes_read != size)
	{
		return 0;
	}
	*val = unsigned_read_ptr_len(size, data);
	return size;
}

uint64_t read_float(nsmkv::MKVReader *reader, uint64_t size, double *val)
{
	uint8_t data[10] = {0};
	if (size == 0 || size > 10)
		return 0;

	size_t bytes_read = 0;
	reader->Read(data, (size_t)size, &bytes_read);
	if (bytes_read != size)
	{
		return 0;
	}
	*val = float_read_ptr_len(size, data);
	return size;
}

uint64_t read_signed(nsmkv::MKVReader *reader, uint64_t size, int64_t *val)
{
	uint8_t data[8] = {0};
	if (size == 0 || size > 8)
		return 0;

	size_t bytes_read = 0;
	reader->Read(data, (size_t)size, &bytes_read);
	if (bytes_read != size)
	{
		return 0;
	}
	*val = signed_read_ptr_len(size, data);
	return size;
}

