#include "SoundBlock.h"
#include <memory.h>
#include <malloc.h>

SoundBlock::SoundBlock()
{
	data = 0;
	size = 0;
	next = 0;
	prev = 0;
	start = 0;
	used = 0;
}

SoundBlock::~SoundBlock()
{
	if (data) free(data);
}

void SoundBlock::SetData(void *new_data, size_t new_size)
{
	if (!data) data = malloc(new_size);
	else if (new_size > size)
	{
		data = realloc(data, new_size);
		if (!data) data = malloc(new_size);
		else size = new_size;
	}

	if (data)
	{
		memcpy(data, new_data, new_size);
		used = new_size;
	}
	else
	{
		used = 0;
	}
	start = 0;
}

void SoundBlock::Advance(size_t d)
{
	used -= d;
	start += d;
}

const void *SoundBlock::GetData()
{
	return (char*)data + start;
}

size_t SoundBlock::GetDataSize()
{
	return used;
}

size_t SoundBlock::Dump(void * out, size_t out_size)
{
	const void * src = GetData();
	if (out_size > used) out_size = used;
	memcpy(out, src, out_size);
	Advance(out_size);
	return out_size;
}

void SoundBlock::Clear()
{
	used = 0;
}