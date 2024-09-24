/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 BinaryField Class

--------------------------------------------------------------------------- */

#include "../nde.h"
#include "BinaryField.h"

//---------------------------------------------------------------------------
BinaryField::BinaryField(const uint8_t *_Data, int len)
{
	InitField();
	Type = FIELD_BINARY;
	if (_Data && len > 0)
	{
		Data = CFDataCreate(NULL, _Data, len);
	}
}

//---------------------------------------------------------------------------
void BinaryField::InitField(void)
{
	Type = FIELD_BINARY;
	Data = NULL;
}

//---------------------------------------------------------------------------
BinaryField::BinaryField()
{
	InitField();
}

//---------------------------------------------------------------------------
BinaryField::~BinaryField()
{
	CFRelease(Data);
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
		Data = CFDataCreate(NULL, data+pos, c);
	}
}

//---------------------------------------------------------------------------
void BinaryField::WriteTypedData(uint8_t *data, size_t len)
{
	unsigned short c;
	size_t pos = 0;

	CHECK_SHORT(len); 

	size_t Size = CFDataGetLength(Data);
	if (Data && Size<=len)
	{
		c = CFDataGetLength(Data);
		PUT_SHORT(c); pos += 2;
		CFDataGetBytes(Data, CFRangeMake(0, Size), data+pos);
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
		*len = CFDataGetLength(Data);
	return CFDataGetBytePtr(Data);
}

//---------------------------------------------------------------------------
void BinaryField::SetData(const uint8_t *_Data, size_t len)
{
	if (!_Data || !len) return;
	if (Data)
		CFRelease(Data);
	Data = CFDataCreate(NULL, _Data, len);
}

CFDataRef BinaryField::GetCFData()
{
	return Data;
}

//---------------------------------------------------------------------------
size_t BinaryField::GetDataSize(void)
{
	if (!Data) return 2;
	return CFDataGetLength(Data) + 2;
}

//---------------------------------------------------------------------------
int BinaryField::Compare(Field *Entry)
{
	if (!Entry) return -1;
	size_t compare_length;
	const uint8_t *compare_data = ((BinaryField*)Entry)->GetData(&compare_length);

	size_t size;
	const uint8_t *data = GetData(&size);
	return memcmp(data, compare_data, min(compare_length, size));
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
				r = !!memmem(d, p, s, l);
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

