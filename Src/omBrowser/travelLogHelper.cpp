#include "main.h"
#include "./travelLogHelper.h"
#include "./graphics.h"
#include "./resource.h"
#include "./ifc_skinhelper.h"
#include "./ifc_imageloader.h"
#include "./menu.h"
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"

#include <exdisp.h>
#include <tlogstg.h>

#define TRAVELLOGPOPUP_MAXCHARWIDTH			42

TravelLogHelper::TravelLogHelper(IWebBrowser2 *pWeb) 
	: ref(1), pWeb2(pWeb), bitmap(NULL), pixelData(NULL), firstFwd(FALSE), entriesCount(0), backEntry(-1)
{
	if (NULL != pWeb2)
		pWeb2->AddRef();
}

TravelLogHelper::~TravelLogHelper()
{
	if (NULL != pWeb2)
		pWeb2->Release();

	if (NULL != bitmap)
		DeleteObject(bitmap);
}

HRESULT TravelLogHelper::CreateInstance(IWebBrowser2 *pWeb2, TravelLogHelper **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	if (NULL == pWeb2)
		return E_INVALIDARG;

	*instance = new TravelLogHelper(pWeb2);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t TravelLogHelper::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t TravelLogHelper::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int TravelLogHelper::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_TravelLogHelper))
		*object = static_cast<ifc_travelloghelper*>(this);
	else if (IsEqualIID(interface_guid, IFC_MenuCustomizer))
		*object = static_cast<ifc_menucustomizer*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT	TravelLogHelper::QueryStorage(ITravelLogStg **ppLog)
{
	HRESULT hr;

	if (NULL == ppLog)
		return E_POINTER;

	*ppLog = NULL;

	if (NULL == pWeb2) return E_UNEXPECTED;

	IServiceProvider *pProvider;
	hr = pWeb2->QueryInterface(IID_IServiceProvider, (void**)&pProvider);
	if (SUCCEEDED(hr))
	{
		hr = pProvider->QueryService(SID_STravelLogCursor, ppLog);
		pProvider->Release();
	}
	return hr;
}

BOOL TravelLogHelper::DrawIcon(HMENU menuInstance, HDC hdc, DRAWITEMSTRUCT *pdis)
{
	if (0 == (ODS_SELECTED & pdis->itemState))
		return FALSE;
	
	LONG entry = pdis->itemID - 101;
	if (entry < 0 || ((ULONG)entry) > entriesCount)
		return FALSE;

	BOOL fForward = (FALSE != firstFwd) ? TRUE : FALSE;
	if (-1 != backEntry && entry >= backEntry)
		fForward = !fForward;

	if (NULL == bitmap)
	{
		ifc_omimageloader *imageLoader;
		if (SUCCEEDED(Plugin_QueryImageLoader(Plugin_GetInstance(), MAKEINTRESOURCE(IDR_MENUARROW_IMAGE), FALSE, &imageLoader)))
		{
			if (SUCCEEDED(imageLoader->LoadBitmapEx(&bitmap, &header, &pixelData)))
			{			
				if (header.biHeight < 0) header.biHeight = -header.biHeight;

				Image_Colorize((BYTE*)pixelData, header.biWidth, header.biHeight, 
						header.biBitCount, GetBkColor(hdc), GetTextColor(hdc), TRUE);
			}
			imageLoader->Release();
		}
	}
	
	BOOL resultOk = FALSE;
	if (NULL != bitmap)
	{
		INT cx = header.biWidth/2;
		INT cy = header.biHeight;
		if (cy < 0) cy = -cy;

		INT side = (pdis->rcItem.bottom - pdis->rcItem.top - 2);
		if (cy < side) side = cy;

		INT top = pdis->rcItem.top + ((pdis->rcItem.bottom - pdis->rcItem.top) - side)/2;
		INT left = pdis->rcItem.left + (GetSystemMetrics(SM_CXMENUCHECK) - side)/2 + 3;

		resultOk = StretchDIBits(hdc, left, top, side, side, 
					((FALSE != fForward) ? cx : 0), 0, cx, cy, pixelData, (BITMAPINFO*)&header, DIB_RGB_COLORS, SRCCOPY);

	}
	return resultOk;
}

INT TravelLogHelper::CustomDraw(HMENU menuInstance, INT action, HDC hdc, LPARAM param)
{
	switch(action)
	{
		case MLMENU_ACTION_DRAWITEM:
			return MLMENU_WANT_DRAWPART;
		case MLMENU_ACTION_DRAWICON:
			return DrawIcon(menuInstance, hdc, (DRAWITEMSTRUCT*)param);
	}
	return FALSE;
}


HRESULT TravelLogHelper::ShowPopup(UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm)
{
	HRESULT hr;
	ITravelLogStg *pLog;
	hr = QueryStorage(&pLog);
	if (FAILED(hr) || NULL == pLog) return hr;
	
	DWORD entriesMax;
	if (FAILED(pLog->GetCount(TLEF_RELATIVE_FORE | TLEF_RELATIVE_BACK, &entriesMax)))
		entriesMax = 0;

	ITravelLogEntry **entries = NULL;

	INT selectedEntry = -1;
	firstFwd = (0 == (TPM_BOTTOMALIGN & fuFlags));
	entriesCount = 0;
	backEntry = -1;

	if (0 != entriesMax)
	{
		entries = (ITravelLogEntry**)calloc(entriesMax, sizeof(ITravelLogEntry*));
		if (NULL != entries)
		{
			IEnumTravelLogEntry *pEnum = NULL;
			TLENUMF tlenum = (0 != (TPM_BOTTOMALIGN & fuFlags)) ? TLEF_RELATIVE_BACK : TLEF_RELATIVE_FORE;
			if (SUCCEEDED(pLog->EnumEntries(tlenum, &pEnum)))
			{
				ULONG fetched;
				pEnum->Reset();
				pEnum->Next(entriesMax - entriesCount, &entries[entriesCount], &fetched);
				if (0 != fetched)
				{
					ITravelLogEntry *t, **l, **r;
					for (l = &entries[entriesCount], r = &entries[entriesCount + fetched - 1]; l < r; l++, r--)
					{
						t = *l;
						*l = *r;
						*r = t;
					}
					entriesCount += fetched;
				}
				pEnum->Release();
			}

			tlenum = (0 != (TLEF_RELATIVE_FORE & tlenum)) ?
						((tlenum & ~TLEF_RELATIVE_FORE) | TLEF_RELATIVE_BACK) :
						tlenum = ((tlenum & ~TLEF_RELATIVE_BACK) | TLEF_RELATIVE_FORE);

			if (SUCCEEDED(pLog->EnumEntries(tlenum, &pEnum)))
			{

				ULONG fetched;
				pEnum->Reset();
				pEnum->Next(entriesMax - entriesCount, &entries[entriesCount], &fetched);
				if (0 != fetched)
				{
					backEntry = entriesCount;
					entriesCount += fetched;
				}
				pEnum->Release();
			}
		}

		if (entriesCount > 0)
		{
			HMENU hMenu = CreatePopupMenu();
			if (NULL != hMenu)
			{
				MENUITEMINFOW mii = {0};
				mii.cbSize = sizeof(MENUITEMINFOW);
				mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
				mii.fState = MFS_ENABLED | MFS_UNCHECKED;
							
				for (ULONG i = 0; i < entriesCount; i++)
				{
					if (NULL != entries[i])
					{
						LPWSTR pszTitle;
						if (SUCCEEDED(entries[i]->GetTitle(&pszTitle)) && NULL != pszTitle)
						{
							INT cchTitle = lstrlen(pszTitle);

							if (cchTitle > TRAVELLOGPOPUP_MAXCHARWIDTH)
							{
								pszTitle[TRAVELLOGPOPUP_MAXCHARWIDTH] = L'\0';
								for (INT k = 0; k < 3; k++)
									pszTitle[TRAVELLOGPOPUP_MAXCHARWIDTH - 1 - k] = L'.'; 

							}
							mii.dwTypeData = pszTitle;
							mii.wID = 101 + i;
							InsertMenuItem(hMenu, i, TRUE, &mii);
							CoTaskMemFree(pszTitle);
						}
					}
				}


				{ // insert current page
					WCHAR szBuffer[256] = {0};

					Plugin_LoadString(IDS_CURRENT_PAGE, szBuffer, ARRAYSIZE(szBuffer));

					mii.dwTypeData = szBuffer;
					mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE | MIIM_FTYPE;
					mii.fState = MFS_ENABLED | MFS_DEFAULT | MFS_CHECKED;
					mii.fType = MFT_STRING | MFT_RADIOCHECK;
					mii.wID = 100;

					if (-1 == backEntry)
						InsertMenuItem(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);
					else
						InsertMenuItem(hMenu, 101 + backEntry, FALSE, &mii);
				}
				
				
				
				HANDLE hHook = Menu_InitializeHook(hwnd, this);
					
				UINT commandId = TrackPopupMenuEx(hMenu, fuFlags | TPM_RETURNCMD, x, y, hwnd, lptpm);

				if (NULL != hHook) Menu_RemoveHook(hHook);

				if (commandId > 100 && commandId <= (100 + entriesCount))
					selectedEntry = commandId - 101;

				DestroyMenu(hMenu);
			}
		}

	}
	
	if (-1 != selectedEntry && NULL != entries[selectedEntry])
	{
		pLog->TravelTo(entries[selectedEntry]);
	}

	if (NULL != entries)
	{
		for (ULONG i = 0; i < entriesCount; i++)
		{
			if (NULL != entries[i]) entries[i]->Release();
		}
		free(entries);

	}
	pLog->Release();
	return hr;
}

#define CBCLASS TravelLogHelper
START_MULTIPATCH;
 START_PATCH(MPIID_TRAVELLOGHELPER)
  M_CB(MPIID_TRAVELLOGHELPER, ifc_travelloghelper, ADDREF, AddRef);
  M_CB(MPIID_TRAVELLOGHELPER, ifc_travelloghelper, RELEASE, Release);
  M_CB(MPIID_TRAVELLOGHELPER, ifc_travelloghelper, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_TRAVELLOGHELPER, ifc_travelloghelper, API_QUERYSTORAGE, QueryStorage);
  M_CB(MPIID_TRAVELLOGHELPER, ifc_travelloghelper, API_SHOWPOPUP, ShowPopup);
 NEXT_PATCH(MPIID_MENUCUSTOMIZER)
  M_CB(MPIID_MENUCUSTOMIZER, ifc_menucustomizer, ADDREF, AddRef);
  M_CB(MPIID_MENUCUSTOMIZER, ifc_menucustomizer, RELEASE, Release);
  M_CB(MPIID_MENUCUSTOMIZER, ifc_menucustomizer, QUERYINTERFACE, QueryInterface);	
  M_CB(MPIID_MENUCUSTOMIZER, ifc_menucustomizer, API_CUSTOMDRAW, CustomDraw);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS