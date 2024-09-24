#pragma once
#include "foundation/types.h"
/* A simple byte-oriented writer.
use this instead of manually writing data as this deals with memory alignment issues
for you.
memory alignment can be critical and annoying on some architectures (e.g. PowerPC)
it also handles little-endian/big-endian issues

Usually you just make one of these things on the stack, passing in your buffer and length
*/

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct bytewriter_struct_t
	{
		size_t byte_length;
		uint8_t *data_ptr;
		void *data;
	} bytewriter_s, *bytewriter_t;

	void bytewriter_init(bytewriter_t byte_writer, void *data, size_t byte_length);
	static size_t bytewriter_size(bytewriter_t byte_writer) /* returns remaining bytes in stream */
	{
		return byte_writer->byte_length;
	}
	static void *bytewriter_pointer(bytewriter_t byte_writer) /* returns a pointer to the current bitstream position */
	{
		return byte_writer->data_ptr;
	}
	static void bytewriter_advance(bytewriter_t byte_writer, size_t bytes) /* advances the byte stream */
	{
		byte_writer->byte_length-=bytes;
		byte_writer->data_ptr+=bytes;
	}

	void bytewriter_write_n(bytewriter_t byte_writer, const void *source, size_t bytes);
	void bytewriter_write_zero_n(bytewriter_t byte_writer, size_t bytes);
	void bytewriter_write_u8(bytewriter_t byte_writer, uint8_t value);
	void bytewriter_write_u16_le(bytewriter_t byte_writer, uint16_t value);
	void bytewriter_write_u16_be(bytewriter_t byte_writer, uint16_t value);
	void bytewriter_write_u32_le(bytewriter_t byte_writer, uint32_t value);
	void bytewriter_write_u32_be(bytewriter_t byte_writer, uint32_t value);
	void bytewriter_write_uuid_be(bytewriter_t byte_writer, GUID guid_value);
#ifdef __cplusplus
}
#endif
