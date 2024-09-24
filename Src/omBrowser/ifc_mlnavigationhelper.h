#ifndef NULLSOFT_WINAMP_ML_NAVIGATION_HELPER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_ML_NAVIGATION_HELPER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {816290AB-A249-4b64-A192-C643D0AD68CA}
static const GUID IFC_MlNavigationHelper = 
{ 0x816290ab, 0xa249, 0x4b64, { 0xa1, 0x92, 0xc6, 0x43, 0xd0, 0xad, 0x68, 0xca } };

class ifc_mlnavigationcallback;

class __declspec(novtable) ifc_mlnavigationhelper : public Dispatchable
{

protected:
	ifc_mlnavigationhelper() {}
	~ifc_mlnavigationhelper() {}

public:
	HRESULT GetDefaultIndex(int *index);
	HRESULT QueryIndex(const wchar_t *name, int *index, BOOL *defaultUsed);
	HRESULT ReleaseIndex(const wchar_t *name);
	
	HRESULT RegisterAlias(const wchar_t *name, const wchar_t *address);
	
	HRESULT RegisterCallback(ifc_mlnavigationcallback *callback, unsigned int *cookie);
	HRESULT UnregisterCallback(unsigned int cookie);
	
public:
	DISPATCH_CODES
	{	
		API_GETDEFAULTINDEX = 10,
		API_QUERYINDEX = 20,
		API_RELEASEINDEX = 30,
		API_REGISTERALIAS = 40,

		API_REGISTERCALLBACK	= 50,
		API_UNREGISTERCALLBACK	= 60,
	};
};

inline HRESULT ifc_mlnavigationhelper::GetDefaultIndex(int *index)
{
	return _call(API_GETDEFAULTINDEX, (HRESULT)E_NOTIMPL, index);
}

inline HRESULT ifc_mlnavigationhelper::QueryIndex(const wchar_t *name, int *index, BOOL *defaultUsed)
{
	return _call(API_QUERYINDEX, (HRESULT)E_NOTIMPL, name, index, defaultUsed);
}

inline HRESULT ifc_mlnavigationhelper::ReleaseIndex(const wchar_t *name)
{
	return _call(API_RELEASEINDEX, (HRESULT)E_NOTIMPL, name);
}

inline HRESULT ifc_mlnavigationhelper::RegisterAlias(const wchar_t *name, const wchar_t *address)
{
	return _call(API_REGISTERALIAS, (HRESULT)E_NOTIMPL, name, address);
}

inline HRESULT ifc_mlnavigationhelper::RegisterCallback(ifc_mlnavigationcallback *callback, UINT *cookie)
{
	return _call(API_REGISTERCALLBACK, (HRESULT)API_REGISTERCALLBACK, callback, cookie); 
}

inline HRESULT ifc_mlnavigationhelper::UnregisterCallback(UINT cookie)
{
	return _call(API_UNREGISTERCALLBACK, (HRESULT)E_NOTIMPL, cookie); 
}

#endif //NULLSOFT_WINAMP_ML_NAVIGATION_HELPER_INTERFACE_HEADER