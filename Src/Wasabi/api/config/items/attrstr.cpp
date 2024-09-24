#include <precomp.h>
#include "attrstr.h"

#include <bfc/memblock.h>

const wchar_t *_string::getValue() 
{
  int l = getDataLen();
  if (l <= 0) return L"";
  MemBlock<wchar_t> mb(l+2);
  getData(mb.getMemory(), l+2);
  returnval = mb;
  return returnval;
}
