/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

IndexField Class
Windows implementation

Field data layout
[4 bytes] Position
[4 bytes] Data Type
[1 byte] Name Length
[Name Length bytes] Name (UTF-8)
--------------------------------------------------------------------------- */

#include "../nde.h"
#include "../ndestring.h"
//---------------------------------------------------------------------------
IndexField::IndexField(unsigned char id, int Pos, int type, const wchar_t *FieldName)
{
	InitField();
	Type = FIELD_INDEX;
	Name = ndestring_wcsdup(FieldName);
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
	ndestring_release(Name);
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

		int string_length = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)(data+pos), c, 0, 0);
		Name = ndestring_malloc((string_length+1)*sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)(data+pos), c, Name, string_length);
		Name[string_length]=0;
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
	
	CHECK_CHAR(len);

	if (Name) 
	{
		size_t string_length = WideCharToMultiByte(CP_UTF8, 0, Name, -1, 0, 0, 0, 0);
		if (string_length)
		{
			PUT_CHAR((uint8_t)string_length-1);
			pos++;

			CHECK_BIN(len, string_length-1);		
			WideCharToMultiByte(CP_UTF8, 0, Name, -1, (LPSTR)(data+pos), (int)string_length-1, 0, 0);
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
wchar_t *IndexField::GetIndexName(void)
{
	return Name;
}

//---------------------------------------------------------------------------
size_t IndexField::GetDataSize(void)
{
	size_t s=9;
	if (Name)
	{
		int string_length=WideCharToMultiByte(CP_UTF8, 0, Name, -1, 0, 0, 0, 0);
		if (string_length)
			s+=string_length-1;
	}
	s++;
	return s;
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