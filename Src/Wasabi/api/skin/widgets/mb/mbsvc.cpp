#include "precomp.h"
#include <direct.h>
#include "mbsvc.h"

#ifndef _WASABIRUNTIME

BEGIN_SERVICES(MbSvc_Svc);
DECLARE_SERVICETSINGLE(svc_miniBrowser, MbSvc);
END_SERVICES(MbSvc_Svc, _MbSvc_Svc);

#ifdef _X86_
extern "C" { int _link_MbSvc; }
#else
extern "C" { int __link_MbSvc; }
#endif

#endif

MbSvc::MbSvc()
{
	BrowserWnd::InitializeLibrary();
}

MbSvc::~MbSvc()
{
	BrowserWnd::UninitializeLibrary();
}

int MbSvc::testGuid(GUID g) {
  return (g == GUID_MINIBROWSER_ANY || g == GUID_MINIBROWSER_IEACTIVEX);
}

MiniBrowser *MbSvc::createMiniBrowser() {
  BrowserWnd *w = new BrowserWnd;
  browsers.addItem(w);
  return w;
}

void MbSvc::destroyMiniBrowser(MiniBrowser *b) {
  ASSERT(b != NULL);
  BrowserWnd *bw = static_cast<BrowserWnd *>(b->minibrowser_getRootWnd());
  ASSERT(bw != NULL);
  int i = browsers.searchItem(bw);
  if (i < 0) return;
  browsers.removeByPos(i);
  delete bw;
}
  
