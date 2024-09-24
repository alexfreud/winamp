#ifndef __WASERVICEFACTORYT_IMPL_H
#define __WASERVICEFACTORYT_IMPL_H

/*<?<autoheader/>*/
/*?>*/

#include "waservicefactorybase.h"

// this is a service factory template that will manufacture any number
// of a given class SERVICE, which is derived from service class SERVICETYPE
template <class SERVICETYPE, class SERVICE>
class waServiceFactoryT : public waServiceFactoryBase<SERVICETYPE, SERVICE> {
public:
  waServiceFactoryT(GUID myGuid = INVALID_GUID) :
    waServiceFactoryBase<SERVICETYPE, SERVICE>(myGuid) {}
  virtual SERVICETYPE *newService() {
    SERVICE *ret = new SERVICE;
    ASSERT(ret != NULL);
    return ret;
  }
  virtual int delService(SERVICETYPE *service) {
    ASSERT(service != NULL);
    delete static_cast<SERVICE*>(service);
    return 1;
  }
};

#endif // __WASERVICEFACTORYT_IMPL_H
