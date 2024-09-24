#include "api.h"
#include "factory_databurner.h"
#include "DataBurner.h"

static const char serviceName[] = "Data Burner";

FOURCC DataBurnerFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *DataBurnerFactory::GetServiceName()
{
	return serviceName;
}

GUID DataBurnerFactory::GetGUID()
{
	return obj_databurnerGUID;
}

void *DataBurnerFactory::GetInterface(int global_lock)
{
	obj_primo *primo=0;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
	if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
	if (!primo)
		return 0;
	DataBurner *creator = new DataBurner(primo);
	obj_databurner *ifc= static_cast<obj_databurner *>(creator);
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int DataBurnerFactory::SupportNonLockingInterface()
{
	return 1;
}

int DataBurnerFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	obj_databurner *creatorIFC = static_cast<obj_databurner *>(ifc);
	DataBurner *creator = static_cast<DataBurner *>(creatorIFC);
	delete creator;
	return 1;
}

const char *DataBurnerFactory::GetTestString()
{
	return NULL;
}

int DataBurnerFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS DataBurnerFactory
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