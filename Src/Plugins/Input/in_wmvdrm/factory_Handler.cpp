#include "main.h"
#include "api.h"
#include "factory_Handler.h"
#include "PlaylistHandler.h"
#include <api/service/services.h>

WPLHandler wplHandler;
ASXHandler asxHandler;


#define DEFINE_HANDLER_FACTORY(CLASSNAME, className) const char *CLASSNAME ## HandlerFactory::GetServiceName() { 	return #CLASSNAME ## "Playlist Handler"; }\
	GUID CLASSNAME ## HandlerFactory::GetGUID(){	return className ## HandlerGUID;}\
	void *CLASSNAME ## HandlerFactory::GetInterface(int global_lock){	return &className ## Handler;}


DEFINE_HANDLER_FACTORY(WPL, wpl);
DEFINE_HANDLER_FACTORY(ASX, asx);

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




#undef CBCLASS
#define CBCLASS WPLHandlerFactory
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
#define CBCLASS ASXHandlerFactory
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




