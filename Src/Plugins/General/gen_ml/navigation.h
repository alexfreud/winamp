#ifndef NULLOSFT_MEDIALIBRARY_NAVIGATION_HEADER
#define NULLOSFT_MEDIALIBRARY_NAVIGATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "./config.h"
#include "./ml_imagelist.h"

typedef LPVOID HNAVCTRL;
typedef LPVOID HNAVITEM;

typedef struct _NAVITEM_I
{
	UINT	mask;
	INT		id;
	LPWSTR	pszText;
	INT		cchTextMax;
	LPWSTR	pszInvariant;
	INT		cchInvariantMax;
	INT		iImage;
	INT		iSelectedImage;
	UINT	state;
	UINT	stateMask;
	UINT	style;
	UINT	styleMask;
	HFONT	hFont;
	LPARAM	lParam;
} NAVITEM_I;

typedef struct _NAVITEMDRAW_I
{	
	HDC			hdc;			//
	COLORREF	clrText;		//
	COLORREF	clrTextBk;		//
	HFONT		hFont;			//
	RECT		*prc;			// 
	UINT		itemState;		// NIS_XXX
	UINT		drawStage;		// NIDS_XXX
	INT			iLevel;			// Zero-based level of the item being drawn. 
} NAVITEMDRAW_I;


// navigation control styles
#define NCS_NORMAL_I				0x0000
#define NCS_FULLROWSELECT_I		0x0001
#define NCS_SHOWICONS_I			0x0002

// nav item draw stage
#define NIDS_PREPAINT_I		0x0001
#define NIDS_POSTPAINT_I	0x0002

// item custom draw callback return values
#define NICDRF_DODEFAULT_I			0x0001
#define NICDRF_SKIPDEFAULT_I		0x0002
#define NICDRF_NOTIFYPOSTPAINT_I		0x0004
#define NICDRF_NEWFONT_I				0x0008


// hit test flags
#define NAVHT_NOWHERE_I			0x0001
#define NAVHT_ONITEM_I			0x0002
#define NAVHT_ONITEMBUTTON_I		0x0004	// only if item currently has children
#define NAVHT_ONITEMINDENT_I		0x0010
#define NAVHT_ONITEMRIGHT_I		0x0020
#define NAVHT_ABOVE_I			0x0100
#define NAVHT_BELOW_I			0x0200
#define NAVHT_TORIGHT_I			0x0400
#define NAVHT_TOLEFT_I			0x0800

// navigation item masks 
#define NIMF_ITEMID_I			0x0001
#define NIMF_TEXT_I				0x0002
#define NIMF_TEXTINVARIANT_I		0x0004
#define NIMF_IMAGE_I				0x0008
#define NIMF_IMAGESEL_I			0x0010
#define NIMF_STATE_I				0x0020
#define NIMF_STYLE_I				0x0040
#define NIMF_FONT_I				0x0080
#define NIMF_PARAM_I				0x0100

// states
#define NIS_NORMAL_I				0x0000
#define NIS_SELECTED_I			0x0001
#define NIS_EXPANDED_I			0x0002
#define NIS_DROPHILITED_I		0x0004
#define NIS_FOCUSED_I			0x0008	// used with draw item

// styles
#define NIS_HASCHILDREN_I		0x0001	// item has children
#define NIS_ALLOWCHILDMOVE_I		0x0002	// allow children to be moved (re-ordered)
#define NIS_ALLOWEDIT_I			0x0004	// allow title edit
#define NIS_ITALIC_I			0x0100	// when displaying item text draw it with italic style 
#define NIS_BOLD_I				0x0200	// when displaying item text draw it with bold style
#define NIS_UNDERLINE_I			0x0400	// when displaying item text draw it with underline style
#define NIS_CUSTOMDRAW_I			0x0010	// custom draw calback
#define NIS_WANTSETCURSOR_I		0x0020	// item want to recive set cursor notification
#define NIS_WANTHITTEST_I		0x0040	// item want to monitor/modify hittest results
/// internal style - do not use
#define NIS_WANTPOSTPAINT_I		0x8000	// custom draw calback


// image types
#define IMAGE_NORMAL_I			0x0000
#define IMAGE_SELECTED_I			0x0001

// item expand command flags
#define NAVITEM_TOGGLE_I			0x0000
#define NAVITEM_EXPAND_I			0x0001
#define NAVITEM_COLLAPSE_I		0x0002

// action codes
#define ACTION_CLICKL_I			0x0000
#define ACTION_CLICKR_I			0x0001
#define ACTION_ENTER_I			0x0002
#define ACTION_DBLCLICKL_I		0x0003
#define ACTION_DBLCLICKR_I		0x0004

// navigation callbacks ident
#define CALLBACK_ONCLICK_I				0x0001
#define CALLBACK_ONSELECTED_I			0x0002
#define CALLBACK_ONKEYDOWN_I				0x0003
#define CALLBACK_ONBEGINDRAG_I			0x0004
#define CALLBACK_ONGETIMAGEINDEX_I		0x0005
#define CALLBACK_ONBEGINTITLEEDIT_I		0x0006
#define CALLBACK_ONENDTITLEEDIT_I		0x0007
#define CALLBACK_ONITEMDELETE_I			0x0008
#define CALLBACK_ONITEMDRAW_I			0x0009
#define CALLBACK_ONSETCURSOR_I			0x000A
#define CALLBACK_ONHITTEST_I			0x000B
#define CALLBACK_ONDESTROY_I				0x000C

typedef BOOL (CALLBACK *ONNAVITEMCLICK_I)(HNAVCTRL /*hNav*/, HNAVITEM /*hItem*/, INT /*actionId*/); // return TRUE if you want to prevent further processing
typedef void (CALLBACK *ONNAVITEMSELECTED_I)(HNAVCTRL /*hNav*/, HNAVITEM /*hItemOld*/, HNAVITEM /*hItemNew*/);
typedef BOOL (CALLBACK *ONNAVCTRLKEYDOWN_I)(HNAVCTRL /*hNav*/,  HNAVITEM /*hItem*/, NMTVKEYDOWN* /*ptvkd*/);
typedef void (CALLBACK *ONNAVCTRLBEGINDRAG_I)(HNAVCTRL /*hNav*/,  HNAVITEM /*hItem*/, POINT /*pt*/);
typedef INT (CALLBACK *ONNAVITEMGETIMAGEINDEX_I)(HNAVCTRL /*hNav*/,  HNAVITEM /*hItem*/, INT /*imageType*/);  // if item image index == -1 you can provide some some other index
typedef BOOL (CALLBACK *ONNAVITEMBEGINTITLEEDIT_I)(HNAVCTRL /*hNav*/,  HNAVITEM /*hItem*/);  // return TRUE to allow edit
typedef BOOL (CALLBACK *ONNAVCTRLENDTITLEEDIT_I)(HNAVCTRL /*hNav*/,  HNAVITEM /*hItem*/, LPCWSTR /*pszNewTitle*/); // if pszNewTitle == NULL edit wa canceled. Return TRUE to accept new title  
typedef void (CALLBACK *ONNAVITEMDELETE_I)(HNAVCTRL /*hNav*/,  HNAVITEM /*hItem*/);  // 
typedef UINT (CALLBACK *ONNAVITEMDRAW_I)(HNAVCTRL /*hNav*/,  HNAVITEM /*hItem*/, NAVITEMDRAW_I* /*pnicd*/, LPARAM /*lParam*/);  // custom draw
typedef BOOL (CALLBACK *ONNAVITEMSETCURSOR_I)(HNAVCTRL /*hNav*/, HNAVITEM /*hItem*/, LPARAM /*lParam*/);  // set cursor
typedef void (CALLBACK *ONNAVITEMHITTEST_I)(HNAVCTRL /*hNav*/, POINT /*pt*/, UINT* /*pFlags*/, HNAVITEM* /*phItem*/, LPARAM /*lParam*/);
typedef void (CALLBACK *ONNAVCTRLDESTROY_I)(HNAVCTRL /*hNav*/);

// name compare flags
#define NICF_DISPLAY_I		0x0001 // compare display name (if specified in combination with othe flags will be used last)
#define NICF_INVARIANT_I	0x0002 // compare invariant name (if specified in combination with other flags will be used after FULL and before DISPLAY
#define NICF_IGNORECASE_I	0x0004 // ignore case (always used when comparing invariant names)

// NavItemI_SetOrder flags
#define NOF_MOVEONEBEFORE_I	0x00	// if order inded already used set order for this item and move old item on one position after until all items rearranged
#define NOF_MOVEONEAFTER_I	0x01	// 	
#define NOF_MOVEBEFORE_I		0x02		//
#define NOF_MOVEAFTER_I		0x03	//

// NavCtrlI_EnumItems callback
typedef BOOL (CALLBACK *NAVENUMPROC_I)(HNAVITEM /*hItem*/, LPARAM /*lParam*/); // return FALSE to stop enumeration

// Use this to in NavCtrlI_InertItem to create hInsertAfter
#define MAKE_NAVITEMSORTORDER(o) ((HNAVITEM)((ULONG_PTR)((WORD)(o))))
#define IS_NAVITEMSORTORDER(_i) ((((ULONG_PTR)(_i)) >> 16) == 0)

#define NUF_LOCK_NONE_I			0x00
#define NUF_LOCK_SELECTED_I		0x01 // try to remember selected position if no selected items in the view it will fallback to NUF_REMEMBER_TOP_I
#define NUF_LOCK_TOP_I			0x02	 // try to remeber top item...

// insert item Insert after flags
#define NCI_FIRST_I				((HNAVITEM)(ULONG_PTR)-0x0FFFF)	// insert first item 
#define NCI_LAST_I				((HNAVITEM)(ULONG_PTR)-0x00000) // insert as last item

// Navigation Control

// NavCtrlI_Create - create new navigation manager
HNAVCTRL NavCtrlI_Create(HWND hwndParent);
BOOL NavCtrlI_SetRect(HNAVCTRL hNav, RECT *prc);
BOOL NavCtrlI_Show(HNAVCTRL hNav, INT nCmdShow);
BOOL NavCtrlI_Enable(HNAVCTRL hNav, BOOL fEnable);
BOOL NavCtrlI_Destroy(HNAVCTRL hNav);
BOOL NavCtrlI_Update(HNAVCTRL hNav); // calls UpdateWindow for the host;
BOOL NavCtrlI_ProcessNotifications(HNAVCTRL hNav, LPNMHDR pnmh, LRESULT *pResult);
C_Config *NavCtrlI_SetConfig(HNAVCTRL hNav, C_Config *pConfig); 

BOOL NavCtrlI_BeginUpdate(HNAVCTRL hNav, UINT fRememberPos); // call to prevent redraw (use NUF_REMEMBER_XXX)
BOOL NavCtrlI_EndUpdate(HNAVCTRL hNav);	// call to allow redraw

BOOL NavCtrlI_DeleteItem(HNAVCTRL hNav, HNAVITEM hItem);
BOOL NavCtrlI_DeleteAll(HNAVCTRL hNav);
HNAVITEM NavCtrlI_FindItem(HNAVCTRL hNav, INT itemId);
HNAVITEM NavCtrlI_FindItemByName(HNAVCTRL hNav, LCID Locale, UINT compFlags, LPCWSTR pszName, INT cchLength); // use one of the NICF_* flags
HNAVITEM NavCtrlI_FindItemByFullName(HNAVCTRL hNav, LCID Locale, UINT compFlags, LPCWSTR pszName, INT cchLength, BOOL fAncestorOk); // use one of the NICF_* flags. if fAncestor is set and item with this name not exist - will fall back to closest ancestor

HNAVITEM NavCtrlI_InsertItem(HNAVCTRL hNav, HNAVITEM hInsertAfter, HNAVITEM hParent, NAVITEM_I *pnis); // if hInsertAfter > 0xFFFF it is consider as an item  otherwise it is sort order, Use MAKE_NAVITEMSORTORDER macro
HNAVITEM NavCtrlI_GetRoot(HNAVCTRL hNav);
HNAVITEM NavCtrlI_GetSelection(HNAVCTRL hNav);
HNAVITEM NavCtrlI_GetFirstVisible(HNAVCTRL hNav);
HNAVITEM NavCtrlI_GetLastVisible(HNAVCTRL hNav);
BOOL NavCtrlI_UpdateLook(HNAVCTRL hNav);

HMLIMGLST NavCtrlI_SetImageList(HNAVCTRL hNav, HMLIMGLST hmlil);
HMLIMGLST NavCtrlI_GetImageList(HNAVCTRL hNav);

HWND NavCtrlI_GetHWND(HNAVCTRL hNav);
LPVOID NavCtrlI_RegisterCallback(HNAVCTRL hNav, LPVOID fnCallback, INT cbType);
HNAVITEM NavCtrlI_HitTest(HNAVCTRL hNav, POINT *ppt, UINT *pFlags);
BOOL NavCtrlI_SetInsertMark(HNAVCTRL hNav, HNAVITEM hItem, BOOL fAfter);

BOOL NavCtrlI_EnumItems(HNAVCTRL hNav, NAVENUMPROC_I pEnumFunc, HNAVITEM hItemStart, LPARAM lParam);
INT NavCtrlI_MapPointsTo(HNAVCTRL hNav, HWND hwndTo, POINT *ppt, UINT cPoints);
INT NavCtrlI_MapPointsFrom(HNAVCTRL hNav, HWND hwndFrom, POINT *ppt, UINT cPoints);

BOOL NavCtrlI_EndEditTitle(HNAVCTRL hNav, BOOL fCancel); 
INT NavCtrlI_GetIndent(HNAVCTRL hNav);
DWORD NavCtrlI_GetStyle(HNAVCTRL hNav); // NCS_XXX
	
// Navigation Item

BOOL NavItemI_EditTitle(HNAVITEM hItem);
HNAVITEM NavItemI_GetChild(HNAVITEM hItem);
INT NavItemI_GetChildrenCount(HNAVITEM hItem);
HFONT NavItemI_GetFont(HNAVITEM hItem);
INT NavItemI_GetId(HNAVITEM hItem);
INT NavItemI_GetImageIndex(HNAVITEM hItem, INT imageType);
BOOL NavItemI_GetIndirect(HNAVITEM hItem, NAVITEM_I *pnis);
BOOL NavItemI_GetText(HNAVITEM hItem, LPWSTR pszText, INT cchMaxLen);
BOOL NavItemI_GetInvariantText(HNAVITEM hItem, LPWSTR pszText, INT cchMaxLen);
HNAVITEM NavItemI_GetNext(HNAVITEM hItem);
WORD NavItemI_GetOrder(HNAVITEM hItem);
HNAVITEM NavItemI_GetParent(HNAVITEM hItem);
HNAVITEM NavItemI_GetPrevious(HNAVITEM hItem);
BOOL NavItemI_GetRect(HNAVITEM hItem, RECT *prc, BOOL fItemRect);
HNAVITEM NavItemI_GetRoot(HNAVITEM hItem);
UINT NavItemI_GetState(HNAVITEM hItem, UINT stateMask);
UINT NavItemI_GetStyle(HNAVITEM hItem, UINT styleMask);
INT NavItemI_GetFullName(HNAVITEM hItem, LPWSTR pszFullName, INT cchMaxLen);


BOOL NavItemI_HasChildren(HNAVITEM hItem); // checks if item style for NIS_HASCHILDREN_I
BOOL NavItemI_HasChildrenReal(HNAVITEM hItem); // cheks if item actually has at least one child
BOOL NavItemI_IsSelected(HNAVITEM hItem);
BOOL NavItemI_IsExpanded(HNAVITEM hItem);
BOOL NavItemI_Expand(HNAVITEM hItem, UINT flag);
BOOL NavItemI_Move(HNAVITEM hItem, HNAVITEM hItemDest, BOOL fAfter);
HFONT NavItemI_SetFont(HNAVITEM hItem, HFONT hFont); // you stil own this font !!! (set hFont to NULL if you want to use treeview font). Returns previously set font.
BOOL NavItemI_SetId(HNAVITEM hItem, INT itemId);
BOOL NavItemI_SetImageIndex(HNAVITEM hItem, INT mlilIndex, INT imageType);
BOOL NavItemI_SetIndirect(HNAVITEM hItem, NAVITEM_I *pnis);
BOOL NavItemI_SetState(HNAVITEM hItem, UINT state, UINT stateMask);
BOOL NavItemI_SetStyle(HNAVITEM hItem, UINT style, UINT mask);
BOOL NavItemI_SetText(HNAVITEM hItem, LPCWSTR pszText);
BOOL NavItemI_SetInvariantText(HNAVITEM hItem, LPCWSTR pszText);


// Sets Item order and modifies all items oreder after it if required.
// if oder == 0xFFFF order will be set to max + 1 for this group
// returns new item order or 0xFFFF if error;
// minimal order value 1.
// flags one of the NOF_XXX
WORD NavItemI_SetOrder(HNAVITEM hItem, WORD order, UINT flags); 
BOOL NavItemI_EnsureVisible(HNAVITEM hItem);
BOOL NavItemI_Select(HNAVITEM hItem);
HIMAGELIST NavItemI_CreateDragImage(HNAVITEM hItem, LPCWSTR pszTipText);
BOOL NavItemI_Invalidate(HNAVITEM hItem, RECT *prc, BOOL fErase);

#endif //NULLOSFT_MEDIALIBRARY_NAVIGATION_HEADER