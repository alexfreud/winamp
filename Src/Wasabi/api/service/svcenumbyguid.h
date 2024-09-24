#ifndef _SVCENUMBYGUID_H
#define _SVCENUMBYGUID_H

/*<?<autoheader/>*/
/*?>*/

#include "svc_enum.h"

/*
 * this is a helper class to fetch a service * by GUID
 * usage: svc_something *svc = SvcEnumByGuid<svc_something>(svcguid);
 * @short: Helper class to fetch unique service by GUID
*/
template <class SERVICETYPE>
class SvcEnumByGuid {
public:
/**
@param _guid The GUID of the service factory to fetch the service from.
*/
  SvcEnumByGuid() : guid(SERVICETYPE::getServiceGuid()) {}
  SvcEnumByGuid(GUID _guid) : guid(_guid) {}

/**
@return The pointer to the service.
*/
  SERVICETYPE *getInterface() {
    waServiceFactory *svc = WASABI_API_SVC->service_getServiceByGuid(guid);
    return castService<SERVICETYPE>(svc);
  }

  operator SERVICETYPE *() { return getInterface(); }

private:
  GUID guid;
};

#endif // _SVCENUMBYGUID_H
