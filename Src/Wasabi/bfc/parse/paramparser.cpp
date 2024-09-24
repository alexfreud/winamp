#include "precomp_wasabi_bfc.h"
#include "paramparser.h"
#include <bfc/nsguid.h>
#include <bfc/wasabi_std.h>

int ParamParser::findGuid(GUID g) {
  for (int i=0;i<getNumItems();i++) {
    const wchar_t *e = enumItem(i);
    GUID eguid = nsGUID::fromCharW(e);
    if (g == eguid) return i;
  }
  return -1;
}

int ParamParser::findString(const wchar_t *str) {
  for (int i=0;i<getNumItems();i++) {
    if (!WCSICMP(str, enumItem(i))) return i;
  }
  return -1;
}