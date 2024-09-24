#include "header.h"
#include "flags.h"
#include "util.h"
#include <limits.h>

static char preamble[] =  { 'A', 'P', 'E', 'T', 'A', 'G', 'E', 'X' };

APEv2::Header::Header(void *data)
{
	memcpy(&headerData, data, sizeof(headerData));

	// covert to host endian
	//headerData.preamble=htons(headerData.preamble);
	headerData.version=ATON16(headerData.version);
	headerData.size=ATON32(headerData.size);
	headerData.items=ATON32(headerData.items);
	headerData.flags=ATON32(headerData.flags);
}

APEv2::Header::Header(const HeaderData *data)
{
	memcpy(&headerData.preamble, preamble, sizeof(headerData.preamble));
	headerData.version = NTOA32(2000);
	headerData.size = data->size;
	headerData.items = data->items;
	headerData.flags = data->flags;
	headerData.reserved = 0;
}

uint32_t APEv2::Header::GetFlags()
{
	return headerData.flags;
}

bool APEv2::Header::Valid()
{
	return !memcmp(&headerData.preamble, preamble, 8) && headerData.reserved == 0;
}

uint32_t APEv2::Header::TagSize()
{
	size_t size = headerData.size;
	if (IsHeader() && HasFooter())
		size+=SIZE;
	if (IsFooter() && HasHeader())
		size+=SIZE;

	if (size > ULONG_MAX)
		return 0;
		
	return (uint32_t)size;
}

bool APEv2::Header::HasHeader()
{
	return !!(headerData.flags & FLAG_HEADER_HAS_HEADER);
}

bool APEv2::Header::HasFooter()
{
	return !(headerData.flags & FLAG_HEADER_NO_FOOTER);
}

bool APEv2::Header::IsFooter()
{
	return !(headerData.flags & FLAG_HEADER_IS_HEADER);
}

bool APEv2::Header::IsHeader()
{
	return !!(headerData.flags & FLAG_HEADER_IS_HEADER);
}

int APEv2::Header::Encode(void *data, size_t len)
{
	if (len < 32)
		return APEV2_TOO_SMALL;

	HeaderData endianCorrectData = headerData;

		endianCorrectData.version=NTOA16(endianCorrectData.version);
	endianCorrectData.size=NTOA32(endianCorrectData.size);
	endianCorrectData.items=NTOA32(endianCorrectData.items);
	endianCorrectData.flags=NTOA32(endianCorrectData.flags);

	memcpy(data, &endianCorrectData, sizeof(endianCorrectData));
	return APEV2_SUCCESS;
}