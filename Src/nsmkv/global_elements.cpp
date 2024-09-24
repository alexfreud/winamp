#include "global_elements.h"
#include "read.h"

uint64_t nsmkv::ReadGlobal(nsmkv::MKVReader *reader, uint64_t id, uint64_t size)
{
	switch(id)
	{
	case mkv_void:

#ifdef WA_VALIDATE
		printf("void (empty), size:%I64u\n", size);
#endif	
		reader->Skip(size);
		return size;
	default:

#ifdef WA_VALIDATE
		printf("*** UNKNOWN ID *** ID:%I64x size:%I64u\n", id, size);
#endif	
		reader->Skip(size);
		return size;
	}
}

uint64_t nsmkv::SkipNode(nsmkv::MKVReader *reader, uint64_t id, uint64_t size)
{
	reader->Skip(size);
	return size;
}