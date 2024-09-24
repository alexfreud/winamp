#include "./folderbrowser.h"
#include "./folderbrowser_internal.h"
#include "./stringvector.h"
#include <vector>
#include "../Winamp/wa_dlg.h"
#include "./skinnedlistbox.h"
#include "./colors.h"

#include <windowsx.h>
#include <strsafe.h>



static LRESULT CALLBACK FolderBrowser_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


typedef struct _FBITEM
{
	INT		index;
	DWORD	styles;
} FBITEM;

typedef struct _FBCOLUMN
{		
	INT bufferOffset;
	INT count;
	INT firstVisible;
	INT firstSelected;
	INT	width;
	BOOL autoAdjust;
	FBITEM *pItems;
} FBCOLUMN;

 

typedef struct _FBDATA
{
	COLORREF		rgbBk;
	COLORREF		rgbText;
	std::vector<FBCOLUMN>	*pColumns;
	StringVector	*pBuffer;
	HWND				hwndDraw;
	HWND				hwndActive;
	LPWSTR			pszRoot;
	int				focusedColumn;
	// filesystem
	FILESYSTEMINFO	filesystem;
} FBDATA;

static int clickoffs = 0;
static size_t hiddenActive	= -1;
static size_t sizerActive	= -1;
static size_t sizerHover	= -1;			
#define GetFolderBrowser(__hwnd) ((FBDATA*)(LONG_PTR)(LONGX86)GetWindowLongPtrW((__hwnd), 0))


BOOL RegisterFolderBrowserControl(HINSTANCE hInstance)
{
	WNDCLASSW wc;
	if (GetClassInfoW(hInstance, FOLDERBROWSER_NAME, &wc)) return TRUE;
	ZeroMemory(&wc, sizeof(WNDCLASSW));

	wc.hInstance		= hInstance;
	wc.lpszClassName	= FOLDERBROWSER_NAME;
	wc.lpfnWndProc	= FolderBrowser_WindowProc;
	wc.style			= CS_DBLCLKS | CS_GLOBALCLASS;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(FBDATA*);
	
	return ( 0 != RegisterClassW(&wc));
	
}



BOOL FolderBrowser_CustomizeListBox(HWND hwndListbox);


__inline size_t FolderBrowser_GetListBoxColumn(HWND hwndList)
{
	SetLastError(0);
	size_t c = (size_t)GetWindowLongPtrW(hwndList, GWLP_USERDATA);
	return (ERROR_SUCCESS == GetLastError()) ?  c : ((size_t)-1);
}

static BOOL FolderBrowser_GetAdjustedClientRect(HWND hwnd, RECT *prc)
{	
	if (!GetClientRect(hwnd, prc)) 
		return FALSE;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	if (!GetScrollInfo(hwnd, SB_HORZ, &si)) ZeroMemory(&si, sizeof(SCROLLINFO));
	prc->left -= si.nPos;
	return TRUE;
}
static BOOL PrepareDrawingListBox(HWND hwndList, FBCOLUMN *pc, LONG height, size_t columnId)
{
	if (-1 != columnId && columnId == (size_t)GetWindowLongPtrW(hwndList, GWLP_USERDATA))
		return TRUE;
	
	
	SetWindowLongPtrW(hwndList, GWLP_USERDATA, (LONGX86)columnId);
	SetWindowPos(hwndList, NULL, 0, 0, pc->width, height, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSENDCHANGING);
	if (pc->count)
	{					
		SendMessageW(hwndList, LB_SETCOUNT, pc->count, 0L);
		SendMessageW(hwndList, LB_SETTOPINDEX, pc->firstVisible, 0L);
	}
	else
	{
		SendMessageW(hwndList, LB_SETCOUNT, 1, 0L);			// ** Keep this two messages 
		SendMessageW(hwndList, LB_SETTOPINDEX, 0, 0L);		// ** for skinned scrollbars
	}
	MLSkinnedScrollWnd_UpdateBars(hwndList, FALSE);
	
	return TRUE;
}

static void RefreshListBoxNC(HWND hHost, HWND hList, POINT ptViewport)
{
	UINT flags = DCX_PARENTCLIP | DCX_CACHE | DCX_WINDOW | DCX_CLIPSIBLINGS |
							DCX_INTERSECTUPDATE | DCX_VALIDATE;
	HDC hdc = GetDCEx(hHost, NULL, flags);
	if (NULL != hdc)
	{
		POINT ptOrig;
		SetViewportOrgEx(hdc, ptViewport.x, ptViewport.y, &ptOrig);
		SendMessageW(hList, WM_PRINT, (WPARAM)hdc, (LPARAM)PRF_NONCLIENT);
		SetViewportOrgEx(hdc, ptOrig.x, ptOrig.y, NULL);
		ReleaseDC(hHost, hdc);
	}
}

static void FolderBrowser_UpdateScrollInfo(HWND hwnd)
{
	RECT rc;

	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return;

	GetClientRect(hwnd, &rc);
	LONG totalWidth = 0;
	for(size_t i = 0; i < pfb->pColumns->size(); i++)
	{
		totalWidth  += (pfb->pColumns->at(i).width + SIZER_WIDTH);
	}
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	if (GetScrollInfo(hwnd, SB_HORZ, &si) && si.nMax != totalWidth)
	{
		INT dx = 0;
		si.fMask = SIF_RANGE | SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nMax = totalWidth;
		if (si.nPage != rc.right - rc.left)
		{
			si.nPage = rc.right - rc.left;
			si.fMask |= SIF_PAGE;
		}
		if ((si.nPos + si.nPage) > (UINT)si.nMax && si.nPos > si.nMin) si.nMax = si.nPos + si.nPage;
		SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);
		
		RECT rw;
		if (dx && pfb->hwndActive && 
			(WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE))
			&& GetWindowRect(pfb->hwndActive, &rw))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
			SetWindowPos(pfb->hwndActive, NULL, rw.left + dx, rw.top, 0, 0, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOSIZE);
		}
	}
}
static INT FolderBrowser_GetPreferredColumnWidth(HWND hwnd, size_t columnIndex)
{
	FBCOLUMN *pc;
	INT nameWidth = 0, prevMaxLen = 0;
	HDC hdc;
	HFONT hf, hfo = NULL;
	SIZE size;
	size_t count;

	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return -1;
	if (columnIndex >= pfb->pColumns->size()) return -1;

	pc = &pfb->pColumns->at(columnIndex);
	count = pc->bufferOffset + pc->count;
	if (count > pfb->pBuffer->Count())  count = pfb->pBuffer->Count();

	hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (!hdc) return -1;

	hf = (HFONT)SendMessageW((pfb->hwndDraw) ? pfb->hwndDraw : hwnd, WM_GETFONT, 0, 0L);
	if (NULL == hf) hf = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
	if (NULL != hf) hfo = (HFONT)SelectObject(hdc, hf);

	for (size_t i = pc->bufferOffset; i < count; i++)
	{
		LPCWSTR pszText = pfb->pBuffer->GetString(i);
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
	ReleaseDC(hwnd, hdc);

	nameWidth += 20;
	if (nameWidth < COLUMN_MIN_WIDTH) nameWidth = COLUMN_MIN_WIDTH;
	if (nameWidth > COLUMN_MAX_WIDTH) nameWidth = COLUMN_MAX_WIDTH;

	return nameWidth;
}

static StringVector *g_pCompareBuffer = NULL;
static INT g_szCompareOffset = 0;
static INT __cdecl FolderBrowser_CompareFolderNames(const void *elem1, const void *elem2)
{
	return (CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,  
										g_pCompareBuffer->GetString(g_szCompareOffset + ((FBITEM*)elem1)->index), -1,
										g_pCompareBuffer->GetString(g_szCompareOffset + ((FBITEM*)elem2)->index), -1) - 2);
}

static void FolderBrowser_SortColumn(HWND hwnd, size_t columnIndex)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || columnIndex >= pfb->pColumns->size()) return;

	FBCOLUMN *pc = &pfb->pColumns->at(columnIndex);
	if (pc->count == 0 || !pc->pItems) return;

	g_pCompareBuffer = pfb->pBuffer;
	g_szCompareOffset = pc->bufferOffset;
	qsort(pc->pItems, pc->count, sizeof(FBITEM), FolderBrowser_CompareFolderNames);
	g_pCompareBuffer = NULL;
}

typedef struct _FINDKEY
{
	LPCWSTR			pszKey;
	INT				cchKey;
	FBCOLUMN			*pCol;
	INT				foundIndex;
	StringVector	*pBuffer;
} FINDKEY;

static INT __cdecl FolderBrowser_FindKey(const void *key, const void *elem)
{
	FINDKEY *pfk = (FINDKEY*)key;
	LPCWSTR pszTest = pfk->pBuffer->GetString(pfk->pCol->bufferOffset + ((FBITEM*)elem)->index);
	return (CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, pfk->pszKey, pfk->cchKey,
								pfk->pBuffer->GetString(pfk->pCol->bufferOffset + ((FBITEM*)elem)->index), -1) - 2);
}

static INT FoderBrowser_FindFolder(StringVector *pBuffer, FBCOLUMN *pColumn, LPCWSTR pszFolder, INT cchFolder)
{
	FINDKEY fk;
	fk.pCol = pColumn;
	fk.pszKey = pszFolder;
	fk.cchKey = cchFolder;
	fk.pBuffer = pBuffer;
	if (!pBuffer || !pszFolder) return -1;

	FBITEM *pi = (FBITEM*)bsearch(&fk, pColumn->pItems, pColumn->count, sizeof(FBITEM), FolderBrowser_FindKey);
	if (!pi) return -1;
	return (INT)(pi - pColumn->pItems);
}

static INT FolderBrowser_AddColumn(HWND hwnd, LPCWSTR pszPath, INT cchPath, INT width, BOOL bAutoAdjust)
{
	HANDLE hFile;
	WIN32_FIND_DATAW fd = {0};
	wchar_t szSearch[2 * MAX_PATH + 4] = {0};

	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || !pszPath) return -1;
	if (cchPath < 0) cchPath = lstrlenW(pszPath);
	if (S_OK != StringCchCopyNW(szSearch, sizeof(szSearch)/sizeof(szSearch[0]), pszPath, cchPath) ||
		S_OK != StringCchCatW(szSearch, sizeof(szSearch)/sizeof(szSearch[0]), L"\\*")) return -1;

	FBCOLUMN col;
	ZeroMemory(&col, sizeof(FBCOLUMN));
	col.bufferOffset = (INT)pfb->pBuffer->Count();
	col.firstSelected = -1;
	col.width = width;
	col.autoAdjust = bAutoAdjust;

	DWORD ws = GetWindowLongPtrW(hwnd, GWL_STYLE);

	hFile = pfb->filesystem.fnFindFirstFile(szSearch, &fd);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		do
		{
			if (0 != (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes) &&
				(0 == (FBS_IGNOREHIDDEN & ws) || 0 == (FILE_ATTRIBUTE_HIDDEN & fd.dwFileAttributes)) &&
				(0 == (FBS_IGNORESYSTEM & ws) || 0 == (FILE_ATTRIBUTE_SYSTEM & fd.dwFileAttributes)) &&
				!(L'.' == fd.cFileName[0] && (L'\0' == fd.cFileName[1] || (L'.' == fd.cFileName[1] && L'\0' == fd.cFileName[2]))))
			{
				pfb->pBuffer->Add(fd.cFileName);
				col.count++;
			}
		} while (pfb->filesystem.fnFindNextFile(hFile, &fd));
		pfb->filesystem.fnFindClose(hFile);
	}

	pfb->pColumns->push_back(col);

	if (pfb->pColumns->size() > 0)
	{
		FBCOLUMN *pc = &pfb->pColumns->back();
		if  (pc->autoAdjust)
		{
			width = FolderBrowser_GetPreferredColumnWidth(hwnd, pfb->pColumns->size() - 1);
			if (width < COLUMN_DEFAULT_WIDTH) width = COLUMN_DEFAULT_WIDTH;
		}
		else if (-1 == width) width = COLUMN_DEFAULT_WIDTH;
		pc->width = width;

		if (pc->count)
		{
			FBITEM *pi = (FBITEM*)calloc(pc->count, sizeof(FBITEM));
			if (pi)
			{
				for (INT i = 0; i < pc->count; i++) 
				{
					pi[i].index = i;
					pi[i].styles = 0;
				}
				pc->pItems = pi;
			}
		}
		FolderBrowser_SortColumn(hwnd, pfb->pColumns->size() - 1);
	}

	return col.count;
}

static INT FolderBrowser_CalculateListItemHeight(HWND hwndList)
{
	HFONT hf, hfo;
	TEXTMETRIC tm;
	HDC hdc = GetDCEx(hwndList, NULL, DCX_CACHE);
	if (!hdc) return 20;

	hf = (HFONT)SendMessageW(hwndList, WM_GETFONT, 0, 0L);
	if (NULL == hf) hf = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
	hfo = (NULL != hf) ? (HFONT)SelectObject(hdc, hf) : NULL;
	if (!GetTextMetrics(hdc, &tm)) tm.tmHeight = 20;
	if (hfo) SelectObject(hdc, hfo);
	ReleaseDC(hwndList, hdc);

	return tm.tmHeight + 2;
}

static INT FolderBrowser_OnGetCurrentPath(HWND hwnd, LPWSTR pszPath, INT cchMax)
{
	HRESULT hr;
	FBCOLUMN *pc;

	if (!pszPath || cchMax < 1) return 0;
	pszPath[0] = L'\0';

	FBDATA *pfb = GetFolderBrowser(hwnd);
    if (!pfb || !pfb->pszRoot) return 0;
	size_t r = cchMax;

	for(size_t i = 0; i < pfb->pColumns->size(); i++)
	{
		pc = &pfb->pColumns->at(i);
		if (-1 == pc->firstSelected) break;

		size_t textIndex = pc->firstSelected;
		if (pc->pItems && textIndex < (size_t)pc->count) textIndex = pc->pItems[textIndex].index;
		textIndex += pc->bufferOffset;

		LPCWSTR pszText = (textIndex < pfb->pBuffer->Count()) ? pfb->pBuffer->GetString(textIndex) : NULL;
		if (!pszText) return 0;

		if (r < 2) hr= STRSAFE_E_INSUFFICIENT_BUFFER;
		else
		{
			if (0 != i)  { *pszPath = L'\\'; r--; pszPath++; }
			hr = StringCchCopyExW(pszPath, r, pszText, &pszPath, &r, STRSAFE_IGNORE_NULLS);
		}
		if (S_OK != hr) return 0;
	}

	return (cchMax - (INT)r);
}

static BOOL FolderBrowser_HideActive(HWND hwnd)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || -1 != hiddenActive ||
		0 == (WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE))) return FALSE;
	hiddenActive = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
	
	if (hiddenActive < pfb->pColumns->size())
	{
		FBCOLUMN *pc = &pfb->pColumns->at(hiddenActive);
		if (pc) pc->firstVisible = (INT)SendMessageW(pfb->hwndActive, LB_GETTOPINDEX, 0, 0L);
		if (pfb->focusedColumn == hiddenActive) SetFocus(hwnd);
		SetWindowLongPtrW(pfb->hwndActive, GWLP_USERDATA, (LONGX86)-1);
		ShowWindow(pfb->hwndActive, SW_HIDE);
	}
	return TRUE;
}

static BOOL FolderBrowser_RestoreActive(HWND hwnd)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || -1 == hiddenActive) return FALSE;
	if (hiddenActive >= pfb->pColumns->size())
	{
		hiddenActive = -1;
		return FALSE;
	}

	RECT rw;
	GetWindowRect(pfb->hwndActive, &rw);
	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
	SetWindowLongPtrW(pfb->hwndActive, GWLP_USERDATA, (LONGX86)hiddenActive);

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	if(!GetScrollInfo(hwnd, SB_HORZ, &si)) si.nPos = 0;
	LONG left = -si.nPos;

	for (size_t i=0; i < hiddenActive; i++) left += (pfb->pColumns->at(i).width + SIZER_WIDTH);
	LONG width = pfb->pColumns->at(hiddenActive).width;
	if (left != rw.left || width != (rw.right - rw.left))
	{
		SetWindowPos(pfb->hwndActive, NULL, left, rw.top, width, rw.bottom - rw.top, 
							SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING |
							((left == rw.left) ? SWP_NOMOVE : 0) | 
							((width == (rw.right - rw.left)) ? SWP_NOSIZE : 0));
	}

	ShowWindow(pfb->hwndActive, SW_SHOWNA);
	if (pfb->focusedColumn == hiddenActive) SetFocus(pfb->hwndActive);
	hiddenActive = -1;

	return TRUE;
}

static INT FolderBrowser_ScrollWindow(HWND hwnd, INT dx, UINT smoothTime, BOOL bRedraw)
{
	SCROLLINFO si;
	size_t hiddenActive = -1;
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return 0;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	if (!GetScrollInfo(hwnd, SB_HORZ, &si)) return 0;

	if ((si.nPos + dx) < si.nMin) dx = si.nMin - si.nPos;
	else if ((si.nPos + dx) > (si.nMax - (INT)si.nPage))
	{
		dx = si.nMax - si.nPos - si.nPage;
		if (dx < 0) dx = 0;
	}
	if (dx == 0) return 0;

	SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0L);

	if (pfb && (WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE)))
	{
		hiddenActive = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
		if (-1 != hiddenActive)
		{
			pfb->pColumns->at(hiddenActive).firstVisible = (INT)SendMessageW(pfb->hwndActive, LB_GETTOPINDEX, 0, 0L);

			if (pfb->focusedColumn == hiddenActive) SetFocus(hwnd);
			SetWindowLongPtrW(pfb->hwndActive, GWLP_USERDATA, (LONGX86)-1);
			ShowWindow(pfb->hwndActive, SW_HIDE);
		}
	}

	si.fMask = SIF_POS;
	si.nPos += dx;
	SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);

	if (bRedraw || smoothTime) SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0L);
	if (smoothTime)  ScrollWindowEx(hwnd, -dx, 0, NULL, NULL, NULL, NULL, MAKELPARAM(SW_SMOOTHSCROLL, 150));
	else ScrollWindowEx(hwnd, -dx, 0, NULL, NULL, NULL, NULL, ((bRedraw) ? (SW_INVALIDATE | SW_ERASE) : 0));

	if (-1 != hiddenActive)
	{
		RECT rw;
		GetWindowRect(pfb->hwndActive, &rw);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 1);

		SetWindowLongPtrW(pfb->hwndActive, GWLP_USERDATA, (LONGX86)hiddenActive);
		SetWindowPos(pfb->hwndActive, NULL, rw.left - dx, rw.top, 0, 0, 
						SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING |
						((bRedraw) ? 0 : SWP_NOREDRAW));

		ShowWindow(pfb->hwndActive, SW_SHOWNA);
		if (pfb->focusedColumn == hiddenActive) SetFocus(pfb->hwndActive);
	}
	if (!bRedraw && !smoothTime) SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0L);

	return dx;
}

static INT FolderBrowser_OnEnsureVisible(HWND hwnd, size_t column, UINT uFlags)
{
	RECT rc;
	SCROLLINFO si;
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return 0;
	LONG left;
	FBCOLUMN *pc;

	size_t count = pfb->pColumns->size();
	if (column >= count) return 0;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	if (!GetScrollInfo(hwnd, SB_HORZ, &si) || !GetClientRect(hwnd, &rc)) return 0;

	pc = NULL;
	left = -si.nPos;
	for (size_t i = 0; i < column; i++)
	{
		pc = &pfb->pColumns->at(i);
		left += (pc->width + SIZER_WIDTH);
	}

	INT dx = 0;
	if (left < rc.left) 
	{
		dx = left - rc.left;
		if (0 == (EVF_NOEXTRALSPACE & uFlags) && column != 0) dx -= COLUMN_EXTRALSPACE;  
	}
	else 
	{
		LONG ol = left;
		if (0 == (EVF_NOEXTRALSPACE & uFlags) && column != 0) ol -= COLUMN_EXTRALSPACE;
		left += (pfb->pColumns->at(column).width + SIZER_WIDTH);
		if (0 == (EVF_NOEXTRARSPACE & uFlags) && column < (count - 1)) 
			left += (pfb->pColumns->at(column + 1).width + SIZER_WIDTH);
		if (left > rc.right) dx = left - rc.right;

		if (ol - dx < rc.left) 
		{
			dx = ol - rc.left;
		}
		if ((INT)si.nPage > si.nMax) si.nPage = si.nMax;
		if ((si.nPos + dx) > (si.nMax - (INT)si.nPage))
		{
			si.nMax = si.nPos + dx + si.nPage;
			si.fMask = SIF_RANGE;
			SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);
		}
	}
	if (dx == 0) return 0;
	return FolderBrowser_ScrollWindow(hwnd, dx, 250, 0 == (EVF_NOREDRAW & uFlags));
}

static INT FolderBrowser_SaveActiveSelection(HWND hwnd)
{
	FBCOLUMN *pc;
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return -1;

	size_t activeColumn = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
	if (activeColumn >= pfb->pColumns->size()) return -1;
	pc = &pfb->pColumns->at(activeColumn);
	if (!pc) return -1;

	for (INT i = 0; i < pc->count; i++) 
		pc->pItems[i].styles &= ~FBIS_SELECTED;

	INT count = (INT)SendMessageW(pfb->hwndActive, LB_GETSELCOUNT, 0, 0L);

	INT *pSelection = NULL;
	if (count > 0)
	{
		pSelection = (INT*)calloc((count + 1), sizeof(INT));
		if (LB_ERR == SendMessageW(pfb->hwndActive, LB_GETSELITEMS, count, (LPARAM)pSelection))
			count = 0;
	}

	for (INT i = 0; i < count; i++) 
	{
		INT k = pSelection[i];
		if (k < pc->count && k >= 0) 
		{
			pc->pItems[k].styles |= FBIS_SELECTED;
		}
	}

	INT selectedItem = (1 == count && pSelection) ? pSelection[0] : -1;
	if (pSelection) free(pSelection);
	return selectedItem;
}

static void FolderBrowser_OnSelectionChanged(HWND hwnd, BOOL bForceUpdate, BOOL bUpdateUI)
{
	FBCOLUMN *pc;
	INT columnWidth = -1;
	BOOL bAutoAdjust = TRUE;
	size_t activeColumn, selectedItem;

	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return;

	activeColumn = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
	if (activeColumn >= pfb->pColumns->size()) return;
	pc = &pfb->pColumns->at(activeColumn);

	selectedItem = FolderBrowser_SaveActiveSelection(hwnd);

	if (activeColumn < (pfb->pColumns->size() -1)) 
	{
		columnWidth = pfb->pColumns->at(activeColumn + 1).width;
		bAutoAdjust = pfb->pColumns->at(activeColumn + 1).autoAdjust;
	}
	pfb->pBuffer->TrimCount(pc->bufferOffset + pc->count);
	while (pfb->pColumns->size() > (activeColumn + 1)) 
	{
		FBCOLUMN *pctmp = &pfb->pColumns->back();

		if (pctmp->pItems) 
		{
			free(pctmp->pItems);
			pctmp->pItems = NULL;
		}
		pfb->pColumns->pop_back();
	}

	//if (((size_t)-1) != selectedItem) 
	{
	/*	if (!bForceUpdate && (size_t)pc->firstSelected == selectedItem) 
		{
			FolderBrowser_OnEnsureVisible(hwnd, activeColumn, 0);
			return;
		}*/

		pc->firstSelected = (INT)selectedItem;
		wchar_t szPath[2*MAX_PATH] = {0};
		INT cchPath = FolderBrowser_OnGetCurrentPath(hwnd, szPath, sizeof(szPath)/sizeof(szPath[0]));
		if (0 != cchPath && ((size_t)-1) != selectedItem) FolderBrowser_AddColumn(hwnd, szPath, cchPath, columnWidth, bAutoAdjust);
	}

	if (bUpdateUI)
	{
		RECT rc;
		FolderBrowser_GetAdjustedClientRect(hwnd, &rc);

		for(size_t i = 0; i <= activeColumn; i++) rc.left += (pfb->pColumns->at(i).width + SIZER_WIDTH);

		FolderBrowser_OnEnsureVisible(hwnd, activeColumn, EVF_NOREDRAW);
		FolderBrowser_UpdateScrollInfo(hwnd);

		InvalidateRect(hwnd, &rc, TRUE);
		UpdateWindow(hwnd);
		if (pfb->hwndActive) UpdateWindow(pfb->hwndActive);
	}

	HWND hwndParent = GetParent(hwnd);
	if (NULL != hwndParent)
	{
		NMHDR hdr;
		hdr.code = FBN_SELCHANGED;
		hdr.hwndFrom = hwnd;
		hdr.idFrom = GetDlgCtrlID(hwnd);
		SendMessageW(hwndParent, WM_NOTIFY, hdr.idFrom, (LPARAM)&hdr);
	}
}

// returns new active column index
static size_t FolderBrowser_UpdateActiveColumn(HWND hwnd, size_t newActiveColumn) 
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return ((size_t)-1);

	HRGN rgnInvalid = NULL;
	FBCOLUMN *pc;
	RECT rc, ri;

	size_t activeColumn = FolderBrowser_GetListBoxColumn(pfb->hwndActive);

	GetClientRect(hwnd, &rc);

	if (newActiveColumn >= pfb->pColumns->size()) newActiveColumn = ((size_t)-1);
	if (newActiveColumn == -1 && pfb->focusedColumn != -1)
	{
		CopyRect(&ri, &rc);
		for (size_t i = 0; i <= (size_t)pfb->focusedColumn; i++) 
		{
			pc = &pfb->pColumns->at(i);
			if (i < (size_t)pfb->focusedColumn) ri.left += (pc->width + SIZER_WIDTH);
			else ri.right = ri.left + (pc->width + SIZER_WIDTH);
		}
		if (NULL == rgnInvalid) rgnInvalid = CreateRectRgnIndirect(&ri);
	}
	if (activeColumn == newActiveColumn) 
	{
		if (rgnInvalid)
		{
			InvalidateRgn(hwnd, rgnInvalid, FALSE);
			DeleteObject(rgnInvalid);
			UpdateWindow(hwnd);
		}
		return activeColumn;
	}

	SetRect(&ri, 0, 0, 0, 0);
	if (activeColumn < pfb->pColumns->size())
	{
		pc = &pfb->pColumns->at(activeColumn);
		pc->firstVisible = (INT)SendMessageW(pfb->hwndActive, LB_GETTOPINDEX, 0, 0L);

		pc->firstSelected = FolderBrowser_SaveActiveSelection(hwnd);
		INT selectedCount = (INT)SendMessageW(pfb->hwndActive, LB_GETSELCOUNT, 0, 0L);

		if (-1 == pc->firstSelected && selectedCount > 0)
		{
			pc->firstSelected = (INT)SendMessageW(pfb->hwndActive, LB_GETANCHORINDEX, 0, 0L);
		}

		if (selectedCount > 0 || LB_ERR == SendMessageW(pfb->hwndActive, LB_GETITEMRECT, pc->firstSelected, (LPARAM)&ri))
			GetClientRect(pfb->hwndActive, &ri);
		MapWindowPoints(pfb->hwndActive, hwnd, (POINT*)&ri, 2);
	}

	if (ri.left != ri.right)
	{
		if (NULL == rgnInvalid) rgnInvalid = CreateRectRgnIndirect(&ri);
		else
		{
			HRGN rgnTmp = CreateRectRgnIndirect(&ri);
			CombineRgn(rgnInvalid, rgnInvalid, rgnTmp, RGN_OR);
			DeleteObject(rgnTmp);
		}
	}

	SetWindowLongPtrW(pfb->hwndActive, GWLP_USERDATA, (LONGX86)newActiveColumn);

	if ((size_t)-1 == newActiveColumn) 
	{
		pfb->focusedColumn = -1;
		if ((WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE)))
		{
			SendMessageW(pfb->hwndActive, WM_SETREDRAW, FALSE, 0L);
			UpdateWindow(hwnd);
			SendMessage(hwnd, WM_SETREDRAW, FALSE, 0L);
			ShowWindow(pfb->hwndActive, SW_HIDE);
			SendMessage(hwnd, WM_SETREDRAW, TRUE, 0L);
		}
	}
	else
	{
		SendMessageW(pfb->hwndActive, WM_SETREDRAW, FALSE, 0L); 

		UpdateWindow(hwnd);
		SendMessage(hwnd, WM_SETREDRAW, FALSE, 0L);

		for (size_t i = 0; i < pfb->pColumns->size(); i++)
		{
			pc = &pfb->pColumns->at(i);
			if (i == newActiveColumn) 
			{
				rc.right = rc.left + pc->width;

				SendMessageW(pfb->hwndActive, LB_SETCOUNT, pc->count, 0L); 
				for (int k = 0; k < pc->count; k++)
				{
					if (FBIS_SELECTED & pc->pItems[k].styles)
						SendMessageW(pfb->hwndActive, LB_SETSEL, TRUE, k);
				}
				if ((UINT)pc->firstSelected < (UINT)pc->count) 
				{
					SendMessageW(pfb->hwndActive, LB_SETSEL, TRUE, pc->firstSelected);
				}
				SendMessageW(pfb->hwndActive, LB_SETTOPINDEX, pc->firstVisible, 0L);

				break;
			}
			else rc.left += (pc->width + SIZER_WIDTH);
		}

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		if (!GetScrollInfo(hwnd, SB_HORZ, &si)) ZeroMemory(&si, sizeof(SCROLLINFO));

		SetWindowPos(pfb->hwndActive, NULL, rc.left - si.nPos, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOREDRAW);

		MLSkinnedScrollWnd_UpdateBars(pfb->hwndActive, FALSE);
		if (0 == (WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE))) 
			ShowWindow(pfb->hwndActive, SW_SHOWNA);

		SendMessage(hwnd, WM_SETREDRAW, TRUE, 0L);
		SendMessageW(pfb->hwndActive, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);
		SendMessageW(pfb->hwndActive, WM_SETREDRAW, TRUE, 0L);

		UpdateWindow(pfb->hwndActive);
	}

	if (rgnInvalid)
	{
		InvalidateRgn(hwnd, rgnInvalid, FALSE);
		DeleteObject(rgnInvalid);
		UpdateWindow(hwnd);
	}

	return newActiveColumn;
}

static BOOL FolderBrowser_OnSetRootFolder(HWND hwnd, LPCWSTR pszRoot)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return FALSE;
	FolderBrowser_UpdateActiveColumn(hwnd, ((size_t)-1));
	pfb->pColumns->clear();
	pfb->pBuffer->Clear();
	pfb->pszRoot = NULL;
	if (pszRoot) pfb->pszRoot = _wcsdup(pszRoot);

	FBCOLUMN col;
	ZeroMemory(&col, sizeof(FBCOLUMN));
	col.width = COLUMN_DEFAULT_WIDTH;
	col.count = 1;
	col.pItems = (FBITEM*)calloc(1, sizeof(FBITEM));
	col.pItems[0].index = 0;

	pfb->pBuffer->Add(pszRoot);
	pfb->pColumns->push_back(col);

	LRESULT result = FolderBrowser_AddColumn(hwnd, pszRoot, -1, -1, TRUE);
	FolderBrowser_UpdateScrollInfo(hwnd);
	return ( -1 != result);
}

static INT FolderBrowser_OnGetRootFolder(HWND hwnd, LPWSTR pszBuffer, INT cchMax)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || !pszBuffer) return -1;

	HRESULT hr = StringCchCopyExW(pszBuffer, cchMax, pfb->pszRoot, NULL, (size_t*)&cchMax, STRSAFE_IGNORE_NULLS);
	return (S_OK == hr) ? cchMax : -1;
}

static BOOL FolderBrowser_OnSetCurrentPath(HWND hwnd, LPCWSTR pszPath, BOOL bRedraw)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || !pfb->pszRoot) return FALSE;

	BOOL updateLast = FALSE;
	DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
	size_t column = 0;
	INT cchPart;

	LPCWSTR pszCursor, pszPart;

	pszCursor = pszPath;
	pszPart = pszCursor;

	if (pszCursor)
	{
		for (; column < pfb->pColumns->size() && !updateLast; column++, pszCursor++) 
		{
			pszPart = pszCursor;
			FBCOLUMN *pc = &pfb->pColumns->at(column);

			if (pc->pItems)
			{
				for (int i = 0; i < pc->count; i++)
					pc->pItems[i].styles = 0;
			}

			while (L'\0' != *pszCursor && L'\\' != *pszCursor) pszCursor++;
			cchPart = (int)(pszCursor - pszPart);
			if (0 == cchPart)
			{
				pc->firstSelected = -1;
				pc->firstVisible = 0;
				break;
			}

			if (-1 == pc->firstSelected || CSTR_EQUAL != CompareStringW(lcid, NORM_IGNORECASE, pszPart, cchPart,
					pfb->pBuffer->GetString(pc->bufferOffset + pc->pItems[pc->firstSelected].index), -1)) 
			{
				pc->firstSelected = FoderBrowser_FindFolder(pfb->pBuffer, pc, pszPart, cchPart);
				if (-1 != pc->firstSelected) 
				{
					pc->firstVisible = pc->firstSelected;
					updateLast = TRUE;
				}
				else break;
			}
			if (-1 != pc->firstSelected) 
			{
				if (pc->pItems && pc->firstSelected < pc->count) pc->pItems[pc->firstSelected].styles |= FBIS_SELECTED;
			}
			if (L'\0' == *pszCursor) break;
		}
	} 

	if (0 == column)
	{
		FBCOLUMN *pc = &pfb->pColumns->at(0);
		if (pc && pc->count > 0 && pc->pItems)
		{
			pc->firstSelected = 0;
			pc->pItems[pc->firstSelected].styles |= FBIS_SELECTED;
		}
		if(CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, pszPart, cchPart, pfb->pszRoot, cchPart))
		{
			pszPath = pfb->pszRoot;
			pszCursor = pfb->pszRoot + lstrlenW(pszPath);
			updateLast = TRUE;
		}
	}

	if (column == pfb->pColumns->size()) updateLast = TRUE;

	if (column < pfb->pColumns->size())
	{
		pfb->pBuffer->TrimCount(pfb->pColumns->at(column).bufferOffset + pfb->pColumns->at(column).count);
		while (pfb->pColumns->size() > (column + 1))  
		{
			FBCOLUMN *pc = &pfb->pColumns->back();
			if (pc->pItems) free(pc->pItems); 
			pfb->pColumns->pop_back();
		}
	}

	while (updateLast)
	{
		updateLast = FALSE;
		INT cchPath = (INT)(pszCursor - pszPath);
		if (0 != cchPath && FolderBrowser_AddColumn(hwnd, pszPath, cchPath, -1, TRUE) >= 0)
		{
			FBCOLUMN *pc = &pfb->pColumns->back();
			pszPart = pszCursor;
			while (L'\0' != *pszCursor && L'\\' != *pszCursor) pszCursor++;
			cchPart = (int)(pszCursor - pszPart);
			if (0 != cchPart)
			{
				pc->firstSelected = FoderBrowser_FindFolder(pfb->pBuffer, pc, pszPart, cchPart);
				if (-1 != pc->firstSelected)
				{
					pc->firstVisible = pc->firstSelected;
					if (pc->pItems && pc->firstSelected < pc->count) pc->pItems[pc->firstSelected].styles |= FBIS_SELECTED;
					updateLast = TRUE;
				}
				if (0x00 != *pszCursor) pszCursor++;
			}
			column++;
		}
	}

	FolderBrowser_HideActive(hwnd);

	if (bRedraw)
	{
		size_t active = column;
		if (active >= pfb->pColumns->size()) active = pfb->pColumns->size() - 1;
		while(0 != active && -1 == pfb->pColumns->at(active).firstSelected) active--;
		FolderBrowser_OnEnsureVisible(hwnd, active, EVF_NOREDRAW);
		FolderBrowser_UpdateScrollInfo(hwnd);
	}
	FolderBrowser_RestoreActive(hwnd);
	if (bRedraw) InvalidateRect(hwnd, NULL, TRUE);
	return TRUE;
}

static LRESULT FolderBrowser_OnCreateWindow(HWND hwnd, CREATESTRUCT *pcs)
{
	FBDATA *pfb;
	pfb = (FBDATA*)calloc(1, sizeof(FBDATA));
	if (!pfb) return -1;

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtrW(hwnd, 0, (LONGX86)(LONG_PTR)pfb) && ERROR_SUCCESS != GetLastError())
	{
		free(pfb);
		return -1;
	}

	SetWindowLongPtrW(hwnd, GWL_STYLE, GetWindowLongPtrW(hwnd, GWL_STYLE) | WS_CLIPCHILDREN);

	pfb->rgbBk = GetSysColor(COLOR_WINDOW);
	pfb->rgbText = GetSysColor(COLOR_WINDOWTEXT);
	pfb->pColumns = new std::vector<FBCOLUMN>();
	pfb->pBuffer = new StringVector(8192, 4096);
	pfb->focusedColumn = -1;
	pfb->hwndActive = CreateWindowExW(WS_EX_NOACTIVATE /*| WS_EX_NOPARENTNOTIFY*/, L"ListBox", L"FolderView active listbox", 
									WS_CHILD | WS_VSCROLL |
									LBS_NODATA | LBS_OWNERDRAWFIXED | LBS_DISABLENOSCROLL | LBS_NOREDRAW | LBS_NOTIFY |
									LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT, 
									0, 0, 1, 1, hwnd, NULL, NULL, 0L);
	pfb->hwndDraw = CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_NOPARENTNOTIFY, L"ListBox", L"FolderView drawing listbox", 
									WS_CHILD | WS_VSCROLL |
									LBS_NODATA | LBS_OWNERDRAWFIXED | LBS_DISABLENOSCROLL | LBS_NOREDRAW | 
									LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT, 
									0, 0, 1, 1, hwnd, NULL, NULL, 0L);
	HFONT hFont = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
	if (pfb->hwndActive)
	{
		SetWindowLongPtrW(pfb->hwndActive, GWLP_USERDATA, (LONGX86)-1);
		SendMessageW(pfb->hwndActive, WM_SETFONT, (WPARAM)hFont, FALSE);
		SendMessageW(pfb->hwndActive, LB_SETITEMHEIGHT, 0, (LPARAM)FolderBrowser_CalculateListItemHeight(pfb->hwndActive));
		FolderBrowser_CustomizeListBox(pfb->hwndActive);
	}
	if (pfb->hwndDraw)
	{
		SetWindowLongPtrW(pfb->hwndDraw, GWLP_USERDATA, (LONGX86)-1);
		SendMessageW(pfb->hwndDraw, WM_SETFONT, (WPARAM)hFont, FALSE);
		SendMessageW(pfb->hwndDraw, LB_SETITEMHEIGHT, 0, (LPARAM)FolderBrowser_CalculateListItemHeight(pfb->hwndDraw));
	}

	// this weird call need to be done to disable vertical scrollbar in skinned mode
	SCROLLINFO si;
	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_PAGE;
	si.nMin = 0;
	si.nMax = 0;
	si.nPage = 100;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE); 
	SendMessageW(hwnd, FBM_SETFILESYSTEMINFO, 0, 0L);
	return 0;
}

static void FolderBrowser_OnDestroyWindow(HWND hwnd)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	SetWindowLongPtrW(hwnd, 0, 0L);
	if (!pfb) return;

	if (pfb->pBuffer) delete(pfb->pBuffer);
	if (pfb->pColumns) 
	{
		while (pfb->pColumns->size() > 1) 
		{
			FBCOLUMN *pc = &pfb->pColumns->back();
			if (pc->pItems) free(pc->pItems);
			pfb->pColumns->pop_back();
		}

		delete(pfb->pColumns);
	}
	if (pfb->hwndDraw) DestroyWindow(pfb->hwndDraw);
	if (pfb->hwndActive) DestroyWindow(pfb->hwndActive);
	free(pfb);
}

static void FolderBrowser_Draw(HWND hwnd, PAINTSTRUCT *pps)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	HDC hdc;
	RECT rc, rp, rs;

	hdc = pps->hdc;

	CopyRect(&rp, &pps->rcPaint);

	FolderBrowser_GetAdjustedClientRect(hwnd, &rc);

	size_t activeColumn = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
	int height = rc.bottom - rc.top;
	for (size_t i = 0; i < pfb->pColumns->size(); i++)
	{
		FBCOLUMN *pc = &pfb->pColumns->at(i);
		if (rc.left < rp.right && (rc.left + pc->width + SIZER_WIDTH) > rp.left)
		{
			SetViewportOrgEx(hdc, rc.left, rc.top, NULL);

			if (i != activeColumn && rp.left < (rc.left + pc->width))
			{
				PrepareDrawingListBox(pfb->hwndDraw, pc, height, i);
				SendMessageW(pfb->hwndDraw, WM_PRINT, (WPARAM)hdc, (LPARAM)(PRF_CLIENT | PRF_NONCLIENT | ((pps->fErase) ? PRF_ERASEBKGND : 0)));
			}

			if (rp.right > (rc.left + pc->width) && (SIZER_WIDTH > 0))
			{
				SetBkColor(hdc, pfb->rgbBk);
				SetRect(&rs, pc->width, rp.top - rc.top, pc->width + SIZER_WIDTH, rp.bottom - rc.top);
				ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rs, NULL, 0, NULL); 
				if ((i == sizerActive || i == sizerHover) && 1 == (rs.right - rs.left)%2)
				{
					COLORREF clr;
					HPEN hp, hpo;

					clr = BlendColors(pfb->rgbText, pfb->rgbBk, (i != sizerActive) ? 76 : 127);

					hp = (HPEN)GetStockObject(DC_PEN);
					hpo = (hp) ? (HPEN)SelectObject(hdc, hp) : NULL;
					SetDCPenColor(hdc, clr);
					LONG l = rs.left + (rs.right - rs.left)/2;
					MoveToEx(hdc, l, rs.top, NULL);
					LineTo(hdc, l, rs.bottom);
					if (hpo) SelectObject(hdc, hpo);
				}
			}
		}
		rc.left += (pc->width + SIZER_WIDTH);
		if (rc.left > rc.right) break;
	}

	if (pps->fErase && rc.left < rp.right)
	{
		if (rc.left < rp.left) rc.left = rp.left;
		SetViewportOrgEx(hdc, 0, 0, NULL);
		SetBkColor(hdc, pfb->rgbBk);
		ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);
	}

	SetWindowLongPtrW(pfb->hwndDraw, GWLP_USERDATA, (LONGX86)-1);
}

static void FolderBrowser_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right) FolderBrowser_Draw(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
}

static COLORREF FolderBrowser_OnGetBkColor(HWND hwnd)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	return (pfb) ? pfb->rgbBk : 0;
}

static BOOL FolderBrowser_OnSetBkColor(HWND hwnd, COLORREF rgbBk)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return FALSE;
	pfb->rgbBk = rgbBk;
	return TRUE;
}

static COLORREF FolderBrowser_OnGetTextColor(HWND hwnd)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	return (pfb) ? pfb->rgbText : 0;
}

static BOOL FolderBrowser_OnSetTextColor(HWND hwnd, COLORREF rgbText)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return FALSE;
	pfb->rgbText = rgbText;
	return TRUE;
}

static BOOL FolderBrowser_OnGetFolderBrowserInfo(HWND hwnd, FOLDERBROWSERINFO *pInfo)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || !pInfo || pInfo->cbSize < sizeof(FOLDERBROWSERINFO)) return FALSE;
	pInfo->activeColumn = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
	pInfo->hwndActive = pfb->hwndActive;
	pInfo->hwndDraw = pfb->hwndDraw;
	return TRUE;
}

static BOOL FolderBrowser_OnMeasureItem(HWND hwnd, INT ctrlId, MEASUREITEMSTRUCT *pmis)
{
	LPCWSTR pszText(NULL);
	HWND hItem = GetDlgItem(hwnd, pmis->CtlID);
	FBDATA *pfb = GetFolderBrowser(hwnd);

	if (!pfb || NULL == hItem) return FALSE;

	size_t c = FolderBrowser_GetListBoxColumn(hItem);

	if (c < pfb->pColumns->size())
	{
		if (c == 0) // special case
		{
			pszText = pfb->pszRoot;
		}
		else
		{
			FBCOLUMN *pc = &pfb->pColumns->at(c);
			size_t i = pmis->itemID;
			if (pc->pItems && i < (UINT)pc->count) i = pc->pItems[i].index;
			i += pc->bufferOffset;
			if (i < pfb->pBuffer->Count()) pszText = pfb->pBuffer->GetString(i);
		}

		INT cchText(0);
		if (NULL != pszText) cchText = lstrlenW(pszText);
		SkinnedListbox::MeasureItem(hItem, pszText, cchText, &pmis->itemWidth, &pmis->itemHeight);
	}

	return TRUE;
}

static BOOL FolderBrowser_OnDrawItem(HWND hwnd, INT ctrlId, DRAWITEMSTRUCT *pdis)
{
	LPCWSTR pszText(NULL);
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return FALSE;

	size_t c = FolderBrowser_GetListBoxColumn(pdis->hwndItem);

	if (c < pfb->pColumns->size())
	{
		if (c == 0) // special case
		{
			pszText = pfb->pszRoot;
			if (pdis->hwndItem != pfb->hwndActive && 
				pfb->pColumns->at(0).pItems && 
				(FBIS_SELECTED & pfb->pColumns->at(0).pItems[0].styles))
			{
				pdis->itemState |= ODS_SELECTED;
			}
		}
		else
		{
			FBCOLUMN *pc = &pfb->pColumns->at(c);
			size_t i = pdis->itemID;
			if (pc->pItems && i < (UINT)pc->count) i = pc->pItems[i].index;
			i += pc->bufferOffset;
			if (i < pfb->pBuffer->Count()) pszText = pfb->pBuffer->GetString(i);
			
			if (pdis->hwndItem != pfb->hwndActive && pc->pItems && 
				(FBIS_SELECTED & pc->pItems[pdis->itemID].styles))
			{
					pdis->itemState |= ODS_SELECTED;
			}
		}

		INT cchText(0);
		if (NULL != pszText) cchText = lstrlenW(pszText);
		
		if (c == pfb->focusedColumn && pdis->hwndItem != pfb->hwndActive)
		{
			pdis->hwndItem = hwnd;
			if(pdis->itemState & ODS_SELECTED)
			{
				SkinnedListbox::DrawItem(pdis, pszText, cchText);
				pdis->itemAction = ODA_FOCUS;
				pdis->itemState &= ~0x0200/*ODS_NOFOCUSRECT*/;
				if (UISF_HIDEFOCUS & SendMessageW(hwnd, WM_QUERYUISTATE, 0, 0L)) pdis->itemState |= 0x0200/*ODS_NOFOCUSRECT*/;
			}
		}
		SkinnedListbox::DrawItem(pdis, pszText, cchText);
	}

	return TRUE;
}

static void FolderBrowser_OnSetFont(HWND hwnd, HFONT hFont, BOOL bRedraw)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return;
	if (pfb->hwndActive) 
	{
		SendMessageW(pfb->hwndActive, WM_SETFONT, (WPARAM)hFont, FALSE);
		SendMessageW(pfb->hwndActive, LB_SETITEMHEIGHT, 0, (LPARAM)FolderBrowser_CalculateListItemHeight(pfb->hwndActive));
	}
	if (pfb->hwndDraw) 
	{
		SendMessageW(pfb->hwndDraw, WM_SETFONT, (WPARAM)hFont, FALSE);
		SendMessageW(pfb->hwndDraw, LB_SETITEMHEIGHT, 0, (LPARAM)FolderBrowser_CalculateListItemHeight(pfb->hwndDraw));
	}
}

static void FolderBrowser_OnSetFocus(HWND hwnd, HWND hwndLost)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (NULL == pfb) return;

	if (pfb && hwndLost != pfb->hwndActive && FolderBrowser_GetListBoxColumn(pfb->hwndActive) > pfb->pColumns->size()) 
	{
		size_t last = 0;
		for (size_t i= 0; i < pfb->pColumns->size(); i++)
		{
			if (-1 != pfb->pColumns->at(i).firstSelected) last = i;
			else break;
		}
		last = FolderBrowser_UpdateActiveColumn(hwnd, last);
		if (-1 != last)
		{
			//INT selCount = (INT)SendMessageW(pfb->hwndActive, LB_GETSELCOUNT, 0, 0L);
			//if (selCount < 1) 
			//{
			//	SendMessageW(pfb->hwndActive, LB_SETSEL, TRUE, 0L);
			//	FolderBrowser_OnSelectionChanged(hwnd, TRUE, TRUE);
			//}
			//else SendMessageW(pfb->hwndActive, LB_SETCARETINDEX, pfb->pColumns->at(last).firstSelected, FALSE); 
			//FolderBrowser_OnEnsureVisible(hwnd, last, 0);
		}
		if (IsWindowVisible(pfb->hwndActive) && IsWindowEnabled(pfb->hwndActive)) SetFocus(pfb->hwndActive);
	}
}

static void FolderBrowser_OnKillFocus(HWND hwnd, HWND hwndRecieve)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (pfb && hwndRecieve != pfb->hwndActive) 
	{
		FolderBrowser_UpdateActiveColumn(hwnd, ((size_t)-1));
	}
}

static LRESULT FolderBrowser_OnGetDlgCode(HWND hwnd, INT vKey, MSG *pMsg)
{
	return DLGC_WANTARROWS;
}

static UINT FolderBrowser_HitTest(HWND hwnd, POINT pt, size_t *pColumn, size_t *pItem)
{
	RECT rc;

	size_t column = -1, item = -1;

	UINT hitTest = HTERROR;

	FBDATA *pfb = GetFolderBrowser(hwnd);

	if (!pfb || !FolderBrowser_GetAdjustedClientRect(hwnd, &rc)) 
		return hitTest;

	hitTest = HTCLIENT;

	size_t activeColumn = FolderBrowser_GetListBoxColumn(pfb->hwndActive);

	for(size_t i = 0; i < pfb->pColumns->size(); i++)
	{
		FBCOLUMN *pc = &pfb->pColumns->at(i);
		rc.right = rc.left + pc->width;
		if (PtInRect(&rc, pt))
		{
			column = i;
			if (pc->count > 0) 
			{
				HWND hwndTest = NULL;
				if (i != activeColumn)
				{
					hwndTest = pfb->hwndDraw;
					SetWindowPos(hwndTest, NULL, 0, 0, pc->width, rc.bottom - rc.top, 
										SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSENDCHANGING);
					SendMessageW(hwndTest, LB_SETCOUNT, pc->count, 0L);
					if (-1 != pc->firstSelected) SendMessageW(pfb->hwndDraw, LB_SETSEL, TRUE, pc->firstSelected);
					SendMessageW(hwndTest, LB_SETTOPINDEX, pc->firstVisible, 0L);
				}
				else 
					hwndTest = pfb->hwndActive;

				if (NULL != hwndTest)
				{
					RECT rw;
					GetClientRect(hwndTest, &rw);
					MapWindowPoints(hwndTest, hwnd, (POINT*)&rw, 2);
					if (hwndTest != pfb->hwndActive) 
						OffsetRect(&rw, rc.left, rc.top);
					if (PtInRect(&rw, pt))
					{
						INT itemIndex = (INT)SendMessageW(hwndTest, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x - rc.left, pt.y - rc.top));
						if (HIWORD(itemIndex)) itemIndex = -1;
						if (itemIndex != -1) item = (size_t)itemIndex;
					}
				}
			}
			break;
		}

		rc.right += SIZER_WIDTH;
		rc.left = rc.right;
		if (rc.left > pt.x) break;
	}
	if (pColumn) *pColumn = column;
	if (pItem) *pItem = item;
	return hitTest;
}

static void FolderBrowser_UpdateScrollHovering(HWND hwnd, UINT uFlags, POINTS pts)
{
	RECT rc;
	FBCOLUMN *pc;
	POINT pt;
	LONG left;

	static size_t hoveredColumn = -1;
	static INT nHoveredPart	= -1;

	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || !FolderBrowser_GetAdjustedClientRect(hwnd, &rc)) 
		return;

	left = rc.left;

	POINTSTOPOINT(pt, pts);
	size_t activeColumn = FolderBrowser_GetListBoxColumn(pfb->hwndActive);

	size_t nHovered = -1;
	for(size_t i = 0; i < pfb->pColumns->size() && pt.x >= rc.left; i++)
	{
		pc = &pfb->pColumns->at(i);
		rc.right = rc.left + pc->width;
		if (i != activeColumn && PtInRect(&rc, pt))
		{
			PrepareDrawingListBox(pfb->hwndDraw, pc, rc.bottom - rc.top, -1);

			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_PAGE | SIF_RANGE;
			if (GetScrollInfo(pfb->hwndDraw, SB_VERT, &si) && (INT)si.nPage <= si.nMax)
			{
				pt.x -= rc.left;
				pt.y -= rc.top;
				MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);
				INT ht = (INT) SendMessageW(pfb->hwndDraw, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
				if (HTVSCROLL == ht || HTHSCROLL == ht)
				{
					nHovered = i;
					SBADJUSTHOVER hoverTest;
					hoverTest.hitTest = ht;
					hoverTest.ptMouse.x = (SHORT)pt.x;
					hoverTest.ptMouse.y = (SHORT)pt.y;
					if (SENDMLIPC(pfb->hwndDraw, IPC_ML_SKINNEDSCROLLWND_ADJUSTHOVER, (WPARAM)&hoverTest))
					{
						if (nHoveredPart != hoverTest.nResult)
						{
							nHoveredPart = hoverTest.nResult;
							RefreshListBoxNC(hwnd, pfb->hwndDraw, *(POINT*)&rc);
						}
					}
				}
				else 
					nHoveredPart = -1;
			}
			break;
		}

		rc.right += SIZER_WIDTH;
		rc.left = rc.right;
	}

	if (hoveredColumn != nHovered && -1 != hoveredColumn)
	{
		rc.left = left;
		for(size_t i = 0; i < hoveredColumn; i++)
			rc.left += pfb->pColumns->at(i).width + SIZER_WIDTH;

		pc = &pfb->pColumns->at(hoveredColumn);

		if (PrepareDrawingListBox(pfb->hwndDraw, pc, rc.bottom - rc.top, -1))
		{
			SendMessageW(pfb->hwndDraw, WM_NCMOUSELEAVE, HTNOWHERE, 0L);
			RefreshListBoxNC(hwnd, pfb->hwndDraw, *(POINT*)&rc);
		}
		nHoveredPart = -1;
	}

	hoveredColumn = nHovered;
	if (-1 != hoveredColumn && -1 != nHoveredPart)
	{
		TRACKMOUSEEVENT tracker;
		tracker.cbSize = sizeof(tracker);
		tracker.hwndTrack = hwnd;
		tracker.dwHoverTime = 0;
		tracker.dwFlags = TME_LEAVE;
		TrackMouseEvent(&tracker);
	}
}

static void FolderBrowser_OnMouseMove(HWND hwnd, UINT uFlags, POINTS pts)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb)  return;

	if (GetCapture() == hwnd && sizerActive < pfb->pColumns->size())
	{
		RECT rc, ri;
		SCROLLINFO si;
		FBCOLUMN *pc;

		GetClientRect(hwnd, &rc);
		CopyRect(&ri, &rc);
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		if (!GetScrollInfo(hwnd, SB_HORZ, &si))
			ZeroMemory(&si, sizeof(SCROLLINFO));
		rc.right = rc.left - si.nPos;
		for (size_t i = 0; i <= sizerActive; i++)
		{
			pc = &pfb->pColumns->at(i);
			rc.right += (pc->width + SIZER_WIDTH);
		}

		rc.left = rc.right - SIZER_WIDTH;
		GetCursorPos(((LPPOINT)&rc) + 1);
		MapWindowPoints(HWND_DESKTOP, hwnd, ((LPPOINT)&rc) + 1, 1);
		rc.right -= clickoffs;

		if (rc.left != rc.right)
		{
			if (pc)
			{
				ri.left = rc.left - pc->width;

				INT w = pc->width + (rc.right - rc.left);

				pc->autoAdjust = FALSE;
				if (w < COLUMN_MIN_WIDTH) w = COLUMN_MIN_WIDTH;
				if (w > COLUMN_MAX_WIDTH) w = COLUMN_MAX_WIDTH;
				if (pc->width != w)
				{
					pc->width = w;
					FolderBrowser_UpdateScrollInfo(hwnd);
					InvalidateRect(hwnd, &ri, TRUE);
					UpdateWindow(hwnd);
				}
			}
		}
	}
	else FolderBrowser_UpdateScrollHovering(hwnd, uFlags, pts);
}

static void FolderBrowser_OnLButtonUp(HWND hwnd, UINT uFlags, POINTS pts)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || pfb->pColumns->size() == 0)  return;

	if (-1 != sizerActive)
	{
		RECT rc;
		SCROLLINFO si;

		GetClientRect(hwnd, &rc);
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		if (!GetScrollInfo(hwnd, SB_HORZ, &si)) ZeroMemory(&si, sizeof(SCROLLINFO));
		rc.right = rc.left - si.nPos;
		for (size_t i = 0; i <= sizerActive; i++)
		{
			rc.right += (pfb->pColumns->at(i).width + SIZER_WIDTH);
		}
		rc.left = rc.right - SIZER_WIDTH;
		sizerActive = -1;
		clickoffs = 0;
		ReleaseCapture();
		FolderBrowser_RestoreActive(hwnd);
		InvalidateRect(hwnd, &rc, FALSE);
	}
}

static void FolderBrowser_OnButtonDown(HWND hwnd, UINT uButtonMsg, UINT uFlags, POINTS pts)
{
	RECT rc;
	POINT pt;
	UINT ht;
	size_t column, item;

	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || pfb->pColumns->size() == 0)  return;

	ht = HTERROR;
	FolderBrowser_GetAdjustedClientRect(hwnd, &rc);

	POINTSTOPOINT(pt, pts);

	// check if sizer clicked
	LONG cPoint = rc.left;
	for(size_t i = 0; i < pfb->pColumns->size() && rc.left <= pt.x; i++)
	{
		FBCOLUMN *pc = &pfb->pColumns->at(i);

		cPoint += pc->width;
		rc.left = cPoint - SIZER_OVERLAP_LEFT;
		cPoint += SIZER_WIDTH;
		rc.right = cPoint + SIZER_OVERLAP_RIGHT;

		if (WM_LBUTTONDOWN == uButtonMsg)
		{
			if (PtInRect(&rc, pt))
			{
				sizerActive = i;
				clickoffs = pts.x - (rc.left + SIZER_OVERLAP_LEFT);
				UpdateWindow(hwnd);
				FolderBrowser_HideActive(hwnd);
				SetCapture(hwnd);
				InvalidateRect(hwnd, &rc, FALSE);
				return;
			}
		}
	}

	ht = FolderBrowser_HitTest(hwnd, pt, &column, &item);
	if (-1 != column && -1 == item)
	{
		while (-1 != column && 0 == pfb->pColumns->at(column).count)
		{
			column--;
			/*item = */pfb->pColumns->at(column).firstSelected;
		}
	}

	if (-1 != column)
	{
		//if (WM_LBUTTONDOWN != uButtonMsg || (HTCLIENT == ht && -1 == item)) 
		//{
		//	FolderBrowser_OnEnsureVisible(hwnd, column, 0);
		//	return; // prevent activation
		//}
		
		if (pfb->hwndActive == GetFocus())
		{
			UpdateWindow(pfb->hwndActive);
			SendMessageW(pfb->hwndActive, WM_SETREDRAW, FALSE, 0L);
			SetFocus(hwnd);
			SendMessageW(pfb->hwndActive, WM_SETREDRAW, TRUE, 0L);
		}
		/*if (-1 != item) */pfb->focusedColumn  = -1;
		
		FolderBrowser_UpdateActiveColumn(hwnd, column);

		if ((WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE)))
		{
			MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);
			ht = (UINT)SendMessageW(pfb->hwndActive, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
			switch(ht)
			{
				case HTERROR: break;
				case HTCLIENT:
					MapWindowPoints(HWND_DESKTOP, pfb->hwndActive, &pt, 1);

					if (0 == (WS_EX_NOPARENTNOTIFY & (DWORD)GetWindowLongPtrW(pfb->hwndActive, GWL_EXSTYLE)))
					{
						HWND hParent = GetParent(pfb->hwndActive);
						if (NULL != hParent)
						{
							POINT ptParent = pt;
							MapWindowPoints(pfb->hwndActive, hParent, &ptParent, 1);
							SendMessageW(hParent, WM_PARENTNOTIFY, MAKEWPARAM(uButtonMsg, 0), MAKELPARAM(ptParent.x, ptParent.y));
						}
					}
					SendMessageW(pfb->hwndActive, uButtonMsg, uFlags, MAKELPARAM(pt.x, pt.y));
			//		maxRight = (LONG)SendMessageW(pfb->hwndActive, LB_ITEMFROMPOINT, uFlags, MAKELPARAM(pt.x, pt.y));
			//		if (0 == HIWORD(maxRight))
			//		{
			//			size_t a = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
			////			if (a < pfb->pColumns->size() && LOWORD(maxRight) == pfb->pColumns->at(a).firstSelected)
			////				pfb->pColumns->at(a).firstSelected = -1;
			//		}
					break;
				default:
					SendMessageW(pfb->hwndActive, (uButtonMsg - WM_MOUSEMOVE) + WM_NCMOUSEMOVE, ht, MAKELPARAM(pt.x, pt.y));
					break;
			}
		}
	}
	else
	{
		HWND hFocus = GetFocus();
		if (hFocus != hwnd && hFocus != pfb->hwndActive)
			SetFocus(hwnd);
	}
}

static void FolderBrowser_OnLButtonDown(HWND hwnd, UINT uFlags, POINTS pts)
{
	FolderBrowser_OnButtonDown(hwnd, WM_LBUTTONDOWN, uFlags, pts);
}

static void FolderBrowser_OnLButtonDblClick(HWND hwnd, UINT uFlags, POINTS pts)
{
	RECT rc;
	LONG maxRight;
	POINT pt;

	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb || pfb->pColumns->size() == 0)  return;

	GetClientRect(hwnd, &rc);

	FolderBrowser_GetAdjustedClientRect(hwnd, &rc);

	maxRight = rc.right;
	POINTSTOPOINT(pt, pts);

	LONG cPoint = rc.left;
	for(size_t i = 0; i < pfb->pColumns->size() && rc.left <= pt.x; i++)
	{
		FBCOLUMN *pc = &pfb->pColumns->at(i);

		cPoint += pc->width;
		rc.left = cPoint - SIZER_OVERLAP_LEFT;
		cPoint += SIZER_WIDTH;
		rc.right = cPoint + SIZER_OVERLAP_RIGHT;

		if (PtInRect(&rc, pt))
		{
			pc->autoAdjust = TRUE;
			INT width = FolderBrowser_GetPreferredColumnWidth(hwnd, i);
			if ( -1 != width && pc->width != width)
			{
				rc.left = (rc.left + SIZER_OVERLAP_LEFT) - pc->width;
				rc.right = maxRight;

				pc->width = width;

				UpdateWindow(hwnd);
				SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0L);
				FolderBrowser_HideActive(hwnd);
				FolderBrowser_UpdateScrollInfo(hwnd);
				FolderBrowser_RestoreActive(hwnd);
				SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0L);
				RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_ERASENOW | RDW_UPDATENOW);
				return;
			}
		}
	}
}

static void FolderBrowser_OnRButtonDown(HWND hwnd, UINT uFlags, POINTS pts)
{
	POINT pt;
	size_t col, item;
	POINTSTOPOINT(pt, pts);
	UINT ht = FolderBrowser_HitTest(hwnd, pt, &col, &item);
	if (HTCLIENT == ht && -1 != col)
	{
		INT ox = FolderBrowser_OnEnsureVisible(hwnd, col, EVF_NOEXTRALSPACE | EVF_NOEXTRARSPACE);
		if (0 != ox) pts.x -= ox;
	}
	TRACE_FMT(TEXT("RDown at (%d,%d) [ht=%d, col=%d, raw=%d]\n"), pt.x, pt.y, ht, col, item);
}

static void FolderBrowser_OnRButtonUp(HWND hwnd, UINT uFlags, POINTS pts)
{
	POINT pt;
	size_t col, item;
	POINTSTOPOINT(pt, pts);
	UINT ht = FolderBrowser_HitTest(hwnd, pt, &col, &item);
	TRACE_FMT(TEXT("RUp at (%d,%d) [ht=%d, col=%d, raw=%d]\n"), pt.x, pt.y, ht, col, item);
	FolderBrowser_RestoreActive(hwnd);
}

static void FolderBrowser_OnKeyDown(HWND hwnd, UINT vkCode, UINT flags)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (pfb) 
	{
		switch(vkCode)
		{
			case VK_PRIOR:
			case VK_NEXT:
			case VK_END:
			case VK_HOME:
			case VK_DOWN:
			case VK_UP:
			case VK_RIGHT:
			case VK_LEFT:
				if (pfb->focusedColumn != -1)
				{
					FolderBrowser_UpdateActiveColumn(hwnd, pfb->focusedColumn);
					if (IsWindowVisible(pfb->hwndActive) && IsWindowEnabled(pfb->hwndActive)) SetFocus(pfb->hwndActive);
				}
				if ((WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE)))
				{
					SendMessageW(pfb->hwndActive, WM_KEYDOWN, (WPARAM)vkCode, (LPARAM)flags);
					return;
				}
				break;
		}
	}
	DefWindowProcW(hwnd, WM_KEYDOWN, (WPARAM)vkCode, (LPARAM)flags);
}

static void FolderBorwser_OnCommand(HWND hwnd, UINT ctrlId, UINT eventId, HWND hwndCtrl)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return;
	if (pfb->hwndActive == hwndCtrl)
	{
		switch(eventId)
		{
			//case LBN_SELCANCEL:
			//	break;
			case LBN_SELCHANGE:
				FolderBrowser_OnSelectionChanged(hwnd, FALSE, TRUE);
				break;
			case LBN_SETFOCUS:
				pfb->focusedColumn = (int)FolderBrowser_GetListBoxColumn(hwndCtrl);
				break;
			case LBN_KILLFOCUS:
				if (hwnd != GetFocus()) 
				{
					FolderBrowser_UpdateActiveColumn(hwnd, ((size_t)-1));
				}
				break;
		}
	}
}

static LRESULT FolderBrowser_OnVKeyToItem(HWND hwnd, UINT vkCode, UINT caretPos, HWND hwndCtrl)
{

	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return -1;
	if (pfb->hwndActive == hwndCtrl)
	{
		size_t activeColumn = FolderBrowser_GetListBoxColumn(pfb->hwndActive);

		switch(vkCode)
		{
			case VK_LEFT: 
				if (0 != (0x80000000 & GetAsyncKeyState(VK_CONTROL)) || 
					0 != (0x80000000 & GetAsyncKeyState(VK_SHIFT))) break;

				if (activeColumn > 0) 
				{
					pfb->focusedColumn  = -1;
					FolderBrowser_UpdateActiveColumn(hwnd, activeColumn - 1); 
					pfb->focusedColumn = (int)FolderBrowser_GetListBoxColumn(pfb->hwndActive);
					FolderBrowser_OnSelectionChanged(hwnd, TRUE, TRUE);
				}
				return -2;

			case VK_RIGHT:
				if (0 != (0x80000000 & GetAsyncKeyState(VK_CONTROL)) || 
					0 != (0x80000000 & GetAsyncKeyState(VK_SHIFT))) break;

				if (activeColumn < (pfb->pColumns->size() -1))
				{
					FBCOLUMN *pc = &pfb->pColumns->at(activeColumn + 1);
					if (pc->count > 0)
					{
						pc->firstSelected = 0;
						pfb->focusedColumn  = -1;
						FolderBrowser_UpdateActiveColumn(hwnd, activeColumn + 1); 
						pfb->focusedColumn = (int)FolderBrowser_GetListBoxColumn(pfb->hwndActive);
						FolderBrowser_OnSelectionChanged(hwnd, TRUE, TRUE);
					}
				}
				return -2;
		}
	}

	return -1;
}

static void FolderBrowser_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return;

	if (0 == (SWP_NOSIZE & pwp->flags) || (SWP_FRAMECHANGED & pwp->flags))
	{
		RECT rc, rw;
		GetClientRect(hwnd, &rc);
		SCROLLINFO si;
		INT dx = 0;

		LONG totalWidth = 0;
		for(size_t i = 0; i < pfb->pColumns->size(); i++) totalWidth += (pfb->pColumns->at(i).width  + SIZER_WIDTH);

		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		if (!GetScrollInfo(hwnd, SB_HORZ, &si))
		{
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			si.nMax = 100;
			si.nPage = 10;
			SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);
		}
		if ( (si.nPage != (rc.right - rc.left) || si.nMax < totalWidth))
		{
			si.fMask = SIF_PAGE;
			if (si.nMax != totalWidth) 
			{
				si.nMax = totalWidth;
				si.fMask |= SIF_RANGE;
			}

			si.nPage = rc.right - rc.left;
			if ((si.nPos + si.nPage) > (UINT)si.nMax && si.nPos > si.nMin) 
			{
				dx = si.nPos;
				si.nPos = si.nMax - si.nPage;
				if (si.nPos < si.nMin) si.nPos = si.nMin;
				dx -= si.nPos;
				si.fMask |= SIF_POS;
				InvalidateRect(hwnd, NULL, FALSE);
			}
			if (0 == si.nMax) 
			{
				si.nMax = 100;
				si.nPage = si.nMax;
			}

			SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);
		}

		if (pfb->hwndActive && 
			(WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE))
			&& GetWindowRect(pfb->hwndActive, &rw))
		{
			if (rw.bottom - rw.top != rc.bottom - rc.top || dx != 0)
			{
				if (dx) MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 2);
				SetWindowPos(pfb->hwndActive, NULL, rw.left + dx, rw.top, rw.right - rw.left, rc.bottom - rc.top, 
					SWP_NOACTIVATE | SWP_NOZORDER | 
					((0 == dx) ? SWP_NOMOVE : 0) |
					((rw.bottom - rw.top == rc.bottom - rc.top) ? SWP_NOSIZE : 0));
				if (0 == (SWP_NOREDRAW & pwp->flags)) InvalidateRect(pfb->hwndActive, NULL, TRUE);
			}
		}
		
		if (0 == (SWP_NOREDRAW & pwp->flags))
		{
			GetWindowRect(pfb->hwndDraw, &rw);
			if (rw.bottom - rw.top != rc.bottom - rc.top) InvalidateRect(hwnd, NULL, FALSE);
		}
	}
}

static void UpdateAfterScroll(HWND hwnd, INT scrollPos, BOOL fRedraw)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (pfb && (WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE)))
	{
		RECT rc;
		size_t a = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
		GetClientRect(hwnd, &rc);
		rc.left -= scrollPos;
		for (size_t i = 0; i < a; i++) rc.left += (pfb->pColumns->at(i).width + SIZER_WIDTH);
		SetWindowPos(pfb->hwndActive, NULL, rc.left, rc.top, 0, 0, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOSIZE |
			((fRedraw) ? 0 : SWP_NOREDRAW));
	}
	if (fRedraw)
	{
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
	}
}

static void FolderBrowser_OnHScroll(HWND hwnd, UINT uCode, UINT uPos)
{
	SCROLLINFO si;
	FBDATA *pfb = GetFolderBrowser(hwnd);
	if (!pfb) return;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	if (!GetScrollInfo(hwnd, SB_HORZ, &si))  return;
	INT line = si.nPage / 10;
	if (line < 1) line = 1;
	INT dx = 0;	

	static INT startPos = 0;
	static INT scrollCount = 0;

	switch(uCode)
	{
		case SB_LINELEFT:	dx = -line; break;
		case SB_LINERIGHT:	dx = line; break;
		case SB_PAGELEFT:	dx = -((int)si.nPage); break;
		case SB_PAGERIGHT:	dx = ((int)si.nPage); break;
		case SB_THUMBTRACK: 
			UpdateAfterScroll(hwnd, si.nPos, TRUE);
			return;
		case SB_LEFT:
			si.fMask = SIF_POS;
			si.nPos = 0;
			SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);
			UpdateAfterScroll(hwnd, si.nPos, TRUE);
			return;
		case SB_RIGHT:
			si.fMask = SIF_POS;
			si.nPos = si.nMax - si.nPage;
			SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);
			UpdateAfterScroll(hwnd, si.nPos, TRUE);
			return;
		case SB_ENDSCROLL:
			if (-1 != hiddenActive)
			{
				RECT rw;
				GetWindowRect(pfb->hwndActive, &rw);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rw, 1);
				SetWindowLongPtrW(pfb->hwndActive, GWLP_USERDATA, (LONGX86)hiddenActive);
				dx = startPos - si.nPos;
				
				SetWindowPos(pfb->hwndActive, NULL, rw.left + dx, rw.top, 0, 0, 
								SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING);
				ShowWindow(pfb->hwndActive, SW_SHOWNA);
				if (pfb->focusedColumn == hiddenActive) SetFocus(pfb->hwndActive);
				hiddenActive = -1;
				startPos = si.nPos;
			}
			scrollCount = 0;
			return;
		default:
			return;
	}

	if (dx != 0)
	{
		if ((si.nPos + dx) < si.nMin) dx = si.nMin - si.nPos;
		else if ((si.nPos + dx) > (si.nMax - (INT)si.nPage))
		{
			dx = si.nMax - si.nPos - si.nPage;
			if (dx < 0) dx = 0;
		}
		if (dx != 0)
		{
			SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0L);

			if (pfb && -1 == hiddenActive && (WS_VISIBLE & GetWindowLongPtrW(pfb->hwndActive, GWL_STYLE)))
			{
				hiddenActive = FolderBrowser_GetListBoxColumn(pfb->hwndActive);
				if (hiddenActive < pfb->pColumns->size())
				{
					FBCOLUMN *pc = &pfb->pColumns->at(hiddenActive);
					if (pc) pc->firstVisible = (INT)SendMessageW(pfb->hwndActive, LB_GETTOPINDEX, 0, 0L);
					if (pfb->focusedColumn == hiddenActive) SetFocus(hwnd);
					SetWindowLongPtrW(pfb->hwndActive, GWLP_USERDATA, (LONGX86)-1);
					ShowWindow(pfb->hwndActive, SW_HIDE);
					startPos = si.nPos;
				}
			}

			si.fMask = SIF_POS;
			si.nPos += dx;
			SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);
			SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0L);
			if (scrollCount) 
			{
				ScrollWindowEx(hwnd, -dx, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_ERASE);
			//	InvalidateRect(hwnd, NULL, TRUE);
			}
			else
			{
				ScrollWindowEx(hwnd, -dx, 0, NULL, NULL, NULL, NULL, MAKELPARAM(SW_SMOOTHSCROLL, 150));
			}
			UpdateWindow(hwnd);	
		}
		scrollCount++;
	}
}

static void FolderBroser_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{
	PAINTSTRUCT ps;
	ZeroMemory(&ps, sizeof(PAINTSTRUCT));
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
	ps.fErase = (0 != (PRF_ERASEBKGND & options));
	FolderBrowser_Draw(hwnd, &ps);
}

static void FolderBrowser_OnMouseWheel(HWND hwnd, UINT vkCode, INT delta, POINTS pts)
{
	if (MK_CONTROL == vkCode)
	{
		SendMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(((delta > 0) ? SB_LINELEFT : SB_LINERIGHT), 0), NULL);
		SendMessageW(hwnd, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
	}
}

static BOOL FolderBrowser_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT hitTest, UINT uMsg)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);	
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	if(!GetScrollInfo(hwnd, SB_HORZ, &si)) si.nPos = 0;

	if(pfb && HTCLIENT == hitTest) 
	{
		POINT pt;
		GetCursorPos(&pt);
		MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);

		LONG left = -si.nPos;

		for (size_t i=0, count = pfb->pColumns->size(); i < count; i++)
		{
			left += pfb->pColumns->at(i).width;
			if (pt.x >= (left - SIZER_OVERLAP_LEFT) && pt.x < (left + SIZER_WIDTH + SIZER_OVERLAP_RIGHT))
			{
				if (sizerHover != i)
				{
					SetCursor(LoadCursor(NULL, IDC_SIZEWE));

					if (IsChild(GetActiveWindow(), hwnd))
					{
						RECT rc;
						GetClientRect(hwnd, &rc);
						rc.left = left;
						rc.right = left + SIZER_WIDTH;
						
						sizerHover = i;
						InvalidateRect(hwnd, &rc, FALSE);

						TRACKMOUSEEVENT tm;
						tm.cbSize = sizeof(TRACKMOUSEEVENT);
						tm.dwFlags = TME_LEAVE;
						tm.hwndTrack = hwnd;
						_TrackMouseEvent(&tm);
					}
				}
				return TRUE;
			}
			left += SIZER_WIDTH;
			if (left > pt.x) break;
		}
	}
	if (sizerHover < pfb->pColumns->size())
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		rc.right = rc.left - si.nPos;
		for (size_t i = 0; i <= sizerHover; i++) rc.right += (pfb->pColumns->at(i).width + SIZER_WIDTH);

		rc.left = rc.right - SIZER_WIDTH;
		sizerHover = -1;
		InvalidateRect(hwnd, &rc, FALSE);
	}
	DefWindowProcW(hwnd, WM_SETCURSOR, (WPARAM)hwndCursor, MAKELPARAM(hitTest, uMsg));
	return TRUE;
}

static BOOL FolderBrowser_OnSetFileSystemInfo(HWND hwnd, FILESYSTEMINFO *pfs)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);	
	if (!pfb) return FALSE;
	if (!pfs)
	{
		pfb->filesystem.fnFindFirstFile = FindFirstFileW;
		pfb->filesystem.fnFindNextFile = FindNextFileW;
		pfb->filesystem.fnFindClose = FindClose;
	}
	else
	{
		if (pfs->cbSize != sizeof(FILESYSTEMINFO) ||
			!pfs->fnFindFirstFile || !pfs->fnFindNextFile || !pfs->fnFindClose) return FALSE;
		CopyMemory(&pfb->filesystem, pfs, sizeof(FILESYSTEMINFO));
	}
	return TRUE;
}

static BOOL FolderBrowser_OnGetFileSystemInfo(HWND hwnd, FILESYSTEMINFO *pfs)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);	
	if (!pfb || !pfs || pfs->cbSize != sizeof(FILESYSTEMINFO)) return FALSE;
	CopyMemory(pfs, &pfb->filesystem, sizeof(FILESYSTEMINFO));
	return TRUE;
}

static void FolderBrowser_OnMouseLeave(HWND hwnd)
{
	FBDATA *pfb = GetFolderBrowser(hwnd);	

	if (pfb && sizerHover < pfb->pColumns->size())
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		if(!GetScrollInfo(hwnd, SB_HORZ, &si)) si.nPos = 0;

		rc.right = rc.left - si.nPos;
		for (size_t i = 0; i <= sizerHover; i++) rc.right += (pfb->pColumns->at(i).width + SIZER_WIDTH);

		rc.left = rc.right - SIZER_WIDTH;
		sizerHover = -1;
		InvalidateRect(hwnd, &rc, FALSE);
	}
	POINTS pts = { -1, -1};
	FolderBrowser_UpdateScrollHovering(hwnd, 0, pts);
}

static void FolderBrowser_OnParentNotify(HWND hwnd, UINT message, LPARAM lParam)
{
	switch(LOWORD(message))
	{
		case WM_RBUTTONDOWN:
			{
				FolderBrowser_HideActive(hwnd);
				DWORD flags = 0;
				if (0 != (0x80000000 & GetAsyncKeyState(VK_CONTROL))) flags |= MK_CONTROL;
				if (0 != (0x80000000 & GetAsyncKeyState(VK_SHIFT))) flags |= MK_SHIFT;
				if (0 != (0x80000000 & GetAsyncKeyState(VK_LBUTTON))) flags |= MK_LBUTTON;
				if (0 != (0x80000000 & GetAsyncKeyState(VK_RBUTTON))) flags |= MK_RBUTTON;
				if (0 != (0x80000000 & GetAsyncKeyState(VK_MBUTTON))) flags |= MK_MBUTTON;
				if (0 != (0x80000000 & GetAsyncKeyState(VK_XBUTTON1))) flags |= MK_XBUTTON1;
				if (0 != (0x80000000 & GetAsyncKeyState(VK_XBUTTON2))) flags |= MK_XBUTTON1;
				SendMessageW(hwnd, LOWORD(message), flags, lParam);
			}
			break;
	}
}

LRESULT CALLBACK FolderBrowser_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:			return FolderBrowser_OnCreateWindow(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			FolderBrowser_OnDestroyWindow(hwnd); return 0;
		case WM_PAINT:			FolderBrowser_OnPaint(hwnd); return 0;
		case WM_DRAWITEM:		return (LRESULT)FolderBrowser_OnDrawItem(hwnd, (INT)wParam, (DRAWITEMSTRUCT*)lParam);
		case WM_SETFONT:			FolderBrowser_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); break;
		case WM_SETFOCUS:		FolderBrowser_OnSetFocus(hwnd, (HWND)wParam); break;
		case WM_KILLFOCUS:		FolderBrowser_OnKillFocus(hwnd, (HWND)wParam); return 0;
		case WM_GETDLGCODE:		return FolderBrowser_OnGetDlgCode(hwnd, (INT)wParam, (MSG*)lParam);
		case WM_LBUTTONDOWN:		FolderBrowser_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONUP:		FolderBrowser_OnLButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONDBLCLK:	FolderBrowser_OnLButtonDblClick(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_RBUTTONDOWN:		FolderBrowser_OnRButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_RBUTTONUP:		FolderBrowser_OnRButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MOUSEMOVE:		FolderBrowser_OnMouseMove(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_WINDOWPOSCHANGED: FolderBrowser_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_KEYDOWN:			FolderBrowser_OnKeyDown(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_COMMAND:			FolderBorwser_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return 0;
		case WM_VKEYTOITEM:		return FolderBrowser_OnVKeyToItem(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		case WM_HSCROLL:			FolderBrowser_OnHScroll(hwnd, LOWORD(wParam), HIWORD(wParam)); return 0;
		case WM_PRINTCLIENT:		FolderBroser_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_SETCURSOR:		return (LRESULT)FolderBrowser_OnSetCursor(hwnd, (HWND)wParam, LOWORD(lParam), HIWORD(lParam)); 
		case WM_MOUSEWHEEL:		FolderBrowser_OnMouseWheel(hwnd, GET_KEYSTATE_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam), MAKEPOINTS(lParam)); return 0;
		case WM_MOUSELEAVE:		FolderBrowser_OnMouseLeave(hwnd); return 0;

		case WM_MOUSEACTIVATE:
			if (HIWORD(lParam) == WM_MBUTTONDOWN)
			{
				return MA_NOACTIVATEANDEAT;
			}
			break;

		case WM_MEASUREITEM:		return (LRESULT)FolderBrowser_OnMeasureItem(hwnd, (INT)wParam, (MEASUREITEMSTRUCT*)lParam);
		case WM_PARENTNOTIFY:	FolderBrowser_OnParentNotify(hwnd, (UINT)wParam, lParam); return 0; 

		case FBM_GETBKCOLOR:				return (LRESULT)FolderBrowser_OnGetBkColor(hwnd);
		case FBM_SETBKCOLOR:				return (LRESULT)FolderBrowser_OnSetBkColor(hwnd, (COLORREF)lParam);
		case FBM_GETTEXTCOLOR:			return (LRESULT)FolderBrowser_OnGetTextColor(hwnd);
		case FBM_SETTEXTCOLOR:			return (LRESULT)FolderBrowser_OnSetTextColor(hwnd, (COLORREF)lParam);
		case FBM_GETFOLDERBROWSERINFO:	return (LRESULT)FolderBrowser_OnGetFolderBrowserInfo(hwnd, (FOLDERBROWSERINFO*)lParam);
		case FBM_SETROOT:				return (LRESULT)FolderBrowser_OnSetRootFolder(hwnd, (LPCWSTR)lParam);
		case FBM_GETROOT:				return (LRESULT)FolderBrowser_OnGetRootFolder(hwnd, (LPWSTR)lParam, (INT)wParam);
		case FBM_SETFILESYSTEMINFO:		return (LRESULT)FolderBrowser_OnSetFileSystemInfo(hwnd, (FILESYSTEMINFO*)lParam);
		case FBM_GETFILESYSTEMINFO:		return (LRESULT)FolderBrowser_OnGetFileSystemInfo(hwnd, (FILESYSTEMINFO*)lParam);
		case FBM_GETCURRENTPATH:			return (LRESULT)FolderBrowser_OnGetCurrentPath(hwnd, (LPWSTR)lParam, (INT)wParam);
		case FBM_SETCURRENTPATH:			return (LRESULT)FolderBrowser_OnSetCurrentPath(hwnd, (LPWSTR)lParam, (BOOL)wParam);
		case FBM_ENSUREVISIBLE:			return (LRESULT)FolderBrowser_OnEnsureVisible(hwnd, LOWORD(wParam), HIWORD(wParam));
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}