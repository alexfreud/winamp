//  The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
//  patent or other intellectual property protection in this work.  This means that
//  it may be modified, redistributed and used in commercial and non-commercial
//  software and hardware without restrictions.  ID3Lib is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//
//  The ID3Lib authors encourage improvements and optimisations to be sent to the
//  ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org).  Approved
//  submissions may be altered, and will be included and released under these terms.
//
//  Mon Nov 23 18:34:01 1998


#include "id3_tag.h"


//	To be used when reading an ID3v2-tag
//	Transforms all FF 00 sequences into FF

luint	ID3_Tag::ReSync(uchar *binarySourceData, luint sourceSize)
{
	uchar	*source, *dest;
	uchar	temp;
	luint	destinationSize;

	source = binarySourceData;
	destinationSize = sourceSize;

	while (source < binarySourceData + sourceSize)
	{
		temp = *source++;

		if (temp == 0xFF && source < binarySourceData + sourceSize)
		{
			if ((temp = *source++) == 0x00)
				destinationSize--;
		}
	}

	dest = source = binarySourceData;

	while ((source < binarySourceData + sourceSize) && (dest < binarySourceData + sourceSize))
	{
		*dest++ = temp = *source++;

		if (temp == 0xFF && source < binarySourceData + sourceSize)
		{
			if ((temp = *source++) != 0x00 && dest < binarySourceData + sourceSize)
				* dest++ = temp;
		}
	}

	for (luint cl=destinationSize;cl<sourceSize;cl++)
		binarySourceData[cl]=0;

	return destinationSize;
}


// How big will the tag be after we unsync?

luint	ID3_Tag::GetUnSyncSize(uchar *buffer, luint size)
{
	luint	extraSize	= 0;
	uchar	*source	= buffer;

	//	Determine the size needed for the destination data
	while (source < buffer + size)
	{
		uchar temp = *source++;

		if (temp == 0xFF)
		{
			// last byte?
			if (source == (buffer + size))
				extraSize++;
			else
			{
				temp = *source;

				if (((temp & 0xE0) == 0xE0) || (temp == 0))
					extraSize++;
			}
		}
	}

	return extraSize + size;
}


//	To be used when writing an ID3v2-tag
//	Transforms:
//	11111111 111xxxxx -> 11111111 00000000 111xxxxx
//	11111111 00000000 -> 11111111 00000000 00000000
//	11111111 <EOF> -> 11111111 00000000 <EOF>

void	ID3_Tag::UnSync(uchar *destData, luint destSize, uchar *sourceData, luint sourceSize)
{
	uchar	*source	= sourceData;
	uchar	*dest	= destData;

	// Now do the real transformation
	while (source < sourceData + sourceSize)
	{
		uchar	temp = *dest++ = *source++;

		if (temp == 0xFF)
		{
			// last byte?
			if (source == (sourceData + sourceSize))
				*dest++ = 0;
			else
			{
				temp = *source;

				if (((temp & 0xE0) == 0xE0) || (temp == 0))
					*dest++ = 0;
			}
		}
	}

	return ;
}