//#define GUID_EQUALS_DEFINED
#include "api__vlb.h"
#include "factory_vlbdecoder.h"
#include "VlbDecoder.h"

static const char serviceName[] = "Dolby VLB Decoder";

FOURCC VLBDecoderFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *VLBDecoderFactory::GetServiceName()
{
	return serviceName;
}

GUID VLBDecoderFactory::GetGUID()
{
	return obj_vlbDecoderGUID;
}

void *VLBDecoderFactory::GetInterface(int global_lock)
{
	obj_vlbDecoder *ifc=new VLBDecoder;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int VLBDecoderFactory::SupportNonLockingInterface()
{
	return 1;
}

int VLBDecoderFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	obj_vlbDecoder *decoder = static_cast<obj_vlbDecoder *>(ifc);
	VLBDecoder *vlbDecoder = static_cast<VLBDecoder *>(decoder);
	delete vlbDecoder;
	return 1;
}

const char *VLBDecoderFactory::GetTestString()
{
	return NULL;
}

int VLBDecoderFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS VLBDecoderFactory
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