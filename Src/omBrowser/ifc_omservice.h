#ifndef NULLSOFT_WINAMP_OMSERVICE_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>


// {2999A8B2-7780-4542-9257-02D4E89310B8}
static const GUID IFC_OmService = 
{ 0x2999a8b2, 0x7780, 0x4542, { 0x92, 0x57, 0x2, 0xd4, 0xe8, 0x93, 0x10, 0xb8 } };


interface IDispatch;
class ifc_omservicehost;

#define OMSVC_E_INSUFFICIENT_BUFFER MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_INSUFFICIENT_BUFFER) //same as STRSAFE_E_INSUFFICIENT_BUFFER


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_omservice : public Dispatchable
{
public:
	typedef enum
	{
		RuntimeFlagsMask = 0xFFFF0000,
	} FlagsMask;

protected:
	ifc_omservice() {}
	~ifc_omservice() {}

public:
	unsigned int GetId();
	HRESULT GetName(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetUrl(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetUrlDirect(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetIcon(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetExternal(IDispatch **ppDispatch);
	HRESULT GetRating(unsigned int *rating);
	HRESULT GetVersion(unsigned int *version);
	HRESULT GetFlags(unsigned int *flags);
	HRESULT SetAddress(const wchar_t *pszAddress);
	HRESULT GetAddress(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetGeneration(unsigned int *generation);
	HRESULT UpdateFlags(unsigned int flags);
	
public:
	DISPATCH_CODES
	{
		API_GETID				= 10,
		API_GETNAME				= 20,
		API_GETURL				= 30,
		API_GETICON				= 40,
		API_GETEXTERNAL			= 50,
		API_GETRATING			= 60,
		API_GETVERSION			= 70,
		API_GETFLAGS			= 80,
		API_SETADDRESS			= 90,
		API_GETADDRESS			= 100,
		API_GETGENERATION		= 110,
		API_GETURLDIRECT		= 120,
		API_UPDATEFLAGS			= 130,
	};
};


inline unsigned int ifc_omservice::GetId()
{
	return _call(API_GETID, 0);
}

inline HRESULT ifc_omservice::GetName(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETNAME, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservice::GetUrl(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETURL, E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservice::GetUrlDirect(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETURLDIRECT, E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservice::GetIcon(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETICON, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservice::GetExternal(IDispatch **ppDispatch)
{
	return _call(API_GETEXTERNAL, (HRESULT)E_NOTIMPL, ppDispatch);
}

inline HRESULT ifc_omservice::GetRating(unsigned int *rating)
{
	return _call(API_GETRATING, (HRESULT)E_NOTIMPL, rating);
}

inline HRESULT ifc_omservice::GetVersion(unsigned int *version)
{
	return _call(API_GETVERSION, (HRESULT)E_NOTIMPL, version);
}

inline HRESULT ifc_omservice::GetFlags(unsigned int *flags)
{
	return _call(API_GETFLAGS, (HRESULT)E_NOTIMPL, flags);
}

inline HRESULT ifc_omservice::SetAddress(const wchar_t *pszAddress)
{
	return _call(API_SETADDRESS, (HRESULT)E_NOTIMPL, pszAddress);
}

inline HRESULT ifc_omservice::GetAddress(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETADDRESS, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservice::GetGeneration(unsigned int *generation)
{
	return _call(API_GETGENERATION, (HRESULT)E_NOTIMPL, generation);
}

inline HRESULT ifc_omservice::UpdateFlags(unsigned int flags)
{
	return _call(API_UPDATEFLAGS, (HRESULT)E_NOTIMPL, flags);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_INTERFACE_HEADER