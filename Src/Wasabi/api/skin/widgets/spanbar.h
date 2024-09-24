#ifndef _SPANBAR_H
#define _SPANBAR_H

#include "pslider.h"
#include <api/syscb/callbacks/corecbi.h>

#define SPANBAR_PARENT PSliderWnd
#define SPANBAR_XMLPARENT PSliderWnd

class SPanBar : public SPANBAR_PARENT, public CoreCallbackI {
public:
  SPanBar();
  virtual ~SPanBar();

  virtual int onInit();
  virtual void lock();
  virtual void unlock();

protected:
  int locked;
  virtual int onSetPosition();

  virtual int corecb_onPanChange(int newpan);
};

extern const wchar_t panBarXuiStr[];
extern char panBarXuiSvcName[];
class PanBarXuiSvc : public XuiObjectSvc<SPanBar, panBarXuiStr, panBarXuiSvcName> {};


#endif
