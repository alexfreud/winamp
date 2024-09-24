#include "api.h"
#include "factory_isoburner.h"
#include "isoburner.h"

static const char serviceName[] = "ISO Burner";

FOURCC ISOBurnerFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *ISOBurnerFactory::GetServiceName()
{
	return serviceName;
}

GUID ISOBurnerFactory::GetGUID()
{
	return obj_isoburnerGUID;
}

void *ISOBurnerFactory::GetInterface(int global_lock)
{
	obj_primo *primo=0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
	if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
	if (!primo)
		return 0;
	ISOBurner *creator = new ISOBurner(primo);
	obj_isoburner *ifc= static_cast<obj_isoburner *>(creator);
	//	if (global_lock)
	//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int ISOBurnerFactory::SupportNonLockingInterface()
{
	return 1;
}

int ISOBurnerFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	obj_isoburner *creatorIFC = static_cast<obj_isoburner *>(ifc);
	ISOBurner *creator = static_cast<ISOBurner *>(creatorIFC);
	delete creator;
	return 1;
}

const char *ISOBurnerFactory::GetTestString()
{
	return NULL;
}

int ISOBurnerFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS ISOBurnerFactory
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