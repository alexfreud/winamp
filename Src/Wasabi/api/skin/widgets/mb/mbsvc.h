#ifndef _SVC_H
#define _SVC_H

#include <api/service/svcs/svc_minibrowser.h>
#include <bfc/ptrlist.h>
#include <api/service/svc_enum.h>
#include "iebrowser.h"

class MbSvc : public svc_miniBrowserI {
public:
  MbSvc();
  ~MbSvc();

  static const char *getServiceName() { return "Internet Explorer ActiveX MiniBrowser Service"; }
  virtual int testQueryFormat(int queryformat) { return WaSvc::MINIBROWSER; }

  virtual int testGuid(GUID g);
  virtual MiniBrowser *createMiniBrowser();
  virtual void destroyMiniBrowser(MiniBrowser *w);
  
private:

  PtrList<BrowserWnd> browsers;
};

#endif
