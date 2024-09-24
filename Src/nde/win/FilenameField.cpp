#include "FilenameField.h"
#include "../nde.h"


//---------------------------------------------------------------------------
FilenameField::FilenameField(const wchar_t *Str, int strkind) : StringField(Str, strkind)
{
	Type = FIELD_FILENAME;
}

//---------------------------------------------------------------------------
FilenameField::FilenameField()
{
	Type = FIELD_FILENAME;
}

//---------------------------------------------------------------------------
int FilenameField::Compare(Field *Entry)
{
	if (!Entry) return -1;
	if (!CompatibleFields(Entry->GetType(), GetType()))
		return 0;
	return mywcsicmp_fn(GetStringW(), ((FilenameField*)Entry)->GetStringW());
}

//---------------------------------------------------------------------------
int FilenameField::Starts(Field *Entry)
{
	if (!Entry) return -1;
	if (!CompatibleFields(Entry->GetType(), GetType()))
		return 0;
	const wchar_t *p = ((StringField*)Entry)->GetStringW();
	const wchar_t *d = GetStringW();
		if (!d || !p) return 0;
	return nde_fnbegins(d, p);
}

//---------------------------------------------------------------------------
int FilenameField::Contains(Field *Entry)
{
	if (!Entry) return -1;
	if (!CompatibleFields(Entry->GetType(), GetType()))
		return 0;
	const wchar_t *p = ((StringField*)Entry)->GetStringW();
	const wchar_t *d = GetStringW();
	if (!d || !p) return 0;
	return nde_fncontains(GetStringW(), ((StringField*)Entry)->GetStringW());
}

Field *FilenameField::Clone(Table *pTable)
{
	FilenameField *clone = new FilenameField(StringW, STRING_IS_NDESTRING);
	clone->Pos = FIELD_CLONE;
	clone->ID = ID;
	clone->MaxSizeOnDisk = (uint32_t)GetDataSize();
	return clone;
}

// TODO: make file system string comparison functions
//---------------------------------------------------------------------------
bool FilenameField::ApplyFilter(Field *Data, int op)
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
	wchar_t *p=0;
	if (Data->GetType() == FIELD_STRING)
		p = ((StringField *)Data)->GetStringW();
	else
		p = ((FilenameField *)Data)->GetStringW();
	wchar_t *d = GetStringW();
	if (!p)
		p = L"";
	if (!d)
		d = L"";

	switch (op)
	{
		case FILTER_EQUALS:
			r = !nde_wcsicmp_fn(d, p);
			break;
		case FILTER_NOTEQUALS:
			r = !!nde_wcsicmp_fn(d, p);
			break;
		case FILTER_CONTAINS:
			r = nde_fncontains(d, p);
			break;
		case FILTER_NOTCONTAINS:
			r = !nde_fncontains(d, p);
			break;
		case FILTER_ABOVE:
			r = (bool)(nde_wcsicmp_fn(d, p) > 0);
			break;
		case FILTER_ABOVEOREQUAL:
			r = (bool)(nde_wcsicmp_fn(d, p) >= 0);
			break;
		case FILTER_BELOW:
			r = (bool)(nde_wcsicmp_fn(d, p) < 0);
			break;
		case FILTER_BELOWOREQUAL:
			r = (bool)(nde_wcsicmp_fn(d, p) <= 0);
			break;
		case FILTER_BEGINS:
			r = nde_fnbegins(d, p);
			break;
		case FILTER_ENDS:
			r = nde_fnends(d, p);
			break;
		case FILTER_LIKE:
			r = (bool)(nde_wcsicmp_fn(d, p) == 0);
			break;
		default:
			r = true;
			break;
	}
	return r;
}