#include "../Agave/Component/ifc_wa5component.h"
#include "api__xspf.h" // services we'll used are extern'd here
#include "XSPFHandlerFactory.h"

// declarations for the pointers to services we'll need
// by convention, these follow the format WASABI_API_* or AGAVE_API_*
// you are free to name it however you want, but this provides good consistency
api_service *WASABI_API_SVC = 0; // the wasabi service manager (our gateway to other services)
api_mldb *AGAVE_API_MLDB = 0; // media library API.  we'll retrieve this the first time we need it, because it will load after us
api_tagz *AGAVE_API_TAGZ = 0;
api_application *WASABI_API_APP = 0;
// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

// our service factories that we're going to register
static XSPFHandlerFactory xspfHandlerFactory;

// here is the implementation of our component
// this class contains the methods called during initialization and shutdown
// to register/deregister our component's services
class XSPFComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service); // this is where the party is
	void DeregisterServices(api_service *service);

protected:
	RECVS_DISPATCH; // some boiler-plate code to implement our dispatch table
};

static HINSTANCE GetMyInstance()
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
		return (HINSTANCE)mbi.AllocationBase;
	return NULL;
}

void XSPFComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service; // we get passed the service manager and we need to save the pointer
	WASABI_API_SVC->service_register(&xspfHandlerFactory);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(GetMyInstance(),playlistLangGUID);
}

void XSPFComponent::DeregisterServices(api_service *service)
{
	WASABI_API_SVC->service_deregister(&xspfHandlerFactory);
}

static XSPFComponent xspfComponent;
// Winamp calls this function after it LoadLibrary's your W5S
// you need to turn a pointer to your component.
extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &xspfComponent;
}

// some boiler-plate code to implement the dispatch table for our component
#define CBCLASS XSPFComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS