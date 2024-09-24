#include <precomp.h>
#ifndef NOSVCMGR
//<?#include "<class data="implementationheader"/>"
#include "api_servicei.h"
//?>

#include <api/api.h>
#include <api/service/svcmgr.h>

api_service *serviceApi = NULL;

api_serviceI::api_serviceI() {
}

api_serviceI::~api_serviceI() {
  ServiceManager::onShutdown();
}

int api_serviceI::service_register(waServiceFactory *svc) {
#ifdef WASABI_COMPILE_COMPONENTS
  int r = ServiceManager::registerService(svc, WASABI_API_COMPONENT->getThisGuid());
#else
  int r = ServiceManager::registerService(svc, WASABI_API_APP->main_getGUID());
#endif
  return r;
}

int api_serviceI::service_deregister(waServiceFactory *svc) {
  int r = ServiceManager::deregisterService(svc);
  return r;
}

int api_serviceI::service_getNumServices(FOURCC svc_type) {
  return ServiceManager::getNumServices(svc_type);
}

waServiceFactory *api_serviceI::service_enumService(FOURCC svc_type, int n) {
  return ServiceManager::enumService(svc_type, n);
}

waServiceFactory *api_serviceI::service_getServiceByGuid(GUID guid) {
  return ServiceManager::getServiceByGuid(guid);
}


int api_serviceI::service_lock(waServiceFactory *owner, void *svcptr) {
  return ServiceManager::lock(owner, svcptr);
}

int api_serviceI::service_clientLock(void *svcptr) {
#ifdef WASABI_COMPILE_COMPONENTS
  return ServiceManager::clientLock(svcptr, WASABI_API_COMPONENT->getThisGuid());
#else
  return ServiceManager::clientLock(svcptr, WASABI_API_APP->main_getGUID());
#endif
}

int api_serviceI::service_release(void *svcptr) {
  return ServiceManager::release(svcptr);
}

const char *api_serviceI::service_getTypeName(FOURCC svc_type) {
  return ServiceManager::getServiceTypeName(svc_type);
}

#ifdef WASABI_COMPILE_COMPONENTS
GUID api_serviceI::service_getOwningComponent(void *svcptr) {
  return ServiceManager::getOwningComponent(svcptr);
}

GUID api_serviceI::service_getLockingComponent(void *svcptr) {
  return ServiceManager::getLockingComponent(svcptr);
}
#endif

int api_serviceI::service_unlock(void *svcptr) {
  return ServiceManager::unlock(svcptr);
}

int api_serviceI::service_isvalid(FOURCC svctype, waServiceFactory *service) {
  return ServiceManager::isValidService(svctype, service);
}
#endif