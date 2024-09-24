#pragma once
#include "foundation/dispatch.h"
#include "service/api_service.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h> /* for CFBundleRef */
#endif

#include "nx/nxapi.h"
#include "nx/nxuri.h"


/* see http://wiki.office.aol.com/wiki/Replicant/Component */

/* in very rare cases, this will get incremented */
const int wasabi2_component_version = 0;

struct WasabiComponentInfo
{
	/* these will be filled in by the host application loading your component */
#if defined(_WIN32)
	HMODULE hModule;
#elif defined(__APPLE__)
	/* depending on whether your component is a bundle or a dylib, one of these will be filled */
	CFBundleRef bundle; 
	void *dl_handle; // pointer returned from dlopen
#else
  void *dl_handle; // pointer returned from dlopen
#endif
	nx_uri_t filename;

	/* these are filled in during ifc_component's constructor.  You don't need to change these */
	int wasabi_version; /* major breaking changes to Wasabi (ABI changes, Dispatchable definition, etc) will change this.  This is used to filter out components that are expecting something completely different than what the host API provides */
	int nx_api_version; /* major breaking changes to the NX API will change this. */
	GUID nx_platform_guid; /* platform that this is meant to link against.  right now this mainly helps avoid loading android components on 'vanilla' linux, and vice-versa */

	/* these are defaulted to INVALID_GUID.  You can optionally override these in your Component's constructor */
	GUID component_guid; /* a unique identifier for your Component */
	GUID framework_guid; /* optionally set to the framework that your component is meant to work in.  Can optionally be used as a filter by the Component Manager.  This avoids loading, e.g., a replicant component on some completely different Wasabi-based framework */
	GUID application_guid; /* if your component is meant to work with a specific application, you can define it here.  Can optionally be used as a filter by the Component Manager. */
};

class NOVTABLE ifc_component : public Wasabi2::Dispatchable
	//public wa::lists::node_ptr /* this is for internal use by the Component Manager! */
{
protected:
	ifc_component(GUID component_guid) : Wasabi2::Dispatchable(DISPATCHABLE_VERSION)
	{
		memset(&component_info, 0, sizeof(component_info)); /* memsetting the whole thing avoids a bunch of ifdef code */
		component_info.wasabi_version = wasabi2_component_version;
		component_info.nx_api_version = nx_api_version;
		component_info.nx_platform_guid = nx_platform_guid;		
		component_info.component_guid = component_guid;
	}
	~ifc_component() {}
public:
	WasabiComponentInfo component_info; /* it's unusual for an interface to have data members, but this makes thing convienent */

	int Initialize(api_service *_service_manager) { return Component_Initialize(_service_manager); }
	int RegisterServices(api_service *_service_manager) { return Component_RegisterServices(_service_manager); }
	int OnLoading(api_service *_service_manager) { return Component_OnLoading(_service_manager); }
	int OnLoaded(api_service *_service_manager) {return Component_OnLoaded(_service_manager); }

	int OnClosing(api_service *_service_manager) { return Component_OnClosing(_service_manager); }
	void DeregisterServices(api_service *_service_manager) { Component_DeregisterServices(_service_manager); }
	int OnClosed(api_service *_service_manager) { return Component_OnClosed(_service_manager); }
	int Quit(api_service *_service_manager) { return Component_Quit(_service_manager); }

	enum
	{
		DISPATCHABLE_VERSION,
	};
private:
	/* these 4 will get called in sequence during component load, see http://wiki.office.aol.com/wiki/Replicant/Component for details */
	// TODO: get rid of default implementations, eventually 
	virtual int WASABICALL Component_Initialize(api_service *_service_manager) { return NErr_Success; }
	virtual int WASABICALL Component_RegisterServices(api_service *_service_manager)=0;
	virtual int WASABICALL Component_OnLoading(api_service *_service_manager) { return NErr_Success; }
	virtual int WASABICALL Component_OnLoaded(api_service *_service_manager) { return NErr_Success; }

	virtual int WASABICALL Component_OnClosing(api_service *_service_manager) { return NErr_Success; }
	virtual void WASABICALL Component_DeregisterServices(api_service *_service_manager)=0;
	virtual int WASABICALL Component_OnClosed(api_service *_service_manager) { return NErr_Success; }
	virtual int WASABICALL Component_Quit(api_service *_service_manager) { return NErr_Success; }
};


extern "C" typedef ifc_component *(*GETCOMPONENT_FUNC)();
