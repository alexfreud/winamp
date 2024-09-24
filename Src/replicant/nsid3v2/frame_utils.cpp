#include "frame_utils.h"
#include "foundation/error.h"
#include "nsid3v2/nsid3v2.h"
#if defined(_WIN32) && !defined(strcasecmp)
#define strcasecmp _stricmp
#else
#include <string.h>
#endif

int ParseDescription(const char *&str, size_t &data_len, size_t &str_cch)
{
	str_cch=0;
	while (data_len && str[str_cch])
	{
		data_len--;
		str_cch++;
	}
	if (!data_len)
		return NErr_Error;

	data_len--;
	return NErr_Success;
}

int ParseDescription(const wchar_t *&str, size_t &data_len, size_t &str_cch, uint8_t &str_encoding)
{
	str_cch=0;
	if (data_len > 2 && str[0] == 0xFFFE)
	{
		str_encoding=2;
		str++;
		str-=3;
	}
	else if (data_len > 2 && str[0] == 0xFEFF)
	{
		str_encoding=1;
		str++;
		data_len-=3;
	}
	else
	{
		data_len--;
	}

	while (data_len > 1 && str[str_cch])
	{
		data_len-=2;
		str_cch++;
	}

	if (!data_len)
		return NErr_Error;

	data_len-=2;
	return NErr_Success;
}


static void ParseBOM(bytereader_t reader, uint8_t *encoding, const uint8_t default_encoding)
{
	if (bytereader_size(reader) >= 2)
	{
		uint16_t bom = bytereader_show_u16_le(reader);
		if (bom == 0xFFFE)
		{
			bytereader_advance(reader, 2);
			*encoding=2;
		}
		else  if (bom == 0xFEFF)
		{
			bytereader_advance(reader, 2);
			*encoding=1;
		}
		else
		{
			*encoding=default_encoding;
		}
	}
	else
	{
		*encoding=default_encoding;
	}
}

int ParseNullTerminatedString(bytereader_t reader, uint8_t encoding, ParsedString &parsed)
{
	switch(encoding)
	{
	case 0: // ISO-8859-1
		if (bytereader_size(reader) == 0)
			return NErr_Insufficient;

		parsed.encoding = 0;
		parsed.data = bytereader_pointer(reader);
		parsed.byte_length = 0;
		while (bytereader_size(reader) && bytereader_read_u8(reader))
			parsed.byte_length++;

		return NErr_Success;
	case 1: // UTF-16
		if (bytereader_size(reader) < 2)
			return NErr_Insufficient;

		parsed.byte_length = 0;
		ParseBOM(reader, &parsed.encoding, 1);
		parsed.data = bytereader_pointer(reader);
		while (bytereader_size(reader) && bytereader_read_u16_le(reader))
			parsed.byte_length+=2;

		return NErr_Success;
	case 2: // UTF-16BE
		if (bytereader_size(reader) < 2)
			return NErr_Insufficient;

		parsed.byte_length = 0;
		ParseBOM(reader, &parsed.encoding, 2);
		parsed.data = bytereader_pointer(reader);
		while (bytereader_size(reader) && bytereader_read_u16_le(reader))
			parsed.byte_length+=2;

		return NErr_Success; 
	case 3: // UTF-8
		if (bytereader_size(reader) == 0)
			return NErr_Insufficient;

		parsed.encoding = 3;
		parsed.data = bytereader_pointer(reader);
		parsed.byte_length = 0;

		size_t start = bytereader_size(reader);
#if 0 // TODO
		/* check for UTF-8 BOM and skip it */
		if (bytereader_size(reader) > 3 && bytereader_read_u8(reader) == 0xEF && bytereader_read_u8(reader) == 0xBB && bytereader_read_u8(reader) == 0xBF)
		{
			parsed.data = bytereader_pointer(reader);
			parsed.byte_length = bytereader_size(reader);
		}
		else
		{
			/* no BOM but skip however far we read into the string */
			size_t offset = start - bytereader_size(reader);
			parsed.data = (const uint8_t *)parsed.data + offset;
			parsed.byte_length -= offset;
		}
#endif
		/* finish it up */
		while (bytereader_size(reader) && bytereader_read_u8(reader))
			parsed.byte_length++;

		return NErr_Success;
	}
	return NErr_Unknown;
}

int ParseFrameTerminatedString(bytereader_t reader, uint8_t encoding, ParsedString &parsed)
{
	switch(encoding)
	{
	case 0: // ISO-8859-1
		parsed.encoding = 0;
		parsed.data = bytereader_pointer(reader);
		parsed.byte_length = bytereader_size(reader);
		return NErr_Success;
	case 1: // UTF-16
		if ((bytereader_size(reader) & 1) == 1) 
			return NErr_Error;
		ParseBOM(reader, &parsed.encoding, 1);
		parsed.data = bytereader_pointer(reader);
		parsed.byte_length = bytereader_size(reader);
		return NErr_Success;
	case 2: // UTF-16BE
		if ((bytereader_size(reader) & 1) == 1) 
			return NErr_Error;
		ParseBOM(reader, &parsed.encoding, 2);
		parsed.data = bytereader_pointer(reader);
		parsed.byte_length = bytereader_size(reader);
		return NErr_Success; 
	case 3: // UTF-8
		parsed.encoding = 3;
		parsed.data = bytereader_pointer(reader);
		parsed.byte_length = bytereader_size(reader);
		if (bytereader_size(reader) > 3 && bytereader_read_u8(reader) == 0xEF && bytereader_read_u8(reader) == 0xBB && bytereader_read_u8(reader) == 0xBF)
		{
			parsed.data = bytereader_pointer(reader);
			parsed.byte_length = bytereader_size(reader);
		}
		return NErr_Success;
	}
	return NErr_Error;
}

int NXStringCreateFromParsedString(nx_string_t *value, ParsedString &parsed, int text_flags)
{
	switch(parsed.encoding)
	{
	case 0: // ISO-8859-1
		if (parsed.byte_length == 0)
			return NXStringCreateEmpty(value);
		if (text_flags & NSID3V2_TEXT_SYSTEM)
			return NXStringCreateWithBytes(value, parsed.data, parsed.byte_length, nx_charset_system);
		else
			return NXStringCreateWithBytes(value, parsed.data, parsed.byte_length, nx_charset_latin1);
	case 1: // UTF-16
		if (parsed.byte_length < 2)
			return NXStringCreateEmpty(value);
		return NXStringCreateWithBytes(value, parsed.data, parsed.byte_length, nx_charset_utf16le);
	case 2: // UTF-16BE
		if (parsed.byte_length < 2)
			return NXStringCreateEmpty(value);
		return NXStringCreateWithBytes(value, parsed.data, parsed.byte_length, nx_charset_utf16be);
	case 3: // UTF-8
		if (parsed.byte_length == 0)
			return NXStringCreateEmpty(value);
		return NXStringCreateWithBytes(value, parsed.data, parsed.byte_length, nx_charset_utf8);
	default:
		return NErr_Unknown;
	}
	
}

bool DescriptionMatches(const ParsedString &parsed, const char *description, int text_flags)
{
	// see if our description matches
	switch(parsed.encoding)
	{
	case 0:  // ISO-8859-1
		return !strcasecmp(description, (const char *)parsed.data);
	case 1: 
		{
			bytereader_value_t utf16;
			bytereader_init(&utf16, parsed.data, parsed.byte_length);
			
			while (*description && bytereader_size(&utf16))
			{
				if ((*description++ & ~0x20) != (bytereader_read_u16_le(&utf16) & ~0x20))
					return false;
			}

			if (*description == 0 && bytereader_size(&utf16) == 0)
				return true;
			else
				return false;
		}

	case 2:
		{
			bytereader_value_t utf16;
			bytereader_init(&utf16, parsed.data, parsed.byte_length);
			
			while (*description && bytereader_size(&utf16))
			{
				if ((*description++ & ~0x20) != (bytereader_read_u16_be(&utf16) & ~0x20))
					return false;
			}
			if (*description == 0 && bytereader_size(&utf16) == 0)
				return true;
			else
				return false;
		}
	case 3:
		return !strcasecmp(description, (const char *)parsed.data);
	}

	return false;
}
