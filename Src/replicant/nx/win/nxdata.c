#include "nx/nxdata.h"
#include "foundation/atomics.h"
#include "foundation/error.h"
#include "nx/nxfile.h"
#include <sys/stat.h>

/* windows implementation */
struct nx_data_struct_t
{
	volatile size_t ref_count;
	nx_string_t mime_type;
	nx_string_t description;
	nx_uri_t source_uri;
	nx_file_stat_t source_stats;
	size_t len;
	uint8_t data[1];
};

static size_t NXDataMallocSize(size_t bytes)
{
	/* TODO: overflow check? */
	const nx_data_t dummy=0;
	size_t header = (size_t)&dummy->data[0] - (size_t)dummy;
	return header + bytes;
}

nx_data_t NXDataRetain(nx_data_t data)
{
	if (!data)
		return 0;

	nx_atomic_inc(&data->ref_count);
	return data;
}

void NXDataRelease(nx_data_t data)
{
	if (data)
	{
		if (nx_atomic_dec(&data->ref_count) == 0)
		{
			free(data->source_stats);
			NXURIRelease(data->source_uri);
			NXStringRelease(data->mime_type);
			NXStringRelease(data->description);
			free(data);
		}
	}
}

int NXDataCreate(nx_data_t *out_data, const void *bytes, size_t length)
{
	void *new_bytes;
	int ret = NXDataCreateWithSize(out_data, &new_bytes, length);
	if (ret != NErr_Success)
		return ret;

	memcpy(new_bytes, bytes, length);
	return NErr_Success;
}

int NXDataCreateWithSize(nx_data_t *out_data, void **bytes, size_t length)
{
	nx_data_t data = 0;
	size_t data_length = NXDataMallocSize(length);
	data = (nx_data_t)malloc(data_length);
	if (!data)
		return NErr_OutOfMemory;

	data->ref_count = 1;
	data->len = length;
	data->mime_type=0;
	data->source_uri=0;
	data->source_stats=0;
	data->description=0;
	if (bytes)
		*bytes = data->data;
	*out_data=data;
	return NErr_Success;
}

int NXDataCreateEmpty(nx_data_t *out_data)
{
	return NXDataCreateWithSize(out_data, 0, 0);
}

int NXDataCreateFromURI(nx_data_t *out_data, nx_uri_t filename)
{
	nx_file_stat_s stat_buffer;
	nx_data_t data;
	size_t data_length;
	size_t bytes_read;
	uint64_t file_length;
	void *bytes;
	int ret;
	int fd;


	fd = NXFile_open(filename, nx_file_O_BINARY|nx_file_O_RDONLY);
	if (fd == -1)
		return NErr_FileNotFound;

	ret = NXFile_fstat(fd, &stat_buffer);
	if (ret != NErr_Success)
	{
		close(fd);
		return ret;
	}

	file_length = stat_buffer.file_size;

	if (file_length > SIZE_MAX)
	{
		close(fd);
		return NErr_IntegerOverflow;
	}

	data_length = (size_t)file_length;

	ret = NXDataCreateWithSize(&data, &bytes, data_length);
	if (ret != NErr_Success)
	{
		close(fd);
		return ret;
	}

	data->source_stats=(nx_file_stat_t)malloc(sizeof(nx_file_stat_s));
	if (!data->source_stats)
	{
		close(fd);
		NXDataRelease(data);
		return NErr_OutOfMemory;
	}

	bytes_read = read(fd, bytes, (int)data_length);
	close(fd);
	if (bytes_read != data_length)
	{
		NXDataRelease(data);
		return NErr_Error;
	}

	*data->source_stats=stat_buffer;
	data->source_uri=NXURIRetain(filename);
	*out_data = data;
	return NErr_Success;
}

int NXDataGet(nx_data_t data, const void **bytes, size_t *length)
{
	if (!data)
		return NErr_BadParameter;

	if (data->len == 0)
		return NErr_Empty;

	*bytes = data->data;
	*length = data->len;
	return NErr_Success;
}

size_t NXDataSize(nx_data_t data)
{
	if (!data)
		return 0;

	return data->len;
}

int NXDataSetMIME(nx_data_t data, nx_string_t mime_type)
{
	nx_string_t old;
	if (!data)
		return NErr_BadParameter;

	old = data->mime_type;
	data->mime_type = NXStringRetain(mime_type);
	NXStringRelease(old);
	return NErr_Success;
}

int NXDataSetDescription(nx_data_t data, nx_string_t description)
{
		nx_string_t old;
	if (!data)
		return NErr_BadParameter;

	old = data->description;
	data->description = NXStringRetain(description);
	NXStringRelease(old);
	return NErr_Success;
}

int NXDataSetSourceURI(nx_data_t data, nx_uri_t source_uri)
{
	nx_uri_t old;
	if (!data)
		return NErr_BadParameter;

	old = data->source_uri;
	data->source_uri = NXURIRetain(source_uri);
	NXURIRelease(old);
	return NErr_Success;
}

int NXDataSetSourceStat(nx_data_t data, nx_file_stat_t source_stats)
{
	nx_file_stat_t new_stats;
	if (!data)
		return NErr_BadParameter;

	if (source_stats)
	{
		new_stats=(nx_file_stat_t)malloc(sizeof(nx_file_stat_s));
		if (!new_stats)
			return NErr_OutOfMemory;

		*new_stats = *source_stats;
		free(data->source_stats);
		data->source_stats=new_stats;
	}
	else
	{
		free(data->source_stats);
		data->source_stats=0;
	}
	return NErr_Success;
}

int NXDataGetMIME(nx_data_t data, nx_string_t *mime_type)
{
	if (!data)
		return NErr_BadParameter;

	if (!data->mime_type)
		return NErr_Empty;

	*mime_type = NXStringRetain(data->mime_type);
	return NErr_Success;
}

int NXDataGetDescription(nx_data_t data, nx_string_t *description)
{
	if (!data)
		return NErr_BadParameter;

	if (!data->description)
		return NErr_Empty;

	*description = NXStringRetain(data->description);
	return NErr_Success;
}

int NXDataGetSourceURI(nx_data_t data, nx_uri_t *source_uri)
{
	if (!data)
		return NErr_BadParameter;

	if (!data->source_uri)
		return NErr_Empty;

	*source_uri = NXURIRetain(data->source_uri);
	return NErr_Success;
}

int NXDataGetSourceStat(nx_data_t data, nx_file_stat_t *source_stats)
{
	if (!data)
		return NErr_BadParameter;
	
	if (!data->source_stats)
		return NErr_Empty;

	*source_stats = data->source_stats;
	return NErr_Success;
}
