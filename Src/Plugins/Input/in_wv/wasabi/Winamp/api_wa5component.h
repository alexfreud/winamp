#ifndef __WASABI_API_WA5COMPONENT_H_
#define __WASABI_API_WA5COMPONENT_H_

#include <bfc/dispatch.h>
class api_service;
#ifdef WIN32
#include <windows.h>
#endif

class NOVTABLE api_wa5component : public Dispatchable
{
public:
	DISPATCH_CODES
	{
		API_WA5COMPONENT_REGISTERSERVICES = 10,
		API_WA5COMPONENT_DEREEGISTERSERVICES = 20,
	};

	void RegisterServices(api_service *service);
	void DeregisterServices(api_service *service);
#ifdef WIN32 // this is a kind of a hack (might be better to create a function that winamp calls to pass it)
	HMODULE hModule;
#endif
};
inline void api_wa5component::RegisterServices(api_service *service)
{
	_voidcall(API_WA5COMPONENT_REGISTERSERVICES, service);
}

inline void api_wa5component::DeregisterServices(api_service *service)
{
	_voidcall(API_WA5COMPONENT_DEREEGISTERSERVICES, service);
}
#endif