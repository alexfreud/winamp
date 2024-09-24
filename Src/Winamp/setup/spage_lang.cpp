#define APSTUDIO_READONLY_SYMBOLS
#include "main.h"
#include "./spage_lang.h"
#include "./setup_resource.h"
#include "../nu/ns_wc.h"
#include "./langutil.h"

typedef struct _LANGREC
{
	LPWSTR	pszFileName;
	INT		nType;
} LANGREC;


static BOOL CALLBACK AddLangToListBox(ENUMLANG *pel, LPVOID pUser);
static INT ListBox_FindLangFileIndex(HWND hwndLB, LPCWSTR pszLangPath);
static LPCWSTR ListBox_GetSelectedLang(HWND hwndLB, LPWSTR pszLangPath, INT cchLen);


setup_page_lang::setup_page_lang() : ref(1), hwnd(NULL)
{
	*szSelectionPath = 0x00;
}
setup_page_lang::~setup_page_lang()
{
}

size_t setup_page_lang::AddRef()
{
	return ++ref;
}

size_t setup_page_lang::Release()
{
	if (1 == ref) 
	{
		delete(this);
		return 0;
	}
	return --ref;
}

HRESULT setup_page_lang::GetName(bool bShort, const wchar_t **pszName)
{
	static wchar_t szName[64] = {0};
	*pszName = (*szName) ? szName : getStringW(IDS_PAGE_LANGUAGE, szName, sizeof(szName)/sizeof(wchar_t));
	return S_OK;
}

HRESULT setup_page_lang::Save(HWND hwndText)
{
	if (S_FALSE == IsDirty()) return S_OK;

	if (!*szSelectionPath) *config_langpack = 0x00;
	else
	{
		StringCbCopyW(config_langpack, sizeof(config_langpack), szSelectionPath);
	}
	config_save_langpack_var();
	return S_OK;
}

HRESULT setup_page_lang::Revert(void)
{
	HRESULT hr(S_OK);

	if (*config_langpack)
	{
		StringCbCopyW(config_langpack, sizeof(szSelectionPath), config_langpack);
	}
	else szSelectionPath[0] = 0x00;
	
	if (hwnd) 
	{	
		HWND hwndLB = GetDlgItem(hwnd, IDC_LB_LANG);
		INT index = ListBox_FindLangFileIndex(hwndLB, szSelectionPath);
		if (LB_ERR == index && *szSelectionPath) index = ListBox_FindLangFileIndex(hwndLB, NULL); // find default embeded lang
		SendMessageW(hwndLB, LB_SETCURSEL, (LB_ERR != index) ? index : 0, 0L);
	}
	return hr;
}

HRESULT setup_page_lang::IsDirty(void)
{
	INT cr;

	cr = ComparePath(config_langpack, szSelectionPath, LANGDIR);
	if (!cr) return E_UNEXPECTED;
	 
	return (CSTR_EQUAL != cr) ? S_OK : S_FALSE;
}

HRESULT setup_page_lang::Validate(void)
{
	return S_OK;
}

HRESULT setup_page_lang::CreateView(HWND hwndParent, HWND *phwnd)
{
	*phwnd = WACreateDialogParam(MAKEINTRESOURCEW(IDD_SETUP_PAGE_LANG), hwndParent, ::DialogProc, (LPARAM)this);
	return S_OK;
}

void setup_page_lang::ListBox_OnSelChange(HWND hwndCtrl)
{
	ListBox_GetSelectedLang(hwndCtrl, szSelectionPath, sizeof(szSelectionPath)/sizeof(wchar_t));
}

void setup_page_lang::ListBox_OnItemDelete(DELETEITEMSTRUCT *pdis)
{
	LANGREC *pr = (LANGREC*)pdis->itemData;
	if (pr)
	{
		if (pr->pszFileName) free(pr->pszFileName);
		free(pr);
	}
}

INT_PTR setup_page_lang::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	INT index;
	HWND hwndLB;
	hwndLB = GetDlgItem(hwnd, IDC_LB_LANG);
	EnumerateLanguages(AddLangToListBox, hwndLB);
	index = ListBox_FindLangFileIndex(hwndLB, szSelectionPath);
	if (LB_ERR == index && *szSelectionPath) index = ListBox_FindLangFileIndex(hwndLB, NULL); // find default embeded lang
	SendMessageW(hwndLB, LB_SETCURSEL, (LB_ERR != index) ? index : 0, 0L);

	PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_LB_LANG, LBN_SELCHANGE), (LPARAM)hwndLB);
	return 0;
}

void setup_page_lang::OnSize(UINT nType, INT cx, INT cy)
{
	HWND hwndCtrl;
	RECT rw;

	hwndCtrl = GetDlgItem(hwnd, IDC_LBL_HEADER);
	if (hwndCtrl)
	{
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, 0, 0, cx - rw.left*2, rw.bottom  - rw.top, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}
	hwndCtrl = GetDlgItem(hwnd, IDC_LB_LANG);
	if (hwndCtrl)
	{
		
		GetWindowRect(hwndCtrl, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
		SetWindowPos(hwndCtrl, NULL, max(0, (cx - (rw.right - rw.left))/2), rw.top, rw.right - rw.left, cy - rw.top - 16, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void setup_page_lang::OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl)
{
	switch(nCtrlID)
	{		
		case IDC_LB_LANG:
			switch(nEvntID)
			{
				case LBN_SELCHANGE: ListBox_OnSelChange(hwndCtrl); break;
			}
			break;
	}
}

INT_PTR setup_page_lang::OnDeleteItem(INT nCtrlID, DELETEITEMSTRUCT *pdis)
{
	switch(nCtrlID)
	{		
		case IDC_LB_LANG:  ListBox_OnItemDelete(pdis); return TRUE;
	}
	return 0;
}

INT_PTR setup_page_lang::PageDlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return OnInitDialog((HWND)wParam, lParam);
		case WM_DESTROY:		break;
		case WM_SIZE:		OnSize((UINT)wParam, LOWORD(lParam), HIWORD(lParam)); break;
		case WM_COMMAND:		OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
	}
	return 0;
}

static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	setup_page_lang *pInst = (setup_page_lang*)GetPropW(hwnd, L"SETUPPAGE");

	switch(uMsg)
	{
		case WM_INITDIALOG:
			pInst = (setup_page_lang*)lParam;
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
				RemovePropW(hwnd,  L"SETUPPAGE");
				pInst = NULL;
			}
			break;
	}
	
	return (pInst) ? pInst->PageDlgProc(uMsg, wParam, lParam) : 0;
}

static BOOL CALLBACK AddLangToListBox(ENUMLANG *pel, LPVOID pUser)
{
	INT index;	
	LANGREC  *pr;
	HWND hwndLB = (HWND)pUser;
	if (!hwndLB) return FALSE;

	pr = (LANGREC*)calloc(1, sizeof(LANGREC));
	if (!pr) return FALSE;
	pr->nType = pel->nType;
	if (pel->pszFileName) pr->pszFileName = _wcsdup(pel->pszFileName);
	
	index = (INT)SendMessageW(hwndLB, LB_ADDSTRING, 0, (LPARAM)pel->pszName);
	if (LB_ERR != index) SendMessageW(hwndLB, LB_SETITEMDATA, index,  (LPARAM)pr);
	else 
	{
		if (pr->pszFileName) free(pr->pszFileName);
		free(pr);
	}
	return TRUE;
}

static INT ListBox_FindLangFileIndex(HWND hwndLB, LPCWSTR pszLangPath)
{
	int index, count;

	if (!hwndLB) return LB_ERR;

	index = LB_ERR;

	count = (INT)SendMessageW(hwndLB, LB_GETCOUNT, 0, 0L);
	for (index = 0; index < count; index++)
	{
		LANGREC *pr = (LANGREC*) SendMessageW(hwndLB, LB_GETITEMDATA, index, 0L);
		if (!pr || LB_ERR == (INT)(INT_PTR)pr) continue;
		if (!pszLangPath || !*pszLangPath)
		{
			if (LANG_FILETYPE_EMBED == pr->nType && (!pr->pszFileName || !*pr->pszFileName)) break;
		}
		else
		{
			if (CSTR_EQUAL == ComparePath(pszLangPath, pr->pszFileName, LANGDIR)) break;
		}
	}
	return (count == index) ? LB_ERR : index;
}

static LPCWSTR ListBox_GetSelectedLang(HWND hwndLB, LPWSTR pszLangPath, INT cchLen)
{
	INT index;
	LANGREC *pr;
	if (!hwndLB || !pszLangPath) return NULL;
	index = (INT)SendMessageW(hwndLB, LB_GETCURSEL, 0, 0L);
	if (LB_ERR == index) return NULL;
	pr = (LANGREC*)SendMessageW(hwndLB, LB_GETITEMDATA, index, 0L);
	if (!pr || LB_ERR == (INT)(INT_PTR)pr) return NULL;

	if (!pr->pszFileName || !*pr->pszFileName) pszLangPath[0] = 0x00;
	else if (S_OK != StringCchCopyW(pszLangPath, cchLen, pr->pszFileName)) return NULL;
	return pszLangPath;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS setup_page_lang
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