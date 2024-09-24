#include "./menuHelpers.h"
#include <strsafe.h>

INT MenuHelper_CopyMenuEx(HMENU hDest, INT iDstStart, HMENU hSource, INT iSrcStart, INT iSrcCount)
{
	if (!hDest || !hSource) return 0;
	TCHAR szText[1024] = {0};

	MENUITEMINFO mii = {sizeof(MENUITEMINFO), };
	mii.fMask = MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_DATA | MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
	mii.dwTypeData = szText;

	if (iDstStart < 0) iDstStart = 0;
	if (iSrcStart < 0) iSrcStart = 0;

	INT pos = iDstStart;

	if ( 0 != iSrcCount)
	{
		INT c = GetMenuItemCount(hSource);
		if (-1 == c || iSrcStart > c) return 0;
		
		if (iSrcCount < 0) 
			iSrcCount = c - iSrcStart;
		else if (iSrcCount < (c - iSrcStart)) 
			c = iSrcCount + iSrcStart;

		for (int i = iSrcStart; i < c; i++) 
		{
			mii.cch = ARRAYSIZE(szText);
			if (GetMenuItemInfo(hSource, i, TRUE, &mii))
			{
				if(InsertMenuItem(hDest, pos, TRUE, &mii)) 
				{
					pos++;
				}
			}
		}
	}
	else
	{
		mii.cch = ARRAYSIZE(szText);
		if (GetMenuItemInfo(hSource, iSrcStart, FALSE, &mii))
		{
			if (InsertMenuItem(hDest, pos, TRUE, &mii))
			{
				pos++;
			}
		}
	}
	return pos - iDstStart;
}

INT MenuHelper_CopyMenu(HMENU hDest, INT iDstStart, HMENU hSource)
{
	return MenuHelper_CopyMenuEx(hDest, iDstStart, hSource, 0, -1);
}

HMENU MenuHelper_DuplcateMenu(HMENU hMenu)
{
	HMENU hDest = CreatePopupMenu();
	if (NULL == hMenu)
		return NULL;

	MenuHelper_CopyMenu(hDest, 0, hMenu);
	return hDest;
}

INT MenuHelper_InsertSeparator(HMENU hMenu, INT iPos)
{
	if (!hMenu) return FALSE;
	MENUITEMINFO mii = {sizeof(MENUITEMINFO), };
	mii.fMask = MIIM_FTYPE;
	mii.fType = MFT_SEPARATOR;
	return InsertMenuItem(hMenu, iPos, TRUE, &mii);
}

void MenuHelper_RemoveRange(HMENU hMenu, UINT min, UINT max)
{
	MENUITEMINFO mii = {sizeof(MENUITEMINFO), };
	mii.fMask = MIIM_ID;
    INT count = GetMenuItemCount(hMenu);
	if (-1 == count)
		return;
	while(count--)
	{
		if (GetMenuItemInfo(hMenu, count, TRUE, &mii) && 
			mii.wID >= min && mii.wID <= max)
		{
			RemoveMenu(hMenu, count, MF_BYPOSITION);
		}
	}
}

void MenuHelper_EnableGroup(HMENU hMenu, UINT *pszItems, INT cchItems, BOOL fByPos, BOOL bEnable)
{
	UINT enableGroup = (bEnable) ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);

	if (!fByPos)
		enableGroup |= MF_BYCOMMAND;

	for (INT i = 0; i < cchItems; i++)
	{
		EnableMenuItem(hMenu, pszItems[i], enableGroup);
	}
}

BOOL MenuHelper_GetMenuItemPos(HMENU hMenu, UINT itemId, INT *pPos)
{
	if (pPos)
		*pPos = -1;

	MENUITEMINFO mii = {sizeof(MENUITEMINFO), };
	mii.fMask = MIIM_ID;

	INT count = GetMenuItemCount(hMenu);
	if (count < 1)
		return FALSE;

	while(count--)
	{
		if (GetMenuItemInfo(hMenu, count, TRUE, &mii) && 
			mii.wID == itemId)
		{
			if (pPos)
				*pPos = count;
			return TRUE;
		}
	}

	return FALSE;
}