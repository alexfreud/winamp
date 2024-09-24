#pragma once
#include "foundation/types.h"
#include "nu/ByteReader.h"
#include "nx/nxstring.h"


/* updates str, data_len and str_cch */
int ParseDescription(const char *&str, size_t &data_len, size_t &str_cch);
int ParseDescription(const wchar_t *&str, size_t &data_len, size_t &str_cch, uint8_t &str_encoding);

struct ParsedString
{
	uint8_t encoding; // 0 - iso-8859-1, 1 - UTF16LE, 2 - UTF16BE, 3 - UTF8
	const void *data;
	size_t byte_length;
};

int ParseNullTerminatedString(bytereader_t reader, uint8_t encoding, ParsedString &parsed);
int ParseFrameTerminatedString(bytereader_t reader, uint8_t encoding, ParsedString &parsed);
int NXStringCreateFromParsedString(nx_string_t *value, ParsedString &parsed, int text_flags);
bool DescriptionMatches(const ParsedString &parsed, const char *description, int text_flags);
