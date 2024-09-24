#include "main.h"
#include "./copyfiles.h"
#include "./copyinternal.h"
#include "./resource.h"
#include "./settings.h"
#include "../nu/trace.h"
#include <shlwapi.h>
#include <strsafe.h>

typedef struct _PREPDLG
{
	HFONT			hfItalic;
	HWND				hActiveHelp;
	HBITMAP			hbmpLogo;
	IAutoComplete	*pac;
	IACList2			*pacl2;
	COPYDATA			*pCopyData;
	TCHAR			szCurrentRoot[MAX_PATH];
} PREPDLG;

typedef struct _CALCDISKSIZE
{
	HWND hCallback;
	DWORD dwError;
	ULARGE_INTEGER bytesFree;
	ULARGE_INTEGER bytesTotal;
	TCHAR szRoot[MAX_PATH];
} CALCDISKSIZE;

#define PREPDLG_PROP		TEXT("PREPDLG")
#define GetPrepDlg(__hdlg)	((PREPDLG*)GetProp((__hdlg), PREPDLG_PROP))

#define TID_UPDATEDISKSIZE		1985
#define DELAY_UPDATEDISKSIZE	100

static void DisplayFormatExample(HWND hdlg, INT nItemId)
{
	TCHAR szBuffer[MAX_PATH*2], szFormat[MAX_PATH] = {0};

	Settings_ReadString(C_COPY, CF_TITLEFMT, szFormat, ARRAYSIZE(szFormat));
	szBuffer[0] = TEXT('\0');
	FormatFileName(szBuffer, ARRAYSIZE(szBuffer), szFormat, 10,
				   TEXT("U2"),
				   TEXT("The Joshua Tree"),
				   TEXT("Exit"),
				   TEXT("Rock"),
				   TEXT("1987"),
				   TEXT("U2"),
				   TEXT("u2_The_Joshua_Tree.Mp3"),
				   TEXT(""));
	SetDlgItemText(hdlg, nItemId, szBuffer);
}

static DWORD WINAPI DiskFreeSpace_ThreadProc(LPVOID param)
{
	CALCDISKSIZE *pcs = (CALCDISKSIZE*)param;
	if (!pcs) return 0;
	pcs->dwError = 0;
	SetLastError(0);
	if (!GetDiskFreeSpaceEx(pcs->szRoot, &pcs->bytesFree, &pcs->bytesTotal, NULL))
		pcs->dwError = GetLastError();
	PostMessage(pcs->hCallback, CPM_UPDATEDISKSIZE, 0, (LPARAM)pcs);
	return 0;
}

static void CopyPrepare_UpdateMessage(HWND hdlg)
{
	TCHAR szBuffer[MAX_PATH*2] = {0};
	PREPDLG *ppd = GetPrepDlg(hdlg);

	szBuffer[0] = TEXT('\0');
	if (ppd && ppd->pCopyData)
	{		
		TCHAR szPath[MAX_PATH] = {0}, szFormat[256] = {0};
		if (S_OK != Settings_ReadString(C_COPY, CF_PATH, szPath, ARRAYSIZE(szPath))) *szPath = TEXT('\0');
		else CleanupDirectoryString(szPath);

		if (1 == ppd->pCopyData->count)
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_COPY_PREP_MESSAGE_SINGLE_FILE, szFormat, ARRAYSIZE(szFormat));
			StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szFormat, PathFindFileName(ppd->pCopyData->ppszFiles[0]), szPath);
		}
		else
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_COPY_PREP_MESSAGE_MULTIPLE_FILES, szFormat, ARRAYSIZE(szFormat));
			StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szFormat, ppd->pCopyData->count, szPath);
		}
	}
	SetDlgItemText(hdlg, IDC_LBL_MESSAGE, szBuffer);
}

HBITMAP CopyFiles_LoadResourcePng(LPCTSTR pszResource)
{
	HBITMAP hbmp;
	MLIMAGESOURCE src = { sizeof(MLIMAGESOURCE), };
	src.lpszName = pszResource;
	src.type = SRC_TYPE_PNG;
	src.flags = 0;
	
	src.hInst = WASABI_API_LNG_HINST;
	hbmp = MLImageLoader_LoadDib(plugin.hwndLibraryParent, &src);
	if(!hbmp) 
	{
		src.hInst = WASABI_API_ORIG_HINST;
		hbmp = MLImageLoader_LoadDib(plugin.hwndLibraryParent, &src);
	}

	DIBSECTION dibsec;
	
	if (hbmp && sizeof(DIBSECTION) == GetObjectW(hbmp, sizeof(DIBSECTION), &dibsec) &&
		BI_RGB == dibsec.dsBmih.biCompression && 1 == dibsec.dsBmih.biPlanes && 32 == dibsec.dsBm.bmBitsPixel)
	{
		MLIMAGEFILTERAPPLYEX filter = { sizeof(MLIMAGEFILTERAPPLYEX), };
		filter.filterUID = MLIF_BLENDONBK_UID;
		filter.cx = dibsec.dsBm.bmWidth;
		filter.cy = dibsec.dsBm.bmHeight;
		filter.bpp = dibsec.dsBm.bmBitsPixel;
		filter.pData = (LPBYTE)dibsec.dsBm.bmBits;
		filter.rgbBk = GetSysColor(COLOR_3DFACE);
		MLImageFilter_ApplyEx(plugin.hwndLibraryParent, &filter);
	}
	return hbmp;
}

static INT_PTR CopyPrepare_OnInitDialog(HWND hdlg, HWND hFocus, LPARAM lParam)
{
	HWND hctrl;
	PREPDLG *ppd = (PREPDLG*)calloc(1, sizeof(PREPDLG));
	if (!ppd) return 0;

	SetProp(hdlg, PREPDLG_PROP, ppd); 
	ppd->pCopyData = (COPYDATA*)lParam;

	hctrl = GetDlgItem(hdlg, IDOK);
	if (hctrl) SendMessageW(hdlg, WM_NEXTDLGCTL, (WPARAM)hctrl, (LPARAM)TRUE);
	SendMessageW(hdlg, WM_COMMAND, MAKEWPARAM(IDC_BTN_OPTIONS, BN_CLICKED), (LPARAM)GetDlgItem(hdlg, IDC_BTN_OPTIONS));

	hctrl = GetDlgItem(hdlg, IDC_LBL_EXAMPLE);
	if (hctrl)
	{
		LOGFONT lf;
		HFONT hf = (HFONT)SendMessage(hctrl, WM_GETFONT, 0, 0L);
		if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (GetObject(hf, sizeof(LOGFONT), &lf))
		{
			lf.lfItalic = TRUE;
			ppd->hfItalic = CreateFontIndirect(&lf);
			if (ppd->hfItalic)
			{
				UINT szIdList[] = { IDC_LBL_EXAMPLE_TITLE, IDC_LBL_EXAMPLE, IDC_LBL_FREE_TITLE, IDC_LBL_FREE, IDC_LBL_REQUIRED_TITLE, IDC_LBL_REQUIRED, };
				for (int i = 0; i < sizeof(szIdList)/sizeof(szIdList[0]); i++) SendDlgItemMessage(hdlg, szIdList[i], WM_SETFONT, (WPARAM)ppd->hfItalic, FALSE);
			}
		}
	}

	if (ppd->pCopyData && ppd->pCopyData->pFSizes)
	{
		TCHAR szBuffer[128] = {0};
		ULONGLONG total = 0;
		for(int i = 0; i < ppd->pCopyData->count; i++) total += ppd->pCopyData->pFSizes[i];
		StrFormatByteSize64(total, szBuffer, ARRAYSIZE(szBuffer));
		SetDlgItemText(hdlg, IDC_LBL_REQUIRED, szBuffer);
	}
	else
	{
		ShowWindow(GetDlgItem(hdlg, IDC_LBL_REQUIRED_TITLE), SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, IDC_LBL_REQUIRED), SW_HIDE);
	}

	if (ppd->pCopyData && ppd->pCopyData->hOwner)
	{
		RECT rw;
		if (!GetWindowRect(ppd->pCopyData->hOwner, &rw)) SetRect(&rw, 0, 0, 0, 0);
		if (hdlg && rw.left != rw.right)
		{
			RECT rw2;
			GetWindowRect(hdlg, &rw2);
			SetWindowPos(hdlg, HWND_TOP, 
						rw.left + ((rw.right - rw.left) - (rw2.right - rw2.left))/2, 
						rw.top + ((rw.bottom - rw.top) - (rw2.bottom - rw2.top))/2,
						0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}

	CopyPrepare_UpdateMessage(hdlg);

	SendMessage(hdlg, DM_REPOSITION, 0, 0L);	

	ppd->hbmpLogo = CopyFiles_LoadResourcePng(MAKEINTRESOURCE(IDB_FILECOPY));
	if (ppd->hbmpLogo) SendDlgItemMessage(hdlg, IDC_PIC_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)ppd->hbmpLogo);
	else ShowWindow(GetDlgItem(hdlg, IDC_PIC_LOGO), SW_HIDE);

	return FALSE;
}

static void CopyPrepare_OnDestroy(HWND hdlg)
{
	PREPDLG *ppd = GetPrepDlg(hdlg);
	RemoveProp(hdlg, PREPDLG_PROP);
	if (ppd)
	{
		if (ppd->hActiveHelp) DestroyWindow(ppd->hActiveHelp);
		if (ppd->hfItalic) DeleteObject(ppd->hfItalic);
		if (ppd->pacl2) ppd->pacl2->Release();
		if (ppd->pac) ppd->pac->Release();

		if (ppd->hbmpLogo)
		{
			HBITMAP hbmp = (HBITMAP)SendDlgItemMessage(hdlg, IDC_PIC_LOGO, STM_GETIMAGE, IMAGE_BITMAP, 0L);
			if (hbmp != ppd->hbmpLogo) DeleteObject(hbmp);
			DeleteObject(ppd->hbmpLogo);
		}
		free(ppd);
	}
}
static void CopyPrepare_OnParentNotify(HWND hdlg, UINT uMsg, LPARAM lParam)
{
	PREPDLG *ppd = GetPrepDlg(hdlg);
	if (ppd && WM_DESTROY == uMsg && ppd->hActiveHelp && ppd->hActiveHelp == (HWND)lParam)
		ppd->hActiveHelp = NULL;
}

static void CALLBACK CopyPrepare_OnUpdateDiskSizeTimer(HWND hdlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{	
	KillTimer(hdlg, idEvent);

	CopyPrepare_UpdateMessage(hdlg);

	PREPDLG *ppd = GetPrepDlg(hdlg);
	if (!ppd) return;
	CALCDISKSIZE *pcs = (CALCDISKSIZE*)calloc(1, sizeof(CALCDISKSIZE));
	if (!pcs) return;

	pcs->hCallback = hdlg;

	if (S_OK == Settings_ReadString(C_COPY, CF_PATH, pcs->szRoot, ARRAYSIZE(pcs->szRoot)))
	{
		PathStripToRoot(pcs->szRoot);
		
		if (TEXT('\0') != *pcs->szRoot && 
				(TEXT('\0') == *ppd->szCurrentRoot || 
				CSTR_EQUAL != CompareString(STRCOMP_INVARIANT, NORM_IGNORECASE, ppd->szCurrentRoot, -1, pcs->szRoot, -1)))
		{
			DWORD threadId;
			SetDlgItemText(hdlg, IDC_LBL_FREE, WASABI_API_LNGSTRINGW(IDS_CALCULATING));
			HANDLE ht = CreateThread(NULL, 0, DiskFreeSpace_ThreadProc, pcs, 0, &threadId);
			if (NULL != ht)
			{
				CloseHandle(ht);
				StringCchCopy(ppd->szCurrentRoot, ARRAYSIZE(ppd->szCurrentRoot), pcs->szRoot);
				return;
			}
			SetDlgItemText(hdlg, IDC_LBL_FREE, TEXT(""));
		}
	}
	if (TEXT('\0') == *pcs->szRoot)
	{
		pcs->dwError = ERROR_INVALID_NAME;
		StringCchCopy(ppd->szCurrentRoot, ARRAYSIZE(ppd->szCurrentRoot), pcs->szRoot);
		PostMessage(pcs->hCallback, CPM_UPDATEDISKSIZE, 0, (LPARAM)pcs);
		return;
	}
	free(pcs);
	
	
}

static void CopyPrepare_OnUpdateDiskSizeResult(HWND hdlg, CALCDISKSIZE *pcs)
{
	if (!pcs) return;
	PREPDLG *ppd = GetPrepDlg(hdlg);
	if (ppd && CSTR_EQUAL == CompareString(STRCOMP_INVARIANT, NORM_IGNORECASE, ppd->szCurrentRoot, -1, pcs->szRoot, -1))
	{
		TCHAR szBuffer[128] = {0};
		szBuffer[0] = TEXT('\0');
	
		if (ERROR_SUCCESS == pcs->dwError) StrFormatByteSize64(pcs->bytesFree.QuadPart, szBuffer, ARRAYSIZE(szBuffer));
		else WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN, szBuffer, sizeof(szBuffer));

		SetDlgItemText(hdlg, IDC_LBL_FREE, szBuffer);
	}
	free(pcs);
}


static void CopyPrepare_OnOptionsClick(HWND hdlg)
{
	RECT rw, rw2;
	BOOL bEnable;
	INT height;
	
	PREPDLG *ppd = GetPrepDlg(hdlg);

	HWND hctrl = GetDlgItem(hdlg, IDC_GRP_OPTIONS);
	if (!hctrl || !GetWindowRect(hctrl, &rw)) return;

	GetWindowRect(hdlg, &rw2);
	OffsetRect(&rw, -rw2.left, -rw2.top);

	if (WS_DISABLED & GetWindowLongPtrW(hctrl, GWL_STYLE))
	{
		height = rw.bottom + 8;
		bEnable = TRUE;
		Settings_SetDirectoryCtrl(C_COPY, CF_PATH, hdlg, IDC_EDT_PATH);
		Settings_SetCheckBox(C_COPY, CF_ADDTOMLDB, hdlg, IDC_CHK_ADDTOMLDB);
		Settings_SetCheckBox(C_COPY, CF_USETITLEFMT, hdlg, IDC_CHK_CUSTOMNAME);
		Settings_SetDlgItemText(C_COPY, CF_TITLEFMT, hdlg, IDC_EDT_NAMEFORMAT);
		SetDlgItemText(hdlg, IDC_BTN_OPTIONS, WASABI_API_LNGSTRINGW(IDS_OPTIONS_HIDE));

		if (ppd && NULL == ppd->pac)
		{
			HRESULT hr;
			hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_IAutoComplete, (LPVOID*)&ppd->pac);
			if (S_OK == hr)
			{
				IAutoComplete2 *pac2;
				if (SUCCEEDED(ppd->pac->QueryInterface(IID_IAutoComplete2, (LPVOID*)&pac2)))
				{
					pac2->SetOptions(ACO_AUTOSUGGEST | ACO_AUTOAPPEND | 0x00000020/*ACF_UPDOWNKEYDROPSLIST*/);
					pac2->Release();
				}
				
				hr = CoCreateInstance(CLSID_ACListISF, NULL, CLSCTX_INPROC_SERVER, IID_IACList2, (LPVOID*)&ppd->pacl2);
				if (S_OK == hr) ppd->pacl2->SetOptions(ACLO_FILESYSDIRS);
			}
			if(ppd->pac) ppd->pac->Init(GetDlgItem(hdlg, IDC_EDT_PATH), ppd->pacl2, NULL, NULL);
		}
	}
	else
	{
		height = rw.top;
		bEnable = FALSE;
		SetDlgItemText(hdlg, IDC_BTN_OPTIONS,  WASABI_API_LNGSTRINGW(IDS_OPTIONS_SHOW));
	}

	SetWindowPos(hdlg, NULL, 0, 0, rw2.right - rw2.left, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	EnableWindow(hctrl, bEnable);
	
	UINT szIdList[] = { IDC_EDT_PATH, IDC_BTN_BROWSE, IDC_CHK_ADDTOMLDB, 
						IDC_CHK_CUSTOMNAME, 	IDC_EDT_NAMEFORMAT, IDC_BTN_HELP, };

	for (int i = 0; i < sizeof(szIdList)/sizeof(szIdList[0]); i++) EnableWindow(GetDlgItem(hdlg, szIdList[i]), bEnable);
	if (bEnable && BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_CHK_CUSTOMNAME)) 
	{
		EnableWindow(GetDlgItem(hdlg, IDC_EDT_NAMEFORMAT), FALSE);
		ShowWindow(GetDlgItem(hdlg, IDC_LBL_EXAMPLE), SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, IDC_LBL_EXAMPLE_TITLE), SW_HIDE);
	}
}

static INT_PTR CopyPrepare_OnHelp(HWND hdlg, HELPINFO *phi)
{
	PREPDLG *ppd = GetPrepDlg(hdlg);
	if (ppd && 0 == (WS_DISABLED & GetWindowLongPtrW(GetDlgItem(hdlg, IDC_GRP_OPTIONS), GWL_STYLE)))
	{
		if (ppd->hActiveHelp) SetWindowPos(ppd->hActiveHelp, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		else ppd->hActiveHelp = MLDisc_ShowHelp(hdlg, MAKEINTRESOURCE(IDS_COPY_FILENAME_FORMAT_TITLE), 
					MAKEINTRESOURCE(IDS_COPY_FILENAME_FORMAT_CAPTION), MAKEINTRESOURCE(IDS_COPY_FILENAME_FORMAT), HF_ALLOWRESIZE);
		SetWindowLongPtrW(hdlg, DWLP_MSGRESULT, TRUE);
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK CopyPrepare_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return CopyPrepare_OnInitDialog(hdlg, (HWND)wParam, lParam);
		case WM_DESTROY:			CopyPrepare_OnDestroy(hdlg); break;
		case WM_PARENTNOTIFY:	CopyPrepare_OnParentNotify(hdlg, LOWORD(wParam), lParam); break;
		case WM_HELP:			return CopyPrepare_OnHelp(hdlg, (HELPINFO*)lParam); 
		case CPM_UPDATEDISKSIZE: CopyPrepare_OnUpdateDiskSizeResult(hdlg, (CALCDISKSIZE*)lParam); break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hdlg, LOWORD(wParam));
					break;
				case IDC_BTN_BROWSE:		if (HIWORD(wParam) == BN_CLICKED) Settings_BrowseForFolder(C_COPY, CF_PATH, hdlg, IDC_EDT_PATH); break;
				case IDC_BTN_OPTIONS:	if (HIWORD(wParam) == BN_CLICKED) CopyPrepare_OnOptionsClick(hdlg); break;
				case IDC_CHK_ADDTOMLDB:	if (BN_CLICKED == HIWORD(wParam)) Settings_FromCheckBox(C_COPY, CF_ADDTOMLDB, hdlg, IDC_CHK_ADDTOMLDB); break;
				case IDC_CHK_CUSTOMNAME: 
					if (BN_CLICKED == HIWORD(wParam))
					{
						Settings_FromCheckBox(C_COPY, CF_USETITLEFMT, hdlg, IDC_CHK_CUSTOMNAME); 
						BOOL bEnable = (BST_UNCHECKED != IsDlgButtonChecked(hdlg, IDC_CHK_CUSTOMNAME));
						EnableWindow(GetDlgItem(hdlg, IDC_EDT_NAMEFORMAT), bEnable);
						ShowWindow(GetDlgItem(hdlg, IDC_LBL_EXAMPLE_TITLE), (bEnable) ? SW_SHOWNA : SW_HIDE);
						ShowWindow(GetDlgItem(hdlg, IDC_LBL_EXAMPLE), (bEnable) ? SW_SHOWNA : SW_HIDE);
						
					}
					break;

				case IDC_EDT_PATH: 
					if (EN_CHANGE == HIWORD(wParam)) 
					{
						Settings_FromDirectoryCtrl(C_COPY, CF_PATH, hdlg, IDC_EDT_PATH);
						SetTimer(hdlg, TID_UPDATEDISKSIZE, DELAY_UPDATEDISKSIZE, CopyPrepare_OnUpdateDiskSizeTimer);
					}
					break;
				case IDC_EDT_NAMEFORMAT:
					if (EN_CHANGE == HIWORD(wParam)) 
					{
						Settings_FromDlgItemText(C_COPY, CF_TITLEFMT, hdlg, IDC_EDT_NAMEFORMAT); 
						DisplayFormatExample(hdlg, IDC_LBL_EXAMPLE);
					}
					break;

				case IDC_BTN_HELP:
                    if (HIWORD(wParam) == BN_CLICKED) 
					{
						HELPINFO hi = {sizeof(HELPINFO), };
						hi.dwContextId = HELPINFO_WINDOW;
						hi.iCtrlId = IDC_EDT_NAMEFORMAT;
						hi.hItemHandle = GetDlgItem(hdlg, IDC_EDT_NAMEFORMAT);
						hi.iContextType = 0;
						hi.MousePos.x = 0; hi.MousePos.y = 0;					
						SendMessageW(hdlg, WM_HELP, 0, (LPARAM)&hi);
					}
					break;
			}
	}
	return 0;
}