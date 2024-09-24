/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

ColumnField Class
Android (linux) implementation

Field data layout:
[1 byte] Field Type
[1 byte] Unused (maybe convert to 'searchable')
[1 byte] Name Length
[Name Length bytes] Name (UTF-8)
--------------------------------------------------------------------------- */

#include "ColumnField.h"
#include "../NDEString.h"
#include "../nde.h"

//---------------------------------------------------------------------------
ColumnField::ColumnField(unsigned char FieldID, const char *FieldName, unsigned char FieldType, Table *parentTable)
{
	InitField();
	Type = FIELD_COLUMN;
	MyType = FieldType;
	Name = ndestring_wcsdup(FieldName);
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
	if (Name) free(Name);
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

	// [1 byte] Field Type
	CHECK_CHAR(len);
	MyType = GET_CHAR(); 
	pos++;

	// [1 byte] unused 
	CHECK_CHAR(len);
//cut:	indexUnique = (BOOL)GET_CHAR();
	pos++;

	// [1 byte] string length
	CHECK_CHAR(len);
	c = GET_CHAR(); 
	pos++;

	if (c)
	{
		CHECK_BIN(len, c);
		Name = ndestring_wcsndup((const char *)(data+pos), c);
	}
}

//---------------------------------------------------------------------------
void ColumnField::WriteTypedData(uint8_t *data, size_t len)
{
	int pos = 0;

	// [1 byte] Field Type
	CHECK_CHAR(len);
	PUT_CHAR(MyType); 
	pos++;

	// [1 byte] unused 
	CHECK_CHAR(len);
	PUT_CHAR(0/*(char)indexUnique*/); 
	pos++;

	CHECK_CHAR(len);
	if (Name) 
	{
		int string_length = strlen(Name);
		if (string_length)
		{
			PUT_CHAR(string_length);
			pos++;

			CHECK_BIN(len, string_length);
			PUT_BINARY(data, (const uint8_t *)Name, string_length, pos);
		}
		else
		{
			PUT_CHAR(0);
		}
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
char *ColumnField::GetFieldName(void)
{
	return Name;
}

//---------------------------------------------------------------------------
size_t ColumnField::GetDataSize(void)
{
	size_t s=3;
	if (Name)
	{
		s+=strlen(Name);
	}
	return s;
}

//---------------------------------------------------------------------------
int ColumnField::Compare(Field * /*Entry*/)
{
	return 0;
}


