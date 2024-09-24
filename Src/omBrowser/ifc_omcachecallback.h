#ifndef NULLSOFT_WINAMP_OMCACHE_CALLBACK_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMCACHE_CALLBACK_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {A82E94C9-38BA-44ba-9879-0F3CAFCB3527}
static const GUID IFC_OmCacheCallback = 
{ 0xa82e94c9, 0x38ba, 0x44ba, { 0x98, 0x79, 0xf, 0x3c, 0xaf, 0xcb, 0x35, 0x27 } };

class ifc_omcacherecord;

class __declspec(novtable) ifc_omcachecallback: public Dispatchable
{

protected:
	ifc_omcachecallback() {}
	~ifc_omcachecallback() {}

public:
	void PathChanged(ifc_omcacherecord *record);
	

public:
	DISPATCH_CODES
	{	
		API_PATHCHANGED = 10,
	};
};

inline void ifc_omcachecallback::PathChanged(ifc_omcacherecord *record)
{
	_voidcall(API_PATHCHANGED, record);
}

#endif //NULLSOFT_WINAMP_OMCACHE_CALLBACK_INTERFACE_HEADER