#include "header.h"
#include "values.h"
#include "util.h"
#include <assert.h>
#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include <string.h>
#include "foundation/error.h"

ID3v2::Header::Header()
{
	marker[0]=0;
	marker[1]=0;
	marker[2]=0;
	version=0;
	revision=0;
	flags=0;
	size=0;
}

ID3v2::Header::Header(uint8_t version, uint8_t revision)
{
	marker[0]='I';
	marker[1]='D';
	marker[2]='3';
	this->version=version;
	this->revision=revision;
	this->flags=0;
	this->size=0;	
}

ID3v2::Header::Header(const void *data)
{
	Parse(data);
}


ID3v2::Header::Header(const ID3v2::Header *copy, uint32_t new_size)
{
	marker[0]=copy->marker[0];
	marker[1]=copy->marker[1];
	marker[2]=copy->marker[2];
	version=copy->version;
	revision=copy->revision;
	flags=copy->flags;
	size = Util::Int32To28(new_size);
}

void ID3v2::Header::Parse(const void *data)
{
	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, Header::SIZE);
	bytereader_read_n(&byte_reader, &marker, 3);
	version = bytereader_read_u8(&byte_reader);
	revision = bytereader_read_u8(&byte_reader);
	flags = bytereader_read_u8(&byte_reader);
	size = bytereader_read_u32_be(&byte_reader);
}

int ID3v2::Header::Serialize(void *data)
{
	bytewriter_s byte_writer;
	bytewriter_init(&byte_writer, data, 10);
	bytewriter_write_n(&byte_writer, marker, 3);
	bytewriter_write_u8(&byte_writer, version);
	bytewriter_write_u8(&byte_writer, revision);
	bytewriter_write_u8(&byte_writer, flags);
	bytewriter_write_u32_be(&byte_writer, size);
	return NErr_Success;
}

int ID3v2::Header::SerializeAsHeader(void *data)
{
	bytewriter_s byte_writer;
	bytewriter_init(&byte_writer, data, 10);
	bytewriter_write_n(&byte_writer, "ID3", 3);
	bytewriter_write_u8(&byte_writer, version);
	bytewriter_write_u8(&byte_writer, revision);
	bytewriter_write_u8(&byte_writer, flags);
	bytewriter_write_u32_be(&byte_writer, size);
	return NErr_Success;
}

int ID3v2::Header::SerializeAsFooter(void *data)
{
	bytewriter_s byte_writer;
	bytewriter_init(&byte_writer, data, 10);
	bytewriter_write_n(&byte_writer, "3DI", 3);
	bytewriter_write_u8(&byte_writer, version);
	bytewriter_write_u8(&byte_writer, revision);
	bytewriter_write_u8(&byte_writer, flags);
	bytewriter_write_u32_be(&byte_writer, size);
	return NErr_Success;
}

bool ID3v2::Header::Valid() const
{
	if (marker[0] != 'I'
		|| marker[1] != 'D'
		|| marker[2] != '3')
		return false;

	if (!Values::KnownVersion(version, revision))
		return false;

	if (flags & ~Values::ValidHeaderMask(version, revision))
		return false;

	if (size & 0x80808080)
		return false;

	return true;
}

bool ID3v2::Header::FooterValid() const
{
	if (marker[0] != '3'
		|| marker[1] != 'D'
		|| marker[2] != 'I')
		return false;

	if (!Values::KnownVersion(version, revision))
		return false;

	if (flags & ~Values::ValidHeaderMask(version, revision))
		return false;

	if (size & 0x80808080)
		return false;

	return true;
}

uint32_t ID3v2::Header::TagSize() const
{
	uint32_t size = Util::Int28To32(this->size);
	return size;
}

bool ID3v2::Header::HasExtendedHeader() const
{
	return !!(flags & ID3v2::Values::ExtendedHeaderFlag(version, revision));
}

uint8_t ID3v2::Header::GetVersion() const
{
	return version;
}

uint8_t ID3v2::Header::GetRevision() const
{
	return revision;
}

bool ID3v2::Header::Unsynchronised() const
{
	return !!(flags & (1 << 7));
}

bool ID3v2::Header::HasFooter() const
{
	if (version < 4)
		return false;
	return !!(flags & 0x10);
}

void ID3v2::Header::ClearExtendedHeader()
{
	flags &= ~ID3v2::Values::ExtendedHeaderFlag(version, revision);
}

void ID3v2::Header::ClearUnsynchronized()
{
	flags &= ~(1 << 7);
}

void ID3v2::Header::SetFooter(bool footer)
{
	if (version >= 4)
	{
		if (footer)
			flags |= 0x10;
		else
			flags &= 0x10;
	}
}
