#include "ebml_signed.h"
#include "ebml_unsigned.h"

int64_t signed_read_ptr_len(uint64_t len, const uint8_t *ptr)
{
	int64_t val = -1;
	uint8_t *dest = (uint8_t *)&val;
	for (int64_t i=0;i!=len;i++)
	{
		dest[len-i-1]=ptr[i];
	}
	return val;

}

