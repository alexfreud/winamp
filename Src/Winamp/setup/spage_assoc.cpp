#define APSTUDIO_READONLY_SYMBOLS
#include "main.h"
#include "api.h"
#include "./spage_assoc.h"
#include "./setup_resource.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoWide.h"
#include "./langutil.h"
#include "./setupcommon.h"
#include "../playlist/svc_playlisthandler.h"
#include <api/service/waservicefactorybase.h>
#include "../Agave/URIHandler/svc_urihandler.h"
#include <api/service/waservicefactory.h>
#include <commctrl.h>

#define MF_SELECTED			0x0001
#define MF_TYPE_MASK		0xFF00
#define MF_TYPE_REREAD		0xFF
#define MF_TYPE_UNKNOWN		0x00
#define MF_TYPE_AUDIO		0x01
#define MF_TYPE_VIDEO		0x02
#define MF_TYPE_PLAYLIST	0x03
#define MF_TYPE_AUXILIARY	0x04

#define ID_REGISTERCD		((TYPE_CATEGORIES_NUM) + 1)
#define ID_REGISTERAGENT	((TYPE_CATEGORIES_NUM) + 2)

#define SET_TYPE(_val, _type)  ((_val) = ((_val) & ~MF_TYPE_MASK) | ((_type) << 8))
#define GET_TYPE(_val)  ((_val) >> 8)
#define IS_NEEDREREAD(_val) GET_TYPE((_val), MF_TYPE_REREAD)
#define IS_SELECTED(_val) (MF_SELECTED & (_val))
#define INITMETA( _type, _selected)  ((WORD)(((_type) << 8) | ((_selected) ? MF_SELECTED : 0x0000)))

static wchar_t szAuxExt[] = L"wsz\0wal\0wlz\0";

static LRESULT WINAPI TreeViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BOOL IsAgentScheduled(void)
{
	HKEY hKey = NULL;
	BOOL bActive(FALSE);
	WCHAR szAgent[MAX_PATH*2] = {0};
	DWORD cb = sizeof(szAgent);

	if (ERROR_SUCCESS != RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey))
		return FALSE;
	if (ERROR_SUCCESS != RegQueryValueExW(hKey, L"WinampAgent", NULL, NULL, (BYTE*)szAgent, &cb))
		szAgent[0] = 0x00;
	RegCloseKey(hKey);

	if (*szAgent)
	{
		WCHAR szPath[MAX_PATH*2] = {0};
		GetModuleFileNameW(NULL, szPath, sizeof(szPath)/sizeof(wchar_t));
		PathUnquoteSpacesW(szAgent);
		PathRemoveFileSpecW(szAgent);
		PathRemoveFileSpecW(szPath);
		DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		bActive = (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, szPath, -1, szAgent, -1));
	}

	return bActive;
}

static BOOL IsAgentExist(void)
{
	wchar_t szAgent[MAX_PATH] = {0};
	if (0 == GetModuleFileNameW(hMainInstance, szAgent, sizeof(szAgent)/sizeof(wchar_t)))
		return FALSE;

	PathRemoveFileSpecW(szAgent);
	if (NULL == PathCombineW(szAgent, szAgent, L"winampa.exe"))
		return FALSE;

	return (FALSE != PathFileExistsW(szAgent));
}

static BOOL RefreshIcons(void)
{
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, NULL, NULL);
	return TRUE;
}

static void RegisterProtocols()
{
	if (config_no_registry)
		return;

	wchar_t winampexe[MAX_PATH] = {0};
	GetModuleFileNameW(hMainInstance, winampexe, MAX_PATH);

	WCHAR szApplication[256] = {0};
	INT r = MultiByteToWideCharSZ(CP_ACP, 0, app_name, -1, NULL, 0);
	if (r > ARRAYSIZE(szApplication) || 0 == r) 
		return;
	if (0 == MultiByteToWideCharSZ(CP_ACP, 0, app_name, -1, szApplication, ARRAYSIZE(szApplication)))
		return;

	if (NULL != WASABI_API_SVC)
	{
		for(size_t i =0;;i++)
		{
			waServiceFactory *sf = WASABI_API_SVC->service_enumService(svc_urihandler::getServiceType(), i);
			if (NULL == sf) break;

			svc_urihandler *handler = (svc_urihandler *)sf->getInterface();
			if (NULL != handler)
			{
				WCHAR szName[128] = {0}, szDesc[256] = {0};
				for (size_t k = 0; ;k++)
				{
					INT ret = handler->EnumProtocols(k, szName, ARRAYSIZE(szName), szDesc, ARRAYSIZE(szDesc));
					if (0 != ret)
						break;

					if (0 == handler->RegisterProtocol(szName, winampexe))
					{
						IFileTypeRegistrar *registrar = 0;
						if (GetRegistrar(&registrar, true) == 0 && registrar)
						{
							registrar->RegisterMediaPlayerProtocol(szName, szApplication);
							registrar->Release();
						}
					}
				}
				sf->releaseInterface(handler);
			}
		}
	}
}

setup_page_assoc::setup_page_assoc() : ref(1), hwnd(NULL), pszTypes(NULL), pMeta(NULL), bRegCD(FALSE), bAgent(FALSE), bExplorerMenu(TRUE)
{
	ZeroMemory(expanded, sizeof(expanded));
	ZeroMemory(szTopExt, sizeof(szTopExt));
	ZeroMemory(szCaretExt, sizeof(szCaretExt));
}

setup_page_assoc::~setup_page_assoc()
{
	if (pszTypes)
	{
		free(pszTypes);
		pszTypes = NULL;
	}
	if (pMeta)
	{
		free(pMeta);
		pMeta = NULL;
	}
}

size_t setup_page_assoc::AddRef()
{
	return ++ref;
}

size_t setup_page_assoc::Release()
{
	if (1 == ref) 
	{
		delete(this);
		return 0;
	}
	return --ref;
}

HRESULT setup_page_assoc::GetName(bool bShort, const wchar_t **pszName)
{
	if (bShort)
	{
		static wchar_t szShortName[32] = {0};
		*pszName = (*szShortName) ? szShortName : getStringW(IDS_PAGE_ASSOCIATIONS, szShortName, sizeof(szShortName)/sizeof(wchar_t));
	}
	else
	{
		static wchar_t szLongName[64] = {0};
		*pszName = (*szLongName) ? szLongName : getStringW(IDS_PAGE_ASSOCIATIONS_LONG, szLongName, sizeof(szLongName)/sizeof(wchar_t));
	}
	return S_OK;
}

HRESULT setup_page_assoc::Save(HWND hwndText)
{
	HRESULT hr(S_OK);
	WORD *pm;
	wchar_t ext_list[16384] = {0}, *p = 0, *pe = ext_list;
	BOOL bFirst(TRUE);
	size_t len = ARRAYSIZE(ext_list);

	// make sure that we honour the agent setting even if no settings changed
	// this allows the agent to be restarted correctly after a normal upgrade
	if (bAgent && IsAgentExist()) 
		config_agent_add(); 
	else
		config_agent_remove();

	// temporary: always enumerate and register protocols
	RegisterProtocols();

	if (S_FALSE == IsDirty()) return S_OK;

	if (!pszTypes) return S_FALSE;

	config_setup_filetypes(0);

	for(pm = pMeta, p = pszTypes; *p != 0; p += lstrlenW(p) + 1, pm++)
	{
		config_register_capability(p, 0);
		config_register(p, IS_SELECTED(*pm));
		if (IS_SELECTED(*pm) && (S_OK == hr) && GET_TYPE(*pm) != MF_TYPE_AUXILIARY)
		{
			if (!len) { hr = S_FALSE; continue; }
			if (!bFirst)
			{
				pe[0] = L':';
				pe++;
				len--;
			}
			else bFirst = FALSE;
			if (S_OK != StringCchCopyExW(pe, len, p, &pe, &len, STRSAFE_IGNORE_NULLS)) hr = S_FALSE;
		}
	}
	if (S_OK == hr) _w_sW("config_extlist", ext_list);

	config_regcdplayer(bRegCD, 0);
	(bExplorerMenu) ? config_adddircontext(0) : config_removedircontext(0);

	config_registermediaplayer(1);

	WritePrivateProfileStringW(L"Jump To File Extra", L"newIconLib", L"refresh", INI_FILE);
	RefreshIcons();

	return hr;
}

static BOOL IsFirstSetup()
{
	wchar_t szVer[512] = {0};
	if (0 == GetPrivateProfileIntW(L"WinampReg", L"IsFirstInst", 1, INI_FILE)) return FALSE;

	GetPrivateProfileStringW(L"Winamp", L"config_extlist", L"", szVer, 512, INI_FILE);
	if (*szVer) return FALSE;

	GetPrivateProfileStringW(L"WinampReg", L"WAVer", L"", szVer, 512, INI_FILE);
	return (!*szVer || CSTR_EQUAL != CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
													NORM_IGNORECASE, szVer, -1, AutoWide(APP_VERSION), -1));
}

static wchar_t *BuildExtensionString(WORD **ppMeta, BOOL bFirstSetup)
{
	INT cWA(0), cPL(0), cAux(0), lWA(0), lPL(0), lAux(0);
	wchar_t *pszType, *pWAExt = in_getextlistW(), *pPLExt, *p;

	if (pWAExt)
	{
		for(p = pWAExt; *p != 0; cWA++, p += lstrlenW(p) + 1);
		lWA = (INT)(p - pWAExt);
	}

   	if (playlistManager) 
	{
		size_t playlistEnum = 0;
		const wchar_t *playlistExt=0;
		while (NULL != (playlistExt = playlistManager->EnumExtension(playlistEnum++))) { lPL += (lstrlenW(playlistExt) + 1); cPL++; }
		lPL += 2;
	}

	for(p = szAuxExt; *p != 0; cAux++, p += lstrlenW(p) + 1);
	lAux = (INT)(p - szAuxExt);

	pszType = (wchar_t*)calloc((lWA + lPL + lAux + 1), sizeof(wchar_t));
	if (ppMeta)
	{
		*ppMeta = (WORD*)calloc((cWA + cPL + cAux), sizeof(WORD));
	}

	if (pszType)
	{
		if (pWAExt) CopyMemory(pszType, pWAExt, lWA*sizeof(wchar_t));
		pPLExt = pszType + lWA;
		p = pPLExt;
		if (playlistManager) 
		{
			size_t playlistEnum=0;
			const wchar_t *playlistExt=0;
			while (lPL > 0 && NULL != (playlistExt = playlistManager->EnumExtension(playlistEnum++)))
			{
				int c = lstrlenW(playlistExt) + 1;
				lstrcpynW(p, playlistExt, c);
				if (c)
				{
					p += c;
					lPL -= c;
				}
			}
			if (lPL > 1) *p = 0x00;
		}
		CopyMemory(p, szAuxExt, lAux*sizeof(wchar_t));
		*(p+lAux)=0;
	}

	if(ppMeta && *ppMeta)
	{
		int i;
		WORD *pm = *ppMeta;
		p = pszType;
		for (i = 0; i < cWA; i++, pm++, p += lstrlenW(p) + 1) *pm = INITMETA(MF_TYPE_REREAD, ((!bFirstSetup) ? config_isregistered(p) : 1));
		for (i = 0; i < cPL; i++, pm++, p += lstrlenW(p) + 1) *pm = INITMETA(MF_TYPE_PLAYLIST, ((!bFirstSetup) ? config_isregistered(p) : 1));
		for (i = 0; i < cAux; i++, pm++, p += lstrlenW(p) + 1) *pm = INITMETA(MF_TYPE_AUXILIARY, ((!bFirstSetup) ? config_isregistered(p) : 1));
	}

	if (pWAExt) GlobalFree(pWAExt);

	return pszType;
}

static BOOL GetPLExtensionName(LPCWSTR pszExt, LPWSTR pszDest, INT cchDest)
{	
	BOOL result(FALSE);
	int n(0);
    waServiceFactory *sf = 0;
	LPCWSTR ext;
	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

	while (NULL != (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n++)))
	{
		svc_playlisthandler * handler = static_cast<svc_playlisthandler *>(sf->getInterface());
		if (handler)
		{
			int k(0);
			while (NULL != (ext = handler->EnumerateExtensions(k++)))
			{
				if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszExt, -1, ext, -1))
				{
					result = (S_OK == StringCchCopyW(pszDest, cchDest, handler->GetName()));
					if (result && CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszExt, -1, L"M3U8", -1)) // ugly...
							result = (S_OK == StringCchCatW(pszDest, cchDest, L" (Unicode)"));
					break;
				}
			}
			sf->releaseInterface(handler);
		}
	}
	return result;
}

static BOOL GetAuxExtensionName(LPCWSTR pszExt, LPWSTR pszDest, INT cchDest)
{
	BOOL result(FALSE);
	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszExt, -1, L"wal", -1))
		result = (S_OK == StringCchCopyW(pszDest, cchDest, getStringW(IDS_WINAMP_SKIN_MODERN, NULL, 0)));
	else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszExt, -1, L"wsz", -1))
		result = (S_OK == StringCchCopyW(pszDest, cchDest, getStringW(IDS_WINAMP_SKIN_CLASSIC, NULL, 0)));
	else if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszExt, -1, L"wlz", -1))
		result = (S_OK == StringCchCopyW(pszDest, cchDest, getStringW(IDS_WINAMP_LANG_PACK, NULL, 0)));
	return result;
}

static BOOL GetExtensionName(LPCWSTR pszFile, INT type, LPWSTR pszDest, INT cchDest)
{
	switch(type)
	{
		case MF_TYPE_AUDIO:
		case MF_TYPE_VIDEO:
			return in_get_extended_fileinfoW(pszFile, L"family", pszDest, cchDest);
		case MF_TYPE_PLAYLIST:
		{
			const wchar_t *pszExt = PathFindExtensionW(pszFile);
			return (L'.' == *pszExt && 0x00 != *(++pszExt)) ? GetPLExtensionName(pszExt, pszDest, cchDest) : FALSE;
		}
		case MF_TYPE_AUXILIARY:
		{
			const wchar_t *pszExt = PathFindExtensionW(pszFile);
			return (L'.' == *pszExt && 0x00 != *(++pszExt)) ? GetAuxExtensionName(pszExt, pszDest, cchDest) : FALSE;
		}
	}		
	return FALSE;
}

HRESULT setup_page_assoc::Revert(void)
{
	HRESULT hr(S_OK);

	if (pszTypes)
	{
		free(pszTypes);
		pszTypes = NULL;
	}

	if (pMeta)
	{
		free(pMeta);
		pMeta = NULL;
	}

	BOOL firstSetup = IsFirstSetup();
    pszTypes = BuildExtensionString(&pMeta, firstSetup);

	ZeroMemory(expanded, sizeof(expanded));
	ZeroMemory(szTopExt, sizeof(szTopExt));
	ZeroMemory(szCaretExt, sizeof(szCaretExt));

	bRegCD = (firstSetup) ? TRUE : config_iscdplayer();
	bAgent = (FALSE != IsAgentExist()) ? ((firstSetup) ? FALSE : IsAgentScheduled()) : FALSE;
	bExplorerMenu = (firstSetup) ? TRUE : config_isdircontext();

	if (hwnd) UpdateUI();
	return hr;
}

HRESULT setup_page_assoc::IsDirty(void)
{
	if (IsFirstSetup()) return S_OK;

	HRESULT hr(S_FALSE);
	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	wchar_t *pszOrigTypes = BuildExtensionString(NULL, TRUE), *p;
	if ((!pszOrigTypes && pszTypes) || (pszOrigTypes && !pszTypes)) 
	{
		if (pszOrigTypes) free(pszOrigTypes);
		return S_OK;
	}
	else if (!pszOrigTypes && !pszTypes) return S_FALSE;

	INT cr = CompareStringW(lcid, NORM_IGNORECASE, pszTypes, -1, pszOrigTypes, -1);
	if (0 == cr) hr = E_UNEXPECTED;
	else if (CSTR_EQUAL == cr)
	{
		WORD *pm;
		for(pm = pMeta, p = pszTypes; *p != 0; p += lstrlenW(p) + 1, pm++)
		{
			if ((MF_SELECTED & *pm) != (BYTE)config_isregistered(p)) { hr = S_OK; break; }
		}
	}
	else hr = S_OK;

	if (pszOrigTypes) free(pszOrigTypes);

	if (S_FALSE == hr && bRegCD != config_iscdplayer()) hr = S_OK;
	if (S_FALSE == hr && bAgent != (IsAgentExist() && IsAgentScheduled())) hr = S_OK;
	if (S_FALSE == hr && bExplorerMenu != config_isdircontext()) hr = S_OK;

	return hr;
}

HRESULT setup_page_assoc::Validate(void)
{
	return S_OK;
}

HRESULT setup_page_assoc::CreateView(HWND hwndParent, HWND *phwnd)
{
	*phwnd = WACreateDialogParam(MAKEINTRESOURCEW((!IsWin8() ? IDD_SETUP_PAGE_ASSOC : IDD_SETUP_PAGE_ASSOC_WIN8)), hwndParent, ::DialogProc, (LPARAM)this);
	return S_OK;
}

void setup_page_assoc::UpdateUI(void)
{
	TVINSERTSTRUCTW is = {0};
	WORD *pm;
	HTREEITEM hBranch[TYPE_CATEGORIES_NUM], hFirst(NULL), hCaret(NULL), hItem;
	INT sBranch[TYPE_CATEGORIES_NUM] = {0};
	INT ids[TYPE_CATEGORIES_NUM] = {IDS_FILETYPE_UNKNOWN, IDS_FILETYPE_AUDIO, IDS_FILETYPE_VIDEO, IDS_FILETYPE_PLAYLIST, IDS_FILETYPE_AUXILIARY};
    INT i;

	wchar_t szText[MAX_PATH] = {0}, buf[MAX_PATH] = {0}, buf2[MAX_PATH] = {0}, *p, *test;
	if (!hwnd || !IsWindow(hwnd)) return;

	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

	HWND hwndTree = GetDlgItem(hwnd,IDC_TREE_TYPES);
	SendMessageW(hwndTree, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);
	if (!pszTypes) return;

	for (i = 0; i < ARRAYSIZE(hBranch); i++)
	{
		hBranch[i] = NULL;
		sBranch[i] = -1;
	}

	INT len = ARRAYSIZE(buf);
	StringCchCopyW(buf, len, L"test.");
	test = buf + lstrlenW(buf);
	len -= (INT)(test  - buf);

	SendMessageW(hwndTree, WM_SETREDRAW, FALSE, 0L);

	is.hInsertAfter = TVI_LAST;
	is.item.mask = TVIF_STATE | TVIF_CHILDREN | TVIF_TEXT | TVIF_PARAM;
	is.item.stateMask = TVIS_STATEIMAGEMASK | TVIS_EXPANDED;
	for(i = 0, pm = pMeta, p = pszTypes; *p != 0 && *(p+1) != 0; pm++, p += lstrlenW(p) + 1, i++)
	{
		StringCchCopyW(test, len, p);
		OutputDebugStringW(test);
		OutputDebugStringW(L"\r\n");
		if (MF_TYPE_REREAD == GET_TYPE(*pm))
		{
			if (!in_get_extended_fileinfoW(buf, L"type", buf2, MAX_PATH))
				ZeroMemory(buf2, sizeof(buf2));

			*pm = *pm & 0x00FF;
			switch(buf2[0])
			{
				case L'0': SET_TYPE(*pm, MF_TYPE_AUDIO); break;
				case L'1': SET_TYPE(*pm, MF_TYPE_VIDEO); break;
			}
		}

		INT index = GET_TYPE(*pm);
		if (!hBranch[index])
		{
			is.hInsertAfter = TVI_SORT;
			is.hParent = TVI_ROOT;
			is.item.cChildren = 1;
			is.item.state = INDEXTOSTATEIMAGEMASK(1) | ((expanded[index]) ? TVIS_EXPANDED : 0);
			is.item.pszText = getStringW(ids[index], NULL, 0);
			is.item.lParam = -(index + 1);
			hBranch[index] = (HTREEITEM)SendMessageW(hwndTree, TVM_INSERTITEMW, 0, (LPARAM)&is);
			is.item.cChildren = 0;
			is.hInsertAfter = TVI_LAST;

			wchar_t t[32] = {0};
			StringCchPrintfW(t, ARRAYSIZE(t), L"#%d", index + 1);
			if (!hFirst && CSTR_EQUAL == CompareStringW(lcid, 0, t, -1, szTopExt, -1)) hFirst = hBranch[index];
			if (!hCaret && CSTR_EQUAL == CompareStringW(lcid, 0, t, -1, szCaretExt, -1)) hCaret = hBranch[index];
		}

		is.hParent = hBranch[index];

		if (MF_SELECTED & *pm) 
		{
			if (-1 == sBranch[index]) sBranch[index] = 1;
			else if (0 == sBranch[index])  sBranch[index] = 2;
		}
		else 
		{ 
			if (-1 == sBranch[index]) sBranch[index] = 0;
			else if (1 == sBranch[index])  sBranch[index] = 2;
		}

		lstrcpynW(szText, p, MAX_PATH);
		INT count = lstrlenW(szText);
		CharUpperW(szText);
		wchar_t szName[MAX_PATH] = {0};
		if (GetExtensionName(buf, GET_TYPE(*pm), szName, MAX_PATH))
		{
			if (count) StringCchCatW(szText, MAX_PATH, L"\t");
			StringCchCatW(szText, MAX_PATH, szName);
		}

		is.item.pszText = szText;
		is.item.lParam = (LPARAM)i;
		is.item.state = INDEXTOSTATEIMAGEMASK((MF_SELECTED & *pm)?2:1);

		hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_INSERTITEMW, 0, (LPARAM)&is);

		if (hItem)
		{
			if (!hFirst && CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, p, -1, szTopExt, -1)) hFirst = hItem;
			if (!hCaret && CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, p, -1, szCaretExt, -1)) hCaret = hItem;
		}
	}

	// insert cd 
	is.hParent = TVI_ROOT;
	is.item.state = INDEXTOSTATEIMAGEMASK(bRegCD + 1);
	is.item.pszText = getStringW(IDS_REGISTER_CDPLAYER, NULL, 0);
	is.item.lParam = -ID_REGISTERCD;
	hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_INSERTITEMW, 0, (LPARAM)&is);
	if (!hFirst || !hCaret)
	{
		wchar_t t[32] = {0};
		StringCchPrintfW(t, ARRAYSIZE(t), L"#%d", ID_REGISTERCD);
		if (!hFirst && CSTR_EQUAL == CompareStringW(lcid, 0, t, -1, szTopExt, -1)) hFirst = hItem;
		if (!hCaret && CSTR_EQUAL == CompareStringW(lcid, 0, t, -1, szCaretExt, -1)) hCaret = hItem;
	}

	// agent
	BOOL bAgentExist = IsAgentExist();
	HWND hwndCtrl = GetDlgItem(hwnd, IDC_CHK_AGENT);
	if (hwndCtrl) EnableWindow(hwndCtrl, bAgentExist);
	hwndCtrl = GetDlgItem(hwnd, IDC_LBL_AGENT_DESC);
	if (hwndCtrl) EnableWindow(hwndCtrl, bAgentExist);

	CheckDlgButton(hwnd, IDC_CHK_AGENT, (bAgent && bAgentExist) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHK_CD, (bRegCD) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHK_EXPLORER_MENU, (bExplorerMenu) ? BST_CHECKED : BST_UNCHECKED);

	is.item.mask = TVIF_STATE;
	is.item.stateMask = TVIS_STATEIMAGEMASK;

	for (int i = 0; i <  sizeof(hBranch)/sizeof(HTREEITEM); i++)
	{
		if (!hBranch[i]) continue;
		is.item.hItem = hBranch[i];
		is.item.state = INDEXTOSTATEIMAGEMASK(sBranch[i] + 1);
		SendMessageW(hwndTree, TVM_SETITEM, 0, (LPARAM)&is.item);
		SendMessageW(hwndTree, TVM_SORTCHILDREN, FALSE, (LPARAM)hBranch[i]);
	}

	if (hCaret)  PostMessageW(hwndTree, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hCaret);
	if (hFirst) PostMessageW(hwndTree, TVM_SELECTITEM, TVGN_FIRSTVISIBLE, (LPARAM)hFirst);

	SendMessageW(hwndTree, WM_SETREDRAW, TRUE, 0L);
}

INT setup_page_assoc::TreeView_OnCustomDraw(NMTVCUSTOMDRAW *ptvcd)
{
	switch(ptvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			return CDRF_DODEFAULT | CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
			return CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
		case CDDS_ITEMPOSTPAINT:
			{
				RECT rt;
				WCHAR szText[256] = {0};
				TVITEMW item = {0};

				item.hItem = (HTREEITEM)ptvcd->nmcd.dwItemSpec;
				item.mask = TVIF_TEXT | TVIF_STATE;
				item.stateMask = TVIS_SELECTED;
				item.pszText = szText;
				item.cchTextMax = 256;
				SendMessageW(ptvcd->nmcd.hdr.hwndFrom, TVM_GETITEMW, 0, (LPARAM)&item);
				*(DWORD_PTR*)&rt = ptvcd->nmcd.dwItemSpec;
				SendMessageW(ptvcd->nmcd.hdr.hwndFrom, TVM_GETITEMRECT, TRUE, (LPARAM)&rt);
				SetTextColor(ptvcd->nmcd.hdc, ptvcd->clrText);
				SetBkColor(ptvcd->nmcd.hdc, ptvcd->clrTextBk);

				DrawTextW(ptvcd->nmcd.hdc, item.pszText, -1, &rt, DT_EXPANDTABS | DT_NOPREFIX | DT_SINGLELINE | DT_CALCRECT);
				rt.right += 8;
				rt.top = ptvcd->nmcd.rc.top;
				rt.bottom = ptvcd->nmcd.rc.bottom;

				ExtTextOutW(ptvcd->nmcd.hdc, 0, 0, ETO_OPAQUE, &rt, L"", 0, NULL);
				rt.left += 4;
				DrawTextW(ptvcd->nmcd.hdc, item.pszText, -1, &rt, DT_EXPANDTABS | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

				if ((TVIS_SELECTED & item.state) && ptvcd->nmcd.hdr.hwndFrom == GetFocus())
				{
					if (0 == (0x01/*UISF_HIDEFOCUS*/ & SendMessageW(ptvcd->nmcd.hdr.hwndFrom, 0x0129/*WM_QUERYUISTATE*/, 0, 0L)))
					{
						rt.left -= 4;
						SetTextColor(ptvcd->nmcd.hdc, GetSysColor(COLOR_WINDOWTEXT));
						SetBkColor(ptvcd->nmcd.hdc, GetSysColor(COLOR_WINDOW));
						DrawFocusRect(ptvcd->nmcd.hdc, &rt);
					}
				}
			}
			break;
	}
	return CDRF_DODEFAULT;
}

BOOL setup_page_assoc::TreeView_OnClick(NMHDR *pnmh)
{
	TVHITTESTINFO ht;
	GetCursorPos(&ht.pt);
	MapWindowPoints(HWND_DESKTOP, pnmh->hwndFrom, &ht.pt, 1);
	if(NULL != SendMessageW(pnmh->hwndFrom, TVM_HITTEST, 0, (LPARAM)&ht))
	{
		if ((TVHT_ONITEM | TVHT_ONITEMRIGHT) & ht.flags)
		{
			TreeView_OnItemStateClick(pnmh->hwndFrom, ht.hItem);
			if (TVHT_ONITEMSTATEICON & ht.flags) return TRUE;
		}
	}
	return FALSE;
}

BOOL setup_page_assoc::TreeView_OnKeyDown(NMTVKEYDOWN *ptvkd)
{
	switch(ptvkd->wVKey)
	{
		case VK_SPACE:
		{
			HTREEITEM hItem = (HTREEITEM)(HTREEITEM)SendMessageW(ptvkd->hdr.hwndFrom, TVM_GETNEXTITEM, (WPARAM)TVGN_CARET, 0L);
			if (hItem) TreeView_OnItemStateClick(ptvkd->hdr.hwndFrom, hItem);
			return TRUE;
		}
	}
	return FALSE;
}

void setup_page_assoc::TreeView_OnItemStateClick(HWND hwndTree, HTREEITEM hItem)
{
	HTREEITEM hParent = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_PARENT, (LPARAM)hItem);
	HTREEITEM hChild = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)hItem);
	TVITEMW item = {0};
	item.hItem = hItem;
	item.mask = TVIF_STATE | TVIF_PARAM;
	item.stateMask = TVIS_STATEIMAGEMASK;

	if (!SendMessageW(hwndTree, TVM_GETITEMW, 0, (LPARAM)&item)) return;
	INT state = ((item.state>>12) - 1);
	INT param = (INT)item.lParam;

	SendMessageW(hwndTree, WM_SETREDRAW, FALSE, 0L);

	state = (2 == state) ? 1 : !state;

	item.mask = TVIF_STATE;
	item.state = INDEXTOSTATEIMAGEMASK(state + 1);
	SendMessageW(hwndTree, TVM_SETITEMW, 0, (LPARAM)&item);

	if (!hChild)
	{
		INT count = 1, selcount = state;

		item.hItem = hItem;
		while(NULL != (item.hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_PREVIOUS, (LPARAM)item.hItem)))
		{
			count++;
			if (!SendMessageW(hwndTree, TVM_GETITEMW, 0, (LPARAM)&item)) continue;
			if (2 == (item.state>>12)) selcount++;
		}
		item.hItem = hItem;
		while(NULL != (item.hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item.hItem)))
		{
			count++;
			if (!SendMessageW(hwndTree, TVM_GETITEMW, 0, (LPARAM)&item)) continue;
			if (2 == (item.state>>12)) selcount++;
		}

		item.hItem = hParent;
		item.state = INDEXTOSTATEIMAGEMASK(((!selcount) ? 1 : ((selcount == count) ? 2 :3)));
		SendMessageW(hwndTree, TVM_SETITEMW, 0, (LPARAM)&item);
		if (param >= 0) pMeta[param] = (pMeta[param] & ~MF_SELECTED) | ((state) ? MF_SELECTED : 0);
		else
		{
			switch(-param)
			{
				case ID_REGISTERCD: bRegCD = state; break;
			}
		}
	}
	else
	{
		item.hItem = hChild;
		while(item.hItem)
		{
			item.mask = TVIF_PARAM;
			if (SendMessageW(hwndTree, TVM_GETITEMW, 0, (LPARAM)&item) && item.lParam >= 0) pMeta[item.lParam] = (pMeta[item.lParam] & ~MF_SELECTED) | ((state) ? MF_SELECTED : 0);

			item.mask = TVIF_STATE;
			item.state = INDEXTOSTATEIMAGEMASK(state + 1);
			SendMessageW(hwndTree, TVM_SETITEMW, 0, (LPARAM)&item);
			item.hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item.hItem);
		}
	}
	UpdateWindow(hwnd);
	SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0L);
	SendMessageW(hwndTree, WM_SETREDRAW, TRUE, 0L);
	SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0L);
	InvalidateRect(hwndTree, NULL, FALSE); 
}

INT_PTR setup_page_assoc::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	HWND hwndTree = GetDlgItem(hwnd, IDC_TREE_TYPES);
	if (hwndTree) 
	{
		HIMAGELIST himl = ImageList_LoadImage(hMainInstance, MAKEINTRESOURCE(IDB_CHECKBOX), 16, 1, CLR_NONE, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_LOADTRANSPARENT);
		if (himl) himl = (HIMAGELIST) SendMessageW(hwndTree, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)himl);
		if (himl) ImageList_Destroy(himl);

		WNDPROC fnOldProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwndTree, GWLP_WNDPROC, (LONGX86)(LONG_PTR)TreeViewProc);
		if (fnOldProc) SetPropW(hwndTree, L"TVPROC", fnOldProc);
	}
	UpdateUI();
	return 0;
}

void setup_page_assoc::OnDestroy(void)
{
	HWND hwndTree = GetDlgItem(hwnd, IDC_TREE_TYPES);
	if (hwndTree) 
	{
		TVITEMW item = {0};
		HIMAGELIST himl;
		INT index1(-1), index2(-1);
		wchar_t *p;

		ZeroMemory(expanded, sizeof(expanded));
		ZeroMemory(szTopExt, sizeof(szTopExt));
		ZeroMemory(szCaretExt, sizeof(szCaretExt));

		item.mask = TVIF_PARAM | TVIF_STATE;
		item.stateMask = TVIS_EXPANDED;
		item.hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_ROOT, 0L);
		while(item.hItem)
		{
			if (SendMessageW(hwndTree, TVM_GETITEMW, 0, (LPARAM)&item))
			{
				INT param = (INT)-item.lParam;
				if (param < TYPE_CATEGORIES_NUM) expanded[param] = (BYTE)(TVIS_EXPANDED & item.state);
			}
			item.hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item.hItem);
		}
		item.mask = TVIF_PARAM;
		item.hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_FIRSTVISIBLE, 0L);
		if (item.hItem && SendMessageW(hwndTree, TVM_GETITEMW, 0, (LPARAM)&item)) index1 = (INT)item.lParam;
		item.hItem = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_CARET, 0L);
		if (item.hItem && SendMessageW(hwndTree, TVM_GETITEMW, 0, (LPARAM)&item)) index2 = (INT)item.lParam;

		if (index1 < 0) StringCchPrintfW(szTopExt, ARRAYSIZE(szTopExt), L"#%d", -index1);
		if (index2 < 0) StringCchPrintfW(szCaretExt, ARRAYSIZE(szCaretExt), L"#%d", -index2);

		if (index1 >= 0 || index2 >= 0)
		{
			INT i;
			for(i = 0, p = pszTypes; *p != 0; p += lstrlenW(p) + 1, i++)
			{
				if (index1 == i) 
				{ 
					StringCchCopyW(szTopExt, ARRAYSIZE(szTopExt), p);
					if (index2 < 0) break;
					index1 = -1;
				}
				if (index2 == i) 
				{ 
					StringCchCopyW(szCaretExt, ARRAYSIZE(szCaretExt), p);
					if (index1 < 0) break;
					index2 = -1;
				}
			}
		}
		himl = (HIMAGELIST) SendMessageW(hwndTree, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)NULL);
		if (himl) ImageList_Destroy(himl);
	}
}

void setup_page_assoc::OnSize(UINT nType, INT cx, INT cy)
{
	RECT rw;
	INT h, r;
	h = cy;
	r = cx;

	HWND hwndCtrl = GetDlgItem(hwnd, IDC_LBL_HEADER);
	if (hwndCtrl)
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, 0, 0, cx - rw.left*2, rw.bottom  - rw.top, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}

	if (IsWin8())
	{
		// hide the treeview (as needed to process things nicely but needs to be hidden on Windows 8 and higher as they work differently)
		hwndCtrl = GetDlgItem(hwnd, IDC_TREE_TYPES);
		if (hwndCtrl)
		{
			SetWindowPos(hwndCtrl, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_HIDEWINDOW);
		}
		return;
	}

	hwndCtrl = GetDlgItem(hwnd, IDC_CHK_EXPLORER_MENU);
	if (hwndCtrl)
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		h = cy - (((rw.bottom - rw.top)*3) + 6);
		r  = max(0, (cx - (rw.right - rw.left))/2) + (rw.right - rw.left);
		SetWindowPos(hwndCtrl, NULL, r - (rw.right - rw.left) - 12,
					 h, rw.right - rw.left + 26,
					 rw.bottom  - rw.top, SWP_NOACTIVATE | SWP_NOZORDER);
		
	}
	hwndCtrl = GetDlgItem(hwnd, IDC_CHK_AGENT);
	if (hwndCtrl)
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, r - (rw.right - rw.left) - 12,
					 h + (rw.bottom  - rw.top) + 2,
					 rw.right - rw.left + 26,
					 rw.bottom  - rw.top, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	hwndCtrl = GetDlgItem(hwnd, IDC_LBL_AGENT_DESC);
	if (hwndCtrl)
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, r - (rw.right - rw.left) - 12,
					 h + (rw.bottom  - rw.top)*2 + 2,
					 rw.right - rw.left + 26,
					 rw.bottom  - rw.top, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	hwndCtrl = GetDlgItem(hwnd, IDC_TREE_TYPES);
	if (hwndCtrl)
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, r - (rw.right - rw.left) - 12,
					 rw.top, rw.right - rw.left + 26,
					 h - rw.top - 4, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void setup_page_assoc::OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl)
{
	switch(nCtrlID)
	{			
		case IDC_CHK_EXPLORER_MENU:
		{
			switch (nEvntID)
			{
				case BN_CLICKED: 
					bExplorerMenu = (BST_CHECKED == (BST_CHECKED & (INT)SendMessageW(hwndCtrl, BM_GETSTATE, 0, 0L)));
					break;
			}
			break;
		}
		case IDC_CHK_AGENT:
		{
			switch (nEvntID)
			{
				case BN_CLICKED: 
					bAgent = (BST_CHECKED == (BST_CHECKED & (INT)SendMessageW(hwndCtrl, BM_GETSTATE, 0, 0L)));
					break;
			}
			break;
		}
		case IDC_CHK_CD:
		{
			switch (nEvntID)
			{
				case BN_CLICKED:
					bRegCD = (BST_CHECKED == (BST_CHECKED & (INT)SendMessageW(hwndCtrl, BM_GETSTATE, 0, 0L)));
					break;
			}
			break;
		}
	}
}

BOOL setup_page_assoc::OnNotify(INT nCtrlID, NMHDR *pnmh, LRESULT *pResult)
{
	switch(nCtrlID)
	{
		case IDC_TREE_TYPES:
			switch(pnmh->code)
			{
				case NM_CUSTOMDRAW:	*pResult = TreeView_OnCustomDraw((NMTVCUSTOMDRAW*)pnmh); return TRUE;
				case NM_CLICK: *pResult = TreeView_OnClick(pnmh); return TRUE;
				case TVN_KEYDOWN: *pResult = TreeView_OnKeyDown((NMTVKEYDOWN*)pnmh); return TRUE;
			}
	}
	return FALSE;
}

INT_PTR setup_page_assoc::PageDlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return OnInitDialog((HWND)wParam, lParam);
		case WM_DESTROY:	OnDestroy(); break;
		case WM_SIZE:		OnSize((UINT)wParam, LOWORD(lParam), HIWORD(lParam)); break;
		case WM_COMMAND:	OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_NOTIFY:
			{
				LRESULT result = 0;
				if (OnNotify((INT)wParam, (NMHDR*)lParam, &result))
				{
					SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, (LONGX86)(LONG_PTR)result);
					return TRUE;
				}
			}
			break;
	}
	return 0;
}

static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	setup_page_assoc *pInst = (setup_page_assoc*)GetPropW(hwnd, L"SETUPPAGE");

	switch(uMsg)
	{
		case WM_INITDIALOG:
			pInst = (setup_page_assoc*)lParam;
			if (pInst)
			{
				pInst->hwnd = hwnd;
				SetPropW(hwnd, L"SETUPPAGE", pInst);
			}
			break;

		case WM_DESTROY:
			if (pInst)
			{
				pInst->PageDlgProc(uMsg, wParam, lParam);
				RemovePropW(hwnd, L"SETUPPAGE");
				pInst = NULL;
			}
			break;
	}

	return (pInst) ? pInst->PageDlgProc(uMsg, wParam, lParam) : 0;
}

static void TreeViewCheckItems(HWND hwnd, TVITEMW *pItem)
{
	while (pItem->hItem)
	{
		SendMessageW(hwnd, TVM_SETITEMW, 0, (LPARAM)pItem);
		HTREEITEM hChild = (HTREEITEM)SendMessageW(hwnd, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)pItem->hItem);
		if (hChild)
		{
			HTREEITEM hTemp = pItem->hItem;
			pItem->hItem = hChild;
			TreeViewCheckItems(hwnd, pItem);
			pItem->hItem = hTemp; 
		}
		pItem->hItem = (HTREEITEM)SendMessageW(hwnd, TVM_GETNEXTITEM , TVGN_NEXT, (LPARAM)pItem->hItem);
	}
}

static LRESULT WINAPI TreeViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC fnOldProc = (WNDPROC)GetPropW(hwnd, L"TVPROC");
	if (!fnOldProc) return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	switch(uMsg)
	{
		case WM_DESTROY:
			RemovePropW(hwnd, L"TVPROC");
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)fnOldProc);
			break;

		case WM_CHAR:
			if (0x01/*(CTRL_A)*/ == wParam)
			{
				TVITEMW item = {0};
				item.mask = TVIF_HANDLE | TVIF_STATE;
				item.stateMask = TVIS_STATEIMAGEMASK;
				item.state = INDEXTOSTATEIMAGEMASK(2);
				item.hItem = (HTREEITEM)SendMessageW(hwnd, TVM_GETNEXTITEM , TVGN_ROOT, 0L);
				SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0L);
				TreeViewCheckItems(hwnd, &item);
				SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0L);
				return 0;
			}
			break;	
	}
	return CallWindowProcW(fnOldProc, hwnd, uMsg, wParam, lParam);
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS setup_page_assoc
START_DISPATCH
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(API_SETUPPAGE_GET_NAME, GetName)
CB(API_SETUPPAGE_CREATEVIEW, CreateView)
CB(API_SETUPPAGE_SAVE, Save)
CB(API_SETUPPAGE_REVERT, Revert)
CB(API_SETUPPAGE_ISDIRTY, IsDirty)
CB(API_SETUPPAGE_VALIDATE, Validate)
END_DISPATCH
#undef CBCLASS