#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nu/ByteReader.h"
#include "nx/nxstring.h"
#include "nu/ByteWriter.h"
#include "nsid3v2/frame_utils.h"

static int ParseText(const void *data, size_t data_len, ParsedString &parsed)
{
	if (data_len == 0)
		return NErr_Insufficient;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);
	
	return ParseFrameTerminatedString(&byte_reader, bytereader_read_u8(&byte_reader), parsed);
}

int NSID3v2_Frame_Text_Get(const nsid3v2_frame_t f, nx_string_t *value, int text_flags)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedString parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseText(data, data_len, parsed) == NErr_Success)
		{
			return NXStringCreateFromParsedString(value, parsed, text_flags);
		}
	}

	return NErr_Empty;
}

int NSID3v2_Tag_Text_Get(const nsid3v2_tag_t t, int frame_enum, nx_string_t *value, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;

	const ID3v2::Frame *frame = tag->FindFirstFrame(frame_enum);
	return NSID3v2_Frame_Text_Get((const nsid3v2_frame_t)frame, value, text_flags);
}


/* ---------------- Setters ---------------- */
int NSID3v2_Frame_Text_Set(nsid3v2_frame_t f, nx_string_t value, int text_flags)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	if (frame)
	{
		/* benski> for now, we're going to store UTF-16LE always. in the future, we'll add functions to NXString to determine a 'best' encoding */
		size_t byte_count=0;
		int ret = NXStringGetBytesSize(&byte_count, value, nx_charset_utf16le, 0);
		if (ret != NErr_DirectPointer && ret != NErr_Success)
			return ret;

		void *data;
		size_t data_len;
		byte_count+=3;  // need one byte for encoding type, two bytes for BOM
		ret = frame->NewData(byte_count, &data, &data_len);
		if (ret != NErr_Success)
			return ret;

		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data, data_len);
		bytewriter_write_u8(&byte_writer, 1); /* mark as UTF-16LE */
		bytewriter_write_u16_le(&byte_writer, 0xFEFF); /* BOM */

		size_t bytes_copied;
		return NXStringGetBytes(&bytes_copied, value, bytewriter_pointer(&byte_writer), bytewriter_size(&byte_writer), nx_charset_utf16le, 0);
	}

	return NErr_Empty;
}

int NSID3v2_Tag_Text_Set(nsid3v2_tag_t t, int frame_enum, nx_string_t value, int text_flags)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;

	ID3v2::Frame *frame = tag->FindFirstFrame(frame_enum);
	if (!frame)
	{
		frame = tag->NewFrame(frame_enum, 0);
		if (!frame)
			return NErr_OutOfMemory;
		tag->AddFrame(frame);
	}
	
	return NSID3v2_Frame_Text_Set((nsid3v2_frame_t)frame, value, text_flags);
}
