#ifndef _SYSMEMCB_H
#define _SYSMEMCB_H



namespace SysMemCallback {
  enum {
    ONMALLOC=10,
    ONFREE=20,
    ONREALLOC=30,
    ONCHANGE=40,
  };
};


#include <api/syscb/callbacks/syscbi.h>

#define SYSMEMCALLBACK_PARENT SysCallbackI
class SysMemCallbackI : public SYSMEMCALLBACK_PARENT {
protected:
  SysMemCallbackI() { }

public:
  virtual void sysmem_onMalloc(void *memory, int size)=0;
  virtual void sysmem_onFree(void *memory)=0;
  virtual void sysmem_onRealloc(void *prev_memory, void *new_memory, int new_size)=0;
  virtual void sysmem_onChange(void *memory)=0;

private:
  virtual FOURCC syscb_getEventType() { return SysCallback::SYSMEM; }
  virtual int syscb_notify(int msg, intptr_t param1=0, intptr_t param2=0) {
    switch (msg) {
      case SysMemCallback::ONMALLOC:
         sysmem_onMalloc((void *)param1, param2);
      break;
      case SysMemCallback::ONFREE:
         sysmem_onFree((void *)param1);
      break;
      case SysMemCallback::ONREALLOC: {
         void **ptrs = (void **)param1;
         sysmem_onRealloc(ptrs[0], ptrs[1], param2);
      }
      break;
      case SysMemCallback::ONCHANGE:
         sysmem_onChange((void *)param1);
      break;
      default: return 0;
    }
    return 1;
  }
};

#endif
