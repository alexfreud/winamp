#include "main.h"
#include "./local_menu.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./navigation.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "../nu/menuHelpers.h"

#define SUBMENU_NAVIGATIONCONTEXT			0

static HMENU Menu_GetNavigationContext(HMENU baseMenu)
{
	HMENU hMenu = GetSubMenu(baseMenu, SUBMENU_NAVIGATIONCONTEXT);
	if (NULL == hMenu) return NULL;

	hMenu = MenuHelper_DuplcateMenu(hMenu);
	if (NULL == hMenu) return NULL;

	HNAVITEM hActive = Navigation_GetActive(NULL);
	if (NULL != hActive)
	{
		EnableMenuItem(hMenu, ID_NAVIGATION_OPEN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	}
	else
	{
		EnableMenuItem(hMenu, ID_NAVIGATION_OPEN, MF_BYCOMMAND | MF_ENABLED);
		SetMenuDefaultItem(hMenu, ID_NAVIGATION_OPEN, FALSE);
	}

	return hMenu;
}

HMENU Menu_GetMenu(UINT menuKind)
{
	HMENU baseMenu = WASABI_API_LOADMENUW(IDR_CONTEXTMENU);
	if (NULL == baseMenu) 
		return NULL;

	switch(menuKind)
	{
		case MENU_NAVIGATIONCONTEXT:
		{
			HMENU menu = Menu_GetNavigationContext(baseMenu);
			if (!GetModuleHandle(L"ml_online.dll"))
			{
				if (DeleteMenu(menu, ID_PLUGIN_PREFERENCES, MF_BYCOMMAND))
				{
					DeleteMenu(menu, 2, MF_BYPOSITION);
				}
			}
			return menu;
		}
	}

	return NULL;
}

BOOL Menu_ReleaseMenu(HMENU hMenu, UINT menuKind)
{
	if (NULL == hMenu) return FALSE;

	switch(menuKind)
	{
		case MENU_NAVIGATIONCONTEXT:
			return DestroyMenu(hMenu);
	}
	return FALSE;
}