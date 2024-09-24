#pragma once

#include "service/ifc_servicefactory.h"
#include "ReferenceCounted.h"
/*
====== Usage ======
disp_t: your Dispatchable base class
implt_t: your implementation class

ObjectFactory<disp_t, impl_t> myFactory;
impl_t myImplementation;

//....

//during service registration
myFactory.Register(WASABI2_API_SVC);

//during service deregistration
myFactory.Deregister(WASABI2_API_SVC);

==== Class requirements ====
your base or implementation class requires the following three static methods
static FOURCC getServiceType(); // return your type (e.g. WaSvc::OBJECT)... might already be defined in the dispatchable base class
static const char *getServiceName(); // return your service name
static GUID getServiceGuid(); // return your service GUID
*/

template <class impl_t, class disp_t>
class CountableObjectFactory : public ifc_serviceFactory
{
public:
	CountableObjectFactory()
	{
	}

	~CountableObjectFactory() 
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
	void *WASABICALL ServiceFactory_GetInterface() { return static_cast<disp_t *>(new ReferenceCounted<impl_t>); }
	int WASABICALL ServiceFactory_ServiceNotify(int msg, intptr_t param1 = 0, intptr_t param2 = 0) { return 0; }
};

