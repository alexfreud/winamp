#ifndef NULLSOFT_WASABI_BROWSERCBI_H
#define NULLSOFT_WASABI_BROWSERCBI_H

#include "syscbi.h"
#include "browsercb.h"

class waServiceFactory;
#define SVCCALLBACK_PARENT SysCallbackI
class BrowserCallbackI : public SVCCALLBACK_PARENT {
protected:
  BrowserCallbackI() { }

public:
	// set *override  = true to prevent the URL from being opened
	// leave it alone otherwise (in case someone else wanted to override it)
  virtual void browsercb_onOpenURL(wchar_t *url, bool *override) { }

private:
  virtual FOURCC syscb_getEventType() { return SysCallback::BROWSER; }

  virtual int syscb_notify(int msg, intptr_t param1, intptr_t param2) {
    switch (msg) {
      case BrowserCallback::ONOPENURL:
        browsercb_onOpenURL(reinterpret_cast<wchar_t*>(param1), reinterpret_cast<bool *>(param2));
      break;
      default: return 0;
    }
    return 1;
  }
};

#endif