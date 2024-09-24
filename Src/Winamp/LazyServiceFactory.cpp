#include "LazyServiceFactory.h"
#include "w5s.h"
#include "api.h"
#include <strsafe.h>
/*
various implementation notes:
1) register for service notifications after registering service, so we can know if our service got loaded indirectly
*/

LazyServiceFactory::LazyServiceFactory(FOURCC _service_type, GUID _service_guid, char *_service_name, char *_service_test_string, const wchar_t *_service_filename)
{
	service_type=_service_type;
	service_guid=_service_guid;
	service_name=_service_name;
	service_test_string = _service_test_string;
	StringCbCopyW(service_filename, sizeof(service_filename), _service_filename);
}

LazyServiceFactory::~LazyServiceFactory()
{
	WASABI_API_SVC->service_deregister(this);
	free(service_name);
	free(service_test_string);
}

FOURCC LazyServiceFactory::GetServiceType()
{
	return service_type;
}

const char *LazyServiceFactory::GetServiceName()
{
	return service_name;
}

GUID LazyServiceFactory::GetGUID()
{
	return service_guid;
}

void *LazyServiceFactory::GetInterface(int global_lock)
{
	//load target W5S
	w5s_load(service_filename);
	// ask the service manager remove our service factory and put the new one in our old place (to keep enumeration valid)
	WASABI_API_SVC->service_compactDuplicates(this);

	// call the service manager to get the "real" service which should now (hopefully) be loaded
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(service_guid);
	if (sf && sf != this)
		return sf->getInterface();
	return 0;
}

int LazyServiceFactory::SupportNonLockingInterface()
{
	return 1;
}

int LazyServiceFactory::ReleaseInterface(void *ifc)
{
	// someone may have held on to our service factory when they loaded the service
	// so they call us instead of the real service factory
	// so go grab the 'real' service and release through that
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(service_guid);
	if (sf && sf != this) // make sure we didn't just grab ourselves :)
	{
		return sf->releaseInterface(ifc);
	}
	return 0;
}

const char *LazyServiceFactory::GetTestString()
{
	return service_test_string;
}

int LazyServiceFactory::ServiceNotify(int msg, intptr_t param1, intptr_t param2)
{
	switch (msg)
	{
	case SvcNotify::ONDEREGISTERED:
		WASABI_API_SYSCB->syscb_deregisterCallback(this);
		break;
	case SvcNotify::ONREGISTERED:
		WASABI_API_SYSCB->syscb_registerCallback(this);
		break;
	}
	return 1;
}

int LazyServiceFactory::Notify(int msg, intptr_t param1, intptr_t param2)
{
	switch (msg)
	{
	case SvcCallback::ONREGISTER:
	{
		waServiceFactory *sf = reinterpret_cast<waServiceFactory*>(param2);
		GUID serviceGUID = sf->getGuid();
		if (sf != this && serviceGUID != INVALID_GUID && serviceGUID == service_guid)
		{
			// real service got loaded, so unregister ourselves
			WASABI_API_SVC->service_compactDuplicates(this);
		}
	}
	break;
	default: return 0;
	}
	return 1;
}

#define CBCLASS LazyServiceFactory
START_MULTIPATCH;
START_PATCH(ServiceFactoryPatch)
M_CB(ServiceFactoryPatch, waServiceFactory, WASERVICEFACTORY_GETSERVICETYPE, GetServiceType);
M_CB(ServiceFactoryPatch, waServiceFactory, WASERVICEFACTORY_GETSERVICENAME, GetServiceName);
M_CB(ServiceFactoryPatch, waServiceFactory, WASERVICEFACTORY_GETGUID, GetGUID);
M_CB(ServiceFactoryPatch, waServiceFactory, WASERVICEFACTORY_GETINTERFACE, GetInterface);
M_CB(ServiceFactoryPatch, waServiceFactory, WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) ;
M_CB(ServiceFactoryPatch, waServiceFactory, WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface);
M_CB(ServiceFactoryPatch, waServiceFactory, WASERVICEFACTORY_GETTESTSTRING, GetTestString);
M_CB(ServiceFactoryPatch, waServiceFactory, WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify);
NEXT_PATCH(SysCallbackPatch)
M_CB(SysCallbackPatch,  SysCallback,  SYSCALLBACK_GETEVENTTYPE, GetEventType);
M_CB(SysCallbackPatch,  SysCallback,  SYSCALLBACK_NOTIFY, Notify);
END_PATCH
END_MULTIPATCH;
#undef CBCLASS