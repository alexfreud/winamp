#ifndef __ROOTOBJCB_H
#define __ROOTOBJCB_H

//<?<autoheader/>
#include "rootobjcb.h"
#include "rootobjcbx.h"

//?>

class NOVTABLE RootObjectCallbackI : public RootObjectCallbackX {
  public:
  DISPATCH(10) virtual void rootobjectcb_onNotify(const wchar_t *a, const wchar_t *b, int c, int d)=0;
};

#endif
