#pragma once

#include "service/ifc_servicefactory.h"

/*
====== Usage ======
disp_t: your Dispatchable base class
implt_t: your implementation class

SingletonServiceFactory<disp_t, impl_t> myFactory;
impl_t myImplementation;

//....

//during service registration
myFactory.Register(WASABI2_API_SVC, &myImplementation);

//during service deregistration
myFactory.Deregister(WASABI2_API_SVC, &myImplementation);

==== Class requirements ====
your base or implementation class requires the following three static methods
static FOURCC getServiceType(); // return your type (e.g. WaSvc::UNIQUE)... might already be defined in the dispatchable base class
static const char *getServiceName(); // return your service name
static GUID getServiceGuid(); // return your service GUID
*/

class WasabiServiceFactory
{
public:
    virtual ~WasabiServiceFactory() {}
	virtual void Register(api_service *serviceManager)=0;	
	virtual void Deregister(api_service *serviceManager)=0;
};


template <class impl_t, class disp_t>
class SingletonServiceFactory : public ifc_serviceFactory
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
		serviceManager->Register(this);
	}

	void Deregister(api_service *serviceManager)
	{
		if (impl)
		{
			serviceManager->Unregister(this);
			impl=0;
		}
	}

private:
	GUID WASABICALL ServiceFactory_GetServiceType() { return impl_t::GetServiceType(); } 
	nx_string_t WASABICALL ServiceFactory_GetServiceName() { return impl_t::GetServiceName(); }
	GUID WASABICALL ServiceFactory_GetGUID() { return impl_t::GetServiceGUID(); } // GUID per service factory, can be INVALID_GUID
	void *WASABICALL ServiceFactory_GetInterface() { return static_cast<disp_t *>(impl); }
	int WASABICALL ServiceFactory_ServiceNotify(int msg, intptr_t param1 = 0, intptr_t param2 = 0) { return 0; }

private:
	impl_t *impl;

};

/* same as above, but this one also constructs the implementation object itself
   useful when:
	 1) you don't need to ever access the implementation object yourself
	 2) the implementation class requires no constructor parameters
	 
	 TODO: might want to change this to "new" the object during Register and "delete" during Deregister,
	 in case destruction during program termination isn't safe.
*/
template <class impl_t, class disp_t>
class SingletonService : public ifc_serviceFactory, public WasabiServiceFactory
{
public:
	SingletonService()
	{
	}

	~SingletonService() 
	{
	}

	void Register(api_service *serviceManager)
	{
		serviceManager->Register(this);
	}

	void Deregister(api_service *serviceManager)
	{
		serviceManager->Unregister(this);
	}

private:
	GUID WASABICALL ServiceFactory_GetServiceType() { return impl_t::GetServiceType(); } 
	nx_string_t WASABICALL ServiceFactory_GetServiceName() { return impl_t::GetServiceName(); }
	GUID WASABICALL ServiceFactory_GetGUID() { return impl_t::GetServiceGUID(); } // GUID per service factory, can be INVALID_GUID
	void *WASABICALL ServiceFactory_GetInterface() { return static_cast<disp_t *>(&impl); }
	int WASABICALL ServiceFactory_ServiceNotify(int msg, intptr_t param1 = 0, intptr_t param2 = 0) { return 0; }

private:
	impl_t impl;

};

