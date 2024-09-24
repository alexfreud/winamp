#ifndef NULLSOFT_WINAMP_OMBROWSER_TRAVELLOG_HELPER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TRAVELLOG_HELPER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {2C4913CF-D04A-402e-81B4-B5A1A392618C}
static const GUID IFC_TravelLogHelper = 
{ 0x2c4913cf, 0xd04a, 0x402e, { 0x81, 0xb4, 0xb5, 0xa1, 0xa3, 0x92, 0x61, 0x8c } };

#include <bfc/dispatch.h>

interface ITravelLogStg;

class __declspec(novtable) ifc_travelloghelper : public Dispatchable
{
protected:
	ifc_travelloghelper() {}
	~ifc_travelloghelper() {}

public:
	HRESULT	QueryStorage(ITravelLogStg **ppLog);
	HRESULT ShowPopup(UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm);

public:
	DISPATCH_CODES
	{
		API_QUERYSTORAGE		= 10,
		API_SHOWPOPUP		= 20,
	};
};

inline HRESULT ifc_travelloghelper::QueryStorage(ITravelLogStg **ppLog)
{
	return _call(API_QUERYSTORAGE, (HRESULT)E_NOTIMPL, ppLog); 
}

inline HRESULT ifc_travelloghelper::ShowPopup(UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm)
{
	return _call(API_SHOWPOPUP, (HRESULT)E_NOTIMPL, fuFlags, x, y, hwnd, lptpm);
}

#endif // NULLSOFT_WINAMP_OMBROWSER_TRAVELLOG_HELPER_INTERFACE_HEADER