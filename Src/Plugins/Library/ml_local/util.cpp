#include "main.h"
#include "../replicant/nu/ns_wc.h"
#include "ml_local.h"
#include <string.h>
#include <shlobj.h>
#include "..\..\General\gen_ml/config.h"
#include "resource.h"

extern "C" {

void process_substantives(wchar_t* dest)
{
	if(!substantives)
	{
		wchar_t *b = dest;
		while (!IsCharSpaceW(*b) && *b) b++;
		while (IsCharSpaceW(*b) && *b) b++;
		while (!IsCharSpaceW(*b) && *b) b++;
		CharLowerW(b);
	}
}

HRESULT ResolveShortCut(HWND hwnd, LPCWSTR pszShortcutFile, LPWSTR pszPath)
{
	IShellLink* psl = 0;
	WIN32_FIND_DATA wfd = {0};

	*pszPath = 0;   // assume failure

	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
							IID_IShellLink, (void **) &psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf); // OLE 2!  Yay! --YO
		if (SUCCEEDED(hres))
		{
			wchar_t wsz[MAX_PATH] = {0};

			hres = ppf->Load(wsz, STGM_READ);
			if (SUCCEEDED(hres))
			{
				hres = psl->Resolve(hwnd, SLR_ANY_MATCH);
				if (SUCCEEDED(hres))
				{
					wchar_t szGotPath[MAX_PATH] = {0};
					wcsncpy(szGotPath, pszShortcutFile, MAX_PATH);
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH );
					wcsncpy(pszPath, szGotPath, MAX_PATH);
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	return SUCCEEDED(hres);
}

void ConvertRatingMenuStar(HMENU menu, UINT menu_id)
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

};