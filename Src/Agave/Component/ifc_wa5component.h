#ifndef __WASABI_IFC_WA5COMPONENT_H_
#define __WASABI_IFC_WA5COMPONENT_H_

#include <bfc/dispatch.h>

class api_service;
#ifdef WIN32
#include <windows.h>
#endif

class NOVTABLE ifc_wa5component : public Dispatchable
{
public:
	DISPATCH_CODES
	{
		API_WA5COMPONENT_REGISTERSERVICES           = 10,
		API_WA5COMPONENT_REGISTERSERVICES_SAFE_MODE = 15,
		API_WA5COMPONENT_DEREEGISTERSERVICES        = 20,
	};

	void RegisterServices( api_service *service );
	void DeregisterServices( api_service *service );

#ifdef WIN32 
	HMODULE hModule;
#else
	void *dlLibrary; // pointer returned from dlopen
#endif
};

inline void ifc_wa5component::RegisterServices( api_service *service )
{
	_voidcall( API_WA5COMPONENT_REGISTERSERVICES, service );
}

inline void ifc_wa5component::DeregisterServices( api_service *service )
{
	_voidcall( API_WA5COMPONENT_DEREEGISTERSERVICES, service );
}

extern "C" typedef ifc_wa5component *(*GETCOMPONENT_FUNC)();

#endif