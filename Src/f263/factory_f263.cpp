#include "api.h"
#include "factory_f263.h"
#include "impl_f263decoder.h"

static const char serviceName[] = "F263 Decoder";

FOURCC F263Factory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *F263Factory::GetServiceName()
{
	return serviceName;
}

GUID F263Factory::GetGUID()
{
	return obj_f263decoderGUID;
}

void *F263Factory::GetInterface(int global_lock)
{
	F263Decoder *f263decoder = new F263Decoder;
	obj_f263decoder *ifc= static_cast<obj_f263decoder *>(f263decoder);
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int F263Factory::SupportNonLockingInterface()
{
	return 1;
}

int F263Factory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	obj_f263decoder *decoderIFC = static_cast<obj_f263decoder *>(ifc);
	F263Decoder *decoder = static_cast<F263Decoder *>(decoderIFC);
	delete decoder;
	return 1;
}

const char *F263Factory::GetTestString()
{
	return NULL;
}

int F263Factory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS F263Factory
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