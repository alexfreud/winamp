#ifndef NULLSOFT_WEBDEV_PLUGIN_EXTERNAL_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_EXTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/dispatchTable.h"

class ExternalDispatch : public IDispatch
{

public:
	typedef enum
	{
		DISPATCH_SERVICE_OPEN	= 700,
		DISPATCH_SERVICE_CREATE = 701,
		DISPATCH_SERVICE_GETINFO = 702,
		DISPATCH_SERVICE_SETINFO = 703,
	} DispatchCodes;

protected:
	ExternalDispatch();
	~ExternalDispatch();

public:
	static HRESULT CreateInstance(ExternalDispatch **instance);
	static LPCWSTR GetName();

public:
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

protected:
	DISPTABLE_INCLUDE();
	DISPHANDLER_REGISTER(OnServiceOpen);
	DISPHANDLER_REGISTER(OnServiceCreate);
	DISPHANDLER_REGISTER(OnServiceGetInfo);
	DISPHANDLER_REGISTER(OnServiceSetInfo);

protected:
	ULONG ref;

};


#endif //NULLSOFT_WEBDEV_PLUGIN_EXTERNAL_HEADER
