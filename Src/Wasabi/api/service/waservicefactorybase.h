#ifndef __WASERVICEFACTORYBASE_IMPL_H
#define __WASERVICEFACTORYBASE_IMPL_H

/*<?<autoheader/>*/
class CfgItem;
/*?>*/

#include "waservicefactoryi.h"

template <class SERVICETYPE, class SERVICE>
class NOVTABLE waServiceFactoryBaseX : public waServiceFactoryI {
public:
  waServiceFactoryBaseX(GUID myGuid = INVALID_GUID) : guid(myGuid) {}
  virtual FOURCC svc_serviceType()=0;
  virtual const char *svc_getServiceName() { return SERVICE::getServiceName(); }
  virtual GUID svc_getGuid() { return guid; }
  virtual void *svc_getInterfaceAndLock() {// this is only for back compat
    return getInterface(TRUE);
  }

  virtual void *svc_getInterface(int global_lock)=0;
  virtual int svc_releaseInterface(void *ptr)=0;

  virtual CfgItem *svc_getCfgInterface() { return NULL; }
  virtual const wchar_t *svc_getTestString() { return NULL; }

  virtual int svc_notify(int msg, int param1 = 0, int param2 = 0) { return 0; }

private:
  GUID guid;
};

// if you derive from this, all you have to do is override the newService()
// and delService() methods... if all you need to do is instantiate a class
// and destroy it use the helper below (waServiceFactoryT)
template <class SERVICETYPE, class SERVICE>
class NOVTABLE waServiceFactoryBase : public waServiceFactoryBaseX<SERVICETYPE, SERVICE> {
public:
  waServiceFactoryBase(GUID myGuid = INVALID_GUID) : waServiceFactoryBaseX<SERVICETYPE, SERVICE>(myGuid) {}
  virtual FOURCC svc_serviceType() { return SERVICETYPE::getServiceType(); }
  virtual void *svc_getInterface(int global_lock) { // new style, client optionally does the locking
    SERVICETYPE *ret = newService();
    if (global_lock) WASABI_API_SVC->service_lock(this, ret);
    return ret;
  }
  virtual int svc_releaseInterface(void *ptr) {
    return delService(static_cast<SERVICE*>(static_cast<SERVICETYPE*>(ptr)));
  }
protected:
  virtual SERVICETYPE *newService()=0;
  virtual int delService(SERVICETYPE *service)=0;
};

// and this one just exposes a service pointer, without factory. note that
// SERVICETYPE and SERVICE do not have to be related (unlike waServiceFactoryBase
// who needs a relationship between the classes in releaseInterface)
template <class SERVICETYPE, class SERVICE>
class NOVTABLE waServiceBase : public waServiceFactoryBaseX<SERVICETYPE, SERVICE> {
public:
  waServiceBase(GUID myGuid = INVALID_GUID) : waServiceFactoryBaseX<SERVICETYPE, SERVICE>(myGuid) {}
  virtual FOURCC svc_serviceType() { return SERVICE::getServiceType(); }
  virtual void *svc_getInterface(int global_lock) { // new style, client optionally does the locking
    SERVICETYPE *ret = getService();
    if (global_lock) WASABI_API_SVC->service_lock(this, ret);
    return ret;
  }
  virtual int svc_releaseInterface(void *ptr) {
    return TRUE;
  }
protected:
  virtual SERVICETYPE *getService()=0;
};



#endif // __WASERVICEFACTORYBASE_IMPL_H
