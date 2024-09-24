#ifndef __WASERVICEFACTORYTSINGLE_IMPL_H
#define __WASERVICEFACTORYTSINGLE_IMPL_H

/*<?<autoheader/>*/
/*?>*/

#include "waservicefactorybase.h"

#include <bfc/bfc_assert.h>

// this is a service factory template that holds one copy of a class
// and reissues the pointer as needed, with reference counting
template <class SERVICETYPE, class SERVICE>
class waServiceFactoryTSingle : public waServiceFactoryBase<SERVICETYPE, SERVICE> {
public:
  waServiceFactoryTSingle(GUID myGuid = INVALID_GUID) :
    refcount(0), waServiceFactoryBase<SERVICETYPE, SERVICE>(myGuid) {
    singleService = new SERVICE;
  }
  waServiceFactoryTSingle(SERVICE *svc, GUID myGuid = INVALID_GUID) :
    singleService(svc), refcount(0),
    waServiceFactoryBase<SERVICETYPE, SERVICE>(myGuid) { }
  ~waServiceFactoryTSingle() {
    delete singleService;
  }
  virtual SERVICETYPE *newService() {
    ASSERT(singleService != NULL);
    refcount++;
    return singleService;
  }
  virtual int delService(SERVICETYPE *service) {
    ASSERT(static_cast<SERVICE*>(service) == singleService);
    refcount--;
    ASSERT(refcount >= 0);
    return 1;
  }

  SERVICE *getSingleService() { return singleService; }

private:
  SERVICE * singleService;
  int refcount;
};



#endif // __WASERVICEFACTORYTSINGLE_IMPL_H
