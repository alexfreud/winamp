#ifndef __PARAMPARSE_H
#define __PARAMPARSE_H

#include <bfc/parse/pathparse.h>

class ParamParser : private PathParserW
{
  public:
    
    ParamParser(const wchar_t *str, const wchar_t *sep=L";", int unique=0) : PathParserW(str, sep, unique) {}

    int findGuid(GUID g);
    int findString(const wchar_t *str);

    int hasGuid(GUID g) { return findGuid(g) >= 0; }
    int hasString(const wchar_t *str) { return findString(str) >= 0; }

    const wchar_t *enumItem(int element) { return enumString(element); }
    int getNumItems() { return getNumStrings(); }
    const wchar_t *getLastItem() { return getLastString(); }
};

#endif
