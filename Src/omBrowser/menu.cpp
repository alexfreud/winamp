#include "main.h"
#include "./menu.h"
#include "./ratingMenu.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedmenu.h"
#include "./ifc_menucustomizer.h"
#include "./resource.h"
#include "../nu/menuHelpers.h"

#define SUBMENU_RATING				0
#define SUBMENU_TOOLBAR				1
#define SUBMENU_ADDRESSBAR			2

static HRESULT Menu_GetSkinnedMenu(ifc_skinnedmenu **skinnedMenu)
{
	if (NULL == skinnedMenu) return E_POINTER;

	ifc_skinhelper *skinHelper = NULL;
	HRESULT hr = Plugin_GetSkinHelper(&skinHelper);
	if (SUCCEEDED(hr) && skinHelper != NULL)
	{
		hr = skinHelper->QueryInterface(IFC_SkinnedMenu, (void**)skinnedMenu);
		skinHelper->Release();
	}
	return hr;
}

static HMENU Menu_CreateRatingMenu(HMENU baseMenu, INT ratingValue)
{
	HMENU menu = GetSubMenu(baseMenu, SUBMENU_RATING);
	if (NULL == menu) return NULL;

	menu = MenuHelper_DuplcateMenu(menu);
	if (NULL == menu) return NULL;

	if (FALSE == RatingMenu_InitializeMenu(menu, ratingValue))
	{
		DestroyMenu(menu);
		menu = NULL;
	}

	return menu;
}

static HMENU Menu_GetToolbarContext(HMENU baseMenu, UINT flags)
{
	HMENU hMenu = GetSubMenu(baseMenu, SUBMENU_TOOLBAR);
	return hMenu;
}

static HMENU Menu_GetAddressbarContext(HMENU baseMenu, UINT flags)
{
	HMENU hMenu = GetSubMenu(baseMenu, SUBMENU_ADDRESSBAR);
	return hMenu;
}

void Menu_ConvertRatingMenuStar(HMENU menu, UINT menu_id)
{
	MENUITEMINFOW mi = {sizeof(mi), MIIM_DATA | MIIM_TYPE, MFT_STRING};
	wchar_t rateBuf[32] = {0}, *rateStr = rateBuf;
	mi.dwTypeData = rateBuf;
	mi.cch = 32;
	if(GetMenuItemInfoW(menu, menu_id, FALSE, &mi))
	{
		while(rateStr && *rateStr)
		{
			if(*rateStr == L'*') *rateStr = L'\u2605';
			rateStr=CharNextW(rateStr);
		}
		SetMenuItemInfoW(menu, menu_id, FALSE, &mi);
	}
}

HMENU Menu_GetMenu(INT menuKind, UINT flags)
{
	HMENU baseMenu = Plugin_LoadMenu(MAKEINTRESOURCE(IDR_CONTEXTMENU));
	if (NULL == baseMenu) 
		return NULL;

	HMENU rate_hmenu = GetSubMenu(baseMenu,0);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_5);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_4);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_3);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_2);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_1);

	switch(menuKind)
	{
		case MENU_RATING:
			return Menu_CreateRatingMenu(baseMenu, RATINGFROMMCF(flags));
		case MENU_TOOLBAR:
			return Menu_GetToolbarContext(baseMenu, flags);
		case MENU_ADDRESSBAR:
			return Menu_GetAddressbarContext(baseMenu, flags);
	}
	return NULL;
}

void Menu_ReleaseMenu(HMENU hMenu, INT menuKind)
{
	if (NULL == hMenu)
		return;

	switch(menuKind)
	{
		case MENU_RATING:
			DestroyMenu(hMenu);
            break;
		case MENU_TOOLBAR:
			break;
		case MENU_ADDRESSBAR:
			break;
	}
}

BOOL Menu_TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm)
{	
	BOOL result = FALSE;

	ifc_skinnedmenu *skinnedMenu = NULL;	
	if (SUCCEEDED(Menu_GetSkinnedMenu(&skinnedMenu)) && skinnedMenu != NULL)
	{
		ifc_menucustomizer *customizer = NULL;
		if (FAILED(RatingMenu_GetCustomizer(hMenu, &customizer)))
			customizer = NULL;

		result = skinnedMenu->TrackPopup(hMenu, fuFlags, x, y, hwnd, lptpm, customizer);

		if (NULL != customizer)
			customizer->Release();

		skinnedMenu->Release();
	}

	return result;
}

HANDLE Menu_InitializeHook(HWND hwnd, ifc_menucustomizer *customizer)
{
	ifc_skinnedmenu *skinnedMenu = NULL;
	HANDLE hook = NULL;
	if (SUCCEEDED(Menu_GetSkinnedMenu(&skinnedMenu)) && skinnedMenu != NULL)
	{
		hook = skinnedMenu->InitPopupHook(hwnd, customizer);
		skinnedMenu->Release();
	}
	return hook;
}

HRESULT Menu_RemoveHook(HANDLE menuHook)
{
	ifc_skinnedmenu *skinnedMenu = NULL;
	HRESULT hr = Menu_GetSkinnedMenu(&skinnedMenu);
	if (SUCCEEDED(hr) && skinnedMenu != NULL)
	{
		hr = skinnedMenu->RemovePopupHook(menuHook);
		skinnedMenu->Release();
	}
	return hr;
}