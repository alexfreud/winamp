#include "header.h"
#include "flags.h"
#include "util.h"
#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static char apev2_preamble[] =  { 'A', 'P', 'E', 'T', 'A', 'G', 'E', 'X' };
APEv2::Header::Header()
{
	memcpy(preamble, apev2_preamble, 8);
	version=2000;
	size=0;
	items=0;
	flags=0;
	reserved=0;
}

APEv2::Header::Header(const void *data)
{
	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, Header::SIZE);

	bytereader_read_n(&byte_reader, preamble, 8);
	version=bytereader_read_u32_le(&byte_reader);
	size=bytereader_read_u32_le(&byte_reader);
	items=bytereader_read_u32_le(&byte_reader);
	flags=bytereader_read_u32_le(&byte_reader);
	bytereader_read_n(&byte_reader, &reserved, 8);	
}

uint32_t APEv2::Header::GetFlags() const
{
	return flags;
}

bool APEv2::Header::Valid() const
{
	return !memcmp(preamble, apev2_preamble, 8) && reserved == 0;
}

uint32_t APEv2::Header::TagSize() const
{
	size_t size = this->size;
	if (IsHeader() && HasFooter())
		size+=SIZE;
	if (IsFooter() && HasHeader())
		size+=SIZE;

	if (size > ULONG_MAX)
		return 0;

	return (uint32_t)size;
}

bool APEv2::Header::HasHeader() const
{
	return !!(flags & FLAG_HEADER_HAS_HEADER);
}

bool APEv2::Header::HasFooter() const
{
	return !(flags & FLAG_HEADER_NO_FOOTER);
}

bool APEv2::Header::IsFooter() const
{
	return !(flags & FLAG_HEADER_IS_HEADER);
}

bool APEv2::Header::IsHeader() const
{
	return !!(flags & FLAG_HEADER_IS_HEADER);
}

int APEv2::Header::Encode(bytewriter_t byte_writer) const
{
	if (bytewriter_size(byte_writer) < 32)
		return NErr_Insufficient;

	bytewriter_write_n(byte_writer, apev2_preamble, 8);
	bytewriter_write_u32_le(byte_writer, version);
	bytewriter_write_u32_le(byte_writer, size);
	bytewriter_write_u32_le(byte_writer, items);
	bytewriter_write_u32_le(byte_writer, flags);
	bytewriter_write_zero_n(byte_writer, 8);

	return NErr_Success;
}