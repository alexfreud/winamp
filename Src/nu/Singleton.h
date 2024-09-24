#pragma once
#ifndef NULLSOFT_UTILITY_SINGLETON_H
#define NULLSOFT_UTILITY_SINGLETON_H

#include <api/service/waservicefactoryi.h>
#include <api/service/services.h>

/*
====== Usage ======
disp_t: your Dispatchable base class
implt_t: your implementation class

SingletonServiceFactory<disp_t, impl_t> myFactory;
impl_t myImplementation;

//....

//during service registration
myFactory.Register(WASABI_API_SVC, &myImplementation);

//during service deregistration
myFactory.Deregister(WASABI_API_SVC, &myImplementation);

==== Class requirements ====
your base or implementation class requires the following three static methods
static FOURCC getServiceType(); // return your type (e.g. WaSvc::UNIQUE)... might already be defined in the dispatchable base class
static const char *getServiceName(); // return your service name
static GUID getServiceGuid(); // return your service GUID
*/

template <class disp_t, class impl_t>
class SingletonServiceFactory : public waServiceFactory
{
public:
	SingletonServiceFactory() : impl(0)
	{
	}

	~SingletonServiceFactory()
	{
	}

	void Register(api_service *serviceManager, impl_t *new_impl)
	{
		impl=new_impl;
		serviceManager->service_register(this);
	}

	void Deregister(api_service *serviceManager)
	{
		if (impl)
		{
			serviceManager->service_deregister(this);
			impl=0;
		}
	}

private:
	FOURCC svc_serviceType() { return impl_t::getServiceType(); } 
	const char *svc_getServiceName() { return impl_t::getServiceName(); }
	GUID svc_getGuid() { return impl_t::getServiceGuid(); } // GUID per service factory, can be INVALID_GUID
	void *svc_getInterface(int global_lock = TRUE) { return static_cast<disp_t *>(impl); }
	int svc_supportNonLockingGetInterface() { return 1; }
	int svc_releaseInterface(void *ifc) 	{		return 0; 	}
	const wchar_t *svc_getTestString() { return 0; }
	int svc_notify(int msg, int param1 = 0, int param2 = 0) { return 0; }

private:
	impl_t *impl;

protected:
#define CBCLASS SingletonServiceFactory<disp_t, impl_t>
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

#endif