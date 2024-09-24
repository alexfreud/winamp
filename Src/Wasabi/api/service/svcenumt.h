#ifndef _SVCENUMT_H
#define _SVCENUMT_H

#include "svcenum.h"

template <class T>
class SvcEnumT : private SvcEnum {
protected:
  SvcEnumT() { type = T::getServiceType(); }

public:
  void reset() { SvcEnum::reset(); }
  T *getFirst(int global_lock = TRUE) { reset(); return getNext(global_lock); }
  T *getNext(int global_lock = TRUE) { return static_cast<T*>(_getNext(global_lock)); }

  // these would just be 'using' but msvc.net sucks butt
  inline int release(void *ptr) { return SvcEnum::release(ptr); }
  inline waServiceFactory *getLastFactory() { return SvcEnum::getLastFactory(); }

protected:
  // override this one (or don't if you want to return all of them)
  virtual int testService(T *svc) { return TRUE; }

private:
  virtual int _testService(void *svc) {
    return testService(static_cast<T *>(svc));
  }
};

#endif // _SVCENUMT_H
