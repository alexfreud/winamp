#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nsid3v2/frame_utils.h"
#include "nu/ByteReader.h"
#include "nx/nxstring.h"
#if defined(_WIN32) && !defined(strcasecmp)
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

struct ParsedPopularimeter
{
	ParsedString email;
	uint8_t rating;
	uint64_t playcount;
};

static int ParsePopularimeter(const void *data, size_t data_len, ParsedPopularimeter &parsed)
{
	int ret;
	if (data_len < 6)
		return NErr_Insufficient;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);

	/* read email (Always latin-1) */
	ret = ParseNullTerminatedString(&byte_reader, 0, parsed.email);
	if (ret != NErr_Success)
		return ret;

	if (bytereader_size(&byte_reader) == 0)
		return NErr_Insufficient;

	parsed.rating = bytereader_read_u8(&byte_reader);

	parsed.playcount=0;
	while (bytereader_size(&byte_reader))
	{

		parsed.playcount <<= 8;
		parsed.playcount |= bytereader_read_u8(&byte_reader);
	}
	return NErr_Success;
}

int NSID3v2_Tag_Popularimeter_GetRatingPlaycount(const nsid3v2_tag_t t, const char *email, uint8_t *rating, uint64_t *playcount)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	if (!tag)
		return NErr_Empty;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_POPULARIMETER);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPopularimeter parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParsePopularimeter(data, data_len, parsed) == NErr_Success)
		{
			if (!strcasecmp(email, (const char *)parsed.email.data))
			{
				*rating = parsed.rating;
				*playcount = parsed.playcount;
				return NErr_Success;
			}

		}
		frame = tag->FindNextFrame(frame);
	}

	return NErr_Empty;
}

int NSID3v2_Frame_Popularity_Get(nsid3v2_frame_t f, nx_string_t *email, uint8_t *rating, uint64_t *playcount, int text_flags)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPopularimeter parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParsePopularimeter(data, data_len, parsed) == NErr_Success)
		{
			int ret = NXStringCreateFromParsedString(email, parsed.email, text_flags);
			if (ret != NErr_Success)
				return ret;
			*rating = parsed.rating;
			*playcount = parsed.playcount;
			return NErr_Success;
		}
	}
	return NErr_Empty;
}
