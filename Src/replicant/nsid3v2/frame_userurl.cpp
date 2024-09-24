#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nsid3v2/frame_utils.h"
#include "nu/ByteReader.h"
#include "nx/nxstring.h"
#if defined(_WIN32) && !defined(strcasecmp)
#define strcasecmp _stricmp
#else
#include <string.h>
#endif

struct ParsedUserURL
{
	ParsedString description;
	ParsedString value;
};

static int ParseUserURL(const void *data, size_t data_len, ParsedUserURL &parsed)
{
	int ret;
	if (data_len < 2)
		return NErr_Insufficient;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);

	uint8_t encoding = bytereader_read_u8(&byte_reader);

	ret = ParseNullTerminatedString(&byte_reader, encoding, parsed.description);
	if (ret != NErr_Success)
		return ret;

	return ParseFrameTerminatedString(&byte_reader, 0, parsed.value);
}

int NSID3v2_Tag_WXXX_Get(const nsid3v2_tag_t t, const char *description, nx_string_t *value, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_USER_TEXT);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedUserURL parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseUserURL(data, data_len, parsed) == NErr_Success && DescriptionMatches(parsed.description, description, text_flags))
		{
			return NXStringCreateFromParsedString(value, parsed.value, text_flags);
		}

		frame = tag->FindNextFrame(frame);
	}

	return NErr_Empty;
}

int NSID3v2_Frame_UserURL_Get(const nsid3v2_frame_t f, nx_string_t *description, nx_string_t *value, int text_flags)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedUserURL parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseUserURL(data, data_len, parsed) == NErr_Success)
		{
			int ret = NXStringCreateFromParsedString(value, parsed.value, text_flags);
			if (ret != NErr_Success)
				return ret;

			return NXStringCreateFromParsedString(description, parsed.description, text_flags);
		}

	}
	return NErr_Error;
}
