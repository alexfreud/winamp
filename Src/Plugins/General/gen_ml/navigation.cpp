#include "./navigation.h"

#include "./ml.h"
#include "./ml_ipc_0313.h"

#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"

#include "./skinning.h"
#include "./stockobjects.h"

#include "../winamp/wa_dlg.h"
#include "../winamp/gen.h"

#include "resource.h"
#include "api__gen_ml.h"
#include "../nu/CGlobalAtom.h"
#include "../nu/trace.h"
#include <windowsx.h>
#include <strsafe.h>

extern "C" winampGeneralPurposePlugin plugin;

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define PACKVERSION(major,minor) MAKELONG(minor,major)

#define SEPARATOR				L'/'
#define MLV_PREFIX_A			L"mlv10_"
#define EMPTY_ITEM_NAME			L"<<empty>>"
#define ITEM_SHORTNAME_MAX		512

#define SAVEVIEW_BUFFER_MAX		1024
#define ML_VIEW_MAX				32

#define NAVITEM_RANGE_MIN		8000
static CGlobalAtom NAVMGR_HWNDPROPW(L"NAVMNGR");
static CGlobalAtom WNDPROP_SCCTRLW(L"SCCTRL");

#define IREC_COUNT				3

#define IREC_NORMAL				0x00
#define IREC_SELECTED			0x01
#define IREC_INACTIVE			0x02

#define DRAGIMAGE_OFFSET_X		8
#define DRAGIMAGE_OFFSET_Y		0

typedef struct _SCEDIT
{
	WNDPROC	fnOldProc;
	BOOL	fUnicode;
}SCEDIT;

typedef struct _SCTREE
{
	WNDPROC	fnOldProc;
	BOOL	fUnicode;
	DWORD	focused;
	BOOL	supressEdit;
}SCTREE;

typedef struct _NAVITEMSTATEREC
{
	LPWSTR	pszFullName;
	INT		fCollapsed;
	INT		nOrder;
} NAVITEMSTATEREC;

typedef struct _NAVMNGR
{
	HWND				hwndHost;
	INT					lastUsedId;
	C_Config			*pConfig;
	HMLIMGLST			hmlilImages;
	INT					lockUpdate;
	HTREEITEM			lockFirst;
	HTREEITEM			lockSelected;
	INT					lockSelPos;
	INT					lastOrderIndex;
	NAVITEMSTATEREC		*pStates;
	INT					statesCount;
	UINT				style;
	NAVITEMDRAW_I		drawStruct;
	HFONT				hfontOld;
	UINT				drawInternal;

	// callbacks
	ONNAVITEMCLICK_I			fnOnItemClick;
	ONNAVITEMSELECTED_I			fnOnItemSelected;
	ONNAVCTRLKEYDOWN_I			fnOnKeyDown;
	ONNAVCTRLBEGINDRAG_I		fnOnBeginDrag;
	ONNAVITEMGETIMAGEINDEX_I	fnOnItemGetImageIndex;
	ONNAVITEMBEGINTITLEEDIT_I	fnOnBeginTitleEdit;
	ONNAVCTRLENDTITLEEDIT_I		fnOnEndTitleEdit;
	ONNAVITEMDELETE_I			fnOnItemDelete; 
	ONNAVITEMDRAW_I				fnOnCustomDraw;
	ONNAVITEMSETCURSOR_I		fnOnSetCursor;
	ONNAVITEMHITTEST_I			fnOnHitTest;
	ONNAVCTRLDESTROY_I			fnOnDestroy;
} NAVMNGR, *PNAVMNGR;

typedef struct _NAVITM
{
    INT			id;
	INT			iImage;
	INT			iSelectedImage;
	WORD		sortOrder;
	UINT		style;
	HWND		hwndTree;
	HTREEITEM	hTreeItem;
	LPWSTR		pszText;
	INT			cchTextMax;
	LPWSTR		pszInvariant;
	INT			cchInvariantMax;
	HFONT		hFont;
	BOOL		fBlockInvalid; // if set to TRUE operation do not need to inavlidate item it will be invalidated later;
	LPARAM		lParam;
} NAVITM, *PNAVITM;

typedef struct _TREEITEMSEARCH
{
	TVITEMEXW	item;
	INT_PTR		itemData;
	BOOL		fFound;
	UINT		flags;
} TREEITEMSEARCH;


typedef struct _SAVEVIEWDATA
{
	TVITEMEXW	item;
	WCHAR		buffer[SAVEVIEW_BUFFER_MAX];
	WCHAR		mlview[ML_VIEW_MAX];
	C_Config	*pConfig;
	INT			counter_root;
	INT			counter_write;
}SAVEVIEWDATA;

typedef struct _NAVENUMSTRUCT
{
	TVITEMW			item;
	NAVENUMPROC_I	callback;
	LPARAM			lParam;
	HNAVCTRL		hNav;
} NAVENUMSTRUCT;

typedef BOOL (CALLBACK *TREEITEMCB)(HWND, HTREEITEM, LPVOID);

#define ABS(x) (((x) > 0) ? (x) : (-x))

#define NAVSTYLE_DEFAULT					0x0000
#define NAVSTYLE_MOUSEDOWNSELECT			0x0001
#define NAVSTYLE_FOCUSED					0x0002
#define NAVSTYLE_EDITMODE					0x0004
// helpers 

static BOOL GetItemColors(HNAVCTRL hNav, UINT itemState, COLORREF *rgbBk, COLORREF *rgbFg)
{
	INT rgbBkIndex, rgbFgIndex;
	switch(itemState)
	{
		case (NIS_SELECTED_I | NIS_FOCUSED_I):	rgbBkIndex = WADLG_SELBAR_BGCOLOR; rgbFgIndex = WADLG_SELBAR_FGCOLOR; break;
		case NIS_SELECTED_I:					rgbBkIndex = WADLG_INACT_SELBAR_BGCOLOR; rgbFgIndex = WADLG_INACT_SELBAR_FGCOLOR; break;
		case NIS_DROPHILITED_I:					rgbBkIndex = WADLG_INACT_SELBAR_BGCOLOR; rgbFgIndex = WADLG_INACT_SELBAR_FGCOLOR; break;
		default:								rgbBkIndex = WADLG_ITEMBG; rgbFgIndex = WADLG_ITEMFG; break;
	}
	if (rgbBk) *rgbBk = WADlg_getColor(rgbBkIndex);
	if (rgbFg) *rgbFg = WADlg_getColor(rgbFgIndex);
	return TRUE;
}

static BOOL EnumerateTreeItems(HWND hwndTree, HTREEITEM hStart, TREEITEMCB pfnItemCallback, LPVOID user)
{
	HTREEITEM hChild;
	if (!pfnItemCallback) return FALSE;
	
	if (NULL == hStart || TVI_ROOT == hStart)  
		hStart = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_ROOT, (LPARAM)0);

	while(NULL != hStart)
	{
		if (!pfnItemCallback(hwndTree, hStart, user)) 
			return FALSE;
		hChild = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)hStart);
		if (NULL != hChild) 
		{
			if (!EnumerateTreeItems(hwndTree, hChild, pfnItemCallback, user)) 
				return FALSE;
		}
		hStart = (HTREEITEM)SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)hStart);
	}
	return TRUE;
}

static BOOL CALLBACK FindTreeItemByIdCB(HWND hwndTree, HTREEITEM hItem, LPVOID user)
{
	if (!user) return FALSE;

	((TREEITEMSEARCH*)user)->item.hItem = hItem;
	if ((BOOL)SendMessageW(hwndTree, TVM_GETITEMW, (WPARAM)0, (LPARAM)&((TREEITEMSEARCH*)user)->item))
	{
		if (((TREEITEMSEARCH*)user)->item.lParam && 
			(INT)((TREEITEMSEARCH*)user)->itemData == ((NAVITM*)((TREEITEMSEARCH*)user)->item.lParam)->id) 
		{
			((TREEITEMSEARCH*)user)->fFound = TRUE;
			return FALSE;
		}
	}
	return TRUE;
}

static INT CompareItemName(LCID Locale, UINT compFlags, LPCWSTR pszString, INT cchLength, HNAVITEM hItem)
{
	INT result;
	if (!hItem || !pszString) return 0; // error

	if (NICF_INVARIANT_I & compFlags)
	{
		result = (((NAVITM*)hItem)->pszInvariant) ? 
				CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, ((NAVITM*)hItem)->pszInvariant, -1, pszString, cchLength) :
				CSTR_LESS_THAN;
		if (CSTR_EQUAL == result) return CSTR_EQUAL;
	}
	if (NICF_DISPLAY_I & compFlags || 0 == (compFlags & ~NICF_IGNORECASE_I))
	{
		result = (((NAVITM*)hItem)->pszText) ? 
				CompareStringW(Locale, (NICF_IGNORECASE_I & compFlags) ? NORM_IGNORECASE : 0, ((NAVITM*)hItem)->pszText, -1, pszString, cchLength) :
				CSTR_LESS_THAN;
	}
	return result;
}

static BOOL CALLBACK FindTreeItemByNameCB(HWND hwndTree, HTREEITEM hItem, LPVOID user)
{
	if (!user) return FALSE;

	((TREEITEMSEARCH*)user)->item.hItem = hItem;

	if ((BOOL)SendMessageW(hwndTree, TVM_GETITEMW, (WPARAM)0, (LPARAM)&((TREEITEMSEARCH*)user)->item))
	{
		if (CSTR_EQUAL == CompareItemName(((TREEITEMSEARCH*)user)->item.state, ((TREEITEMSEARCH*)user)->flags, 
											(LPCWSTR)((TREEITEMSEARCH*)user)->itemData, ((TREEITEMSEARCH*)user)->item.cchTextMax, 
											(NAVITM*)((TREEITEMSEARCH*)user)->item.lParam))
		{
			((TREEITEMSEARCH*)user)->fFound = TRUE;
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL CALLBACK ItemSizedCB(HWND hwndTree, HTREEITEM hItem, LPVOID user)
{
	((TVITEMW*)user)->hItem = hItem;
	if (!SendMessageW(hwndTree, TVM_GETITEMW, (WPARAM)0, (LPARAM)user)) return FALSE;
	if (((TVITEMW*)user)->lParam)
	{
		if (NIS_CUSTOMDRAW_I & ((NAVITM*)((TVITEMW*)user)->lParam)->style)
		{
			*(HTREEITEM*)((TVITEMW*)user)->pszText = hItem;
			if (SendMessageW(hwndTree, TVM_GETITEMRECT, FALSE, (LPARAM)((TVITEMW*)user)->pszText)) 
			{
				InvalidateRect(hwndTree, (RECT*)((TVITEMW*)user)->pszText, TRUE);
			}
		}
	}
	return TRUE;
}

static HNAVITEM FindNavItemByNavIdEx(HWND hwndTree, INT itemId, HTREEITEM hStart)
{
	TREEITEMSEARCH search;
	search.fFound	= FALSE;
	search.itemData	= (INT_PTR)itemId;
	search.item.mask	= TVIF_PARAM;
	search.flags	= 0;

	EnumerateTreeItems(hwndTree, hStart, FindTreeItemByIdCB, &search);
	return (search.fFound) ? (HNAVITEM)search.item.lParam : NULL;
}

static INT GetNextFreeItemId(HNAVCTRL hNav)
{
	if (!hNav) return 0;
	while (FindNavItemByNavIdEx(((NAVMNGR*)hNav)->hwndHost, ++((NAVMNGR*)hNav)->lastUsedId, TVI_ROOT)); 
	return ((NAVMNGR*)hNav)->lastUsedId;
}

static INT GetRealImageIndex(HNAVCTRL hNav, HNAVITEM hItem, INT imageType, COLORREF rgbBk, COLORREF rgbFg)
{
	INT mlilIndex;
	if (!hNav) return -1;
	mlilIndex = NavItemI_GetImageIndex(hItem, imageType);
	if (-1 == mlilIndex )
	{
		if (((NAVMNGR*)hNav)->fnOnItemGetImageIndex) mlilIndex = ((NAVMNGR*)hNav)->fnOnItemGetImageIndex(hNav, hItem, imageType);
		if (-1 == mlilIndex) return -1;
	}
	return MLImageListI_GetRealIndex(((NAVMNGR*)hNav)->hmlilImages, mlilIndex, rgbBk, rgbFg);
}

static BOOL CALLBACK EnumNavItemCB(HWND hwndTree, HTREEITEM hItem, LPVOID user)
{
	NAVENUMSTRUCT *pData;
	if (!user) return FALSE;

	pData = (NAVENUMSTRUCT*)user;
	pData->item.hItem = hItem;

	return  (BOOL)SendMessageW(hwndTree, TVM_GETITEMW, (WPARAM)0, (LPARAM)&(pData->item)) ?
					pData->callback((HNAVITEM)pData->item.lParam, pData->lParam) : TRUE;
}

static BOOL CALLBACK SaveNavigationCB(HWND hwndTree, HTREEITEM hItem, LPVOID user)
{
	if (!user) return FALSE; // very bad

	((SAVEVIEWDATA*)user)->item.hItem = hItem;

	if ((BOOL)SendMessageW(hwndTree, TVM_GETITEMW, (WPARAM)0, (LPARAM)&((SAVEVIEWDATA*)user)->item))
	{
		INT order;
		BOOL bCollapsed = (SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)hItem) && !(TVIS_EXPANDED & ((SAVEVIEWDATA*)user)->item.state));
		if (!SendMessageW(hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_PARENT, (LPARAM)hItem))
		{
			order = ++((SAVEVIEWDATA*)user)->counter_root;
		}
		else order = -1;
		if (order != -1 || bCollapsed || (TVIS_SELECTED & ((SAVEVIEWDATA*)user)->item.state))
		{
			INT remain = NavItemI_GetFullName((HNAVITEM)((SAVEVIEWDATA*)user)->item.lParam, ((SAVEVIEWDATA*)user)->buffer, SAVEVIEW_BUFFER_MAX);
			if (remain)
			{
				if ((order != -1 || bCollapsed) && remain < (SAVEVIEW_BUFFER_MAX - 8) )
				{
					if (S_OK == StringCchPrintfW(((SAVEVIEWDATA*)user)->buffer + remain, SAVEVIEW_BUFFER_MAX - remain, L",%d,%d", bCollapsed, order) &&
						S_OK == StringCchPrintfW(((SAVEVIEWDATA*)user)->mlview, ML_VIEW_MAX, MLV_PREFIX_A L"%02d", (((SAVEVIEWDATA*)user)->counter_write + 1)))
					{
						((SAVEVIEWDATA*)user)->pConfig->WriteString(((SAVEVIEWDATA*)user)->mlview, ((SAVEVIEWDATA*)user)->buffer);
						((SAVEVIEWDATA*)user)->counter_write++;
					}
				}
			}
		}
	}

	return TRUE;
}

static HNAVITEM GetNavItemFromTreeItem(HNAVCTRL hNav, HTREEITEM hTreeItem)
{
	TVITEMW item;
	item.mask = TVIF_PARAM;
	item.hItem = hTreeItem;

	if (!hNav) return NULL;

	if (!SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETITEMW, (WPARAM)0, (LPARAM)&item)) return NULL;
	return (HNAVITEM)item.lParam;
}

static HNAVITEM NavItemI_GetNextEx(HNAVITEM hItem, UINT flag) // use TVGN_  as flags
{
	TVITEMW treeItem;

	if (!hItem) return NULL;

	treeItem.hItem = (HTREEITEM)SendMessageW(((NAVITM*)hItem)->hwndTree,  TVM_GETNEXTITEM, (WPARAM)flag, (LPARAM)((NAVITM*)hItem)->hTreeItem);

	if (!treeItem.hItem) return NULL;

	treeItem.mask = TVIF_PARAM;
	return (SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETITEMW, (WPARAM)0, (LPARAM)&treeItem)) ? (HNAVITEM)treeItem.lParam : NULL;
}

static void PerformCustomHitTest(HNAVCTRL hNav, POINT pt, UINT *pHitFlags, HNAVITEM *phItem) // returns nav hit test result
{
	UINT flags, test;

	flags = 0;
	test = (pHitFlags) ? *pHitFlags : 1;

	if (TVHT_NOWHERE & test)			flags |= NAVHT_NOWHERE_I;
	if (TVHT_ONITEMLABEL & test)		flags |= NAVHT_ONITEM_I;
	if (TVHT_ONITEMINDENT & test)	flags |= NAVHT_ONITEMINDENT_I;
	if (TVHT_ONITEMRIGHT & test)		flags |= NAVHT_ONITEMRIGHT_I;
	if (TVHT_ABOVE & test)			flags |= NAVHT_ABOVE_I;
	if (TVHT_BELOW & test)			flags |= NAVHT_BELOW_I;
	if (TVHT_TORIGHT & test)		flags |= NAVHT_TORIGHT_I;
	if (TVHT_TOLEFT & test)			flags |= NAVHT_TOLEFT_I;

	if (pHitFlags)  *pHitFlags = flags;

	if (hNav && phItem && *phItem && pHitFlags)
	{
		if (TVHT_ONITEMICON & test)	*pHitFlags |= (SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM,
																	(WPARAM)TVGN_CHILD,	(LPARAM)((NAVITM*)(*phItem))->hTreeItem)) ?
																NAVHT_ONITEMBUTTON_I : NAVHT_ONITEM_I;

		if ((NIS_WANTHITTEST_I & ((NAVITM*)(*phItem))->style) && ((NAVMNGR*)hNav)->fnOnHitTest)
		{
			((NAVMNGR*)hNav)->fnOnHitTest(hNav, pt, pHitFlags, phItem, ((NAVITM*)(*phItem))->lParam);
		}
	}
}

static HTREEITEM TreeItemFromCursor(HNAVCTRL hNav, HTREEITEM *phtiWantExpand)
{
	HTREEITEM hTreeItem;
	TVHITTESTINFO hit;

	if (phtiWantExpand) *phtiWantExpand = NULL;

	if(!GetCursorPos(&hit.pt)) return NULL;

	MapWindowPoints(HWND_DESKTOP, ((NAVMNGR*)hNav)->hwndHost, &hit.pt, 1);
	hTreeItem = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_HITTEST, (WPARAM)0, (LPARAM)&hit);

	if (0 == (TVHT_ONITEM & hit.flags))
	{
		if ((TVHT_ONITEMRIGHT | TVHT_ONITEMINDENT) & hit.flags)
		{
			if (0 == (TVS_FULLROWSELECT & GetWindowLongPtrW(((NAVMNGR*)hNav)->hwndHost, GWL_STYLE))) hTreeItem = NULL;
		}
	}

	if(hTreeItem)
	{
		HNAVITEM hItem;
		hItem = GetNavItemFromTreeItem(hNav, hTreeItem);
		PerformCustomHitTest(hNav, hit.pt, &hit.flags, &hItem);

		hTreeItem = (hItem) ? ((NAVITM*)hItem)->hTreeItem : NULL;
		if (NAVHT_ONITEMBUTTON_I & hit.flags)
		{
			if (phtiWantExpand) *phtiWantExpand = hTreeItem;
			hTreeItem = NULL;
		}
	}
	return hTreeItem;
}

static NAVITEMSTATEREC *NavCtrlI_FindItemStateRec(HNAVCTRL hNav, HNAVITEM hItem, BOOL fClearOnSuccess)
{
	wchar_t name[1024] = {0};

	if (!hNav || !hItem) return NULL;
	if (NavItemI_GetFullName(hItem, name, 1024))
	{
		for (INT idx = 0; idx < ((NAVMNGR*)hNav)->statesCount; idx++)
		{
			if (((NAVMNGR*)hNav)->pStates[idx].pszFullName && 
				CSTR_EQUAL == CompareStringW(LOCALE_USER_DEFAULT, 0, name, -1, 
												((NAVMNGR*)hNav)->pStates[idx].pszFullName, -1)) 
			{
				if (fClearOnSuccess) 
				{
					free(((NAVMNGR*)hNav)->pStates[idx].pszFullName);
					((NAVMNGR*)hNav)->pStates[idx].pszFullName = NULL;
				}
				return &((NAVMNGR*)hNav)->pStates[idx];
			}
		}
	}
	return NULL;
}

static LRESULT EditCtrlWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SCEDIT *pEdit;
	pEdit = (SCEDIT*)GetPropW(hwnd, WNDPROP_SCCTRLW);
	if (!pEdit) return (IsWindowUnicode(hwnd)) ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : DefWindowProcA(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
		case WM_NCDESTROY: // detach
			RemovePropW(hwnd, WNDPROP_SCCTRLW);
			if (pEdit->fUnicode) 
			{
				CallWindowProcW(pEdit->fnOldProc, hwnd, uMsg, wParam, lParam);
				SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pEdit->fnOldProc);
			}
			else
			{
				CallWindowProcA(pEdit->fnOldProc, hwnd, uMsg, wParam, lParam);
				SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pEdit->fnOldProc);            
			}
			free(pEdit);
			return 0;
		case WM_ERASEBKGND: return 1;
		case WM_NCPAINT:
			{
				HDC hdc;
			
				hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_WINDOW | DCX_INTERSECTUPDATE | DCX_CLIPSIBLINGS);
				if (hdc)
				{
					RECT rc;
					GetClientRect(hwnd, &rc);
					FrameRect(hdc, &rc,  (HBRUSH)MlStockObjects_Get(ITEMTEXT_BRUSH));
					ReleaseDC(hwnd, hdc);
				}
				return 0;
			}
			break;
		case WM_PAINT:
			{
				RECT rc, rv;
				GetClientRect(hwnd, &rc);
				SetRect(&rv, 0, 0, rc.right, 1);
				ValidateRect(hwnd, &rv);
				SetRect(&rv, 0, rc.bottom - 1, rc.right, rc.bottom);
				ValidateRect(hwnd, &rv);
				SetRect(&rv, 0, 1, 1, rc.bottom);
				ValidateRect(hwnd, &rv);
				SetRect(&rv, rc.right-1, 1, rc.right, rc.bottom);
				ValidateRect(hwnd, &rv);
				break;
			}
		case WM_GETDLGCODE:
			switch(wParam)
			{
				case VK_RETURN:
					SendMessageW(GetParent(hwnd), TVM_ENDEDITLABELNOW, FALSE, 0L); return 0; // tell trew-view that we done and return 0
				case VK_ESCAPE:
					SendMessageW(GetParent(hwnd), TVM_ENDEDITLABELNOW, TRUE, 0L); return 0; // tell trew-view that we done and return 0
			}
			break;
		case WM_SETCURSOR:
			if (HTCLIENT == LOWORD(lParam))
			{
				SetCursor(LoadCursor(NULL, IDC_IBEAM));
				return 0;
			}
			break;
	}
	return (pEdit->fUnicode) ? CallWindowProcW(pEdit->fnOldProc, hwnd, uMsg, wParam, lParam) :
								CallWindowProcA(pEdit->fnOldProc, hwnd, uMsg, wParam, lParam);
}

static LRESULT TreeViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SCTREE *pTree;
	pTree = (SCTREE*)GetPropW(hwnd, WNDPROP_SCCTRLW);
	if (!pTree)
		return (IsWindowUnicode(hwnd)) ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : DefWindowProcA(hwnd, uMsg, wParam, lParam);
	
	switch(uMsg)
	{
		case WM_NCDESTROY: // detach
			RemovePropW(hwnd, WNDPROP_SCCTRLW);
			if (pTree->fUnicode) 
			{
				CallWindowProcW(pTree->fnOldProc, hwnd, uMsg, wParam, lParam);
				SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pTree->fnOldProc);
			}
			else
			{
				CallWindowProcA(pTree->fnOldProc, hwnd, uMsg, wParam, lParam);
				SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pTree->fnOldProc);            
			}
			free(pTree);
			return 0;
		case WM_ERASEBKGND:	
			if (wParam && (TVS_FULLROWSELECT & GetWindowLongPtrW(hwnd, GWL_STYLE)))
			{				
				RECT rc;
				HTREEITEM hItem;

				GetClientRect(hwnd, &rc);

				hItem = (HTREEITEM)SendMessageW(hwnd, TVM_GETNEXTITEM, TVGN_LASTVISIBLE, 0L);

				if (hItem)
				{
					RECT ri;
					*(HTREEITEM*)&ri = hItem;
					if (SendMessageW(hwnd, TVM_GETITEMRECT, FALSE, (LPARAM)&ri)) rc.top  = ri.bottom;
				}				
				if (rc.top < rc.bottom)
				{
					SetBkColor((HDC)wParam, WADlg_getColor(WADLG_ITEMBG));
					ExtTextOutW((HDC)wParam, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);
				}
				return 1;	
			}
			return 0;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			{
				TVHITTESTINFO hit;
				HNAVITEM hItem;
				NAVMNGR *pMngr;

				hit.pt.x = GET_X_LPARAM(lParam);
				hit.pt.y = GET_Y_LPARAM(lParam);
				SendMessageW(hwnd, TVM_HITTEST, (WPARAM)0, (LPARAM)&hit);

				pMngr = (NAVMNGR*)GetPropW(hwnd, NAVMGR_HWNDPROPW); 

				if (hit.hItem)
				{
					hItem = GetNavItemFromTreeItem(pMngr, hit.hItem);
					PerformCustomHitTest(pMngr, hit.pt, &hit.flags, &hItem);
					if (hItem && pMngr && (NAVSTYLE_MOUSEDOWNSELECT & pMngr->style))
					{
						SendMessageW(hwnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)((NAVITM*)hItem)->hTreeItem);
					}
				}
				else
					hItem = NULL;

				if (!hItem)
				{
					SendMessageW(hwnd, TVM_ENDEDITLABELNOW, (WPARAM)TRUE, 0L);
					return 0;
				}
				else
				{	
					HWND hEdit = FindWindowExW(hwnd, NULL, L"Edit", NULL);
					if (NULL != hEdit && IsWindowVisible(hEdit))
					{
						PostMessageW(hwnd, uMsg, wParam, lParam);
					}
					else
					{
						pTree->supressEdit = FALSE;
						if (0 != pTree->focused)
						{
							DWORD clicked = GetTickCount();
							if (clicked >= pTree->focused && (clicked - pTree->focused) < 200)
								pTree->supressEdit = TRUE;
							
							pTree->focused = 0;
						}
					}
				}
			}
			break;

		case WM_LBUTTONDBLCLK:
			if (TVS_FULLROWSELECT == (TVS_FULLROWSELECT & (UINT)GetWindowLongPtrW(hwnd, GWL_STYLE)))
			{
				TVHITTESTINFO test;
				test.pt.x = GET_X_LPARAM(lParam);
				test.pt.y = GET_Y_LPARAM(lParam);
				if(SendMessageW(hwnd, TVM_HITTEST, 0, (LPARAM)&test) && (TVHT_ONITEMRIGHT & test.flags))
				{
					HWND hParent;
					NMHDR hdr;
					hdr.code = NM_DBLCLK;
					hdr.hwndFrom = hwnd;
					hdr.idFrom = GetDlgCtrlID(hwnd);
                    hParent = GetParent(hwnd);
					if (hParent) SendMessageW(hParent, WM_NOTIFY, hdr.idFrom, (LPARAM)&hdr);	
					return 0;
				}
			}
			break;

		case WM_CTLCOLOREDIT:
			SetTextColor((HDC)wParam, WADlg_getColor(WADLG_ITEMFG));
			SetBkColor((HDC)wParam, WADlg_getColor(WADLG_ITEMBG));
			return (LRESULT)MlStockObjects_Get(ITEMBCK_BRUSH);
			
		case WM_SIZE:
			{				
				TVITEMW item;
				RECT ri;

				item.mask = TVIF_PARAM;
				item.pszText = (LPWSTR)&ri;
				EnumerateTreeItems(hwnd,  NULL, ItemSizedCB, &item);
			}
			break;
		case WM_CHAR:
			pTree->supressEdit = FALSE;
			return (VK_RETURN == wParam) ? 0 : 
						((pTree->fUnicode) ? CallWindowProcW(pTree->fnOldProc, hwnd, uMsg, wParam, lParam) :
												CallWindowProcA(pTree->fnOldProc, hwnd, uMsg, wParam, lParam));

		case WM_GETDLGCODE:
			{
				LRESULT r = (pTree->fUnicode) ? CallWindowProcW(pTree->fnOldProc, hwnd, uMsg, wParam, lParam) :
								CallWindowProcA(pTree->fnOldProc, hwnd, uMsg, wParam, lParam);
				if (lParam)
				{
					MSG *pMsg = (LPMSG)lParam;
					if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
					{
						switch(pMsg->wParam)
						{
							case VK_RETURN: r |= DLGC_WANTMESSAGE; break;
							case VK_TAB:
							case VK_ESCAPE: SendMessageW(hwnd, TVM_ENDEDITLABELNOW, TRUE, 0L); break;
						}
					}
				}
				return r;
			}

		case WM_SETFOCUS:
			pTree->focused = GetTickCount();
			break;

		case WM_TIMER:
			if (42 == wParam && FALSE != pTree->supressEdit)
			{
				KillTimer(hwnd, wParam);
				pTree->supressEdit = FALSE;
				return 0;
			}
			break;
	}
	return (pTree->fUnicode) ? CallWindowProcW(pTree->fnOldProc, hwnd, uMsg, wParam, lParam) :
								CallWindowProcA(pTree->fnOldProc, hwnd, uMsg, wParam, lParam);
}

static BOOL SubclassEditControl(HWND hwndEdit)
{
	if (!hwndEdit || !IsWindow(hwndEdit)) return FALSE;
	SCEDIT *pEdit = (SCEDIT*)calloc(1, sizeof(SCEDIT));
	if (!pEdit) return FALSE;

	pEdit->fUnicode = IsWindowUnicode(hwndEdit);
	pEdit->fnOldProc = (WNDPROC)(LONG_PTR)((pEdit->fUnicode) ? SetWindowLongPtrW(hwndEdit, GWLP_WNDPROC, (LONGX86)(LONG_PTR)EditCtrlWndProc) :
															   SetWindowLongPtrA(hwndEdit, GWLP_WNDPROC, (LONGX86)(LONG_PTR)EditCtrlWndProc));
	if (!pEdit->fnOldProc || !SetPropW(hwndEdit, WNDPROP_SCCTRLW, pEdit))
	{
		if (pEdit->fnOldProc)
		{
			if (pEdit->fUnicode) SetWindowLongPtrW(hwndEdit, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pEdit->fnOldProc);
			else SetWindowLongPtrA(hwndEdit, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pEdit->fnOldProc);
		}
		free(pEdit);
		return FALSE;
	}

	return TRUE;
}

static BOOL SubclassTreeView(HWND hwndTree)
{
	if (!hwndTree || !IsWindow(hwndTree)) return FALSE;
	SCTREE *pTree = (SCTREE*)calloc(1, sizeof(SCTREE));
	if (!pTree) return FALSE;

	pTree->fUnicode = IsWindowUnicode(hwndTree);
	pTree->fnOldProc = (WNDPROC)(LONG_PTR)((pTree->fUnicode) ? SetWindowLongPtrW(hwndTree, GWLP_WNDPROC, (LONGX86)(LONG_PTR)TreeViewWndProc) :
															   SetWindowLongPtrA(hwndTree, GWLP_WNDPROC, (LONGX86)(LONG_PTR)TreeViewWndProc));
	if (!pTree->fnOldProc || !SetPropW(hwndTree, WNDPROP_SCCTRLW, pTree))
	{
		if (pTree->fnOldProc)
		{
			if (pTree->fUnicode) SetWindowLongPtrW(hwndTree, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pTree->fnOldProc);
			else SetWindowLongPtrA(hwndTree, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pTree->fnOldProc);
		}
		free(pTree);
		return FALSE;
	}
	return TRUE;
}

static BOOL NavCtrlI_SaveStates(HNAVCTRL hNav)
{
	BOOL result;
	SAVEVIEWDATA		save; 

	if (!hNav || !((NAVMNGR*)hNav)->pConfig) return FALSE;

	save.item.mask		= TVIF_STATE | TVIF_PARAM;
	save.item.stateMask	= TVIS_EXPANDED | TVIS_SELECTED;
	save.pConfig		= ((NAVMNGR*)hNav)->pConfig;
	save.counter_root	= 0;
	save.counter_write	= 0;

	result = EnumerateTreeItems(((NAVMNGR*)hNav)->hwndHost, TVI_ROOT, SaveNavigationCB, &save);
	if (!result) save.counter_write = 0;
	((NAVMNGR*)hNav)->pConfig->WriteInt(MLV_PREFIX_A L"count", save.counter_write);

	return  result;
}

static BOOL NavCtrlI_DeleteStates(HNAVCTRL hNav)
{
	if (!hNav) return FALSE;
	if (((NAVMNGR*)hNav)->pStates) 
	{
		INT i;
		for (i =0; i < ((NAVMNGR*)hNav)->statesCount; i++)
		{
			if (((NAVMNGR*)hNav)->pStates[i].pszFullName) free(((NAVMNGR*)hNav)->pStates[i].pszFullName);
		}
		free(((NAVMNGR*)hNav)->pStates);
		((NAVMNGR*)hNav)->pStates = NULL;
	}
	((NAVMNGR*)hNav)->statesCount = 0;
	return TRUE;
}

static BOOL NavCtrlI_LoadStates(HNAVCTRL hNav)
{
	INT index, count, len;
	wchar_t mlview[ML_VIEW_MAX] = {0};
	const wchar_t *data;
	if (!hNav || !((NAVMNGR*)hNav)->pConfig) return FALSE;

	if (!NavCtrlI_DeleteStates(hNav)) return FALSE;

	count  = ((NAVMNGR*)hNav)->pConfig->ReadInt(MLV_PREFIX_A L"count", 0);
	if (count < 1) return TRUE;

	((NAVMNGR*)hNav)->pStates = (NAVITEMSTATEREC*)calloc(count, sizeof(NAVITEMSTATEREC));
	if (!((NAVMNGR*)hNav)->pStates) return FALSE;

	for (index = 0; index < count; index++)
	{
		StringCchPrintfW(mlview, ML_VIEW_MAX, MLV_PREFIX_A L"%02d", index + 1);
		//AutoWide aw(((NAVMNGR*)hNav)->pConfig->ReadString(mlview, ""), CP_UTF8);
		data = ((NAVMNGR*)hNav)->pConfig->ReadString(mlview, L"");//(LPCWSTR)aw;
		if (data && *data)
		{
			len = lstrlenW(data);
			if (len > 4 && len < 4096)
			{
				while (len > 0 && data[len-1] != L',') len--;
				if (len)
				{
					((NAVMNGR*)hNav)->pStates[index].nOrder = _wtoi(&data[len]);
					len--;
					while (len > 0 && data[len-1] != L',')len--;
					if (len) ((NAVMNGR*)hNav)->pStates[index].fCollapsed = ( 0 != _wtoi(&data[len]));
					if (len > 1) 
					{
						((NAVMNGR*)hNav)->pStates[index].pszFullName = (LPWSTR)calloc(len, sizeof(wchar_t));
						if (!((NAVMNGR*)hNav)->pStates[index].pszFullName) continue;
						if (S_OK == StringCchCopyNW(((NAVMNGR*)hNav)->pStates[index].pszFullName, len, data, len -1))
						{
							((NAVMNGR*)hNav)->statesCount++;
						}
					}
				}
			}
		}
	}

	((NAVMNGR*)hNav)->lastOrderIndex = ((NAVMNGR*)hNav)->statesCount;
	return (count == ((NAVMNGR*)hNav)->statesCount);
}

static BOOL OnNavTree_Click(HNAVCTRL hNav, INT actionId, LRESULT *pResult)
{
	if (hNav && ((NAVMNGR*)hNav)->fnOnItemClick)
	{
		HTREEITEM hTreeItem, hTreeExpand = 0;

		if (ACTION_ENTER_I == actionId)
		{
			hTreeExpand = hTreeItem = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_CARET, (LPARAM)0L);
		}
		else
		{
			hTreeItem = TreeItemFromCursor(hNav, &hTreeExpand);
		}

		if (hTreeExpand || ((ACTION_DBLCLICKL_I == actionId) && hTreeItem && !hTreeExpand))
		{
			if (ACTION_CLICKL_I == actionId || (ACTION_DBLCLICKL_I == actionId && hTreeItem && !hTreeExpand))
			{
				if (ACTION_DBLCLICKL_I == actionId) hTreeExpand = hTreeItem;
				hTreeItem = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_CARET, (LPARAM)0L);
				while (hTreeItem)
				{
					hTreeItem = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_PARENT, (LPARAM)hTreeItem);
					if (hTreeItem == hTreeExpand) break;
				}

				if (hTreeItem == hTreeExpand &&
					(TVIS_EXPANDED == (TVIS_EXPANDED & SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETITEMSTATE, (WPARAM)hTreeExpand, (LPARAM)TVIS_EXPANDED))))
				{
					SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)hTreeExpand);
				}

				SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_EXPAND, (WPARAM)TVE_TOGGLE, (LPARAM)hTreeExpand);
				InvalidateRect(((NAVMNGR*)hNav)->hwndHost, NULL, TRUE);

				if (ACTION_DBLCLICKL_I == actionId)
				{
					HNAVITEM hItem;
					hItem = GetNavItemFromTreeItem(hNav, hTreeExpand);
					if (hItem) ((NAVMNGR*)hNav)->fnOnItemClick(hNav, hItem, actionId);
				}

				*pResult = TRUE; 
				return (NULL == hTreeItem);
			}
			else if (ACTION_DBLCLICKL_I != actionId) hTreeItem = hTreeExpand;
		}

		if (hTreeItem)
		{
			HNAVITEM hItem;
			hItem = GetNavItemFromTreeItem(hNav, hTreeItem);
			if (hItem)
			{
				*pResult = ((NAVMNGR*)hNav)->fnOnItemClick(hNav, hItem, actionId);
				return (BOOL)*pResult;
			}
			return FALSE;
		}
	}
	return FALSE;
}

static BOOL OnTV_DeleteItem(HNAVCTRL hNav, TVITEMW *pItem)
{
	if (pItem->lParam) 
	{
		if (((NAVMNGR*)hNav)->fnOnItemDelete) ((NAVMNGR*)hNav)->fnOnItemDelete(hNav, (HNAVITEM)pItem->lParam);
		NavItemI_SetText((HNAVITEM)pItem->lParam, NULL);
		NavItemI_SetInvariantText((HNAVITEM)pItem->lParam, NULL);
		free((NAVITM*)pItem->lParam);
	}
	return FALSE;
}

static BOOL OnTV_CustomDraw(HNAVCTRL hNav, NMTVCUSTOMDRAW *cd, LRESULT *pResult)
{	
	NAVMNGR *manager;

	manager = (NAVMNGR*)hNav;

	*pResult = CDRF_DODEFAULT;

	switch (cd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			manager->drawStruct.hdc	 = cd->nmcd.hdc;
			manager->drawInternal = 0;
			if ((TVS_FULLROWSELECT & GetWindowLongPtrW(manager->hwndHost, GWL_STYLE))) manager->drawInternal |= 0x0001;
			if (0x8000 & GetAsyncKeyState( GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON)) manager->drawInternal |= 0x0010;
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
			manager->drawStruct.drawStage = NIDS_PREPAINT_I;
			manager->drawStruct.prc	 = &cd->nmcd.rc;
			manager->drawStruct.iLevel = cd->iLevel;

			if (CDIS_SELECTED & cd->nmcd.uItemState)
			{
				manager->drawStruct.itemState = NIS_SELECTED_I;
				
				if (0 != (CDIS_FOCUS & cd->nmcd.uItemState) || 
					0 != (NAVSTYLE_FOCUSED & manager->style))
				{
					manager->drawStruct.itemState |= NIS_FOCUSED_I;
				}
			}
			else
			{
				HTREEITEM hTreeItem;

				manager->drawStruct.itemState = NIS_NORMAL_I;

				if (0x0010 & manager->drawInternal) TreeItemFromCursor(hNav, &hTreeItem);
				else hTreeItem = NULL;

				if (!hTreeItem)
				{
					UINT state;
					state = (UINT)SendMessageW(cd->nmcd.hdr.hwndFrom, TVM_GETITEMSTATE, 
											(WPARAM)cd->nmcd.dwItemSpec, TVIS_DROPHILITED);

					if (TVIS_DROPHILITED & state) 
						manager->drawStruct.itemState = NIS_DROPHILITED_I;
				}
			}

			GetItemColors(hNav, manager->drawStruct.itemState, &cd->clrTextBk, &cd->clrText);

			if (0 == (0x0001 & manager->drawInternal))
			{
				COLORREF rgbBk, rgbFg;
				GetItemColors(hNav, NIS_NORMAL_I, &rgbBk, &rgbFg);
				SetBkColor(cd->nmcd.hdc, rgbBk);
				SetTextColor(cd->nmcd.hdc, rgbFg);
			}
			else
			{
				SetBkColor(cd->nmcd.hdc, cd->clrTextBk);
				SetTextColor(cd->nmcd.hdc, cd->clrText);
			}

			if (cd->nmcd.lItemlParam) 
			{
				NAVITM *pItem;

				pItem = ((NAVITM*)cd->nmcd.lItemlParam);

				if (pItem->hFont) manager->drawStruct.hFont = pItem->hFont;
				else if ((NIS_BOLD_I | NIS_ITALIC_I | NIS_UNDERLINE_I) & pItem->style)
				{
					LOGFONTW lf = { 0 };

					manager->drawStruct.hFont = (HFONT)::SendMessageW(cd->nmcd.hdr.hwndFrom, WM_GETFONT, 0, 0);
					GetObjectW(manager->drawStruct.hFont, sizeof(LOGFONTW), &lf);
					if (NIS_BOLD_I & pItem->style) lf.lfWeight = FW_BOLD;
					if (NIS_ITALIC_I & pItem->style) lf.lfItalic = TRUE;
					if (NIS_UNDERLINE_I & pItem->style) lf.lfUnderline = TRUE;
					manager->drawStruct.hFont = CreateFontIndirectW(&lf);
				}
				else manager->drawStruct.hFont = NULL;

				if (manager->drawStruct.hFont)
				{
					manager->hfontOld = (HFONT)SelectObject(cd->nmcd.hdc, manager->drawStruct.hFont);
					*pResult |= CDRF_NEWFONT;
					if (pItem->hFont != manager->drawStruct.hFont) *pResult |= CDRF_NOTIFYPOSTPAINT;
				}

				if((NIS_CUSTOMDRAW_I & pItem->style) && manager->fnOnCustomDraw)
				{
					INT result;
					manager->drawStruct.clrTextBk = cd->clrTextBk;
					manager->drawStruct.clrText = cd->clrText;
					result = manager->fnOnCustomDraw(hNav, pItem, &manager->drawStruct, pItem->lParam);
					if (NICDRF_SKIPDEFAULT_I & result) 
					{
						SelectObject(cd->nmcd.hdc, manager->hfontOld);
						DeleteObject(manager->drawStruct.hFont);
						manager->drawStruct.hFont = NULL;
						*pResult |= CDRF_SKIPDEFAULT;
					}
					if (NICDRF_NEWFONT_I & result) *pResult |= CDRF_NEWFONT;
					if (NICDRF_NOTIFYPOSTPAINT_I & result)
					{
						pItem->style |= NIS_WANTPOSTPAINT_I;
						*pResult |= CDRF_NOTIFYPOSTPAINT;
					}
					else pItem->style &= ~NIS_WANTPOSTPAINT_I;
				}
			}
			break;

		case CDDS_ITEMPOSTPAINT:

			if(NIS_WANTPOSTPAINT_I & ((NAVITM*)cd->nmcd.lItemlParam)->style)
			{
				INT result;
				manager->drawStruct.drawStage = NIDS_POSTPAINT_I;
				result = manager->fnOnCustomDraw(hNav, (HNAVITEM)cd->nmcd.lItemlParam, &manager->drawStruct, ((NAVITM*)cd->nmcd.lItemlParam)->lParam);
				if (NICDRF_SKIPDEFAULT_I & result) *pResult |= CDRF_SKIPDEFAULT;
			}

			if (manager->drawStruct.hFont && manager->drawStruct.hFont != ((NAVITM*)cd->nmcd.lItemlParam)->hFont)
			{
				SelectObject(cd->nmcd.hdc, manager->hfontOld);
				DeleteObject(manager->drawStruct.hFont);
				manager->drawStruct.hFont = NULL;
			}
			break;
	}
	return TRUE;
}

static BOOL OnTV_GetDispInfo(HNAVCTRL hNav, NMTVDISPINFOW *di, LRESULT *pResult)
{
	BOOL handled;
	NAVMNGR *manager;

	manager = (NAVMNGR*)hNav;

	handled = FALSE;
	if((TVIF_IMAGE | TVIF_SELECTEDIMAGE) & di->item.mask) 
	{
		if (di->item.lParam) 
		{
			INT	 itemState;
			COLORREF rgbBk, rgbFg;

			itemState = NIS_NORMAL_I;
			if (TVS_FULLROWSELECT == (TVS_FULLROWSELECT & (UINT)GetWindowLongPtrW(manager->hwndHost, GWL_STYLE)))
			{
				itemState = di->item.state;
				if (TVIS_DROPHILITED & di->item.state) 
				{				
					if (0x8000 & GetAsyncKeyState( GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON))
					{
						HTREEITEM hTreeItem;
						TreeItemFromCursor(hNav, &hTreeItem);
						itemState = (hTreeItem) ? NIS_NORMAL_I : NIS_DROPHILITED_I;
					}
					else 
						itemState = NIS_DROPHILITED_I;
				}
				else if (TVIS_SELECTED & di->item.state)
				{
					itemState = NIS_SELECTED_I;
					if (0 != ((NAVSTYLE_FOCUSED | NAVSTYLE_EDITMODE) & manager->style)) 
						itemState |= NIS_FOCUSED_I;
				}
			}

			GetItemColors(hNav, itemState, &rgbBk, &rgbFg);
			if (TVIF_IMAGE & di->item.mask)
			{
				di->item.iImage = GetRealImageIndex(hNav, (NAVITM*)di->item.lParam, IMAGE_NORMAL_I, rgbBk, rgbFg);
			}

			if (TVIF_SELECTEDIMAGE & di->item.mask)
			{
				di->item.iSelectedImage = ((TVIF_IMAGE & di->item.mask) && 
											((NAVITM*)(di->item.lParam))->iImage == ((NAVITM*)(di->item.lParam))->iSelectedImage) ?
											di->item.iImage :
											GetRealImageIndex(hNav, (NAVITM*)di->item.lParam, IMAGE_SELECTED_I, rgbBk, rgbFg);											
			}
			handled = TRUE;
			
		}
	}
	if (TVIF_CHILDREN & di->item.mask)
	{
		di->item.cChildren = (di->item.lParam) ? (NIS_HASCHILDREN_I & ((NAVITM*)di->item.lParam)->style) : 0;
		handled = TRUE;
	}
	if (TVIF_TEXT & di->item.mask)
	{
		di->item.pszText = (di->item.lParam) ?
							((NAVITM*)di->item.lParam)->pszText : WASABI_API_LNGSTRINGW(IDS_BAD_ITEM);
		handled = TRUE;
	}
	return handled;
}

static BOOL OnTV_Click(HNAVCTRL hNav, NMHDR *pnmh, LRESULT *pResult)
{
	return OnNavTree_Click(hNav, ACTION_CLICKL_I, pResult);
}

static BOOL OnTV_BeginDrag(HNAVCTRL hNav, NMTREEVIEWW *pnmtv)
{
	if (hNav && ((NAVMNGR*)hNav)->fnOnBeginDrag)
	{
		((NAVMNGR*)hNav)->fnOnBeginDrag(hNav, (HNAVITEM)pnmtv->itemNew.lParam, pnmtv->ptDrag);
	}
	return FALSE;
}

static BOOL OnTV_SelectionChanged(HNAVCTRL hNav, NMTREEVIEWW *pnmtv)
{
	if (hNav && ((NAVMNGR*)hNav)->fnOnItemSelected)
	{
		((NAVMNGR*)hNav)->fnOnItemSelected(hNav, (HNAVITEM)pnmtv->itemOld.lParam, (HNAVITEM)pnmtv->itemNew.lParam);
		MLSkinnedScrollWnd_UpdateBars(((NAVMNGR*)hNav)->hwndHost, TRUE);
	}
	return FALSE;
}

static BOOL OnTV_RightClick(HNAVCTRL hNav, NMHDR *pnmh, LRESULT *pResult)
{
	return OnNavTree_Click(hNav, ACTION_CLICKR_I, pResult);
}

static BOOL OnTV_DoubleClick(HNAVCTRL hNav, NMHDR *pnmh, LRESULT *pResult)
{
	return OnNavTree_Click(hNav, ACTION_DBLCLICKL_I, pResult);
}

static BOOL OnTV_KeyDown(HNAVCTRL hNav, NMTVKEYDOWN *ptvkd, LRESULT *pResult)
{
	if (VK_RETURN == ptvkd->wVKey) return OnNavTree_Click(hNav, ACTION_ENTER_I, pResult);

	if (hNav && ((NAVMNGR*)hNav)->fnOnKeyDown)
	{
		HNAVITEM hItem;
		hItem = NavCtrlI_GetSelection(hNav);

		if (hItem)
		{
			*pResult = ((NAVMNGR*)hNav)->fnOnKeyDown(hNav, hItem, ptvkd);
			return (BOOL)*pResult;
		}
	}
	return FALSE;
}

static BOOL OnTV_BeginLabelEdit(HNAVCTRL hNav, NMTVDISPINFOW *di, LRESULT *pResult)
{
	NAVMNGR *manager;
	NAVITM *item;
	manager = (NAVMNGR*)hNav;

	if (NULL == di)
		return FALSE;

	item = (NAVITM*)di->item.lParam;
	*pResult = TRUE;

	if (NULL != item &&
		0 != (NIS_ALLOWEDIT_I & item->style))
	{
		if (NULL == manager->fnOnBeginTitleEdit ||
			FALSE != manager->fnOnBeginTitleEdit(hNav, (HNAVITEM)item))
		{
			HWND treeWindow, editWindow;

			treeWindow = manager->hwndHost; 

			editWindow = (HWND)SendMessageW(treeWindow, TVM_GETEDITCONTROL, 0, 0);
			if (NULL != editWindow)
				SubclassEditControl(editWindow);

			manager->style |= NAVSTYLE_EDITMODE;
			*pResult = FALSE;
		}
	}

	return TRUE;
}

static BOOL OnTV_EndLabelEdit(HNAVCTRL hNav, NMTVDISPINFOW *di, LRESULT *pResult)
{
	NAVMNGR *manager;
	manager = (NAVMNGR*)hNav;

	manager->style &= ~NAVSTYLE_EDITMODE;

	if (NULL != manager->fnOnEndTitleEdit && 
		FALSE == manager->fnOnEndTitleEdit(hNav, (HNAVITEM)di->item.lParam, di->item.pszText)) 
	{
		*pResult = FALSE;
	}
	else
	{
		*pResult = (NULL != di->item.pszText) ? 
					NavItemI_SetText((HNAVITEM)di->item.lParam, di->item.pszText) : 
					FALSE;
	}

	return TRUE;
}

static BOOL OnTV_SetCursor(HNAVCTRL hNav, NMMOUSE *pm, LRESULT *pResult)
{
	if (!pm->dwItemData) 
	{
		TVHITTESTINFO hit;

		if(GetCursorPos(&hit.pt))
		{
			HTREEITEM hTreeItem;
			MapWindowPoints(HWND_DESKTOP, ((NAVMNGR*)hNav)->hwndHost, &hit.pt, 1);
			hTreeItem = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_HITTEST, (WPARAM)0, (LPARAM)&hit);	
			if (hTreeItem) pm->dwItemData = (DWORD_PTR)GetNavItemFromTreeItem(hNav, hTreeItem);
		}
	}

	if (pm->dwItemData) 
	{
		if ((NIS_WANTSETCURSOR_I & ((NAVITM*)pm->dwItemData)->style) && ((NAVMNGR*)hNav)->fnOnSetCursor)
		{
			*pResult = ((NAVMNGR*)hNav)->fnOnSetCursor(hNav, (HNAVITEM)pm->dwItemData, ((NAVITM*)pm->dwItemData)->lParam);
		}
	}

	return TRUE;
}

static BOOL OnTV_SetFocus(HNAVCTRL hNav,  NMHDR *pnmh)
{
	NAVMNGR *manager = (NAVMNGR*)hNav;

	if (NULL == manager)
		return FALSE;

	if (0 == (NAVSTYLE_FOCUSED & manager->style))
	{
		manager->style |= NAVSTYLE_FOCUSED;

		HNAVITEM selectedItem = NavCtrlI_GetSelection(hNav);
		if (NULL != selectedItem)
			NavItemI_Invalidate(selectedItem, NULL, FALSE);
	}

	return TRUE;
}

static BOOL OnTV_KillFocus(HNAVCTRL hNav,  NMHDR *pnmh)
{
	NAVMNGR *manager = (NAVMNGR*)hNav;

	if (NULL == manager)
		return FALSE;

	if (0 != (NAVSTYLE_FOCUSED & manager->style))
	{
		manager->style &= ~NAVSTYLE_FOCUSED;

		HNAVITEM selectedItem = NavCtrlI_GetSelection(hNav);
		if (NULL != selectedItem)
			NavItemI_Invalidate(selectedItem, NULL, FALSE);
	}

	return TRUE;
}

// manager
HNAVCTRL NavCtrlI_Create(HWND hwndParent)
{
	PNAVMNGR pMngr = (PNAVMNGR)calloc(1, sizeof(NAVMNGR));
	if (!pMngr) return NULL;

	pMngr->hwndHost	= CreateWindowExW(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, 
											WC_TREEVIEWW, L"",
											WS_CHILD | WS_TABSTOP | TVS_SHOWSELALWAYS | TVS_FULLROWSELECT | TVS_NOHSCROLL | TVS_EDITLABELS,
											0, 0, 1, 1, hwndParent, NULL, NULL, NULL);

	if (!IsWindow(pMngr->hwndHost) || !SetPropW(pMngr->hwndHost, NAVMGR_HWNDPROPW, pMngr))  
	{
		NavCtrlI_Destroy(pMngr);
		return NULL;
	}

	pMngr->lastUsedId = NAVITEM_RANGE_MIN;
	pMngr->style = NAVSTYLE_DEFAULT;

	SubclassTreeView(pMngr->hwndHost); // need to be before Skinning

	if (SkinWindowEx(pMngr->hwndHost, SKINNEDWND_TYPE_SCROLLWND, SWS_USESKINFONT | SWS_USESKINCOLORS))
	{
		MLSkinnedScrollWnd_SetMode(pMngr->hwndHost, SCROLLMODE_TREEVIEW);
		HWND hTooltip = (HWND)SendMessageW(pMngr->hwndHost, TVM_GETTOOLTIPS, 0, 0L);
		if (NULL != hTooltip)
		{
			SkinWindowEx(hTooltip, SKINNEDWND_TYPE_TOOLTIP, SWS_USESKINFONT | SWS_USESKINCOLORS);
		}
	}
	SendMessageW(pMngr->hwndHost, (TV_FIRST + 44)/*TVM_SETEXTENDEDSTYLE*/, 
						(WPARAM)pMngr->hwndHost, (LPARAM)0x0004/*TVS_EX_DOUBLEBUFFER*/);

	SendMessageW(pMngr->hwndHost, CCM_SETVERSION, 6, 0);
	SendMessageW(pMngr->hwndHost, TVM_SETSCROLLTIME, 200, 0);

	return (HNAVCTRL)pMngr;
}

BOOL NavCtrlI_SetRect(HNAVCTRL hNav, RECT *prc)
{
	return (hNav && prc) ? SetWindowPos(((NAVMNGR*)hNav)->hwndHost, NULL, 
										prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
										SWP_NOACTIVATE | SWP_NOZORDER) : FALSE;
}

BOOL NavCtrlI_Show(HNAVCTRL hNav, INT nCmdShow)
{
	return (hNav) ? ShowWindow(((NAVMNGR*)hNav)->hwndHost, nCmdShow) : FALSE;
}

BOOL NavCtrlI_Enable(HNAVCTRL hNav, BOOL fEnable)
{
	return (hNav) ? EnableWindow(((NAVMNGR*)hNav)->hwndHost, fEnable) : FALSE;
}

BOOL NavCtrlI_Destroy(HNAVCTRL hNav)
{
	if (!hNav) return FALSE;

	if (NULL != ((NAVMNGR*)hNav)->fnOnDestroy)
	{
		((NAVMNGR*)hNav)->fnOnDestroy(hNav);
	}

	NavCtrlI_SaveStates(hNav);

	if (((NAVMNGR*)hNav)->hwndHost)
	{
		RemovePropW(((NAVMNGR*)hNav)->hwndHost, NAVMGR_HWNDPROPW);
		DestroyWindow(((NAVMNGR*)hNav)->hwndHost);
	}

	NavCtrlI_DeleteStates(hNav);

	free(hNav);
	return TRUE;
}

BOOL NavCtrlI_Update(HNAVCTRL hNav)
{
	return (hNav) ? UpdateWindow(((NAVMNGR*)hNav)->hwndHost) : FALSE;
}

C_Config *NavCtrlI_SetConfig(HNAVCTRL hNav, C_Config *pConfig)
{
	C_Config *pTemp;
	if (!hNav) return NULL;
	pTemp = ((NAVMNGR*)hNav)->pConfig;
	((NAVMNGR*)hNav)->pConfig = pConfig;

	if (pConfig) NavCtrlI_LoadStates(hNav);

	return pTemp;
}

HWND NavCtrlI_GetHWND(HNAVCTRL hNav)
{
	return (hNav) ? ((NAVMNGR*)hNav)->hwndHost : NULL;
}

BOOL NavCtrlI_ProcessNotifications(HNAVCTRL hNav, LPNMHDR pnmh, LRESULT *pResult)
{
	if (!hNav || pnmh->hwndFrom != ((PNAVMNGR)hNav)->hwndHost) return FALSE;
	switch(pnmh->code)
	{
		case TVN_KEYDOWN:		return OnTV_KeyDown(hNav, (NMTVKEYDOWN*)pnmh, pResult);
		case TVN_SELCHANGEDW:	return OnTV_SelectionChanged(hNav, (NMTREEVIEWW*)pnmh);
		case NM_RCLICK:			return OnTV_RightClick(hNav, pnmh, pResult);
		case NM_DBLCLK:			return OnTV_DoubleClick(hNav, pnmh, pResult);
		case TVN_BEGINDRAGW:		return OnTV_BeginDrag(hNav, (NMTREEVIEWW*)pnmh);
		case NM_CLICK:			return OnTV_Click(hNav, pnmh, pResult);
		case TVN_GETDISPINFOW:	return OnTV_GetDispInfo(hNav, (NMTVDISPINFOW*)pnmh, pResult);
		case NM_CUSTOMDRAW:		return OnTV_CustomDraw(hNav, (NMTVCUSTOMDRAW*)pnmh, pResult); 
		case TVN_DELETEITEMW:	return OnTV_DeleteItem(hNav, &((NMTREEVIEWW*)pnmh)->itemOld);
		case TVN_BEGINLABELEDITW: return OnTV_BeginLabelEdit(hNav, (NMTVDISPINFOW*)pnmh, pResult);
		case TVN_ENDLABELEDITW:	return OnTV_EndLabelEdit(hNav, (NMTVDISPINFOW*)pnmh, pResult);
		case NM_SETCURSOR:		return OnTV_SetCursor(hNav, (NMMOUSE*)pnmh, pResult);
		case NM_SETFOCUS:		return OnTV_SetFocus(hNav, pnmh);
		case NM_KILLFOCUS:		return OnTV_KillFocus(hNav, pnmh);
	}
	return FALSE;
}

LPVOID NavCtrlI_RegisterCallback(HNAVCTRL hNav, LPVOID fnCallback, INT cbType)
{
	LPVOID temp;
	if (!hNav) return NULL;
	switch(cbType)
	{
		case CALLBACK_ONCLICK_I: 
			temp = ((NAVMNGR*)hNav)->fnOnItemClick;
			((NAVMNGR*)hNav)->fnOnItemClick = (ONNAVITEMCLICK_I)fnCallback; 
			return temp;  
		case CALLBACK_ONSELECTED_I:
			temp = ((NAVMNGR*)hNav)->fnOnItemSelected;
			((NAVMNGR*)hNav)->fnOnItemSelected = (ONNAVITEMSELECTED_I)fnCallback; 
			return temp;  
		case CALLBACK_ONKEYDOWN_I:
			temp = ((NAVMNGR*)hNav)->fnOnKeyDown;
			((NAVMNGR*)hNav)->fnOnKeyDown = (ONNAVCTRLKEYDOWN_I)fnCallback; 
			return temp; 
		case CALLBACK_ONBEGINDRAG_I:
			temp = ((NAVMNGR*)hNav)->fnOnBeginDrag;
			((NAVMNGR*)hNav)->fnOnBeginDrag = (ONNAVCTRLBEGINDRAG_I)fnCallback; 
			return temp; 
		case CALLBACK_ONDESTROY_I:
			temp = ((NAVMNGR*)hNav)->fnOnDestroy;
			((NAVMNGR*)hNav)->fnOnDestroy = (ONNAVCTRLDESTROY_I)fnCallback; 
			return temp; 
		case CALLBACK_ONGETIMAGEINDEX_I:
			temp = ((NAVMNGR*)hNav)->fnOnItemGetImageIndex;
			((NAVMNGR*)hNav)->fnOnItemGetImageIndex = (ONNAVITEMGETIMAGEINDEX_I)fnCallback; 
			return temp; 
		case CALLBACK_ONBEGINTITLEEDIT_I:
			temp = ((NAVMNGR*)hNav)->fnOnBeginTitleEdit;
			((NAVMNGR*)hNav)->fnOnBeginTitleEdit = (ONNAVITEMBEGINTITLEEDIT_I)fnCallback; 
			return temp; 
		case CALLBACK_ONENDTITLEEDIT_I:
			temp = ((NAVMNGR*)hNav)->fnOnEndTitleEdit;
			((NAVMNGR*)hNav)->fnOnEndTitleEdit = (ONNAVCTRLENDTITLEEDIT_I)fnCallback; 
			return temp; 
		case CALLBACK_ONITEMDELETE_I:
			temp = ((NAVMNGR*)hNav)->fnOnItemDelete;
			((NAVMNGR*)hNav)->fnOnItemDelete = (ONNAVITEMDELETE_I)fnCallback; 
			return temp; 
		case CALLBACK_ONITEMDRAW_I:
			temp = ((NAVMNGR*)hNav)->fnOnCustomDraw;
			((NAVMNGR*)hNav)->fnOnCustomDraw = (ONNAVITEMDRAW_I)fnCallback; 
			return temp;
		case CALLBACK_ONSETCURSOR_I:
			temp = ((NAVMNGR*)hNav)->fnOnSetCursor;
			((NAVMNGR*)hNav)->fnOnSetCursor = (ONNAVITEMSETCURSOR_I)fnCallback; 
			return temp;
		case CALLBACK_ONHITTEST_I:
			temp = ((NAVMNGR*)hNav)->fnOnHitTest;
			((NAVMNGR*)hNav)->fnOnHitTest = (ONNAVITEMHITTEST_I)fnCallback; 
			return temp;
	}

	return NULL;

}

BOOL NavCtrlI_UpdateLook(HNAVCTRL hNav)
{
	INT minHeight;

	if (!hNav || !((NAVMNGR*)hNav)->hwndHost) return FALSE;

	minHeight = -1;

	SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SETBKCOLOR, (WPARAM)0, (LPARAM)WADlg_getColor(WADLG_ITEMBG));
	SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SETTEXTCOLOR, (WPARAM)0, (LPARAM)WADlg_getColor(WADLG_ITEMFG));
	SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SETINSERTMARKCOLOR, (WPARAM)0, (LPARAM)WADlg_getColor(WADLG_ITEMFG));

	MLSkinnedWnd_SkinChanged(((NAVMNGR*)hNav)->hwndHost, FALSE, FALSE);

	HFONT hFont = (HFONT)SendMessageW(((NAVMNGR*)hNav)->hwndHost, WM_GETFONT, 0, 0L);

	HDC hdc;
	hdc = GetDCEx(((NAVMNGR*)hNav)->hwndHost, NULL, DCX_CACHE);
	if (hdc)
	{
		if (NULL == hFont)
			hFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
		TEXTMETRICW tm;
		if (hFont)
		{
 			HFONT hfntOld = (HFONT)SelectObject(hdc, hFont);

			if (GetTextMetricsW(hdc, &tm)) minHeight = tm.tmHeight + 2/*Borders*/;
			SelectObject(hdc, hfntOld);
		}
		ReleaseDC(((NAVMNGR*)hNav)->hwndHost, hdc);
	}

	if (((NAVMNGR*)hNav)->pConfig)
	{
		BOOL fFullRowSelect;
		DWORD dwWndStyle;
		INT height;

		height = ((NAVMNGR*)hNav)->pConfig->ReadInt(L"Navigation_ItemHeight", 18);
		if (minHeight > 0 && height < minHeight) height = minHeight;

		SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SETITEMHEIGHT, (WPARAM)height, (LPARAM)0);
		((NAVMNGR*)hNav)->style = (((NAVMNGR*)hNav)->style & ~NAVSTYLE_MOUSEDOWNSELECT) | ((0 != ((NAVMNGR*)hNav)->pConfig->ReadInt(L"Navigation_MouseDownSel", 0)) ? NAVSTYLE_MOUSEDOWNSELECT : 0);
		fFullRowSelect = (((NAVMNGR*)hNav)->pConfig->ReadInt(L"Navigation_FullRowSel", 1) != 0);
		dwWndStyle = (DWORD)GetWindowLongPtrW(((NAVMNGR*)hNav)->hwndHost, GWL_STYLE);
		if (fFullRowSelect != (TVS_FULLROWSELECT == (TVS_FULLROWSELECT & dwWndStyle)))
		{
			SetWindowLongPtrW(((NAVMNGR*)hNav)->hwndHost, GWL_STYLE,
					(dwWndStyle & ~TVS_FULLROWSELECT) | ((fFullRowSelect) ? TVS_FULLROWSELECT : 0));
		}
		NavCtrlI_SetImageList(hNav, ((NAVMNGR*)hNav)->hmlilImages);
	}

	return TRUE;
}

HMLIMGLST NavCtrlI_SetImageList(HNAVCTRL hNav, HMLIMGLST hmlil)
{
	HMLIMGLST	hmlilOld;
	HIMAGELIST hilTree, hilNew;

	if (!hNav) return NULL;

	hmlilOld = ((NAVMNGR*)hNav)->hmlilImages;
	((NAVMNGR*)hNav)->hmlilImages = hmlil;

	hilNew = MLImageListI_GetRealList(hmlil);
	hilTree = (HIMAGELIST)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETIMAGELIST, (WPARAM)TVSIL_NORMAL, (LPARAM)0);

	if (!((NAVMNGR*)hNav)->pConfig || ((NAVMNGR*)hNav)->pConfig->ReadInt(L"Navigation_ShowIcons", 1))
	{
		if (hilTree != hilNew) SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SETIMAGELIST, (WPARAM)TVSIL_NORMAL, (LPARAM)hilNew);
	}
	else if (hilTree) SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SETIMAGELIST, (WPARAM)TVSIL_NORMAL, (LPARAM)NULL);
	return hmlilOld;
}

HMLIMGLST NavCtrlI_GetImageList(HNAVCTRL hNav)
{
	return (hNav) ? ((NAVMNGR*)hNav)->hmlilImages : NULL;
}

HNAVITEM NavCtrlI_FindItem(HNAVCTRL hNav, INT itemId)
{
	return (hNav) ? FindNavItemByNavIdEx(((NAVMNGR*)hNav)->hwndHost, itemId, TVI_ROOT) : NULL;
}

HNAVITEM NavCtrlI_FindItemByName(HNAVCTRL hNav, LCID Locale, UINT compFlags, LPCWSTR pszName, INT cchLength)
{
	TREEITEMSEARCH search;

	if (!hNav || !pszName || !((NAVMNGR*)hNav)->hwndHost) return NULL;

	search.fFound	= FALSE;
	search.itemData	= (INT_PTR)pszName;
	search.item.mask	= TVIF_PARAM;
	search.flags	= compFlags;
	// pack extra info
	search.item.cchTextMax	= cchLength;
	search.item.state		= Locale;

	EnumerateTreeItems(((NAVMNGR*)hNav)->hwndHost, TVI_ROOT, FindTreeItemByNameCB, &search);
	return (search.fFound) ? (HNAVITEM)search.item.lParam : NULL;
}

HNAVITEM NavCtrlI_FindItemByFullName(HNAVCTRL hNav, LCID Locale, UINT compFlags, LPCWSTR pszFullName, INT cchLength, BOOL fAncestorOk)
{
	INT len, separatorCount;
	WCHAR shortname[ITEM_SHORTNAME_MAX] = {0};
	LPCWSTR end;
	TVITEMW treeItem;
	HNAVITEM hNavParent;

	if (!hNav || !pszFullName || !((NAVMNGR*)hNav)->hwndHost) return NULL;

	hNavParent = NULL;

	len = (cchLength < 0)? lstrlenW(pszFullName) : cchLength;
	if (!len) return NULL;
	end = pszFullName + len;

	ZeroMemory(&treeItem, sizeof(TVITEMW));
	treeItem.mask = TVIF_PARAM;

	len = 0;
	separatorCount = 0;

	while (pszFullName != end + 1)
	{
		if (pszFullName == end) { separatorCount++; }
		
		if (SEPARATOR == *pszFullName) separatorCount++;
		else 
		{
			if (separatorCount)
			{
				INT i;
				for (i = separatorCount/2; i > 0; i--) 
				{
					if (len == ITEM_SHORTNAME_MAX -1) return NULL; // ugh...
					shortname[len++] = SEPARATOR;
				}
				
				if (separatorCount%2 && len)
				{
					if (CSTR_EQUAL != CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, EMPTY_ITEM_NAME, -1, shortname, len))
					{ 
						treeItem.hItem = (HTREEITEM)((NULL == hNavParent) ? 
													SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_ROOT, (LPARAM)0) :
													SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)((NAVITM*)hNavParent)->hTreeItem));
						while(treeItem.hItem)
						{
							if (SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETITEMW, (WPARAM)0, (LPARAM)&treeItem))
							{
								if (CSTR_EQUAL == CompareItemName(Locale, compFlags, shortname, len, (HNAVITEM)treeItem.lParam))
								{ // found!!!
									if (pszFullName == end) return (HNAVITEM)treeItem.lParam;
									break;
								}
							}
							treeItem.hItem = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)treeItem.hItem);
						}
						if (treeItem.hItem) 
						{
							hNavParent = (HNAVITEM)treeItem.lParam;
							if (!hNavParent) return NULL; // thats bad
						}
						else return (fAncestorOk) ? hNavParent : NULL;
					}
					len = 0;
				}
				separatorCount = 0;
			}
			if (len == ITEM_SHORTNAME_MAX -1) return NULL; // ugh...
			shortname[len++] = *pszFullName;
		}
		pszFullName++;
	}
	
	return NULL;
}

HNAVITEM NavCtrlI_InsertItem(HNAVCTRL hNav, HNAVITEM hInsertAfter, HNAVITEM hParent, NAVITEM_I *pnis)
{
	TVINSERTSTRUCTW is = {0};
	NAVITM *pNavItem = 0;
	HTREEITEM hItem = 0;

	if (!pnis) return NULL;

	if (NIMF_ITEMID_I & pnis->mask) { if (NavCtrlI_FindItem(hNav, pnis->id)) return NULL; }
	else pnis->id = GetNextFreeItemId(hNav);

	if (!pnis->id) return NULL;

	pNavItem = (NAVITM*)calloc(1, sizeof(NAVITM));
	if (!pNavItem) return NULL;

	is.hParent = (hParent) ? ((NAVITM*)hParent)->hTreeItem : TVI_ROOT;
	is.hInsertAfter = TVI_LAST;

	if (NCI_LAST_I == hInsertAfter) is.hInsertAfter = TVI_LAST;
	else if (NCI_FIRST_I == hInsertAfter) is.hInsertAfter = TVI_FIRST;
	else if (hInsertAfter && !IS_NAVITEMSORTORDER(hInsertAfter)) is.hInsertAfter = ((NAVITM*)hInsertAfter)->hTreeItem;

	if (is.hInsertAfter == is.hParent) is.hInsertAfter = TVI_FIRST;

	pNavItem->id			= pnis->id;
	pNavItem->hwndTree		= ((NAVMNGR*)hNav)->hwndHost;

	is.item.mask			= TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_TEXT;
	is.item.lParam			= (LPARAM)pNavItem;
	is.item.pszText			= LPSTR_TEXTCALLBACKW;
	is.item.iImage			= I_IMAGECALLBACK;
	is.item.iSelectedImage	= I_IMAGECALLBACK;
	is.item.cChildren		= I_CHILDRENCALLBACK;

	if (NIMF_STYLE_I & pnis->mask)		NavItemI_SetStyle(pNavItem, pnis->style, pnis->styleMask);
	if (NIMF_TEXT_I & pnis->mask)		NavItemI_SetText(pNavItem, pnis->pszText);
	if (NIMF_TEXTINVARIANT_I & pnis->mask)		NavItemI_SetInvariantText(pNavItem, pnis->pszInvariant);
	if (NIMF_FONT_I & pnis->mask)		NavItemI_SetFont(pNavItem, pnis->hFont);
	if (NIMF_PARAM_I & pnis->mask)		pNavItem->lParam = pnis->lParam;

	NavItemI_SetImageIndex(pNavItem, (NIMF_IMAGE_I & pnis->mask) ? pnis->iImage : -1, IMAGE_NORMAL_I);
	NavItemI_SetImageIndex(pNavItem, (NIMF_IMAGESEL_I & pnis->mask) ? pnis->iSelectedImage : -1, IMAGE_SELECTED_I);

	NavCtrlI_BeginUpdate(hNav, NUF_LOCK_SELECTED_I);

	hItem = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_INSERTITEMW, (WPARAM)0, (LPARAM)&is);

	if (hItem)
	{
		UINT state, stateMask;
		NAVITEMSTATEREC *pRec;
		BOOL itemRecSearched;
		WORD order;
		UINT flags;

		pNavItem->hTreeItem = hItem;
		itemRecSearched = FALSE;
		order = (WORD)-1;
		flags = NOF_MOVEAFTER_I;

		if (hInsertAfter) 
		{
			if (IS_NAVITEMSORTORDER(hInsertAfter)) { order = (WORD)hInsertAfter; flags = NOF_MOVEONEAFTER_I; }
			else 
			{
				HTREEITEM hTreePrev;
				hTreePrev = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_PREVIOUS, (LPARAM)hItem);
				if (!hTreePrev) { order = 1; flags = NOF_MOVEONEBEFORE_I; }
				else
				{
					HNAVITEM hPrev;
					hPrev = GetNavItemFromTreeItem(hNav, hTreePrev);
					order = (hPrev) ? ((NAVITM*)hPrev)->sortOrder : (WORD)-1;
					flags = NOF_MOVEONEAFTER_I;
				}
			}
		}
		else
		{
			if (TVI_ROOT == is.hParent || !is.hParent) 
			{ 
				pRec = NavCtrlI_FindItemStateRec(hNav, pNavItem, TRUE); 
				order = (pRec) ? pRec->nOrder : ++((NAVMNGR*)hNav)->lastOrderIndex;
				flags = NOF_MOVEAFTER_I;
				itemRecSearched = TRUE; 
			}
		}
		NavItemI_SetOrder(pNavItem, order, flags);

		if (NIMF_STATE_I & pnis->mask) 
		{ 
			state = pnis->state;  
			stateMask = pnis->stateMask; 
		}
		else 
		{ 
			state = 0; 
			stateMask = 0; 
		}

		if (0 == (NIS_EXPANDED_I & stateMask))
		{
			if (!itemRecSearched) pRec = NavCtrlI_FindItemStateRec(hNav, pNavItem, TRUE);
			state |= (pRec && pRec->fCollapsed) ? 0 : NIS_EXPANDED_I; 
			stateMask |= NIS_EXPANDED_I;
		}
		NavItemI_SetState(pNavItem, state, stateMask);
	}
	else
	{
		NavItemI_SetText(pNavItem, NULL);
		NavItemI_SetInvariantText(pNavItem, NULL);
		free(pNavItem);
		pNavItem = NULL;
	}
	NavCtrlI_EndUpdate(hNav);

	return (HNAVITEM)pNavItem;
}

BOOL NavCtrlI_DeleteItem(HNAVCTRL hNav, HNAVITEM hItem)
{
	return (hNav && hItem) ? (BOOL)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_DELETEITEM, 0, (LPARAM)((NAVITM*)hItem)->hTreeItem) : FALSE;
}

BOOL NavCtrlI_DeleteAll(HNAVCTRL hNav)
{
	return (hNav) ? (BOOL)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT) : FALSE;
}

HNAVITEM NavCtrlI_GetRoot(HNAVCTRL hNav)
{
	HTREEITEM hTreeChild;
	if (!hNav) return NULL;
	hTreeChild = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_ROOT, (LPARAM)0L);
	return (hTreeChild) ? GetNavItemFromTreeItem(hNav, hTreeChild) : NULL;
}

HNAVITEM NavCtrlI_GetSelection(HNAVCTRL hNav)
{
	HTREEITEM hTreeChild;
	if (!hNav) return NULL;
	hTreeChild = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_CARET, (LPARAM)0L);
	return (hTreeChild) ? GetNavItemFromTreeItem(hNav, hTreeChild) : NULL;
}

HNAVITEM NavCtrlI_GetFirstVisible(HNAVCTRL hNav)
{
	HTREEITEM hTreeChild;
	if (!hNav) return NULL;
	hTreeChild = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_FIRSTVISIBLE, (LPARAM)0L);
	return (hTreeChild) ? GetNavItemFromTreeItem(hNav, hTreeChild) : NULL;
}

HNAVITEM NavCtrlI_GetLastVisible(HNAVCTRL hNav)
{
	HTREEITEM hTreeChild;
	if (!hNav) return NULL;
	hTreeChild = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, (WPARAM)TVGN_LASTVISIBLE, (LPARAM)0L);
	return (hTreeChild) ? GetNavItemFromTreeItem(hNav, hTreeChild) : NULL;
}

HNAVITEM NavCtrlI_HitTest(HNAVCTRL hNav, POINT *ppt, UINT *pFlags)
{
	TVHITTESTINFO tvhi;
	HNAVITEM hItem;

	if (!hNav || !((NAVMNGR*)hNav)->hwndHost || !ppt)
	{
		if (pFlags) *pFlags = NAVHT_NOWHERE_I;
		return NULL;
	}
	tvhi.pt = *ppt;
	SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_HITTEST, (WPARAM)0, (LPARAM)&tvhi);

	hItem = (tvhi.hItem) ? GetNavItemFromTreeItem(hNav, tvhi.hItem) : NULL;

	PerformCustomHitTest(hNav, tvhi.pt, &tvhi.flags, (hItem) ? &hItem : NULL);
	if (pFlags) *pFlags = tvhi.flags;

	return hItem;
}

BOOL NavCtrlI_SetInsertMark(HNAVCTRL hNav, HNAVITEM hItem, BOOL fAfter)
{
	return (hNav) ? (BOOL)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SETINSERTMARK,
										(WPARAM)fAfter, 
										(hItem) ? (LPARAM)((NAVITM*)hItem)->hTreeItem : NULL) : FALSE;
}

BOOL NavCtrlI_EnumItems(HNAVCTRL hNav, NAVENUMPROC_I pEnumFunc, HNAVITEM hItemStart, LPARAM lParam)
{
	NAVENUMSTRUCT navenum;
	if (!hNav || !pEnumFunc) return FALSE;

	navenum.hNav		= hNav;
	navenum.callback	= pEnumFunc;
	navenum.lParam		= lParam;
	navenum.item.mask	= TVIF_PARAM;

	return EnumerateTreeItems(((NAVMNGR*)hNav)->hwndHost, (hItemStart) ? ((NAVITM*)hItemStart)->hTreeItem : TVI_ROOT, EnumNavItemCB, &navenum);
}

INT NavCtrlI_BeginUpdate(HNAVCTRL hNav, UINT fRememberPos)
{
	if (!hNav || !((NAVMNGR*)hNav)->hwndHost || !IsWindow(((NAVMNGR*)hNav)->hwndHost)) return -1;
	if (!((NAVMNGR*)hNav)->lockUpdate) 
	{
		((NAVMNGR*)hNav)->lockFirst = NULL;
		((NAVMNGR*)hNav)->lockSelected = NULL;
		if (fRememberPos)
		{
			((NAVMNGR*)hNav)->lockFirst = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, (LPARAM)TVI_ROOT);			
			if (NUF_LOCK_SELECTED_I & fRememberPos)
			{
				((NAVMNGR*)hNav)->lockSelected = (HTREEITEM)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)TVI_ROOT);
				if (((NAVMNGR*)hNav)->lockSelected)
				{
					RECT rc;
					*(HTREEITEM*)&rc = ((NAVMNGR*)hNav)->lockSelected;
					if (!SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETITEMRECT, FALSE, (LPARAM)&rc)) ((NAVMNGR*)hNav)->lockSelected = NULL;
					else ((NAVMNGR*)hNav)->lockSelPos = rc.top;
				}
			}
		}
		UpdateWindow(((NAVMNGR*)hNav)->hwndHost);
		SendMessageW(((NAVMNGR*)hNav)->hwndHost, WM_SETREDRAW, (WPARAM)FALSE, 0L);
	}
	return ++((NAVMNGR*)hNav)->lockUpdate;
}

INT NavCtrlI_EndUpdate(HNAVCTRL hNav)
{
	if (!hNav || !((NAVMNGR*)hNav)->hwndHost || !IsWindow(((NAVMNGR*)hNav)->hwndHost)) return -1;
	if (((NAVMNGR*)hNav)->lockUpdate)
	{
		((NAVMNGR*)hNav)->lockUpdate--;
		if (!((NAVMNGR*)hNav)->lockUpdate)
		{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS | SIF_RANGE;

			if (((NAVMNGR*)hNav)->lockSelected)
			{
				RECT rc;
				*(HTREEITEM*)&rc = ((NAVMNGR*)hNav)->lockSelected;
				if (!SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETITEMRECT, FALSE, (LPARAM)&rc)) 
				{
					SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_SELECTITEM, (WPARAM)TVGN_FIRSTVISIBLE, (LPARAM)((NAVMNGR*)hNav)->lockSelected);
				}
				else if (((NAVMNGR*)hNav)->lockSelPos != rc.top)
				{
					INT iHeight = (INT)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETITEMHEIGHT , 0, 0L);
					if (iHeight)
					{
						INT pos, oldPos;
						if (((NAVMNGR*)hNav)->lockSelPos > rc.top) iHeight = 0 - iHeight;
						WPARAM wCmd = MAKEWPARAM((((NAVMNGR*)hNav)->lockSelPos > rc.top) ? SB_LINEUP : SB_LINEDOWN, 0);
						oldPos = 0xFFFFFF;
						for(pos = ((NAVMNGR*)hNav)->lockSelPos; pos != rc.top; pos += iHeight)
						{
							if (ABS((oldPos - rc.top)) <= ABS((pos - rc.top))) break;
							oldPos = pos;
							SendMessageW(((NAVMNGR*)hNav)->hwndHost, WM_VSCROLL, wCmd, 0L);
							SendMessageW(((NAVMNGR*)hNav)->hwndHost, WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), 0L);
						}
					}
				}
			}
			else if (((NAVMNGR*)hNav)->lockFirst)
			{
				SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_ENSUREVISIBLE, (WPARAM)0, (LPARAM)((NAVMNGR*)hNav)->lockFirst);
			}

			((NAVMNGR*)hNav)->lockFirst = NULL;			
			((NAVMNGR*)hNav)->lockSelected = NULL;		

			if(GetScrollInfo(((NAVMNGR*)hNav)->hwndHost, SB_HORZ, &si) && (si.nMin != si.nPos))
			{
				SendMessageW(((NAVMNGR*)hNav)->hwndHost, WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), NULL);
				SendMessageW(((NAVMNGR*)hNav)->hwndHost, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
			}
			SendMessageW(((NAVMNGR*)hNav)->hwndHost, WM_SETREDRAW, (WPARAM)TRUE, 0L);
		}
	}
	return ((NAVMNGR*)hNav)->lockUpdate;
}

INT NavCtrlI_MapPointsTo(HNAVCTRL hNav, HWND hwndTo, POINT *ppt, UINT cPoints)
{
	return (hNav) ? MapWindowPoints(((NAVMNGR*)hNav)->hwndHost, hwndTo, ppt, cPoints) : 0;
}

INT NavCtrlI_MapPointsFrom(HNAVCTRL hNav, HWND hwndFrom, POINT *ppt, UINT cPoints)
{
	return (hNav) ? MapWindowPoints(hwndFrom, ((NAVMNGR*)hNav)->hwndHost, ppt, cPoints) : 0;
}

BOOL NavCtrlI_EndEditTitle(HNAVCTRL hNav, BOOL fCancel)
{
	return (hNav) ? (BOOL)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_ENDEDITLABELNOW, (WPARAM)fCancel, 0L) : FALSE;
}

INT NavCtrlI_GetIndent(HNAVCTRL hNav)
{
	return (hNav) ? (BOOL)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETINDENT, (WPARAM)0, 0L) : 0;
}

DWORD NavCtrlI_GetStyle(HNAVCTRL hNav)
{
	DWORD style;
	style = NCS_NORMAL_I;
	if (hNav) 
	{
		if (TVS_FULLROWSELECT == (TVS_FULLROWSELECT & (DWORD)GetWindowLongPtrW(((NAVMNGR*)hNav)->hwndHost, GWL_STYLE)))
				style |= NCS_FULLROWSELECT_I;
		if (NULL != (HIMAGELIST)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETIMAGELIST, (WPARAM)TVSIL_NORMAL, (LPARAM)0))
				style |= NCS_SHOWICONS_I;
	}
	return style;
}

BOOL NavItemI_EditTitle(HNAVITEM hItem)
{
	NAVITM *item;
	HWND editWindow;

	if (NULL == hItem)
		return FALSE;

	item = (NAVITM*)hItem;
	if (0 == (NIS_ALLOWEDIT_I & item->style))
		return FALSE;

	SendMessageW(item->hwndTree, TVM_ENSUREVISIBLE, 0, (LPARAM)item->hTreeItem);

	editWindow = (HWND)SendMessageW(item->hwndTree, TVM_EDITLABELW, 0, (LPARAM)item->hTreeItem);
	return (NULL != editWindow);
}

INT NavItemI_GetId(HNAVITEM hItem)
{
	return (hItem) ? ((NAVITM*)hItem)->id : 0;
}

HNAVITEM NavItemI_GetChild(HNAVITEM hItem)
{
	return NavItemI_GetNextEx(hItem, TVGN_CHILD);
}

HNAVITEM NavItemI_GetNext(HNAVITEM hItem)
{
	return NavItemI_GetNextEx(hItem, TVGN_NEXT);
}

HNAVITEM NavItemI_GetRoot(HNAVITEM hItem)
{
	return NavItemI_GetNextEx(hItem, TVGN_ROOT);
}

HNAVITEM NavItemI_GetParent(HNAVITEM hItem)
{
	return NavItemI_GetNextEx(hItem, TVGN_PARENT);
}

HNAVITEM NavItemI_GetPrevious(HNAVITEM hItem)
{
	return NavItemI_GetNextEx(hItem, TVGN_PREVIOUS);
}

INT NavItemI_GetChildrenCount(HNAVITEM hItem)
{
	HTREEITEM hChild;
	int counter;

	if (!hItem) return -1;

	hChild = (HTREEITEM)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)((NAVITM*)hItem)->hTreeItem);
    if (!hChild) return 0;
	counter = 1;
	while(NULL != (hChild = (HTREEITEM)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)hChild))) counter++;
	return counter;
}

BOOL NavItemI_IsExpanded(HNAVITEM hItem)
{
	return (hItem && (TVIS_EXPANDED == (TVIS_EXPANDED & (UINT)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETITEMSTATE,
										(WPARAM)((NAVITM*)hItem)->hTreeItem, (LPARAM)TVIS_EXPANDED))));
}

INT NavItemI_GetImageIndex(HNAVITEM hItem, INT imageType)
{
	NAVITM *item;
	item = (NAVITM*)hItem;

	if (NULL == item)
		return -1;

	if (0 != (NIS_DEFAULTIMAGE & item->style))
		return -1;

	switch(imageType)
	{
		case IMAGE_NORMAL_I: return ((NAVITM*)hItem)->iImage;
		case IMAGE_SELECTED_I: return ((NAVITM*)hItem)->iSelectedImage;
	}
	return -1;
}

BOOL NavItemI_GetIndirect(HNAVITEM hItem, NAVITEM_I *pnis)
{
	NAVITM *pItem;
	if (!hItem || !pnis) return FALSE;
	pItem = (NAVITM*)hItem;

	if (NIMF_ITEMID_I & pnis->mask) pnis->id = pItem->id;
	if (NIMF_IMAGE_I & pnis->mask) pnis->iImage = pItem->iImage;
	if (NIMF_IMAGESEL_I & pnis->mask) pnis->iSelectedImage = pItem->iSelectedImage;
	if (NIMF_STYLE_I & pnis->mask) pnis->style = (pnis->styleMask & pItem->style);
	if (NIMF_TEXT_I & pnis->mask && !NavItemI_GetText(pItem, pnis->pszText, pnis->cchTextMax)) return FALSE;
	if (NIMF_TEXTINVARIANT_I & pnis->mask && !NavItemI_GetInvariantText(pItem, pnis->pszInvariant, pnis->cchInvariantMax)) return FALSE;
	if (NIMF_STATE_I & pnis->mask) pnis->state = NavItemI_GetState(pItem, 0xFFFFFFFF);
	if (NIMF_FONT_I & pnis->mask) pnis->hFont = NavItemI_GetFont(pItem);
	if (NIMF_PARAM_I & pnis->mask)	 pnis->lParam = pItem->lParam;

	return TRUE;
}

BOOL NavItemI_GetText(HNAVITEM hItem, LPWSTR pszText, INT cchMaxLen)
{
	if (!hItem || !pszText) return FALSE;
	return (S_OK == StringCchCopyW(pszText, cchMaxLen, (((NAVITM*)hItem)->pszText) ? ((NAVITM*)hItem)->pszText : L""));
}

BOOL NavItemI_GetInvariantText(HNAVITEM hItem, LPWSTR pszText, INT cchMaxLen)
{
	if (!hItem || !pszText) return FALSE;
	return (S_OK == StringCchCopyW(pszText, cchMaxLen, (((NAVITM*)hItem)->pszInvariant) ? ((NAVITM*)hItem)->pszInvariant : L""));
}

BOOL NavItemI_HasChildren(HNAVITEM hItem)
{
	return (hItem && (0 != (NIS_HASCHILDREN_I & ((NAVITM*)hItem)->style)));
}

BOOL NavItemI_HasChildrenReal(HNAVITEM hItem)
{
	return (hItem && SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)((NAVITM*)hItem)->hTreeItem));
}

BOOL NavItemI_IsSelected(HNAVITEM hItem)
{
	return (NIS_SELECTED_I == NavItemI_GetState(hItem, NIS_SELECTED_I));
}

BOOL NavItemI_SetId(HNAVITEM hItem, INT itemId)
{
	if (!hItem || itemId < 0) return FALSE;
	if (((NAVITM*)hItem)->id == itemId) return TRUE;
	if (FindNavItemByNavIdEx(((NAVITM*)hItem)->hwndTree, itemId, TVI_ROOT)) return FALSE;
	((NAVITM*)hItem)->id = itemId; 
	return TRUE;
}

BOOL NavItemI_SetImageIndex(HNAVITEM hItem, INT mlilIndex, INT imageType)
{
	if (!hItem) return FALSE;

	switch(imageType)
	{
		case IMAGE_NORMAL_I:	((NAVITM*)hItem)->iImage = mlilIndex; break;
		case IMAGE_SELECTED_I:	((NAVITM*)hItem)->iSelectedImage = mlilIndex; break;
		default: return FALSE;
	}
	NavItemI_Invalidate(hItem, NULL, FALSE);
	return TRUE;
}

BOOL NavItemI_SetStyle(HNAVITEM hItem, UINT style, UINT mask)
{
	UINT newStyle;
	if (!hItem) return FALSE;

	newStyle =  (((NAVITM*)hItem)->style & ~mask) | style;
	if (((NAVITM*)hItem)->style != newStyle)
	{
		((NAVITM*)hItem)->style = newStyle;
		NavItemI_Invalidate(hItem, NULL, FALSE);
	}
	return TRUE;
}

BOOL NavItemI_SetText(HNAVITEM hItem, LPCWSTR pszText)
{
	if (!hItem) return FALSE;
	if (!pszText)
	{
		free(((NAVITM*)hItem)->pszText);
		((NAVITM*)hItem)->pszText = NULL;
		((NAVITM*)hItem)->cchTextMax = 0;
	}
	else
	{
		INT len = lstrlenW(pszText);
		if (len >= ((NAVITM*)hItem)->cchTextMax)
		{
			LPVOID data;
			data = realloc(((NAVITM*)hItem)->pszText, sizeof(WCHAR)*(len + 4));
			if (!data) return FALSE;
			((NAVITM*)hItem)->pszText = (LPWSTR)data;
			((NAVITM*)hItem)->cchTextMax = len + 4;
		}
		if (S_OK != StringCchCopyW(((NAVITM*)hItem)->pszText, ((NAVITM*)hItem)->cchTextMax, pszText)) return FALSE;
	}
	NavItemI_Invalidate(hItem, NULL, FALSE);
	return TRUE;
}

BOOL NavItemI_SetInvariantText(HNAVITEM hItem, LPCWSTR pszText)
{
	if (!hItem) return FALSE;
	if (!pszText)
	{
		free(((NAVITM*)hItem)->pszInvariant);
		((NAVITM*)hItem)->pszInvariant = NULL;
		((NAVITM*)hItem)->cchInvariantMax = 0;
	}
	else
	{
		INT len = lstrlenW(pszText);
		if (len >= ((NAVITM*)hItem)->cchInvariantMax)
		{
			LPVOID data;
			data = realloc(((NAVITM*)hItem)->pszInvariant, sizeof(WCHAR)*(len + 4));
			if (!data) return FALSE;
			((NAVITM*)hItem)->pszInvariant = (LPWSTR)data;
			((NAVITM*)hItem)->cchInvariantMax = len + 4;
		}
		if (S_OK != StringCchCopyW(((NAVITM*)hItem)->pszInvariant, ((NAVITM*)hItem)->cchInvariantMax, pszText)) return FALSE;
	}
	return TRUE;
}

BOOL NavItemI_SetState(HNAVITEM hItem, UINT state, UINT stateMask)
{	
	TVITEMW item;
	if (!hItem) return FALSE;
	item.hItem		= ((NAVITM*)hItem)->hTreeItem;
	item.mask		= TVIF_STATE;
	item.state		= 0;
	item.stateMask	= 0;
	switch(state)
	{
		case NIS_SELECTED_I:		item.state |= TVIS_SELECTED; break;
		case NIS_EXPANDED_I:		item.state |= TVIS_EXPANDED; break;
		case NIS_DROPHILITED_I:	item.state |= TVIS_DROPHILITED; break;
	}
	switch(stateMask)
	{
		case NIS_SELECTED_I:		item.stateMask |= TVIS_SELECTED; break;
		case NIS_EXPANDED_I:		item.stateMask |= TVIS_EXPANDED; break;
		case NIS_DROPHILITED_I:	item.stateMask |= TVIS_DROPHILITED; break;
	}
	return (BOOL)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_SETITEMW, (WPARAM)0, (LPARAM)&item);
}

UINT NavItemI_GetState(HNAVITEM hItem, UINT stateMask)
{
	UINT treeState, navState;
	if (!hItem) return 0;

	treeState = (UINT)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETITEMSTATE, (WPARAM)((NAVITM*)hItem)->hTreeItem, (LPARAM)0xFFFFFFFF);
	navState = 0;
	if (TVIS_SELECTED & treeState) navState |= NIS_SELECTED_I;
	if (TVIS_EXPANDED & treeState) navState |= NIS_EXPANDED_I;
	if (TVIS_DROPHILITED & treeState) navState |= NIS_DROPHILITED_I;
	return (navState & stateMask);
}

UINT NavItemI_GetStyle(HNAVITEM hItem, UINT styleMask)
{
	return (hItem) ? (((NAVITM*)hItem)->style & styleMask) : 0;
}

BOOL NavItemI_SetIndirect(HNAVITEM hItem, NAVITEM_I *pnis)
{
	if (!hItem || !pnis) return FALSE;

	BOOL fResult = TRUE;
	((NAVITM*)hItem)->fBlockInvalid = TRUE;
	if (NIMF_ITEMID_I & pnis->mask && !NavItemI_SetId(hItem, pnis->id)) fResult = FALSE;
	if (NIMF_IMAGE_I & pnis->mask && !NavItemI_SetImageIndex(hItem, pnis->iImage, IMAGE_NORMAL_I)) fResult = FALSE;
	if (NIMF_IMAGESEL_I & pnis->mask && !NavItemI_SetImageIndex(hItem, pnis->iSelectedImage, IMAGE_SELECTED_I)) fResult = FALSE;
	if (NIMF_STYLE_I & pnis->mask && !NavItemI_SetStyle(hItem, pnis->style, pnis->styleMask)) fResult = FALSE;
	if (NIMF_TEXT_I & pnis->mask && !NavItemI_SetText(hItem, pnis->pszText)) fResult = FALSE;
	if (NIMF_TEXTINVARIANT_I & pnis->mask && !NavItemI_SetText(hItem, pnis->pszInvariant)) fResult = FALSE;
	if (NIMF_STATE_I & pnis->mask && !NavItemI_SetState(hItem, pnis->state, pnis->stateMask)) fResult = FALSE;
	if (NIMF_FONT_I & pnis->mask && !NavItemI_SetFont(hItem, pnis->hFont)) fResult = FALSE;
	if (NIMF_PARAM_I & pnis->mask)	 ((NAVITM*)hItem)->lParam = pnis->lParam;

	((NAVITM*)hItem)->fBlockInvalid = FALSE;
	if (!NavItemI_Invalidate(hItem, NULL, FALSE)) fResult = FALSE;

	return fResult;
}

BOOL NavItemI_GetRect(HNAVITEM hItem, RECT *prc, BOOL fItemRect)
{
	if (!hItem || !prc) return FALSE;
	*((HTREEITEM*)prc) = ((NAVITM*)hItem)->hTreeItem;
	return (BOOL)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETITEMRECT, (WPARAM)fItemRect, (LPARAM)prc);
}

BOOL NavItemI_Invalidate(HNAVITEM hItem, RECT *prc, BOOL fErase)
{	
	if (!hItem) return FALSE;
	if (!((NAVITM*)hItem)->fBlockInvalid)
	{
		RECT rc;
		if (NavItemI_GetRect(hItem, &rc, FALSE))
		{
			if (prc) IntersectRect(&rc, &rc, prc);
			InvalidateRect(((NAVITM*)hItem)->hwndTree, &rc, fErase);
			return TRUE;
		}
	}
	return TRUE;
}

BOOL NavItemI_Expand(HNAVITEM hItem, UINT flag)
{
	UINT tiFlag;
	if (!hItem) return FALSE;

	switch(flag)
	{
		case NAVITEM_TOGGLE_I:		tiFlag = TVE_TOGGLE; break;
		case NAVITEM_EXPAND_I:		tiFlag = TVE_EXPAND; break;
		case NAVITEM_COLLAPSE_I:	tiFlag = TVE_COLLAPSE; break;
		default:				return FALSE;
	}
	return (BOOL)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_EXPAND, (WPARAM)tiFlag, (LPARAM)((NAVITM*)hItem)->hTreeItem);
}

INT NavItemI_GetFullName(HNAVITEM hItem, LPWSTR pszFullName, INT cchMaxLen)
{
	wchar_t *current, *scanner, *text;
	INT remaining;

	if (!pszFullName || !hItem) return 0;
	pszFullName[0]	= 0x00;
	remaining		= cchMaxLen;
	current			= pszFullName;

	while(hItem && remaining)
	{
		text = (((NAVITM*)hItem)->pszInvariant) ? ((NAVITM*)hItem)->pszInvariant : ((NAVITM*)hItem)->pszText;
		if (!text) text = EMPTY_ITEM_NAME;

		if (current != pszFullName)
		{
			*current = SEPARATOR;
			current++;
			remaining--;
		}
		scanner = text + lstrlenW(text) - 1;
		BOOL second = FALSE;

		while (scanner >= text && remaining)
		{
			*current = *scanner;
			current++;
			remaining--;
			if (SEPARATOR == *scanner && !second) { second = TRUE;  continue; }
			second = FALSE;
			scanner--;
		}

		hItem = NavItemI_GetParent(hItem);
	}

	if (remaining)
	{
		*current = 0x00;
		current--;
		scanner = pszFullName;
		while (scanner < current)
		{
			wchar_t tmp = *scanner;
			*scanner = *current;
			*current = tmp;
			scanner++;
			current--;
		}
	}
	else
	{
		*pszFullName = 0x00;
		return 0;
	}

	return cchMaxLen - remaining;
}

BOOL NavItemI_Move(HNAVITEM hItem, HNAVITEM hItemDest, BOOL fAfter)
{
	WORD order;
	UINT flags;
	HNAVCTRL hNav;
	if (!hItem) return FALSE;

	if (hItemDest)
	{
		order = ((NAVITM*)hItemDest)->sortOrder;
		flags = (fAfter) ? NOF_MOVEONEAFTER_I : NOF_MOVEONEBEFORE_I;
	}
	else 
	{
		order = 1;
		flags = NOF_MOVEONEBEFORE_I;
	}

	hNav = (HNAVCTRL)GetPropW(((NAVITM*)hItem)->hwndTree, NAVMGR_HWNDPROPW);
	NavCtrlI_BeginUpdate(hNav, NUF_LOCK_NONE_I);
	order = NavItemI_SetOrder(hItem, order, flags);
	NavItemI_EnsureVisible(hItem);
	NavCtrlI_EndUpdate(hNav);

	return ((WORD)-1 != order);
}

static INT CALLBACK CommpareByOrderCB(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (lParam1 == lParam2) return 0;
	if (NULL == lParam1) return 1;
	return (((NAVITM*)lParam1)->sortOrder - ((NAVITM*)lParam2)->sortOrder);
}

static BOOL NavItemI_SetOrderWorker(HNAVITEM hItem, HTREEITEM hTreeFirst, WORD order, UINT flags)
{
	bool bRecurse;

	if (!hItem) return FALSE;

	bRecurse = false;

	if (0 == order) 
	{
		order = 1;
		flags = NOF_MOVEONEBEFORE_I;
	}

	if (((WORD)-1) == order || ((NAVITM*)hItem)->sortOrder != order) 
	{
		TVITEMW treeItem; // keep it here so it will be released prior to recurtion
		((NAVITM*)hItem)->sortOrder = (((WORD)-1) == order) ? 1 : order;

		treeItem.mask = TVIF_HANDLE | TVIF_PARAM;
		treeItem.hItem = hTreeFirst;

		while (treeItem.hItem) 
		{
			if (treeItem.hItem != ((NAVITM*)hItem)->hTreeItem)
			{
				if (!SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETITEMW, (WPARAM)0, (LPARAM)&treeItem)) return FALSE;
				if (((WORD)-1) == order)
				{
					if (treeItem.lParam && ((NAVITM*)hItem)->sortOrder <= ((NAVITM*)treeItem.lParam)->sortOrder)
						((NAVITM*)hItem)->sortOrder = ((NAVITM*)treeItem.lParam)->sortOrder + 1;
				}
				else 
				{
					if (treeItem.lParam && ((NAVITM*)treeItem.lParam)->sortOrder == order)
					{
						switch(flags)
						{
							case NOF_MOVEONEBEFORE_I: 
								hItem = (HNAVITEM)treeItem.lParam;
								order = ((NAVITM*)hItem)->sortOrder + 1;
								break;
							case NOF_MOVEONEAFTER_I:
								order++;
								flags = NOF_MOVEONEBEFORE_I;
								break;
							case NOF_MOVEBEFORE_I:
								if (1 == order) 
								{
									hItem = (HNAVITEM)treeItem.lParam;
									order = ((NAVITM*)hItem)->sortOrder + 1;
									flags = NOF_MOVEONEBEFORE_I;
								}
								else order--;
								break;
							case NOF_MOVEAFTER_I:
								order++;
								break;
							default: return FALSE;
						}
						bRecurse = true;
						break;
					}
				}
			}
			treeItem.hItem = (HTREEITEM)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)treeItem.hItem);
		}
	}

	return (bRecurse) ? NavItemI_SetOrderWorker(hItem, hTreeFirst, order, flags) : TRUE;
}

WORD NavItemI_SetOrder(HNAVITEM hItem, WORD order, UINT flags)
{
	HTREEITEM hTreeParent;
	if (!hItem) return (WORD)-1;

	hTreeParent = (HTREEITEM)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETNEXTITEM, (WPARAM)TVGN_PARENT, (LPARAM)((NAVITM*)hItem)->hTreeItem);

	if (NavItemI_SetOrderWorker(hItem, 
								(HTREEITEM)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETNEXTITEM,
														(WPARAM)((hTreeParent) ? TVGN_CHILD : TVGN_ROOT), (LPARAM)hTreeParent),
								order, flags))
	{
		HNAVCTRL hNav;
		TVSORTCB sort;

		sort.hParent = hTreeParent;
		sort.lpfnCompare = CommpareByOrderCB;
		sort.lParam = 0;

		hNav = (HNAVCTRL)GetPropW(((NAVITM*)hItem)->hwndTree, NAVMGR_HWNDPROPW);
		NavCtrlI_BeginUpdate(hNav, NUF_LOCK_NONE_I);
		SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_SORTCHILDRENCB, (WPARAM)FALSE, (LPARAM)&sort);
		NavCtrlI_EndUpdate(hNav);
	}
	return ((NAVITM*)hItem)->sortOrder;
}

WORD NavItemI_GetOrder(HNAVITEM hItem)
{
	return (hItem) ? ((NAVITM*)hItem)->sortOrder : 0xFFFF;
}

BOOL NavItemI_Select(HNAVITEM hItem)
{
	BOOL fResult;
	if (!hItem) return FALSE;
	fResult = (BOOL)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)((NAVITM*)hItem)->hTreeItem);
	SendMessageW(((NAVITM*)hItem)->hwndTree, WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), NULL);
	SendMessageW(((NAVITM*)hItem)->hwndTree, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
	return fResult;
}

BOOL NavItemI_EnsureVisible(HNAVITEM hItem)
{
	BOOL fResult;

	if (!hItem) return FALSE;
	fResult = (BOOL)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_ENSUREVISIBLE, (WPARAM)0, (LPARAM)((NAVITM*)hItem)->hTreeItem);
	if (fResult)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_RANGE;
		if(GetScrollInfo(((NAVITM*)hItem)->hwndTree, SB_HORZ, &si) && (si.nMin != si.nPos))
		{
			SendMessageW(((NAVITM*)hItem)->hwndTree, WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), NULL);
			SendMessageW(((NAVITM*)hItem)->hwndTree, WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
		}
	}

	return fResult;
}

HFONT NavItemI_GetFont(HNAVITEM hItem)
{
	return (hItem) ? ((NAVITM*)hItem)->hFont : NULL;
}

HFONT NavItemI_SetFont(HNAVITEM hItem, HFONT hFont)
{
	HFONT hFontOld;
	if (!hItem) return NULL;
	hFontOld = ((NAVITM*)hItem)->hFont;
	((NAVITM*)hItem)->hFont = hFont;
	NavItemI_Invalidate(hItem, NULL, FALSE);
	return hFontOld;
}

HIMAGELIST NavItemI_CreateDragImage(HNAVITEM hItem, LPCWSTR pszTipText)
{
	HIMAGELIST hIL;
	HNAVCTRL hNav;
	HBITMAP hbmp;
	INT cy, cx, ilIndex;
	RECT rcImage, rcText, rcTip;
	HDC hdcMem, hdcScreen;

	HFONT hFont, hFontOld, hFontTip;
	BOOL fDestroyFont;

	fDestroyFont = FALSE;
	hFont = NULL;
	hFontTip = NULL;

	if (!hItem) return NULL;

	hNav = (HNAVCTRL)GetPropW(((NAVITM*)hItem)->hwndTree, NAVMGR_HWNDPROPW);
	if (!hNav) return NULL;

	cx = DRAGIMAGE_OFFSET_X;
	cy = (INT)SendMessageW(((NAVITM*)hItem)->hwndTree, TVM_GETITEMHEIGHT, (WPARAM)0, (LPARAM)0L) + 2*DRAGIMAGE_OFFSET_Y;
	if (!cy) return NULL;

	hdcScreen = GetDCEx(((NAVITM*)hItem)->hwndTree, NULL, DCX_WINDOW | DCX_CACHE);
	hdcMem = (hdcScreen) ? CreateCompatibleDC(NULL) : NULL;
	if (!hdcMem)
	{
		if (hdcScreen) ReleaseDC(((NAVITM*)hItem)->hwndTree, hdcScreen);
		return NULL;
	}

	SetRect(&rcText, 0,0,0,0);
	SetRect(&rcTip, 0,0,0,0);

	if (((NAVITM*)hItem)->pszText)
	{
		if (((NAVITM*)hItem)->hFont) hFont = ((NAVITM*)hItem)->hFont;
		else 
		{
			hFont = (HFONT)::SendMessageW(((NAVITM*)hItem)->hwndTree, WM_GETFONT, 0, 0);
			if (NULL == hFont) 
				hFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);

			if ((NIS_BOLD_I | NIS_ITALIC_I | NIS_UNDERLINE_I) & ((NAVITM*)hItem)->style)
			{
				LOGFONTW lf = { 0 };
				GetObjectW(hFont, sizeof(LOGFONTW), &lf);
				if (NIS_BOLD_I & ((NAVITM*)hItem)->style) lf.lfWeight = FW_BOLD;
				if (NIS_ITALIC_I & ((NAVITM*)hItem)->style) lf.lfItalic = TRUE;
				if (NIS_UNDERLINE_I & ((NAVITM*)hItem)->style) lf.lfUnderline = TRUE;
				hFont = CreateFontIndirectW(&lf);
				fDestroyFont = TRUE;
			}
		}

		if (hFont) hFontOld = (HFONT)SelectObject(hdcMem, hFont);

		DrawTextW(hdcMem, ((NAVITM*)hItem)->pszText, -1, &rcText, DT_CALCRECT | DT_NOPREFIX);
		if(rcText.bottom - rcText.top > cy) cy = (rcText.bottom - rcText.top) + 2;
		cx += ((rcText.right - rcText.left) + DRAGIMAGE_OFFSET_X);
	}

	ilIndex = -1;

	if (((NAVMNGR*)hNav)->hmlilImages) 
	{
		hIL = (HIMAGELIST)SendMessageW(((NAVMNGR*)hNav)->hwndHost, TVM_GETIMAGELIST, (WPARAM)TVSIL_NORMAL, (LPARAM)0);
		if (hIL && MLImageListI_GetRealList(((NAVMNGR*)hNav)->hmlilImages) == hIL)
		{
			COLORREF rgbBk, rgbFg;
			GetItemColors(hNav, NIS_SELECTED_I | NIS_FOCUSED_I, &rgbBk, &rgbFg);
			ilIndex = GetRealImageIndex(hNav, (NAVITM*)hItem, IMAGE_SELECTED_I, rgbBk, rgbFg);
		}
	}
	else 
		hIL = NULL;

	if (-1 != ilIndex && NULL != hIL)
	{
		IMAGEINFO ii;
		ImageList_GetImageInfo(hIL, ilIndex, &ii);
		cx += (ii.rcImage.right - ii.rcImage.left) + 5;
		SetRect(&rcImage, 0, 0, ii.rcImage.right - ii.rcImage.left, ii.rcImage.bottom - ii.rcImage.top);
	}
	else SetRect(&rcImage, 0,0,0,0);

	OffsetRect(&rcImage, DRAGIMAGE_OFFSET_X, ((cy - DRAGIMAGE_OFFSET_Y) - (rcImage.bottom - rcImage.top))/2);
	OffsetRect(&rcText, rcImage.right + ((rcImage.right != rcImage.left) ? 5 : 0), ((cy - DRAGIMAGE_OFFSET_Y) - (rcText.bottom - rcText.top))/2);

	if (pszTipText && *pszTipText)
	{
		HFONT hFontTmp;
		hFontTip = (HFONT)::SendMessageW(((NAVITM*)hItem)->hwndTree, WM_GETFONT, 0, 0);
		if (!hFontTip) hFontTip = (HFONT)MlStockObjects_Get(DEFAULT_FONT);

		hFontTmp = (HFONT)SelectObject(hdcMem, hFontTip);
		DrawTextW(hdcMem, pszTipText, -1, &rcTip, DT_CALCRECT | DT_NOPREFIX);
		SelectObject(hdcMem, hFontTmp);
		if (rcTip.right - rcTip.left > (cx - rcText.left  + 3)) cx = rcText.left + (rcTip.right - rcTip.left) + 3;
		OffsetRect(&rcTip, rcText.left, cy + 1);
		cy += ((rcTip.bottom - rcTip.top) + 3);
	}

	hbmp = (3 != cx) ? CreateCompatibleBitmap(hdcScreen, cx, cy) : NULL;

	if (hbmp)
	{
		HGDIOBJ hgdiOld;
		RECT rc;

		hgdiOld = SelectObject(hdcMem, hbmp);

		SetBkColor(hdcMem, WADlg_getColor(WADLG_SELBAR_BGCOLOR));
		SetTextColor(hdcMem, WADlg_getColor(WADLG_SELBAR_FGCOLOR));

		SetRect(&rc, 0, 0, cx, cy);
		ExtTextOutW(hdcMem, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);

		if (hIL && -1 != ilIndex)
		{
			ImageList_DrawEx(hIL, ilIndex, hdcMem, rcImage.left, rcImage.top,
							(rcImage.right - rcImage.left), (rcImage.bottom - rcImage.top), CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL);
		}

		if (((NAVITM*)hItem)->pszText) 	DrawTextW(hdcMem, ((NAVITM*)hItem)->pszText, -1, &rcText, DT_NOPREFIX);
		if (pszTipText && *pszTipText)
		{
			HFONT hFontTmp;
			hFontTmp = (HFONT)SelectObject(hdcMem, hFontTip);
			DrawTextW(hdcMem, pszTipText, -1, &rcTip, DT_NOPREFIX);
			SelectObject(hdcMem, hFontTmp);
		}

		SelectObject(hdcMem, hgdiOld);

		hIL = ImageList_Create(cx, cy, ILC_COLOR24 | ILC_MASK, 0, 1);
		if (hIL && -1 == ImageList_Add(hIL, hbmp, NULL))
		{
			ImageList_Destroy(hIL);
			hIL = NULL;
		}

		DeleteObject(hbmp);
	}

	if (hFont) 
	{
		SelectObject(hdcMem, hFontOld);
		if (fDestroyFont) DeleteObject(hFont);
	}

//	if (hFontTip) DeleteObject(hFontTip); // we not creating it...

	DeleteDC(hdcMem);
	ReleaseDC(((NAVITM*)hItem)->hwndTree, hdcScreen);

	return hIL;
}