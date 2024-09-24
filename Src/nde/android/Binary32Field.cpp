/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Binary32Field Class
Field data layout:
[4 bytes] length
[length bytes] binary data
--------------------------------------------------------------------------- */

#include "../nde.h"
#include "Binary32Field.h"
#include "../ndestring.h"
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
		Size = c;
		ndestring_release((ndestring_t)Data);
		Data = (uint8_t *)ndestring_malloc(c);
		GET_BINARY(Data, data, c, pos);
	}
}

//---------------------------------------------------------------------------
void Binary32Field::WriteTypedData(uint8_t *data, size_t len)
{
	uint32_t c;
	size_t pos = 0;

	CHECK_INT(len); //len-=4;

	if (Data && Size<=len)
	{
		c = (uint32_t)Size;
		PUT_INT(c); pos += 4;
		if (Data)
			PUT_BINARY(data, (unsigned char*)Data, c, pos);
	}
	else
	{
		PUT_INT(0);
	}
}

//---------------------------------------------------------------------------
size_t Binary32Field::GetDataSize(void)
{
	if (!Data) return 4;
	return Size + 4;
}
