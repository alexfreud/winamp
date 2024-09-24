#include "./serviceFactory.h"
#include "./serviceManager.h"

OmServiceFactory::OmServiceFactory()
	: object(NULL)
{
}

OmServiceFactory::~OmServiceFactory()
{
	if (NULL != object)
		object->Release();
}

FOURCC OmServiceFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *OmServiceFactory::GetServiceName()
{
	return "OmServiceManager Interface";
}

GUID OmServiceFactory::GetGUID()
{
	return IFC_OmServiceManager;
}

void *OmServiceFactory::GetInterface(int global_lock)
{	
	if (NULL == object)
	{
		object = OmServiceManager::CreateInstance();
		if (NULL == object) return NULL;
	}

	object->AddRef();
	return object;
}

int OmServiceFactory::SupportNonLockingInterface()
{
	return 1;
}

int OmServiceFactory::ReleaseInterface(void *ifc)
{
	OmServiceManager *object = (OmServiceManager*)ifc;
	if (NULL != object) object->Release();

	return 1;
}

const char *OmServiceFactory::GetTestString()
{
	return NULL;
}

int OmServiceFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

HRESULT OmServiceFactory::Register(api_service *service)
{
	if (NULL == service) return E_INVALIDARG;
	service->service_register(this);
	return S_OK;
}

HRESULT OmServiceFactory::Unregister(api_service *service)
{
	if (NULL == service) return E_INVALIDARG;
	service->service_deregister(this);
	return S_OK;
}

#define CBCLASS OmServiceFactory
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