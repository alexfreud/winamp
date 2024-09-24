#include "api__png.h"
#include "factory_pngwrite.h"
#include "PNGWriter.h"

FOURCC PNGWriteFactory::GetServiceType()
{
	return PNGWriter::getServiceType(); 
}

const char *PNGWriteFactory::GetServiceName()
{
	return PNGWriter::getServiceName();
}

// {D089F671-283E-4999-A5C8-FE3DD851F3F1}
static const GUID pngWriteGUID = 
{ 0xd089f671, 0x283e, 0x4999, { 0xa5, 0xc8, 0xfe, 0x3d, 0xd8, 0x51, 0xf3, 0xf1 } };


GUID PNGWriteFactory::GetGUID()
{
	return pngWriteGUID;
}

void *PNGWriteFactory::GetInterface(int global_lock)
{
	svc_imageWriter *ifc=new PNGWriter;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int PNGWriteFactory::SupportNonLockingInterface()
{
	return 1;
}

int PNGWriteFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	svc_imageWriter *png = static_cast<svc_imageWriter *>(ifc);
	PNGWriter *pngWriter = static_cast<PNGWriter *>(png);
	delete pngWriter;
	return 1;
}

const char *PNGWriteFactory::GetTestString()
{
	return 0;
}

int PNGWriteFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS PNGWriteFactory
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
