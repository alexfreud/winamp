#include "main.h"
#include "./toolbarStatic.h"
#include "./toolbar.h"
#include <strsafe.h>

#define SPACER_WIDTH_UNITS	6
#define SPACER_WIDTH_PX		8

ToolbarStatic::ToolbarStatic(LPCSTR pszName, UINT nStyle, INT nIcon, LPCWSTR pszText, LPCWSTR pszDescription) :
	ToolbarItem(pszName, nStyle, nIcon, pszText, pszDescription), spaceWidth(SPACER_WIDTH_PX)
{

}

ToolbarItem* CALLBACK ToolbarStatic::CreateInstance(ToolbarItem::Template *item)
{
	if (NULL == item) 
		return NULL;

	return new ToolbarStatic( (NULL != item->name) ? item->name : TOOLCLS_STATIC,
							(item->style | styleStatic),
							item->iconId,
							item->text, 
							item->description);
}

BOOL ToolbarStatic::AdjustRect(HWND hToolbar, RECT *proposedRect)
{	
	if (0 != (styleSpacer & style))
	{
		if (0 == (styleFlexible & style) || 
			(proposedRect->right - proposedRect->left) < spaceWidth)
		{
			proposedRect->right = proposedRect->left + spaceWidth;
		}
		return TRUE;
	}
	
	if (0 != (styleSeparator & style))
	{
		SIZE iconSize;
		if (!Toolbar_GetIconSize(hToolbar, iconId, &iconSize))
			ZeroMemory(&iconSize, sizeof(SIZE));

		if (0 == (styleFlexible & style) || 
			(proposedRect->right - proposedRect->left) < iconSize.cx)
		{
			proposedRect->right = proposedRect->left + iconSize.cx;
		}

		proposedRect->top += ((proposedRect->bottom - proposedRect->top) - iconSize.cy)/2;
		proposedRect->bottom = proposedRect->top + iconSize.cy;
		return TRUE;
	}

	return FALSE;
}

BOOL ToolbarStatic::Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state)
{
	if (0 != (styleSpacer & style))
		return FALSE;
	

	if (0 != (styleSeparator & style))
	{
		TOOLBARDRAWICONPARAM param;
		param.hdcDst = hdc;
		param.iconIndex = iconId;
		param.x = rect.left;
		param.y = rect.top;
		param.cx = rect.right - rect.left;
		param.cy = rect.bottom - rect.top;
		param.itemState = state;
		return Toolbar_DrawIcon(hToolbar, &param);
	}

	return FALSE;
}

INT ToolbarStatic::GetTip(LPTSTR pszBuffer, INT cchBufferMax)
{
	return 0;
}

void ToolbarStatic::UpdateSkin(HWND hToolbar)
{
	spaceWidth  = SPACER_WIDTH_PX;
	HDC hdc = GetDCEx(hToolbar, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{	
		HFONT font = (HFONT)SendMessage(hToolbar, WM_GETFONT, 0, 0L);
		HFONT originalFont = (HFONT)SelectObject(hdc, font);

		TEXTMETRIC tm;
		if (GetTextMetrics(hdc, &tm))
		{
			spaceWidth = MulDiv(SPACER_WIDTH_UNITS, tm.tmAveCharWidth, 4);
		}
		
		SelectObject(hdc, originalFont);
		ReleaseDC(hToolbar, hdc);
	}
}

BOOL ToolbarStatic::FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax)
{
	if (0 != ((styleSpacer | styleSeparator) & style))
	{
		pmii->fMask = MIIM_FTYPE;
		pmii->fType = MFT_MENUBREAK;
		return TRUE;
	}
	return FALSE;
}
