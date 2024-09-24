#ifndef _MYFACTORY_H_
#define _MYFACTORY_H_

#include "api__bmp.h"
#include <api/service/waservicefactory.h>
#include <api/service/services.h>

template <class T, class Base>
class MyFactory : public waServiceFactory
{
public:
	MyFactory(GUID guid) : guid(guid) {}
	FOURCC GetServiceType() { return T::getServiceType(); }
	const char *GetServiceName() { return T::getServiceName(); }
	GUID GetGUID() { return guid; }
	void *GetInterface(int global_lock) { return (Base*)new T; }
	int SupportNonLockingInterface() {return 1;}
	int ReleaseInterface(void *ifc) { delete static_cast<T *>(static_cast<Base *>(ifc)); return 1; }
	const char *GetTestString() {return 0;}
	int ServiceNotify(int msg, int param1, int param2) {return 1;}
private:
	GUID guid;
protected:
	#define CBCLASS MyFactory
	START_DISPATCH_INLINE;
		CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
		CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
		CB(WASERVICEFACTORY_GETGUID, GetGUID)
		CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
		CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
		CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
		CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
		CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
	END_DISPATCH;
	#undef CBCLASS
	//RECVS_DISPATCH;
};

#endif