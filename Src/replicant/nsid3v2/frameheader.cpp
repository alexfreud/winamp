#include "frameheader.h"
#include "util.h"
#include "values.h"
#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include <string.h>
#include "foundation/error.h"
/* === ID3v2 common === */
ID3v2::FrameHeader::FrameHeader(const ID3v2::Header &_header) : tagHeader(_header)
{
}

static bool CharOK(int8_t c)
{
	if (c >= '0' && c <= '9')
		return true;

	if (c >= 'A' && c <= 'Z')
		return true;

	return false;
}

/* === ID3v2.2 === */
ID3v2_2::FrameHeader::FrameHeader(const ID3v2_2::FrameHeader &frame_header, const ID3v2::Header &_header) : ID3v2::FrameHeader(_header)
{
	frameHeaderData = frame_header.frameHeaderData;
}

ID3v2_2::FrameHeader::FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags) : ID3v2::FrameHeader(_header)
{
	memcpy(&frameHeaderData.id, id, 3);
	frameHeaderData.id[3]=0;
	memset(&frameHeaderData.size, 0, 3);
}

ID3v2_2::FrameHeader::FrameHeader(const ID3v2::Header &_header, const void *data) : ID3v2::FrameHeader(_header)
{
	char temp_data[FrameHeader::SIZE];
	if (tagHeader.Unsynchronised())
	{
		ID3v2::Util::UnsynchroniseTo(temp_data, data, sizeof(temp_data));
		data = temp_data;
	}

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, FrameHeader::SIZE);

	bytereader_read_n(&byte_reader, &frameHeaderData.id, 3);
	frameHeaderData.id[3]=0;
	bytereader_read_n(&byte_reader, &frameHeaderData.size, 3);
}

bool ID3v2_2::FrameHeader::IsValid() const
{
	if (CharOK(frameHeaderData.id[0])
		&& CharOK(frameHeaderData.id[1])
		&& CharOK(frameHeaderData.id[2]))
		return true;

	return false;
}

const int8_t *ID3v2_2::FrameHeader::GetIdentifier() const
{
	return frameHeaderData.id;
}

bool ID3v2_2::FrameHeader::Unsynchronised() const
{
	return tagHeader.Unsynchronised();
}

uint32_t ID3v2_2::FrameHeader::FrameSize() const
{
	return (frameHeaderData.size[0] << 16) | (frameHeaderData.size[1] << 8) | (frameHeaderData.size[2]);
}

void ID3v2_2::FrameHeader::SetSize(uint32_t data_size)
{
	frameHeaderData.size[0] = data_size >> 16;
	frameHeaderData.size[1] = data_size >> 8;
	frameHeaderData.size[2] = data_size;
}

int ID3v2_2::FrameHeader::SerializedSize(uint32_t *written) const
{
	if (tagHeader.Unsynchronised())
	{
		uint8_t data[SIZE];
		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data, SIZE);
		bytewriter_write_n(&byte_writer, frameHeaderData.id, 3);
		bytewriter_write_n(&byte_writer, frameHeaderData.size, 3);
		*written = ID3v2::Util::SynchronisedSize(data, SIZE);
	}
	else
	{
		*written = SIZE;
	}
	return NErr_Success;
}

int ID3v2_2::FrameHeader::Serialize(void *data) const
{
	if (tagHeader.Unsynchronised())
	{
		uint8_t temp[SIZE];
		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, temp, SIZE);
		bytewriter_write_n(&byte_writer, frameHeaderData.id, 3);
		bytewriter_write_n(&byte_writer, frameHeaderData.size, 3);
		ID3v2::Util::SynchroniseTo(data, temp, SIZE);
	}
	else
	{
		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data, SIZE);
		bytewriter_write_n(&byte_writer, frameHeaderData.id, 3);
		bytewriter_write_n(&byte_writer, frameHeaderData.size, 3);
	}
	return NErr_Success;
}

/* === ID3v2.3+ common === */
ID3v2_3::FrameHeaderBase::FrameHeaderBase(const ID3v2_3::FrameHeaderBase &frame_header_base, const ID3v2::Header &_header) : ID3v2::FrameHeader(_header)
{
	memcpy(id, frame_header_base.id, 4);
	size=frame_header_base.size;
	flags[0] = frame_header_base.flags[0];
	flags[1] = frame_header_base.flags[1];
}

ID3v2_3::FrameHeaderBase::FrameHeaderBase(const ID3v2::Header &_header) : ID3v2::FrameHeader(_header)
{
}

ID3v2_3::FrameHeaderBase::FrameHeaderBase(const ID3v2::Header &_header, const int8_t *_id, int _flags) : ID3v2::FrameHeader(_header)
{
	memcpy(id, _id, 4);
	size=0;
	// TODO: flags
	flags[0]=0;
	flags[1]=0;
}

const int8_t *ID3v2_3::FrameHeaderBase::GetIdentifier() const
{
	return id;
}


bool ID3v2_3::FrameHeaderBase::IsValid() const
{
	if (CharOK(id[0])
		&& CharOK(id[1])
		&& CharOK(id[2])
		&& CharOK(id[3]))
		return true;

	return false;
}



/* === ID3v2.3 === */
ID3v2_3::FrameHeader::FrameHeader(const ID3v2_3::FrameHeader &frame_header, const ID3v2::Header &tag_header) : ID3v2_3::FrameHeaderBase(frame_header, tag_header)
{
}

ID3v2_3::FrameHeader::FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags) : ID3v2_3::FrameHeaderBase(_header, id, flags)
{
}

ID3v2_3::FrameHeader::FrameHeader(const ID3v2::Header &_header, const void *data) : ID3v2_3::FrameHeaderBase(_header)
{
	char temp_data[FrameHeaderBase::SIZE];
	if (tagHeader.Unsynchronised())
	{
		ID3v2::Util::UnsynchroniseTo(temp_data, data, sizeof(temp_data));
		data = temp_data;
	}

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, FrameHeaderBase::SIZE);

	bytereader_read_n(&byte_reader, &id, 4);
	size = bytereader_read_u32_be(&byte_reader);
	bytereader_read_n(&byte_reader, &flags, 2);
}

int ID3v2_3::FrameHeaderBase::SerializedSize(uint32_t *written) const
{
	if (tagHeader.Unsynchronised())
	{
		uint8_t data[SIZE];
		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data, SIZE);
		bytewriter_write_n(&byte_writer, id, 4);
		bytewriter_write_u32_be(&byte_writer, size);
		bytewriter_write_u8(&byte_writer, flags[0]);
		bytewriter_write_u8(&byte_writer, flags[1]);
		*written = ID3v2::Util::SynchronisedSize(data, SIZE);
	}
	else
	{
		*written = SIZE;
	}
	return NErr_Success;
}

int ID3v2_3::FrameHeaderBase::Serialize(void *data, uint32_t *written) const
{
	if (tagHeader.Unsynchronised())
	{
		uint8_t temp[SIZE];
		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, temp, SIZE);
		bytewriter_write_n(&byte_writer, id, 4);
		bytewriter_write_u32_be(&byte_writer, size);
		bytewriter_write_u8(&byte_writer, flags[0]);
		bytewriter_write_u8(&byte_writer, flags[1]);
		*written = ID3v2::Util::SynchroniseTo(data, temp, SIZE);
	}
	else
	{
		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data, SIZE);
		bytewriter_write_n(&byte_writer, id, 4);
		bytewriter_write_u32_be(&byte_writer, size);
		bytewriter_write_u8(&byte_writer, flags[0]);
		bytewriter_write_u8(&byte_writer, flags[1]);
		*written = SIZE;
	}
	return NErr_Success;
}

uint32_t ID3v2_3::FrameHeader::FrameSize() const
{
	return size;
}

bool ID3v2_3::FrameHeader::ReadOnly() const
{
	return !!(flags[0] & (1<<5));
}

bool ID3v2_3::FrameHeader::Encrypted() const
{
	return !!(flags[1] & (1<<6));
}

bool ID3v2_3::FrameHeader::Unsynchronised() const
{
	return tagHeader.Unsynchronised();
}

bool ID3v2_3::FrameHeader::Grouped() const
{
	return !!(flags[1] & (1 << 5));
}

bool ID3v2_3::FrameHeader::Compressed() const
{
	return !!(flags[1] & (1 << 7));
}

bool ID3v2_3::FrameHeader::TagAlterPreservation() const
{
	return !!(flags[0] & (1<<7));
}

bool ID3v2_3::FrameHeader::FileAlterPreservation() const
{
	return !!(flags[0] & (1<<6));
}

void ID3v2_3::FrameHeader::ClearCompressed()
{
	flags[1] &= ~(1 << 7);
}

void ID3v2_3::FrameHeader::SetSize(uint32_t data_size)
{
	if (Compressed())
		data_size+=4;
	if (Grouped())
		data_size++;
	size = data_size;
}

/* === ID3v2.4 === */
ID3v2_4::FrameHeader::FrameHeader(const ID3v2_4::FrameHeader &frame_header, const ID3v2::Header &tag_header) : ID3v2_3::FrameHeaderBase(frame_header, tag_header)
{
}

ID3v2_4::FrameHeader::FrameHeader(const ID3v2::Header &_header, const int8_t *id, int flags) : ID3v2_3::FrameHeaderBase(_header, id, flags)
{
}

ID3v2_4::FrameHeader::FrameHeader(const ID3v2::Header &_header, const void *data) : ID3v2_3::FrameHeaderBase(_header)
{
	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, FrameHeaderBase::SIZE);

	bytereader_read_n(&byte_reader, &id, 4);
	size = bytereader_read_u32_be(&byte_reader);
	bytereader_read_n(&byte_reader, &flags, 2);
}

uint32_t ID3v2_4::FrameHeader::FrameSize() const
{
	// many programs write non-syncsafe sizes (iTunes is the biggest culprit)
	// so we'll try to detect it.  unfortunately this isn't foolproof
	// ID3v2_4::Frame will have some additional checks
	int mask = size & 0x80808080;
	if (mask)
		return size;
	else
		return ID3v2::Util::Int28To32(size);
}

bool ID3v2_4::FrameHeader::ReadOnly() const
{
	return !!(flags[0] & (1<<4));
}

bool ID3v2_4::FrameHeader::Encrypted() const
{
	return !!(flags[1] & (1<<3));
}

bool ID3v2_4::FrameHeader::Unsynchronised() const
{
	return tagHeader.Unsynchronised() || !!(flags[1] & (1 << 1));
}

bool ID3v2_4::FrameHeader::FrameUnsynchronised() const
{
	return !!(flags[1] & (1 << 1));
}

bool ID3v2_4::FrameHeader::DataLengthIndicated() const
{
	return !!(flags[1] & (1 << 0));
}

bool ID3v2_4::FrameHeader::Compressed() const
{
	return !!(flags[1] & (1 << 3));
}

bool ID3v2_4::FrameHeader::Grouped() const
{
	return !!(flags[1] & (1 << 6));
}

bool ID3v2_4::FrameHeader::TagAlterPreservation() const
{
	return !!(flags[0] & (1<<6));
}

bool ID3v2_4::FrameHeader::FileAlterPreservation() const
{
	return !!(flags[0] & (1<<5));
}

void ID3v2_4::FrameHeader::ClearUnsynchronized()
{
	flags[1] &= ~(1 << 1);
}

void ID3v2_4::FrameHeader::ClearCompressed()
{
	flags[1] &= ~(1 << 3);
}

void ID3v2_4::FrameHeader::SetSize(uint32_t data_size)
{
	if (Compressed() || DataLengthIndicated())
		data_size+=4;
	if (Grouped())
		data_size++;
	size = ID3v2::Util::Int32To28(data_size);
}

int ID3v2_4::FrameHeader::SerializedSize(uint32_t *written) const
{
		*written = SIZE;
	return NErr_Success;
}

int ID3v2_4::FrameHeader::Serialize(void *data, uint32_t *written) const
{
		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data, SIZE);
		bytewriter_write_n(&byte_writer, id, 4);
		bytewriter_write_u32_be(&byte_writer, size);
		bytewriter_write_u8(&byte_writer, flags[0]);
		bytewriter_write_u8(&byte_writer, flags[1]);
		*written = SIZE;
	return NErr_Success;
}
