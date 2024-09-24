#include "frame.h"
#include "util.h"
#ifdef _WIN32
#include "zlib/zlib.h"
#else
#include "zlib/zlib.h"
#endif
#include "frames.h"
#include <string.h>
#include <stdlib.h>
#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include "foundation/error.h"
#include "nsid3v2.h"

/* === ID3v2 common === */
ID3v2::Frame::Frame()
{
	data = 0;
	data_size = 0;
}

ID3v2::Frame::~Frame()
{
	free(data);
}

int ID3v2::Frame::GetData(const void **_data, size_t *data_len) const
{
	if (data)
	{
		*_data = data;
		*data_len = data_size;
		return NErr_Success;
	}
	else
		return NErr_NullPointer;
}

size_t ID3v2::Frame::GetDataSize() const
{
	return data_size;
}

int ID3v2::Frame::NewData(size_t new_len, void **_data, size_t *_data_len)
{
	// we DO NOT update the header, as its meant to hold the original data
	void *new_data = realloc(data, new_len);
	if (new_data)
	{
		data = new_data;
		data_size = new_len;
		*_data = data;
		*_data_len = data_size;
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

bool ID3v2::Frame::Encrypted() const
{
	return false;
}

bool ID3v2::Frame::Compressed() const
{
	return false;
}

bool ID3v2::Frame::Grouped() const
{
	return false;
}

bool ID3v2::Frame::ReadOnly() const
{
	return false;
}

bool ID3v2::Frame::FrameUnsynchronised() const
{
	return false;
}

bool ID3v2::Frame::DataLengthIndicated() const
{
	return false;
}

bool ID3v2::Frame::TagAlterPreservation() const
{
	return false;
}

bool ID3v2::Frame::FileAlterPreservation() const
{
	return false;
}

static inline void Advance(const void *&data, size_t &len, size_t amount)
{
	data = (const uint8_t *)data + amount;
	len -= amount;
}

static inline void AdvanceBoth(const void *&data, size_t &len, size_t &len2, size_t amount)
{
	data = (const uint8_t *)data + amount;
	len -= amount;
	len2 -= amount;
}


/* === ID3v2.2 === */
ID3v2_2::Frame::Frame(const ID3v2::Header &_header, const int8_t *id, int flags) : header(_header, id, flags)
{
}

ID3v2_2::Frame::Frame(const FrameHeader &_header) : header(_header)
{
}

int ID3v2_2::Frame::Parse(const void *_data, size_t len, size_t *read)
{
	*read = 0;
	data_size = header.FrameSize(); // size of frame AFTER re-synchronization

	/* check to make sure that we have enough input data to read the data */
	if (header.Unsynchronised())
	{
		/* this is tricky, because the stored size reflects after re-synchronization,
		but the incoming data is unsynchronized */
		if (ID3v2::Util::UnsynchronisedInputSize(_data, data_size) > len)
			return 1;
	}
	else if (data_size > len)
		return 1;

	/* allocate memory (real data_size) */
	data = malloc(data_size);
	if (!data)
		return 1;

	/* === Read the data === */
	if (header.Unsynchronised())
	{
		*read += ID3v2::Util::UnsynchroniseTo(data, _data, data_size);
	}
	else // normal data
	{
		memcpy(data, _data, data_size);
		*read += data_size;
	}

	return NErr_Success;
}


int ID3v2_2::Frame::SerializedSize(uint32_t *length, const ID3v2::Header &tag_header, int flags) const
{
	ID3v2_2::FrameHeader new_header(header, tag_header);
	// TODO: for now, we're not going to deal with compression
	new_header.SetSize(data_size);

	uint32_t current_length=0;
	new_header.SerializedSize(&current_length);

	if (new_header.Unsynchronised())
	{
		current_length += ID3v2::Util::SynchronisedSize(data, data_size);
	}
	else
	{
		current_length += new_header.FrameSize();
	}

	*length = current_length;
	return NErr_Success;
}

int ID3v2_2::Frame::Serialize(void *output, uint32_t *written, const ID3v2::Header &tag_header, int flags) const
{
	size_t current_length = FrameHeader::SIZE;
	uint8_t *data_ptr = (uint8_t *)output;
	ID3v2_2::FrameHeader new_header(header, tag_header);
	new_header.SetSize(data_size);

	// write frame header
	new_header.Serialize(data_ptr);
	data_ptr += FrameHeader::SIZE;
	if (new_header.Unsynchronised())
	{
		current_length += ID3v2::Util::SynchroniseTo(data_ptr, data, data_size);
	}
	else
	{
		memcpy(data_ptr, data, data_size);
		current_length += data_size;
	}
	*written = current_length;
	return NErr_Success;
}


const int8_t *ID3v2_2::Frame::GetIdentifier() const
{
	return header.GetIdentifier();
}


/* === ID3v2.3 === */
ID3v2_3::Frame::Frame(const ID3v2::Header &_header, const int8_t *id, int flags) : header(_header, id, flags)
{
}

ID3v2_3::Frame::Frame(const FrameHeader &_header) : header(_header)
{
}

/* helper function
reads num_bytes from input into output, dealing with re-synchronization and length checking 
increments input pointer
increments bytes_read value by number of input bytes read (different from num_bytes when data is unsynchronized
decrements input_len by bytes read
decrements output_len by bytes written
*/
bool ID3v2_3::Frame::ReadData(void *output, const void *&input, size_t &input_len, size_t &frame_len, size_t num_bytes, size_t *bytes_read) const
{
	/* verify that we have enough data in the frame */
	if (num_bytes > frame_len)
		return false;

	/* verify that we have enough data in the buffer */
	size_t bytes_to_read;
	if (header.Unsynchronised())
		bytes_to_read = ID3v2::Util::UnsynchronisedInputSize(input, num_bytes);
	else
		bytes_to_read = num_bytes;

	if (bytes_to_read > input_len)
		return false;

	/* read data */
	if (header.Unsynchronised())
	{
		*bytes_read += ID3v2::Util::SynchroniseTo(&output, input, num_bytes);
	}
	else
	{
		*bytes_read += num_bytes;
		memcpy(output, input, num_bytes);
	}

	/* increment input pointer */
	input = (const uint8_t *)input + bytes_to_read;

	/* decrement sizes */
	frame_len -= num_bytes;
	input_len -= bytes_to_read;
	return true;	
}

/* benski> this function is a bit complex
we have two things to worry about, and can have any combination of the two
1) Is the data 'unsynchronized'
2) Is the data compressed (zlib)

we keep track of three sizes:
len - number of bytes in input buffer
data_size - number of bytes of output data buffer
frame_size - number of bytes of data in frame AFTER re-synchronization

frame_size==data_size when compression is OFF
*/
int ID3v2_3::Frame::Parse(const void *_data, size_t len, size_t *read)
{
	*read = 0;
	size_t frame_size = header.FrameSize(); // size of frame AFTER re-synchronization

	if (header.Compressed())
	{
		// read 4 bytes of decompressed size
		uint8_t raw_size[4];
		if (ReadData(raw_size, _data, len, frame_size, 4, read) == false)
			return 1;

		bytereader_value_t byte_reader;
		bytereader_init(&byte_reader, raw_size, 4);

		data_size = bytereader_read_u32_be(&byte_reader);
	}

	/* Check for group identity.  If this exists, we'll store it separate from the raw data */
	if (header.Grouped())
	{
		// read 1 byte for group identity
		if (ReadData(&group_identity, _data, len, frame_size, 1, read) == false)
			return 1;
	}

	if (!header.Compressed())
	{
		data_size = frame_size;
	}

	/* check to make sure that we have enough input data to read the data */
	if (!header.Compressed() && header.Unsynchronised())
	{
		/* this is tricky, because the stored size reflects after re-synchronization,
		but the incoming data is unsynchronized */
		if (ID3v2::Util::UnsynchronisedInputSize(_data, data_size) > len)
			return 1;
	}
	else if (frame_size > len)
		return 1;

	/* allocate memory (real data_size) */
	data = malloc(data_size);
	if (!data)
		return NErr_OutOfMemory;

	/* === Read the data === */
	if (header.Compressed())
	{
		if (header.Unsynchronised()) // compressed AND unsynchronized.. what a pain!!
		{
			// TODO: combined re-synchronization + inflation
			void *temp = malloc(frame_size);
			if (!temp)
				return NErr_OutOfMemory;

			*read += ID3v2::Util::UnsynchroniseTo(temp, _data, frame_size);

			uLongf uncompressedSize = data_size;
			int ret = uncompress((Bytef *)data, &uncompressedSize, (const Bytef *)temp, frame_size);
			free(temp);
			if (ret != Z_OK)
				return 1;
		}
		else
		{
			uLongf uncompressedSize = data_size;
			if (uncompress((Bytef *)data, &uncompressedSize, (const Bytef *)_data, frame_size) != Z_OK)
				return 1;
			*read += frame_size;
		}
	}
	else if (header.Unsynchronised())
	{
		*read += ID3v2::Util::UnsynchroniseTo(data, _data, data_size);
	}
	else // normal data
	{
		memcpy(data, _data, data_size);
		*read += data_size;
	}

	return NErr_Success;
}

int ID3v2_3::Frame::SerializedSize(uint32_t *length, const ID3v2::Header &tag_header, int flags) const
{
	ID3v2_3::FrameHeader new_header(header, tag_header);
	// TODO: for now, we're not going to deal with compression
	new_header.ClearCompressed();
	new_header.SetSize(data_size);

	uint32_t current_length=0;
	new_header.SerializedSize(&current_length);

	if (new_header.Unsynchronised())
	{
		if (new_header.Compressed())
		{
			uint8_t data_length[4];
			bytewriter_s byte_writer;
			bytewriter_init(&byte_writer, data_length, 4);
			bytewriter_write_u32_be(&byte_writer, data_size);
			current_length += ID3v2::Util::SynchronisedSize(&data_length, 4);
		}

		if (new_header.Grouped())
			current_length += ID3v2::Util::SynchronisedSize(&group_identity, 1);
		current_length += ID3v2::Util::SynchronisedSize(data, data_size);
	}
	else
	{
		current_length += new_header.FrameSize();
	}

	*length = current_length;
	return NErr_Success;
}

int ID3v2_3::Frame::Serialize(void *output, uint32_t *written, const ID3v2::Header &tag_header, int flags) const
{
	size_t current_length = FrameHeaderBase::SIZE;
	uint8_t *data_ptr = (uint8_t *)output;
	ID3v2_3::FrameHeader new_header(header, tag_header);
	// TODO: for now, we're not going to deal with compression
	new_header.ClearCompressed();
	new_header.SetSize(data_size);

	// write frame header
	uint32_t header_size;
	new_header.Serialize(data_ptr, &header_size);
	data_ptr += header_size;
	if (new_header.Unsynchronised())
	{
		if (new_header.Compressed())
		{
			uint8_t data_length[4];
			bytewriter_s byte_writer;
			bytewriter_init(&byte_writer, data_length, 4);
			bytewriter_write_u32_be(&byte_writer, data_size);
			current_length += ID3v2::Util::SynchroniseTo(data_ptr, &data_length, 4);
			data_ptr+=4;
		}

		if (new_header.Grouped())
			current_length += ID3v2::Util::SynchroniseTo(data_ptr++, &group_identity, 1);
		current_length += ID3v2::Util::SynchroniseTo(data_ptr, data, data_size);
	}
	else
	{
		if (new_header.Compressed())
		{
			bytewriter_s byte_writer;
			bytewriter_init(&byte_writer, data_ptr, 4);
			bytewriter_write_u32_be(&byte_writer, data_size);
			data_ptr+=4;
		}

		if (new_header.Grouped())
		{
			*data_ptr++ = group_identity;
			current_length++;
		}
		memcpy(data_ptr, data, data_size);
		current_length += data_size;
	}
	*written = current_length;
	return NErr_Success;
}

const int8_t *ID3v2_3::Frame::GetIdentifier() const
{
	return header.GetIdentifier();
}

bool ID3v2_3::Frame::Encrypted() const
{
	return header.Encrypted();
}

bool ID3v2_3::Frame::Compressed() const
{
	return header.Compressed();
}

bool ID3v2_3::Frame::Grouped() const
{
	return header.Grouped();
}

bool ID3v2_3::Frame::ReadOnly() const
{
	return header.ReadOnly();
}

bool ID3v2_3::Frame::TagAlterPreservation() const
{
	return header.TagAlterPreservation();
}

bool ID3v2_3::Frame::FileAlterPreservation() const
{
	return header.FileAlterPreservation();
}


/* === ID3v2.4 === */
ID3v2_4::Frame::Frame(const ID3v2::Header &_header, const int8_t *id, int flags) : header(_header, id, flags)
{
}

ID3v2_4::Frame::Frame(const FrameHeader &_header) : header(_header)
{
}

/* helper function
reads num_bytes from input into output, dealing with re-synchronization and length checking 
increments input pointer
increments bytes_read value by number of input bytes read (different from num_bytes when data is unsynchronized
decrements input_len by bytes read
decrements output_len by bytes written
*/
bool ID3v2_4::Frame::ReadData(void *output, const void *&input, size_t &input_len, size_t &frame_len, size_t num_bytes, size_t *bytes_read) const
{
	/* verify that we have enough data in the frame */
	if (num_bytes > frame_len)
		return false;

	/* verify that we have enough data in the buffer */
	size_t bytes_to_read = num_bytes;

	if (bytes_to_read > input_len)
		return false;

	/* read data */

	*bytes_read += num_bytes;
	memcpy(output, input, num_bytes);

	/* increment input pointer */
	input = (const uint8_t *)input + bytes_to_read;

	/* decrement sizes */
	frame_len -= num_bytes;
	input_len -= bytes_to_read;
	return true;	
}

/* benski> this function is a bit complex
we have two things to worry about, and can have any combination of the two
1) Is the data 'unsynchronized'
2) Is the data compressed (zlib)

we keep track of three sizes:
len - number of bytes in input buffer
data_size - number of bytes of output data buffer
frame_size - number of bytes of data in frame AFTER re-synchronization

frame_size==data_size when compression is OFF
*/
int ID3v2_4::Frame::Parse(const void *_data, size_t len, size_t *read)
{
	*read = 0;
	size_t frame_size = header.FrameSize(); 

	// TODO: if frame_size >= 128, verify size.  iTunes v2.4 parser bug ...


	/* Check for group identity.  If this exists, we'll store it separate from the raw data */
	/* Note: ID3v2.4 puts group identity BEFORE data length indicator, where as v2.3 has it the other way */
	if (header.Grouped())
	{
		// read 1 byte for group identity
		if (ReadData(&group_identity, _data, len, frame_size, 1, read) == false)
			return 1;
	}

	if (header.Compressed() || header.DataLengthIndicated())
	{
		// read 4 bytes of decompressed size
		uint8_t raw_size[4];
		if (ReadData(raw_size, _data, len, frame_size, 4, read) == false)
			return 1;

		bytereader_value_t byte_reader;
		bytereader_init(&byte_reader, raw_size, 4);

		data_size = bytereader_read_u32_be(&byte_reader);
	}

	if (!(header.Compressed() || header.DataLengthIndicated()))
	{
		data_size = frame_size;
	}

	/* check to make sure that we have enough input data to read the data */

	if (frame_size > len)
		return 1;

	if (!header.Compressed() && header.Unsynchronised())
	{
		data_size = ID3v2::Util::UnsynchronisedOutputSize(_data, frame_size);
	}

	/* allocate memory (real data_size) */
	data = malloc(data_size);
	if (!data)
		return NErr_OutOfMemory;

	/* === Read the data === */
	if (header.Compressed())
	{
		if (header.Unsynchronised()) // compressed AND unsynchronized.. what a pain!!
		{
			// TODO: combined re-synchronization + inflation
			size_t sync_size = ID3v2::Util::UnsynchronisedOutputSize(_data, frame_size);
			void *temp = malloc(sync_size);
			if (!temp)
				return NErr_OutOfMemory;

			*read += ID3v2::Util::UnsynchroniseTo(temp, _data, sync_size);

			uLongf uncompressedSize = data_size;
			int ret = uncompress((Bytef *)data, &uncompressedSize, (const Bytef *)temp, sync_size);
			/* TODO: realloc and set data_size to uncompressedSize if uncompressedSize was actually lower */
			free(temp);
			if (ret != Z_OK)
				return 1;
		}
		else
		{
			uLongf uncompressedSize = data_size;
			if (uncompress((Bytef *)data, &uncompressedSize, (const Bytef *)_data, frame_size) != Z_OK)
				return 1;
			/* TODO: realloc and set data_size to uncompressedSize if uncompressedSize was actually lower */
			*read += frame_size;
		}
	}
	else if (header.Unsynchronised())
	{
		*read += ID3v2::Util::UnsynchroniseTo(data, _data, data_size);
	}
	else // normal data
	{
		memcpy(data, _data, data_size);
		*read += data_size;
	}

	return 0;
}

int ID3v2_4::Frame::SerializedSize(uint32_t *length, const ID3v2::Header &tag_header, int flags) const
{
	ID3v2_4::FrameHeader new_header(header, tag_header);
	// TODO: for now, we're not going to deal with compression
	new_header.ClearCompressed();
	switch(flags & Serialize_UnsynchronizeMask)
	{
	case Serialize_Unsynchronize:
		// TODO:
		break;
	case Serialize_NoUnsynchronize:
		new_header.ClearUnsynchronized();
		break;
	}

	// TODO: this doesn't handle compression
	if (new_header.Unsynchronised())
	{
		size_t unsynchronized_data_size = ID3v2::Util::SynchronisedSize(data, data_size);
		new_header.SetSize(unsynchronized_data_size);
	}
	else
	{
		new_header.SetSize(data_size);
	}


	size_t current_length = ID3v2_4::FrameHeader::SIZE;

	if (new_header.Unsynchronised())
	{
		if (new_header.DataLengthIndicated() || new_header.Compressed())
		{
			current_length += 4;
		}

		if (new_header.Grouped())
			current_length += ID3v2::Util::SynchronisedSize(&group_identity, 1);
		current_length += ID3v2::Util::SynchronisedSize(data, data_size);
	}
	else
	{
		current_length += new_header.FrameSize();
	}

	*length = current_length;
	return NErr_Success;
}

int ID3v2_4::Frame::Serialize(void *output, uint32_t *written, const ID3v2::Header &tag_header, int flags) const
{
	size_t current_length = ID3v2_4::FrameHeader::SIZE;
	uint8_t *data_ptr = (uint8_t *)output;
	ID3v2_4::FrameHeader new_header(header, tag_header);
	// TODO: for now, we're not going to deal with compression
	new_header.ClearCompressed();
	switch(flags & Serialize_UnsynchronizeMask)
	{
	case Serialize_Unsynchronize:
		// TODO:
		break;
	case Serialize_NoUnsynchronize:
		new_header.ClearUnsynchronized();
		break;
	}

	// TODO: this doesn't handle compression
	if (new_header.Unsynchronised())
	{
		size_t unsynchronized_data_size = ID3v2::Util::SynchronisedSize(data, data_size);
		new_header.SetSize(unsynchronized_data_size);
	}
	else
	{
		new_header.SetSize(data_size);
	}

	// write frame header
	uint32_t header_size;
	new_header.Serialize(data_ptr, &header_size);
	data_ptr += header_size;

	if (new_header.Compressed() || new_header.DataLengthIndicated())
	{
		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data_ptr, 4);
		bytewriter_write_u32_be(&byte_writer, ID3v2::Util::Int32To28(data_size));
		data_ptr+=4;
		current_length+=4;
	}

	if (new_header.Unsynchronised())
	{

		if (Grouped())
			current_length += ID3v2::Util::SynchroniseTo(data_ptr++, &group_identity, 1);
		current_length += ID3v2::Util::SynchroniseTo(data_ptr, data, data_size);
	}
	else
	{

		if (new_header.Grouped())
		{
			*data_ptr++ = group_identity;
			current_length++;
		}
		memcpy(data_ptr, data, data_size);
		current_length += data_size;
	}
	*written = current_length;
	return NErr_Success;
}

const int8_t *ID3v2_4::Frame::GetIdentifier() const
{
	return header.GetIdentifier();
}

bool ID3v2_4::Frame::Encrypted() const
{
	return header.Encrypted();
}

bool ID3v2_4::Frame::Compressed() const
{
	return header.Compressed();
}

bool ID3v2_4::Frame::Grouped() const
{
	return header.Grouped();
}

bool ID3v2_4::Frame::ReadOnly() const
{
	return header.ReadOnly();
}

bool ID3v2_4::Frame::FrameUnsynchronised() const
{
	return header.FrameUnsynchronised();
}

bool ID3v2_4::Frame::DataLengthIndicated() const
{
	return header.DataLengthIndicated();
}


bool ID3v2_4::Frame::TagAlterPreservation() const
{
	return header.TagAlterPreservation();
}

bool ID3v2_4::Frame::FileAlterPreservation() const
{
	return header.FileAlterPreservation();
}
