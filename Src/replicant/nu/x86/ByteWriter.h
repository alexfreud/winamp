#pragma once
#include "foundation/types.h"
#include "foundation/endian.h"
/* this is separated out to processor-specific types for a few reasons 

1) Unaligned writes are fast/easy on x86, slow on some platforms (ARM, x64), very slow on a few (Itanium) and crash on others (PowerPC)
2) ARM is very good at *ptr++, x86 is very good at ptr[offset]
3) Endian issues
*/
typedef struct ByteWriter
{
	uint8_t *data;
	size_t data_length;
	size_t offset;
} ByteWriter, *nu_bytewriter_t;

/* --------- Construction & Utility --------- */
#define BYTEWRITER_INIT(data, length) { (uint8_t *)data, length, 0 }

inline static uint32_t bw_bytes_written(nu_bytewriter_t bw)
{
	return bw->offset;
}

/* --------- Little Endian writers --------- */
inline static void bytewriter_fourcc_string(nu_bytewriter_t bw, const char *fourcc)
{
	bw->data[bw->offset] = fourcc[0];
	bw->data[bw->offset+1] = fourcc[1];
	bw->data[bw->offset+2] = fourcc[2];
	bw->data[bw->offset+3] = fourcc[3];
	bw->offset += 4;
}

inline static void bytewriter_uint32_le(nu_bytewriter_t bw, uint32_t value)
{
	*(uint32_t *)(&bw->data[bw->offset]) = value;
	bw->offset+=4;
}

inline static void bytewriter_uint16_le(nu_bytewriter_t bw, uint16_t value)
{
	*(uint16_t *)(&bw->data[bw->offset]) = value;
	bw->offset+=2;
}

/* --------- Big Endian writers --------- */
inline static void bytewriter_uint32_be(nu_bytewriter_t bw, uint32_t value)
{
	*(uint32_t *)(&bw->data[bw->offset]) = _byteswap_ulong(value);
	bw->offset+=4;
}

inline static void bytewriter_uint24_be(nu_bytewriter_t bw, uint32_t value)
{
	bw->data[bw->offset] = (uint8_t)(value >> 16) & 0xFF;
	bw->data[bw->offset+1] = (uint8_t)(value >>  8) & 0xFF;
	bw->data[bw->offset+2] = (uint8_t)value & 0xFF;
	bw->offset+=3;
}

inline static void bytewriter_uint16_le(nu_bytewriter_t bw, uint16_t value)
{
	*(uint16_t *)(&bw->data[bw->offset]) = value;
	bw->offset+=2;
}

/* --------- Neutral Endian writers --------- */
inline static void bytewriter_uint32_zero(nu_bytewriter_t bw)
{
	*(uint32_t *)(&bw->data[bw->offset]) = 0;
	bw->offset+=4;
}

inline static void bytewriter_uint32_nzero(nu_bytewriter_t bw, uint32_t num_zeroes)
{
	memset(bw->data, 0, num_zeroes*4);
	bw->offset+=num_zeroes*4;
}

inline static void bytewriter_uint8(nu_bytewriter_t bw, uint8_t value)
{
	*(uint8_t *)&bw->data[bw->offset] = value;
	bw->offset++;
}
