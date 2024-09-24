#include <precomp.h>
#ifndef NOSVCMGR
#include "svcmgr.h"

#ifdef WASABI_COMPILE_COMPONENTS
#include <api/wac/wac.h>
#include <api/wac/compon.h>
#endif

#ifdef WASABI_COMPILE_SYSCB
#include <api/syscb/cbmgr.h>
#include <api/syscb/callbacks/syscb.h>
#include <api/syscb/callbacks/svccb.h>
#endif

#include <bfc/multimap.h>
#include <bfc/map.h>
#include <bfc/critsec.h>

static MultiMap<FOURCC, waServiceFactory> services;// list of factories by class
static Map<waServiceFactory*, GUID> ownermap;	// who presented it
static Map<GUID, waServiceFactory*> services_by_guid;// unique services
static Map<void *, waServiceFactory*> lockmap; // who to tell when it's unlocked
//CUTstatic Map<void *, GUID> clientmap;	// who locked it

static CriticalSection cs;

int ServiceManager::registerService(waServiceFactory *service, GUID owner) {
  ASSERT(owner != INVALID_GUID);
  if (owner == INVALID_GUID) return 0;
  FOURCC svctype = service->getServiceType();
  cs.enter();
  if (!services.multiHaveItem(svctype, service)) {
    services.multiAddItem(svctype, service);

    ownermap.addItem(service, owner);

    GUID svcguid = service->getGuid();
    if (svcguid != INVALID_GUID) services_by_guid.addItem(svcguid, service);
  }
  cs.leave();

  service->serviceNotify(SvcNotify::ONREGISTERED);

  #ifdef WASABI_COMPILE_SYSCB
  CallbackManager::issueCallback(SysCallback::SERVICE,
    SvcCallback::ONREGISTER,
    (int)svctype, reinterpret_cast<int>(service));
  #endif

  return 1;
}

int ServiceManager::deregisterService(waServiceFactory *service, int internal) {
  FOURCC svctype = service->getServiceType();
  // make sure it was there
  cs.enter();
  if (services.multiHaveItem(svctype, service)) {
  // make sure there aren't still services issued by this guy
//  ASSERT(internal || !lockmap.reverseGetItem(service));
    services.multiDelItem(svctype, service);
    ownermap.delItem(service);
    services_by_guid.reverseDelItem(service);
  }
  cs.leave();
  service->serviceNotify(SvcNotify::ONDEREGISTERED);

  #ifdef WASABI_COMPILE_SYSCB
  CallbackManager::issueCallback(SysCallback::SERVICE,
    SvcCallback::ONDEREGISTER,
    (int)svctype, reinterpret_cast<int>(service));
  #endif

  return 1;
}

int ServiceManager::getNumServices(FOURCC svc_type) {
  INCRITICALSECTION(cs);
  return services.multiGetNumItems(svc_type);
}

int ServiceManager::getNumServices() {
  return services.getNumItems(); 
}

waServiceFactory *ServiceManager::enumService(int n) {
  return services_by_guid.enumItemByPos(n, NULL);
}

int ServiceManager::getNumOwners() {
  return ownermap.getNumItems(); 
}

int ServiceManager::getNumServicesByGuid() {
  return services_by_guid.getNumItems(); 
}

int ServiceManager::getNumLocks() {
  return lockmap.getNumItems();
}

waServiceFactory *ServiceManager::enumService(FOURCC svc_type, int n) {
  INCRITICALSECTION(cs);
  waServiceFactory *ret = NULL;
  services.multiGetItem(svc_type, n, &ret);
  return ret;
}

waServiceFactory *ServiceManager::getServiceByGuid(GUID guid) {
  INCRITICALSECTION(cs);
  if (guid == INVALID_GUID) return NULL;
  waServiceFactory *ret=NULL;
  services_by_guid.getItem(guid, &ret);
  return ret;
}

void ServiceManager::sendNotification(int msg, int param1, int param2) {
  cs.enter();
  for (int x = 0; x < services.multiGetNumPairs(); x++) {
    for (int y = 0; ; y++) {
      waServiceFactory *svc;
      if (!services.multiGetItemDirect(x, y, &svc)) {
        break;
      }
      svc->serviceNotify(msg, param1, param2);
    }
  }
  cs.leave();
#ifdef WASABI_COMPILE_COMPONENTS
  // also notify components
  for (int i = 0; ; i++) {
    WaComponent *wac = ComponentManager::enumComponent(i);
    if (wac == NULL) break;
    wac->onNotify(WAC_NOTIFY_SERVICE_NOTIFY, msg, param1, param2);
  }
#endif
  #ifdef WASABI_COMPILE_SYSCB
  // and syscallbacks
  CallbackManager::issueCallback(SysCallback::RUNLEVEL, msg);
  #endif
}

int ServiceManager::lock(waServiceFactory *owner, void *svcptr) {
  INCRITICALSECTION(cs);
  if (owner == NULL || svcptr == NULL) return 0;
  // we allow multiple entries for same service
  lockmap.addItem(svcptr, owner);
  return 1;
}

int ServiceManager::unlock(void *svcptr) {
  if (svcptr == NULL) return 0;

  waServiceFactory *wsvc = NULL;
  cs.enter();
  if (!lockmap.getItem(svcptr, &wsvc)) {
    cs.leave();
    DebugString("WARNING: got unlock with no lock record!");
    return 0;
  }

  int r = lockmap.delItem(svcptr);
  ASSERT(r);

//CUT  // this might fail, client locking isn't enforceable
//CUT  clientmap.delItem(svcptr);

  cs.leave();

  return 1;
}

int ServiceManager::clientLock(void *svcptr, GUID lockedby) {
//CUT  INCRITICALSECTION(cs);
//CUT  ASSERT(svcptr != NULL);
//CUT  if (svcptr == NULL) return 0;
//CUT  ASSERT(lockedby != INVALID_GUID);
//CUT  if (lockedby == INVALID_GUID) return 0;
//CUT  clientmap.addItem(svcptr, lockedby);
  return 1;
}

int ServiceManager::release(void *svcptr) {
  if (svcptr == NULL) return 0;

  waServiceFactory *wsvc = NULL;
  cs.enter();	// note cs getting locked twice via release+unlock
  if (!lockmap.getItem(svcptr, &wsvc)) {
    cs.leave();
    DebugString("WARNING: got release with no lock record!");
    return 0;
  }
  unlock(svcptr);
  cs.leave();
  
  ASSERT(wsvc != NULL);
  return wsvc->releaseInterface(svcptr);
}

#ifdef WASABI_COMPILE_COMPONENTS
GUID ServiceManager::getOwningComponent(void *svcptr) {
  INCRITICALSECTION(cs);
  GUID ret = INVALID_GUID;
  waServiceFactory *svc=NULL;
  if (lockmap.getItem(svcptr, &svc)) ownermap.getItem(svc, &ret);
  return ret;
}

GUID ServiceManager::getLockingComponent(void *svcptr) {
  INCRITICALSECTION(cs);
  GUID ret = INVALID_GUID;
//CUT  clientmap.getItem(svcptr, &ret);
  return ret;
}
#endif

using namespace WaSvc;

static struct {
  FOURCC type;
  const char *name;
} svc_names[] = {
  { DEVICE,		"Portable Device" },
  { FILEREADER,		"File Reader" },
  { FILESELECTOR,	"File Selector" },
  { IMAGEGENERATOR,	"Image Generator" },
  { IMAGELOADER,	"Image Loader" },
  { ITEMMANAGER,	"Item Manager" },
  { MEDIACONVERTER,	"Media Converter" },
  { MEDIACORE,		"Media Core" },
  { PLAYLISTREADER,	"Playlist Reader" },
  { PLAYLISTWRITER,	"Playlist Writer" },
  { SCRIPTOBJECT,	"Script Class" },
  { XMLPROVIDER,	"XML Provider" },
  { DB,                 "Database" },
  { EVALUATOR,		"Evaluator" },
  { COREADMIN,    "Core Administrator" },
  { NONE, NULL },	// this one has to be last
};

const char *ServiceManager::getServiceTypeName(FOURCC svc_type) {
  for (int i = 0; ; i++) {
    if (svc_names[i].name == NULL) break;
    if (svc_names[i].type == svc_type) return svc_names[i].name;
  }
  return NULL;
}

FOURCC ServiceManager::safe_getServiceType(waServiceFactory *was) {
#ifndef _DEBUG
  return 0;
#endif
  FOURCC type = 0;
  _TRY
  {
    type = was->getServiceType();
  }
  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)
  {
    OutputDebugString("EXCEPTION_EXECUTE_HANDLER\n");
  }
  return type;
}

const char *ServiceManager::safe_getServiceName(waServiceFactory *was) {
#ifndef _DEBUG
  return 0;
#endif
  const char *name = NULL;
  _TRY
  {
    name = was->getServiceName();
  }
  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)
  {
    OutputDebugString("EXCEPTION_EXECUTE_HANDLER\n");
  }
  return name;
}

void ServiceManager::onShutdown() {
/*
  ownermap.purge();
  services.purge();
  services_by_guid.purge();
  clientmap.purge();
*/

  int nlocks = lockmap.getNumPairs();
  if (nlocks <= 0) {
    //lockmap.purge();
    #ifdef GEN_FF // i need this to have clean session restart, feel free to remove the ifdef if you fgeel that should always be done
      ownermap.deleteAll();
      services.multiRemoveAll();
      services_by_guid.deleteAll();
      lockmap.deleteAll();
    #endif
    return;
  }

#ifndef _DEBUG
  DebugString("-----------------\n");
  for (int i = 0; i < nlocks; i++) {
    void *ptr = lockmap.enumIndexByPos(i, NULL); ASSERT(ptr != NULL);
    StringPrintf s("lock: %d type:'", (int)ptr);

//    GUID g = lockermap.enumItemByPos(i, INVALID_GUID);ASSERT(g != INVALID_GUID);
//    s += g;
//    WaComponent *wac = ComponentManager::getComponentFromGuid(g);

    waServiceFactory *was=NULL;
    was = lockmap.enumItemByPos(i, NULL);
    ASSERT(was != NULL);
    FOURCC type = safe_getServiceType(was);
    const char *tname = ServiceManager::getServiceTypeName(type);
    if (tname != NULL) {
      s += tname;
    } else {
      FOURCC v = BSWAP(type);
      unsigned char bleh[5]="    ";
      MEMCPY(bleh, &v, 4);
      s += String((char *)bleh);
    }
    s += "' from service:'";
    s += safe_getServiceName(was);

//    s += " wac:";
//    if (wac) s += wac->getName();
#ifdef WASABI_COMPILE_COMPONENTS
    s += "' owned by:'";

    GUID g = INVALID_GUID;
    ownermap.getItem(was, &g);
    if (g != INVALID_GUID) {
      s += g;
      WaComponent *wac = ComponentManager::getComponentFromGuid(g);
      if (wac) s += wac->getName();
    } else s += "(unregistered)";
#else
    GUID g;
#endif

    s += "' registered lock to '";
    g = INVALID_GUID;
    //clientmap.getItem(ptr, &g);
    s += g;

    s += "'\n";
    DebugString(s.v());
  }
  DebugString("-----------------\n");
#endif
#ifdef GEN_FF // i need this to have clean session restart, feel free to remove the ifdef if you fgeel that should always be done
  ownermap.deleteAll();
  services.multiRemoveAll();
  services_by_guid.deleteAll();
  lockmap.deleteAll();
#endif
}

int ServiceManager::isValidService(FOURCC svctype, waServiceFactory *service) {
  INCRITICALSECTION(cs);
  return services.multiHaveItem(svctype, service);
}
#endif