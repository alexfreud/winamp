#include "main.h"
#include "./fileview.h"
#include "./fileview_internal.h"
#include "./resource.h"
#include "./stockobjects.h"
#include "../winamp/wa_dlg.h"
#include "../nu/menushortcuts.h"
#include "../nu/CGlobalAtom.h"
#include <shlwapi.h>
#include <strsafe.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(blah) (sizeof(blah)/sizeof(*blah))
#endif

#define FOLDERBROWSER_MIN_HEIGHT		64
#define FILELIST_MIN_HEIGHT			120

#define IDC_FILEVIEW_TOOLBAR			100001
#define IDC_FILELIST                100010
#define IDC_FILELIST_HEADER			100011

#define IDT_INVALIDATELIST		1982
#define IDT_ACTIVATELISTITEM		1983
#define DELAY_INVALIDATELIST		75
#define DELAY_ACTIVATELISTITEM		0

#define ID_INTERNAL_UPDATE_COLUMN_INFO		20000

typedef struct _FILEVIEW
{
	FILEDATA	fileData;
	UINT		style;
	UINT		sortColumn;
	BOOL		bAscending;
	INT			nMaxLineCount;
	INT			infoTipIndex;
	BOOL		infoTipFolded;
	INT			statusIndex;
	HMENU		hMenu;
	FILEVIEWCOLUMN	szColumns[128 + 1]; // max allowed number of columns
	INT			columnCount;
	LONG		cyDivider;
	LONG		cyToolbar;
	LONG		yDivider;
} FILEVIEW;

#if (_WIN32_WINNT < 0x501)
typedef struct tagLVSETINFOTIP
{
    UINT cbSize;
    DWORD dwFlags;
    LPWSTR pszText;
    int iItem;
    int iSubItem;
} LVSETINFOTIP, *PLVSETINFOTIP;

#define  LVM_SETINFOTIP         (LVM_FIRST + 173)

#endif // (_WIN32_WINNT < 0x501)

static CGlobalAtom FILEVIEW_DATAW(L"FILEVIEW");

#define GetFileView(__hwnd) ((FILEVIEW*)GetPropW((__hwnd), FILEVIEW_DATAW))

static HMLIMGLST hmlilFileTypesSmall = NULL;
static HMLIMGLST hmlilFileTypesLarge = NULL;


static INT_PTR CALLBACK FileView_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND FileView_CreateDialog(HWND hwndParent, UINT fStyle, HWND hwndInsertAfter, INT x, INT y, INT cx, INT cy)
{
	HWND hwnd = WASABI_API_CREATEDIALOGPARAMW(IDD_FILEVIEW, hwndParent, FileView_DialogProc, 0L);
	if (NULL == hwnd) return NULL;
	SetWindowPos(hwnd, hwndInsertAfter, x, y, cx, cy, SWP_NOACTIVATE);
	FileView_SetStyle(hwnd, fStyle, 0xFFFFFFFF);
	return hwnd;
}

static void FileView_LoadImages()
{
	MLIMAGESOURCE_I mlis;
	ZeroMemory(&mlis, sizeof(MLIMAGESOURCE_I));
	mlis.type		= SRC_TYPE_PNG;
	mlis.hInst		= plugin.hDllInstance;

	if (NULL == hmlilFileTypesSmall)
	{
		hmlilFileTypesSmall = MLImageListI_Create(16, 16, MLILC_COLOR32_I, 3, 2, 3, hmlifMngr);
		if (NULL != hmlilFileTypesSmall)
		{
			INT imageList[] = { IDB_FILETYPE_UNKNOWN_SMALL, IDB_FILETYPE_AUDIO_SMALL, IDB_FILETYPE_VIDEO_SMALL,  IDB_FILETYPE_PLAYLIST_SMALL,};
			for(int i = 0; i < sizeof(imageList)/sizeof(imageList[0]); i++) 
			{
				mlis.lpszName	= MAKEINTRESOURCEW(imageList[i]);
				MLImageListI_Add(hmlilFileTypesSmall, &mlis, MLIF_FILTER1_UID, imageList[i]);
			}
		}
	}

	if (NULL == hmlilFileTypesLarge)
	{
		hmlilFileTypesLarge = MLImageListI_Create(32, 32, MLILC_COLOR32_I, 3, 2, 3, hmlifMngr);
		if (NULL != hmlilFileTypesLarge)
		{
			INT imageList[] = { IDB_FILETYPE_UNKNOWN_LARGE, IDB_FILETYPE_AUDIO_LARGE, IDB_FILETYPE_VIDEO_LARGE, IDB_FILETYPE_PLAYLIST_LARGE, };
			for(int i = 0; i < sizeof(imageList)/sizeof(imageList[0]); i++) 
			{
				mlis.lpszName	= MAKEINTRESOURCEW(imageList[i]);
				MLImageListI_Add(hmlilFileTypesLarge, &mlis, MLIF_FILTER1_UID, imageList[i]);
			}
		}
	}
}

static LRESULT FileView_NotifyParent(HWND hdlg, UINT uCode, NMHDR *phdr)
{
	HWND hParent = GetParent(hdlg);
	if (!phdr || !hParent) return 0L;
	phdr->code		= uCode;
	phdr->hwndFrom	= hdlg;
	phdr->idFrom		= GetDlgCtrlID(hdlg);
	return SendMessageW(hParent, WM_NOTIFY, (WPARAM)phdr->idFrom, (LPARAM)phdr);
}

static BOOL FileView_OnSetRoot(HWND hwnd, LPCWSTR pszRoot)
{
	HWND hBrowser = GetDlgItem(hwnd, IDC_FOLDER_BROWSER);
	if (NULL == hBrowser) return FALSE;

	BOOL br = FolderBrowser_SetRoot(hBrowser, pszRoot);
	PostMessageW(hwnd, FVM_REFRESH, 0, 0L);
	return br;
}

static INT FileView_OnGetRoot(HWND hwnd, LPCWSTR pszBufffer, INT cchMax)
{
	HWND hBrowser = GetDlgItem(hwnd, IDC_FOLDER_BROWSER);
	if (NULL == hBrowser) return -1;
	return FolderBrowser_GetRoot(hBrowser, pszBufffer, cchMax);
}

static void FileView_AdjustSizes(HWND hdlg)
{
	HWND hctrl;
	RECT rc, rw;
	UINT flags;	
	INT nAddressBarHeight = 18;

	GetClientRect(hdlg, &rc);

	HDWP hdwp = BeginDeferWindowPos(4);
	if (NULL == hdwp) return;
	flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOMOVE;

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_LBL_ADDRESS)) && GetWindowRect(hctrl, &rw))
	{
		HDC hdc = GetDCEx(hctrl, NULL, DCX_CACHE);
		if (hdc)
		{
			HFONT hf = (HFONT)SendMessageW(hctrl, WM_GETFONT, 0, 0L);
			if (NULL == hf) hf = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
			if (NULL != hf)
			{
				SIZE size;
				WCHAR szText[128] = {0};
				INT cch = (INT)SendMessageW(hctrl, WM_GETTEXT, (WPARAM)(sizeof(szText)/sizeof(szText[0])), (LPARAM)szText);
				HFONT hfo = (HFONT)SelectObject(hdc, hf);
				if (GetTextExtentPoint32W(hdc, szText, cch, &size))
				{
					nAddressBarHeight = size.cy + 4;
					rw.right = rw.left + size.cx;
				}
				SelectObject(hdc, hfo);
			}
			ReleaseDC(hctrl, hdc);
		}
		hdwp = DeferWindowPos(hdwp, hctrl, NULL, 0, 0, rw.right - rw.left, nAddressBarHeight, flags);
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_EDT_PATH)) && GetWindowRect(hctrl, &rw))
	{
		hdwp = DeferWindowPos(hdwp, hctrl, NULL, 0, 0, rc.right - rc.left, nAddressBarHeight, flags);
	}

	if (NULL != (hctrl = GetDlgItem(hdlg, IDC_FILEVIEW_TOOLBAR)) && GetWindowRect(hctrl, &rw))
	{
		INT height = (INT)SendMessageW(hctrl, FVM_GETIDEALHEIGHT, 0, 0L);

		FILEVIEW *pfv = GetFileView(hdlg);
		if (pfv) pfv->cyToolbar = height;
		
		hdwp = DeferWindowPos(hdwp, hctrl, NULL, 0, 0, rc.right - rc.left, height, flags);
	}

	EndDeferWindowPos(hdwp);
}

static void FileView_LayoutWindows(HWND hdlg, BOOL bRedraw)
{
	INT idList[] = {IDC_LBL_ADDRESS, IDC_EDT_PATH, IDC_FOLDER_BROWSER, IDC_HDELIM, IDC_FILEVIEW_TOOLBAR, IDC_FILELIST, };
	WINDOWPOS szwp[ARRAYSIZE(idList)], *pwp;
	RECT rc;
	LONG top, left, bottom;

	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;

	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(idList));
	if (NULL == hdwp) return;

	for (INT i = 0; i < ARRAYSIZE(idList); i++)
	{
		szwp[i].hwnd = GetDlgItem(hdlg, idList[i]);
		szwp[i].flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | ((bRedraw) ? 0 : SWP_NOREDRAW);

		if (!szwp[i].hwnd || !GetWindowRect(szwp[i].hwnd, &rc)) SetRect(&rc, 0, 0, 0, 0);

		szwp[i].x = rc.left;
		szwp[i].y = rc.top;
		szwp[i].cx = rc.right - rc.left;
		szwp[i].cy = rc.bottom - rc.top;
	}

	GetClientRect(hdlg, &rc);

	top = rc.top + 2;
	left = rc.left + 2;
	if ((pwp = &szwp[0])->hwnd) // label address
	{
		pwp->y = top;
		pwp->x = left;
		hdwp = DeferWindowPos(hdwp, pwp->hwnd, NULL, pwp->x, pwp->y, pwp->cx, pwp->cy, pwp->flags);
		left += (pwp->cx + 2);
	}
	if ((pwp = &szwp[1])->hwnd) // edit path
	{
		pwp->y = top;
		pwp->x = left;
		pwp->cx = rc.right - pwp->x;
		if (pwp->cx < 0) pwp->cx = 0;
		hdwp = DeferWindowPos(hdwp, pwp->hwnd, NULL, pwp->x, pwp->y, pwp->cx, pwp->cy, pwp->flags);
		DWORD style = GetWindowLongPtrW(pwp->hwnd, GWL_STYLE);
		if ((pwp->cx == 0) != (0 != (WS_DISABLED & style)))
			SetWindowLongPtrW(pwp->hwnd, GWL_STYLE, (style & ~WS_DISABLED) | ((0 == pwp->cx) ? WS_DISABLED : 0));
	}

	left = rc.left;
	top = max((szwp[0].y + szwp[0].cy), (szwp[1].y + szwp[1].cy)) + 2;

	LONG yDividerReal = pfv->yDivider;
	LONG cyList = (rc.bottom - (yDividerReal + pfv->cyDivider));
	if (cyList < FILELIST_MIN_HEIGHT) cyList = FILELIST_MIN_HEIGHT;
	yDividerReal = rc.bottom - cyList - pfv->cyDivider;

	if (((yDividerReal + pfv->cyDivider) - top) < FOLDERBROWSER_MIN_HEIGHT)
	{
		cyList -= (FOLDERBROWSER_MIN_HEIGHT - ((yDividerReal + pfv->cyDivider) - top));
		if (cyList < FILELIST_MIN_HEIGHT) cyList = FILELIST_MIN_HEIGHT;
		yDividerReal = rc.bottom - cyList - pfv->cyDivider;
	}

	if ((pwp = &szwp[2])->hwnd) // folderbrowser
	{
		pwp->x = left;
		pwp->y = top;
		pwp->cx = rc.right - rc.left;
		pwp->cy = yDividerReal - top;
		if (pwp->cy < (FOLDERBROWSER_MIN_HEIGHT - pfv->cyDivider)) pwp->cy = 0;
		hdwp = DeferWindowPos(hdwp, pwp->hwnd, NULL, pwp->x, pwp->y, pwp->cx, pwp->cy, pwp->flags);
		DWORD style = GetWindowLongPtrW(pwp->hwnd, GWL_STYLE);
		if ((pwp->cy == 0) != (0 != (WS_DISABLED & style)))
			SetWindowLongPtrW(pwp->hwnd, GWL_STYLE, (style & ~WS_DISABLED) | ((0 == pwp->cy) ? WS_DISABLED : 0));
		top += pwp->cy;
	}

	if ((pwp = &szwp[3])->hwnd) // delimiter
	{
		pwp->x = left;
		pwp->y = top;
		pwp->cx = rc.right - rc.left;
		pwp->cy = (szwp[2].cy > 0) ? pfv->cyDivider : 0;
		if (pwp->cy < 0) pwp->cy = 0;
		hdwp = DeferWindowPos(hdwp, pwp->hwnd, NULL, pwp->x, pwp->y, pwp->cx, pwp->cy, pwp->flags);
		top += pwp->cy;
		DWORD style = GetWindowLongPtrW(pwp->hwnd, GWL_STYLE);
		if ((pwp->cy == 0) != (0 != (WS_DISABLED & style)))
			SetWindowLongPtrW(pwp->hwnd, GWL_STYLE, (style & ~WS_DISABLED) | ((0 == pwp->cy) ? WS_DISABLED : 0));
	}

	bottom = rc.bottom;

	if ((pwp = &szwp[4])->hwnd) // toolbar
	{
		pwp->x = left;
		pwp->y = top;
		pwp->cx = rc.right - rc.left;
		pwp->cy = pfv->cyToolbar;
		if ((bottom - pwp->cy) < 64) pwp->cy = 0;
		hdwp = DeferWindowPos(hdwp, pwp->hwnd, NULL, pwp->x, pwp->y, pwp->cx, pwp->cy, pwp->flags);
		top += pwp->cy;
		DWORD style = GetWindowLongPtrW(pwp->hwnd, GWL_STYLE);
		if ((pwp->cy == 0) != (0 != (WS_DISABLED & style)))
			SetWindowLongPtrW(pwp->hwnd, GWL_STYLE, (style & ~WS_DISABLED) | ((0 == pwp->cy) ? WS_DISABLED : 0));
	}

	if ((pwp = &szwp[5])->hwnd) // file list
	{
		pwp->x = left;
		pwp->y = top;
		pwp->cx = rc.right - rc.left;
		pwp->cy = bottom - top;
		if (pwp->cy < 0) pwp->cy = 0;
		hdwp = DeferWindowPos(hdwp, pwp->hwnd, NULL, pwp->x, pwp->y, pwp->cx, pwp->cy, pwp->flags);
		DWORD style = GetWindowLongPtrW(pwp->hwnd, GWL_STYLE);
		if ((pwp->cy == 0) != (0 != (WS_DISABLED & style)))
			SetWindowLongPtrW(pwp->hwnd, GWL_STYLE, (style & ~WS_DISABLED) | ((0 == pwp->cy) ? WS_DISABLED : 0));
	}
	EndDeferWindowPos(hdwp);

	pfv->nMaxLineCount = (INT)SendDlgItemMessageW(hdlg, IDC_FILELIST, LVM_GETCOUNTPERPAGE, 0, 0L);
}

static INT FileView_GetPreferredColumnWidth(HWND hdlg, UINT columnId)
{
	HWND hctrl;
	FILEDATA *pfd;
	INT nameWidth = 0, prevMaxLen = 0;
	HDC hdc;
	HFONT hf, hfo = NULL;
	SIZE size;

	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return -1;

	pfd = &pfv->fileData;

	hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	if (!hctrl) return -1;

	hdc = GetDCEx(hctrl, NULL, DCX_CACHE);
	if (!hdc) return -1;

	hf = (HFONT)SendMessageW(hctrl, WM_GETFONT, 0, 0L);
	if (NULL == hf) hf = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
	if (NULL != hf) hfo = (HFONT)SelectObject(hdc, hf);

	for (size_t i = 0; i < pfd->count; i++)
	{
		LPCWSTR pszText = pfd->pRec[i].Info.cFileName;
		INT len = (pszText) ? lstrlenW(pszText) : 0;

		if (len > 0 && len > (prevMaxLen - 3) && 
				hdc && GetTextExtentPoint32W(hdc, pszText, len, &size) && 
				size.cx > nameWidth)
		{
			nameWidth = size.cx;
			prevMaxLen = len;
		}
	}

	if (NULL != hfo) SelectObject(hdc, hfo);
	ReleaseDC(hctrl, hdc);

	if (nameWidth < 100) nameWidth = 100;
	return nameWidth;
}

static void FileView_UpdateFileView(HWND hdlg, LPCWSTR pszPath)
{
	HWND hwndList;

	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;

	if (pszPath && L'\0' != *pszPath && 
			CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
												NORM_IGNORECASE, pszPath, -1, pfv->fileData.szPath, -1)) return;

	pfv->fileData.count = 0;
	pfv->fileData.szPath[0] = 0x00;

	hwndList = GetDlgItem(hdlg, IDC_FILELIST);
	if (hwndList) SendMessageW(hwndList, LVM_SETITEMCOUNT, 0,  0L);

	size_t count = ((size_t)-1);
	if (pszPath && L'\0' != *pszPath)
	{
		FILESYSTEMINFO fi = {sizeof(FILESYSTEMINFO), };
		HWND hfb = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
		if (!hfb || !FolderBrowser_GetFileSystemInfo(hfb, &fi)) return;
		count = FileView_ReadFileData(&pfv->fileData, pszPath, pfv->style, &fi);
	}

	FileViewMeta_TruncateQueue(0);

	if (((size_t)-1) != count) 
	{
		StringCchCopyW(pfv->fileData.szPath, sizeof(pfv->fileData.szPath)/sizeof(pfv->fileData.szPath[0]), pszPath);
		FileView_SortByColumn(&pfv->fileData, pfv->sortColumn);
		if (hwndList) 
		{
			SendMessageW(hwndList, LVM_SETITEMCOUNT, (WPARAM)count,  LVSICF_NOINVALIDATEALL);
			if (FVS_LISTVIEW == (FVS_VIEWMASK & pfv->style)) 
			{
				INT w = FileView_GetPreferredColumnWidth(hdlg, FVCOLUMN_NAME);
				if (-1 != w) SendMessageW(hwndList, LVM_SETCOLUMNWIDTH, 0,  w + 20);
			}
		}
	}

	NMHDR nmhdr;
	FileView_NotifyParent(hdlg, FVN_FOLDERCHANGED, &nmhdr);
}

static BOOL FileView_IsTypePlayable(UINT fileType, UINT uEnqueueFilter)
{
	switch(fileType)
	{
		case FVFT_AUDIO:	return (0 != (FVEF_AUDIO & uEnqueueFilter)); 
		case FVFT_VIDEO:	return (0 != (FVEF_VIDEO & uEnqueueFilter)); 
		case FVFT_PLAYLIST:	return (0 != (FVEF_PLAYLIST & uEnqueueFilter)); 
		case FVFT_UNKNOWN: 	return (0 != (FVEF_UNKNOWN & uEnqueueFilter)); 
	}
	return FALSE;
}

static void FileView_InitializeListColumn(HWND hdlg, LVCOLUMNW *pListColumn, const FILEVIEWCOLUMN *pViewColumn)
{
	pListColumn->mask		= LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	pListColumn->pszText		= IS_INTRESOURCE(pViewColumn->pszText) ? WASABI_API_LNGSTRINGW((UINT)(UINT_PTR)pViewColumn->pszText) : pViewColumn->pszText;
	pListColumn->cx			= pViewColumn->width;
	pListColumn->fmt			= pViewColumn->format;
	if (-1 == pListColumn->cx)
	{
		pListColumn->cx = FileView_GetPreferredColumnWidth(hdlg, pViewColumn->id);
		if (-1 == pListColumn->cx) pListColumn->cx = 140;
		pListColumn->cx += 24;
	}
}

static void CALLBACK OnDividerMoved(HWND hdiv, INT nPos, LPARAM param)
{
	HWND hParent;
	hParent = GetParent(hdiv);
	if (hParent) FileView_SetDividerPos(hParent, nPos, FVRF_VALIDATE);
}

static BOOL FileView_OnInitDialog(HWND hdlg, HWND hwndFocus, LPARAM lParam)
{
	FILEVIEW *pfv;
	RECT rw;
	pfv = (FILEVIEW*)calloc(1, sizeof(FILEVIEW));
	if (pfv)
	{
		pfv->style = ((UINT)-1);
		pfv->bAscending = TRUE;
		pfv->sortColumn = -1;
		pfv->yDivider = 240;
	}
	else
	{
		DestroyWindow(hdlg);
		return 0;
	}

	HWND hctrl;
	SkinWindowEx(hdlg, SKINNEDWND_TYPE_AUTO, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT);

	hctrl = GetDlgItem(hdlg, IDC_LBL_ADDRESS);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_STATIC, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT);

	hctrl = GetDlgItem(hdlg, IDC_EDT_PATH);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_AUTO, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWES_VCENTER | SWES_SELECTONCLICK);

	hctrl = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_AUTO, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT);

	hctrl = GetDlgItem(hdlg, IDC_HDELIM);
	SkinWindowEx(hctrl, SKINNEDWND_TYPE_DIVIDER, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWDIV_HORZ /*| SWDIV_NOHILITE*/);
	MLSkinnedDivider_SetCallback(hctrl, OnDividerMoved, NULL);
	pfv->cyDivider = (GetWindowRect(hctrl, &rw)) ? (rw.bottom - rw.top) : 0;

	HWND htool = FileViewToolbar_Create(hdlg);
	if (htool)
	{
		SetWindowLongPtrW(htool, GWLP_ID, IDC_FILEVIEW_TOOLBAR); 
		SetWindowPos(htool, GetDlgItem(hdlg, IDC_HDELIM), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
		SkinWindowEx(htool, SKINNEDWND_TYPE_AUTO, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT);
		pfv->cyToolbar = (GetWindowRect(htool, &rw)) ? (rw.bottom - rw.top) : 0;
	}


	INT viewIndex = 0;
	for (int i = 0; i < RegisteredColumnsCount; i++)
	{	
		if (FVCOLUMN_NAME == szRegisteredColumns[i].id)
		{
			CopyMemory(&pfv->szColumns[viewIndex], &szRegisteredColumns[i], sizeof(FILEVIEWCOLUMN));
			viewIndex++;
			break;
		}
	}
	pfv->columnCount = viewIndex;
	
	
	if (!SetPropW(hdlg, FILEVIEW_DATAW, pfv))
	{
		free(pfv);
		DestroyWindow(hdlg);
		return FALSE;
	}

	FileViewMeta_InitializeStorage(hdlg);
	FileView_LoadImages();
	FileView_SetStyle(hdlg, FVS_LISTVIEW | FVS_HIDEEXTENSION | FVS_IGNOREHIDDEN | FVS_SHOWAUDIO | FVS_SHOWVIDEO | FVS_SHOWPLAYLIST | FVS_SHOWUNKNOWN, 0xFFFFFFFF);
	FileView_AdjustSizes(hdlg);
	FileView_LayoutWindows(hdlg, FALSE);
	
	if (WASABI_API_APP)
	{
		HACCEL hAccel = WASABI_API_LOADACCELERATORSW(IDR_ACCELERATOR_FILEVIEW);
		if (hAccel) WASABI_API_APP->app_addAccelerators(hdlg, &hAccel, 1, TRANSLATE_MODE_CHILD);
	}

	return FALSE;
}

static void FileView_OnDestroy(HWND hdlg)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (pfv)
	{
		RemovePropW(hdlg,FILEVIEW_DATAW);
		if (pfv->fileData.allocated)
		{
			if (pfv->fileData.pRec) free(pfv->fileData.pRec);
			if (pfv->fileData.pSort) free(pfv->fileData.pSort);
		}
		free(pfv);
	}
	FileViewMeta_ReleaseStorage(hdlg);
	if (WASABI_API_APP) WASABI_API_APP->app_removeAccelerators(hdlg);
}

static void FileView_OnWindowPosChanged(HWND hdlg, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;
	FileView_LayoutWindows(hdlg, (0 == (SWP_NOREDRAW & pwp->flags)));
}

static BOOL FileView_OnRefresh(HWND hdlg, BOOL bIncludeFolder, BOOL bForce)
{
	NMHDR nmhdr;

	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return FALSE;

	if (bForce) pfv->fileData.szPath[0] = 0x00;

	if (bIncludeFolder)
	{
		wchar_t szText[MAX_PATH*4] = {0};
		HWND hctrl = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
		if (FolderBrowser_GetCurrentPath(hctrl, szText, sizeof(szText)/sizeof(szText[0])))
		{
			FolderBrowser_SetCurrentPath(hctrl, L"", FALSE);
			FolderBrowser_SetCurrentPath(hctrl, szText, TRUE);
		}
	}

	nmhdr.code = FBN_SELCHANGED;
	nmhdr.idFrom = IDC_FOLDER_BROWSER;
	nmhdr.hwndFrom = GetDlgItem(hdlg, (INT)nmhdr.idFrom);
	SendMessageW(hdlg, WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);

	return TRUE;
}

static BOOL FileView_OnSetView(HWND hdlg, UINT uView, BOOL bRefresh)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return FALSE;

	if ((FVS_VIEWMASK & pfv->style) == uView) return TRUE;

	HWND hctrl, hnew;
	RECT rw;
	BOOL bFocused;
	DWORD style;

	switch(uView)
	{
		case FVS_LISTVIEW: style = LVS_LIST; break;
		case FVS_ICONVIEW: style = LVS_ICON; break;
		case FVS_DETAILVIEW: style = LVS_REPORT; break;
		default: return FALSE;
	}

	style |= WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP | 
			LVS_SHOWSELALWAYS | LVS_ALIGNTOP | LVS_OWNERDATA | LVS_SHAREIMAGELISTS;

	hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	if (!hctrl || !GetWindowRect(hctrl, &rw)) SetRect(&rw, 0, 0, 1, 1);

	MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)&rw, 2);
	bFocused = (hctrl != NULL && hctrl == GetFocus());


	hnew = CreateWindowExW(WS_EX_NOPARENTNOTIFY, WC_LISTVIEWW, L"Files", style, 
							rw.left, rw.top, rw.right - rw.left, rw.bottom - rw.top, 
							hdlg, NULL, plugin.hDllInstance, 0L);
	if (!hnew) 	return FALSE;

	pfv->style = (pfv->style & ~FVS_VIEWMASK) | uView;

	if (hctrl) SetWindowLongPtrW(hctrl, GWLP_ID, 0);
	SetWindowLongPtrW(hnew, GWLP_ID, IDC_FILELIST);
	DWORD exStyle;

	exStyle = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWLVS_DOUBLEBUFFER;
	if (FVS_DETAILVIEW == uView) exStyle |= SWLVS_FULLROWSELECT;

	SkinWindowEx(hnew, SKINNEDWND_TYPE_LISTVIEW, exStyle); 
	if (FVS_DETAILVIEW == uView)
	{
		LVCOLUMNW lvc;
		INT sortIndex = -1;
		for (int i = 0; i < pfv->columnCount; i++)
		{
			FileView_InitializeListColumn(hdlg, &lvc, &pfv->szColumns[i]);
			if (pfv->szColumns[i].id == pfv->sortColumn) sortIndex = i;
			SendMessageW(hnew, LVM_INSERTCOLUMNW, i, (LPARAM)&lvc);
		}
		MLSkinnedListView_DisplaySort(hnew, sortIndex, pfv->bAscending);
	}

	SendMessageW(hnew, LVM_SETIMAGELIST, LVSIL_NORMAL, (LPARAM)MLImageListI_GetRealList(hmlilFileTypesLarge));
	SendMessageW(hnew, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)MLImageListI_GetRealList(hmlilFileTypesSmall));
	if (FVS_DETAILVIEW != uView) SendMessageW(hnew, LVM_ARRANGE, LVA_ALIGNLEFT | LVA_ALIGNTOP | LVA_SNAPTOGRID, 0L);	
//	if (FVS_ICONVIEW == uView) SendMessageW(hnew, LVM_SETICONSPACING, 0, MAKELPARAM(96, 96));	
	exStyle = LVS_EX_INFOTIP | LVS_EX_LABELTIP;
	if (FVS_DETAILVIEW == uView) exStyle |= LVS_EX_HEADERDRAGDROP;
	SendMessageW(hnew, LVM_SETEXTENDEDLISTVIEWSTYLE, exStyle, exStyle);

	HWND hHeader = (HWND)SendMessageW(hnew, LVM_GETHEADER, 0, 0L);
	if (hHeader) SetWindowLongPtrW(hHeader, GWLP_ID, IDC_FILELIST_HEADER);

	FileViewMeta_TruncateQueue(0);

	SetWindowPos(hnew, (hctrl) ? hctrl : GetDlgItem(hdlg, IDC_HDELIM), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ShowWindow(hnew, SW_SHOWNA);
	if (bFocused) SetFocus(hnew);
	if (hctrl)
	{
		if (WASABI_API_APP) WASABI_API_APP->app_removeAccelerators(hctrl);
		DestroyWindow(hctrl);
	}

	SendDlgItemMessageW(hdlg,IDC_FILEVIEW_TOOLBAR, FVM_SETSTYLE, FVS_VIEWMASK, (LPARAM)uView);
	if (bRefresh) FileView_OnRefresh(hdlg, FALSE, TRUE);
	return TRUE;
}

static void CALLBACK FileView_OnInvalidateListElapsed(HWND hdlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hdlg, idEvent);
	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	if (hctrl) InvalidateRect(hctrl, NULL, FALSE);	
}

static void CALLBACK FileView_OnMetaDataDiscovered(LPCWSTR pszFileName, ULONG_PTR param)
{
	if (!param) return;
	HWND hdlg = (HWND)param;
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;

	LPCWSTR pszFile = PathFindFileNameW(pszFileName);
	INT cchPath = (INT)(ULONG_PTR)(pszFile - pszFileName);
	if (cchPath) cchPath--;

	if (CSTR_EQUAL != CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, pfv->fileData.szPath, -1, pszFileName, cchPath))
		return;

	SetTimer(hdlg, IDT_INVALIDATELIST, DELAY_INVALIDATELIST, FileView_OnInvalidateListElapsed);

	if (pfv && (((size_t)pfv->infoTipIndex) < pfv->fileData.count || ((size_t)pfv->statusIndex) < pfv->fileData.count))
	{
		WCHAR szBuffer[2048] = {0};
		LPCWSTR pszTipFile;
		FILEDATA *pfd = &pfv->fileData;
		FILERECORD *pfr;
		size_t index;

		HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);

		if (((size_t)pfv->infoTipIndex) < pfv->fileData.count)
		{
			index = ((pfv->bAscending) ? pfv->infoTipIndex : (pfd->count - pfv->infoTipIndex - 1));
			pfr = &pfd->pRec[pfd->pSort[index]];

			if (pfr->extOffset == (lstrlenW(pfr->Info.cFileName) + 1))
			{
				StringCchPrintfW(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), L"%s.%s", pfr->Info.cFileName, (pfr->Info.cFileName + pfr->extOffset));
				pszTipFile = szBuffer;
			}
			else pszTipFile = pfr->Info.cFileName;

			if (CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, pszFile, -1, pszTipFile, -1))
			{
				if (FileViewMeta_Discover(pfv->fileData.szPath, pfr, NULL, NULL, 0) && pfr->pMeta)
				{
					LVSETINFOTIP tip;
					ZeroMemory(&tip, sizeof(LVSETINFOTIP));
					tip.cbSize = sizeof(LVSETINFOTIP);
					tip.dwFlags = 0;
					tip.iItem = pfv->infoTipIndex;
					tip.iSubItem = 0;
					tip.pszText = szBuffer;

					if (pfv->infoTipFolded)
					{
						StringCchPrintfExW(tip.pszText, sizeof(szBuffer)/sizeof(szBuffer[0]), &tip.pszText, NULL, STRSAFE_IGNORE_NULLS, L"%s\r\n", pszTipFile);
					}
					FileView_FormatFileInfo(pfr, tip.pszText, sizeof(szBuffer)/sizeof(szBuffer[0]) - (ULONG_PTR)(tip.pszText - szBuffer), FIF_TOOLTIP);
					tip.pszText = szBuffer;
					SendMessageW(hctrl, LVM_SETINFOTIP, 0, (LPARAM)&tip);

					if (pfv->statusIndex == pfv->infoTipIndex)
					{
						NMHDR  nmhdr;
						FileView_NotifyParent(hdlg, FVN_STATUSCHANGED, &nmhdr);
						pfv->statusIndex = -1;
					}
				}
				pfv->infoTipIndex = -1;
				pfv->infoTipFolded = FALSE;
			}
		}

		if (((size_t)pfv->statusIndex) < pfv->fileData.count)
		{
			index = ((pfv->bAscending) ? pfv->statusIndex : (pfd->count - pfv->statusIndex - 1));
			pfr = &pfd->pRec[pfd->pSort[index]];

			if (pfr->extOffset == (lstrlenW(pfr->Info.cFileName) + 1))
			{
				StringCchPrintfW(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), L"%s.%s", pfr->Info.cFileName, (pfr->Info.cFileName + pfr->extOffset));
				pszTipFile = szBuffer;
			}
			else pszTipFile = pfr->Info.cFileName;

			if (CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, pszFile, -1, pszTipFile, -1))
			{
				if (FileViewMeta_Discover(pfv->fileData.szPath, pfr, NULL, NULL, 0) && pfr->pMeta)
				{
					NMHDR  nmhdr;
					FileView_NotifyParent(hdlg, FVN_STATUSCHANGED, &nmhdr);
				}
				pfv->statusIndex = -1;
			}
		}
	}
}

static COLORREF rgbText, rgbTextBk;
static INT_PTR FileList_OnCustomDraw(HWND hdlg, NMLVCUSTOMDRAW *pcd)
{
	switch(pcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:	
			return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
			rgbText = pcd->clrText;
			rgbTextBk = pcd->clrTextBk;
			break;
	}
	return CDRF_DODEFAULT;
}

static void FileList_OnGetDispInfo(HWND hdlg, NMLVDISPINFOW *pdi)
{
	FILEVIEW *pfv = GetFileView(hdlg);

	if (!pfv) return;

	FILEDATA *pfd = &pfv->fileData;
	if (pdi->item.iItem >= (INT)pfd->count)
	{
		pdi->item.pszText = L"Bad Index";
		return;
	}
	size_t index = ((pfv->bAscending) ? pdi->item.iItem : (pfd->count - pdi->item.iItem - 1));
	FILERECORD *pfr = &pfd->pRec[pfd->pSort[index]];

	if ((LVIF_TEXT & pdi->item.mask))
	{
		switch(pfv->szColumns[pdi->item.iSubItem].id)
		{
			case FVCOLUMN_NAME:			pdi->item.pszText = pfr->Info.cFileName; break;
			case FVCOLUMN_SIZE:			WASABI_API_LNG->FormattedSizeString(pdi->item.pszText, pdi->item.cchTextMax, (((__int64)pfr->Info.nFileSizeHigh << 32) | pfr->Info.nFileSizeLow)); break;
			case FVCOLUMN_TYPE:			FileView_FormatType(pfr->fileType, pdi->item.pszText, pdi->item.cchTextMax); break;
			case FVCOLUMN_MODIFIED:		FileView_FormatFileTime(&pfr->Info.ftLastWriteTime, pdi->item.pszText, pdi->item.cchTextMax); break;
			case FVCOLUMN_CREATED:		FileView_FormatFileTime(&pfr->Info.ftCreationTime, pdi->item.pszText, pdi->item.cchTextMax); break;
			case FVCOLUMN_EXTENSION:
				if (pfr->extOffset)
				{
					StringCchCopyW(pdi->item.pszText, pdi->item.cchTextMax, &pfr->Info.cFileName[pfr->extOffset]); 
					CharUpperBuffW(pdi->item.pszText, lstrlenW(pdi->item.pszText));
				}
				else pdi->item.pszText = NULL;
				break;
			case FVCOLUMN_ATTRIBUTES:	FileView_FormatAttributes(pfr->Info.dwFileAttributes, pdi->item.pszText, pdi->item.cchTextMax); break;
			case FVCOLUMN_ARTIST:
			case FVCOLUMN_ALBUM:
			case FVCOLUMN_TITLE:
			case FVCOLUMN_INMLDB:
			case FVCOLUMN_GENRE:
			case FVCOLUMN_YEAR:
			case FVCOLUMN_LENGTH:
			case FVCOLUMN_BITRATE:
			case FVCOLUMN_TRACK:
			case FVCOLUMN_DISC:
			case FVCOLUMN_COMMENT:
			case FVCOLUMN_PUBLISHER:
			case FVCOLUMN_COMPOSER:
			case FVCOLUMN_ALBUMARTIST:
				if (FileViewMeta_Discover(pfv->fileData.szPath, pfr, FileView_OnMetaDataDiscovered, (ULONG_PTR)hdlg, pfv->nMaxLineCount) &&
					pfr->pMeta)
				{
					INT nVal, nVal2;
					switch(pfv->szColumns[pdi->item.iSubItem].id)
					{
						case FVCOLUMN_ARTIST:	FileViewMeta_GetString(pfr->pMeta, MF_ARTIST, (LPCWSTR*)&pdi->item.pszText); break;
						case FVCOLUMN_ALBUM:		FileViewMeta_GetString(pfr->pMeta, MF_ALBUM, (LPCWSTR*)&pdi->item.pszText); break;
						case FVCOLUMN_TITLE:		FileViewMeta_GetString(pfr->pMeta, MF_TITLE, (LPCWSTR*)&pdi->item.pszText); break;
						case FVCOLUMN_INMLDB:
							if (FileViewMeta_GetInt(pfr->pMeta, MF_SOURCE, &nVal) && METADATA_SOURCE_UNKNOWN != nVal) 
									FileView_FormatYesNo(METADATA_SOURCE_MLDB == nVal, pdi->item.pszText, pdi->item.cchTextMax); 
							break;
						case FVCOLUMN_GENRE:			FileViewMeta_GetString(pfr->pMeta, MF_GENRE, (LPCWSTR*)&pdi->item.pszText); break;
						case FVCOLUMN_YEAR:
							if (FileViewMeta_GetInt(pfr->pMeta, MF_YEAR, &nVal)) FileView_FormatYear(nVal, pdi->item.pszText, pdi->item.cchTextMax); 
							break;
						case FVCOLUMN_LENGTH:
							if (FileViewMeta_GetInt(pfr->pMeta, MF_LENGTH, &nVal)) FileView_FormatLength(nVal, pdi->item.pszText, pdi->item.cchTextMax); 
							break;
						case FVCOLUMN_BITRATE:
							if (FileViewMeta_GetInt(pfr->pMeta, MF_BITRATE, &nVal)) 
								FileView_FormatBitrate(nVal, pdi->item.pszText, pdi->item.cchTextMax); break;
						case FVCOLUMN_TRACK:
							if (!FileViewMeta_GetInt(pfr->pMeta, MF_TRACKNUM, &nVal)) nVal = -1;
							if (!FileViewMeta_GetInt(pfr->pMeta, MF_TRACKCOUNT, &nVal2)) nVal2 = -1;
							if (nVal != -1) FileView_FormatIntSlashInt(nVal, nVal2, pdi->item.pszText, pdi->item.cchTextMax); 
							break;
						case FVCOLUMN_DISC:
							if (!FileViewMeta_GetInt(pfr->pMeta, MF_DISCNUM, &nVal)) nVal = -1;
							if (!FileViewMeta_GetInt(pfr->pMeta, MF_DISCCOUNT, &nVal2)) nVal2 = -1;
							if (nVal != -1) FileView_FormatIntSlashInt(nVal, nVal2, pdi->item.pszText, pdi->item.cchTextMax); 
							break;
						case FVCOLUMN_COMMENT:		FileViewMeta_GetString(pfr->pMeta, MF_COMMENT, (LPCWSTR*)&pdi->item.pszText); break;
						case FVCOLUMN_PUBLISHER:		FileViewMeta_GetString(pfr->pMeta, MF_PUBLISHER, (LPCWSTR*)&pdi->item.pszText); break;
						case FVCOLUMN_COMPOSER:		FileViewMeta_GetString(pfr->pMeta, MF_COMPOSER, (LPCWSTR*)&pdi->item.pszText); break;
						case FVCOLUMN_ALBUMARTIST:	FileViewMeta_GetString(pfr->pMeta, MF_ALBUMARTIST, (LPCWSTR*)&pdi->item.pszText); break;
					}
				}
				break;
		}
	}
	if(LVIF_IMAGE & pdi->item.mask)
	{
		INT index;
		switch(pfr->fileType)
		{
			case FVFT_AUDIO:	index = 1; break;
			case FVFT_VIDEO:	index = 2; break;
			case FVFT_PLAYLIST: index = 3; break;
			default:			index = 0; break;
		}
		pdi->item.iImage = MLImageListI_GetRealIndex((FVS_ICONVIEW == (FVS_VIEWMASK & pfv->style)) ? hmlilFileTypesLarge : hmlilFileTypesSmall, index, rgbTextBk, rgbText);
	}
}

static void CALLBACK FileView_OnActivateListItemElapsed(HWND hdlg, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hdlg, idEvent);
	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	if (!hctrl) return;
	INT index = (INT)SendMessageW(hctrl, LVM_GETNEXTITEM, 0, (LPARAM)(LVNI_ALL | LVNI_FOCUSED));
	if (-1 == index)
	{
		LVITEM lvi;
		lvi.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
		lvi.state = LVIS_FOCUSED | LVIS_SELECTED;
		SendMessageW(hctrl, LVM_SETITEMSTATE, 0, (LPARAM)&lvi);
	}
}

static void FileList_OnSetFocus(HWND hdlg, NMHDR *phdr)
{
	SetTimer(hdlg, IDT_ACTIVATELISTITEM, DELAY_ACTIVATELISTITEM, FileView_OnActivateListItemElapsed);
}

static void FileList_OnGetInfoTip(HWND hdlg, NMLVGETINFOTIPW *pit)
{
	if (LVGIT_UNFOLDED & pit->dwFlags) pit->pszText[0] = L'\0';
	else StringCchCatW(pit->pszText, pit->cchTextMax, L"\r\n");

	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;

	FILEDATA *pfd = &pfv->fileData;
	if (pit->iItem < (INT)pfd->count)
	{
		size_t index = ((pfv->bAscending) ? pit->iItem : (pfd->count - pit->iItem - 1));
		FILERECORD *pfr = &pfd->pRec[pfd->pSort[index]];

		if (FVFT_UNKNOWN != pfr->fileType && 
			!FileViewMeta_Discover(pfv->fileData.szPath, pfr, FileView_OnMetaDataDiscovered, (ULONG_PTR)hdlg, 0))
		{
			pfv->infoTipFolded = (0 == (LVGIT_UNFOLDED & pit->dwFlags));
			pfv->infoTipIndex = pit->iItem;
		}

		INT len = lstrlenW(pit->pszText);
		FileView_FormatFileInfo(pfr, pit->pszText + len, pit->cchTextMax - len, FIF_TOOLTIP);
	}
}

static void FileList_OnKeyDown(HWND hdlg, NMLVKEYDOWN *pkd)
{
}

static void FileList_OnColumnClick(HWND hdlg, NMLISTVIEW *pnmv)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;

	UINT uColumn  = pfv->szColumns[pnmv->iSubItem].id;
	BOOL bAscending = (uColumn == pfv->sortColumn) ? !pfv->bAscending : TRUE;
	HCURSOR hc = SetCursor(LoadCursor(NULL, IDC_WAIT));
	FileView_SetSort(hdlg, uColumn, bAscending);
	SetCursor(hc);
}

static void FileList_OnItemChanged(HWND hdlg, NMLISTVIEW *pnmv)
{
	if (LVIF_STATE & pnmv->uChanged)
	{

		NMFVSTATECHANGED nmsc;
		nmsc.iFrom		= pnmv->iItem;
		nmsc.iTo			= pnmv->iItem;
		nmsc.uNewState	= pnmv->uNewState;
		nmsc.uOldState	= pnmv->uOldState;
		FileView_NotifyParent(hdlg, FVN_STATECHANGED, (NMHDR*)&nmsc);
	}
}

static void FileList_OnStateChanged(HWND hdlg, NMLVODSTATECHANGE *pnmsc)
{
		NMFVSTATECHANGED nmsc;
		nmsc.iFrom		= pnmsc->iFrom;
		nmsc.iTo			= pnmsc->iTo;
		nmsc.uNewState	= pnmsc->uNewState;
		nmsc.uOldState	= pnmsc->uOldState;
		FileView_NotifyParent(hdlg, FVN_STATECHANGED, (NMHDR*)&nmsc);
}

static HMENU activePopup = NULL;

void FileView_DisplayPopupMenu(HWND hdlg, UINT uMenu, UINT uFlags, POINT pt)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;
	
	HMENU hMenu = FileView_GetMenu(hdlg, uMenu);
	if (!hMenu) return;
	uMenu = FileViewMenu_GetMenuType(hdlg, pfv->hMenu, hMenu);
	if (WASABI_API_APP)
	{
		HACCEL szAccel[24] = {0};
		INT c = WASABI_API_APP->app_getAccelerators(hdlg, szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
		AppendMenuShortcuts(hMenu, szAccel, c, MSF_REPLACE);
	}

	NMFVMENU popup;
	ZeroMemory(&popup, sizeof(NMFVMENU));
	popup.hMenu		= hMenu;
	popup.uMenuType	= uMenu;
	popup.ptAction	= pt;

	activePopup = hMenu;

	if (0 == FileView_NotifyParent(hdlg, FVN_INITMENU, (NMHDR*)&popup))
	{
		SendMessageW(g_hwnd, WM_SETCURSOR, (WPARAM)hdlg, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
		popup.uCommand = (UINT)MediaLibrary_TrackPopup(hMenu, uFlags | TPM_RETURNCMD, 
								popup.ptAction.x, popup.ptAction.y, hdlg);

		if (0 == FileView_NotifyParent(hdlg, FVN_MENUCOMMAND, (NMHDR*)&popup) && 0 != popup.uCommand)
		{
			SendMessageW(hdlg, WM_COMMAND, MAKEWPARAM(popup.uCommand, 0), 0L);
		}
	}
	activePopup = NULL;
}

static void FileView_OnInitMenuPopup(HWND hdlg, HMENU hMenu, UINT iIndex, BOOL bWinMenu)
{
	FILEVIEW *pfv = GetFileView(hdlg);

	if (!pfv || hMenu == activePopup) return;

	UINT uType = FileViewMenu_GetMenuType(hdlg, pfv->hMenu, hMenu);
	if (((UINT)-1) != uType && FileViewMenu_GetSubMenu(hdlg, pfv->hMenu, uType))
	{
		if (WASABI_API_APP)
		{
			HACCEL szAccel[24] = {0};
			INT c = WASABI_API_APP->app_getAccelerators(hdlg, szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
			AppendMenuShortcuts(hMenu, szAccel, c, MSF_REPLACE);
		}

		NMFVMENU popup;
		ZeroMemory(&popup, sizeof(NMFVMENU));

		popup.hMenu = hMenu;
		popup.uMenuType = uType;
		FileView_NotifyParent(hdlg, FVN_INITMENU, (NMHDR*)&popup);
	}
}

static void FileView_OnUninitMenuPopup(HWND hdlg, HMENU hMenu)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;

	UINT uType = FileViewMenu_GetMenuType(hdlg, pfv->hMenu, hMenu);
	if (((UINT)-1) != uType)
	{
		NMFVMENU popup;
		ZeroMemory(&popup, sizeof(NMFVMENU));
		popup.hMenu = hMenu;
		popup.uMenuType = uType;
		FileView_NotifyParent(hdlg, FVN_UNINITMENU, (NMHDR*)&popup);
	}
}

static void FileView_DisplayColumnMenu(HWND hdlg)
{
	POINT pt;
	GetCursorPos(&pt);
	FileView_DisplayPopupMenu(hdlg, FVMENU_COLUMNS, TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, pt);
}

static void FileView_DisplayFileMenu(HWND hdlg)
{
	POINT pt;
	GetCursorPos(&pt);
	FileView_DisplayPopupMenu(hdlg, FVMENU_FILECONTEXT, TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, pt);
}

static void FileListHeader_OnRClick(HWND hdlg, NMHDR *phdr)
{
	FileView_DisplayColumnMenu(hdlg);
}

static BOOL FileListHeader_OnEndDrag(HWND hdlg, NMHEADER *phdr)
{
	PostMessage(hdlg, WM_COMMAND, MAKEWPARAM(ID_INTERNAL_UPDATE_COLUMN_INFO, 0), (LPARAM)phdr->hdr.hwndFrom);
	return FALSE;
}

static BOOL FileListHeader_OnItemChanging(HWND hdlg, NMHEADER *phdr)
{
	if (phdr->pitem && (HDI_WIDTH & phdr->pitem->mask))
	{
		FILEVIEW *pfv = GetFileView(hdlg);
		if (pfv && phdr->iItem < pfv->columnCount)
		{
			FILEVIEWCOLUMN *pc = &pfv->szColumns[phdr->iItem];
			if (phdr->pitem->cxy < pc->widthMin) phdr->pitem->cxy = pc->widthMin;
			if (pc->widthMax > 0 && phdr->pitem->cxy > pc->widthMax) phdr->pitem->cxy = pc->widthMax;
		}
	}
	return FALSE;
}

static void FileListHeader_OnItemChanged(HWND hdlg, NMHEADER *phdr)
{
	SendMessage(hdlg, WM_COMMAND, MAKEWPARAM(ID_INTERNAL_UPDATE_COLUMN_INFO, 0), (LPARAM)phdr->hdr.hwndFrom);
}

static void FileList_NotifyActivate(HWND hdlg, NMITEMACTIVATE *pnma, UINT uCode)
{
	NMFVFILEACTIVATE fva;

	KillTimer(hdlg, IDT_ACTIVATELISTITEM);

	fva.iFile = pnma->iItem;
	fva.ptAction = pnma->ptAction;
	fva.uKeyFlags = pnma->uKeyFlags;
	fva.uNewState = pnma->uNewState;
	fva.uOldState = pnma->uOldState;
	FileView_NotifyParent(hdlg, uCode, (NMHDR*)&fva);
}

static void FileList_OnClick(HWND hdlg, NMITEMACTIVATE *pnma)
{
	FileList_NotifyActivate(hdlg, pnma, NM_CLICK);
}

static void FileList_OnDblClick(HWND hdlg, NMITEMACTIVATE *pnma)
{
	FileList_NotifyActivate(hdlg, pnma, NM_DBLCLK);
}

static void FileList_OnRClick(HWND hdlg, NMITEMACTIVATE *pnma)
{
	FileList_NotifyActivate(hdlg, pnma, NM_RCLICK);
	FileView_DisplayFileMenu(hdlg);
}

static void FileList_OnRDblClick(HWND hdlg, NMITEMACTIVATE *pnma)
{
	FileList_NotifyActivate(hdlg, pnma, NM_RDBLCLK);
}

static INT_PTR FileView_OnNotify(HWND hdlg, INT ctrlId, NMHDR *phdr)
{
	switch(phdr->idFrom)
	{
		case IDC_FILELIST:
			switch(phdr->code)
			{
				case LVN_GETDISPINFOW:		FileList_OnGetDispInfo(hdlg, (NMLVDISPINFOW*)phdr); return TRUE;
				case NM_SETFOCUS:			FileList_OnSetFocus(hdlg, phdr); break;
				case LVN_GETINFOTIPW:		FileList_OnGetInfoTip(hdlg, (NMLVGETINFOTIPW*)phdr); break;
				case LVN_KEYDOWN:			FileList_OnKeyDown(hdlg, (NMLVKEYDOWN*)phdr); break;
				case LVN_COLUMNCLICK:		FileList_OnColumnClick(hdlg, (NMLISTVIEW*)phdr); break;
				case LVN_ITEMCHANGED:		FileList_OnItemChanged(hdlg, (NMLISTVIEW*)phdr); break;
				case LVN_ODSTATECHANGED:	FileList_OnStateChanged(hdlg, (NMLVODSTATECHANGE*)phdr); break;
				case NM_CLICK:				FileList_OnClick(hdlg, (NMITEMACTIVATE*)phdr); break;
				case NM_DBLCLK:				FileList_OnDblClick(hdlg, (NMITEMACTIVATE*)phdr); break;
				case NM_RCLICK:				FileList_OnRClick(hdlg, (NMITEMACTIVATE*)phdr); break;
				case NM_RDBLCLK:				FileList_OnRDblClick(hdlg, (NMITEMACTIVATE*)phdr); break;
				case NM_CUSTOMDRAW:			return FileList_OnCustomDraw(hdlg, (NMLVCUSTOMDRAW*)phdr);
				case NM_KILLFOCUS:			KillTimer(hdlg, IDT_ACTIVATELISTITEM); break;
			}
			break;
		case IDC_FILELIST_HEADER:
			switch(phdr->code)
			{
				case NM_RCLICK:			FileListHeader_OnRClick(hdlg, phdr); break;
				case HDN_ENDDRAG:		return FileListHeader_OnEndDrag(hdlg, (NMHEADER*)phdr); 
				case HDN_ITEMCHANGINGW:	return FileListHeader_OnItemChanging(hdlg, (NMHEADER*)phdr);
				case HDN_ITEMCHANGEDW:	FileListHeader_OnItemChanged(hdlg, (NMHEADER*)phdr); break;
			}
		case IDC_FOLDER_BROWSER:
			switch(phdr->code)
			{
				case FBN_SELCHANGED:
					{
						wchar_t szText[MAX_PATH*2] = {0};
						FolderBrowser_GetCurrentPath(phdr->hwndFrom, szText, sizeof(szText)/sizeof(szText[0]));
						HWND hedt = GetDlgItem(hdlg, IDC_EDT_PATH);
						if (hedt && hedt != GetFocus())
						{
							SetWindowTextW(hedt, szText);
						}
						FileView_UpdateFileView(hdlg, szText);
					}
					break;
			}
			break;
		case IDC_FILEVIEW_TOOLBAR:
			switch(phdr->code)
			{
				case FVN_INITMENU:
				case FVN_UNINITMENU:
					return FileView_NotifyParent(hdlg, phdr->code, phdr);
			}
			break;
	}
	return 0;
}

static void FileView_OnSetFont(HWND hdlg, BOOL bRedraw)
{
	FileView_AdjustSizes(hdlg);
	FileView_LayoutWindows(hdlg, bRedraw);
}

static BOOL FileView_OnSetStyle(HWND hdlg, UINT uStyle, UINT uStyleMask)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return FALSE;

	if (FVS_VIEWMASK & uStyleMask) FileView_OnSetView(hdlg, (FVS_VIEWMASK & uStyle), FALSE);
	uStyleMask &= ~FVS_VIEWMASK;
	pfv->style = (pfv->style & ~uStyleMask) | (uStyle & uStyleMask);
	if (FVS_IGNOREHIDDEN & uStyleMask)
	{
		HWND hctrl = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
		if (hctrl) SetWindowLongPtrW(hctrl, GWL_STYLE, 
						(GetWindowLongPtrW(hctrl, GWL_STYLE) & ~FBS_IGNOREHIDDEN) | ((FVS_IGNOREHIDDEN & uStyle) ? FBS_IGNOREHIDDEN : 0));
	}
	return TRUE;
}

static void FileView_InvertStyle(HWND hdlg, UINT style)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;
	FileView_OnSetStyle(hdlg, pfv->style ^ style, style);
	FileView_Refresh(hdlg, FALSE);
}

static UINT FileView_OnGetStyle(HWND hdlg)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	return (pfv) ? pfv->style : 0;
}

static BOOL FileView_OnSetSort(HWND hdlg, UINT uColumn, BOOL bAscending)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return FALSE;

	INT index = 0;
	for (index = 0; index < pfv->columnCount && uColumn != pfv->szColumns[index].id; index++);
	if (index == pfv->columnCount) return FALSE;

	if (uColumn  == (INT)pfv->sortColumn) 
	{
		if (pfv->bAscending == bAscending) return TRUE;
		pfv->bAscending = bAscending;
	}
	else 
	{
		pfv->sortColumn = uColumn;
		pfv->bAscending = bAscending;
		FileView_SortByColumn(&pfv->fileData, pfv->sortColumn);
	}

	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	if (hctrl)
	{
		MLSkinnedListView_DisplaySort(hctrl, index, pfv->bAscending);
		SendMessageW(hctrl, LVM_REDRAWITEMS, 0, pfv->fileData.count);
		InvalidateRect(hctrl, NULL, TRUE);
		UpdateWindow(hctrl);
	}

	return TRUE;
}

static UINT FileView_OnGetSort(HWND hdlg)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	return (pfv) ? MAKELONG(pfv->sortColumn, pfv->bAscending) : ((UINT)-1);
}

static INT FileView_OnGetColumnCount(HWND hdlg)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	return  (pfv) ? pfv->columnCount : 0;
}

typedef struct _CORDER
{
	INT id;
	INT order;
} CORDER;

static INT __cdecl QSort_CompareOrder(const void *p1, const void *p2)
{
	return (((CORDER*)p1)->order - ((CORDER*)p2)->order);
}

static INT FileView_OnGetColumnArray(HWND hdlg, INT iCount, UINT *puArray)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return -1;
	CORDER szProxy[512] = {0};
	INT count;
	for (count = 0; count < pfv->columnCount; count++) 
	{ 
		szProxy[count].id = pfv->szColumns[count].id;
		szProxy[count].order = pfv->szColumns[count].order;
	}
	qsort(szProxy, pfv->columnCount, sizeof(szProxy[0]), QSort_CompareOrder);
	for (count = 0; count < pfv->columnCount && count < iCount; count++)  puArray[count] = szProxy[count].id;

	return count;
}

static BOOL FileView_OnDeleteColumn(HWND hdlg, UINT uColumn)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return FALSE;

	INT index = 0;
	for (index = 0; index < pfv->columnCount && uColumn != pfv->szColumns[index].id; index++);
	if (index == pfv->columnCount) return FALSE;

	if (index < (pfv->columnCount - 1))
	{
		MoveMemory(&pfv->szColumns[index], &pfv->szColumns[index + 1], sizeof(FILEVIEWCOLUMN)*(pfv->columnCount - index - 1));
	}

	pfv->columnCount--;
	HWND hList = GetDlgItem(hdlg, IDC_FILELIST);
	if (hList && (FVS_DETAILVIEW == (FVS_VIEWMASK & pfv->style)))
	{
		SendMessageW(hList, LVM_DELETECOLUMN, index, 0L);

		UINT c = pfv->sortColumn;
		pfv->sortColumn = ((UINT)-1);

		if (c == uColumn)  
		{
			if (index > 0) index--;
			c = pfv->szColumns[index].id;
		}

		FileView_SetSort(hdlg, c, pfv->bAscending); 
	}

	return TRUE;
}

static INT FileView_OnInsertColumn(HWND hdlg, FVCOLUMN *pColumn)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return -1;

	INT registeredIndex;
	for( registeredIndex = 0; 
			registeredIndex < RegisteredColumnsCount && szRegisteredColumns[registeredIndex].id != pColumn->id; 
			registeredIndex++);

	if (szRegisteredColumns[registeredIndex].id != pColumn->id) return -1;

	INT index = 0;
	for (index = 0; index < pfv->columnCount && pColumn->id != pfv->szColumns[index].id; index++);
	if (index != pfv->columnCount) 
	{
		return index;
	}

	INT order = 0;
	UINT insertPos = (UINT)pfv->columnCount;

	if (FVCF_ORDER & pColumn->mask) 
	{
		order = pColumn->order;
		if (FVCO_DEFAULT_ORDER == order) order = szRegisteredColumns[registeredIndex].order;
		
		insertPos = 0;
		for (INT index = 0; index < pfv->columnCount; index++)
		{
			if (pfv->szColumns[index].order < order) insertPos = (index + 1);
			else break;
		}
	}
	else order = pfv->columnCount;
	
	
	if (insertPos < (UINT)pfv->columnCount) 
	{
		index = insertPos;
		MoveMemory(&pfv->szColumns[index + 1], &pfv->szColumns[index], sizeof(FILEVIEWCOLUMN)*(pfv->columnCount - index));
	}

	else index = pfv->columnCount;
	CopyMemory(&pfv->szColumns[index], &szRegisteredColumns[registeredIndex], sizeof(FILEVIEWCOLUMN));
	pfv->szColumns[index].order = order;
	if (FVCF_WIDTH & pColumn->mask)  pfv->szColumns[index].width = pColumn->width;
	if (FVCF_WIDTHMIN & pColumn->mask)  pfv->szColumns[index].widthMin = pColumn->widthMin;
	if (FVCF_WIDTHMAX & pColumn->mask)  pfv->szColumns[index].widthMax = pColumn->widthMax;

	if (pfv->szColumns[index].width < pfv->szColumns[index].widthMin) 
		pfv->szColumns[index].width = pfv->szColumns[index].widthMin;
	if (pfv->szColumns[index].widthMax > 0 && pfv->szColumns[index].width > pfv->szColumns[index].widthMax) 
		pfv->szColumns[index].width = pfv->szColumns[index].widthMax;

	pfv->columnCount++;

	HWND hList = GetDlgItem(hdlg, IDC_FILELIST);
	if (hList && (FVS_DETAILVIEW == (FVS_VIEWMASK & pfv->style)))
	{
		LVCOLUMNW lvc;
		FileView_InitializeListColumn(hdlg, &lvc, &pfv->szColumns[index]);
		SendMessageW(hList, LVM_INSERTCOLUMNW, insertPos, (LPARAM)&lvc);
	}

	UINT c = pfv->sortColumn;
	pfv->sortColumn = ((UINT)-1);
	FileView_SetSort(hdlg, c, pfv->bAscending); 
	return index;
}

static BOOL FileView_OnGetColumnName(HWND hdlg, UINT uColumn, INT cchTextMax, LPWSTR pszText)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv || !pszText) return FALSE;
	LPWSTR pszColumn = NULL;
	for(INT i = 0; i < pfv->columnCount; i++)
	{
		if (uColumn == pfv->szColumns[i].id)
		{
			pszColumn = pfv->szColumns[i].pszText;
			break;
		}
	}

	if (NULL == pszColumn)
	{
		for(INT i = 0; i < RegisteredColumnsCount; i++)
		{
			if (szRegisteredColumns[i].id == uColumn) 
			{
				pszColumn = szRegisteredColumns[i].pszText;
				return TRUE;
			}
		}
	}
	if (NULL != pszColumn)
	{
		if (IS_INTRESOURCE(pszColumn)) WASABI_API_LNGSTRINGW_BUF((UINT)(UINT_PTR)pszColumn, pszText, cchTextMax);
		else StringCchCopyW(pszText, cchTextMax, pszColumn);
		return TRUE;
	}

	return FALSE;
}

static INT FileView_OnGetColumnWidth(HWND hdlg, UINT uColumn)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return -1;

	INT index = 0;
	for (index = 0; index < pfv->columnCount && uColumn != pfv->szColumns[index].id; index++);

	return  (index == pfv->columnCount) ? -1 : pfv->szColumns[index].width;
}

static BOOL FileView_OnSetFileSystemInfo(HWND hdlg, FILESYSTEMINFO *pfsi)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
	return (hctrl) ? FolderBrowser_SetFileSystemInfo(hctrl, pfsi) : FALSE;
}

static BOOL FileView_OnGetFileSystemInfo(HWND hdlg, FILESYSTEMINFO *pfsi)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
	return (hctrl) ? FolderBrowser_GetFileSystemInfo(hctrl, pfsi) : FALSE;
}

static INT FileView_OnGetFileCount(HWND hdlg)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	return (pfv) ? (INT)pfv->fileData.count : 0;
}

static INT FileView_OnGetSelectedCount(HWND hdlg)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	return ((NULL != hctrl) ? (INT)SendMessageW(hctrl, LVM_GETSELECTEDCOUNT, 0, 0L) : 0);
}

static INT FileView_OnGetNextFile(HWND hdlg, INT iStart, UINT uFlags)
{
	INT iFile;
	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	UINT lvFlags = (uFlags & 0xFF0F);
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return -1;

	iFile = ((NULL != hctrl) ? (INT)SendMessageW(hctrl, LVM_GETNEXTITEM, iStart, (LPARAM)lvFlags) : -1); 
	if ((0x00F0 & uFlags) && -1 != iFile)
	{
		if (FVNF_PLAYABLE & uFlags)
		{
			size_t index = ((pfv->bAscending) ? iFile : (pfv->fileData.count - iFile - 1));
			FILERECORD *pfr = &pfv->fileData.pRec[pfv->fileData.pSort[index]];
			while (!FileView_IsTypePlayable(pfr->fileType, FVEF_ALLKNOWN))
			{
				iFile = (INT)SendMessageW(hctrl, LVM_GETNEXTITEM, iFile, (LPARAM)lvFlags);
				if (-1 == iFile) return -1;
				index = ((pfv->bAscending) ? iFile : (pfv->fileData.count - iFile - 1));
				pfr = &pfv->fileData.pRec[pfv->fileData.pSort[index]];
			}
		}
	}
	return iFile;
}

static BOOL FileView_OnGetFile(HWND hdlg, INT iFile, FVITEM *pFile)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv || ((size_t)iFile) >= pfv->fileData.count || !pFile) return FALSE;
	
	size_t index = ((pfv->bAscending) ? iFile : (pfv->fileData.count - iFile - 1));
	FILERECORD *pfr = &pfv->fileData.pRec[pfv->fileData.pSort[index]];

	if (FVIF_TEXT & pFile->mask)
	{
		if (NULL == pFile->pszText) return FALSE;
		
		if (pfr->extOffset != (lstrlenW(pfr->Info.cFileName) + 1)) pFile->pszText = pfr->Info.cFileName;
		else
		{
			HRESULT hr;
			hr = StringCchPrintfW(pFile->pszText, pFile->cchTextMax, L"%s.%s", 
					pfr->Info.cFileName, (pfr->Info.cFileName + pfr->extOffset));
			if (S_OK != hr) return FALSE;
		}
	}

	if (FVIF_STATE & pFile->mask)
	{
		HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
		if (!hctrl) return FALSE;
		pFile->state = (UINT)SendMessageW(hctrl, LVM_GETITEMSTATE, iFile, (LPARAM)pFile->stateMask);
	}

	if (FVIF_SIZE & pFile->mask)
	{
		pFile->dwSizeLow = pfr->Info.nFileSizeLow;
		pFile->dwSizeHigh = pfr->Info.nFileSizeHigh;
	}

	if (FVIF_ATTRIBUTES & pFile->mask) pFile->uAttributes = pfr->Info.dwFileAttributes;
	if (FVIF_CREATETIME & pFile->mask) pFile->ftCreationTime = pfr->Info.ftCreationTime;
	if (FVIF_ACCESSTIME & pFile->mask) pFile->ftLastAccessTime = pfr->Info.ftLastAccessTime;
	if (FVIF_WRITETIME & pFile->mask) pFile->ftLastWriteTime = pfr->Info.ftLastWriteTime;
	if (FVIF_TYPE & pFile->mask) pFile->wType = pfr->fileType;

	return TRUE;
}

static BOOL FileView_OnGetCurrentPath(HWND hdlg, LPWSTR pszBuffer, INT cchBuffer)
{
	if (!pszBuffer || !cchBuffer) return FALSE;
	pszBuffer[0] = L'\0';

	HWND hctrl = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
	if (!hctrl) return FALSE;

	return FolderBrowser_GetCurrentPath(hctrl, pszBuffer, cchBuffer);
}

static BOOL FileView_OnSetCurrentPath(HWND hdlg, LPCWSTR pszPath, BOOL bRedraw)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
	if (!hctrl) return FALSE;
	return FolderBrowser_SetCurrentPath(hctrl, pszPath, bRedraw);
}

static DWORD FileView_OnGetFolderSize(HWND hdlg, BOOL bSelectedOnly, DWORD *pdwSizeHigh)
{
	ULONGLONG size = 0;
	FILEVIEW *pfv = GetFileView(hdlg);
	if (pfv)
	{
		if (!bSelectedOnly)  size = pfv->fileData.folderSize;
		else 
		{
			HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
			if (hctrl)
			{
				INT i = -1;
				while( -1 != (i = (INT)SendMessage(hctrl, LVM_GETNEXTITEM, i, LVNI_SELECTED)))
				{
					size_t index = ((pfv->bAscending) ? i : (pfv->fileData.count - i - 1));
					FILERECORD *pfr = &pfv->fileData.pRec[pfv->fileData.pSort[index]];
					size += ((ULONGLONG)(((__int64)pfr->Info.nFileSizeHigh << 32) | pfr->Info.nFileSizeLow));
				}
			}
		}
	}

	if (pdwSizeHigh) *pdwSizeHigh = (DWORD)(size >> 32);
	return (DWORD)(MAXDWORD & size);
}

static BOOL FileView_OnGetStatusText(HWND hdlg, LPWSTR pszText, INT cchTextMax)
{
	if (!pszText || !cchTextMax) return FALSE;
	pszText[0] = L'\0';

	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return FALSE;

	pfv->statusIndex = -1;
	if (!pfv->fileData.count)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_FILEVIEW_EMPTYFOLDER, pszText, cchTextMax);
		return TRUE;
	}

	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	INT selectedCount = 0, selectedIndex = -1;
	if (hctrl)
	{
		selectedCount = (INT)SendMessageW(hctrl, LVM_GETSELECTEDCOUNT, 0, 0L);
		if (1 == selectedCount)
		{
			selectedIndex = (INT)SendMessageW(hctrl, LVM_GETNEXTITEM, -1, (LPARAM)FVNF_SELECTED);
		}
	}

	WCHAR szTemplate[256] = {0}, szSize[64] = {0};
	DWORD sizeLow, sizeHigh;

	if (1 == selectedCount && -1 != selectedIndex)
	{
		size_t index = ((pfv->bAscending) ? selectedIndex : (pfv->fileData.count - selectedIndex - 1));
		FILERECORD *pfr = &pfv->fileData.pRec[pfv->fileData.pSort[index]];
		if (FVFT_UNKNOWN != pfr->fileType && 
			!FileViewMeta_Discover(pfv->fileData.szPath, pfr, FileView_OnMetaDataDiscovered, (ULONG_PTR)hdlg, 0))
		{
			pfv->statusIndex = selectedIndex;
		}
		FileView_FormatFileInfo(pfr, pszText, cchTextMax, FIF_STATUS);
		return TRUE;
	}

	WASABI_API_LNGSTRINGW_BUF(((selectedCount < 1) ? IDS_FILEVIEWSTATUS_GROUPTEMPLATE : IDS_FILEVIEWSTATUS_SELECTEDGROUPTEMPLATE), 
								szTemplate, sizeof(szTemplate)/sizeof(szTemplate[0]));

	sizeLow = FileView_OnGetFolderSize(hdlg, (selectedCount > 1), &sizeHigh);
	WASABI_API_LNG->FormattedSizeString(szSize, sizeof(szSize)/sizeof(szSize[0]), (((__int64)sizeHigh << 32) | sizeLow));
	return  (S_OK == StringCchPrintfW(pszText, cchTextMax, szTemplate, ((selectedCount < 1) ? pfv->fileData.count : selectedCount), szSize));
}

#define IsKeyword(__keyword, __val) (CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE,(__keyword), -1, (__val), -1))
wchar_t *FileView_TagFunc(const wchar_t *tag, void *p)
{
	static WCHAR szBuffer[1024];
	LPCWSTR pszVal;
	INT nVal, nVal2;

	FILERECORD *pfr = (FILERECORD*)p;

	if (!pfr) return 0;

	if (pfr->pMeta)
	{
		if (IsKeyword(L"artist", tag)) return (FileViewMeta_GetString(pfr->pMeta, MF_ARTIST, &pszVal)) ? (LPWSTR)pszVal : 0;
		else if (IsKeyword(L"album", tag)) return (FileViewMeta_GetString(pfr->pMeta, MF_ALBUM, &pszVal)) ? (LPWSTR)pszVal : 0;
		else if (IsKeyword(L"title", tag)) return (FileViewMeta_GetString(pfr->pMeta, MF_TITLE, &pszVal)) ? (LPWSTR)pszVal : 0;
		else if (IsKeyword(L"albumartist", tag)) return (FileViewMeta_GetString(pfr->pMeta, MF_ALBUMARTIST, &pszVal)) ? (LPWSTR)pszVal : 0;
		else if (IsKeyword(L"comment", tag)) return (FileViewMeta_GetString(pfr->pMeta, MF_COMMENT, &pszVal)) ? (LPWSTR)pszVal : 0;
		else if (IsKeyword(L"composer", tag)) return (FileViewMeta_GetString(pfr->pMeta, MF_COMPOSER, &pszVal)) ? (LPWSTR)pszVal : 0;
		else if (IsKeyword(L"genre", tag)) return (FileViewMeta_GetString(pfr->pMeta, MF_GENRE, &pszVal)) ? (LPWSTR)pszVal : 0;
		else if (IsKeyword(L"publisher", tag)) return (FileViewMeta_GetString(pfr->pMeta, MF_PUBLISHER, &pszVal)) ? (LPWSTR)pszVal : 0;
		else if (IsKeyword(L"bitrate", tag)) 
		{ 
			if (!FileViewMeta_GetInt(pfr->pMeta, MF_BITRATE, &nVal)) return 0;
			FileView_FormatBitrate(nVal, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])); 
			return szBuffer; 
		}
		else if (IsKeyword(L"year", tag)) 
		{ 
			if (!FileViewMeta_GetInt(pfr->pMeta, MF_YEAR, &nVal)) return 0;
			FileView_FormatBitrate(nVal, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])); 
			return szBuffer; 
		}
		else if (IsKeyword(L"tracknumber", tag) || IsKeyword(L"track", tag) ) 
		{ 
			if (!FileViewMeta_GetInt(pfr->pMeta, MF_TRACKNUM, &nVal)) return 0;
			if (!FileViewMeta_GetInt(pfr->pMeta, MF_TRACKCOUNT, &nVal2)) nVal2 = -1;
			FileView_FormatIntSlashInt(nVal, nVal2, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])); 
			return szBuffer; 
		}
		else if (IsKeyword(L"disc", tag)) 
		{ 
			if (!FileViewMeta_GetInt(pfr->pMeta, MF_DISCNUM, &nVal)) return 0;
			if (!FileViewMeta_GetInt(pfr->pMeta, MF_DISCCOUNT, &nVal2)) nVal2 = -1;
			FileView_FormatIntSlashInt(nVal, nVal2, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])); 
			return szBuffer; 
		}
	}
	if (IsKeyword(L"filename", tag)) 
	{
		if (pfr->extOffset == (lstrlenW(pfr->Info.cFileName) + 1))
		{
			StringCchPrintfW(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), L"%s.%s",
								pfr->Info.cFileName, (pfr->Info.cFileName + pfr->extOffset));
			return szBuffer;
		}
		return pfr->Info.cFileName;
	}
	return 0;
}

static BOOL FileView_OnIsFilePlayable(HWND hdlg, INT iFile, UINT uEnqueueFilter)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv || ((size_t)iFile) >= pfv->fileData.count) return FALSE;
	size_t index = ((pfv->bAscending) ? iFile : (pfv->fileData.count - iFile - 1));
	FILERECORD *pfr = &pfv->fileData.pRec[pfv->fileData.pSort[index]];
	return FileView_IsTypePlayable(pfr->fileType, uEnqueueFilter);
}

static INT FileView_OnEnqueueSelection(HWND hdlg, UINT uEnqueueFilter, INT *pnFocused)
{
	FILERECORD *pfr;
	wchar_t szPath[2*MAX_PATH] = {0}, szAtf[2048] = {0};

	enqueueFileWithMetaStructW efs = {0};
	waFormatTitleExtended ft = {0};

	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv || 0 == uEnqueueFilter) return 0;

	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	if (!hctrl) return 0;
	INT sCount = (INT)SendMessageW(hctrl, LVM_GETSELECTEDCOUNT, 0, 0L);
	if (0 == sCount) return 0;

	ft.spec = NULL;
	ft.TAGFUNC = FileView_TagFunc;
	ft.TAGFREEFUNC = NULL;

	ft.useExtendedInfo = 1;

	sCount = 0;
	INT iFile = -1, iFocused = -1;

	if (pnFocused)
	{
		*pnFocused = -1;
		iFocused = (INT)SendMessageW(hctrl, LVM_GETNEXTITEM, iFile, (LPARAM)LVIS_FOCUSED);
	}
	
	while (-1 != (iFile = (INT)SendMessageW(hctrl, LVM_GETNEXTITEM, iFile, (LPARAM)LVIS_SELECTED)))
	{
		size_t index = ((pfv->bAscending) ? iFile : (pfv->fileData.count - iFile - 1));
		pfr = &pfv->fileData.pRec[pfv->fileData.pSort[index]];

		if (FileView_IsTypePlayable(pfr->fileType, uEnqueueFilter) && 
			PathCombineW(szPath, pfv->fileData.szPath, pfr->Info.cFileName))
		{
			if (pfr->extOffset == (lstrlenW(pfr->Info.cFileName) + 1))
			{
				StringCchCatW(szPath, sizeof(szPath)/sizeof(szPath[0]), L".");
				StringCchCatW(szPath, sizeof(szPath)/sizeof(szPath[0]), (pfr->Info.cFileName + pfr->extOffset));
			}

			if (pfr->pMeta && METATYPE_PLAYLIST != pfr->pMeta->type)
			{
				szAtf[0] = L'\0';
				ft.filename = szPath;
				ft.out = szAtf;
				ft.out_len = sizeof(szAtf)/sizeof(szAtf[0]);
				ft.p = pfr;
				SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&ft, IPC_FORMAT_TITLE_EXTENDED);
				FileViewMeta_GetInt(pfr->pMeta, MF_LENGTH, &efs.length);
				if (szAtf[0]) efs.title = szAtf;
				else if (!FileViewMeta_GetString(pfr->pMeta, MF_TITLE, &efs.title)) efs.title = NULL;
			}
			else
			{
				efs.length	= -1;
				efs.title	= NULL;
			}

			efs.filename = szPath;
			efs.ext      = NULL;

			if (iFocused == iFile && pnFocused) *pnFocused = (INT)SENDWAIPC(plugin.hwndParent, IPC_GETLISTLENGTH, 0);

			if(SENDWAIPC(plugin.hwndParent, IPC_ENQUEUEFILEW, (WPARAM)&efs)) sCount++;
			else if (iFocused == iFile && pnFocused) *pnFocused = -1;
		}
	}

	return sCount;
}

static void FileView_OnPlaySelection(HWND hdlg, BOOL bShiftPressed)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return;
	BOOL bPlay = ((FALSE == bShiftPressed) == (0 == (FVS_ENQUEUE & pfv->style)));
	if (bPlay) SENDWAIPC(plugin.hwndParent, IPC_DELETE, 0);
	if (FileView_OnEnqueueSelection(hdlg, FVEF_ALLKNOWN, NULL) && bPlay)
	{
		SENDWAIPC(plugin.hwndParent, IPC_SETPLAYLISTPOS, 0);
		SENDWAIPC(plugin.hwndParent, IPC_STARTPLAY, 0);
	}
}

static void FileView_UpdateColumnInfo(HWND hdlg, HWND hHeader)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv || NULL == hHeader || 0 == pfv->columnCount) return;

	INT count = (INT)SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0L);
	if (count)
	{
		HDITEM item;
		item.mask = HDI_ORDER | HDI_WIDTH;
		if (count > pfv->columnCount) count = pfv->columnCount;

		for (int i = 0; i < count; i++)
		{
			if (SendMessage(hHeader, HDM_GETITEM, (WPARAM)i, (LPARAM)&item))
			{
				pfv->szColumns[i].order = item.iOrder;
				pfv->szColumns[i].width = item.cxy;
			}
		}
	}
}

static void FileView_OnCommand(HWND hdlg, INT eventId, INT ctrlId, HWND hwndCtrl)
{
	FILEVIEW *pfv = GetFileView(hdlg);

	switch (ctrlId)
	{
		case IDC_EDT_PATH:
			switch(eventId)
			{
				case EN_CHANGE: 
					if (hwndCtrl == GetFocus())
					{
						HWND hctrl = GetDlgItem(hdlg, IDC_FOLDER_BROWSER);
						if (IsWindow(hctrl))
						{
							WCHAR szPath[MAX_PATH*2] = {0};
							GetWindowTextW(hwndCtrl, szPath, sizeof(szPath)/sizeof(szPath[0]));
							FolderBrowser_SetCurrentPath(hctrl, szPath, TRUE);
							PostMessageW(hdlg, FVM_REFRESH, 0, 0L);
						}
					}
					break;
			}
			break;

		case ID_FILEVIEW_SETMODE_ICON:		FileView_OnSetView(hdlg, FVS_ICONVIEW, TRUE); break;
		case ID_FILEVIEW_SETMODE_LIST:		FileView_OnSetView(hdlg, FVS_LISTVIEW, TRUE); break;
		case ID_FILEVIEW_SETMODE_DETAIL:		FileView_OnSetView(hdlg, FVS_DETAILVIEW, TRUE); break;
		case ID_FILEVIEW_REFRESH:			FileView_OnRefresh(hdlg, TRUE, TRUE); break; 
		case ID_FILEVIEW_PLAYSELECTION:
			if (1 != eventId || GetFocus() == GetDlgItem(hdlg, IDC_FILELIST))
				FileView_OnPlaySelection(hdlg, FALSE); 
			else if (1 == eventId)
			{
				HWND hFocus = GetFocus();
				if (hFocus == GetDlgItem(hdlg, IDC_EDT_PATH) || 
					hFocus == GetDlgItem(hdlg, IDC_FOLDER_BROWSER) ||
					IsChild(GetDlgItem(hdlg, IDC_FOLDER_BROWSER), hFocus))
					SendMessageW(hdlg, WM_NEXTDLGCTL, 0, MAKELPARAM(0, 0));
			}
			break; 
		case ID_FILEVIEW_PLAYSELECTION_SHIFT:
			if (1 != eventId || GetFocus() == GetDlgItem(hdlg, IDC_FILELIST))
				FileView_OnPlaySelection(hdlg, TRUE); 
			break; 
		case ID_FILEVIEW_SELECT_ALL:
			if (1 != eventId || GetFocus() == GetDlgItem(hdlg, IDC_FILELIST))
			{
				LVITEMW lvi;
				lvi.stateMask = LVIS_SELECTED;
				lvi.state = LVIS_SELECTED;
				SendMessageW(GetDlgItem(hdlg, IDC_FILELIST), LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
			}
			else if (1 == eventId)
			{
				HWND hFocus = GetFocus();
				if (hFocus == GetDlgItem(hdlg, IDC_EDT_PATH)) SendMessageW(hFocus, EM_SETSEL, 0, -1);
			}
			break;
		case IDM_FILEVIEW_SORTASCENDING:
			if (pfv) FileView_OnSetSort(hdlg, pfv->sortColumn, !pfv->bAscending);
			break;
		case IDM_FILEVIEW_HIDEEXTENSION:	 FileView_InvertStyle(hdlg, FVS_HIDEEXTENSION); break;
		case IDM_FILEVIEW_IGNOREHIDDEN:	FileView_InvertStyle(hdlg, FVS_IGNOREHIDDEN); break;
		case IDM_FILEVIEW_SHOWAUDIO:		FileView_InvertStyle(hdlg, FVS_SHOWAUDIO); break;
		case IDM_FILEVIEW_SHOWVIDEO:		FileView_InvertStyle(hdlg, FVS_SHOWVIDEO); break;
		case IDM_FILEVIEW_SHOWPLAYLIST:	FileView_InvertStyle(hdlg, FVS_SHOWPLAYLIST); break;
		case IDM_FILEVIEW_SHOWUNKNOWN:	FileView_InvertStyle(hdlg, FVS_SHOWUNKNOWN); break;
		case ID_INTERNAL_UPDATE_COLUMN_INFO: FileView_UpdateColumnInfo(hdlg, hwndCtrl); break;
	}
	if (ctrlId >= IDM_COLUMN_SHOW_MIN && ctrlId <= IDM_COLUMN_SHOW_MAX)
	{
		INT cid = ctrlId - IDM_COLUMN_SHOW_MIN;
		INT index;
		for (index = 0; index < pfv->columnCount && cid != pfv->szColumns[index].id; index++);
		if (index == pfv->columnCount) 
		{
			FVCOLUMN fvc;
			fvc.id = cid;
			fvc.order = FVCO_DEFAULT_ORDER;
			fvc.mask = FVCF_ORDER;
			FileView_InsertColumn(hdlg, &fvc);
		}
		else FileView_DeleteColumn(hdlg, cid);
	}
	else if (ctrlId >= IDM_COLUMN_ARRANGE_MIN && ctrlId <= IDM_COLUMN_ARRANGE_MAX)
	{
		if (pfv && pfv->sortColumn != (ctrlId - IDM_COLUMN_ARRANGE_MIN)) 
			FileView_OnSetSort(hdlg, ctrlId - IDM_COLUMN_ARRANGE_MIN, pfv->bAscending);
	}
}

static BOOL FileView_OnSetFileState(HWND hdlg, INT iFile, FVITEM *pFile)
{
	if (!pFile) return FALSE;
	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	if (!hctrl) return FALSE;
	LVITEMW lvi;
	lvi.state = pFile->state;
	lvi.stateMask = pFile->stateMask;
	return (BOOL)SendMessageW(hctrl, LVM_SETITEMSTATE, (WPARAM)iFile, (LPARAM)&lvi);
}

static HMENU FileView_OnGetMenu(HWND hdlg, UINT uMenuType)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return NULL;
	if (!pfv->hMenu)
	{
		pfv->hMenu = FileViewMenu_Initialize();
		if (!pfv->hMenu) return NULL;
	}
	return FileViewMenu_GetSubMenu(hdlg, pfv->hMenu, uMenuType);
}

static UINT FileView_OnGetActionCommand(HWND hdlg, UINT uAction)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	if (!pfv) return 0;

	switch(uAction)
	{
		case FVA_PLAY: 
			return (0 == (FVS_ENQUEUE & pfv->style)) ? ID_FILEVIEW_PLAYSELECTION : ID_FILEVIEW_PLAYSELECTION_SHIFT;
		case FVA_ENQUEUE: 
			return (0 != (FVS_ENQUEUE & pfv->style)) ? ID_FILEVIEW_PLAYSELECTION : ID_FILEVIEW_PLAYSELECTION_SHIFT;
	}
	return 0;
}

static INT FileView_OnHitTest(HWND hdlg, FVHITTEST *pht)
{
	HWND hctrl = GetDlgItem(hdlg, IDC_FILELIST);
	if (!pht || !hctrl) return -1;

	LVHITTESTINFO ht;
	ht.pt = pht->pt;
	MapWindowPoints(hdlg, hctrl, &ht.pt, 1);

	INT r = (INT)SendMessageW(hctrl, LVM_HITTEST, 0, (LPARAM)&ht);
	pht->iItem = ht.iItem;
	pht->uFlags = (0xFF & ht.flags);
	return r;
}

static INT FileView_OnGetDividerPos(HWND hdlg)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	return (pfv) ? pfv->yDivider : 0;
}

static INT FileView_OnSetDividerPos(HWND hdlg, INT nPos, UINT uFlags)
{
	FILEVIEW *pfv = GetFileView(hdlg);
	RECT rc;
	if (!pfv) return 0;

	if (FVRF_VALIDATE & uFlags)
	{
		if (nPos < pfv->yDivider && GetWindowRect(GetDlgItem(hdlg, IDC_FOLDER_BROWSER), &rc))
		{
			MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)&rc, 2);
			if (((nPos + pfv->cyDivider) -  rc.top) < FOLDERBROWSER_MIN_HEIGHT) 
				nPos = rc.top  + (FOLDERBROWSER_MIN_HEIGHT - pfv->cyDivider);
		}

		if (nPos > pfv->yDivider && GetClientRect(hdlg, &rc))
		{
			if ((nPos + pfv->cyDivider) > (rc.bottom - FILELIST_MIN_HEIGHT))
				nPos = rc.bottom - FILELIST_MIN_HEIGHT - pfv->cyDivider;
		}
	}

	if (nPos != pfv->yDivider)
	{
		pfv->yDivider = nPos;
		if (0 == (FVRF_NOREDRAW & uFlags))
		{
			FileView_LayoutWindows(hdlg, TRUE);
			UpdateWindow(hdlg);
		}
	}

	return pfv->yDivider;
}

static INT_PTR CALLBACK FileView_DialogProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return FileView_OnInitDialog(hdlg, (HWND)wParam, lParam);
		case WM_DESTROY:				FileView_OnDestroy(hdlg); return TRUE;
		case WM_WINDOWPOSCHANGED:	FileView_OnWindowPosChanged(hdlg, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:				FileView_OnCommand(hdlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam); return TRUE;
		case WM_NOTIFY:				MSGRESULT(hdlg, FileView_OnNotify(hdlg, (INT)wParam, (LPNMHDR) lParam)); 
		case WM_SETFONT:				FileView_OnSetFont(hdlg, LOWORD(lParam)); break;
		case WM_INITMENUPOPUP:		FileView_OnInitMenuPopup(hdlg, (HMENU)wParam, LOWORD(lParam), HIWORD(lParam)); break;
		case WM_UNINITMENUPOPUP:		FileView_OnUninitMenuPopup(hdlg, (HMENU)wParam); break;

		case FVM_SETROOT:			MSGRESULT(hdlg, FileView_OnSetRoot(hdlg, (LPCWSTR)lParam));
		case FVM_GETROOT:			MSGRESULT(hdlg, FileView_OnGetRoot(hdlg, (LPCWSTR)lParam, (INT)wParam));
		case FVM_REFRESH:			MSGRESULT(hdlg, FileView_OnRefresh(hdlg, (BOOL)wParam, TRUE));
		case FVM_SETSTYLE:			MSGRESULT(hdlg, FileView_OnSetStyle(hdlg, (UINT)lParam, (UINT)wParam));
		case FVM_GETSTYLE:			MSGRESULT(hdlg, FileView_OnGetStyle(hdlg));
		case FVM_SETSORT:			MSGRESULT(hdlg, FileView_OnSetSort(hdlg, LOWORD(wParam), HIWORD(wParam)));
		case FVM_GETSORT:			MSGRESULT(hdlg, FileView_OnGetSort(hdlg));
		case FVM_GETCOLUMNCOUNT:		MSGRESULT(hdlg, FileView_OnGetColumnCount(hdlg));
		case FVM_GETCOLUMNARRAY:		MSGRESULT(hdlg, FileView_OnGetColumnArray(hdlg, (INT)wParam, (UINT*)lParam)); 
		case FVM_DELETECOLUMN:		MSGRESULT(hdlg, FileView_OnDeleteColumn(hdlg, (UINT)wParam));
		case FVM_INSERTCOLUMN:		MSGRESULT(hdlg, FileView_OnInsertColumn(hdlg, (FVCOLUMN*)lParam));
		case FVM_GETCOLUMNAME:		MSGRESULT(hdlg, FileView_OnGetColumnName(hdlg, (UINT)LOWORD(wParam), (INT)HIWORD(wParam), (LPWSTR)lParam));
		case FVM_SETFILESYSTEMINFO:	MSGRESULT(hdlg, FileView_OnSetFileSystemInfo(hdlg, (FILESYSTEMINFO*)lParam));
		case FVM_GETFILESYSTEMINFO:	MSGRESULT(hdlg, FileView_OnGetFileSystemInfo(hdlg, (FILESYSTEMINFO*)lParam));
		case FVM_GETFILECOUNT:		MSGRESULT(hdlg, FileView_OnGetFileCount(hdlg));
		case FVM_GETSELECTEDCOUNT:	MSGRESULT(hdlg, FileView_OnGetSelectedCount(hdlg));
		case FVM_GETNEXTFILE:		MSGRESULT(hdlg, FileView_OnGetNextFile(hdlg, (INT)wParam, (UINT)lParam));
		case FVM_GETFILE:			MSGRESULT(hdlg, FileView_OnGetFile(hdlg, (INT)wParam, (FVITEM*)lParam));
		case FVM_GETCURRENTPATH:		MSGRESULT(hdlg, FileView_OnGetCurrentPath(hdlg, (LPWSTR)lParam, (INT)wParam));
		case FVM_SETCURRENTPATH:		MSGRESULT(hdlg, FileView_OnSetCurrentPath(hdlg, (LPCWSTR)lParam, (BOOL)wParam));

		case FVM_GETFOLDERBROWSER:	MSGRESULT(hdlg, GetDlgItem(hdlg, IDC_FOLDER_BROWSER));
		case FVM_GETFOLDERSIZE:		MSGRESULT(hdlg, FileView_OnGetFolderSize(hdlg, (BOOL)wParam, (DWORD*)lParam));
		case FVM_GETSTATUSTEXT:		MSGRESULT(hdlg, FileView_OnGetStatusText(hdlg, (LPWSTR)lParam, (INT)wParam));
		case FVM_ENQUEUESELECTION:	MSGRESULT(hdlg, FileView_OnEnqueueSelection(hdlg, (UINT)wParam, (INT*)lParam));
		case FVM_ISFILEPLAYABLE:		MSGRESULT(hdlg, FileView_OnIsFilePlayable(hdlg, (INT)lParam, (UINT)wParam));
		case FVM_SETFILESTATE:		MSGRESULT(hdlg, FileView_OnSetFileState(hdlg, (INT)wParam, (FVITEM*)lParam));
		case FVM_GETMENU:			MSGRESULT(hdlg, FileView_OnGetMenu(hdlg, (UINT)wParam));
		case FVM_GETACTIONCMD:		MSGRESULT(hdlg, FileView_OnGetActionCommand(hdlg, (UINT)wParam));
		case FVM_HITTEST:			MSGRESULT(hdlg, FileView_OnHitTest(hdlg, (FVHITTEST*)lParam));
		case FVM_GETCOLUMNWIDTH:		MSGRESULT(hdlg, FileView_OnGetColumnWidth(hdlg, (UINT)wParam));	
		case FVM_GETDIVIDERPOS:		MSGRESULT(hdlg, FileView_OnGetDividerPos(hdlg));	
		case FVM_SETDIVIDERPOS:		MSGRESULT(hdlg, FileView_OnSetDividerPos(hdlg, (INT)wParam, (UINT)lParam));	
	}
	return 0;
}