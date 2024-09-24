#include "FilenameField.h"
#include "../nde.h"


//---------------------------------------------------------------------------
FilenameField::FilenameField(const char *Str, int strkind) : StringField(Str, strkind)
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
	return mystricmp_fn(GetString(), ((FilenameField*)Entry)->GetString());
}

//---------------------------------------------------------------------------
int FilenameField::Starts(Field *Entry)
{
	if (!Entry) return -1;
	if (!CompatibleFields(Entry->GetType(), GetType()))
		return 0;
	return (mystristr_fn(GetString(), ((FilenameField*)Entry)->GetString()) == GetString());
}

//---------------------------------------------------------------------------
int FilenameField::Contains(Field *Entry)
{
	if (!Entry) return -1;
	if (!CompatibleFields(Entry->GetType(), GetType()))
		return 0;
	return (mystristr_fn(GetString(), ((FilenameField*)Entry)->GetString()) != NULL);
}


Field *FilenameField::Clone(Table *pTable)
{
	FilenameField *clone = new FilenameField(String, STRING_IS_NDESTRING);
	clone->Pos = FIELD_CLONE;
	clone->ID = ID;
	clone->MaxSizeOnDisk = GetDataSize();
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
		if (!String)
			return r;

		if (String && String[0] == 0)
			return r;

		return !r;
	}
	//
	bool r;
	const char *p=0;
	if (Data->GetType() == FIELD_STRING)
		p = ((StringField *)Data)->GetString();
	else
		p = ((FilenameField *)Data)->GetString();
	const char *d = GetString();
	if (!p)
		p = "";
	if (!d)
		d = "";

	switch (op)
	{
	case FILTER_EQUALS:
		r = !nde_stricmp_fn(d, p);
		break;
	case FILTER_NOTEQUALS:
		r = !!nde_stricmp_fn(d, p);
		break;
	case FILTER_CONTAINS:
		r = (NULL != stristr_fn(d, p));
		break;
	case FILTER_NOTCONTAINS:
		r = (NULL == stristr_fn(d, p));
		break;
	case FILTER_ABOVE:
		r = (bool)(nde_stricmp_fn(d, p) > 0);
		break;
	case FILTER_ABOVEOREQUAL:
		r = (bool)(nde_stricmp_fn(d, p) >= 0);
		break;
	case FILTER_BELOW:
		r = (bool)(nde_stricmp_fn(d, p) < 0);
		break;
	case FILTER_BELOWOREQUAL:
		r = (bool)(nde_stricmp_fn(d, p) <= 0);
		break;
	case FILTER_BEGINS:
		r = (bool)(nde_strnicmp_fn(d, p, strlen(p)) == 0);
		break;
	case FILTER_ENDS:
		{
			int lenp = (int)strlen(p), lend = (int)strlen(d);
			if (lend < lenp) return 0;  // too short
			r = (bool)(nde_stricmp_fn((d + lend) - lenp, p) == 0);
		}
		break;
	case FILTER_LIKE:
		r = (bool)(nde_stricmp_fn(d, p) == 0);
		break;
	default:
		r = true;
		break;
	}
	return r;
}
