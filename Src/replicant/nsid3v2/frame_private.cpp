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


struct ParsedPrivate
{
	ParsedString owner;
	const void *private_data;
	size_t private_byte_length;
};

static int ParsePrivate(const void *data, size_t data_len, ParsedPrivate &parsed)
{
	if (data_len == 0)
		return NErr_Insufficient;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);

	int ret = ParseNullTerminatedString(&byte_reader, 0, parsed.owner);
	if (ret != NErr_Success)
		return ret;

	parsed.private_data = bytereader_pointer(&byte_reader);
	parsed.private_byte_length = bytereader_size(&byte_reader);

	return NErr_Success;
}

int NSID3v2_Frame_Private_Get(const nsid3v2_frame_t f, nx_string_t *description, const void **out_data, size_t *length)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPrivate parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParsePrivate(data, data_len, parsed) == NErr_Success)
		{
			int ret = NXStringCreateFromParsedString(description, parsed.owner, 0);
			if (ret != NErr_Success)
				return ret;

			*out_data = parsed.private_data;
			*length = parsed.private_byte_length;

			return NErr_Success;
		}

	}
	return NErr_Empty;
}
