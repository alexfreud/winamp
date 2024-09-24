#include "main.h"
#include "./navigation.h"
#include "./resource.h"
#include "./wasabi.h"
#include "./service.h"
#include "../omBrowser/browserView.h"
#include "../winamp/wa_ipc.h"
#include "../replicant/nu/Autowide.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "./local_menu.h"
#include "../gen_ml/menu.h"
#include <strsafe.h>

#define NAVITEM_PREFIX	L"nowplaying_svc_"

#define E_NAVITEM_UNKNOWN		E_NOINTERFACE

typedef struct __NAVENUMRESULT
{
	HNAVITEM hItem;
    OmService *service;
	UINT serviceId;
	LPCWSTR pszPrefix;
	INT		cchPrefix;
	HWND hLibrary;
	NAVITEM itemInfo;
	WCHAR szBuffer[256];
} NAVENUMRESULT;

typedef struct __FORCEURLDATA
{
	UINT	serviceId;
	LPWSTR	url;
} FORCEURLDATA;

#define FORCEURLPROP		L"MLNOWPLAYING_FORCEURL"

static void Navigation_RemoveForceUrl()
{
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return;

	FORCEURLDATA *data = (FORCEURLDATA*)GetProp(hLibrary, FORCEURLPROP);
	RemoveProp(hLibrary, FORCEURLPROP);
	if (NULL != data)
	{
		Plugin_FreeString(data->url);
		free(data);
	}
}

static HRESULT Navigation_SetForceUrl(UINT serviceId, LPCWSTR pszUrl)
{
	if (NULL == pszUrl) return E_INVALIDARG;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_FAIL;

	FORCEURLDATA *data = (FORCEURLDATA*)GetProp(hLibrary, FORCEURLPROP);

	if (NULL != data)
	{
		Plugin_FreeString(data->url);
		if (data->serviceId != serviceId)
		{
			free(data);
			data = NULL;
		}
	}

	if (NULL == data)
	{
		data = (FORCEURLDATA*)calloc(1, sizeof(FORCEURLDATA));
		if (NULL == data) return E_OUTOFMEMORY;
		data->serviceId = serviceId;
	}
	
	data->url = Plugin_CopyString(pszUrl);
	if (NULL == data->url || FALSE == SetProp(hLibrary, FORCEURLPROP, data))
	{
		Navigation_RemoveForceUrl();
		return E_FAIL;
	}

	return S_OK;
}

static HRESULT Navigation_GetForceUrl(UINT serviceId, const wchar_t **ppszUrl)
{
	if (NULL == ppszUrl) return E_POINTER;
	*ppszUrl = NULL;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_FAIL;

	FORCEURLDATA *data = (FORCEURLDATA*)GetProp(hLibrary, FORCEURLPROP);

	if (NULL == data || data->serviceId != serviceId)
		return E_NOINTERFACE;

    *ppszUrl = data->url;
	return S_OK;
}

static INT Navigation_GetIconIndex(LPCWSTR pszImage)
{
	HWND hLibrary = Plugin_GetLibrary();	
	if (NULL == hLibrary) return -1;

	HMLIMGLST hmlilNavigation = MLNavCtrl_GetImageList(hLibrary);
	if (NULL == hmlilNavigation) return -1;
	
	MLIMAGESOURCE mlis;
	ZeroMemory(&mlis, sizeof(mlis));
	mlis.cbSize = sizeof(mlis);
	mlis.hInst = NULL;
	mlis.bpp = 24;
	mlis.lpszName = pszImage;
	mlis.type = SRC_TYPE_PNG;
	mlis.flags = ISF_FORCE_BPP | ISF_PREMULTIPLY | ISF_LOADFROMFILE;
	
	MLIMAGELISTITEM item;
	ZeroMemory(&item, sizeof(item));
	item.cbSize = sizeof(item);
	item.hmlil = hmlilNavigation;
	item.filterUID = MLIF_FILTER3_UID;
	item.pmlImgSource = &mlis;
	
	return MLImageList_Add(hLibrary, &item);
}

static HNAVITEM Navigation_CreateItem(HWND hLibrary, HNAVITEM hParent, OmService *service)
{
	if (NULL == hLibrary || NULL == service) 
		return NULL; 

	WCHAR szName[256] = {0}, szInvariant[64] = {0};
    if (FAILED(service->GetName(szName, ARRAYSIZE(szName))))
		return NULL;

	if (FAILED(StringCchPrintf(szInvariant, ARRAYSIZE(szInvariant), NAVITEM_PREFIX L"%u", service->GetId())))
		return NULL;

	NAVINSERTSTRUCT nis = {0};
	nis.hInsertAfter = NULL;
	nis.hParent = hParent;
	
	WCHAR szIcon[512] = {0};
	INT iIcon = (SUCCEEDED(service->GetIcon(szIcon, ARRAYSIZE(szIcon)))) ? 
				Navigation_GetIconIndex(szIcon) : -1;
	
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.mask = NIMF_TEXT | NIMF_STYLE | NIMF_TEXTINVARIANT | NIMF_PARAM;
	if (-1 != iIcon) 
		nis.item.mask |= (NIMF_IMAGE | NIMF_IMAGESEL);
	
	nis.item.id = 0;
	nis.item.pszText = szName;
	nis.item.pszInvariant = szInvariant;
	nis.item.style = NIS_ALLOWCHILDMOVE;
	nis.item.styleMask = nis.item.style;
	nis.item.lParam = (LPARAM)service;
	nis.item.iImage = iIcon;
	nis.item.iSelectedImage = iIcon;
		
	HNAVITEM hItem = MLNavCtrl_InsertItem(hLibrary,  &nis);
	if (NULL != hItem)
		service->AddRef();
	
	return hItem;
}

static HNAVITEM Navigation_GetMessageItem(INT msg, INT_PTR param1)
{
	HWND hLibrary = Plugin_GetLibrary();
	HNAVITEM hItem  = (msg < ML_MSG_NAVIGATION_FIRST) ? 	MLNavCtrl_FindItemById(hLibrary, param1) : (HNAVITEM)param1;
	return hItem;
}

static HRESULT Navigation_GetService(HWND hLibrary, HNAVITEM hItem, OmService **service)
{
	WCHAR szBuffer[64] = {0};
	
	if (NULL == service) return E_POINTER;
	*service = NULL;

	if (NULL == hLibrary || NULL == hItem) return E_INVALIDARG;

	NAVITEM itemInfo = {0};
	itemInfo.cbSize = sizeof(NAVITEM);
	itemInfo.hItem = hItem;
	itemInfo.pszInvariant = szBuffer;
	itemInfo.cchInvariantMax = ARRAYSIZE(szBuffer);
	itemInfo.mask = NIMF_PARAM | NIMF_TEXTINVARIANT;

	if (FALSE == MLNavItem_GetInfo(hLibrary, &itemInfo))
		return E_FAIL;

	INT cchInvariant = lstrlen(szBuffer);
	INT cchPrefix = ARRAYSIZE(NAVITEM_PREFIX) - 1;
	if (cchInvariant <= cchPrefix ||
		CSTR_EQUAL != CompareString(CSTR_INVARIANT, 0, NAVITEM_PREFIX, cchPrefix, szBuffer, cchPrefix))
	{
		return E_NAVITEM_UNKNOWN;
	}

	*service = (OmService*)itemInfo.lParam;
	(*service)->AddRef();
	return S_OK;
}


static BOOL CALLBACK Navigation_ItemEnumerator(HNAVITEM hItem, LPARAM param)
{
	if (NULL == hItem) return TRUE;
	NAVENUMRESULT *result = (NAVENUMRESULT*)param;
	if (NULL == result) return FALSE;

	result->itemInfo .hItem = hItem;
	if (FALSE != MLNavItem_GetInfo(result->hLibrary, &result->itemInfo) && 
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, result->itemInfo.pszInvariant, result->cchPrefix, 
						result->pszPrefix, result->cchPrefix))
	{
		OmService *service = (OmService*)result->itemInfo.lParam;
		if (NULL != service && service->GetId() == result->serviceId)
		{
			result->hItem = hItem;
			result->service = service;
			service->AddRef();
			return FALSE;
		}
	}
	
	return TRUE;
}

static HRESULT Navigation_CreateView(HNAVITEM hItem, HWND hParent, HWND *hView)
{
	if (NULL == hView) return E_POINTER;
	*hView = NULL;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_UNEXPECTED;

	if (NULL == hItem || NULL == hParent) return E_INVALIDARG;

	HRESULT hr;

	OmService *service = NULL;
	hr = Navigation_GetService(hLibrary, hItem, &service);
	if (SUCCEEDED(hr))
	{
		if (NULL == OMBROWSERMNGR) 
			hr = E_UNEXPECTED;

		if (SUCCEEDED(hr))
		{
			hr = OMBROWSERMNGR->Initialize(NULL, Plugin_GetWinamp());
			if (SUCCEEDED(hr))
			{
				LPCWSTR forceUrl;
				if (FAILED(Navigation_GetForceUrl(service->GetId(), &forceUrl)))
					forceUrl = NULL;

				hr = OMBROWSERMNGR->CreateView(service, hParent, forceUrl, 0, hView);
				Navigation_RemoveForceUrl();
			}
		}

		wchar_t nowplayingurl[1024] = {0};
		// May 2022 - this service url is dead and would need either fixing up or replacing
		lstrcpynW(nowplayingurl, AutoWide(g_config->ReadString("nowplayingurl", "http://client.winamp.com/nowplaying")), ARRAYSIZE(nowplayingurl));
		service->SetUrl(nowplayingurl[0] ? nowplayingurl : SERVICE_HOMEURL);
		service->Release();
	}
	return hr;
}

static BOOL Navigation_GetViewRect(RECT *rect)
{
	if (NULL == rect) return FALSE;
	
	HWND hWinamp = Plugin_GetWinamp();
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hWinamp || NULL == hLibrary) 
		return FALSE;

	HWND hFrame = (HWND)SENDMLIPC(hLibrary, ML_IPC_GETCURRENTVIEW, 0);
	if (NULL == hFrame) 
		hFrame = hLibrary;
	
	return GetWindowRect(hFrame, rect);
}

static HRESULT Navigation_CreatePopup(HNAVITEM hItem, HWND *hWindow)
{
	if (NULL == hWindow) return E_POINTER;
	*hWindow = NULL;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_UNEXPECTED;

	if (NULL == hItem) return E_INVALIDARG;

	HRESULT hr;

	OmService *service;
	hr = Navigation_GetService(hLibrary, hItem, &service);
	if (SUCCEEDED(hr))
	{
		HWND hWinamp = Plugin_GetWinamp();

		if (NULL == OMBROWSERMNGR) 
			hr = E_UNEXPECTED;

		if (SUCCEEDED(hr))
		{
			hr = OMBROWSERMNGR->Initialize(NULL, hWinamp);
			if (SUCCEEDED(hr))
			{
				RECT rect;
				if (FALSE == Navigation_GetViewRect(&rect))
					hr = E_FAIL;
				
				if (SUCCEEDED(hr))
				{
					rect.left += 16;
					rect.top += 16;
					
					hr = OMBROWSERMNGR->CreatePopup(service, rect.left, rect.top, 
									rect.right - rect.left, rect.bottom - rect.top,	hWinamp, NULL, 0, hWindow);
				}
			}
		}
		
		service->Release();
	}

	return hr;	
}


static void Navigation_OnDestroy()
{
	Navigation_RemoveForceUrl();

	if (NULL != OMBROWSERMNGR)
	{
		OMBROWSERMNGR->Finish();
	}
}

static void Navigation_OpenPreferences()
{
	winampMediaLibraryPlugin *(*gp)();
	gp = (winampMediaLibraryPlugin * (__cdecl *)(void))GetProcAddress(GetModuleHandle(L"ml_online.dll"), "winampGetMediaLibraryPlugin");
	if (gp)
	{
		winampMediaLibraryPlugin *mlplugin = gp();
		if (mlplugin && (mlplugin->version >= MLHDR_VER_OLD && mlplugin->version <= MLHDR_VER))
		{
			mlplugin->MessageProc(ML_MSG_CONFIG, 0, 0, 0);
		}
		else
			SendMessage(Plugin_GetWinamp(), WM_WA_IPC, (WPARAM)-1, IPC_OPENPREFSTOPAGE);
	}
	else
		SendMessage(Plugin_GetWinamp(), WM_WA_IPC, (WPARAM)-1, IPC_OPENPREFSTOPAGE);
}

static HRESULT Navigation_ShowContextMenu(HNAVITEM hItem, HWND hHost, POINTS pts)
{
	if (NULL == hItem || NULL == hHost)
		return E_INVALIDARG;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_UNEXPECTED;

	HRESULT hr;

	OmService *service;
	hr = Navigation_GetService(hLibrary, hItem, &service);
	if (FAILED(hr)) return hr;
	
	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(hLibrary, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}
	
	HMENU hMenu = Menu_GetMenu(MENU_NAVIGATIONCONTEXT);
	if (NULL != hMenu)
	{
		INT commandId = Menu_TrackPopup(hLibrary, hMenu,
										TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD, 
										pt.x, pt.y, hHost, NULL);
		
		Menu_ReleaseMenu(hMenu, MENU_NAVIGATIONCONTEXT);

		switch(commandId)
		{
			case ID_NAVIGATION_OPEN:
				MLNavItem_Select(hLibrary, hItem);
				break;
	
			case ID_NAVIGATION_OPENNEWWINDOW:
				{
					HWND hWindow;
					if (SUCCEEDED(Navigation_CreatePopup(hItem, &hWindow)))
					{
						ShowWindow(hWindow, SW_SHOWNORMAL);
					}
				}
				break;
			case ID_NAVIGATION_HELP:
				SENDWAIPC(Plugin_GetWinamp(), IPC_OPEN_URL, L"https://help.winamp.com/hc/articles/8105304048660-The-Winamp-Media-Library");
				break;

			case ID_PLUGIN_PREFERENCES:
				Navigation_OpenPreferences();
				break;
		}
	}

	service->Release();
	
	return hr;
}

BOOL Navigation_Initialize(void)
{
	OmService *service;
	HWND hLibrary = Plugin_GetLibrary();

	MLNavCtrl_BeginUpdate(hLibrary, NUF_LOCK_TOP);

	if (SUCCEEDED(OmService::CreateInstance(&service)))
	{
		HNAVITEM hParent = NULL;
		Navigation_CreateItem(hLibrary, hParent, service);
		service->Release();
	}
	
	MLNavCtrl_EndUpdate(hLibrary);

	return TRUE;
}

static void Navigation_OnDeleteItem(HNAVITEM hItem)
{
	if (NULL == hItem) return;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return;

	OmService *service;
	if (SUCCEEDED(Navigation_GetService(hLibrary, hItem, &service)))
	{
		
		NAVITEM itemInfo;
		itemInfo.cbSize = sizeof(NAVITEM);
		itemInfo.hItem = hItem;
		itemInfo.mask = NIMF_PARAM;
		itemInfo.lParam = 0L;
		MLNavItem_SetInfo(hLibrary, &itemInfo);

		service->Release(); // create
		service->Release(); // Navigation_GetService
	}
}

BOOL Navigation_ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result)
{
	if (msg == ML_MSG_NO_CONFIG)
	{
		if (!GetModuleHandle(L"ml_online.dll"))
		{
			*result = TRUE;
			return TRUE;
		}
	}
	else if (msg == ML_MSG_CONFIG)
	{
		Navigation_OpenPreferences();
		*result = TRUE;
		return TRUE;
	}

	if (msg < ML_MSG_TREE_BEGIN || msg > ML_MSG_TREE_END)
		return FALSE;

	switch(msg)
	{
		case ML_MSG_TREE_ONCREATEVIEW: 
			{
				HWND hView;
				HNAVITEM hItem = Navigation_GetMessageItem(msg, param1);
				HRESULT hr = Navigation_CreateView(hItem, (HWND)param2, &hView);
				if (SUCCEEDED(hr)) 
				{
					*result = (INT_PTR)hView;
					return TRUE;
				}
			}
			break;
			
 		case ML_MSG_NAVIGATION_ONDESTROY:
			Navigation_OnDestroy();
			break;

		case ML_MSG_NAVIGATION_CONTEXTMENU:
			{
				HNAVITEM hItem = Navigation_GetMessageItem(msg, param1);
				HRESULT hr = Navigation_ShowContextMenu(hItem, (HWND)param2, MAKEPOINTS(param3));
				if (SUCCEEDED(hr)) 
				{
					*result = TRUE;
					return TRUE;
				}
			}
			break;

		case ML_MSG_NAVIGATION_ONDELETE:
			{				
				HNAVITEM hItem = Navigation_GetMessageItem(msg, param1);
				Navigation_OnDeleteItem(hItem);
				break;
			}
	}

	return FALSE;
}

HNAVITEM Navigation_FindService(UINT serviceId, OmService **serviceOut)
{
	NAVENUMRESULT result;
	result.hItem = NULL;
	result.service = NULL;

	result.serviceId = serviceId;
	result.pszPrefix = NAVITEM_PREFIX;
	result.cchPrefix = lstrlen(result.pszPrefix);

	result.hLibrary = Plugin_GetLibrary();
	result.itemInfo.cbSize = sizeof(result.itemInfo);
	result.itemInfo.mask = NIMF_TEXTINVARIANT | NIMF_PARAM;
	result.itemInfo.cchInvariantMax = ARRAYSIZE(result.szBuffer);
	result.itemInfo.pszInvariant = result.szBuffer;

	NAVCTRLENUMPARAMS param;
	param.enumProc = Navigation_ItemEnumerator;
	param.hItemStart = NULL;
	param.lParam = (LPARAM)&result;
	
	if (NULL != result.hLibrary)
        MLNavCtrl_EnumItems(result.hLibrary, &param);
	
	if (NULL != serviceOut)
		*serviceOut = result.service;
	else if (NULL != result.service)
		result.service->Release();
	
	return result.hItem;
}

HRESULT Navigation_ShowService(UINT serviceId, LPCWSTR pszUrl, UINT navFlags)
{
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary)
		return E_FAIL;

	OmService *service;
	HNAVITEM hItem = Navigation_FindService(serviceId, &service);
	if (NULL == hItem)
		return E_FAIL;
		
	OmService *activeService;
	HWND hView = Navigation_GetActiveView(&activeService);
	if (NULL == hView || activeService->GetId() != service->GetId())
	{
		hView = NULL;
		activeService = NULL;
	}
	
	HRESULT hr = S_OK;
	
	if (NULL != hView)
	{
		if (NULL == pszUrl && 0 != (NAVFLAG_FORCEACTIVE & navFlags)) 
			pszUrl = NAVIGATE_HOME;

		if (NULL != pszUrl && FALSE == BrowserView_Navigate(hView, pszUrl, TRUE))
			hr = E_FAIL;
	}
	else
	{
		if (NULL != pszUrl)
			hr = Navigation_SetForceUrl(serviceId, pszUrl);
		else
			Navigation_RemoveForceUrl();
				
		if (SUCCEEDED(hr) && FALSE == MLNavItem_Select(hLibrary, hItem))
		{
			Navigation_RemoveForceUrl();
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
	{
		if (0 != (NAVFLAG_ENSUREITEMVISIBLE & navFlags))
			MLNavItem_EnsureVisible(hLibrary, hItem);
		
		if (0 != (NAVFLAG_ENSUREMLVISIBLE & navFlags))
			SENDMLIPC(hLibrary, ML_IPC_ENSURE_VISIBLE, 0L);
	}
	
	service->Release();
	if (NULL != activeService) 
		activeService->Release();
	
	return hr;
	
}
HNAVITEM Navigation_GetActive(OmService **serviceOut)
{
	HWND hLibrary = Plugin_GetLibrary();
	
	OmService *service;
	HNAVITEM hActive = (NULL != hLibrary) ?  MLNavCtrl_GetSelection(hLibrary) : NULL;
	if (NULL == hActive || FAILED(Navigation_GetService(hLibrary, hActive, &service)))
	{
		hActive = NULL;
		service = NULL;
	}
		
	if (NULL != serviceOut) 
		*serviceOut = service;

	return hActive;
}

HWND Navigation_GetActiveView(OmService **serviceOut)
{
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary)
	{
		if (NULL != serviceOut) *serviceOut = NULL;
		return NULL;
	}
	

	HWND hView =((HWND)SENDMLIPC(hLibrary, ML_IPC_GETCURRENTVIEW, 0));
	if (NULL != hView)
	{
		WCHAR szBuffer[128] = {0};
		if (!GetClassName(hView, szBuffer, ARRAYSIZE(szBuffer)) || CSTR_EQUAL != CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, szBuffer, -1, L"Nullsoft_omBrowserView", -1))
			hView = NULL;
	}

	OmService *service;
	HNAVITEM hActive = (NULL != hLibrary) ? MLNavCtrl_GetSelection(hLibrary) : NULL;
	if (NULL == hView || FALSE == BrowserView_GetService(hView, &service))
	{
		hView = NULL;
		service = NULL;
	}

	if (NULL != serviceOut) 
		*serviceOut = service;
	else if (NULL != service)
		service->Release();
	
	return hView;
}