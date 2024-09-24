#include "FilenameField.h"
#include "nde.h"

/*
Mac OS X implementation of FilenameField
 only the equals operator will be case-sensitive.  substring search, ends, starts, etc. will be case-insensitive, 
 to make things like "filename ends .mp3" easier
 
TODO: it'd be massive overhead, but it'd be more correct to check if the file system is actually case sensitive (for the path being searched)
*/

//---------------------------------------------------------------------------
FilenameField::FilenameField(CFStringRef Str) : StringField(Str)
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
	if (Entry->GetType() != GetType()) return 0;
	
	CFStringRef compareString = ((StringField*)Entry)->GetString();
	if (!String && !compareString) return 0;
	if (!String && compareString) return 1;
	if (!compareString) return -1;
	
	return CFStringCompare(String, compareString, 0);
}

Field *FilenameField::Clone(Table *pTable)
{
	FilenameField *clone = new FilenameField(String);
	clone->Pos = FIELD_CLONE;
	clone->ID = ID;
	clone->MaxSizeOnDisk = GetDataSize();
	return clone;
}
