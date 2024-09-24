#include "./menushortcuts.h"
#include <strsafe.h>

// change key string to capitalised only so it's more UI consistant (otherwise messes up the localisation look)
wchar_t* CapitaliseKeyName(wchar_t* szAccelName){
	if(szAccelName) {
		wchar_t *p = &szAccelName[1];
		int space = 0;
		while(p && *p){
			if(*p == L' ') space = 1;
			else {
				if(!space) {
					*p = tolower(*p);
				}
				else {
					space = 0;
				}
			}
			p = CharNextW(p);
		}
	}
	return szAccelName;
}

BOOL AppendShortcutTextEx(LPWSTR pszItemText, INT cchTextMax, WORD wID, ACCEL *pszAccel, INT cchAccel, UINT uMode) 
{
	BOOL bDirty = FALSE;
	if (!pszItemText) return FALSE;

	UINT cchLen = lstrlenW(pszItemText);

	if (MSF_REPLACE == (0x0F & uMode))
	{
		UINT len;
		for (len = 0; len < cchLen && L'\t' != pszItemText[len]; len++);
		if (cchLen != len)
		{
			cchLen = len;
			pszItemText[len] = L'\0';
			bDirty = TRUE;
		}
	}

	if (wID > 0)
	{
		wchar_t szAccelName[64] = {0};
		BOOL bFirst = TRUE;
		pszItemText += cchLen;
		size_t cchText = cchTextMax - cchLen;

		for(int k = 0; k < cchAccel; k++)
		{
			if (wID == pszAccel[k].cmd)
			{
				HRESULT hr = StringCchCopyExW(pszItemText, cchText, ((bFirst) ? L"\t" : L", "),  &pszItemText, &cchText, STRSAFE_IGNORE_NULLS);
				if (S_OK == hr && (FCONTROL & pszAccel[k].fVirt)) {
					GetKeyNameTextW(MapVirtualKey(VK_CONTROL, 0)<<16, szAccelName, sizeof(szAccelName)/sizeof(szAccelName[0]));
					hr = StringCchCopyExW(pszItemText, cchText, CapitaliseKeyName(szAccelName), &pszItemText, &cchText, STRSAFE_IGNORE_NULLS);
					if(S_OK == hr) StringCchCatExW(pszItemText, cchText, L"+", &pszItemText, &cchText, STRSAFE_IGNORE_NULLS);
				}
				if (S_OK == hr && (FALT & pszAccel[k].fVirt)) {
					GetKeyNameTextW(MapVirtualKey(VK_MENU, 0)<<16, szAccelName, sizeof(szAccelName)/sizeof(szAccelName[0]));
					hr = StringCchCopyExW(pszItemText, cchText, CapitaliseKeyName(szAccelName), &pszItemText, &cchText, STRSAFE_IGNORE_NULLS);
					if(S_OK == hr) hr = StringCchCatExW(pszItemText, cchText, L"+", &pszItemText, &cchText, STRSAFE_IGNORE_NULLS);
				}
				if (S_OK == hr && (FSHIFT & pszAccel[k].fVirt)) {
					GetKeyNameTextW(MapVirtualKey(VK_SHIFT, 0)<<16, szAccelName, sizeof(szAccelName)/sizeof(szAccelName[0]));
					hr = StringCchCopyExW(pszItemText, cchText, CapitaliseKeyName(szAccelName), &pszItemText, &cchText, STRSAFE_IGNORE_NULLS);
					if(S_OK == hr) hr = StringCchCatExW(pszItemText, cchText, L"+", &pszItemText, &cchText, STRSAFE_IGNORE_NULLS);
				}
				if (S_OK == hr)
				{
					if (FVIRTKEY & pszAccel[k].fVirt)
					{
						szAccelName[0] = L'\0';
						UINT vkey = MapVirtualKey(pszAccel[k].key, 0);

						/* benski> this removes "NUM" from the descriptions of certain characters */
						switch(pszAccel[k].key)
						{
						case VK_INSERT:
						case VK_DELETE:
						case VK_HOME:
						case VK_END:
						case VK_NEXT:  // Page down
						case VK_PRIOR: // Page up
						case VK_LEFT:
						case VK_RIGHT:
						case VK_UP:
						case VK_DOWN:
							vkey |= 0x100; // Add extended bit
						}
						vkey<<=16;
						if (GetKeyNameTextW(vkey, szAccelName, sizeof(szAccelName)/sizeof(szAccelName[0]))) {
							hr = StringCchCopyExW(pszItemText, cchText, CapitaliseKeyName(szAccelName), &pszItemText, &cchText, STRSAFE_IGNORE_NULLS);
						}
					}
					else if (cchText > 1) 
					{
						pszItemText[0] = (wchar_t)pszAccel[k].key;
						pszItemText[1] = L'\0';
						pszItemText += 2;
						cchText -= 2;
					}
				}

				bFirst = FALSE;
				bDirty = (S_OK == hr);
			}
		}
	}
	return bDirty;
}

BOOL AppendMenuShortcutsEx(HMENU hMenu, ACCEL *pszAccel, INT cchAccel, UINT uMode)
{
	wchar_t szText[4096] = {0};
	if (!hMenu) return FALSE;

	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW), };

	INT c = GetMenuItemCount(hMenu);
	if (0 == c || -1 == c) return TRUE;

	for (int i = 0; i < c; i++)
	{
		mii.fMask = MIIM_ID | MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU;
		mii.cch = sizeof(szText)/sizeof(szText[0]);
		mii.dwTypeData = szText;
		
		if (GetMenuItemInfoW(hMenu, i, TRUE, &mii) && 
			0 == ((MFT_SEPARATOR | MFT_MENUBREAK |MFT_MENUBARBREAK) & mii.fType))
		{
			if (AppendShortcutTextEx(mii.dwTypeData, sizeof(szText)/sizeof(szText[0]), (WORD)mii.wID, pszAccel, cchAccel, uMode))
			{
				mii.fMask = MIIM_STRING;
				SetMenuItemInfoW(hMenu, i, TRUE, &mii);
			}
			if (NULL != mii.hSubMenu && (MSF_WALKSUBMENU & uMode))
			{
				AppendMenuShortcutsEx(mii.hSubMenu, pszAccel, cchAccel, uMode);	
			}
		}
	}
	return TRUE;
}

BOOL AppendMenuShortcuts(HMENU hMenu, HACCEL *phAccel, INT count, UINT uMode)
{
	ACCEL *pszAccel = NULL;
	INT c = 0;

	if (!hMenu) return FALSE;

	if (phAccel)
	{
		for (int i = 0; i < count; i++)
		{
			c += CopyAcceleratorTable(phAccel[i], NULL, 0);
		}
		if (c)
		{
			pszAccel = (ACCEL*)calloc(c, sizeof(ACCEL));
			if (!pszAccel) return FALSE;
		}
		int k = 0;
		for (int i = 0; i < count; i++)
		{
			k += CopyAcceleratorTable(phAccel[i], &pszAccel[k], c - k);
		}
	}
	BOOL r = AppendMenuShortcutsEx(hMenu, pszAccel, c, uMode);
	if (pszAccel) free(pszAccel);
	return r;
}

BOOL AppendShortcutText(LPWSTR pszItemText, INT cchTextMax, WORD wID, HACCEL *phAccel, INT count, UINT uMode)
{
	ACCEL *pszAccel = NULL;
	INT c = 0;

	if (!pszItemText) return FALSE;

	if (phAccel)
	{
		for (int i = 0; i < count; i++)
		{
			c += CopyAcceleratorTable(phAccel[i], NULL, 0);
		}
		if (c)
		{
			pszAccel = (ACCEL*)calloc(c, sizeof(ACCEL));
			if (!pszAccel) return FALSE;
		}
		int k = 0;
		for (int i = 0; i < count; i++)
		{
			k += CopyAcceleratorTable(phAccel[i], &pszAccel[k], c - k);
		}
	}
	BOOL r = AppendShortcutTextEx(pszItemText, cchTextMax, wID, pszAccel, c, uMode);
	if (pszAccel) free(pszAccel);
	return r;
}