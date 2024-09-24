#ifndef __API_LOCALESI_IMPL_H
#define __API_LOCALESI_IMPL_H

/*<?<autoheader/>*/
#include "api_locales.h"
#include "api_localesx.h"
/*?>*/

class api_localesI : public api_localesX 
{
public:
  api_localesI();
  virtual ~api_localesI();
  
  DISPATCH(10) const wchar_t *locales_getTranslation(const wchar_t *str); // if not found, returns the str paramer
  DISPATCH(20) void locales_addTranslation(const wchar_t *from, const wchar_t *to);
  DISPATCH(30) const wchar_t *locales_getBindFromAction(int action);
  //DISPATCH(40) int locales_getNumEntries();
  //DISPATCH(50) const wchar_t *locales_enumEntry(int n);
  DISPATCH(60) void locales_registerAcceleratorSection(const wchar_t *name, ifc_window *wnd, int global = 0);
};

#endif // __API_LOCALESI_IMPL_H