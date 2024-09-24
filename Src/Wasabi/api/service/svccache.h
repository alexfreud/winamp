#ifndef _SVC_CACHE_H
#define _SVC_CACHE_H

#include <api/service/svc_enum.h>
#include <bfc/ptrlist.h>

/**
  This is a caching version of SvcEnum. Upon creation, it enumerates all
  service factories in the family and keeps them in a list. Then you can
  call findService() with a search string to quickly find the service you
  want. If you don't have a search string, you can still use a SvcEnum.
*/

class SvcCache {
protected:
  SvcCache(FOURCC type);
 
public:
  waServiceFactory *findServiceFactory(const wchar_t *searchval);

private:
  class waServiceFactoryCompare {
  public:
    static int compareItem(waServiceFactory *p1, waServiceFactory* p2);
    static int compareAttrib(const wchar_t *attrib, waServiceFactory *item);
  };
  PtrListQuickSorted<waServiceFactory, waServiceFactoryCompare> list;
};

template <class T>
class SvcCacheT : public SvcCache {
public:
  SvcCacheT() : SvcCache(T::getServiceType()) { }

  T *findService(const char *key, int global_lock=TRUE) {
    waServiceFactory *sf = findServiceFactory(key);
    if (sf == NULL) return NULL;
    T *ret = castService<T>(sf, global_lock);
    return ret;
  }
};

#endif
