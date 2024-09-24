#include "main.h"
#include "./local_menu.h"
#include "./resource.h"
#include "..\..\General\gen_ml/menu.h"
#include "../nu/menushortcuts.h"

#define RATING_MARKER		MAKELONG(MAKEWORD('R','A'),MAKEWORD('T','E'))
#define RATING_MINSPACECX	16

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
	if (!MLRating_CalcRect(plugin.hwndLibraryParent, NULL, 5, &rect))
		return FALSE;
	
	pmis->itemHeight = rect.bottom - rect.top + 6;
	
	TEXTMETRIC tm = {0};
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
	rdp.rc.left += spaceCX + WASABI_API_APP->getScaleX(13);
	rdp.value = ratingValue;
	rdp.maxValue = 5;
	
	UINT menuState = GetMenuState(hMenu, pdis->itemID, MF_BYCOMMAND);
	rdp.trackingValue = (0 != (ODS_SELECTED & pdis->itemState) && 
					0 == ((MF_DISABLED | MF_GRAYED) & menuState)) ? 
					rdp.value :
					0;

	rdp.fStyle = RDS_LEFT | RDS_VCENTER | RDS_HOT;
	rdp.hMLIL = NULL;
	rdp.index = 0;

	return MLRating_Draw(plugin.hwndLibraryParent, &rdp);
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

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;

	MENUINFO mi;
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;

	return Menu_FindRatingMenuRecur(hMenu, &mi, &mii);
}

BOOL Menu_SetRatingValue(HMENU ratingMenu, INT ratingValue)
{
	if (NULL == ratingMenu) return FALSE;

	INT ratingList[] = { ID_RATE_0, ID_RATE_1, ID_RATE_2, 
						ID_RATE_3, ID_RATE_4, ID_RATE_5};
	
	/// set rating marker
	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;
	mi.dwMenuData = RATING_MARKER;
	if (!SetMenuInfo(ratingMenu, &mi))
		return FALSE;


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


void SyncMenuWithAccelerators(HWND hwndDlg, HMENU menu)
{
	HACCEL szAccel[24] = {0};
	INT c = WASABI_API_APP->app_getAccelerators(hwndDlg, szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
	AppendMenuShortcuts(menu, szAccel, c, MSF_REPLACE);
}

void SwapPlayEnqueueInMenu(HMENU listMenu)
{
	int playPos=-1, enqueuePos=-1;
	MENUITEMINFOW playItem={sizeof(MENUITEMINFOW), 0,}, enqueueItem={sizeof(MENUITEMINFOW), 0,};

	int numItems = GetMenuItemCount(listMenu);

	int alt = 0;
	for (int i=0;i<numItems;i++)
	{
		UINT id = GetMenuItemID(listMenu, i);
		if (id == ID_MEDIAWND_PLAYSELECTEDFILES || id == ID_AUDIOWND_PLAYSELECTION || id == ID_QUERYWND_PLAYQUERY)
		{
			playItem.fMask = MIIM_ID;
			playPos = i;
			GetMenuItemInfoW(listMenu, i, TRUE, &playItem);
			if (id == ID_AUDIOWND_PLAYSELECTION) alt = 1;
			else if (id == ID_QUERYWND_PLAYQUERY) alt = 2;
		}
		else if (id == ID_MEDIAWND_ENQUEUESELECTEDFILES || id == ID_AUDIOWND_ENQUEUESELECTION || id == ID_QUERYWND_ENQUEUEQUERY)
		{
			enqueueItem.fMask = MIIM_ID;
			enqueuePos = i;
			GetMenuItemInfoW(listMenu, i, TRUE, &enqueueItem);
			if (id == ID_AUDIOWND_ENQUEUESELECTION) alt = 1;
			else if (id == ID_QUERYWND_ENQUEUEQUERY) alt = 2;
		}
	}

	playItem.wID = (alt == 1 ? ID_MEDIAWND_ENQUEUESELECTEDFILES : (alt == 2 ? ID_QUERYWND_ENQUEUEQUERY : ID_AUDIOWND_ENQUEUESELECTION));
	enqueueItem.wID = (alt == 1 ? ID_MEDIAWND_PLAYSELECTEDFILES : (alt == 2 ? ID_QUERYWND_PLAYQUERY : ID_AUDIOWND_PLAYSELECTION));
	SetMenuItemInfoW(listMenu, playPos, TRUE, &playItem);
	SetMenuItemInfoW(listMenu, enqueuePos, TRUE, &enqueueItem);
}

void UpdateMenuItems(HWND hwndDlg, HMENU menu, UINT accel_id)
{
	bool swapPlayEnqueue=false;
	if (g_config->ReadInt(L"enqueuedef", 0) == 1)
	{
		SwapPlayEnqueueInMenu(menu);
		swapPlayEnqueue=true;
	}

	if(!IsWindow(hwndDlg))
	{
		HACCEL accel = WASABI_API_LOADACCELERATORSW(accel_id);
		int size = CopyAcceleratorTable(accel,0,0);
		AppendMenuShortcuts(menu, &accel, size, MSF_REPLACE);
	}
	else
	{
		SyncMenuWithAccelerators(hwndDlg, menu);
	}

	if (swapPlayEnqueue) SwapPlayEnqueueInMenu(menu);
}

INT DoTrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm)
{
	if (NULL == hMenu)
		return NULL;

	return Menu_TrackPopupParam(plugin.hwndLibraryParent, hMenu, fuFlags, x, y,
								hwnd, lptpm, (ULONG_PTR)Menu_FindRatingMenu(hMenu));
}