#pragma once
#include <stdint.h>
#include "foundation/types.h"

/* A simple byte-oriented reader.
use this instead of manual parsing as this deals with memory alignment issues
for you.
memory alignment can be critical and annoying on some architectures (e.g. PowerPC)
it also handles little-endian/big-endian issues

Usually you just make one of these things on the stack, passing in your buffer and length

S is signed and U is unsigned
Show functions will give you data w/o moving the stream position
Align versions of the functions will assume the stream is properly aligned
LE versions of the functions treat the byte stream as little-endian oriented
*/

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct bytereader_struct_t
	{
		size_t byte_length;
		const uint8_t *data_ptr;
		const void *data;
	} bytereader_value_t, bytereader_s, *bytereader_t;

	void bytereader_init(bytereader_t byte_reader, const void *data, size_t byte_length);
	static size_t bytereader_size(bytereader_t byte_reader) /* returns remaining bytes in stream */
	{
		return byte_reader->byte_length;
	}
	void bytereader_advance(bytereader_t byte_reader, size_t bytes); /* advances the byte stream */
	void bytereader_reset(bytereader_t byte_reader); /* reset the data pointer and size back to the original position */
	static const void *bytereader_pointer(bytereader_t byte_reader) /* returns a pointer to the current bitstream position */
	{
		return byte_reader->data_ptr;
	}

	/* returns the number of bytes to the next 0, or the end of the buffer */
	size_t bytereader_find_zero(bytereader_t byte_reader);

	/* n byte functions (basically memcpy) */
	void bytereader_show_n(bytereader_t byte_reader, void *destination, size_t bytes);
	void bytereader_read_n(bytereader_t byte_reader, void *destination, size_t bytes);

	/* 1 byte functions */
	uint8_t bytereader_show_u8(bytereader_t byte_reader);
	uint8_t bytereader_read_u8(bytereader_t byte_reader);
	int8_t bytereader_show_s8(bytereader_t byte_reader);
	int8_t bytereader_read_s8(bytereader_t byte_reader);

	/* 2 byte little endian functions */
	uint16_t bytereader_show_u16_le(bytereader_t byte_reader);
	uint16_t bytereader_read_u16_le(bytereader_t byte_reader);
	int16_t bytereader_show_s16_le(bytereader_t byte_reader);
	int16_t bytereader_read_s16_le(bytereader_t byte_reader);

	/* 2 byte big-endian functions */
	uint16_t bytereader_show_u16_be(bytereader_t byte_reader);
	uint16_t bytereader_read_u16_be(bytereader_t byte_reader);
	int16_t bytereader_show_s16_be(bytereader_t byte_reader);
	int16_t bytereader_read_s16_be(bytereader_t byte_reader);

	/* 4 byte big-endian functions */
	uint32_t bytereader_show_u32_be(bytereader_t byte_reader);
	uint32_t bytereader_read_u32_be(bytereader_t byte_reader);

	/* 4 byte little-endian functions */
	uint32_t bytereader_show_u32_le(bytereader_t byte_reader);
	uint32_t bytereader_read_u32_le(bytereader_t byte_reader);

	/* float functions */
	float bytereader_show_f32_be(bytereader_t byte_reader);
	float bytereader_read_f32_be(bytereader_t byte_reader);

	GUID bytereader_read_uuid_be(bytereader_t byte_reader);
	GUID bytereader_read_uuid_le(bytereader_t byte_reader);
#ifdef __cplusplus
}
#endif
