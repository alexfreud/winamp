/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

ColumnField Class
Mac OS X implementation

Field data layout:
[1 byte] Field Type
[1 byte] Unused (maybe convert to 'searchable')
[1 byte] Name Length
[Name Length bytes] Name (UTF-8)
--------------------------------------------------------------------------- */

#include "../nde.h"

//---------------------------------------------------------------------------
ColumnField::ColumnField(unsigned char FieldID, CFStringRef FieldName, unsigned char FieldType, Table *parentTable)
{
	InitField();
	Type = FIELD_COLUMN;
	MyType = FieldType;
	Name = (CFStringRef)CFRetain(FieldName);
	ID = FieldID;
}

//---------------------------------------------------------------------------
void ColumnField::InitField(void)
{
	searchable = false;
	MyType = FIELD_UNKNOWN;
	Type = FIELD_COLUMN;
	Name = NULL;
	ID = 0;
}

//---------------------------------------------------------------------------
ColumnField::ColumnField()
{
	InitField();
}

//---------------------------------------------------------------------------
ColumnField::~ColumnField()
{
	if (Name) 
		CFRelease(Name);
}

void ColumnField::SetSearchable(bool val)
{
	searchable=val;
}

bool ColumnField::IsSearchableField() const
{
	return searchable;
}

//---------------------------------------------------------------------------
void ColumnField::ReadTypedData(const uint8_t *data, size_t len)
{
	unsigned char c;
	int pos=0;

	CHECK_CHAR(len);
	MyType = GET_CHAR(); pos++;

	CHECK_CHAR(len);
	pos++;

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
void ColumnField::WriteTypedData(uint8_t *data, size_t len)
{
	int pos = 0;

	CHECK_CHAR(len);
	PUT_CHAR(MyType); pos++;

	CHECK_CHAR(len);
	PUT_CHAR(0/*(char)indexUnique*/); 
	pos++;

	if (Name)
	{
		CFIndex lengthRequired=0;
		CFStringGetBytes(Name, CFRangeMake(0, CFStringGetLength(Name)), kCFStringEncodingUTF8, 0, false, NULL, 0, &lengthRequired);
		CHECK_BIN(len, lengthRequired+1);
		PUT_CHAR(lengthRequired); pos++;

		CFStringGetBytes(Name, CFRangeMake(0, CFStringGetLength(Name)), kCFStringEncodingUTF8, 0, false, data+pos, lengthRequired, 0);
	}
	else
	{
		PUT_CHAR(0);
	}
}

//---------------------------------------------------------------------------
unsigned char ColumnField::GetDataType(void)
{
	return MyType;
}

//---------------------------------------------------------------------------
void ColumnField::SetDataType(unsigned char type) 
{
	if ((MyType == FIELD_INTEGER || MyType == FIELD_BOOLEAN || MyType == FIELD_DATETIME || MyType == FIELD_LENGTH) && 
		(type == FIELD_INTEGER || type == FIELD_BOOLEAN || type == FIELD_DATETIME || type == FIELD_LENGTH)) {
			MyType = type;
		}
		// going from string to filename or filename to string is OK
		if ((MyType == FIELD_FILENAME && type == FIELD_STRING)
			|| (MyType == FIELD_STRING && type == FIELD_FILENAME))
		{
			MyType = type;
		}
}

//---------------------------------------------------------------------------
CFStringRef ColumnField::GetFieldName(void)
{
	return Name;
}

//---------------------------------------------------------------------------
size_t ColumnField::GetDataSize(void)
{
	if (Name)
	{
		CFIndex lengthRequired=0;
		CFStringGetBytes(Name, CFRangeMake(0, CFStringGetLength(Name)), kCFStringEncodingUTF8, 0, false, NULL, 0, &lengthRequired);
		return lengthRequired+3;
	}
	else
		return 3;
}

//---------------------------------------------------------------------------
int ColumnField::Compare(Field * /*Entry*/)
{
	return 0;
}


