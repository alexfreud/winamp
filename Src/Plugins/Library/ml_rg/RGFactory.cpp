#include "main.h"
#include "RGFactory.h"
#include "Process.h"
static const char serviceName[] = "Replay Gain Processor";

FOURCC RGFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *RGFactory::GetServiceName()
{
	return serviceName;
}

GUID RGFactory::GetGUID()
{
	return RGGUID;
}

void *RGFactory::GetInterface(int global_lock)
{
	obj_replaygain *ifc=new ProcessReplayGain;
//	if (global_lock)
//		plugin.service->service_lock(this, (void *)ifc);
	return ifc;
}

int RGFactory::SupportNonLockingInterface()
{
	return 1;
}

int RGFactory::ReleaseInterface(void *ifc)
{
	//plugin.service->service_unlock(ifc);
	obj_replaygain *api_ = static_cast<obj_replaygain *>(ifc);
	ProcessReplayGain *svc_ = static_cast<ProcessReplayGain *>(api_);
	delete svc_;
	return 1;
}

const char *RGFactory::GetTestString()
{
	return 0;
}

int RGFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS RGFactory
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
