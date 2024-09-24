#include "util.h"

uint32_t ID3v2::Util::Int28To32(uint32_t val)
{
	// TODO: big endian safe?
	uint32_t ret;

	ret = (val & 0x7FU);
	val >>= 1;
	ret |= (val & 0x3F80U);
	val >>= 1;
	ret |= (val & 0x1FC000U);
	val >>= 1;
	ret |= (val & 0xFE00000U);
	
	return ret;
	/*
	uint8_t *bytes = (uint8_t *)&ret;
	const uint8_t *value = (const uint8_t *)&val;

	ret = (value[0] << 21) + (value[1] << 14) + (value[2] << 7) + (value[3]);
	
	//	for (size_t i=0;i<sizeof(uint32_t);i++ )
	//		value[sizeof(uint32_t)-1-i]=(uint8_t)(val>>(i*8)) & 0xFF;

	return ret;
	*/
}

uint32_t ID3v2::Util::Int32To28(uint32_t val)
{
	// TODO: big endian safe?
	uint32_t ret;
	ret = (val & 0x7FU);
	ret |= (val & 0x3F80U) << 1;
	ret |= (val & 0x1FC000U) << 2;
	ret |= (val & 0xFE00000U) << 3;
	
	return ret;
}

size_t ID3v2::Util::UnsynchroniseTo(void *_output, const void *_input, size_t bytes)
{
	uint8_t *output = (uint8_t *)_output;
	const uint8_t *input = (const uint8_t *)_input;
	size_t bytes_read = 0;
	while (bytes)
	{
		if (input[0] == 0xFF && input[1] == 0)
		{
			*output++ = 0xFF;
			input+=2;
			bytes_read+=2;
			bytes--;
		}
		else
		{
			*output++=*input++;
			bytes_read++;
			bytes--;
		}
	}
	return bytes_read;
}

size_t ID3v2::Util::UnsynchronisedInputSize(const void *data, size_t output_bytes)
{
	const uint8_t *input = (const uint8_t *)data;
	size_t bytes_read = 0;
	while (output_bytes)
	{
		if (input[0] == 0xFF && input[1] == 0)
		{
			input+=2;
			bytes_read+=2;
			output_bytes--;
		}
		else
		{
			input++;
			bytes_read++;
			output_bytes--;
		}
	}
	return bytes_read;
}

size_t ID3v2::Util::UnsynchronisedOutputSize(const void *data, size_t input_bytes)
{
		const uint8_t *input = (const uint8_t *)data;
	size_t bytes_written = 0;
	while (input_bytes)
	{
		if (input[0] == 0xFF && input_bytes > 1 && input[1] == 0)
		{
			input+=2;
			bytes_written++;
			input_bytes-=2;
		}
		else
		{
			input++;
			bytes_written++;
			input_bytes--;
		}
	}
	return bytes_written;
}

// returns output bytes used
size_t ID3v2::Util::SynchroniseTo(void *_output, const void *data, size_t bytes)
{
	uint8_t *output = (uint8_t *)_output;
	const uint8_t *input = (const uint8_t *)data;
	size_t bytes_needed = 0;
	while (bytes)
	{
		*output++=*input;
		bytes_needed++;
		if (*input++ == 0xFF)
		{
			if (bytes == 1)
			{
				// if this is the last byte, we need to make room for an extra 0
				*output = 0;
				return bytes_needed + 1;
			}
			else if ((*input & 0xE0) == 0xE0 || *input == 0)
			{
				*output++ = 0;
				bytes_needed++;
			}
		}
		bytes--;
	}
	return bytes_needed;
}

size_t ID3v2::Util::SynchronisedSize(const void *data, size_t bytes)
{
	const uint8_t *input = (const uint8_t *)data;
	size_t bytes_needed = 0;
	while (bytes)
	{
		bytes_needed++;
		if (*input++ == 0xFF)
		{
			if (bytes == 1)
			{
				// if this is the last byte, we need to make room for an extra 0
				return bytes_needed + 1;
			}
			else if ((*input & 0xE0) == 0xE0 || *input == 0)
			{
				bytes_needed++;
			}
		}
		bytes--;
	}
	return bytes_needed;
}
