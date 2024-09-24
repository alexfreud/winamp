#include "./fileview.h"
#include "./fileview_internal.h"
#include "./resource.h"
#include "../nu/menushortcuts.h"
#include <windowsx.h>
#include <strsafe.h>

#define SUBMENU_VIEWMODE 	0
#define SUBMENU_OPTIONS		1
#define SUBMENU_ARRANGEBY	2
#define SUBMENU_COLUMNS		3
#define SUBMENU_FILELIST		4
#define SUBMENU_COMMON		5
#define SUBMENU_FILELIST_PLAY			0
#define SUBMENU_FILELIST_VIEWCONTEXT		1
#define SUBMENU_FILELIST_OPCONTEXT		2


static INT MenuCopyEx(HMENU hDest, INT iDstStart, HMENU hSource, INT iSrcStart, INT iSrcCount)
{
	if (!hDest || !hSource) return 0;
	wchar_t szText[1024] = {0};
	INT pos;

	MENUITEMINFOW mii = {sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_DATA | MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
	mii.dwTypeData = szText;

	if (iDstStart < 0) iDstStart = 0;
	if (iSrcStart < 0) iSrcStart = 0;

	pos = iDstStart;

	if ( 0 != iSrcCount)
	{
		INT c = GetMenuItemCount(hSource);
		if (iSrcStart > c) return 0;
		if (iSrcCount < 0) iSrcCount = c - iSrcStart;
		else if (iSrcCount < (c - iSrcStart)) c = iSrcCount + iSrcStart;

		for (int i = iSrcStart; i < c; i++) 
		{
			mii.cch = sizeof(szText)/sizeof(szText[0]);
			if (GetMenuItemInfoW(hSource, i, TRUE, &mii))
			{
				if(InsertMenuItemW(hDest, pos, TRUE, &mii)) 
				{
					pos++;
				}
			}
		}
	}
	else
	{
		mii.cch = sizeof(szText)/sizeof(szText[0]);
		if (GetMenuItemInfoW(hSource, iSrcStart, FALSE, &mii))
		{
			if (InsertMenuItemW(hDest, pos, TRUE, &mii))
			{
				pos++;
			}
		}
	}

	return pos - iDstStart;
}

static INT MenuCopy(HMENU hDest, INT iDstStart, HMENU hSource)
{
	return MenuCopyEx(hDest, iDstStart, hSource, 0, -1);
}

static BOOL MenuInsertSeparator(HMENU hMenu, INT iPos)
{
	if (!hMenu) return FALSE;
	MENUITEMINFOW mii = {sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_FTYPE;
	mii.fType = MFT_SEPARATOR;
	return InsertMenuItemW(hMenu, iPos, TRUE, &mii);
}

static BOOL MenuInsertCommonItem(HMENU hMenu, INT iPos, HMENU hViewMenu, UINT itemId)
{
	if (!hMenu) return FALSE;
	HMENU hCommon = GetSubMenu(hViewMenu, SUBMENU_COMMON);
	if (!hCommon) return FALSE;
	return (MenuCopyEx(hMenu, iPos, hCommon, itemId, 0) > 0);
}

static HMENU FileViewMenu_GetViewModeMenu(HWND hView, HMENU hViewMenu)
{
	HMENU hMenu = GetSubMenu(hViewMenu, SUBMENU_VIEWMODE);
	if (!hMenu) return NULL;
	UINT style = (FVS_VIEWMASK & FileView_GetStyle(hView));
	MENUITEMINFOW mii = {sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_STATE | MIIM_FTYPE;
	if (GetMenuItemInfoW(hMenu, ID_FILEVIEW_SETMODE_ICON, FALSE, &mii))
	{
		mii.fType |= MFT_RADIOCHECK;
		mii.fState = (FVS_ICONVIEW == style) ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfoW(hMenu, ID_FILEVIEW_SETMODE_ICON, FALSE, &mii);
	}
	if (GetMenuItemInfoW(hMenu, ID_FILEVIEW_SETMODE_LIST, FALSE, &mii))
	{
		mii.fType |= MFT_RADIOCHECK;
		mii.fState = (FVS_LISTVIEW == style) ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfoW(hMenu, ID_FILEVIEW_SETMODE_LIST, FALSE, &mii);
	}
	if (GetMenuItemInfoW(hMenu, ID_FILEVIEW_SETMODE_DETAIL, FALSE, &mii))
	{
		mii.fType |= MFT_RADIOCHECK;
		mii.fState = (FVS_DETAILVIEW == style) ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfoW(hMenu, ID_FILEVIEW_SETMODE_DETAIL, FALSE, &mii);
	}
	return hMenu;
}

static HMENU FileViewMenu_GetOptionsMenu(HWND hView, HMENU hViewMenu)
{
	HMENU hMenu = GetSubMenu(hViewMenu, SUBMENU_OPTIONS);
	if (!hMenu) return NULL;
	UINT style = FileView_GetStyle(hView);
	CheckMenuItem(hMenu, IDM_FILEVIEW_HIDEEXTENSION, MF_BYCOMMAND | ((FVS_HIDEEXTENSION & style) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hMenu, IDM_FILEVIEW_IGNOREHIDDEN, MF_BYCOMMAND | ((FVS_IGNOREHIDDEN & style) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hMenu, IDM_FILEVIEW_SHOWAUDIO, MF_BYCOMMAND | ((FVS_SHOWAUDIO & style) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hMenu, IDM_FILEVIEW_SHOWVIDEO, MF_BYCOMMAND | ((FVS_SHOWVIDEO & style) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hMenu, IDM_FILEVIEW_SHOWPLAYLIST, MF_BYCOMMAND | ((FVS_SHOWPLAYLIST & style) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hMenu, IDM_FILEVIEW_SHOWUNKNOWN, MF_BYCOMMAND | ((FVS_SHOWUNKNOWN & style) ? MF_CHECKED : MF_UNCHECKED));
	return hMenu;
}

static HMENU FileViewMenu_GetArrangeByMenu(HWND hView, HMENU hViewMenu)
{
	HMENU hMenu = GetSubMenu(hViewMenu, SUBMENU_ARRANGEBY);
	if (!hMenu) return NULL;

	UINT sort = FileView_GetSort(hView);
	UINT sortCol = LOWORD(sort);
	UINT szColumn[512] = {0};
	WCHAR szText[256] = {0};
	INT count;

	MENUITEMINFOW mii = {sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_ID;
	count = GetMenuItemCount(hMenu);

	while(count--)
	{
		if (GetMenuItemInfoW(hMenu, count, TRUE, &mii) && 
			mii.wID >= IDM_COLUMN_ARRANGE_MIN && mii.wID <= IDM_COLUMN_ARRANGE_MAX)
		{
			RemoveMenu(hMenu, count, MF_BYPOSITION);
		}
	}

	mii.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID | MIIM_FTYPE;
	mii.fType = MFT_RADIOCHECK;

	count = FileView_GetColumnArray(hView, 512, &szColumn);

	for (int i = 0; i < count; i++)
	{
		FileView_GetColumnName(hView, szColumn[i], sizeof(szText)/sizeof(szText[0]), szText);
		mii.dwTypeData = szText;
		mii.wID = IDM_COLUMN_ARRANGE_MIN + szColumn[i];
		mii.fState = (sortCol == szColumn[i]) ? MFS_CHECKED : 0;
		InsertMenuItemW(hMenu, i, TRUE, &mii);
	}

	CheckMenuItem(hMenu, IDM_FILEVIEW_SORTASCENDING, MF_BYCOMMAND | (HIWORD(sort) ? MF_CHECKED : MF_UNCHECKED));

	return hMenu;
}

static HMENU FileViewMenu_GetColumnsMenu(HWND hView, HMENU hViewMenu)
{
	HMENU hMenu = GetSubMenu(hViewMenu, SUBMENU_COLUMNS);
	if (!hMenu) return NULL;

	UINT szColumn[512] = {0};
	INT count;

	MENUITEMINFOW mii = {sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_ID;
	count = GetMenuItemCount(hMenu);

	while(count--)
	{
		if (GetMenuItemInfoW(hMenu, count, TRUE, &mii) && 
			mii.wID >= IDM_COLUMN_SHOW_MIN && mii.wID <= IDM_COLUMN_SHOW_MAX)
		{
			RemoveMenu(hMenu, count, MF_BYPOSITION);
		}
	}

	mii.fMask = MIIM_STRING | MIIM_STATE | MIIM_ID;

	count = FileView_GetColumnArray(hView, 512, &szColumn);

	for (INT i = 0; i < RegisteredColumnsCount; i++)
	{
		mii.dwTypeData = IS_INTRESOURCE(szRegisteredColumns[i].pszText) ? 
			WASABI_API_LNGSTRINGW((UINT)(UINT_PTR)szRegisteredColumns[i].pszText) : szRegisteredColumns[i].pszText;
		mii.wID = IDM_COLUMN_SHOW_MIN + szRegisteredColumns[i].id;

		INT index = 0;
		while(index < count && szRegisteredColumns[i].id != szColumn[index]) index++;
		mii.fState = (index < count) ? MFS_CHECKED : 0;
		if(FVCOLUMN_NAME == szRegisteredColumns[i].id) mii.fState |= MFS_DISABLED;
		InsertMenuItemW(hMenu, i, TRUE, &mii);
	}
	return hMenu;
}

static HMENU FileViewMenu_GetPlayMenu(HWND hView, HMENU hViewMenu)
{
	HMENU hMenu = GetSubMenu(hViewMenu, SUBMENU_FILELIST);
	if (hMenu) hMenu = GetSubMenu(hMenu, SUBMENU_FILELIST_PLAY);
	if (!hMenu) return NULL;

	BOOL bEnablePlay = (FileView_GetSelectedCount(hView) &&
				(-1 != FileView_GetNextFile(hView, -1, FVNF_PLAYABLE | FVNF_SELECTED)));

	MENUITEMINFOW mii = {sizeof(MENUITEMINFOW), };
	mii.fMask = MIIM_ID | MIIM_STATE;
	mii.fState = (bEnablePlay) ? MFS_ENABLED : MFS_DISABLED;

	mii.wID = FileView_GetActionCommand(hView, FVA_PLAY);
	SetMenuItemInfoW(hMenu, 0, TRUE, &mii);
	mii.wID = mii.wID = FileView_GetActionCommand(hView, FVA_ENQUEUE);
	SetMenuItemInfoW(hMenu, 1, TRUE, &mii);

	return hMenu;
}

static void FileViewMenu_PopulateFileOpMenu(HMENU hMenu, HWND hView, HMENU hViewMenu)
{
	INT c, pos = 0;

	c = MenuCopy(hMenu, pos, FileViewMenu_GetPlayMenu(hView, hViewMenu));
	pos += c;
	if (MenuInsertCommonItem(hMenu, pos, hViewMenu, ID_FILEVIEW_SELECT_ALL) && c)
		MenuInsertSeparator(hMenu, pos);
}

static HMENU FileViewMenu_GetFileViewMenu(HWND hView, HMENU hViewMenu)
{
	HMENU hMenu = GetSubMenu(hViewMenu, SUBMENU_FILELIST);
	if (hMenu) hMenu = GetSubMenu(hMenu, SUBMENU_FILELIST_VIEWCONTEXT);
	if (!hMenu) return NULL;

	INT c, pos= 0;

	c = GetMenuItemCount(hMenu);
	while(c--) RemoveMenu(hMenu, c, MF_BYPOSITION);

	c = MenuCopyEx(hMenu, pos, hViewMenu, SUBMENU_VIEWMODE, 1);
	pos += c;
	if (c && MenuInsertSeparator(hMenu, pos)) pos++;
	pos += MenuCopyEx(hMenu, pos, hViewMenu, SUBMENU_ARRANGEBY, 1);
	if (MenuInsertCommonItem(hMenu, pos, hViewMenu, ID_FILEVIEW_REFRESH)) pos++;
	if (MenuInsertSeparator(hMenu, pos)) pos++;
	MenuCopyEx(hMenu, pos, hViewMenu, SUBMENU_OPTIONS, 1);
	return hMenu;
}

static HMENU FileViewMenu_GetFileOpMenu(HWND hView, HMENU hViewMenu)
{
	HMENU hMenu = GetSubMenu(hViewMenu, SUBMENU_FILELIST);
	if (hMenu) hMenu = GetSubMenu(hMenu, SUBMENU_FILELIST_OPCONTEXT);
	if (!hMenu) return NULL;

	INT c, pos = 0;

	c = GetMenuItemCount(hMenu);
	while(c--) RemoveMenu(hMenu, c, MF_BYPOSITION);

	c = MenuCopy(hMenu, pos, FileViewMenu_GetPlayMenu(hView, hViewMenu));
	pos += c;
	if (MenuInsertCommonItem(hMenu, pos, hViewMenu, ID_FILEVIEW_SELECT_ALL) && c)
		MenuInsertSeparator(hMenu, pos);
	return hMenu;
}

static HMENU FileViewMenu_GetFileMenu(HWND hView, HMENU hViewMenu)
{
	FVHITTEST ht;
	GetCursorPos(&ht.pt);
	MapWindowPoints(HWND_DESKTOP, hView, &ht.pt, 1);

	if (-1 != FileView_HitTest(hView, &ht) && (FVHT_ONFILE & ht.uFlags)) 
		return FileViewMenu_GetFileOpMenu(hView, hViewMenu);
	else 
		return FileViewMenu_GetFileViewMenu(hView, hViewMenu);;
}

HMENU FileViewMenu_Initialize()
{
	return WASABI_API_LOADMENUW(IDR_MENU_FILEVIEW);
}

HMENU FileViewMenu_GetSubMenu(HWND hView, HMENU hViewMenu, UINT uMenuType)
{
	HMENU hMenu = NULL;
	switch(uMenuType)
	{
		case FVMENU_OPTIONS:		hMenu = FileViewMenu_GetOptionsMenu(hView, hViewMenu); break;
		case FVMENU_ARRANGEBY:	hMenu = FileViewMenu_GetArrangeByMenu(hView, hViewMenu); break;
		case FVMENU_COLUMNS:		hMenu = FileViewMenu_GetColumnsMenu(hView, hViewMenu); break;
		case FVMENU_PLAY:		hMenu = FileViewMenu_GetPlayMenu(hView, hViewMenu); break;
		case FVMENU_FILECONTEXT:		hMenu = FileViewMenu_GetFileMenu(hView, hViewMenu); break;
		case FVMENU_FILEVIEWCONTEXT:	hMenu = FileViewMenu_GetFileViewMenu(hView, hViewMenu); break;
		case FVMENU_FILEOPCONTEXT:	hMenu = FileViewMenu_GetFileOpMenu(hView, hViewMenu); break;
		case FVMENU_VIEWMODE:	hMenu = FileViewMenu_GetViewModeMenu(hView, hViewMenu); break;
	}
	return hMenu;
}

UINT FileViewMenu_GetMenuType(HWND hView, HMENU hViewMenu, HMENU hMenu)
{
	UINT type =	((UINT)-1);
	if (hMenu == GetSubMenu(hViewMenu, SUBMENU_VIEWMODE)) type = FVMENU_VIEWMODE;
	else if (hMenu == GetSubMenu(hViewMenu, SUBMENU_OPTIONS)) type = FVMENU_OPTIONS;
	else if (hMenu == GetSubMenu(hViewMenu, SUBMENU_ARRANGEBY)) type = FVMENU_ARRANGEBY;
	else if (hMenu == GetSubMenu(hViewMenu, SUBMENU_COLUMNS)) type = FVMENU_COLUMNS;
	else if (hMenu == GetSubMenu(GetSubMenu(hViewMenu, SUBMENU_FILELIST), SUBMENU_FILELIST_PLAY)) type = FVMENU_PLAY;
	else if (hMenu == GetSubMenu(GetSubMenu(hViewMenu, SUBMENU_FILELIST), SUBMENU_FILELIST_VIEWCONTEXT)) type = FVMENU_FILEVIEWCONTEXT;
	else if (hMenu == GetSubMenu(GetSubMenu(hViewMenu, SUBMENU_FILELIST), SUBMENU_FILELIST_OPCONTEXT)) type = FVMENU_FILEOPCONTEXT;

	return type;
}