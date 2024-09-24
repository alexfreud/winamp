#pragma once
#include "foundation/types.h"

namespace ID3v2
{
	namespace Util
	{	
		/* pass a value you read as if it was a 32bit integer */
		uint32_t Int28To32(uint32_t val);

		uint32_t Int32To28(uint32_t val);

		// returns input bytes used
		size_t UnsynchroniseTo(void *output, const void *input, size_t output_bytes);

		// returns number of real bytes required to read 'bytes' data
		size_t UnsynchronisedInputSize(const void *input, size_t output_bytes);

		
		size_t UnsynchronisedOutputSize(const void *input, size_t input_bytes);

		// returns output bytes used
		size_t SynchroniseTo(void *output, const void *input, size_t bytes);

		// returns number of bytes required to store synchronized version of 'data' (bytes long)
		size_t SynchronisedSize(const void *data, size_t bytes);
	}
}
