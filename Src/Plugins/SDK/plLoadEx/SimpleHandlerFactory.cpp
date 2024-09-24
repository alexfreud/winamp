#include "SimpleHandlerFactory.h"
#include "SimpleHandler.h"
/*
 This is the GUID for our service factory
 don't re-use this.  
 make your own guid with guidgen.exe
 lives somewhere like C:\Program Files\Microsoft Visual Studio\2019\Professional\Common7\Tools
*/

// {1CCF6445-A452-45e8-BE72-846991CBCAF6}
static const GUID SimpleHandlerGUID = 
{ 0x1ccf6445, 0xa452, 0x45e8, { 0xbe, 0x72, 0x84, 0x69, 0x91, 0xcb, 0xca, 0xf6 } };


// our playlist handler.
static Cef_Handler simpleHandler;

FOURCC SimpleHandlerFactory::GetServiceType()
{
	return svc_playlisthandler::getServiceType();
}

const char *SimpleHandlerFactory::GetServiceName()
{
	return "Simple Playlist Loader";
}

GUID SimpleHandlerFactory::GetGuid()
{
	return SimpleHandlerGUID;
}

void *SimpleHandlerFactory::GetInterface(int global_lock)
{
// simpleHandler is a singleton object, so we can just return a pointer to it
	// depending on what kind of service you are making, you might have to 
	// 'new' an object and return that instead (and then free it in ReleaseInterface)
	return &simpleHandler;
}

int SimpleHandlerFactory::ReleaseInterface(void *ifc)
{
	// no-op because we returned a singleton (see above)
	return 1;
}

// Define the dispatch table
#define CBCLASS SimpleHandlerFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName) 
CB(WASERVICEFACTORY_GETGUID, GetGuid)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
END_DISPATCH;
#undef CBCLASS