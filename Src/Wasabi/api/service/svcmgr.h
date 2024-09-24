#ifndef _SVCMGR_H
#define _SVCMGR_H

#include <api/service/service.h>

class ServiceManager {
public:
  static int registerService(waServiceFactory *service, GUID owner);
  static int deregisterService(waServiceFactory *service, int internal = 0);

  static int getNumServices(FOURCC svc_type);
  static waServiceFactory *enumService(FOURCC svc_type, int n);

  static waServiceFactory *getServiceByGuid(GUID guid);

  static void sendNotification(int msg, int param1 = 0, int param2 = 0);

  static int lock(waServiceFactory *owner, void *svcptr);
  static int unlock(void *svcptr);
  static int clientLock(void *svcptr, GUID lockedby);
  static int release(void *svcptr);

  static GUID getOwningComponent(void *svcptr);
  static GUID getLockingComponent(void *svcptr);
  static const char *getServiceTypeName(FOURCC svc_type);

  static void onShutdown();

  static FOURCC safe_getServiceType(waServiceFactory *was);
  static const char *safe_getServiceName(waServiceFactory *was);

  static int getNumServices();
  static waServiceFactory *enumService(int n);
  static int getNumServicesByGuid();
  static int getNumOwners();
  static int getNumLocks();

  static int isValidService(FOURCC svctype, waServiceFactory *service);
};

#endif
