#include "info.h"
#include "read.h"

nsavi::Info::Info()
{

}

nsavi::Info::~Info()
{
	for (auto itr = this->begin(); itr != this->end(); itr++)
	{
		free((void*)itr->second);
	}
}

int nsavi::Info::Read(nsavi::avi_reader* reader, uint32_t data_len)
{
	while (data_len)
	{
		riff_chunk chunk;
		uint32_t bytes_read = 0;
		nsavi::read_riff_chunk(reader, &chunk, &bytes_read);
		data_len -= bytes_read;
		size_t malloc_size = chunk.size + 1;
		if (malloc_size == 0)
			return READ_INVALID_DATA;

		char* str = (char*)calloc(malloc_size, sizeof(char));
		if (!str)
			return READ_OUT_OF_MEMORY;

		reader->Read(str, chunk.size, &bytes_read);
		str[chunk.size] = 0;
		data_len -= bytes_read;

		Set(chunk.id, str);

		if (chunk.size & 1)
		{
			reader->Skip(1);
			data_len--;
		}
	}
	return 0;
}

void nsavi::Info::Set(uint32_t chunk_id, const char* data)
{
	auto it = this->find(chunk_id);
	if (this->end() == it)
	{
		this->insert({ chunk_id, data });
	}
	else
	{
		it->second = data;
	}
}
const char* nsavi::Info::GetMetadata(uint32_t id)
{
	InfoMap::iterator itr = InfoMap::find(id);
	if (itr != InfoMap::end())
	{
		return itr->second;
	}
	return 0;
}