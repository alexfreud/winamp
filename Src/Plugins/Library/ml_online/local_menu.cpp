#include "./main.h"
#include "./local_menu.h"
#include "../nu/menuHelpers.h"
#include "./resource.h"
#include "./api__ml_online.h"
#include "../../General/gen_ml/menu.h"

#include <windows.h>
#include <strsafe.h>

#define MENU_SERVICECONTEXT		0
#define MENU_GALERYCONTEXT		1
#define MENU_RATING				2
#define MENU_VIEW				3
#define MENU_NAVIGATION			4
#define MENU_TOOLBAR			5

#define RATING_MARKER			MAKELONG(MAKEWORD('R','A'),MAKEWORD('T','E'))

#define RATING_MINSPACECX		16



typedef BOOL (__cdecl *MLISSKINNEDPOPUPENABLED)(void);
typedef HANDLE (__cdecl *MLINITSKINNEDPOPUPHOOK)(HWND /*hwnd*/, HMLIMGLST /*hmlil*/, INT /*width*/, UINT /*skinStyle*/,
												MENUCUSTOMIZEPROC /*customProc*/, ULONG_PTR /*customParam*/);
typedef HANDLE (__cdecl *MLREMOVESKINNEDPOPUPHOOK)(HANDLE /*hPopupHook*/);

#if 0
static BOOL Menu_IsRatingStar(HMENU hMenu, INT itemId, INT *valueOut)
{
	WCHAR szBuffer[8] = {0};
	INT cchBuffer = GetMenuStringW(hMenu, itemId, szBuffer, ARRAYSIZE(szBuffer), MF_BYCOMMAND);
	if (cchBuffer < 1 || cchBuffer > 5) 
        return FALSE;
	
	for (INT i = 1; i < cchBuffer; i++)
	{
		if (szBuffer[i -1] != szBuffer[i])
			return FALSE;
	}

	if (NULL != valueOut)
		*valueOut = cchBuffer;

	return TRUE;
}

static BOOL Menu_MeasureRating(HMENU hMenu, HDC hdc, MEASUREITEMSTRUCT *pmis)
{
	if (NULL == hdc || !Menu_IsRatingStar(hMenu, pmis->itemID, NULL))
		return FALSE;

	RECT rect;
	if (!MLRating_CalcRect(Plugin_GetLibrary(), NULL, 5, &rect))
		return FALSE;
	
	pmis->itemHeight = rect.bottom - rect.top + 6;
	
	TEXTMETRIC tm;
	if (GetTextMetrics(hdc, &tm) && 
		(UINT)(tm.tmHeight + 2) > pmis->itemHeight)
	{
		pmis->itemHeight = tm.tmHeight + 2;
	}
				
	INT spaceCX = (pmis->itemHeight > RATING_MINSPACECX) ? pmis->itemHeight : RATING_MINSPACECX;
	pmis->itemWidth = rect.right - rect.left + (2 * spaceCX) - (GetSystemMetrics(SM_CXMENUCHECK) - 1);

	return TRUE;
}

static BOOL Menu_DrawRating(HMENU hMenu, HDC hdc, DRAWITEMSTRUCT *pdis)
{
	INT ratingValue;
	if (NULL == hdc || !Menu_IsRatingStar(hMenu, pdis->itemID, &ratingValue))
		return FALSE;

	INT spaceCX = ((pdis->rcItem.bottom - pdis->rcItem.top) > RATING_MINSPACECX) ? 
					(pdis->rcItem.bottom - pdis->rcItem.top) : 
					RATING_MINSPACECX;

	RATINGDRAWPARAMS rdp = {0};
	rdp.cbSize = sizeof(RATINGDRAWPARAMS);
	rdp.hdcDst = hdc;
	rdp.rc = pdis->rcItem;
	rdp.rc.left += spaceCX;
	rdp.value = ratingValue;
	rdp.maxValue = 5;
	
	UINT menuState = GetMenuState(hMenu, pdis->itemID, MF_BYCOMMAND);
	rdp.trackingValue = (0 == ((MF_DISABLED | MF_GRAYED) & menuState)) ? rdp.value : 0;

	rdp.fStyle = RDS_LEFT | RDS_VCENTER | RDS_HOT;
	rdp.hMLIL = NULL;
	rdp.index = 0;

	return MLRating_Draw(Plugin_GetLibrary(), &rdp);
}

static BOOL CALLBACK Menu_CustomDrawProc(INT action, HMENU hMenu, HDC hdc, LPARAM param, ULONG_PTR user)
{
	switch(action)
	{
		case MLMENU_ACTION_MEASUREITEM:
			if (hMenu == (HMENU)user)
				return Menu_MeasureRating(hMenu, hdc, (MEASUREITEMSTRUCT*)param);
			break;
		case MLMENU_ACTION_DRAWITEM:
			if (hMenu == (HMENU)user)
				return MLMENU_WANT_DRAWPART;
			break;
		case MLMENU_ACTION_DRAWBACK:
			break;
		case MLMENU_ACTION_DRAWICON:
			break;
		case MLMENU_ACTION_DRAWTEXT:
			if (hMenu == (HMENU)user)
				return Menu_DrawRating(hMenu, hdc, (DRAWITEMSTRUCT*)param);
			break;
	}
	return FALSE;
}
#endif

BOOL Menu_SetRatingValue(HMENU ratingMenu, INT ratingValue)
{
	if (NULL == ratingMenu) return FALSE;

	INT ratingList[] = { ID_RATING_VALUE_1, ID_RATING_VALUE_2, ID_RATING_VALUE_3, 
						ID_RATING_VALUE_4, ID_RATING_VALUE_5};
	ratingValue--;

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	
	UINT type, state;
	for (INT i = 0; i < ARRAYSIZE(ratingList); i++)
	{
		mii.fMask = MIIM_STATE | MIIM_FTYPE;
		if (GetMenuItemInfo(ratingMenu, ratingList[i], FALSE, &mii))
		{
			if (ratingValue == i)
			{
				type = mii.fType | MFT_RADIOCHECK;
				state = mii.fState | MFS_CHECKED;
			}
			else
			{
				type = mii.fType & ~MFT_RADIOCHECK;
				state = mii.fState & ~MFS_CHECKED;
			}

			mii.fMask = 0;
			if (type != mii.fType)
			{
				mii.fType = type;
				mii.fMask |= MIIM_FTYPE;
			}
			
			if (state != mii.fState)
			{
				mii.fState = state;
				mii.fMask |= MIIM_STATE;
			}

			if (0 != mii.fMask)
				SetMenuItemInfo(ratingMenu, ratingList[i], FALSE, &mii);
		}
	}
	return TRUE;
}

static HMENU Menu_FindRatingMenuRecur(HMENU hMenu, MENUINFO *pmi, MENUITEMINFO *pmii)
{
	if (GetMenuInfo(hMenu, pmi) && RATING_MARKER == pmi->dwMenuData)
		return hMenu;

	INT count = GetMenuItemCount(hMenu);
	for(INT i = 0; i < count; i++)
	{
		if (GetMenuItemInfo(hMenu, i, TRUE, pmii) && NULL != pmii->hSubMenu)
		{								
			HMENU hRating = Menu_FindRatingMenuRecur(pmii->hSubMenu, pmi, pmii);
			if (NULL != hRating) 
				return hRating;
		}
	}
	return NULL;
}

HMENU Menu_FindRatingMenu(HMENU hMenu)
{
	if (NULL == hMenu) 
		return NULL;

	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;

	MENUINFO mi;
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;

	return Menu_FindRatingMenuRecur(hMenu, &mi, &mii);
}

static BOOL Menu_InsertRatingMenu(HMENU hDest, INT iPos, HMENU baseMenu, INT ratingValue)
{
	if ( 0 == MenuHelper_CopyMenuEx(hDest, iPos, baseMenu, MENU_RATING, 1))
		return FALSE;

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;
	if (GetMenuItemInfo(hDest, iPos, TRUE, &mii))
	{
		if (NULL != mii.hSubMenu)
		{
			MENUINFO mi = {0};
			mi.cbSize = sizeof(MENUINFO);
			mi.fMask = MIM_MENUDATA;
			mi.dwMenuData = RATING_MARKER;
			SetMenuInfo(mii.hSubMenu, &mi);

			Menu_SetRatingValue(mii.hSubMenu, ratingValue);
		}
	}
	return TRUE;
}

static HMENU Menu_CreateRatingMenu(HMENU baseMenu, INT ratingValue)
{
	HMENU menu = GetSubMenu(baseMenu, MENU_RATING);
	if (NULL == menu) return NULL;

	menu = MenuHelper_DuplcateMenu(menu);
	if (NULL == menu) return NULL;

	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;
	mi.dwMenuData = RATING_MARKER;
	SetMenuInfo(menu, &mi);

	Menu_SetRatingValue(menu, ratingValue);
	return menu;
}

static HMENU Menu_GetServiceContext(HMENU baseMenu, UINT flags, BOOL fGaleryMode)
{
	HMENU menu = GetSubMenu(baseMenu, (FALSE == fGaleryMode) ? MENU_SERVICECONTEXT : MENU_GALERYCONTEXT);
	if (NULL == menu) return NULL;

	menu = MenuHelper_DuplcateMenu(menu);
	if (NULL == menu) return NULL;

	HMENU embedMenu;
	INT inserted;
	INT total = GetMenuItemCount(menu);

	if (FALSE == fGaleryMode)
	{
		// rating
		if (Menu_InsertRatingMenu(menu, 0, baseMenu, RATINGFROMMCF(flags)))
			total++;
	}

	if (0 != (MCF_NAVIGATION & flags))
	{// navigation
		embedMenu = GetSubMenu(baseMenu, MENU_NAVIGATION);
		if (NULL != embedMenu)
		{
			inserted = MenuHelper_CopyMenu(menu, 0, embedMenu);
			if (inserted > 0 && total > 0 && MenuHelper_InsertSeparator(menu, inserted))
				inserted++;
			total += inserted;
		}
	}

	if (0 != (MCF_VIEW & flags))
	{// view
		embedMenu = GetSubMenu(baseMenu, MENU_VIEW);
		if (NULL != embedMenu)
		{
			inserted = MenuHelper_CopyMenu(menu, 0, embedMenu);
			if (inserted > 0 && total > 0 && MenuHelper_InsertSeparator(menu, inserted))
			{
				inserted++;
			}
			total += inserted;
		}

		EnableMenuItem(menu, ID_VIEW_OPEN , MF_BYCOMMAND | ((0 == (MCF_VIEWACTIVE & flags)) ? MF_ENABLED : MF_DISABLED));
		if ( 0 == (MCF_VIEWACTIVE & flags))
			SetMenuDefaultItem(menu, ID_VIEW_OPEN, FALSE);
	}

	return menu;
}

static HMENU Menu_GetToolbarContext(HMENU baseMenu, UINT flags)
{
	HMENU hMenu = GetSubMenu(baseMenu, MENU_TOOLBAR);
	return hMenu;
}

void Menu_ConvertRatingMenuStar(HMENU menu, UINT menu_id)
{
	MENUITEMINFOW mi = {sizeof(mi), MIIM_DATA | MIIM_TYPE, MFT_STRING};
	wchar_t rateBuf[32], *rateStr = rateBuf;
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
	HMENU baseMenu = WASABI_API_LOADMENUW(IDR_CONTEXTMENU);
	if (NULL == baseMenu) 
		return NULL;

	HMENU rate_hmenu = GetSubMenu(baseMenu,2);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_5);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_4);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_3);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_2);
	Menu_ConvertRatingMenuStar(rate_hmenu, ID_RATING_VALUE_1);

	switch(menuKind)
	{
		case OMMENU_SERVICECONTEXT:
		case OMMENU_GALERYCONTEXT:
			return Menu_GetServiceContext(baseMenu, flags, (OMMENU_GALERYCONTEXT == menuKind));
		case OMMENU_RATING:
			return Menu_CreateRatingMenu(baseMenu, RATINGFROMMCF(flags));
		case OMMENU_TOOLBAR:
			return Menu_GetToolbarContext(baseMenu, flags);
	}
	return NULL;
}

void Menu_ReleaseMenu(HMENU hMenu, INT menuKind)
{
	if (NULL == hMenu)
		return;

	switch(menuKind)
	{
		case OMMENU_SERVICECONTEXT:
			DestroyMenu(hMenu);
			break;
		case OMMENU_GALERYCONTEXT:
			DestroyMenu(hMenu);
            break;
		case OMMENU_RATING:
			DestroyMenu(hMenu);
            break;
		case OMMENU_TOOLBAR:
			break;
	}
}

INT DoTrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y,  HWND hwnd, LPTPMPARAMS lptpm)
{
	if (NULL == hMenu)
		return NULL;

	return Menu_TrackPopupParam(Plugin_GetLibrary(), hMenu, fuFlags, x, y,
								hwnd, lptpm, (ULONG_PTR)Menu_FindRatingMenu(hMenu));
}