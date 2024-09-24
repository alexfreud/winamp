#ifndef NULLSOFT_WASABI_SVCCBI_H
#define NULLSOFT_WASABI_SVCCBI_H

#include "syscbi.h"
#include "svccb.h"

class waServiceFactory;
#define SVCCALLBACK_PARENT SysCallbackI
class SvcCallbackI : public SVCCALLBACK_PARENT {
protected:
  SvcCallbackI() { }

public:
  virtual void svccb_onSvcRegister(FOURCC type, waServiceFactory *svc) { }
  virtual void svccb_onSvcDeregister(FOURCC type, waServiceFactory *svc) { }

private:
  virtual FOURCC syscb_getEventType() { return SysCallback::SERVICE; }

  virtual int syscb_notify(int msg, intptr_t param1, intptr_t param2) {
    switch (msg) {
      case SvcCallback::ONREGISTER:
        svccb_onSvcRegister((FOURCC)param1, reinterpret_cast<waServiceFactory*>(param1));
      break;
      case SvcCallback::ONDEREGISTER:
        svccb_onSvcRegister((FOURCC)param1, reinterpret_cast<waServiceFactory*>(param1));
      break;
      default: return 0;
    }
    return 1;
  }
};

#endif