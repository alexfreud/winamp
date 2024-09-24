#include "ByteWriter.h"
#include <string.h>

/* generic endian-agnostic implementation.  this code assumes that unaligned accesses are illegal */
void bytewriter_init(bytewriter_t byte_writer, void *data, size_t byte_length)
{
	byte_writer->data = data;
	byte_writer->byte_length = byte_length;
	byte_writer->data_ptr = (uint8_t *)data;
}

/* n byte functions */
void bytewriter_write_n(bytewriter_t byte_writer, const void *source, size_t bytes)
{
	memcpy(byte_writer->data_ptr, source, bytes);
	byte_writer->data_ptr += bytes;
}

void bytewriter_write_u8(bytewriter_t byte_writer, uint8_t value)
{
	*byte_writer->data_ptr++ = value;
	byte_writer->byte_length--;
}

void bytewriter_write_u16_le(bytewriter_t byte_writer, uint16_t value)
{
	*byte_writer->data_ptr++ = value & 0xFF;
	value >>= 8;
	*byte_writer->data_ptr++ = value & 0xFF;
	byte_writer->byte_length -= 2;
}

void bytewriter_write_u16_be(bytewriter_t byte_writer, uint16_t value)
{
	*byte_writer->data_ptr++ = (value >> 8) & 0xFF;
	*byte_writer->data_ptr++ = value & 0xFF;
	byte_writer->byte_length -= 2;
}

void bytewriter_write_u32_le(bytewriter_t byte_writer, uint32_t value)
{
	*byte_writer->data_ptr++ = value & 0xFF;
	value >>= 8;
	*byte_writer->data_ptr++ = value & 0xFF;
	value >>= 8;
	*byte_writer->data_ptr++ = value & 0xFF;
	value >>= 8;
	*byte_writer->data_ptr++ = value & 0xFF;
	byte_writer->byte_length -= 4;
}

void bytewriter_write_u32_be(bytewriter_t byte_writer, uint32_t value)
{
	*byte_writer->data_ptr++ = (value >> 24) & 0xFF;
	*byte_writer->data_ptr++ = (value >> 16) & 0xFF;
	*byte_writer->data_ptr++ = (value >> 8) & 0xFF;
	*byte_writer->data_ptr++ = value & 0xFF;
	byte_writer->byte_length -= 4;
}

void bytewriter_write_zero_n(bytewriter_t byte_writer, size_t bytes)
{
	size_t i;
	for (i=0;i<bytes;i++)
	{
		*byte_writer->data_ptr++ = 0;
	}
	byte_writer->byte_length -= bytes;
}

void bytewriter_write_uuid_be(bytewriter_t byte_writer, GUID guid_value)
{
	bytewriter_write_u32_be(byte_writer, guid_value.Data1);
	bytewriter_write_u16_be(byte_writer, guid_value.Data2);
	bytewriter_write_u16_be(byte_writer, guid_value.Data3);
	bytewriter_write_n(byte_writer, guid_value.Data4, 8);
}
