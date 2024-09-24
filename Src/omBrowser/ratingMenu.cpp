#include "main.h"
#include "./ratingMenu.h"
#include "./resource.h"
#include "../nu/menuHelpers.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedrating.h"
#include "./ifc_menucustomizer.h"

#define RATING_MARKER			 MAKELONG(MAKEWORD('R','A'),MAKEWORD('T','E'))

static BOOL RatingMenu_IsStar(HMENU hMenu, INT itemId, INT *valueOut)
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

static HMENU RatingMenu_FindMenuRecur(HMENU hMenu, MENUINFO *pmi, MENUITEMINFO *pmii)
{
	if (GetMenuInfo(hMenu, pmi) && RATING_MARKER == pmi->dwMenuData)
		return hMenu;

	INT count = GetMenuItemCount(hMenu);
	for(INT i = 0; i < count; i++)
	{
		if (GetMenuItemInfo(hMenu, i, TRUE, pmii) && NULL != pmii->hSubMenu)
		{								
			HMENU hRating = RatingMenu_FindMenuRecur(pmii->hSubMenu, pmi, pmii);
			if (NULL != hRating) 
				return hRating;
		}
	}
	return NULL;
}

HMENU RatingMenu_FindMenu(HMENU hMenu)
{
	if (NULL == hMenu) 
		return NULL;

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;

	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;

	return RatingMenu_FindMenuRecur(hMenu, &mi, &mii);
}

BOOL RatingMenu_InitializeMenu(HMENU ratingMenu, INT ratingValue)
{
	if (NULL == ratingMenu) return FALSE;
	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;
	mi.dwMenuData = RATING_MARKER;
	if (FALSE == SetMenuInfo(ratingMenu, &mi))
		return FALSE;

	return RatingMenu_SetValue(ratingMenu, ratingValue);
}

BOOL RatingMenu_SetValue(HMENU ratingMenu, INT ratingValue)
{
	if (NULL == ratingMenu) return FALSE;

	INT ratingList[] = { ID_RATING_VALUE_1, ID_RATING_VALUE_2, ID_RATING_VALUE_3, 
						ID_RATING_VALUE_4, ID_RATING_VALUE_5};
	ratingValue--;

	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	
	UINT type = 0, state = 0;
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

HRESULT RatingMenu_GetCustomizer(HMENU hMenu, ifc_menucustomizer **customizer)
{
	if (NULL == customizer) return E_POINTER;
	*customizer = NULL;
    
	HMENU ratingMenu = RatingMenu_FindMenu(hMenu);
	if (NULL == ratingMenu)
		return S_OK;
	
	ifc_skinhelper *skinHelper = NULL;
	HRESULT hr = Plugin_GetSkinHelper(&skinHelper);
	if (FAILED(hr) || skinHelper == NULL) return hr;

	ifc_skinnedrating *skinnedRating = NULL;
	hr = skinHelper->QueryInterface(IFC_SkinnedRating, (void**)&skinnedRating);
	if (SUCCEEDED(hr) && skinnedRating != NULL)
	{
		hr = skinnedRating->CreateMenuCustomizer(ratingMenu, customizer);
		skinnedRating->Release();
	}

	skinHelper->Release();
	return hr;
}