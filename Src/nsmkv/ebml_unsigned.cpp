#include "ebml_unsigned.h"

uint64_t unsigned_read_ptr_len(uint64_t len, const uint8_t *ptr)
{
	uint64_t val=*ptr++;
	while (--len)
	{
		val <<= 8;
		val |= *ptr++;
	}
	return val;
}

