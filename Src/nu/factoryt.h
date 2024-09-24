#pragma once

#include <api/service/waservicefactoryi.h>
#include <api/service/services.h>

/*
====== Usage ======
disp_t: your Dispatchable base class
implt_t: your implementation class

ServiceFactoryT<disp_t, impl_t> myFactory;
impl_t myImplementation;

//....

//during service registration
myFactory.Register(WASABI_API_SVC);

//during service deregistration
myFactory.Deregister(WASABI_API_SVC);

==== Class requirements ====
your base or implementation class requires the following three static methods
static FOURCC getServiceType(); // return your type (e.g. WaSvc::UNIQUE)... might already be defined in the dispatchable base class
static const char *getServiceName(); // return your service name
static GUID getServiceGuid(); // return your service GUID
must implementation a constructor that requires no parameters
*/

template <class disp_t, class impl_t>
class ServiceFactoryT : public waServiceFactory
{
public:
	ServiceFactoryT()
	{
	}

	~ServiceFactoryT()
	{
	}

	void Register(api_service *serviceManager)
	{
		serviceManager->service_register(this);
	}

	void Deregister(api_service *serviceManager)
	{
		serviceManager->service_deregister(this);
	}

private:
	FOURCC svc_serviceType() { return impl_t::getServiceType(); } 
	const char *svc_getServiceName() { return impl_t::getServiceName(); }
	GUID svc_getGuid() { return impl_t::getServiceGuid(); } // GUID per service factory, can be INVALID_GUID
	void *svc_getInterface(int global_lock = TRUE) { return static_cast<disp_t *>(new impl_t); }
	int svc_supportNonLockingGetInterface() { return 1; }
	int svc_releaseInterface(void *ifc) { disp_t *disp = static_cast<disp_t *>(ifc);	impl_t *impl = static_cast<impl_t *>(disp); delete impl; return 1; }
	const wchar_t *svc_getTestString() { return 0; }
	int svc_notify(int msg, int param1 = 0, int param2 = 0) { return 0; }


protected:
#define CBCLASS ServiceFactoryT<disp_t, impl_t>
	START_DISPATCH_INLINE;
	CB(WASERVICEFACTORY_GETSERVICETYPE, svc_serviceType);
	CB(WASERVICEFACTORY_GETSERVICENAME, svc_getServiceName);
	CB(WASERVICEFACTORY_GETGUID, svc_getGuid);
	CB(WASERVICEFACTORY_GETINTERFACE, svc_getInterface);
	CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, svc_supportNonLockingGetInterface);
	CB(WASERVICEFACTORY_RELEASEINTERFACE, svc_releaseInterface);
	CB(WASERVICEFACTORY_GETTESTSTRING, svc_getTestString);
	CB(WASERVICEFACTORY_SERVICENOTIFY, svc_notify);
	END_DISPATCH;
#undef CBCLASS
};

