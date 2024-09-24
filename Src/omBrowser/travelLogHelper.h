#ifndef NULLSOFT_WINAMP_TRAVELLOG_HELPER_HEADER
#define NULLSOFT_WINAMP_TRAVELLOG_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./browserInternal.h"
#include "./ifc_menucustomizer.h"
#include "./ifc_travelloghelper.h"
#include "./obj_ombrowser.h"
#include <bfc/multipatch.h>

interface IWebBrowser2;

#define MPIID_TRAVELLOGHELPER	10
#define MPIID_MENUCUSTOMIZER		20


class TravelLogHelper : public MultiPatch<MPIID_TRAVELLOGHELPER, ifc_travelloghelper>,
						public MultiPatch<MPIID_MENUCUSTOMIZER, ifc_menucustomizer>
	
{
protected:
	TravelLogHelper(IWebBrowser2 *pWeb2);
	~TravelLogHelper();

public:
	static HRESULT CreateInstance(IWebBrowser2 *pWeb2, TravelLogHelper **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_travelloghelper */
	HRESULT	QueryStorage(ITravelLogStg **ppLog);
	HRESULT ShowPopup(UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm);

	/* ifc_menucustomizer */
	INT CustomDraw(HMENU menuInstance, INT action, HDC hdc, LPARAM param);

protected:
	BOOL DrawIcon(HMENU menuInstance, HDC hdc, DRAWITEMSTRUCT *pdis);

protected:
	RECVS_MULTIPATCH;;

protected:
	ULONG ref;
	IWebBrowser2 *pWeb2;
	HBITMAP bitmap;
	BITMAPINFOHEADER header;
	void	*pixelData;
	BOOL	firstFwd;
	ULONG	entriesCount;
	LONG	backEntry;
};

#endif //NULLSOFT_WINAMP_TRAVELLOG_HELPER_HEADER