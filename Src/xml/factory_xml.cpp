#include "api__xml.h"
#include "factory_xml.h"
#include "XMLReader.h"

static const char serviceName[] = "XML Parser";

FOURCC XMLFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *XMLFactory::GetServiceName()
{
	return serviceName;
}

GUID XMLFactory::GetGUID()
{
	return obj_xmlGUID;
}

void *XMLFactory::GetInterface(int global_lock)
{
	obj_xml *ifc=new XMLReader;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int XMLFactory::SupportNonLockingInterface()
{
	return 1;
}

int XMLFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	obj_xml *xml = static_cast<obj_xml *>(ifc);
	XMLReader *xmlreader = static_cast<XMLReader *>(xml);
	delete xmlreader;
	return 1;
}

const char *XMLFactory::GetTestString()
{
	return 0;
}

int XMLFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS XMLFactory
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
