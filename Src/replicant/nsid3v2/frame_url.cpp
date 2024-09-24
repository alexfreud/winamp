#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nu/ByteReader.h"
#include "nx/nxstring.h"
#include "nsid3v2/frame_utils.h"

static int ParseText(const void *data, size_t data_len, ParsedString &parsed)
{
	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);
	
	return ParseFrameTerminatedString(&byte_reader, 0, parsed);
}

int NSID3v2_Frame_URL_Get(const nsid3v2_frame_t f, nx_string_t *value, int text_flags)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedString parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && ParseText(data, data_len, parsed) == NErr_Success)
		{
			return NXStringCreateFromParsedString(value, parsed, text_flags);
		}
	}

	return NErr_Empty;
}

int NSID3v2_Tag_URL_Get(const nsid3v2_tag_t t, int frame_enum, nx_string_t *value, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;
	const ID3v2::Frame *frame = tag->FindFirstFrame(frame_enum);
	return NSID3v2_Frame_URL_Get((const nsid3v2_frame_t)frame, value, text_flags);
}


