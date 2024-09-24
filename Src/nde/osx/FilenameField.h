#ifndef NDE_FILENAMEFIELD_H
#define NDE_FILENAMEFIELD_H

/*
  Mostly the same as StringField
  but this implements OS-dependent string comparisons that make sense for the file system
*/

#include "nde.h"
#include "NDEString.h"

class FilenameField : public StringField
{
protected:
	virtual int Compare(Field *Entry);
	virtual Field *Clone(Table *pTable);

public:
	FilenameField(CFStringRef Str);
	FilenameField();
};

#endif