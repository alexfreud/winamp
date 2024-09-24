#include "ByteReader.h"
#include <string.h>
/* generic LITTLE ENDIAN implementation */
void bytereader_init(bytereader_t byte_reader, const void *data, size_t byte_length)
{
	byte_reader->data = data;
	byte_reader->byte_length = byte_length;
	byte_reader->data_ptr = (const uint8_t *)data;
}


void bytereader_advance(bytereader_t byte_reader, size_t bytes)
{
	byte_reader->byte_length -= bytes;
	byte_reader->data_ptr += bytes;
}

void bytereader_reset(bytereader_t byte_reader)
{
	size_t diff = byte_reader->data_ptr - (const uint8_t *)byte_reader->data;
	byte_reader->byte_length += diff;
	byte_reader->data_ptr = (const uint8_t *)byte_reader->data;
}

size_t bytereader_find_zero(bytereader_t byte_reader)
{
	size_t i=0;
	
	for (i=0;i<byte_reader->byte_length && byte_reader->data_ptr[i];i++)
	{
		// empty loop
	}
	return i;
}

/* n byte functions */
void bytereader_show_n(bytereader_t byte_reader, void *destination, size_t bytes)
{
	memcpy(destination, byte_reader->data_ptr, bytes);
}

void bytereader_read_n(bytereader_t byte_reader, void *destination, size_t bytes)
{
	memcpy(destination, byte_reader->data_ptr, bytes);
	bytereader_advance(byte_reader, bytes);
}

/* 1 byte functions */
uint8_t bytereader_show_u8(bytereader_t byte_reader)
{
	return byte_reader->data_ptr[0];
}

uint8_t bytereader_read_u8(bytereader_t byte_reader)
{
	uint8_t ret = byte_reader->data_ptr[0];
	bytereader_advance(byte_reader, 1);
	return ret;
}

int8_t bytereader_show_s8(bytereader_t byte_reader)
{
	return *(const int8_t *)(byte_reader->data_ptr);
}

int8_t bytereader_read_s8(bytereader_t byte_reader)
{
	int8_t ret = *(const int8_t *)(byte_reader->data_ptr);
	bytereader_advance(byte_reader, 1);
	return ret;
}

/* 2 byte little-endian functions */

uint16_t bytereader_show_u16_le(bytereader_t byte_reader)
{
	return (uint16_t)byte_reader->data_ptr[0] | ((uint16_t)byte_reader->data_ptr[1] << 8);
}

uint16_t bytereader_read_u16_le(bytereader_t byte_reader)
{
	uint16_t ret = bytereader_show_u16_le(byte_reader);
	bytereader_advance(byte_reader, 2);
	return ret;
}

int16_t bytereader_show_s16_le(bytereader_t byte_reader)
{
	return (int16_t)byte_reader->data_ptr[0] | ((int16_t)byte_reader->data_ptr[1] << 8);
}

int16_t bytereader_read_s16_le(bytereader_t byte_reader)
{
	int16_t ret = bytereader_show_s16_le(byte_reader);
	bytereader_advance(byte_reader, 2);
	return ret;
}

/* 2 byte big-endian functions */
uint16_t bytereader_show_u16_be(bytereader_t byte_reader)
{
	return (uint16_t)byte_reader->data_ptr[1] | ((uint16_t)byte_reader->data_ptr[0] << 8);
}

uint16_t bytereader_read_u16_be(bytereader_t byte_reader)
{
	uint16_t ret = bytereader_show_u16_be(byte_reader);
	bytereader_advance(byte_reader, 2);
	return ret;
}

int16_t bytereader_show_s16_be(bytereader_t byte_reader)
{
	return (int16_t)byte_reader->data_ptr[1] | ((int16_t)byte_reader->data_ptr[0] << 8);
}

int16_t bytereader_read_s16_be(bytereader_t byte_reader)
{
	int16_t ret = bytereader_show_s16_be(byte_reader);
	bytereader_advance(byte_reader, 2);
	return ret;
}

/* 4 byte functions */

uint32_t bytereader_show_u32_be(bytereader_t byte_reader)
{
	uint32_t x;
	// big endian extract
	
	x = byte_reader->data_ptr[0];
	x <<= 8;
	x |= byte_reader->data_ptr[1];
	x <<= 8;
	x |= byte_reader->data_ptr[2];
	x <<= 8;
	x |= byte_reader->data_ptr[3];
	return x;

}

uint32_t bytereader_read_u32_be(bytereader_t byte_reader)
{
	uint32_t ret = bytereader_show_u32_be(byte_reader);
	bytereader_advance(byte_reader, 4);
	return ret;
}

/* 4 byte little-endian functions */
uint32_t bytereader_show_u32_le(bytereader_t byte_reader)
{
	uint32_t x;
	// little endian extract

	x = byte_reader->data_ptr[3];
	x <<= 8;
	x |= byte_reader->data_ptr[2];
	x <<= 8;
	x |= byte_reader->data_ptr[1];
	x <<= 8;
	x |= byte_reader->data_ptr[0];
	return x;
}

uint32_t bytereader_read_u32_le(bytereader_t byte_reader)
{
	uint32_t ret = bytereader_show_u32_le(byte_reader);
	bytereader_advance(byte_reader, 4);
	return ret;
}

/* float functions */
float bytereader_show_f32_be(bytereader_t byte_reader)
{
	uint32_t ret = bytereader_show_u32_be(byte_reader);
	return *(float *)(&ret);
}

float bytereader_read_f32_be(bytereader_t byte_reader)
{
	uint32_t ret = bytereader_read_u32_be(byte_reader);
	return *(float *)(&ret);
}

GUID bytereader_read_uuid_be(bytereader_t byte_reader)
{
	GUID guid_value;
	guid_value.Data1 = bytereader_read_u32_be(byte_reader);
	guid_value.Data2 = bytereader_read_u16_be(byte_reader);
	guid_value.Data3 = bytereader_read_u16_be(byte_reader);
	bytereader_read_n(byte_reader, guid_value.Data4, 8);
	return guid_value;
}

GUID bytereader_read_uuid_le(bytereader_t byte_reader)
{
	GUID guid_value;
	guid_value.Data1 = bytereader_read_u32_le(byte_reader);
	guid_value.Data2 = bytereader_read_u16_le(byte_reader);
	guid_value.Data3 = bytereader_read_u16_le(byte_reader);
	bytereader_read_n(byte_reader, guid_value.Data4, 8);
	return guid_value;
}
