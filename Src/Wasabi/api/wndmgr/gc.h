#ifndef __GARBAGECOLLECT_H
#define __GARBAGECOLLECT_H

#include <api/syscb/callbacks/gccb.h>

class GarbageCollector : public GarbageCollectCallbackI {
  public:
    GarbageCollector();
    virtual ~GarbageCollector();

    virtual int gccb_onGarbageCollect();

  private:
   uint32_t last;
};

extern GarbageCollector  *garbageCollector;

#endif
