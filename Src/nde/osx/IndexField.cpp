/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

IndexField Class

--------------------------------------------------------------------------- */

#include "nde.h"

//---------------------------------------------------------------------------
IndexField::IndexField(unsigned char id, int Pos, int type, CFStringRef FieldName)
{
	InitField();
	Type = FIELD_INDEX;
	Name = (CFStringRef)CFRetain(FieldName);
	ID = id;
	Position = Pos;
	DataType = type;
}

//---------------------------------------------------------------------------
void IndexField::InitField(void)
{
	index = 0;
	Type = FIELD_INDEX;
	Name = NULL;
	ID = 0;
	Position = -1;
	DataType = FIELD_UNKNOWN;
}

//---------------------------------------------------------------------------
IndexField::IndexField()
{
	InitField();
}

//---------------------------------------------------------------------------
IndexField::~IndexField()
{
	if (Name)
		CFRelease(Name);
	delete index;
}

//---------------------------------------------------------------------------
void IndexField::ReadTypedData(const uint8_t *data, size_t len)
{
	unsigned char c;
	int pos=0;
	CHECK_INT(len);
	Position = GET_INT(); pos += 4;
	CHECK_INT(len);
	DataType = GET_INT(); pos += 4;
	CHECK_CHAR(len);
	c = GET_CHAR(); pos++;
	if (c)
	{
		CHECK_BIN(len, c);
		if (Name)
			CFRelease(Name);
		Name = CFStringCreateWithBytes(kCFAllocatorDefault, data+pos, c, kCFStringEncodingUTF8, false); 
	}
}

//---------------------------------------------------------------------------
void IndexField::WriteTypedData(uint8_t *data, size_t len)
{
	int pos=0;
	CHECK_INT(len);
	PUT_INT(Position); pos += 4;

	CHECK_INT(len);
	PUT_INT(DataType); pos += 4;
	CHECK_CHAR(len);
	
	if (Name)
	{
		CFIndex lengthRequired=0;
		CFStringGetBytes(Name, CFRangeMake(0, CFStringGetLength(Name)), kCFStringEncodingUTF8, 0, false, NULL, 0, &lengthRequired);
		CHECK_BIN(len, lengthRequired+1);
		PUT_CHAR(lengthRequired); pos++;
		
		CFStringGetBytes(Name, CFRangeMake(0, CFStringGetLength(Name)), kCFStringEncodingUTF8, 0, false, data+pos, lengthRequired, 0);
	}
}

//---------------------------------------------------------------------------
CFStringRef IndexField::GetIndexName(void)
{
	return Name;
}

//---------------------------------------------------------------------------
size_t IndexField::GetDataSize(void)
{
	if (Name)
	{
		CFIndex lengthRequired=0;
		CFStringGetBytes(Name, CFRangeMake(0, CFStringGetLength(Name)), kCFStringEncodingUTF8, 0, false, NULL, 0, &lengthRequired);
		return lengthRequired+9;
	}
	else
		return 9;
}

//---------------------------------------------------------------------------
int IndexField::Compare(Field * /*Entry*/)
{
	return 0;
}

//---------------------------------------------------------------------------
int IndexField::TranslateToIndex(int Id, IndexField *toindex)
{
	if (index && toindex && toindex->index)
		return index->TranslateIndex(Id, toindex->index);
	return -1;
}


