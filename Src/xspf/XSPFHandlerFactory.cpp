#include "XSPFHandlerFactory.h"
#include "XSPFHandler.h"
/*
 This is the GUID for our service factory
 don't re-use this.  
 make your own guid with guidgen.exe
 lives somewhere like C:\program files\Microsoft Visual Studio .NET 2003\Common7\Tools\Bin
*/

// {51D17273-566F-4fa9-AFE0-1345C65B8B1B}
static const GUID XSPFHandlerGUID = 
{ 0x51d17273, 0x566f, 0x4fa9, { 0xaf, 0xe0, 0x13, 0x45, 0xc6, 0x5b, 0x8b, 0x1b } };


// our playlist handler.
static XSPFHandler xspfHandler;

FOURCC XSPFHandlerFactory::GetServiceType()
{
	return svc_playlisthandler::getServiceType();
}

const char *XSPFHandlerFactory::GetServiceName()
{
	return "XSPF Playlist Loader";
}

GUID XSPFHandlerFactory::GetGuid()
{
	return XSPFHandlerGUID;
}

void *XSPFHandlerFactory::GetInterface(int global_lock)
{
	// xspfHandler is a singleton object, so we can just return a pointer to it
	// depending on what kind of service you are making, you might have to 
	// 'new' an object and return that instead (and then free it in ReleaseInterface)
	return &xspfHandler;
}

int XSPFHandlerFactory::ReleaseInterface(void *ifc)
{
	// no-op because we returned a singleton (see above)
	return 1;
}

// Define the dispatch table
#define CBCLASS XSPFHandlerFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName) 
CB(WASERVICEFACTORY_GETGUID, GetGuid)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
END_DISPATCH;
#undef CBCLASS