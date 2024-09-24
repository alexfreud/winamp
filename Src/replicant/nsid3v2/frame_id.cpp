#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nsid3v2/frame_utils.h"
#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include "nx/nxstring.h"

struct ParsedID
{
	ParsedString owner;
	const void *identifier_data;
	size_t identifier_byte_length;
};

static int ParseID(const void *data, size_t data_len, ParsedID &parsed)
{
	int ret;
	if (data_len < 1)
		return NErr_Insufficient;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);

	/* owner is always latin-1 */
	ret = ParseNullTerminatedString(&byte_reader, 0, parsed.owner);
	if (ret != NErr_Success)
		return ret;
	parsed.identifier_data = bytereader_pointer(&byte_reader);
	parsed.identifier_byte_length = bytereader_size(&byte_reader);
	return NErr_Success;
}

int NSID3v2_Tag_ID_Find(const nsid3v2_tag_t t, const char *owner, nsid3v2_frame_t *out_frame, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_ID);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedID parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseID(data, data_len, parsed) == NErr_Success && (!owner || DescriptionMatches(parsed.owner, owner, text_flags)))
		{
			*out_frame = (nsid3v2_frame_t)frame;
			return NErr_Success;
		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Empty;
}

int NSID3v2_Frame_ID_Get(nsid3v2_frame_t f, nx_string_t *owner, const void **id_data, size_t *length, int text_flags)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedID parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseID(data, data_len, parsed) == NErr_Success)
		{
			if (owner)
			{
				int ret = NXStringCreateFromParsedString(owner, parsed.owner, text_flags);
				if (ret != NErr_Success)
					return ret;
			}

			*id_data = parsed.identifier_data;
			*length = parsed.identifier_byte_length;

			return NErr_Success;
		}

	}
	return NErr_Empty;
}

int NSID3v2_Tag_ID_Get(const nsid3v2_tag_t t, const char *owner, const void **id_data, size_t *length, int text_flags)
{
		const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;

	ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_ID);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedID parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseID(data, data_len, parsed) == NErr_Success && (!owner || DescriptionMatches(parsed.owner, owner, text_flags)))
		{
			*id_data = parsed.identifier_data;
			*length = parsed.identifier_byte_length;

			return NErr_Success;
		}

		frame = tag->FindNextFrame(frame);
	}
	return NErr_Empty;
}



/* ---------------- Setters ---------------- */
int NSID3v2_Frame_ID_Set(nsid3v2_frame_t f, const char *owner, const void *id_data, size_t length, int text_flags)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	if (frame)
	{
		size_t owner_length=owner?strlen(owner):0;

		/* TODO: overflow check */
		size_t total_size = owner_length + 1 + length;

		void *data;
		size_t data_len;
		int ret = frame->NewData(total_size, &data, &data_len);
		if (ret != NErr_Success)
			return ret;

		bytewriter_s byte_writer;
		bytewriter_init(&byte_writer, data, data_len);
		bytewriter_write_n(&byte_writer, owner, owner_length);
		bytewriter_write_u8(&byte_writer, 0); // write null terminator separately, in case owner is NULL
		bytewriter_write_n(&byte_writer, id_data, length);

		return NErr_Success;
	}
	return NErr_Empty;
}

int NSID3v2_Tag_ID_Set(nsid3v2_tag_t t, const char *owner, const void *id_data, size_t length, int text_flags)
{
	ID3v2::Tag *tag = (ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;

	ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_ID);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedID parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseID(data, data_len, parsed) == NErr_Success && (!owner || DescriptionMatches(parsed.owner, owner, text_flags)))
		{
			break;
		}

		frame = tag->FindNextFrame(frame);
	}

	if (!frame)
	{
		frame = tag->NewFrame(NSID3V2_FRAME_ID, 0);
		if (!frame)
			return NErr_OutOfMemory;
		tag->AddFrame(frame);
	}

	return NSID3v2_Frame_ID_Set((nsid3v2_frame_t)frame, owner, id_data, length, text_flags);	
}
