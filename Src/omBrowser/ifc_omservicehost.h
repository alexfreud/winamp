#ifndef NULLSOFT_WINAMP_OMSERVICE_HOST_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_HOST_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {75339603-8A3A-490d-84B1-DD493004AAE2}
static const GUID IFC_OmServiceHost = 
{ 0x75339603, 0x8a3a, 0x490d, { 0x84, 0xb1, 0xdd, 0x49, 0x30, 0x4, 0xaa, 0xe2 } };

interface IDispatch;
class ifc_omservice;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_omservicehost : public Dispatchable
{
protected:
	ifc_omservicehost() {}
	~ifc_omservicehost() {}

public:
	HRESULT GetExternal(ifc_omservice *service, IDispatch **ppDispatch); // ppDispatch probably will be already set with JSAPI according to generation
	HRESULT GetBasePath(ifc_omservice *service, wchar_t *buffer, unsigned int bufferMax);
	HRESULT GetDefaultName(ifc_omservice *service, wchar_t *buffer, unsigned int bufferMax);
	HRESULT GetUrl(ifc_omservice *service, wchar_t *buffer, unsigned int bufferMax); // buffer will be set with ifc_omservice->GetUrl() you can modify it if you want. Return: S_OK on success, E_NOTIMPL - if do not care or E_XXX for errror

	HRESULT QueryCommandState(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId);
	HRESULT ExecuteCommand(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg);

public:
	DISPATCH_CODES
	{
		API_GETEXTERNAL			= 10,
		API_GETBASEPATH			= 20,
		API_GETDEFAULTNAME		= 30,
		API_QUERYCOMMANDSTATE	= 40,
		API_EXECUTECOMMAND		= 50,
		API_GETURL				= 60,
	};
};

inline HRESULT ifc_omservicehost::GetExternal(ifc_omservice *service, IDispatch **ppDispatch)
{
	return _call(API_GETEXTERNAL, (HRESULT)E_NOTIMPL, service, ppDispatch);
}

inline HRESULT ifc_omservicehost::GetBasePath(ifc_omservice *service, wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETBASEPATH, (HRESULT)E_NOTIMPL, service, buffer, bufferMax);
}

inline HRESULT ifc_omservicehost::GetDefaultName(ifc_omservice *service, wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETDEFAULTNAME, (HRESULT)E_NOTIMPL, service, buffer, bufferMax);
}

inline HRESULT ifc_omservicehost::QueryCommandState(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId)
{
	return _call(API_QUERYCOMMANDSTATE, (HRESULT)E_NOTIMPL, service, hBrowser, commandGroup, commandId);
}

inline HRESULT ifc_omservicehost::ExecuteCommand(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg)
{
	return _call(API_EXECUTECOMMAND, (HRESULT)E_NOTIMPL, service, hBrowser, commandGroup, commandId, commandArg);
}

inline HRESULT ifc_omservicehost::GetUrl(ifc_omservice *service, wchar_t *buffer, unsigned int bufferMax)
{
	return _call(API_GETURL, (HRESULT)E_NOTIMPL, service, buffer, bufferMax);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_HOST_INTERFACE_HEADER