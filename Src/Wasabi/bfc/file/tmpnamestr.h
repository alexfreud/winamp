#ifndef _TMPNAMESTR_H
#define _TMPNAMESTR_H

#include <bfc/string/StringW.h>

class TmpNameStrW : public StringW
{
public:
  TmpNameStrW()
	{
    wchar_t tmp[WA_MAX_PATH]=L"";
    TMPNAM(tmp);
    setValue(tmp); 
  }
};


#endif
