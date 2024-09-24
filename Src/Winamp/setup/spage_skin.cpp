#define APSTUDIO_READONLY_SYMBOLS
#include "main.h"
#include "./spage_skin.h"
#include "./setup_resource.h"
#include "./skininfo.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoWide.h"
#include "./loadimage.h"
#include "./langutil.h"
#include "./setupcommon.h"

typedef struct _SKINLINK
{
	LPCWSTR pszName;
	LPCWSTR pszTarget;
} SKINLINK;

typedef struct _SKINREC
{ 
	LPWSTR	pszName;
	LPWSTR	pszFileName;
	INT		nType;
	BOOL	bLink; // if bLink == TRUE than nType index in links
} SKINREC;

#define CURRENT_SKIN_ID		1
#define SEPARATOR_DATA		0xEFFF

#define LINK_FONT_ITALIC	FALSE
#define LINK_FONT_WEIGHT	FW_MEDIUM
//#define LINK_TEXT_COLOR				RGB(0, 50, 193)
//#define LINK_TEXT_HIGHLITECOLOR		RGB(16, 64, 212)

#define PREVIEW_WIDTH		178
#define PREVIEW_HEIGHT		75

static SKINLINK links[] = 
{
	{ MAKEINTRESOURCEW(IDS_SKIN_PROMO1), BENTO_SKIN_NAME },
	{ MAKEINTRESOURCEW(IDS_SKIN_PROMO2), BIG_BENTO_SKIN_NAME },
	{ MAKEINTRESOURCEW(IDS_SKIN_CURRENT), MAKEINTRESOURCEW(CURRENT_SKIN_ID) },
};

static wchar_t szClassic[64] = {0, };

#define CLASSIC_NAME() ((!*szClassic) ? getStringW(IDS_CLASSIC_SKIN_NAME, szClassic, sizeof(szClassic)/sizeof(wchar_t)) : szClassic)

static BOOL CALLBACK AddSkinToListBox(ENUMSKIN *pes, LPVOID pUser);
static LRESULT WINAPI ListBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LPCWSTR ResolveLink(SKINREC *pr);
static INT ListBox_FindSkinFileIndex(HWND hwndLB, LPCWSTR pszSkinPath, BOOL bSearchLinks);

setup_page_skin::setup_page_skin() : ref(1), hwnd(NULL), hfLink(NULL), idxSelected(-1)
{
	*szSelectionPath = 0x00;
}

setup_page_skin::~setup_page_skin()
{
}

size_t setup_page_skin::AddRef()
{
	return ++ref;
}

size_t setup_page_skin::Release()
{
	if (1 == ref) 
	{
		delete(this);
		return 0;
	}
	return --ref;
}

HRESULT setup_page_skin::GetName(bool bShort, const wchar_t **pszName)
{
	if (bShort)
	{
		static wchar_t szShortName[32] = {0};
		*pszName = (*szShortName) ? szShortName : getStringW(IDS_PAGE_SKIN, szShortName, sizeof(szShortName)/sizeof(wchar_t));
	}
	else 
	{
		static wchar_t szLongName[64] = {0};
		*pszName = (*szLongName) ? szLongName : getStringW(IDS_PAGE_SKIN_LONG, szLongName, sizeof(szLongName)/sizeof(wchar_t));
	}
	return S_OK;
}

HRESULT setup_page_skin::Save(HWND hwndText)
{
	if (S_FALSE == IsDirty()) return S_OK;

	if (!*szSelectionPath) *config_skin = 0x00;
	else if (S_OK != StringCchCopyW(config_skin, sizeof(config_skin)/sizeof(wchar_t), szSelectionPath)) return S_FALSE;

	_w_sW("skin", config_skin);
	if (hwnd) 
	{	
		HWND hwndLB = GetDlgItem(hwnd, IDC_LB_SKIN);
		INT index = ListBox_FindSkinFileIndex(hwndLB, szSelectionPath, TRUE);
		if (LB_ERR == index && *szSelectionPath) index = ListBox_FindSkinFileIndex(hwndLB, NULL, TRUE); // find default embeded skin
		SendMessageW(hwndLB, LB_SETCURSEL, (LB_ERR != index) ? index : 0, 0L);
	}
	return S_OK;//(count) ? S_OK : S_FALSE;
}

static BOOL IsForceBento()
{
	wchar_t szVer[128] = {0};
	if (GetPrivateProfileIntW(L"WinampReg", L"IsFirstInst", 0, INI_FILE)) return TRUE;
	GetPrivateProfileStringW(L"WinampReg", L"WAVer", L"", szVer, 128, INI_FILE);
	return (!*szVer || CSTR_LESS_THAN == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
														NORM_IGNORECASE, szVer, -1, AutoWide(APP_VERSION), -1));
}

HRESULT setup_page_skin::Revert(void)
{
	HRESULT hr(S_OK);

	if (NULL != config_skin && !IsForceBento()) StringCchCopyW(szSelectionPath, sizeof(szSelectionPath)/sizeof(wchar_t), config_skin);
	else StringCchCopyW(szSelectionPath, sizeof(szSelectionPath)/sizeof(wchar_t), L"Bento");

	if (hwnd) 
	{	
		HWND hwndLB = GetDlgItem(hwnd, IDC_LB_SKIN);
		INT index = ListBox_FindSkinFileIndex(hwndLB, szSelectionPath, TRUE);
		if (LB_ERR == index && *szSelectionPath) index = ListBox_FindSkinFileIndex(hwndLB, NULL, TRUE); // find default embeded skin
		SendMessageW(hwndLB, LB_SETCURSEL, (LB_ERR != index) ? index : 0, 0L);
	}
	return hr;
}

HRESULT setup_page_skin::IsDirty(void)
{
	INT cr;

	if (IsForceBento()) return S_OK;

	cr = ComparePath(config_skin, szSelectionPath, SKINDIR);
	if (!cr) return E_UNEXPECTED;
	return (CSTR_EQUAL != cr) ? S_OK : S_FALSE;
}

HRESULT setup_page_skin::Validate(void)
{
	return S_OK;
}

HRESULT setup_page_skin::CreateView(HWND hwndParent, HWND *phwnd)
{
	*phwnd = WACreateDialogParam(MAKEINTRESOURCEW(IDD_SETUP_PAGE_SKIN), hwndParent, ::DialogProc, (LPARAM)this);
	return S_OK;
}

void setup_page_skin::ListBox_OnSelChange(HWND hwndCtrl)
{
	SKININFO si = {0};
	SKINREC *pr;
	WCHAR	szText[MAX_PATH*2] = {0};
	INT typeID, index;
	
	index = (INT)SendMessageW(hwndCtrl, LB_GETCURSEL, 0, 0L);
	if (idxSelected == index) return;

	pr = (SKINREC*)SendMessageW(hwndCtrl, LB_GETITEMDATA, index, 0L);
	if (pr && LB_ERR != (INT)(INT_PTR)pr && pr->bLink) // resolve link
	{
		index = ListBox_FindSkinFileIndex(hwndCtrl, ResolveLink(pr), FALSE);
		pr = (LB_ERR != index) ? (SKINREC*)SendMessageW(hwndCtrl, LB_GETITEMDATA, index, 0L) : NULL;
	}
	if (!pr || LB_ERR == (INT)(INT_PTR)pr) { SendMessageW(hwndCtrl, LB_SETCURSEL, idxSelected, 0L);  return; }
	idxSelected = index;

	if (!pr->pszFileName || !*pr->pszFileName) szSelectionPath[0] = 0x00;
	else StringCchCopyW(szSelectionPath, sizeof(szSelectionPath)/sizeof(wchar_t), pr->pszFileName);
		
	ZeroMemory(&si, sizeof(SKININFO));
	si.cbSize = sizeof(SKININFO);
	si.fMask = SIF_PREVIEW;
	StringCchCopyW(si.szName, SI_NAMEMAX, pr->pszName);

	GetSkinInfo(BuildFullPath(SKINDIR, szSelectionPath, szText, sizeof(szText)/sizeof(wchar_t)), &si);

	switch(si.type)
	{
		case SKIN_TYPE_MODERN: typeID = IDS_MODERN; break;
		case SKIN_TYPE_CLASSIC: typeID = IDS_CLASSIC; break;
		default: typeID = IDS_UNKNOWN; break;
	}

	SetDlgItemTextW(hwnd, IDC_EDT_NAME, si.szName);
	SetDlgItemTextW(hwnd, IDC_EDT_TYPE, getStringW(typeID, NULL, 0));
	SetDlgItemTextW(hwnd, IDC_EDT_AUTHOR, si.szAuthor);
	SetDlgItemTextW(hwnd, IDC_EDT_VERSION, si.szVersion);
	if (!si.hPreview) si.hPreview = WALoadImage2(L"PNG", MAKEINTRESOURCEW(IDB_PREVIEW_NO), FALSE);
	if (si.hPreview)
	{
		HBRUSH hb = (HBRUSH)SendMessageW(hwnd, WM_CTLCOLORSTATIC, NULL, (LPARAM)GetDlgItem(hwnd, IDC_PIC_PREVIEW));
		WABlendOnColor(si.hPreview, RGB(137,145,156));
		si.hPreview = WAResizeImage(si.hPreview, PREVIEW_WIDTH, PREVIEW_HEIGHT, hb);
		if (!si.hPreview) si.hPreview = WALoadImage2(L"PNG", MAKEINTRESOURCEW(IDB_PREVIEW_NO), FALSE);
	}

	HBITMAP hPrev = (HBITMAP)SendDlgItemMessageW(hwnd, IDC_PIC_PREVIEW, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)si.hPreview);
	if (hPrev) DeleteObject(hPrev);
}

void setup_page_skin::ListBox_OnItemDelete(DELETEITEMSTRUCT *pdis)
{
	SKINREC *pr = (SKINREC*)pdis->itemData;
	if (pr)
	{
		if (pr->pszName) free(pr->pszName);
		if (pr->pszFileName) free(pr->pszFileName);
		free(pr);
	}
}

void setup_page_skin::ListBox_OnDrawItem(DRAWITEMSTRUCT *pdis)
{
	RECT ri;
	SKINREC *pr = (SKINREC*)pdis->itemData;
	static UINT trackFocus = 0x0000FFFF;

	if (pdis->itemID == -1) return;

	CopyRect(&ri, &pdis->rcItem);
	switch(pdis->itemAction)
	{
		case ODA_FOCUS:
			if (0 == (0x0200/*ODS_NOFOCUSRECT*/ & pdis->itemState) && pr)
			{
				if (LOWORD(trackFocus) != pdis->itemID || HIWORD(trackFocus) != (ODS_FOCUS & pdis->itemState) ? 0x0001: 0x0000)  
					DrawFocusRect(pdis->hDC, &ri);
				trackFocus = pdis->itemID | ((ODS_FOCUS & pdis->itemState) ? 0x00010000 : 0x00000000);
			}
			else trackFocus = pdis->itemID;
			return;

		case ODA_SELECT:
			if (0 == (ODS_SELECTED & pdis->itemState) && pr) 
				ExtTextOutW(pdis->hDC, 0, 0, ETO_OPAQUE, &ri, L"", 0, NULL);
			break;
	}

	if ((ODS_SELECTED & pdis->itemState) && pr)
	{
		COLORREF rgb = SetBkColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
		ExtTextOutW(pdis->hDC, 0, 0, ETO_OPAQUE, &ri, L"", 0, NULL);
		SetBkColor(pdis->hDC, rgb);
	}

	if (pr)
	{
		if (pr->pszName)
		{
			HFONT hfOld;
			COLORREF rgb(0);

			if (pr->bLink)
			{
				if (!hfLink)
				{
					LOGFONTW lf = {0};
					if (GetObject(GetCurrentObject(pdis->hDC, OBJ_FONT), sizeof(LOGFONTW), &lf))
					{
						lf.lfItalic = LINK_FONT_ITALIC;
						lf.lfWeight = LINK_FONT_WEIGHT;
						hfLink = CreateFontIndirectW(&lf);
					}
				}
				hfOld = (HFONT)SelectObject(pdis->hDC, hfLink);
				rgb = SetTextColor(pdis->hDC, GetSysColor((ODS_SELECTED & pdis->itemState) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)); 
			}
			else
			{ 
				hfOld = NULL; 
				rgb = SetTextColor(pdis->hDC, GetSysColor((ODS_SELECTED & pdis->itemState) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)); 
			}

			ri.left += 2;
			COLORREF oldrgb = SetBkColor(pdis->hDC, GetSysColor((ODS_SELECTED & pdis->itemState) ? COLOR_HIGHLIGHT : COLOR_WINDOW));
			DrawTextW(pdis->hDC, pr->pszName, -1, &ri, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_WORD_ELLIPSIS);
			SetBkColor(pdis->hDC, oldrgb);
			if (pr->bLink && hfOld) SelectObject(pdis->hDC, hfOld);
			SetTextColor(pdis->hDC, rgb);
		}
	}
	else
	{
		HPEN hPen, hPenOld;
		hPen = CreatePen(PS_DOT, 0, GetSysColor(COLOR_WINDOWTEXT));
		hPenOld = (HPEN)SelectObject(pdis->hDC, hPen);
		MoveToEx(pdis->hDC, ri.left + 1, ri.top + (ri.bottom - ri.top)/2, NULL);
		LineTo(pdis->hDC, ri.right - 1, ri.top + (ri.bottom - ri.top)/2);
		SelectObject(pdis->hDC, hPenOld);
	}
}

INT_PTR setup_page_skin::ListBox_OnItemCompare(COMPAREITEMSTRUCT *pcis)
{	SKINREC *pr1, *pr2;
	INT cr;

	pr1 = (SKINREC*)pcis->itemData1;
	pr2 = (SKINREC*)pcis->itemData2;
	if (pr1 == pr2) return 0;
    if (!pr1) return (pr2->bLink) ? 1 : -1;
	if (!pr2) return (pr1->bLink) ? -1 : 1;
	if (pr1->bLink) return (pr2->bLink) ? (pcis->itemID1 - pcis->itemID2) : -1;
	if (pr2->bLink) return (pr1->bLink) ? (pcis->itemID1 - pcis->itemID2) : 1;
	cr = CompareStringW(pcis->dwLocaleId, NORM_IGNORECASE, pr1->pszName, -1, pr2->pszName, -1);
	if (CSTR_EQUAL == cr)
	{
		INT t1, t2;
		t1 = (SKIN_FILETYPE_WAL == pr1->nType) ? SKIN_FILETYPE_ZIP : ((SKIN_FILETYPE_ZIP == pr1->nType) ? SKIN_FILETYPE_WAL : pr1->nType);
		t2 = (SKIN_FILETYPE_WAL == pr2->nType) ? SKIN_FILETYPE_ZIP : ((SKIN_FILETYPE_ZIP == pr2->nType) ? SKIN_FILETYPE_WAL : pr2->nType);
		return t1 - t2;
	}
	return cr - 2;
}

INT_PTR setup_page_skin::ListBox_OnVKeyToItem(HWND hwndLB, WORD vKey, INT index)
{
	switch(vKey)
	{
		case VK_UP:
		case VK_LEFT:
			if (NULL == SendMessageW(hwndLB, LB_GETITEMDATA, index - 1, 0L)) return index - 2;
			break;
		case VK_DOWN:
		case VK_RIGHT:
			if (NULL == SendMessageW(hwndLB, LB_GETITEMDATA, index + 1, 0L)) return index + 2;
			break;
		case VK_NEXT:
		case VK_PRIOR:
		{
			RECT rc;
			INT page, count;
			GetClientRect(hwndLB, &rc);
			page = (rc.bottom - rc.top)/(INT)SendMessageW(hwndLB, LB_GETITEMHEIGHT, 0, 0L) - 1;
			count = (INT)SendMessageW(hwndLB, LB_GETCOUNT, 0, 0L);
			index = (VK_NEXT == vKey) ? min(index + page, count) : max(0, index - page);
			if (NULL == SendMessageW(hwndLB, LB_GETITEMDATA, index, 0L)) return index + ((VK_NEXT == vKey) ? 1 : -1);
			break;
		}
	}
	return -1;
}

INT_PTR setup_page_skin::ListBox_OnCharToItem(HWND hwndLB, WORD vKey, INT index)
{
	return -2;
}

INT_PTR setup_page_skin::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	HWND hwndLB, hwndGrp;
	int count, linksCount(0), index;
	wchar_t szBuffer[MAX_PATH] = {0};

	idxSelected = -1;

	hwndGrp = GetDlgItem(hwnd, IDC_GRP_PREVIEW);
	SetWindowLongPtrW(hwndGrp, GWL_STYLE, GetWindowLongPtrW(hwndGrp, GWL_STYLE) | WS_CLIPSIBLINGS);

	hwndLB = GetDlgItem(hwnd, IDC_LB_SKIN);
	EnumerateSkins(AddSkinToListBox, hwndLB);

	count = sizeof(links)/sizeof(SKINLINK);

	for (int i = count - 1; i >= 0; i--) 
	{
		SKINREC *pr = (SKINREC*)calloc(1, sizeof(SKINREC));
		if (pr)
		{
			LPCWSTR pszName = links[i].pszName;
			if (IS_INTRESOURCE(pszName)) pszName = getStringW((INT)(INT_PTR)pszName, szBuffer, sizeof(szBuffer)/sizeof(wchar_t));
			pr->pszName = _wcsdup(pszName);
			pr->bLink = TRUE;
			pr->nType = i;

			if(LB_ERR != ListBox_FindSkinFileIndex(hwndLB, ResolveLink(pr), FALSE) && 
				LB_ERR != SendMessageW(hwndLB, LB_INSERTSTRING, 0, (LPARAM)pr)) 
			{
				linksCount++;
			}
			else 
			{
				if (pr)
				{
					if (pr->pszName) free(pr->pszName);
					free(pr);
				}
			}
		}
	}

	if (linksCount) SendMessageW(hwndLB, LB_INSERTSTRING, linksCount, (LPARAM)NULL);

	WNDPROC fnOldProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwndLB, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ListBoxProc);
	if (fnOldProc) SetPropW(hwndLB, L"SKINLB", fnOldProc);

	index = ListBox_FindSkinFileIndex(hwndLB, szSelectionPath, TRUE);
	if (LB_ERR == index && *szSelectionPath) index = ListBox_FindSkinFileIndex(hwndLB, NULL, TRUE); // find default embeded lang
	SendMessageW(hwndLB, LB_SETCURSEL, (LB_ERR != index) ? index : 0, 0L);

	PostMessageW(hwnd, WM_COMMAND, MAKEWPARAM(IDC_LB_SKIN, LBN_SELCHANGE), (LPARAM)hwndLB);
	return 0;
}

void setup_page_skin::OnDestroy(void)
{
	if (hfLink) { DeleteObject(hfLink); hfLink = NULL; }
}

void setup_page_skin::OnCommand(INT nCtrlID, INT nEvntID, HWND hwndCtrl)
{
	switch(nCtrlID)
	{		
		case IDC_LB_SKIN:
			switch(nEvntID)
			{
				case LBN_SELCHANGE: ListBox_OnSelChange(hwndCtrl); break;
			}
			break;
	}
}

INT_PTR setup_page_skin::OnColorStatic(HDC hdc, HWND hwndCtrl)
{
	INT ctrlID = GetDlgCtrlID(hwndCtrl);
	switch(ctrlID)
	{
		case IDC_LBL_NAME:
		case IDC_LBL_TYPE:
		case IDC_LBL_AUTHOR:
		case IDC_LBL_VERSION:
			SetTextColor(hdc, RGB(0, 70, 213));
			break;
	}
	return NULL;
}

INT_PTR setup_page_skin::OnDrawItem(INT nCtrlID, DRAWITEMSTRUCT *pdis)
{
	switch(nCtrlID)
	{
		case IDC_LB_SKIN: ListBox_OnDrawItem(pdis); return TRUE;
	}
	return 0;
}

INT_PTR setup_page_skin::OnMeasureItem(INT nCtrlID, MEASUREITEMSTRUCT *pmis)
{
	switch(nCtrlID)
	{
		case IDC_LB_SKIN:
			return TRUE;
	}
	return FALSE;
}

INT_PTR setup_page_skin::OnDeleteItem(INT nCtrlID, DELETEITEMSTRUCT *pdis)
{
	switch(nCtrlID)
	{		
		case IDC_LB_SKIN:  ListBox_OnItemDelete(pdis); return TRUE;
	}
	return 0;
}

INT_PTR setup_page_skin::OnCompareItem(INT nCtrlID, COMPAREITEMSTRUCT *pcis)
{
	switch(nCtrlID)
	{		
		case IDC_LB_SKIN:  return ListBox_OnItemCompare(pcis);
	}
	return 0;
}

INT_PTR setup_page_skin::OnVKeyToItem(WORD vKey, INT index, HWND hwndCtrl)
{
	switch(GetDlgCtrlID(hwndCtrl))
	{
		case IDC_LB_SKIN:	return ListBox_OnVKeyToItem(hwndCtrl, vKey, index);
	}
	return -1;
}

INT_PTR setup_page_skin::OnCharToItem(WORD vKey, INT index, HWND hwndCtrl)
{
	switch(GetDlgCtrlID(hwndCtrl))
	{
		case IDC_LB_SKIN:	return ListBox_OnCharToItem(hwndCtrl, vKey, index);
	}
	return -1;
}

INT_PTR setup_page_skin::PageDlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return OnInitDialog((HWND)wParam, lParam);
		case WM_DESTROY:		OnDestroy(); break;
		case WM_CTLCOLORSTATIC: return OnColorStatic((HDC)wParam, (HWND)lParam);
		case WM_DRAWITEM:	MSGRESULT(hwnd, OnDrawItem((INT)wParam, (DRAWITEMSTRUCT*)lParam));
		case WM_MEASUREITEM:	MSGRESULT(hwnd, OnMeasureItem((INT)wParam, (MEASUREITEMSTRUCT*)lParam));
		case WM_COMMAND:		OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_DELETEITEM:	MSGRESULT(hwnd, OnDeleteItem((INT)wParam, (DELETEITEMSTRUCT*)lParam));
		case WM_COMPAREITEM:	return OnCompareItem((INT)wParam, (COMPAREITEMSTRUCT*)lParam); 
		case WM_VKEYTOITEM: return OnVKeyToItem(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); 
		case WM_CHARTOITEM: return OnCharToItem(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); 
	}
	return 0;
}

static BOOL CALLBACK AddSkinToListBox(ENUMSKIN *pes, LPVOID pUser)
{
	HWND hwndLB = (HWND)pUser;
	if (!hwndLB) return FALSE;

	SKINREC *pr = (SKINREC*)calloc(1, sizeof(SKINREC));
	if (!pr) return FALSE;
	pr->nType = pes->nType;
	if (pes->pszName) pr->pszName = _wcsdup(pes->pszName);
	if (pes->pszFileName) pr->pszFileName = _wcsdup(pes->pszFileName);

	if (LB_ERR == SendMessageW(hwndLB, LB_ADDSTRING, 0, (LPARAM)pr)) 
	{
		if (pr->pszName) free(pr->pszName);
		if (pr->pszFileName) free(pr->pszFileName);
		free(pr);
	}
	return TRUE;
}

static LPCWSTR ResolveLink(SKINREC *pr)
{
	if (!pr) return NULL;
	if (!pr->bLink) return pr->pszFileName;
	if (pr->nType < 0 || pr->nType >= sizeof(links)/sizeof(SKINLINK)) return NULL;
	if(CURRENT_SKIN_ID == (INT)(INT_PTR)links[pr->nType].pszTarget) return config_skin;
	if (IS_INTRESOURCE(links[pr->nType].pszTarget)) return getStringW((INT)(INT_PTR)links[pr->nType].pszTarget, NULL, 0);
	return  links[pr->nType].pszTarget;
}

static INT ListBox_FindSkinFileIndex(HWND hwndLB, LPCWSTR pszSkinPath, BOOL bSearchLinks)
{	
	int index, count;
	SKINREC *pr;
	LPCWSTR pszFileName;

	if (!hwndLB) return LB_ERR;

	index = LB_ERR;

	count = (INT)SendMessageW(hwndLB, LB_GETCOUNT, 0, 0L);
	for (index = 0; index < count; index++)
	{
		pr = (SKINREC*) SendMessageW(hwndLB, LB_GETITEMDATA, index, 0L);
		if (!pr || LB_ERR == (INT)(INT_PTR)pr) continue;

		if (pr->bLink)
		{
			if(bSearchLinks) pszFileName = ResolveLink(pr);
			else continue;
		}
		else pszFileName = pr->pszFileName;

		if (!pszSkinPath || !*pszSkinPath)
		{
			if (!pszFileName || !*pszFileName) break;
		}
		else
		{			
			if (pszFileName && CSTR_EQUAL == ComparePath(pszSkinPath, pszFileName, SKINDIR)) break;
		}
	}		
	return (count == index) ? LB_ERR : index;
}

static INT_PTR WINAPI DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	setup_page_skin *pInst = (setup_page_skin*)GetPropW(hwnd, L"SETUPPAGE");
	switch(uMsg)
	{
		case WM_INITDIALOG:
			pInst = (setup_page_skin*)lParam;
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

static LRESULT WINAPI ListBoxProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WNDPROC fnOldProc = (WNDPROC) GetPropW( hwnd, L"SKINLB" );
	if ( !fnOldProc ) return DefWindowProcW( hwnd, uMsg, wParam, lParam );
	switch ( uMsg )
	{
		case WM_DESTROY:
			RemovePropW( hwnd, L"SKINLB" );
			SetWindowLongPtrW( hwnd, GWLP_WNDPROC, (LONG_PTR) fnOldProc );
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if ( SendMessageW( hwnd, LB_GETITEMDATA, LOWORD( SendMessageW( hwnd, LB_ITEMFROMPOINT, 0, lParam ) ), 0L ) == NULL )
				return 0;
			break;
	}

	return CallWindowProcW( fnOldProc, hwnd, uMsg, wParam, lParam );
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS setup_page_skin
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