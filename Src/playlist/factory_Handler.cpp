#include "api__playlist.h"
#include "factory_Handler.h"
#include "handler.h"
#include "api/service/services.h"

M3UHandler m3uHandler;
PLSHandler plsHandler;
B4SHandler b4sHandler;

static const char m3uServiceName[] = "M3U Playlist Handler";
static const char plsServiceName[] = "PLS Playlist Handler";

#define DEFINE_HANDLER_FACTORY(CLASSNAME, className) const char *CLASSNAME ## HandlerFactory::GetServiceName() { 	return #CLASSNAME ## "Playlist Handler"; }\
	GUID CLASSNAME ## HandlerFactory::GetGUID(){	return className ## HandlerGUID;}\
	void *CLASSNAME ## HandlerFactory::GetInterface(int global_lock){	return &className ## Handler;}

DEFINE_HANDLER_FACTORY(M3U, m3u);
DEFINE_HANDLER_FACTORY(PLS, pls);
DEFINE_HANDLER_FACTORY(B4S, b4s);

/* --------------------------------------------------------------------- */
FOURCC CommonHandlerFactory::GetServiceType()
{
	return WaSvc::PLAYLISTHANDLER; 
}

int CommonHandlerFactory::SupportNonLockingInterface()
{
	return 1;
}

int CommonHandlerFactory::ReleaseInterface(void *ifc)
{
	return 1;
}

const char *CommonHandlerFactory::GetTestString()
{
	return NULL;
}

int CommonHandlerFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS M3UHandlerFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
#undef CBCLASS
#define CBCLASS CommonHandlerFactory
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;

#undef CBCLASS
#define CBCLASS PLSHandlerFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
#undef CBCLASS
#define CBCLASS CommonHandlerFactory
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;



#undef CBCLASS
#define CBCLASS B4SHandlerFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
#undef CBCLASS
#define CBCLASS CommonHandlerFactory
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;


