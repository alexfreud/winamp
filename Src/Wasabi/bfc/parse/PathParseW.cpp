#include <bfc/bfc_assert.h>
#include "pathparse.h"

PathParserW::PathParserW(const wchar_t *_str, const wchar_t *sep, int uniquestrs) :
  processed(FALSE), str(_str ? _str : L""), separators(sep), uniques(uniquestrs)
{
  ASSERT(sep != NULL);
}

int PathParserW::getNumStrings() {
  process();
  return strings.getNumItems();
}

wchar_t *PathParserW::enumString(int i) {
  process();
  return strings[i];
}

wchar_t *PathParserW::enumStringSafe(int i, wchar_t *def_val) {
  wchar_t *ret = enumString(i);
  if (ret == NULL) ret = def_val;
  return ret;
}

void PathParserW::process() {
  if (processed) return;
  processed = 1;
  preProcess(str);
  wchar_t *nonconst = str.getNonConstVal();
  wchar_t *context=0;
  
  wchar_t *pt = WCSTOK(nonconst, separators, &context);
  if (pt == NULL) return;
  postProcess(pt);
  strings.addItem(pt);
  for (;;) {
    wchar_t *pt = WCSTOK(NULL, separators, &context);
    if (pt == NULL) break;
    postProcess(pt);
    if (uniques) {
      int exists = 0;
      foreach(strings)
        if (!WCSICMP(strings.getfor(), pt)) 
				{
          exists=1;
          break;
        }
      endfor;
      if (exists) continue;
    }
    strings.addItem(pt);
  }
}

