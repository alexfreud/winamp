/* ---------------------------------------------------------------------------
Nullsoft Database Engine
--------------------
codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

StringField Class

Mac OS X (CFStringRef) implementation
--------------------------------------------------------------------------- */

#include "../nde.h"
#include "StringField.h"

//---------------------------------------------------------------------------
StringField::StringField(CFStringRef Str)
{
	InitField();
	Type = FIELD_STRING;
	if (Str)
	{
		SetNDEString(Str);
	}
}


//---------------------------------------------------------------------------
void StringField::InitField(void)
{
	Type = FIELD_STRING;
	String=0;
}

//---------------------------------------------------------------------------
StringField::StringField()
{
	InitField();
}

//---------------------------------------------------------------------------
StringField::~StringField()
{
	if (String)
		CFRelease(String);
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
		if (c >= 2 // enough room for BOM
			&& (c % 2) == 0) // can't be unicode if it's not an even multiple of 2
		{
			wchar_t BOM=0;
			memcpy(&BOM, data, 2);
			if (BOM == 0xFEFF || BOM == 0xFFFE)
			{
				unicode=true;
			}
		}

		CHECK_BIN(len, c);
		if (unicode)
		{
			if (String)
				CFRelease(String);
			String = CFStringCreateWithBytes(kCFAllocatorDefault, data, c, kCFStringEncodingUTF16, true); 
		}
		else
		{
			if (String)
				CFRelease(String);
			String = CFStringCreateWithBytes(kCFAllocatorDefault, data, c, kCFStringEncodingWindowsLatin1, false); 
		}
	}
}

//---------------------------------------------------------------------------
void StringField::WriteTypedData(uint8_t *data, size_t len)
{
	int pos=0;

	CHECK_SHORT(len);
	if (String)
	{
		CFIndex lengthRequired=0;
		CFStringGetBytes(String, CFRangeMake(0, CFStringGetLength(String)), kCFStringEncodingUTF16, 0, true, NULL, 0, &lengthRequired);
		CHECK_BIN(len, lengthRequired+2);
		PUT_SHORT(lengthRequired); pos+=2;

		CFStringGetBytes(String, CFRangeMake(0, CFStringGetLength(String)), kCFStringEncodingUTF16, 0, true, data+pos, lengthRequired, 0);
	}
	else
	{
		PUT_SHORT(0);
	}
}

CFStringRef StringField::GetString()
{
	return String;
}

void StringField::SetNDEString(CFStringRef Str)
{
	if (!Str)
		return;
	
	CFStringRef old = String;
	String = (CFStringRef)CFRetain(Str);
	CFRelease(old);
}

//---------------------------------------------------------------------------
size_t StringField::GetDataSize(void)
{
	if (String)
	{
		CFIndex lengthRequired=0;
		CFStringGetBytes(String, CFRangeMake(0, CFStringGetLength(String)), kCFStringEncodingUTF16, 0, true, NULL, 0, &lengthRequired);
		return lengthRequired+2;
	}
	else
		return 2;
}

//---------------------------------------------------------------------------
int StringField::Compare(Field *Entry)
{
	if (!Entry) return -1;
	if (Entry->GetType() != GetType()) return 0;

	CFStringRef compareString = ((StringField*)Entry)->GetString();
	if (!String && !compareString) return 0;
	if (!String && compareString) return 1;
	if (!compareString) return -1;

	return CFStringCompare(String, compareString, kCFCompareCaseInsensitive|kCFCompareNonliteral|kCFCompareDiacriticInsensitive);
}

//---------------------------------------------------------------------------
int StringField::Starts(Field *Entry)
{
	if (!Entry) return -1;
	if (Entry->GetType() != GetType()) return 0;

	CFStringRef compareString = ((StringField*)Entry)->GetString();
	if (!String || !compareString) return 0;

	CFRange findRange = CFStringFind(String, compareString, 	kCFCompareCaseInsensitive|kCFCompareNonliteral|kCFCompareAnchored|kCFCompareDiacriticInsensitive);
	if (findRange.location == kCFNotFound)
		return 0;
	return findRange.location == 0;
}

//---------------------------------------------------------------------------
int StringField::Contains(Field *Entry)
{
	if (!Entry) return -1;
	if (Entry->GetType() != GetType()) return 0;

		CFStringRef compareString = ((StringField*)Entry)->GetString();
	if (!String || !compareString) return 0;

	CFRange findRange =  CFStringFind(String, compareString, 	kCFCompareCaseInsensitive|kCFCompareNonliteral|kCFCompareDiacriticInsensitive);
	return findRange.location != kCFNotFound;

}

Field *StringField::Clone(Table *pTable)
{
	StringField *clone = new StringField(String);
	clone->Pos = FIELD_CLONE;
	clone->ID = ID;
	clone->MaxSizeOnDisk = GetDataSize();
	return clone;
}


//---------------------------------------------------------------------------
bool StringField::ApplyFilter(Field *Data, int op)
{
	if (op == FILTER_ISEMPTY || op == FILTER_ISNOTEMPTY)
	{
		bool r = (op == FILTER_ISEMPTY);
		if (!String)
			return r;

		if (CFStringGetLength(String) == 0)
			return r;

		return !r;
	}
	//
	bool r;
	StringField *compField = (StringField *)Data;
	switch (op)
	{
	case FILTER_EQUALS:
		r = !Compare(Data);
		break;
	case FILTER_NOTEQUALS:
		r = !!Compare(Data);
		break;
	case FILTER_CONTAINS:
		r = !!Contains(Data);
		break;
	case FILTER_NOTCONTAINS:
		r = !Contains(Data);
		break;
	case FILTER_ABOVE:
		r = (bool)(Compare(Data) > 0);
		break;
	case FILTER_ABOVEOREQUAL:
		r = (bool)(Compare(compField) >= 0);
		break;
	case FILTER_BELOW:
		r = (bool)(Compare(compField) < 0);
		break;
	case FILTER_BELOWOREQUAL:
		r = (bool)(Compare(compField) <= 0);
		break;
	case FILTER_BEGINS:
		r = !!Starts(compField);
		break;
	case FILTER_ENDS:
		{
			CFStringRef compareString = ((StringField*)Data)->GetString();
			if (!String || !compareString) return 0;

			CFRange findRange = CFStringFind(String, compareString, 	kCFCompareCaseInsensitive|kCFCompareNonliteral|kCFCompareAnchored|kCFCompareBackwards);
			if (findRange.location == kCFNotFound)
				r=0;
			else
				r = findRange.location != 0;
		}
		break;
	case FILTER_LIKE:
		/* TODO
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
		*/
		r = !!Compare(compField);
		break;
	case FILTER_BEGINSLIKE:
		/*
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

		r = (bool)(nde_wcsnicmp(d, p, wcslen(p)) == 0);
		*/
		r = !!Starts(compField);
		break;
	default:
		r = true;
		break;
	}
	return r;
}

