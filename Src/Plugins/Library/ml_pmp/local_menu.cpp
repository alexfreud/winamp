#include "./local_menu.h"
#include "../../General/gen_ml/ml.h"
#include "../../General/gen_ml/menu.h"
#include "./resource1.h"
#include "../nu/menushortcuts.h"
#include "api__ml_pmp.h"
#include "main.h"

extern winampMediaLibraryPlugin plugin;
extern HWND hwndMediaView;

#define RATING_MARKER			 MAKELONG(MAKEWORD('R','A'),MAKEWORD('T','E'))

#define RATING_MINSPACECX		16
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)))
#endif

static HMENU Menu_FindRatingByChild(HMENU hMenu, MENUINFO *pmi, MENUITEMINFO *pmii, UINT childId)
{
	INT count = GetMenuItemCount(hMenu);
	for(INT i = 0; i < count; i++)
	{
		if (GetMenuItemInfo(hMenu, i, TRUE, pmii))
		{
			if (childId == pmii->wID) return hMenu;
			if (NULL != pmii->hSubMenu)
			{								
				HMENU hRating = Menu_FindRatingByChild(pmii->hSubMenu, pmi, pmii, childId);
				if (NULL != hRating) 
					return hRating;
			}
		}
	}
	return NULL;
}

static HMENU Menu_FindRatingByMarker(HMENU hMenu, MENUINFO *pmi, MENUITEMINFO *pmii)
{
	if (GetMenuInfo(hMenu, pmi) && RATING_MARKER == pmi->dwMenuData)
		return hMenu;

	INT count = GetMenuItemCount(hMenu);
	for(INT i = 0; i < count; i++)
	{
		if (GetMenuItemInfo(hMenu, i, TRUE, pmii) && NULL != pmii->hSubMenu)
		{								
			HMENU hRating = Menu_FindRatingByMarker(pmii->hSubMenu, pmi, pmii);
			if (NULL != hRating) 
				return hRating;
		}
	}
	return NULL;
}

HMENU Menu_FindRatingMenu(HMENU hMenu, BOOL fUseMarker)
{
	if (NULL == hMenu) 
		return NULL;

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;

	if (FALSE == fUseMarker)
		mii.fMask |= MIIM_ID;

	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;
	
	return (FALSE == fUseMarker) ? 
			Menu_FindRatingByChild(hMenu, &mi, &mii, ID_RATE_5) : 
			Menu_FindRatingByMarker(hMenu, &mi, &mii);
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

	for (int i=0;i<numItems;i++)
	{
		UINT id = GetMenuItemID(listMenu, i);
		if (id == ID_TRACKSLIST_PLAYSELECTION)
		{
			playItem.fMask = MIIM_ID;
			playPos = i;
			GetMenuItemInfoW(listMenu, i, TRUE, &playItem);
		}
		else if (id == ID_TRACKSLIST_ENQUEUESELECTION)
		{
			enqueueItem.fMask = MIIM_ID;
			enqueuePos= i;
			GetMenuItemInfoW(listMenu, i, TRUE, &enqueueItem);
		}
	}

	playItem.wID = ID_TRACKSLIST_ENQUEUESELECTION;
	enqueueItem.wID = ID_TRACKSLIST_PLAYSELECTION;		
	SetMenuItemInfoW(listMenu, playPos, TRUE, &playItem);
	SetMenuItemInfoW(listMenu, enqueuePos, TRUE, &enqueueItem);
}

INT Menu_TrackSkinnedPopup(HMENU hMenu, UINT fuFlags, INT x, INT y,  HWND hwnd, LPTPMPARAMS lptpm)
{
	if (NULL == hMenu)
		return NULL;

	bool swapPlayEnqueue=false;
	if (gen_mlconfig->ReadInt(L"enqueuedef", 0) == 1)
	{
		SwapPlayEnqueueInMenu(hMenu);
		swapPlayEnqueue = true;
	}

	SyncMenuWithAccelerators(hwndMediaView, hMenu);

	if (swapPlayEnqueue)
		SwapPlayEnqueueInMenu(hMenu);

	return Menu_TrackPopupParam(plugin.hwndLibraryParent, hMenu, fuFlags, x, y,
								hwnd, lptpm, (ULONG_PTR)Menu_FindRatingMenu(hMenu, TRUE));
}