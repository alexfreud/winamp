#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nsid3v2/frame_utils.h"
#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include "nx/nxstring.h"

struct ParsedUserText
{
	ParsedString description;
	ParsedString value;
};

static int ParseUserText(const void *data, size_t data_len, ParsedUserText &parsed)
{
	int ret;
	if (data_len == 0)
		return NErr_Insufficient;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);

	uint8_t encoding = bytereader_read_u8(&byte_reader);

	ret = ParseNullTerminatedString(&byte_reader, encoding, parsed.description);
	if (ret != NErr_Success)
		return ret;

	return ParseFrameTerminatedString(&byte_reader, encoding, parsed.value);
}

int NSID3v2_Tag_TXXX_Find(const nsid3v2_tag_t t, const char *description, nsid3v2_frame_t *out_frame, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_USER_TEXT);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedUserText parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseUserText(data, data_len, parsed) == NErr_Success && DescriptionMatches(parsed.description, description, text_flags))
		{
			*out_frame = (nsid3v2_frame_t)frame;
			return NErr_Success;
		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Empty;
}

int NSID3v2_Tag_TXXX_Get(const nsid3v2_tag_t t, const char *description, nx_string_t *value, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_USER_TEXT);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedUserText parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseUserText(data, data_len, parsed) == NErr_Success && DescriptionMatches(parsed.description, description, text_flags))
		{
			return NXStringCreateFromParsedString(value, parsed.value, text_flags);
		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Empty;
}

int NSID3v2_Frame_UserText_Get(const nsid3v2_frame_t f, nx_string_t *description, nx_string_t *value, int text_flags)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedUserText parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseUserText(data, data_len, parsed) == NErr_Success)
		{
			int ret = NXStringCreateFromParsedString(value, parsed.value, text_flags);
			if (ret != NErr_Success)
				return ret;

			if (description)
				return NXStringCreateFromParsedString(description, parsed.description, text_flags);
			else
				return NErr_Success;
		}

	}
	return NErr_Error;
}
/* ---------------- Setters ---------------- */
int NSID3v2_Frame_UserText_Set(nsid3v2_frame_t f, const char *description, nx_string_t value, int text_flags)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	if (frame)
	{
		/* benski> for now, we're going to store UTF-16LE always. in the future, we'll add functions to NXString to determine a 'best' encoding */
		size_t description_length=strlen(description);

		size_t byte_count_value=0;
		int ret = NXStringGetBytesSize(&byte_count_value, value, nx_charset_utf16le, 0);
		if (ret != NErr_DirectPointer && ret != NErr_Success)
			return ret;

		/* TODO: overflow check */
		size_t total_size = 1 /* encoding */ + 2 /* BOM for description */ + description_length*2 + 2 /* null separator */ + 2 /* BOM for value */ + byte_count_value;

		void *data;
		size_t data_len;
		ret = frame->NewData(total_size, &data, &data_len);
		if (ret != NErr_Success)
			return ret;

		size_t bytes_copied;

		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data, data_len);
		bytewriter_write_u8(&byte_writer, 1); /* mark as UTF-16LE */
		bytewriter_write_u16_le(&byte_writer, 0xFEFF); /* BOM for description */
		for (size_t i=0;i<description_length;i++)
			bytewriter_write_u16_le(&byte_writer, description[i]);
		bytewriter_write_u16_le(&byte_writer, 0); /* NULL separator*/
		bytewriter_write_u16_le(&byte_writer, 0xFEFF); /* BOM for value */
		NXStringGetBytes(&bytes_copied, value, bytewriter_pointer(&byte_writer), bytewriter_size(&byte_writer), nx_charset_utf16le, 0);
		return NErr_Success;
	}
	return NErr_Error;
}

int NSID3v2_Tag_TXXX_Set(nsid3v2_tag_t t, const char *description, nx_string_t value, int text_flags)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;

	ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_USER_TEXT);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedUserText parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseUserText(data, data_len, parsed) == NErr_Success && DescriptionMatches(parsed.description, description, text_flags))
		{
			break;
		}
		frame = tag->FindNextFrame(frame);
	}

	if (!frame)
	{
		frame = tag->NewFrame(NSID3V2_FRAME_USER_TEXT, 0);
		if (!frame)
			return NErr_OutOfMemory;
		tag->AddFrame(frame);
	}

	return NSID3v2_Frame_UserText_Set((nsid3v2_frame_t)frame, description, value, text_flags);
}
