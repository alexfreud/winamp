#include "main.h"
#include "./local_menu.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./navigation.h"
#include "./import.h"

#include "../../General/gen_ml/ml_ipc_0313.h"
#include "../nu/menuHelpers.h"
#include "./ombrowser/browserView.h"
#include "./ombrowser/toolbar.h"

#include "./serviceHelper.h"
#include <ifc_omservice.h>

#define SUBMENU_VIEW			0
#define SUBMENU_SERVICEMNGR		1
#define SUBMENU_NAVIGATION		2
#define SUBMENU_PLUGIN			3

static INT Menu_AddViewContext(HMENU destMenu, HMENU baseMenu, BOOL viewActive)
{
	if (NULL == destMenu) return 0;

	HMENU sorceMenu = GetSubMenu(baseMenu, SUBMENU_VIEW);
	if (NULL == sorceMenu) return 0;

	INT c = GetMenuItemCount(destMenu);
	c = MenuHelper_CopyMenu(destMenu, c, sorceMenu);
	
	
	if (FALSE != viewActive)
	{
		EnableMenuItem(destMenu, ID_VIEW_OPEN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	}
	else
	{
		EnableMenuItem(destMenu, ID_VIEW_OPEN, MF_BYCOMMAND | MF_ENABLED);
		SetMenuDefaultItem(destMenu, ID_VIEW_OPEN, FALSE);
	}
	return c;
}

typedef struct __NAVITEMPAIR
{
	INT wId;
	LPCSTR toolItem;
} NAVITEMPAIR;

static INT Menu_AddNavigationContext(HMENU destMenu, HMENU baseMenu)
{
	if (NULL == destMenu) return 0;

	HMENU sorceMenu = GetSubMenu(baseMenu, SUBMENU_NAVIGATION);
	if (NULL == sorceMenu) return 0;

	INT c = GetMenuItemCount(destMenu);
	c = MenuHelper_CopyMenu(destMenu, c, sorceMenu);
	if (0 == c) return c;
	
	HWND hView = NULL;

	Navigation *navigation;
	if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
	{
		hView = navigation->GetActiveView(NULL);
		navigation->Release();
	}
	
	HWND hToolbar = (NULL != hView) ? BrowserView_GetToolbar(hView) : NULL;
	if (NULL != hToolbar)
	{
		NAVITEMPAIR szItems[] = 
		{ 
			{ID_NAVIGATION_BACK, TOOLITEM_BUTTON_BACK},
			{ID_NAVIGATION_FORWARD, TOOLITEM_BUTTON_FORWARD},
			{ID_NAVIGATION_HOME, TOOLITEM_BUTTON_HOME},
			{ID_NAVIGATION_STOP, TOOLITEM_BUTTON_STOP},
			{ID_NAVIGATION_REFRESH, TOOLITEM_BUTTON_REFRESH},
		};

		for (INT i = 0; i < ARRAYSIZE(szItems); i++)
		{
			INT index = Toolbar_FindItem(hToolbar, szItems[i].toolItem);
			if (-1 != index)
			{
				BOOL fDisabled = Toolbar_GetItemStyle(hToolbar, MAKEINTRESOURCE(index), TBIS_DISABLED);
				EnableMenuItem(destMenu, szItems[i].wId, MF_BYCOMMAND | (fDisabled) ? (MF_DISABLED | MF_GRAYED) : (MF_ENABLED));
			}
		}
	}

	return c;
}

static INT Menu_AddServiceContext(HMENU destMenu, HMENU baseMenu, ifc_omservice *service)
{
	if (NULL == destMenu || NULL == service) return 0;

	HMENU sorceMenu = GetSubMenu(baseMenu, SUBMENU_SERVICEMNGR);
	if (NULL == sorceMenu) return 0;

	INT origCount = GetMenuItemCount(destMenu);
	INT c = MenuHelper_CopyMenu(destMenu, origCount, sorceMenu);
	UINT serviceFlags;
	if (FAILED(service->GetFlags(&serviceFlags))) serviceFlags = 0;

	if (0 != ((WDSVCF_ROOT | WDSVCF_SPECIAL) & serviceFlags))
	{
		UINT szDisabled[] = {ID_SERVICE_EDIT, ID_SERVICE_DELETE, ID_SERVICE_RELOAD, 
						ID_SERVICE_LOCATE, ID_SERVICE_EDITEXTERNAL };
		
		for (INT i = 0; i < ARRAYSIZE(szDisabled); i++)
			EnableMenuItem(destMenu, szDisabled[i], MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	
	UINT uEnable;
	
	uEnable = (S_OK == ImportService_GetFileSupported()) ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
	EnableMenuItem(destMenu, ID_SERVICE_IMPORT_FILE, MF_BYCOMMAND | uEnable);

	uEnable = (S_OK == ImportService_GetUrlSupported()) ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
	EnableMenuItem(destMenu, ID_SERVICE_IMPORT_URL, MF_BYCOMMAND | uEnable);
	
	return c;
}

static INT Menu_AddPluginContext(HMENU destMenu, HMENU baseMenu, ifc_omservice *service)
{
	UINT serviceFlags;
	if (NULL == service || FAILED(service->GetFlags(&serviceFlags))) 
		serviceFlags = 0;

	if (NULL == destMenu || NULL == service ||
		0 == (WDSVCF_ROOT & serviceFlags))
	{
		return 0;
	}

	HMENU sorceMenu = GetSubMenu(baseMenu, SUBMENU_PLUGIN);
	if (NULL == sorceMenu) return 0;

	INT origCount = GetMenuItemCount(destMenu);
	INT c = MenuHelper_CopyMenu(destMenu, origCount, sorceMenu);
			
	return c;
}

static HMENU Menu_GetServiceContext(HMENU baseMenu, ifc_omservice *service)
{
	HMENU hMenu = CreatePopupMenu();
	if(NULL == hMenu) return NULL;

	ifc_omservice *activeService = NULL;
	Navigation *navigation;
	if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
	{
		navigation->GetActive(&activeService);
		navigation->Release();
	}

	BOOL fActive = (NULL != service && activeService == service);

	INT c, total;
	total = Menu_AddViewContext(hMenu, baseMenu, fActive);
	if (FALSE != fActive)
	{
		c = Menu_AddNavigationContext(hMenu, baseMenu);
		if (0 != c && total > 0 && 0 != MenuHelper_InsertSeparator(hMenu, total))
			total++;
		
		total += c;
	}

	if (NULL != service)
	{
		c = Menu_AddServiceContext(hMenu, baseMenu, service);
		if (0 != c && total > 0 && 0 != MenuHelper_InsertSeparator(hMenu, total))
			total++;
		
		total += c;

		c = Menu_AddPluginContext(hMenu, baseMenu, service);
		if (0 != c && total > 0 && 0 != MenuHelper_InsertSeparator(hMenu, total))
			total++;
		
		total += c;
	}

	
	if (NULL != activeService)
		activeService->Release();

	return hMenu;
}

HMENU Menu_GetMenu(UINT menuKind, ifc_omservice *service)
{
	HMENU baseMenu = WASABI_API_LOADMENUW(IDR_CONTEXTMENU);
	if (NULL == baseMenu) 
		return NULL;

	switch(menuKind)
	{
		case MENU_SERVICECONTEXT:
			return Menu_GetServiceContext(baseMenu, service);
	}

	return NULL;
}

BOOL Menu_ReleaseMenu(HMENU hMenu, UINT menuKind)
{
	if (NULL == hMenu) return FALSE;

	switch(menuKind)
	{
		case MENU_SERVICECONTEXT:
			return DestroyMenu(hMenu);
	}
	return FALSE;
}