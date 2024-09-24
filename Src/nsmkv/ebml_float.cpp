#include "ebml_float.h"

double float_read_ptr_len(uint64_t len, const uint8_t *ptr)
{
	
// TODO: big endian support
	if (len == 4)
	{
		float val;
		uint8_t *dest = (uint8_t *)&val;
		dest[3]=ptr[0];
		dest[2]=ptr[1];
		dest[1]=ptr[2];
		dest[0]=ptr[3];
		return val;
	}
	else if (len == 8)
	{
		double val;
		uint8_t *dest = (uint8_t *)&val;
		dest[7]=ptr[0];
		dest[6]=ptr[1];
		dest[5]=ptr[2];
		dest[4]=ptr[3];
		dest[3]=ptr[4];
		dest[2]=ptr[5];
		dest[1]=ptr[6];
		dest[0]=ptr[7];
		return val;
	}
	else if (len == 10)
	{
		long double val;
		memset(&val, 0, sizeof(val));
		uint8_t *dest = (uint8_t *)&val;
		dest[9]=ptr[0];
		dest[8]=ptr[1];
		dest[7]=ptr[2];
		dest[6]=ptr[3];
		dest[5]=ptr[4];
		dest[4]=ptr[5];
		dest[3]=ptr[6];
		dest[2]=ptr[7];
		dest[1]=ptr[8];
		dest[0]=ptr[9];
		return val;
	}
	else
		return 0;

	
}