#include "api.h"
#include "factory_isocreator.h"
#include "isocreator.h"

static const char serviceName[] = "ISO Creator";

FOURCC ISOCreatorFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *ISOCreatorFactory::GetServiceName()
{
	return serviceName;
}

GUID ISOCreatorFactory::GetGUID()
{
	return obj_isocreatorGUID;
}

void *ISOCreatorFactory::GetInterface(int global_lock)
{
	obj_primo *primo=0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
	if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
	if (!primo)
		return 0;

	ISOCreator *creator = new ISOCreator(primo);
	obj_isocreator *ifc= static_cast<obj_isocreator *>(creator);
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int ISOCreatorFactory::SupportNonLockingInterface()
{
	return 1;
}

int ISOCreatorFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	obj_isocreator *creatorIFC = static_cast<obj_isocreator *>(ifc);
	ISOCreator *creator = static_cast<ISOCreator *>(creatorIFC);
	delete creator;
	return 1;
}

const char *ISOCreatorFactory::GetTestString()
{
	return NULL;
}

int ISOCreatorFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS ISOCreatorFactory
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