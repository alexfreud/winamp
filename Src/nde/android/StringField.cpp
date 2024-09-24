/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

StringField Class
Android (linux) specific version

Field data layout:
[2 bytes] string length (bytes)
[length bytes] String data.  UTF-16 data will start with a BOM 

--------------------------------------------------------------------------- */

#include "../nde.h"
#include "StringField.h"

//---------------------------------------------------------------------------
StringField::StringField(const char *Str, int strkind)
{
	InitField();
	Type = FIELD_STRING;
	if (Str)
	{
		if (strkind == STRING_IS_WCHAR)
			String = ndestring_wcsdup(Str);
		else
		{
			String = const_cast<char *>(Str);
			ndestring_retain(String);
		}
	}
}

//---------------------------------------------------------------------------
void StringField::InitField(void)
{
	Type = FIELD_STRING;
	//	String = NULL;
	String = NULL;
}

//---------------------------------------------------------------------------
StringField::StringField()
{
	InitField();
}

//---------------------------------------------------------------------------
StringField::~StringField()
{
	ndestring_release(String);
	String=0;
}


//---------------------------------------------------------------------------
void StringField::ReadTypedData(const uint8_t *data, size_t len)
{
	size_t pos=0;
	unsigned short c;

	CHECK_SHORT(len);
	c = GET_SHORT();
	pos+=2;
	if (c)
	{
		bool unicode=false;
		bool utf16BE=false;
		if (c >= 2 // enough room for BOM
			&& (c & 1) == 0) // can't be unicode if it's not an even multiple of 2
		{
			uint16_t BOM=GET_SHORT();
			if (BOM == 0xFEFF)
			{
				pos+=2;
				c-=2;	
				unicode=true;
			}
			else if (BOM == 0xFFFE)
			{
				pos+=2;
				c-=2;	
				unicode=true;
				utf16BE=true;
			}
		}

		CHECK_BIN(len, c);
		if (unicode)
		{
			ndestring_release(String);
			if (utf16BE)
			{
				size_t bytes = utf16BE_to_utf8((uint16_t *)(data+pos), c>>1, 0, 0);
				String = ndestring_malloc(bytes+1);
				utf16BE_to_utf8((uint16_t *)(data+pos), c>>1, String, bytes);
				String[bytes]=0;		
			}
			else
			{
				size_t bytes = utf16LE_to_utf8((uint16_t *)(data+pos), c>>1, 0, 0);
				String = ndestring_malloc(bytes+1);
				utf16LE_to_utf8((uint16_t *)(data+pos), c>>1, String, bytes);
				String[bytes]=0;				
			}
		}
		else
		{
			// TODO: check for utf-8 byte marker
			String = ndestring_malloc(c+1);
			GET_BINARY((uint8_t *)String, data, c, pos);
			String[c]=0;
		}
	}
}

//---------------------------------------------------------------------------
void StringField::WriteTypedData(uint8_t *data, size_t len)
{
	int pos=0;

	if (String)
	{
		unsigned short c = (unsigned short)strlen(String);
		// write size
		CHECK_SHORT(len);
		PUT_SHORT(c); pos+=2;

		// write string
		CHECK_BIN(len, c);
		PUT_BINARY(data, (uint8_t *)String, c, pos);
	}
	else
	{
		CHECK_SHORT(len);
		PUT_SHORT(0); pos+=2;
	}
}


//---------------------------------------------------------------------------
char *StringField::GetString(void)
{
	return String;
}

//---------------------------------------------------------------------------
void StringField::SetString(const char *Str)
{
	if (!Str) return;

	ndestring_release(String);
	String = NULL;
	String = ndestring_wcsdup(Str);
}

//---------------------------------------------------------------------------
void StringField::SetNDEString(char *Str)
{
	if (!Str) return;

	// copy and then release, just in case we're copying into ourselves
	char *oldStr = String;
	String = Str;
	ndestring_retain(String);
	ndestring_release(oldStr);
}
//---------------------------------------------------------------------------
size_t StringField::GetDataSize(void)
{
	if (String)
	{
		return strlen(String) + 2;
	}
	else
	{
		return 2;
	}
}

//---------------------------------------------------------------------------
int StringField::Compare(Field *Entry)
{
	if (!Entry) return -1;
	if (Entry->GetType() != GetType()) return 0;
	return mystricmp(GetString(), ((StringField*)Entry)->GetString());
}

//---------------------------------------------------------------------------
int StringField::Starts(Field *Entry)
{
	if (!Entry) return -1;
	if (Entry->GetType() != GetType()) return 0;
	return (mystristr(GetString(), ((StringField*)Entry)->GetString()) == GetString());
}

//---------------------------------------------------------------------------
int StringField::Contains(Field *Entry)
{
	if (!Entry) return -1;
	if (Entry->GetType() != GetType()) return 0;
	return (mystristr(GetString(), ((StringField*)Entry)->GetString()) != NULL);
}

Field *StringField::Clone(Table *pTable)
{
	StringField *clone = new StringField(String, STRING_IS_NDESTRING);
	clone->Pos = FIELD_CLONE;
	clone->ID = ID;
	clone->MaxSizeOnDisk = GetDataSize();
	return clone;
}

bool StringField::ApplyFilter(Field *Data, int op)
{
	// TODO: maybe do this?

	if (op == FILTER_ISEMPTY || op == FILTER_ISNOTEMPTY)
	{
		bool r = (op == FILTER_ISEMPTY);
		if (!String)
			return r;

		if (String && String[0] == 0)
			return r;

		return !r;
	}
	//
	bool r;
	StringField *compField = (StringField *)Data;

	const char *p = compField->GetString();
	const char *d = GetString();
	if (!p)
		p = "";
	if (!d)
		d = "";

	switch (op)
	{
	case FILTER_EQUALS:
		r = !nde_stricmp(d, p);
		break;
	case FILTER_NOTEQUALS:
		r = !!nde_stricmp(d, p);
		break;
	case FILTER_CONTAINS:
		r = (NULL != stristr_ignore(d, p));
		break;
	case FILTER_NOTCONTAINS:
		r = (NULL == stristr_ignore(d, p));
		break;
	case FILTER_ABOVE:
		r = (bool)(nde_stricmp(d, p) > 0);
		break;
	case FILTER_ABOVEOREQUAL:
		r = (bool)(nde_stricmp(d, p) >= 0);
		break;
	case FILTER_BELOW:
		r = (bool)(nde_stricmp(d, p) < 0);
		break;
	case FILTER_BELOWOREQUAL:
		r = (bool)(nde_stricmp(d, p) <= 0);
		break;
	case FILTER_BEGINS:
		r = (bool)(nde_strnicmp(d, p, strlen(p)) == 0);
		break;
	case FILTER_ENDS:
		{
			size_t lenp = strlen(p), lend = strlen(d);
			if (lend < lenp) return 0;  // too short
			r = (bool)(nde_stricmp((d + lend) - lenp, p) == 0);
		}
		break;
	case FILTER_LIKE:
		r = (bool)(nde_stricmp(d, p) == 0);
		break;
	case FILTER_BEGINSLIKE:
		r = (bool)(nde_strnicmp_ignore(d, p, strlen(p)) == 0);
		break;
	default:
		r = true;
		break;
	}
	return r;
}

