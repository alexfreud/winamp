#include "extendedheader.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include "foundation/error.h"

ID3v2_21::ExtendedHeaderBase::ExtendedHeaderBase(const ID3v2::Header &_tagHeader) : tagHeader(_tagHeader)
{
	memset(&headerData, 0, sizeof(ExtendedHeaderData));
	data = 0;
	data_size = 0;
}

uint32_t ID3v2_21::ExtendedHeaderBase::Size() const
{
	return headerData.size;
}

int ID3v2_21::ExtendedHeaderBase::Parse(const void *_data, size_t len, size_t *bytes_read)
{
	if (len < SIZE)
		return 1;

	if (tagHeader.Unsynchronised())
	{
		*bytes_read = ID3v2::Util::UnsynchroniseTo(&headerData, _data, SIZE);
	}
	else
	{
		memcpy(&headerData, _data, SIZE);
		*bytes_read = SIZE;
	}

	_data = (const uint8_t *)_data+SIZE;

	/* read any data after the header */
	data_size = Size();
	if (data_size)
	{
		/* sanity check size */
		if (tagHeader.Unsynchronised())
		{
			if (ID3v2::Util::UnsynchronisedInputSize(_data, data_size) > len)
				return 1;
		}
		else if (data_size > len)
			return 1;

		/* allocate and read data */
		data = malloc(data_size);
		if (tagHeader.Unsynchronised())
		{
			*bytes_read += ID3v2::Util::UnsynchroniseTo(data, _data, data_size);
		}
		else
		{
			memcpy(data, _data, data_size);
			*bytes_read += data_size;
		}
	}

	return 0;
}

/* === ID3v2.3 === */
ID3v2_3::ExtendedHeader::ExtendedHeader(const ID3v2::Header &_tagHeader) : ID3v2_21::ExtendedHeaderBase(_tagHeader)
{
}

/* === ID3v2.4 === */
ID3v2_4::ExtendedHeader::ExtendedHeader(const ID3v2::Header &_tagHeader) : ID3v2_21::ExtendedHeaderBase(_tagHeader)
{
}

uint32_t ID3v2_4::ExtendedHeader::Size() const
{
	return ID3v2::Util::Int28To32(headerData.size);
}

int ID3v2_4::ExtendedHeader::Parse(const void *_data, size_t len, size_t *bytes_read)
{
	if (len < SIZE)
		return 1;

	memcpy(&headerData, _data, SIZE);
	*bytes_read = SIZE;


	_data = (const uint8_t *)_data+SIZE;

	/* read any data after the header */
	data_size = Size();
	if (data_size)
	{
		/* sanity check size */
		if (data_size > len)
			return 1;

		/* allocate and read data */
		data = malloc(data_size);
		if (!data)
			return NErr_OutOfMemory;
		memcpy(data, _data, data_size);
		*bytes_read += data_size;
	}

	return 0;
}
