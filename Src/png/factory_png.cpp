#include "api__png.h"
#include "factory_png.h"
#include "PNGLoader.h"

FOURCC PNGFactory::GetServiceType()
{
	return PNGLoader::getServiceType(); 
}

const char *PNGFactory::GetServiceName()
{
	return PNGLoader::getServiceName();
}


// {5E04FB28-53F5-4032-BD29-032B87EC3725}
static const GUID pngGUID = 
{ 0x5e04fb28, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x37, 0x25 } };

GUID PNGFactory::GetGUID()
{
	return pngGUID;
}

void *PNGFactory::GetInterface(int global_lock)
{
	svc_imageLoader *ifc=new PNGLoader;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int PNGFactory::SupportNonLockingInterface()
{
	return 1;
}

int PNGFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	svc_imageLoader *png = static_cast<svc_imageLoader *>(ifc);
	PNGLoader *pngloader = static_cast<PNGLoader *>(png);
	delete pngloader;
	return 1;
}

const char *PNGFactory::GetTestString()
{
	return 0;
}

int PNGFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS PNGFactory
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
