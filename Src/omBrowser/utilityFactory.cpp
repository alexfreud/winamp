#include "main.h"
#include "./utilityFactory.h"
#include "./utility.h"

OmUtilityFactory::OmUtilityFactory() 
	: object(NULL)
{
}

OmUtilityFactory::~OmUtilityFactory()
{
	if (NULL != object)
		object->Release();
}

FOURCC OmUtilityFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *OmUtilityFactory::GetServiceName()
{
	return "OmUtility Interface";
}

GUID OmUtilityFactory::GetGUID()
{
	return IFC_OmUtility;
}

void *OmUtilityFactory::GetInterface(int global_lock)
{	
	if (NULL == object)
	{
		object = OmUtility::CreateInstance();
		if (NULL == object) return NULL;
	}

	object->AddRef();
	return object;
}

int OmUtilityFactory::SupportNonLockingInterface()
{
	return 1;
}

int OmUtilityFactory::ReleaseInterface(void *ifc)
{
	OmUtility *object = (OmUtility*)ifc;
	if (NULL != object) object->Release();
	
	return 1;
}

const char *OmUtilityFactory::GetTestString()
{
	return NULL;
}

int OmUtilityFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

HRESULT OmUtilityFactory::Register(api_service *service)
{
	if (NULL == service) return E_INVALIDARG;
	service->service_register(this);
	return S_OK;
}

HRESULT OmUtilityFactory::Unregister(api_service *service)
{
	if (NULL == service) return E_INVALIDARG;
	service->service_deregister(this);
	return S_OK;
}

#define CBCLASS OmUtilityFactory
START_DISPATCH;
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