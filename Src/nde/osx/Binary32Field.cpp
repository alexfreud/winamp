/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Binary32Field Class

--------------------------------------------------------------------------- */

#include "nde.h"
#include "Binary32Field.h"
#include "ndestring.h"
//---------------------------------------------------------------------------
Binary32Field::Binary32Field(const uint8_t *_Data, size_t len) : BinaryField(_Data, len)
{
	InitField();
}

//---------------------------------------------------------------------------
void Binary32Field::InitField(void)
{
	Type = FIELD_BINARY32;
}

//---------------------------------------------------------------------------
Binary32Field::Binary32Field()
{
	InitField();
}

//---------------------------------------------------------------------------
void Binary32Field::ReadTypedData(const uint8_t *data, size_t len)
{
	uint32_t c;
	size_t pos = 0;

	CHECK_INT(len); //len-=4;
	c = GET_INT(); pos += 4;
	if (c && c<=len)
	{
		size_t size = c;
		uint8_t *buf = (uint8_t *)malloc(size);
		GET_BINARY(buf, data, c, pos);
		Data = CFDataCreateWithBytesNoCopy(NULL, buf, size, kCFAllocatorMalloc);
	}
}

//---------------------------------------------------------------------------
void Binary32Field::WriteTypedData(uint8_t *data, size_t len)
{
	uint32_t c;
	size_t pos = 0;

	CHECK_INT(len); //len-=4;

	size_t Size = CFDataGetLength(Data);
	if (Data && Size<=len)
	{
		c = CFDataGetLength(Data);
		PUT_INT(c); pos += 4;
		CFDataGetBytes(Data, CFRangeMake(0, Size), data+pos);
	}
	else
	{
		PUT_INT(0);
	}
}

//---------------------------------------------------------------------------
size_t Binary32Field::GetDataSize(void)
{
	if (!Data) 
		return 4;
	return CFDataGetLength(Data) + 4;
}
