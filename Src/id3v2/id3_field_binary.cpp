// The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
// patent or other intellectual property protection in this work. This means that
// it may be modified, redistributed and used in commercial and non-commercial
// software and hardware without restrictions. ID3Lib is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
// 
// The ID3Lib authors encourage improvements and optimisations to be sent to the
// ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org). Approved
// submissions may be altered, and will be included and released under these terms.
// 
// Mon Nov 23 18:34:01 1998
// improved/optimized/whatever 10/30/00 JF

#include <windows.h>
#include <stdio.h>
#include <memory.h>
#include "id3_field.h"

void ID3_Field::Set(uchar *newData, luint newSize)
{
	Clear();

	if (newSize)
	{
		if (newSize == 4294967295/*-1*/)
		{
			ID3_THROW(ID3E_NoMemory);
			return;
		}

		if (!(data = (unsigned char *)calloc(newSize, sizeof(unsigned char))))
		{
			ID3_THROW(ID3E_NoMemory);
		}
		else
		{
			memcpy (data, newData, newSize);
			size = newSize;

			type = ID3FTY_BINARY;
			hasChanged = true;
		}
	}

	return;
}

void ID3_Field::Get(uchar *buffer, luint buffLength)
{
	if (data && size && buffLength && buffer)
	{
		luint actualBytes = MIN(buffLength, size);
		memcpy(buffer, data, actualBytes);
	}
}

luint ID3_Field::ParseBinary (uchar *buffer, luint posn, luint buffSize)
{
	luint bytesUsed = 0;

	bytesUsed = buffSize - posn;

	if (fixedLength != -1)
		bytesUsed = MIN (fixedLength, bytesUsed);

	Set(&buffer[ posn ], bytesUsed);

	hasChanged = false;

	return bytesUsed;
}

luint ID3_Field::RenderBinary (uchar *buffer)
{
	luint bytesUsed = 0;

	bytesUsed = BinSize();
	memcpy(buffer, data, bytesUsed);

	hasChanged = false;

	return bytesUsed;
}