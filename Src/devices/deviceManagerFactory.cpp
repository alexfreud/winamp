#include "main.h"
#include "./deviceManager.h"
#include "./deviceManagerFactory.h"

DeviceManagerFactory::DeviceManagerFactory() 
	: object(NULL)
{
}

DeviceManagerFactory::~DeviceManagerFactory()
{
	if (NULL != object)
		object->Release();
}

FOURCC DeviceManagerFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *DeviceManagerFactory::GetServiceName()
{
	return "Device Manager Interface";
}

GUID DeviceManagerFactory::GetGUID()
{
	return DeviceManagerGUID;
}

void *DeviceManagerFactory::GetInterface(int global_lock)
{	
	if (NULL == object)
	{
		if (FAILED(DeviceManager::CreateInstance(&object)))
			object = NULL;
		
		if (NULL == object) 
			return NULL;
	}

	object->AddRef();
	return object;
}

int DeviceManagerFactory::SupportNonLockingInterface()
{
	return 1;
}

int DeviceManagerFactory::ReleaseInterface(void *ifc)
{
	DeviceManager *object = (DeviceManager*)ifc;
	if (NULL != object) 
		object->Release();
	
	return 1;
}

const char *DeviceManagerFactory::GetTestString()
{
	return NULL;
}

int DeviceManagerFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

HRESULT DeviceManagerFactory::Register(api_service *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	service->service_register(this);
	return S_OK;
}

HRESULT DeviceManagerFactory::Unregister(api_service *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	service->service_deregister(this);
	return S_OK;
}

#define CBCLASS DeviceManagerFactory
START_DISPATCH;
CB( WASERVICEFACTORY_GETSERVICETYPE,                GetServiceType )
CB( WASERVICEFACTORY_GETSERVICENAME,                GetServiceName )
CB( WASERVICEFACTORY_GETGUID,                       GetGUID )
CB( WASERVICEFACTORY_GETINTERFACE,                  GetInterface )
CB( WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface )
CB( WASERVICEFACTORY_RELEASEINTERFACE,              ReleaseInterface )
CB( WASERVICEFACTORY_GETTESTSTRING,                 GetTestString )
CB( WASERVICEFACTORY_SERVICENOTIFY,                 ServiceNotify )
END_DISPATCH;

#undef CBCLASS