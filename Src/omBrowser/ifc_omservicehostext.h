#ifndef NULLSOFT_WINAMP_OMSERVICE_HOST_EXTENSION_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_HOST_EXTENSION_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {0DBA9261-79B5-4803-BB29-6246DE6C92C9}
static const GUID IFC_OmServiceHostExt = 
{ 0xdba9261, 0x79b5, 0x4803, { 0xbb, 0x29, 0x62, 0x46, 0xde, 0x6c, 0x92, 0xc9 } };


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_omservicehostext : public Dispatchable
{
protected:
	ifc_omservicehostext() {}
	~ifc_omservicehostext() {}

public:
	HRESULT GetHost(ifc_omservicehost **ppHost);
	HRESULT SetHost(ifc_omservicehost *host);
		
public:
	DISPATCH_CODES
	{
		API_GETHOST	= 10,
		API_SETHOST = 20,
	};
};

inline HRESULT ifc_omservicehostext::GetHost(ifc_omservicehost **ppHost)
{
	return _call(API_GETHOST, (HRESULT)E_NOTIMPL, ppHost);
}

inline HRESULT ifc_omservicehostext::SetHost(ifc_omservicehost *host)
{
	return _call(API_SETHOST, (HRESULT)E_NOTIMPL, host);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_HOST_EXTENSION_INTERFACE_HEADER