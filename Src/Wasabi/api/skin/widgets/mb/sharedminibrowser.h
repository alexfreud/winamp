#ifndef _SHAREDMINIBROWSER_H
#define _SHAREDMINIBROWSER_H

#include "../studio/skincb.h"

class SkinMonitor;

class SharedMiniBrowser {
public:
  static void navigateUrl(const char *url);
  static void shutdown();

  static int m_inserted;
  static SkinMonitor *m_monitor;

};

class SkinMonitor : public SkinCallbackI {
public:
  SkinMonitor() {
    WASABI_API_SYSCB->syscb_registerCallback(this);
  }
  virtual ~SkinMonitor() {
    WASABI_API_SYSCB->syscb_deregisterCallback(this);
  }
  virtual int skincb_onReset() { 
    SharedMiniBrowser::m_inserted = 0;
    return 0;
  }
};


#endif // _SHAREDMINIBROWSER_H
