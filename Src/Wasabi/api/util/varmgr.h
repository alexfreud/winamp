#ifndef __VARMGR_H
#define __VARMGR_H

#include <bfc/wasabi_std.h>

class GuiObject;
class Container;
class WaComponent;


class PublicVarManager {
public: 
  static StringW *translate_nocontext(const wchar_t *str);
  static StringW *translate(const wchar_t *str, GuiObject *o=NULL, Container *c=NULL);
};

#endif
