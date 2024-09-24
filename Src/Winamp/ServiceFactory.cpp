#include "main.h"
#include "api.h"
#include <api/service/waservicefactorybase.h>

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS waServiceFactoryX
START_DISPATCH;
  CB(WASERVICEFACTORY_GETSERVICETYPE, x_getServiceType);
  CB(WASERVICEFACTORY_GETSERVICENAME, x_getServiceName);
  CB(WASERVICEFACTORY_GETGUID, getGuid);
  case WASERVICEFACTORY_GETINTERFACE:
    switch (nparam) {
      default: cb<CBCLASS>(&CBCLASS::getInterface, retval, params); break;
      case 0: cb<CBCLASS>(&CBCLASS::_RETIRED_getInterface, retval, params); break;
    }
  break;
  CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, supportNonLockingGetInterface);
  CB(WASERVICEFACTORY_RELEASEINTERFACE, releaseInterface);
  CB(WASERVICEFACTORY_GETTESTSTRING, getTestString);
  CB(WASERVICEFACTORY_SERVICENOTIFY, serviceNotify);
END_DISPATCH;
#undef CBCLASS 