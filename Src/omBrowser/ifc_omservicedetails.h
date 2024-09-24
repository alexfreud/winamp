#ifndef NULLSOFT_WINAMP_OMSERVICE_DETAILS_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_DETAILS_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {74C85447-B11A-47ea-95CB-B2ED54E8029E}
static const GUID IFC_OmServiceDetails = 
{ 0x74c85447, 0xb11a, 0x47ea, { 0x95, 0xcb, 0xb2, 0xed, 0x54, 0xe8, 0x2, 0x9e } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_omservicedetails : public Dispatchable
{
protected:
	ifc_omservicedetails() {}
	~ifc_omservicedetails() {}

public:
	HRESULT GetDescription(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetAuthorFirst(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetAuthorLast(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetUpdated(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetPublished(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetThumbnail(wchar_t *pszBuffer, unsigned int cchBufferMax);
	HRESULT GetScreenshot(wchar_t *pszBuffer, unsigned int cchBufferMax);

public:
	DISPATCH_CODES
	{
		API_GETDESCRIPTION		= 10,
		API_GETAUTHORFIRST		= 20,
		API_GETAUTHORLAST		= 30,
		API_GETUPDATED			= 40,
		API_GETPUBLISHED		= 50,
		API_GETTHUMBNAIL			= 60,
		API_GETSCREENSHOT		= 70,
	};
};


inline HRESULT ifc_omservicedetails::GetDescription(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETDESCRIPTION, E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservicedetails::GetAuthorFirst(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETAUTHORFIRST, E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservicedetails::GetAuthorLast(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETAUTHORLAST, E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservicedetails::GetUpdated(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETUPDATED, E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservicedetails::GetPublished(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETPUBLISHED, E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservicedetails::GetThumbnail(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETTHUMBNAIL, E_NOTIMPL, pszBuffer, cchBufferMax);
}

inline HRESULT ifc_omservicedetails::GetScreenshot(wchar_t *pszBuffer, unsigned int cchBufferMax)
{
	return _call(API_GETSCREENSHOT, E_NOTIMPL, pszBuffer, cchBufferMax);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_DETAILS_INTERFACE_HEADER