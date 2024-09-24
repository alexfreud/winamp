/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 BinaryField Class
Field data layout:
[2 bytes] length
[length bytes] binary data
--------------------------------------------------------------------------- */

#include "../nde.h"
#include "BinaryField.h"
#include "../NDEString.h"
//---------------------------------------------------------------------------
BinaryField::BinaryField(const uint8_t *_Data, int len)
{
	InitField();
	Type = FIELD_BINARY;
	if (_Data && len > 0)
	{
		Data = (uint8_t *)ndestring_malloc(len);
		memcpy(Data, _Data, len);
		Size = len;
	}
}

//---------------------------------------------------------------------------
void BinaryField::InitField(void)
{
	Type = FIELD_BINARY;
	Data = NULL;
	Size = 0;
}

//---------------------------------------------------------------------------
BinaryField::BinaryField()
{
	InitField();
}

//---------------------------------------------------------------------------
BinaryField::~BinaryField()
{
	ndestring_release((ndestring_t)Data);
}

//---------------------------------------------------------------------------
void BinaryField::ReadTypedData(const uint8_t *data, size_t len)
{
	unsigned short c;
	int pos = 0;

	CHECK_SHORT(len);

	c = GET_SHORT(); pos += 2; 
	if (c && c<=len)
	{
		Size = c;
		ndestring_release((ndestring_t)Data);
		Data = (uint8_t *)ndestring_malloc(c);
		GET_BINARY(Data, data, c, pos);
	}
}

//---------------------------------------------------------------------------
void BinaryField::WriteTypedData(uint8_t *data, size_t len)
{
	size_t pos = 0;

	CHECK_SHORT(len); 

	if (Data && Size<=len)
	{
		unsigned short c = (unsigned short)Size;
		PUT_SHORT(c); pos += 2;
		if (Data)
			PUT_BINARY(data, (unsigned char*)Data, c, pos);
	}
	else
	{
		PUT_SHORT(0);
	}
}

//---------------------------------------------------------------------------
const uint8_t *BinaryField::GetData(size_t *len)
{
	if (len)
		*len = Size;
	return Data;
}

//---------------------------------------------------------------------------
void BinaryField::SetData(const uint8_t *_Data, size_t len)
{
	if (!_Data || !len) return;
	ndestring_release((ndestring_t)Data);
	Size = 0;
	Data = (uint8_t *)ndestring_malloc(len);
	memcpy(Data, _Data, len);
	Size = len;
}

//---------------------------------------------------------------------------
size_t BinaryField::GetDataSize(void)
{
	if (!Data) return 2;
	return Size + 2;
}

//---------------------------------------------------------------------------
int BinaryField::Compare(Field *Entry)
{
	if (!Entry) return -1;
	size_t compare_length;
	const uint8_t *compare_data = ((BinaryField*)Entry)->GetData(&compare_length);
	return memcmp(Data, compare_data, min(compare_length, Size));
}

//---------------------------------------------------------------------------
bool BinaryField::ApplyFilter(Field *FilterData, int op)
{
	size_t l, s;
	const uint8_t *p = ((BinaryField *)FilterData)->GetData(&l);
	const uint8_t *d = GetData(&s);
	if (!p)
		p = (const uint8_t *)"";
	if (!d)
		d = (const uint8_t *)"";
	bool r;
	switch (op)
	{
		case FILTER_EQUALS:
			if (l != s)
				r = false;
			else
				r = !memcmp(d, p, min(s, l));
			break;
		case FILTER_CONTAINS:
			if (l > s)
				r = FALSE;
			else
				r = !!memmem(d, s, p, l);
			break;
		case FILTER_ABOVE:
			r = (memcmp(d, p, min(s, l)) > 0);
			break;
		case FILTER_BELOW:
			r = (memcmp(d, p, min(s, l)) < 0);
			break;
		case FILTER_BELOWOREQUAL:
			r = (memcmp(d, p, min(s, l)) <= 0);
			break;
		case FILTER_ABOVEOREQUAL:
			r = (memcmp(d, p, min(s, l)) >= 0);
			break;
		case FILTER_ISEMPTY:
			r = (s == 0);
			break;
		case FILTER_ISNOTEMPTY:
			r = (s != 0);
			break;
		default:
			r = true;
			break;
	}
	return r;
}

