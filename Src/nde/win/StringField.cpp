/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

StringField Class
Windows specific version

Field data layout:
[2 bytes] string length (bytes)
[length bytes] String data.  UTF-16 data will start with a BOM 
--------------------------------------------------------------------------- */

#include "../nde.h"
#include "StringField.h"
#include "../../nu/AutoChar.h"
#include "../../nu/AutoWide.h"

static wchar_t CharSwap(wchar_t value)
{
	return (value >> 8) | (value << 8);
}

//---------------------------------------------------------------------------
StringField::StringField(const wchar_t *Str, int strkind)
{
	InitField();
	Type = FIELD_STRING;
	if (Str)
	{
		if (strkind == STRING_IS_WCHAR)
			StringW = ndestring_wcsdup(Str);
		else
		{
			StringW = const_cast<wchar_t *>(Str);
			ndestring_retain(StringW);
		}
	}
}

//---------------------------------------------------------------------------
void StringField::InitField(void)
{
	Type = FIELD_STRING;
	StringW = NULL;
	optimized_the = 0;
}

//---------------------------------------------------------------------------
StringField::StringField()
{
	InitField();
}

//---------------------------------------------------------------------------
StringField::~StringField()
{
	ndestring_release(StringW);
	StringW=0;
}

//---------------------------------------------------------------------------
void StringField::ReadTypedData(const uint8_t *data, size_t len)
{
	unsigned short c;

	CHECK_SHORT(len);
	c = (unsigned short)(data[0]|(data[1]<<8));
	data+=2;
	if (c)
	{
		bool unicode=false;
		bool reverseEndian=false;
		if (c >= 2 // enough room for BOM
			&& (c % 2) == 0) // can't be unicode if it's not an even multiple of 2
		{
			wchar_t BOM=0;
			memcpy(&BOM, data, 2);
			if (BOM == 0xFEFF)
			{
				data+=2;
				c-=2;	
				unicode=true;
			}
			else if (BOM == 0xFFFE)
			{
				data+=2;
				c-=2;	
				unicode=true;
				reverseEndian=true;
			}
		}

		CHECK_BIN(len, c);
		if (unicode)
		{
			ndestring_release(StringW);
			StringW = ndestring_malloc(c+sizeof(wchar_t));

			memcpy(StringW, data, c);
			StringW[c/2]=0;
			if (reverseEndian)
			{
				for (unsigned short i=0;i<c;i++)
					StringW[i]=CharSwap(StringW[i]);
			}
		}
		else
		{
			int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)data, c, 0, 0);
			StringW = ndestring_malloc((len+1)*sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)data, c, StringW, len);
			StringW[len]=0;
		}
	}
}

//---------------------------------------------------------------------------
void StringField::WriteTypedData(uint8_t *data, size_t len)
{
	int pos=0;

	if (StringW)
	{
		unsigned short c = (unsigned short)wcslen(StringW) * sizeof(wchar_t) + 2 /* for BOM */;
		// write size
		CHECK_SHORT(len);
		PUT_SHORT(c); pos+=2;

		// write byte order mark
		CHECK_BIN(len, 2);
		wchar_t BOM = 0xFEFF;
		PUT_BINARY(data, (uint8_t *)&BOM, 2, pos);
		pos+=2;
		c-=2;

		// write string
		CHECK_BIN(len, c);
		PUT_BINARY(data, (uint8_t *)StringW, c, pos);
	}
	else
	{
		CHECK_SHORT(len);
		PUT_SHORT(0); /*pos+=2;*/
	}
}

//---------------------------------------------------------------------------
wchar_t *StringField::GetStringW(void)
{
	return StringW;
}

//---------------------------------------------------------------------------
void StringField::SetStringW(const wchar_t *Str)
{
	if (!Str) return;

	ndestring_release(StringW);
	StringW = NULL;
	StringW = ndestring_wcsdup(Str);
	optimized_the=0;
}

//---------------------------------------------------------------------------
void StringField::SetNDEString(wchar_t *Str)
{
	if (!Str) return;

	// copy and then release, just in case we're copying into ourselves
	wchar_t *oldStr = StringW;
	StringW = Str;
	ndestring_retain(StringW);
	ndestring_release(oldStr);
	optimized_the=0;
}

//---------------------------------------------------------------------------
size_t StringField::GetDataSize(void)
{
	if (StringW)
	{
		return wcslen(StringW)*2 +2 /*for BOM*/ + 2 /*for byte length*/;
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
	return mywcsicmp(GetStringW(), ((StringField*)Entry)->GetStringW());
}

//---------------------------------------------------------------------------
int StringField::Starts(Field *Entry)
{
	if (!Entry) return -1;
	if (Entry->GetType() != GetType()) return 0;
	const wchar_t *p = ((StringField*)Entry)->GetStringW();
	const wchar_t *d = GetStringW();
	if (!d || !p) return 0;
	return nde_wcsbegins(d, p);
}

//---------------------------------------------------------------------------
int StringField::Contains(Field *Entry)
{
	if (!Entry) return -1;
	if (Entry->GetType() != GetType()) return 0;
	const wchar_t *p = ((StringField*)Entry)->GetStringW();
	const wchar_t *d = GetStringW();
	if (!d || !p) return 0;
	return nde_wcscontains(GetStringW(), ((StringField*)Entry)->GetStringW());
}

Field *StringField::Clone(Table *pTable)
{
	StringField *clone = new StringField(StringW, STRING_IS_NDESTRING);
	clone->Pos = FIELD_CLONE;
	clone->ID = ID;
	clone->MaxSizeOnDisk = (uint32_t)GetDataSize();
	return clone;
}

// todo: make configurable words to skip, as well as trailing whitespace removal
#define IsCharSpaceW(c) (c == L' ' || c == L'\t')
inline bool IsTheW(const wchar_t *str) { if (str && (str[0] == L't' || str[0] == L'T') && (str[1] == L'h' || str[1] == L'H') && (str[2] == L'e' || str[2] == L'E') && (str[3] == L' ')) return true; else return false; }
#define SKIP_THE_AND_WHITESPACEW(x) { wchar_t *save##x=(wchar_t*)x; while (IsCharSpaceW(*x) && *x) x++; if (IsTheW(x)) x+=4; while (IsCharSpaceW(*x)) x++; if (!*x) x=save##x; }

bool StringField::ApplyFilter(Field *Data, int op)
{
	// TODO: maybe do this?

	if (op == FILTER_ISEMPTY || op == FILTER_ISNOTEMPTY)
	{
		bool r = (op == FILTER_ISEMPTY);
		if (!StringW)
			return r;

		if (StringW && StringW[0] == 0)
			return r;

		return !r;
	}
	//
	bool r;
	StringField *compField = (StringField *)Data;

	const wchar_t *p = compField->GetStringW();
	const wchar_t *d = GetStringW();
	if (!p)
		p = L"";
	if (!d)
		d = L"";

	switch (op)
	{
		case FILTER_EQUALS:
			r = !nde_wcsicmp(d, p);
			break;
		case FILTER_NOTEQUALS:
			r = !!nde_wcsicmp(d, p);
			break;
		case FILTER_CONTAINS:
			r = nde_wcscontains(d, p);
			break;
		case FILTER_NOTCONTAINS:
			r = !nde_wcscontains(d, p);
			break;
		case FILTER_ABOVE:
			r = (bool)(nde_wcsicmp(d, p) > 0);
			break;
		case FILTER_ABOVEOREQUAL:
			r = (bool)(nde_wcsicmp(d, p) >= 0);
			break;
		case FILTER_BELOW:
			r = (bool)(nde_wcsicmp(d, p) < 0);
			break;
		case FILTER_BELOWOREQUAL:
			r = (bool)(nde_wcsicmp(d, p) <= 0);
			break;
		case FILTER_BEGINS:
			r = nde_wcsbegins(d, p);
			break;
		case FILTER_ENDS:
				r = nde_wcsends(d, p);
			break;
		case FILTER_LIKE:

			if (compField->optimized_the)
				p = compField->optimized_the;
			else
			{
				SKIP_THE_AND_WHITESPACEW(p);
				compField->optimized_the = p;				
			}

			if (optimized_the)
				d = optimized_the;
			else
			{
				SKIP_THE_AND_WHITESPACEW(d);
				optimized_the=d;
			}

			r = (bool)(nde_wcsicmp(d, p) == 0);
			break;
		case FILTER_BEGINSLIKE:

			if (compField->optimized_the)
				p = compField->optimized_the;
			else
			{
				SKIP_THE_AND_WHITESPACEW(p);
				compField->optimized_the = p;				
			}

			if (optimized_the)
				d = optimized_the;
			else
			{
				SKIP_THE_AND_WHITESPACEW(d);
				optimized_the=d;
			}

			r = nde_wcsbegins(d, p);
			break;
		default:
			r = true;
			break;
	}
	return r;
}
