#include "main.h"
#include <windowsx.h>
#include "resource.h"
#include "../winamp/wa_ipc.h"
#include "../nu/DialogSkinner.h"
#include "../nu/menushortcuts.h"
#include "./commandbar.h"
#include "./copyfiles.h"
#include "./settings.h"
#include <shlwapi.h>
#include <strsafe.h>


#define IDT_NOTIFYINFO			1985
#define IDT_UPDATESTATUS		1986
#define DELAY_NOTIFYINFO		200
#define DELAY_UPDATESTATUS		5

#define IDC_FILEVIEW			10000

#define DATAVIEW_PROPW			L"DATAVIEW"

#define STRCOMP_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

typedef struct _DATAVIEW
{
	CHAR cLetter;
	WCHAR szSongInfoCache[MAX_PATH];
} DATAVIEW;


static INT_PTR WINAPI DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR WINAPI DataCmdBar_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CALLBACK DataViewTimer_OnNotifyInfoElapsed(HWND hdlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static void CALLBACK DataViewTimer_OnStatusUpdateElapsed(HWND hdlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

HWND CreateCdDataViewWindow(HWND hwndParent, CHAR cLetter)
{
	return WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_CDROM_DATA, hwndParent, DlgProc, (LPARAM)cLetter);
}

#define GetDataView(__hwnd) ((DATAVIEW*)GetPropW(__hwnd, DATAVIEW_PROPW))

static void DataView_NotifyInfoWindow(HWND hdlg, INT iFile, BOOL bForceRefresh)
{
	DATAVIEW *pdv = GetDataView(hdlg);
	
	HWND hParent = GetParent(hdlg);
	if (hParent)
	{
		wchar_t szPath[MAX_PATH], *pszPath;
		szPath[0] = L'\0';
		if (-1 != iFile)
		{
			HWND hfv = GetDlgItem(hdlg, IDC_FILEVIEW);
			if (hfv)
			{
				if (FileView_GetCurrentPath(hfv, szPath, ARRAYSIZE(szPath)) &&
					S_OK == StringCchCatW(szPath, ARRAYSIZE(szPath), L"\\"))
				{
					FVITEM item;
					item.mask = FVIF_TEXT | FVIF_TYPE;
					INT l = lstrlenW(szPath);
					pszPath = szPath + l;
					item.cchTextMax = l;
					item.pszText = pszPath;
					item.cchTextMax = ARRAYSIZE(szPath) - item.cchTextMax;
					if (!FileView_GetFile(hfv, iFile, &item) || FVFT_AUDIO != item.wType) szPath[0] = L'\0';
					else if (item.pszText != szPath) StringCchCopy(szPath, ARRAYSIZE(szPath), item.pszText);
				}
				else szPath[0] = L'\0';
			}
		}

		if (pdv && !bForceRefresh && 
				CSTR_EQUAL == CompareString(STRCOMP_INVARIANT, NORM_IGNORECASE, pdv->szSongInfoCache, -1, szPath, -1)) 
		{
			return;
		}
		SendMessageW(hParent, WM_SHOWFILEINFO, (WPARAM)((bForceRefresh) ? WISF_FORCE : WISF_NORMAL), (LPARAM)szPath);
		if (pdv) StringCchPrintfW(pdv->szSongInfoCache, sizeof(pdv->szSongInfoCache)/sizeof(pdv->szSongInfoCache[0]), szPath);
	}
}

static void DataViewStatus_UpdateText(HWND hdlg)
{		
	WCHAR szStatus[256] = {0};
	
	HWND hFileView = GetDlgItem(hdlg, IDC_FILEVIEW);

	HWND hbar = ViewContainer_GetCmdBar(GetParent(hdlg));
	if (!hbar) return;

	if (hFileView) 
		FileView_GetStatusText(hFileView, szStatus, sizeof(szStatus)/sizeof(szStatus[0]));
    DataView_NotifyInfoWindow(hdlg, -1, FALSE);
	SetWindowTextW(hbar, szStatus);
}
typedef struct _COLUMN
{
	INT id;
	INT	width;
} COLUMN;

static COLUMN defaultColumns[] = 
{
	{ FVCOLUMN_NAME,			200 },
	{ FVCOLUMN_SIZE,			82 },
	{ FVCOLUMN_TYPE,			64 },
	{ FVCOLUMN_MODIFIED,		140 },
	{ FVCOLUMN_EXTENSION,	64 },
};

static LPTSTR DataView_ColumnsToStr(LPTSTR pszText, size_t cchTextMax, COLUMN *pColumns, INT count)
{
	if (!pszText || cchTextMax < 1) return NULL;
	pszText[0] = TEXT('\0');
	
	if (!pColumns) return pszText;
	LPTSTR pc = pszText;
	for(int i = 0; i < count; i++)
	{
		HRESULT hr = StringCchPrintfEx(pc, cchTextMax, &pc, &cchTextMax, STRSAFE_IGNORE_NULLS,
				TEXT("%c(%d, %d)"), ((0 == i) ? TEXT(' ') : TEXT(',')), pColumns[i].id, pColumns[i].width);
		if (S_OK != hr) return NULL;
	}
	return pszText;
}

static void DataView_LoadFileViewColumns(HWND hdlg)
{
	UINT szColumns[256] = {0};
	HWND hctrl = GetDlgItem(hdlg, IDC_FILEVIEW);
	if (!hctrl) return;
	
	INT count = FileView_GetColumnArray(hctrl, ARRAYSIZE(szColumns), szColumns);
	for (int i = count-1; i >= 0; i--) FileView_DeleteColumn(hctrl, szColumns[i]);
	TCHAR szColumnList[4096] = {0};

	if (S_OK != Settings_ReadValue(C_DATAVIEW, DVF_COLUMNLIST, szColumnList, sizeof(szColumnList)) || 
		TEXT('\0') == *szColumnList)
	{	
		DataView_ColumnsToStr(szColumnList, ARRAYSIZE(szColumnList), defaultColumns, ARRAYSIZE(defaultColumns));
	}
	
	for (LPCTSTR pc = szColumnList, pBlock = NULL; TEXT('\0') != *pc; pc++)
	{
		if (TEXT('(') == *pc) pBlock = (pc + 1);
		else if (TEXT(')') == *pc)
		{
			if (pBlock && pBlock != pc)
			{
				FVCOLUMN fvc;
				fvc.mask = 0;
				while (TEXT(' ') == *pBlock && pBlock != pc) pBlock++;
				if (pBlock != pc)
				{
					fvc.id = StrToInt(pBlock);
					while (TEXT(',') != *pBlock && pBlock != pc) pBlock++;
					if (pBlock != pc) 
					{	
						while ((TEXT(',') == *pBlock || TEXT(' ') == *pBlock) && pBlock != pc) pBlock++;
						if (pBlock != pc)
						{
							fvc.width = StrToInt(pBlock);
							if (fvc.width > 0) fvc.mask |= FVCF_WIDTH;
						}
					}
					FileView_InsertColumn(hctrl, &fvc);
				}
			}
			pBlock = NULL;
		}
	}

	INT orderBy, orderAsc;
	Settings_GetInt(C_DATAVIEW, DVF_ORDERBY, &orderBy);
	Settings_GetBool(C_DATAVIEW, DVF_ORDERASC, &orderAsc);
	FileView_SetSort(hctrl, orderBy, orderAsc);
}

static void DataView_SaveFileViewColumns(HWND hdlg)
{
	UINT	szOrder[256] = {0};
	COLUMN	szColumns[256] = {0};
	TCHAR	szBuffer[1024] = {0};

	HWND hctrl = GetDlgItem(hdlg, IDC_FILEVIEW);
	if (!hctrl) return;

	INT count = FileView_GetColumnArray(hctrl, ARRAYSIZE(szOrder), szOrder);
	for (int i = 0; i < count; i++) 
	{
		szColumns[i].id = szOrder[i];
		szColumns[i].width = FileView_GetColumnWidth(hctrl, szOrder[i]);
	}
	DataView_ColumnsToStr(szBuffer, ARRAYSIZE(szBuffer), szColumns, count);
	Settings_SetString(C_DATAVIEW, DVF_COLUMNLIST, szBuffer);

	DWORD sort = FileView_GetSort(hctrl);
	Settings_SetInt(C_DATAVIEW, DVF_ORDERBY, LOWORD(sort));
	Settings_SetBool(C_DATAVIEW, DVF_ORDERASC, HIWORD(sort));


}
static UINT DataView_ReadFileViewStyle()
{
	UINT style = 0;
	INT nVal;

	style = FVS_DETAILVIEW;
	if (S_OK == Settings_GetInt(C_DATAVIEW, DVF_VIEWMODE, &nVal))
	{
		switch(nVal)
		{
			case FVS_ICONVIEW:
			case FVS_LISTVIEW:		
				style = nVal; 
				break;
		}
	}
	
	if (S_OK == Settings_GetInt(C_DATAVIEW, DVF_SHOWAUDIO, &nVal) && nVal) style |= FVS_SHOWAUDIO;
	if (S_OK == Settings_GetInt(C_DATAVIEW, DVF_SHOWVIDEO, &nVal) && nVal) style |= FVS_SHOWVIDEO;
	if (S_OK == Settings_GetInt(C_DATAVIEW, DVF_SHOWPLAYLIST, &nVal) && nVal) style |= FVS_SHOWPLAYLIST;
	if (S_OK == Settings_GetInt(C_DATAVIEW, DVF_SHOWUNKNOWN, &nVal) && nVal) style |= FVS_SHOWUNKNOWN;
	if (S_OK == Settings_GetInt(C_DATAVIEW, DVF_HIDEEXTENSION, &nVal) && nVal) style |= FVS_HIDEEXTENSION;
	if (S_OK == Settings_GetInt(C_DATAVIEW, DVF_IGNOREHIDDEN, &nVal) && nVal) style |= FVS_IGNOREHIDDEN;
	if (S_OK == Settings_GetInt(C_GLOBAL, GF_ENQUEUEBYDEFAULT, &nVal) && nVal) 
		style |= FVS_ENQUEUE;

	return style;
}

static void DataView_SaveFileViewStyle(UINT uStyle)
{
	Settings_SetInt(C_DATAVIEW, DVF_VIEWMODE, (FVS_VIEWMASK & uStyle));
	Settings_SetBool(C_DATAVIEW, DVF_SHOWAUDIO, (FVS_SHOWAUDIO & uStyle));
	Settings_SetBool(C_DATAVIEW, DVF_SHOWVIDEO, (FVS_SHOWVIDEO & uStyle));
	Settings_SetBool(C_DATAVIEW, DVF_SHOWPLAYLIST, (FVS_SHOWPLAYLIST & uStyle));
	Settings_SetBool(C_DATAVIEW, DVF_SHOWUNKNOWN, (FVS_SHOWUNKNOWN & uStyle));
	Settings_SetBool(C_DATAVIEW, DVF_HIDEEXTENSION, (FVS_HIDEEXTENSION & uStyle));
	Settings_SetBool(C_DATAVIEW, DVF_IGNOREHIDDEN, (FVS_IGNOREHIDDEN & uStyle));
}

static BOOL DataView_OnInitDialog(HWND hdlg, HWND hwndFocus, LPARAM lParam)
{
	RECT rc;
	HWND hctrl;
	
	DATAVIEW *pdv = (DATAVIEW*)calloc(1, sizeof(DATAVIEW));
	if (pdv)
	{
		pdv->cLetter = (CHAR)lParam;
		pdv->szSongInfoCache[0] = L'\0';  
		SetPropW(hdlg, DATAVIEW_PROPW, pdv);
	}

	GetClientRect(hdlg, &rc);

	MLSkinWindow2(plugin.hwndLibraryParent, hdlg, SKINNEDWND_TYPE_AUTO, SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	MLFILEVIEWCREATESTRUCT fvcs;
	fvcs.hwndParent = hdlg;
	fvcs.hwndInsertAfter = hdlg;
	fvcs.fStyle = DataView_ReadFileViewStyle();
	fvcs.x = 0;
	fvcs.y = 0;
	fvcs.cx = rc.right - rc.left;
	fvcs.cy = rc.bottom - rc.top - 4;
	hctrl = (HWND)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_CREATEFILEVIEW, (WPARAM)&fvcs);

	if (hctrl)
	{
		wchar_t szRoot[] = L"x:";
		if (pdv) szRoot[0] = pdv->cLetter;
		//szRoot[0] = L'C';
		
		FileView_SetRoot(hctrl, szRoot);
		SetWindowLongPtrW(hctrl, GWLP_ID, IDC_FILEVIEW);
		
		DataView_LoadFileViewColumns(hdlg);
		SetWindowLongPtrW(hctrl, GWL_STYLE, GetWindowLongPtrW(hctrl, GWL_STYLE) | WS_VISIBLE);
		
		INT nPos;
		if (S_OK == Settings_GetInt(C_DATAVIEW, DVF_DIVIDERPOS, &nPos))
			FileView_SetDividerPos(hctrl, nPos, FVRF_NOREDRAW);
	}	

	ViewContainer_CreateCmdBar(GetParent(hdlg), hdlg, IDD_COMMANDBAR_DATA, DataCmdBar_DialogProc, (ULONG_PTR)hctrl);

	DataView_NotifyInfoWindow(hdlg, -1, TRUE); // ignore cache
	SendMessage(hdlg, WM_COMMAND, MAKEWPARAM(ID_DRIVE_MODE_CHANGED, 0), 0L);
	
	TCHAR  szRoot[MAX_PATH] = {0};
	if (hctrl && S_OK == Settings_ReadString(C_DATAVIEW, DVF_LASTFOLDER, szRoot, ARRAYSIZE(szRoot)))
		FileView_SetCurrentPath(hctrl, szRoot, TRUE);
	
	return FALSE;
}

static void DataView_OnDestroy(HWND hdlg)
{
	KillTimer(hdlg, IDT_NOTIFYINFO);
    DataView_NotifyInfoWindow(hdlg, -1, FALSE);
	
	HWND hView = GetDlgItem(hdlg, IDC_FILEVIEW);
	if (hView)
	{
		WCHAR szRoot[MAX_PATH] = {0};
		FileView_GetCurrentPath(hView, szRoot, ARRAYSIZE(szRoot));
		Settings_SetString(C_DATAVIEW, DVF_LASTFOLDER, szRoot);
		
		DataView_SaveFileViewColumns(hdlg);
		DataView_SaveFileViewStyle(FileView_GetStyle(hView));
		
		Settings_SetInt(C_DATAVIEW, DVF_DIVIDERPOS, FileView_GetDividerPos(hView));
			
	}
	DATAVIEW *pdv = GetDataView(hdlg);
	if (pdv)
	{
		RemovePropW(hdlg, DATAVIEW_PROPW);
		free(pdv);
	}
	ViewContainer_DestroyCmdBar(GetParent(hdlg));
}

static void CALLBACK DataViewTimer_OnStatusUpdateElapsed(HWND hdlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hdlg, idEvent);
	DataViewStatus_UpdateText(hdlg);

	BOOL enablePlay = FALSE;
	HWND hctrl = GetDlgItem(hdlg, IDC_FILEVIEW);

	if (hctrl && FileView_GetSelectedCount(hctrl) > 0)
	{	
		enablePlay = (-1 != FileView_GetNextFile(hctrl, -1, FVNF_PLAYABLE | FVNF_SELECTED));
	}

	HWND hbar = ViewContainer_GetCmdBar(GetParent(hdlg));
	if (hbar)
	{
		if (NULL != (hctrl = GetDlgItem(hbar, IDC_BTN_PLAYEX))) EnableWindow(hctrl, enablePlay);
		if (NULL != (hctrl = GetDlgItem(hbar, IDC_BTN_COPY))) EnableWindow(hctrl, enablePlay);
	}

}

static void CALLBACK DataViewTimer_OnNotifyInfoElapsed(HWND hdlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hdlg, IDT_NOTIFYINFO);
	HWND hFileView = GetDlgItem(hdlg, IDC_FILEVIEW);
	
	INT iFile = -1;
	if (hFileView)
	{
		iFile = FileView_GetSelectedCount(hFileView);
		if (1 != iFile) iFile = -1;
		else iFile = FileView_GetNextFile(hFileView, -1, FVNF_FOCUSED);
	}
	
	DataView_NotifyInfoWindow(hdlg, 	iFile, FALSE);
}

static void DataView_OnWindowPosChanged(HWND hdlg, WINDOWPOS *pwp)
{
	if (0 == (SWP_NOSIZE & pwp->flags))
	{
		HWND hctrl;
		RECT rc, rw;
		GetClientRect(hdlg, &rc);
		
		hctrl = GetDlgItem(hdlg, IDC_FILEVIEW);
		if (hctrl && GetWindowRect(hctrl, &rw))
		{				
			MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)&rw, 2);
			SetWindowPos(hctrl, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 
						SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | ((SWP_NOREDRAW | SWP_NOCOPYBITS) & pwp->flags));
		}
	}
}



static void DataView_OnCopySelection(HWND hdlg)
{
	LPWSTR *ppszFiles = NULL;
	ULONGLONG *pFSizes;
	INT	count, iFile;
	size_t cchPath, cchFile;
	WCHAR szPath[MAX_PATH] = {0};
	FVITEM file = {0};

	HWND hctrl = GetDlgItem(hdlg, IDC_FILEVIEW);
	if (!hctrl) return;

	
	count = FileView_GetSelectedCount(hctrl);
	if (count < 1) return;

	if (!FileView_GetCurrentPath(hctrl, szPath, sizeof(szPath)/sizeof(szPath[0])) ||
		S_OK != StringCchLengthW(szPath, sizeof(szPath)/sizeof(szPath[0]), &cchPath)) return;

	ppszFiles = (LPWSTR*)CoTaskMemAlloc(sizeof(LPWSTR)*count);
	pFSizes = (ULONGLONG*)CoTaskMemAlloc(sizeof(ULONGLONG)*count);
	if (!ppszFiles || !pFSizes) 
	{
		if (ppszFiles) CoTaskMemFree(ppszFiles);
		if (pFSizes) CoTaskMemFree(pFSizes);
		return;
	}
	
	iFile = -1;
	count = 0;
	file.mask = FVIF_TEXT | FVIF_SIZE;
	TCHAR szBuffer[MAX_PATH] = {0};
	while (-1 != (iFile = FileView_GetNextFile(hctrl, iFile, FVNF_SELECTED | FVNF_PLAYABLE)))
	{
		file.pszText = szBuffer;
		file.cchTextMax = ARRAYSIZE(szBuffer);
		if (FileView_GetFile(hctrl, iFile, &file))
		{
			cchFile = (file.pszText) ? lstrlenW(file.pszText) : 0;
			if (cchFile)
			{	
				pFSizes[count] = (ULONGLONG)(((__int64)file.dwSizeHigh << 32) | file.dwSizeLow);
				ppszFiles[count] = (LPWSTR)CoTaskMemAlloc(sizeof(WCHAR)*(cchPath + cchFile + 4));
				if (ppszFiles[count])
				{
					PathCombineW(ppszFiles[count], szPath, file.pszText);
					count++;
				}
			}
		}
	}
	
	if (!MLDisc_CopyFiles(hdlg, ppszFiles, pFSizes, count))
	{
		if (ppszFiles)
		{
			for (int i = 0; i < count; i++) CoTaskMemFree(ppszFiles[i]);
			CoTaskMemFree(ppszFiles);
		}
		if (pFSizes) CoTaskMemFree(pFSizes);
	}
}

static void DataView_OnDriveModeChanged(HWND hdlg)
{
	DATAVIEW *pdv = GetDataView(hdlg);
	if (!pdv) return;
	
	HWND hbar;
	if (NULL != (hbar = ViewContainer_GetCmdBar(GetParent(hdlg))))
	{
		HWND hctrl;
		if (NULL != (hctrl = GetDlgItem(hbar, IDC_BTN_EJECT))) 
		{
			UINT cMode = DriveManager_GetDriveMode(pdv->cLetter);
			EnableWindow(hctrl, (DM_MODE_READY == cMode));
		}
	}
	
}

static void DataView_OnCommand(HWND hdlg, INT eventId, INT ctrlId, HWND hwndCtrl)
{
	switch(ctrlId)
	{
		case ID_COPY_SELECTION:
			DataView_OnCopySelection(hdlg);    
			break;
		case ID_DRIVE_MODE_CHANGED:
			DataView_OnDriveModeChanged(hdlg);
			break;
	}
}

static void FileView_OnFolderChanged(HWND hdlg, NMHDR *phdr)
{
	DataView_NotifyInfoWindow(hdlg, -1, FALSE);
	SetTimer(hdlg, IDT_UPDATESTATUS, DELAY_UPDATESTATUS, DataViewTimer_OnStatusUpdateElapsed);
}

static void FileView_OnStatusChanged(HWND hdlg, NMHDR *phdr)
{
	SetTimer(hdlg, IDT_UPDATESTATUS, DELAY_UPDATESTATUS, DataViewTimer_OnStatusUpdateElapsed);
}

static void FileView_OnFileStateChanged(HWND hdlg, NMFVSTATECHANGED *pnmsc)
{
	SetTimer(hdlg, IDT_UPDATESTATUS, DELAY_UPDATESTATUS, DataViewTimer_OnStatusUpdateElapsed);
	SetTimer(hdlg, IDT_NOTIFYINFO, DELAY_NOTIFYINFO, DataViewTimer_OnNotifyInfoElapsed);	
}

static void FileView_OnDoubleClick(HWND hdlg, NMFVFILEACTIVATE *pnma)
{
	if (-1 == pnma->iFile) return;
	
	HWND hctrl = GetDlgItem(hdlg, IDC_FILEVIEW);
	if (!hctrl) return;

	SendMessageW(hctrl, WM_COMMAND,
		MAKEWPARAM(FileView_GetActionCommand(hctrl, (FVS_ENQUEUE & (FileView_GetStyle(hctrl)) ? FVA_ENQUEUE : FVA_PLAY)), 0), 0L);
}

static void FileView_OnUninitOptionsMenu(HWND hdlg, HMENU hMenu, UINT uCommand)
{	
	HWND hParent = GetParent(hdlg);
	if (!hMenu || !hParent || !ViewContainer_GetMiniInfoEnabled(hParent)) return;
	INT index = GetMenuItemCount(hMenu);

	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW), };
	mii.fMask = 	MIIM_ID;
	while (index--)
	{
		if (GetMenuItemInfoW(hMenu, index, TRUE, &mii) && ID_MINIINFO_SHOW == mii.wID)
		{
			if (DeleteMenu(hMenu, index, MF_BYPOSITION))
					DeleteMenu(hMenu, --index, MF_BYPOSITION);
			break;
		}
	}
}

static void FileView_OnInitOptionsMenu(HWND hdlg, HMENU hMenu)
{
	INT index;
	HWND hParent;

	hParent = GetParent(hdlg);
	if (!hMenu || !hParent || !ViewContainer_GetMiniInfoEnabled(hParent)) return;
	
	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW), };
	
	index = GetMenuItemCount(hMenu);

	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_SEPARATOR;
	
	if (InsertMenuItemW(hMenu, index, TRUE, &mii))
	{
		wchar_t szText[1024] = {0};
		WASABI_API_LNGSTRINGW_BUF(IDS_SHOW_INFO, szText, sizeof(szText)/sizeof(szText[0]));

		if (WASABI_API_APP)
		{
			HACCEL szAccel[24] = {0};
			INT c = WASABI_API_APP->app_getAccelerators(GetParent(hdlg), szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
			AppendShortcutText(szText, sizeof(szText)/sizeof(szText[0]), ID_MINIINFO_SHOW, szAccel, c, MSF_REPLACE);
		}

		index++;
		mii.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID;
		mii.dwTypeData = szText;
		mii.wID = ID_MINIINFO_SHOW;
		mii.fState = (ViewContainer_GetMiniInfoVisible(hParent)) ? MFS_CHECKED : MFS_UNCHECKED;

		InsertMenuItemW(hMenu, index, TRUE, &mii);
	}
}

static void FileView_OnInitFileContextMenu(HWND hdlg, HMENU hMenu, HWND hView)
{
	HWND hbar = ViewContainer_GetCmdBar(GetParent(hdlg));
	HWND hButton = (hbar) ? GetDlgItem(hbar, IDC_BTN_COPY) : NULL; 
	if (!hButton) return;
	
	wchar_t szText[1024] = {0};
	GetWindowTextW(hButton, szText, sizeof(szText)/sizeof(szText[0]));

	if (WASABI_API_APP)
	{
		HACCEL szAccel[24] = {0};
		INT c = WASABI_API_APP->app_getAccelerators(GetParent(hdlg), szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
		AppendShortcutText(szText, sizeof(szText)/sizeof(szText[0]), ID_COPY_SELECTION, szAccel, c, MSF_REPLACE);
	}
	
	MENUITEMINFOW mii = { sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID;
	mii.dwTypeData = szText;
	mii.wID	= ID_COPY_SELECTION;		// TODO:  make id uniqueue
	mii.fState = (IsWindowEnabled(hButton)) ? MFS_ENABLED : MFS_DISABLED;
	
	InsertMenuItemW(hMenu, 2, TRUE, &mii);
}

static BOOL FileView_OnInitMenu(HWND hdlg, NMFVMENU *pnm)
{
	switch(pnm->uMenuType)
	{
		case FVMENU_OPTIONS: FileView_OnInitOptionsMenu(hdlg, pnm->hMenu); break;
		case FVMENU_FILEOPCONTEXT: FileView_OnInitFileContextMenu(hdlg, pnm->hMenu, pnm->hdr.hwndFrom); break;
	}
	return FALSE;
}

static void  FileView_OnUninitMenu(HWND hdlg, NMFVMENU *pnm)
{
	switch(pnm->uMenuType)
	{
		case FVMENU_OPTIONS: FileView_OnUninitOptionsMenu(hdlg, pnm->hMenu, pnm->uCommand); break;
		case FVMENU_FILEOPCONTEXT: DeleteMenu(pnm->hMenu, ID_COPY_SELECTION, MF_BYCOMMAND); break;
	}
}

static BOOL FileView_OnMenuCommand(HWND hdlg, NMFVMENU *pnm)
{
	switch(pnm->uCommand)
	{
		case ID_MINIINFO_SHOW:
			SendMessageW(GetParent(hdlg), WM_COMMAND, MAKEWPARAM(ID_MINIINFO_SHOW, 0), 0L);
			return TRUE;
		case ID_COPY_SELECTION:
			SendMessageW(hdlg, WM_COMMAND, MAKEWPARAM(ID_COPY_SELECTION, 0), 0L);
			return TRUE;
	}
	return FALSE;
}

static INT_PTR DataView_OnNotify(HWND hdlg, INT ctrlId, NMHDR *phdr)
{
	switch(phdr->idFrom)
	{
		case IDC_FILEVIEW:
			switch(phdr->code)
			{ 
				case FVN_FOLDERCHANGED: FileView_OnFolderChanged(hdlg, phdr); break;
				case FVN_STATECHANGED:	FileView_OnFileStateChanged(hdlg, (NMFVSTATECHANGED*)phdr); break;
				case FVN_STATUSCHANGED: FileView_OnStatusChanged(hdlg, phdr); break;
				case NM_DBLCLK:			FileView_OnDoubleClick(hdlg, (NMFVFILEACTIVATE*)phdr); break;
				case FVN_INITMENU:		return FileView_OnInitMenu(hdlg, (NMFVMENU*)phdr);
				case FVN_UNINITMENU:		FileView_OnUninitMenu(hdlg, (NMFVMENU*)phdr); break;
				case FVN_MENUCOMMAND:	return FileView_OnMenuCommand(hdlg, (NMFVMENU*)phdr);
			}
			break;
	}
	return 0;
}


static void DataView_OnQueryInfo(HWND hdlg)
{
	KillTimer(hdlg, IDT_NOTIFYINFO);
	DataView_NotifyInfoWindow(hdlg, -1, TRUE);
	SetTimer(hdlg, IDT_NOTIFYINFO, DELAY_NOTIFYINFO, DataViewTimer_OnNotifyInfoElapsed);
}

static void DataView_OnDisplayChange(HWND hdlg)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_FILEVIEW);
	if (!hctrl) return;
	BOOL bVal;
	Settings_GetBool(C_GLOBAL, GF_ENQUEUEBYDEFAULT, &bVal);
	FileView_SetStyle(hctrl, (bVal) ? FVS_ENQUEUE : 0L, FVS_ENQUEUE);
}

static INT_PTR WINAPI DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:		return DataView_OnInitDialog(hdlg, (HWND)wParam, lParam);
		case WM_DESTROY:			DataView_OnDestroy(hdlg); break;
		case WM_WINDOWPOSCHANGED: DataView_OnWindowPosChanged(hdlg, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:			DataView_OnCommand(hdlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam); break;
		case WM_NOTIFY:			MSGRESULT(hdlg, DataView_OnNotify(hdlg, (INT)wParam, (LPNMHDR) lParam));
		case WM_QUERYFILEINFO:	DataView_OnQueryInfo(hdlg); break;
		case WM_DISPLAYCHANGE:	DataView_OnDisplayChange(hdlg);  break;
			
	}
	
	return 0;
}