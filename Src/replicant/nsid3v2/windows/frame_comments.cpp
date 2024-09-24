#include "nsid3v2/nsid3v2.h"
#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"
#include "nsid3v2/frame_utils.h"
#include "nu/AutoWide.h"
#include <strsafe.h>

struct ParsedComments
{
	const char *language;
	uint8_t description_encoding; // 0 - iso-8859-1, 1 - UTF16LE, 2 - UTF16BE, 3 - UTF8
	union 
	{
		const char *as8;
		const wchar_t *as16;
	} description_data;
	size_t description_cch;
	uint8_t value_encoding; // 0 - iso-8859-1, 1 - UTF16LE, 2 - UTF16BE, 3 - UTF8
	union 
	{
		const char *as8;
		const wchar_t *as16;
	} value_data;
	size_t value_cch;
};

static int ParseComments(const void *data, size_t data_len, ParsedComments &parsed)
{
	int ret;
	if (data_len < 5)
		return NErr_Error;

	const uint8_t *data8 = (const uint8_t *)data;
	switch(data8[0])
	{
	case 0: // ISO-8859-1
		parsed.description_encoding = 0;
		parsed.language = (const char *)&data8[1];
		
		parsed.value_encoding = 0;
		parsed.description_data.as8 = (const char *)&data8[4];
		data_len-=4;

		ret = ParseDescription(parsed.description_data.as8, data_len, parsed.description_cch);
		if (ret != NErr_Success)
			return ret;

		parsed.value_data.as8 = parsed.description_data.as8 + 2 + parsed.description_cch;
		parsed.value_cch = data_len;

		return NErr_Success;
	case 1: // UTF-16
		parsed.language = (const char *)&data8[1];
		parsed.description_encoding=1;
		parsed.description_data.as16 = (const wchar_t *)&data8[4];
		data_len-=4;

		ret = ParseDescription(parsed.description_data.as16, data_len, parsed.description_cch, parsed.description_encoding);
		if (ret != NErr_Success)
			return ret;

		parsed.value_data.as16 = parsed.description_data.as16 + 2 + parsed.description_cch;
		parsed.value_cch = data_len/2;

		if (parsed.value_cch && parsed.value_data.as16[0] == 0xFFFE)
		{
			parsed.value_encoding=2;
			parsed.value_data.as16++;
			parsed.value_cch--;
		}
		else if (parsed.value_cch && parsed.value_data.as16[0] == 0xFEFF)
		{
			parsed.value_encoding=1;
			parsed.value_data.as16++;
			parsed.value_cch--;
		}
		else
		{
			parsed.value_encoding=1;
		}

		return NErr_Success;

	case 2: // UTF-16 BE
		parsed.language = (const char *)&data8[1];
		parsed.description_encoding=2;
		parsed.description_data.as16 = (const wchar_t *)&data8[4];
		data_len-=3;

		ret = ParseDescription(parsed.description_data.as16, data_len, parsed.description_cch, parsed.description_encoding);
		if (ret != NErr_Success)
			return ret;

		parsed.value_data.as16 = parsed.description_data.as16 + 2 + parsed.description_cch;
		parsed.value_cch = data_len/2;
		parsed.value_encoding=2;

		return NErr_Success;
	case 3: // UTF-8
		parsed.description_encoding = 3;
		parsed.language = (const char *)&data8[1];
		parsed.value_encoding = 3;
		parsed.description_data.as8 = (const char *)&data8[4];
		data_len-=4;

		ret = ParseDescription(parsed.description_data.as8, data_len, parsed.description_cch);
		if (ret != NErr_Success)
			return ret;

		// check for UTF-8 BOM
		if (parsed.description_cch >= 3 && parsed.description_data.as8[0] == 0xEF && parsed.description_data.as8[1] == 0xBB && parsed.description_data.as8[2] == 0xBF)
		{
			parsed.description_data.as8+=3;
			parsed.description_cch-=3;
		}

		if (!data_len)
			return NErr_Error;

		parsed.value_data.as8 = parsed.description_data.as8 + 2 + parsed.description_cch;
		parsed.value_cch = data_len;

		// check for UTF-8 BOM
		if (parsed.value_cch >= 3 && parsed.value_data.as8[0] == 0xEF && parsed.value_data.as8[1] == 0xBB && parsed.value_data.as8[2] == 0xBF)
		{
			parsed.value_data.as8+=3;
			parsed.value_cch-=3;
		}

		return NErr_Success;
	}
	return NErr_NotImplemented;
}


int NSID3v2_Tag_Comments_GetUTF16(const nsid3v2_tag_t t, const wchar_t *description, wchar_t *buf, size_t buf_cch, int text_flags)
{
	const ID3v2::Tag *tag = (const ID3v2::Tag *)t;
	const ID3v2::Frame *frame = tag->FindFirstFrame(NSID3V2_FRAME_COMMENTS);
	while (frame)
	{
		const void *data;
		size_t data_len;
		ParsedComments parsed;
		if (frame->GetData(&data, &data_len) == NErr_Success && data_len > 0 && ParseComments(data, data_len, parsed) == NErr_Success)
		{
			// see if our description matches
			switch(parsed.description_encoding)
			{
			case 0:  // ISO-8859-1
				{
					UINT codepage = (text_flags & NSID3V2_TEXT_SYSTEM)?28591:CP_ACP;
					if (CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, AutoWide(parsed.description_data.as8, codepage), -1, description, -1) != CSTR_EQUAL)
						goto next_frame;
				}
				break;
			case 1:
				if (CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, parsed.description_data.as16, -1, description, -1) != CSTR_EQUAL)
					goto next_frame;
				break;
			case 2:
				// TODO!
				goto next_frame;
				break;
			case 3:		
				if (CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, AutoWide(parsed.description_data.as8, CP_UTF8), -1, description, -1) != CSTR_EQUAL)
					goto next_frame;
				break;
			}

			switch(parsed.value_encoding)
			{
			case 0: // ISO-8859-1
				{
					UINT codepage = (text_flags & NSID3V2_TEXT_SYSTEM)?28591:CP_ACP;
					int utf16_len = MultiByteToWideChar(codepage, 0, parsed.value_data.as8, parsed.value_cch, 0, 0);

					if (utf16_len)
					{
						utf16_len = MultiByteToWideChar(codepage, 0, parsed.value_data.as8, parsed.value_cch, buf, utf16_len-1);
						buf[utf16_len]=0;
					}
					else
					{
						buf[0]=0;
					}
				}
				return NErr_Success;
			case 1: // UTF-16
				StringCchCopyNW(buf, buf_cch, parsed.value_data.as16, parsed.value_cch);
				return NErr_Success;
			case 2: // UTF-16BE
				{
					size_t toCopy = buf_cch-1;
					if (parsed.value_cch < toCopy)
						toCopy = parsed.value_cch;
					for (size_t i=0;i<toCopy;i++)
					{
						buf[i] = ((parsed.value_data.as16[i] >> 8) & 0xFF) | (((parsed.value_data.as16[i]) & 0xFF) << 8);
					}
					buf[toCopy]=0;
				}
				return NErr_Success;
			case 3: // UTF-8
				{
					int utf16_len = MultiByteToWideChar(CP_UTF8, 0, parsed.value_data.as8, parsed.value_cch, 0, 0);

					if (utf16_len)
					{
						utf16_len = MultiByteToWideChar(CP_UTF8, 0, parsed.value_data.as8, parsed.value_cch, buf, utf16_len-1);
						buf[utf16_len]=0;
					}
					else
					{
						buf[0]=0;
					}
				}
				return NErr_Success;
			}

next_frame:
			frame = tag->FindNextFrame(frame);
		}
	}

	return NErr_Error;
}

