#ifndef _SERVICEI_H
#define _SERVICEI_H

// here is where we define some helper implementations of the service factory
// interface

#include "service.h"

#include "waservicefactoryi.h"
#include "waservicefactorybase.h"
#include "waservicefactoryt.h"
#include "waservicefactorytsingle.h"
#include <bfc/ptrlist.h>
#include <bfc/foreach.h>

// DEPRECATED
#define waService waServiceFactory
#define waServiceT waServiceFactoryT
#define waServiceTSingle waServiceFactoryTSingle

// Declaring services via macros
// ie:
// BEGIN_SERVICES(YourClass_Svc)
// DECLARE_SERVICE(SomeSvcCreator<YourServiceClass1>);
// DECLARE_SERVICE(SomeSvcCreator<YourServiceClass2>);
// DECLARE_SERVICE(SomeSvcCreator<YourServiceClass3>);
// DECLARE_SERVICETMULTI(svc_serviceclass, YourMultiInstanceServiceClass1>);
// DECLARE_SERVICETMULTI(svc_serviceclass, YourMultiInstanceServiceClass2>);
// DECLARE_SERVICETMULTI(svc_serviceclass, YourMultiInstanceServiceClass3>);
// DECLARE_SERVICETSINGLE(svc_serviceclass, YourSingleInstanceServiceClass1>);
// DECLARE_SERVICETSINGLE(svc_serviceclass, YourSingleInstanceServiceClass2>);
// DECLARE_SERVICETSINGLE(svc_serviceclass, YourSingleInstanceServiceClass3>);
// END_SERVICES(YourClass_Svc, _YourClass_Svc);

// The macro DECLARE_MODULE_SVCMGR should be defined in your main cpp file if you are compiling
// a standalone exe without using the Application class. In this case you should also add
// MODULE_SVCMGR_ONINIT(); in your app startup and MODULE_SVCMGR_ONSHUTDOWN() in your app shutdown
// (class Application does this already)

// You can use BEGIN_SYS_SERVICES to declare a service that is initialized in priority and 
// shutdown late (normal version inits late, shutdowns early)


class StaticServices {
  public:
    virtual void registerServices()=0;
    virtual void unregisterServices()=0;
};

class AutoServiceList : public PtrList<StaticServices> {
 public:
  AutoServiceList() 
  {
#ifdef _WIN32 // PORT ME
    OutputDebugStringA(">\n");
#endif
  }
};

class StaticServiceMgr {
  public:
    AutoServiceList __m_modules;
    AutoServiceList __m_modules2;
};

#define DECLARE_MODULE_SVCMGR StaticServiceMgr *staticServiceMgr = NULL;

extern StaticServiceMgr *staticServiceMgr;

#define MODULE_SVCMGR_ONINIT() { \
foreach(staticServiceMgr->__m_modules) \
  staticServiceMgr->__m_modules.getfor()->registerServices(); \
endfor; \
}

#define MODULE_SVCMGR_ONINIT2() { \
foreach(staticServiceMgr->__m_modules2) \
  staticServiceMgr->__m_modules2.getfor()->registerServices(); \
endfor; \
}

#define MODULE_SVCMGR_ONSHUTDOWN() { \
foreach(staticServiceMgr->__m_modules) \
  staticServiceMgr->__m_modules.getfor()->unregisterServices(); \
endfor; \
}
 
#define MODULE_SVCMGR_ONSHUTDOWN2() { \
foreach(staticServiceMgr->__m_modules2) \
  staticServiceMgr->__m_modules2.getfor()->unregisterServices(); \
endfor; \
}
 
#define INIT_MODULE_SVCMGR() \
  if (!staticServiceMgr) staticServiceMgr = new StaticServiceMgr; 

#define BEGIN_SERVICES(CLASSNAME) \
class CLASSNAME : public StaticServices { \
public: \
  CLASSNAME() { \
    INIT_MODULE_SVCMGR() \
    staticServiceMgr->__m_modules.addItem(this); \
  } \
  virtual ~CLASSNAME() {} \
  virtual void registerServices() { \
  ASSERT(staticServiceMgr);

#define BEGIN_SERVICES_DEBUG(CLASSNAME, TEXT) \
class CLASSNAME : public StaticServices { \
public: \
  CLASSNAME() { \
    INIT_MODULE_SVCMGR() \
    OutputDebugString("Registering "); \
    OutputDebugString(TEXT); \
    OutputDebugString("\n"); \
    staticServiceMgr->__m_modules.addItem(this); \
  } \
  virtual ~CLASSNAME() {} \
  virtual void registerServices() { \
  ASSERT(staticServiceMgr);

#define BEGIN_SYS_SERVICES(CLASSNAME) \
class CLASSNAME : public StaticServices { \
public: \
  CLASSNAME() { \
    INIT_MODULE_SVCMGR() \
    staticServiceMgr->__m_modules2.addItem(this); \
  } \
  virtual ~CLASSNAME() {} \
  virtual void registerServices() {

#define DECLARE_SERVICE(INSTANTIATIONCODE) \
    registerService(new INSTANTIATIONCODE); 

#define DECLARE_SERVICE_DEBUG(INSTANTIATIONCODE, TEXT) \
    OutputDebugString("Starting "); \
    OutputDebugString(TEXT); \
    OutputDebugString("\n"); \
    registerService(new INSTANTIATIONCODE); 
  
#define DECLARE_SERVICETMULTI(SVCTYPE, SVCCLASS) \
    registerService(new waServiceFactoryT<SVCTYPE, SVCCLASS>); 

#define DECLARE_SERVICETMULTI_DEBUG(SVCTYPE, SVCCLASS, TEXT) \
    OutputDebugString("Starting "); \
    OutputDebugString(TEXT); \
    OutputDebugString("\n"); \
    registerService(new waServiceFactoryT<SVCTYPE, SVCCLASS>); 

#define DECLARE_SERVICETSINGLE(SVCTYPE, SVCCLASS) \
    registerService(new waServiceFactoryTSingle<SVCTYPE, SVCCLASS>); 

#define DECLARE_SERVICETSINGLE_DEBUG(SVCTYPE, SVCCLASS, TEXT) \
    OutputDebugString("Starting "); \
    OutputDebugString(TEXT); \
    OutputDebugString("\n"); \
    registerService(new waServiceFactoryTSingle<SVCTYPE, SVCCLASS>, TEXT); 
  
#define END_SERVICES(CLASSNAME, INSTANCENAME) \
  } \
  virtual void unregisterServices() { \
    foreach(services) \
      waServiceFactoryI *svc = services.getfor(); \
      WASABI_API_SVC->service_deregister(svc); \
      delete svc; \
    endfor; \
    services.removeAll();\
  } \
private: \
  void registerService(waServiceFactoryI *svc) { \
    if (svc != NULL) { \
      services.addItem(svc); \
      WASABI_API_SVC->service_register(svc); \
    } \
  } \
  PtrList<waServiceFactoryI> services; \
}; \
CLASSNAME INSTANCENAME;

#endif
