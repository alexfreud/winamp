#include "nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nsid3v2/frame_utils.h"
#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include "nx/nxstring.h"

struct ParsedPicture
{
	ParsedString mime;
	uint8_t picture_type;
	ParsedString description;
	const void *picture_data;
	size_t picture_byte_length;
};

static int ParsePicture(const void *data, size_t data_len, ParsedPicture &parsed)
{
	int ret;
	if (data_len < 4)
		return NErr_NeedMoreData;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);

	uint8_t encoding = bytereader_read_u8(&byte_reader);
	/* mime type is always latin-1 */
	ret = ParseNullTerminatedString(&byte_reader, 0, parsed.mime);
	if (ret != NErr_Success)
		return ret;

	if (bytereader_size(&byte_reader) < 2)
		return NErr_NeedMoreData;

	parsed.picture_type = bytereader_read_u8(&byte_reader);

	ret = ParseNullTerminatedString(&byte_reader, encoding, parsed.description);
	if (ret != NErr_Success)
		return ret;

	parsed.picture_data = bytereader_pointer(&byte_reader);
	parsed.picture_byte_length = bytereader_size(&byte_reader);
	return NErr_Success;
}

static int ParsePicturev2_2(const void *data, size_t data_len, ParsedPicture &parsed)
{
	int ret;
	if (data_len < 6)
		return NErr_NeedMoreData;

	bytereader_value_t byte_reader;
	bytereader_init(&byte_reader, data, data_len);

	uint8_t encoding = bytereader_read_u8(&byte_reader);

	/* three byte "Image Format" field */
	parsed.mime.encoding = 0;
	parsed.mime.data = bytereader_pointer(&byte_reader);
	parsed.mime.byte_length = 3;

	bytereader_advance(&byte_reader, 3);

	parsed.picture_type = bytereader_read_u8(&byte_reader);

	ret = ParseNullTerminatedString(&byte_reader, encoding, parsed.description);
	if (ret != NErr_Success)
		return ret;

	parsed.picture_data = bytereader_pointer(&byte_reader);
	parsed.picture_byte_length = bytereader_size(&byte_reader);
	return NErr_Success;
}

int NSID3v2_Frame_Picture_Get(const nsid3v2_frame_t f, nx_string_t *mime, uint8_t *picture_type, nx_string_t *description, const void **picture_data, size_t *length, int text_flags)
{
	const ID3v2::Frame *frame = (const ID3v2::Frame *)f;
	if (frame)
	{
		const void *data;
		size_t data_len;
		ParsedPicture parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0)		
		{
			int ret;
			if (frame->GetVersion() == 2)
				ret = ParsePicturev2_2(data, data_len, parsed);
			else
				ret = ParsePicture(data, data_len, parsed);

			if (ret == NErr_Success)
			{
				int ret;
				if (mime)
				{
					ret = NXStringCreateFromParsedString(mime, parsed.mime, text_flags);
					if (ret != NErr_Success)
						return ret;
				}

				if (description)
				{
					ret = NXStringCreateFromParsedString(description, parsed.description, text_flags);
					if (ret != NErr_Success)
						return ret;
				}

				if (picture_type)
					*picture_type = parsed.picture_type;

				if (picture_data)
					*picture_data = parsed.picture_data;
				if (length)
					*length = parsed.picture_byte_length;

				return NErr_Success;
			}
			else
			{
				return ret;
			}
		}

	}
	return NErr_Empty;
}

/* ---------------- Setters ---------------- */
static const char *GetMIME2_2(nx_string_t mime)
{
	if (!mime)
		return "\0\0\0";

	if (NXStringKeywordCompareWithCString(mime, "image/jpeg") == NErr_True	|| NXStringKeywordCompareWithCString(mime, "image/jpg") == NErr_True)
		return "JPG";

	if (NXStringKeywordCompareWithCString(mime, "image/png") == NErr_True)
		return "PNG";

	if (NXStringKeywordCompareWithCString(mime, "image/gif") == NErr_True)
		return "GIF";

	if (NXStringKeywordCompareWithCString(mime, "image/bmp") == NErr_True)
		return "BMP";

	return "\0\0\0";
}

int NSID3v2_Frame_Picture_Set(nsid3v2_frame_t f, nx_string_t mime, uint8_t picture_type, nx_string_t description, const void *picture_data, size_t length, int text_flags)
{
	ID3v2::Frame *frame = (ID3v2::Frame *)f;
	if (frame)
	{
		if (frame->GetVersion() == 2)
		{
			/* first, we need to get the total encoded size */

			size_t byte_count_description=0;
			if (description)
			{
				int ret = NXStringGetBytesSize(&byte_count_description, description, nx_charset_latin1, 0);
				if (ret != NErr_DirectPointer && ret != NErr_Success)
					return ret;
			}

			size_t total_size = 1 /* text encoding */ 
				+ 3 /* Image Format is 3 bytes in ID3v2.2*/
				+ 1 /* picture type */
				+ byte_count_description + 1 /* description + null terminator */
				+ length; /* picture length */

			void *data;
			size_t data_size;
			int ret = frame->NewData(total_size, &data, &data_size);
			if (ret != NErr_Success)
				return ret;

			size_t bytes_copied;
			bytewriter_s byte_writer;
			bytewriter_init(&byte_writer, data, data_size);
			bytewriter_write_u8(&byte_writer, 0); /* mark as Latin-1 */
			bytewriter_write_n(&byte_writer, GetMIME2_2(mime), 3);
			bytewriter_write_u8(&byte_writer, picture_type); 
			if (description)
			{
				NXStringGetBytes(&bytes_copied, description, bytewriter_pointer(&byte_writer), bytewriter_size(&byte_writer), nx_charset_latin1, 0);
				bytewriter_advance(&byte_writer, bytes_copied);
			}
			bytewriter_write_u8(&byte_writer, 0); /* description null terminator */
			bytewriter_write_n(&byte_writer, picture_data, length);
			return NErr_Success;
		}
		else
		{
			/* first, we need to get the total encoded size */
			size_t byte_count_mime=0;
			if (mime)
			{
				int ret = NXStringGetBytesSize(&byte_count_mime, mime, nx_charset_latin1, 0);
				if (ret != NErr_DirectPointer && ret != NErr_Success)
					return ret;
			}

			size_t byte_count_description=0;
			if (description)
			{
				int ret = NXStringGetBytesSize(&byte_count_description, description, nx_charset_latin1, 0);
				if (ret != NErr_DirectPointer && ret != NErr_Success)
					return ret;
			}

			size_t total_size = 1 /* text encoding */ 
				+ byte_count_mime + 1 /* mime + null terminator */
				+ 1 /* picture type */
				+ byte_count_description + 1 /* description + null terminator */
				+ length; /* picture length */

			void *data;
			size_t data_size;
			int ret = frame->NewData(total_size, &data, &data_size);
			if (ret != NErr_Success)
				return ret;

			size_t bytes_copied;
			bytewriter_s byte_writer;
			bytewriter_init(&byte_writer, data, data_size);
			bytewriter_write_u8(&byte_writer, 0); /* mark as Latin-1 */
			if (mime)
			{
				NXStringGetBytes(&bytes_copied, mime, bytewriter_pointer(&byte_writer), bytewriter_size(&byte_writer), nx_charset_latin1, 0);
				bytewriter_advance(&byte_writer, bytes_copied);
			}
			bytewriter_write_u8(&byte_writer, 0); /* MIME null terminator */
			bytewriter_write_u8(&byte_writer, picture_type); 
			if (description)
			{
				NXStringGetBytes(&bytes_copied, description, bytewriter_pointer(&byte_writer), bytewriter_size(&byte_writer), nx_charset_latin1, 0);
				bytewriter_advance(&byte_writer, bytes_copied);
			}
			bytewriter_write_u8(&byte_writer, 0); /* description null terminator */
			bytewriter_write_n(&byte_writer, picture_data, length);
			return NErr_Success;
		}
	}
	return NErr_Empty;
}
