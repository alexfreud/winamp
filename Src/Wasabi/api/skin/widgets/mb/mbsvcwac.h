#ifndef _MB_H
#define _MB_H

#include "../ns_database/nde.h"
#include "../studio/wac.h"

#define WACNAME WACmb

class GenWnd;

class WACNAME : public WAComponentClient {
  public:
    WACNAME();
    virtual ~WACNAME();

    virtual const char *getName() { return "Internet Explorer ActiveX MiniBrowser Service"; };
    virtual GUID getGUID();

  private:
};

extern WACNAME *wacmb;

#endif
