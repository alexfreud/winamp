#ifndef __SERVICE_APII_IMPL_H
#define __SERVICE_APII_IMPL_H

/*<?<autoheader/>*/
#include "api_service.h"
#include "api_servicex.h"

class waServiceFactory;
/*?>*/

class api_serviceI : public api_serviceX {
public:
  NODISPATCH api_serviceI();
  NODISPATCH virtual ~api_serviceI();

// services
  /**
    Register a service. Service registration is done
    on creation of the WAC.
    
    @see WAComponentClient
    @ret 1, success; 0, failure.
    @param p Pointer to your service factory.
  */
  DISPATCH(10) int service_register(waServiceFactory *svc);
  
  /**
    Deregister a service. Service deregistration is done
    on destruction of the WAC.
    
    @see WAComponentClient
    @ret 1, success; 0, failure.
    @param p Pointer to your service factory.
  */
  DISPATCH(20) int service_deregister(waServiceFactory *svc);
  
  /**
    Get the number of services registered for a specific
    service type. This should only be called after
    Wasabi is fully started (after WAC creation).
    
    @see FOURCC
    @ret Number of services present in that service type.
    @param svc_type Service type.
  */
  DISPATCH(30) int service_getNumServices(FOURCC svc_type);	// see common/svc_enum.h
  
  // enumerate by family
  /**
    Enumerate services by family. This should only 
    be called after Wasabi is fully started (after WAC creation).
    
    @see FOURCC
    @ret Requested service.
    @param svc_type Service type.
    @param n        Number of the service.
  */
  DISPATCH(40) waServiceFactory *service_enumService(FOURCC svc_type, int n);
  
  // fetch by GUID
  /**
    Get a service by it's GUID. This should only
    be called after Wasabi is fully started (after WAC creation).

    @ret Requested service.
    @param guid Service GUID.
  */
  DISPATCH(50) waServiceFactory *service_getServiceByGuid(GUID guid);
  
  // service owner calls this when it issues a service *
  /**
    Lock a service. Service owner must call this when
    it issues a new service pointer to a client.
    
    @ret 1, success; 0, failure;
    @param owner Service owner.
    @param svcptr Service pointer returned to client.
  */
  DISPATCH(60) int service_lock(waServiceFactory *owner, void *svcptr);
  
  // service client calls this when it uses a service *
  /**
    ClientLock a service. Service client must call
    this before using the service.
    
    @ret 1, success; 0, failure;
    @param svcptr Service pointer.
  */
  DISPATCH(70) int service_clientLock(void *svcptr);
  
  // service client calls this when done w/ service *
  /**
    Release a service. Service client must call this
    when he's finished using the service. If the service
    is NOT released it will cause improper shutdown of
    the service.
    
    @ret 1, success; 0, failure;
  */
  DISPATCH(80) int service_release(void *svcptr);
  /**
    Get the pretty printed type name of a service type based
    on it's FOURCC.
    
    @see FOURCC
    @ret Service name (readable).
    @param svc_type Service type.
  */
  DISPATCH(90) const char *service_getTypeName(FOURCC svc_type);
  
#ifdef WASABI_COMPILE_COMPONENTS

/*[interface.service_getOwningComponent.cpp]#ifdef WASABI_COMPILE_COMPONENTS*/
/*[interface.service_getOwningComponent.h]#ifdef WASABI_COMPILE_COMPONENTS*/
/*[dispatchable.service_getOwningComponent.enum]#ifdef WASABI_COMPILE_COMPONENTS*/
/*[dispatchable.service_getOwningComponent.bridge]#ifdef WASABI_COMPILE_COMPONENTS*/
  /**
    Get the owning component of a service from
    a service pointer.
    
    @ret GUID of the owning component.
    @param svcptr Service pointer.
  */
  DISPATCH(100) GUID service_getOwningComponent(void *svcptr);
  
  /**
    Get the locking component for a service from
    a service pointer.
    
    @ret GUID of the locking component.
    @param svcptr Service pointer.
  */
  DISPATCH(110) GUID service_getLockingComponent(void *svcptr);

/*[interface.service_unlock.cpp]#endif // WASABI_COMPILE_COMPONENTS*/
/*[interface.service_unlock.h]#endif // WASABI_COMPILE_COMPONENTS*/
/*[dispatchable.service_unlock.enum]#endif // WASABI_COMPILE_COMPONENTS*/
/*[dispatchable.service_unlock.bridge]#endif // WASABI_COMPILE_COMPONENTS*/
#endif

  DISPATCH(120) int service_unlock(void *svcptr);

  DISPATCH(130) int service_isvalid(FOURCC svctype, waServiceFactory *service);
};

/*[interface.footer.h]
extern api_service *serviceApi;
*/

#endif // __SERVICE_APII_IMPL_H
