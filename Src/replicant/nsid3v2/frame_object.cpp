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

struct ParsedObject
{
	ParsedString mime;
	ParsedString filename;
	ParsedString description;
	const void *object_data;
	size_t object_byte_length;
};

static int ParseObject(const void *data, size_t data_len, ParsedObject &parsed)
{
	int ret;
	if (data_len == 0)
		return NErr_Insufficient;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);

	/* encoding */
	uint8_t encoding = bytereader_read_u8(&byte_reader);

	/* read mime type (Always latin-1) */
	ret = ParseNullTerminatedString(&byte_reader, 0, parsed.mime);
	if (ret != NErr_Success)
		return ret;

	/* read filename */
	ret = ParseNullTerminatedString(&byte_reader, encoding, parsed.filename);
	if (ret != NErr_Success)
		return ret;

	/* read content description */
	ret = ParseNullTerminatedString(&byte_reader, encoding, parsed.description);
	if (ret != NErr_Success)
		return ret;


	parsed.object_data = bytereader_pointer(&byte_reader);
	parsed.object_byte_length = bytereader_size(&byte_reader);

	return NErr_Success;
}

int NSID3v2_Frame_Object_Get(const nsid3v2_frame_t f, nx_string_t *mime, nx_string_t *filename, nx_string_t *description, const void **out_data, size_t *length, int text_flags)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedObject parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseObject(data, data_len, parsed) == NErr_Success)
		{
			int ret = NXStringCreateFromParsedString(mime, parsed.mime, text_flags);
			if (ret != NErr_Success)
				return ret;

			ret = NXStringCreateFromParsedString(filename, parsed.filename, text_flags);
			if (ret != NErr_Success)
				return ret;

			ret = NXStringCreateFromParsedString(description, parsed.description, text_flags);
			if (ret != NErr_Success)
				return ret;

			*out_data = parsed.object_data;
			*length = parsed.object_byte_length;

			return NErr_Success;
		}

	}
	return NErr_Empty;
}
