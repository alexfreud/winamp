#include "main.h"
#include "./toolbarButton.h"
#include "./toolbar.h"
#include "./graphics.h"
#include "./resource.h"
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"
#include <strsafe.h>

#define CMDLINK_SPACECX_UNITS		2

ToolbarButton::ToolbarButton(LPCSTR  pszName, INT nCommand, UINT nStyle, INT nIcon, LPCWSTR pszText, LPCWSTR pszDescription) :
	ToolbarItem(pszName, nStyle, nIcon, pszText, pszDescription),
	commandId(nCommand), offsetX(0), offsetY(0), rgbText(0), rgbHilite(0)
{
}

ToolbarItem* CALLBACK ToolbarButton::CreateInstance(ToolbarItem::Template *item)
{
	if (NULL == item) 
		return NULL;

	return new ToolbarButton( (NULL != item->name) ? item->name : TOOLCLS_BUTTON,
							item->commandId,
							item->style,
							item->iconId,
							item->text, 
							item->description);
}

static LPCTSTR ToolbarButton_GetResourceString(LPCTSTR pszResource, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (IS_INTRESOURCE(pszResource))
	{		
        Plugin_LoadString((INT)(INT_PTR)pszResource, pszBuffer, cchBufferMax);
		return pszBuffer;
	}
	return pszResource;	
}

static BOOL ToolbarButton_GetTextSise(HWND hToolbar, LPCWSTR pszText, INT cchText, SIZE *textSize, TEXTMETRIC *ptm)
{
	if (NULL == textSize)
	return FALSE;

	if (cchText < 0)
		cchText = lstrlenW(pszText);

	HDC hdc = GetDCEx(hToolbar, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return FALSE;

	BOOL result = FALSE;
	HFONT font = (HFONT)SendMessage(hToolbar, WM_GETFONT, 0, 0L);
	HFONT originalFont = (HFONT)SelectObject(hdc, font);

	if (NULL != ptm && !GetTextMetrics(hdc, ptm))
		ZeroMemory(ptm, sizeof(TEXTMETRIC));

	if (0 != cchText)
	{
		if (GetTextExtentPoint32(hdc, pszText, cchText, textSize))
			result = TRUE;
	}
	else
		ZeroMemory(textSize, sizeof(SIZE));

	SelectObject(hdc, originalFont);
	ReleaseDC(hToolbar, hdc);
	return result;
}

BOOL ToolbarButton::AdjustRect(HWND hToolbar, RECT *proposedRect)
{
	if (0 != (ToolbarButton::styleCommandLink & style))
	{
		SIZE linkSize = {0};
		LPCTSTR pszText;
		WCHAR szBuffer[128] = {0};
		TOOLBARTEXTMETRIC ttm = {0};

		pszText = ToolbarButton_GetResourceString(text, szBuffer, ARRAYSIZE(szBuffer));
		if (!ToolbarButton_GetTextSise(hToolbar, pszText, -1, &linkSize, NULL))
			ZeroMemory(&linkSize, sizeof(SIZE)); 

		if (!Toolbar_GetTextMetrics(hToolbar, &ttm))
			ZeroMemory(&ttm, sizeof(TOOLBARTEXTMETRIC));

		offsetX = MulDiv(CMDLINK_SPACECX_UNITS, ttm.aveCharWidth, 4);
		offsetY = ttm.baseY;
		proposedRect->right = proposedRect->left + linkSize.cx + 2 * offsetX + ttm.overhang;

		proposedRect->top = ttm.origY;
		proposedRect->bottom = proposedRect->top + ttm.height;
		return TRUE;
	}

	SIZE iconSize;
	if (!Toolbar_GetIconSize(hToolbar, iconId, &iconSize))
		ZeroMemory(&iconSize, sizeof(SIZE));

	proposedRect->right = proposedRect->left + iconSize.cx;
	INT offsetY = (proposedRect->bottom - proposedRect->top) - iconSize.cy;
	if (0 != offsetY)
	{
		proposedRect->top += offsetY/2;
		if (0 != (offsetY%2)) proposedRect->top++;
	}
	proposedRect->bottom = proposedRect->top + iconSize.cy;
	return TRUE;
}

void ToolbarButton::UpdateSkin(HWND hToolbar)
{
	rgbText = Toolbar_GetTextColor(hToolbar);
	rgbHilite = Toolbar_GetHiliteColor(hToolbar);
}

BOOL ToolbarButton::Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state)
{
	if (statePressed == ((statePressed | stateHighlighted) & state))
		state &= ~statePressed;

	if (0 != (ToolbarButton::styleCommandLink & style))
	{		
		LPCTSTR pszText;
		WCHAR szBuffer[256] = {0};
		pszText = ToolbarButton_GetResourceString(text, szBuffer, ARRAYSIZE(szBuffer));
		INT cchText = lstrlen(pszText);
		if (0 == cchText)
			return FALSE;

		
		COLORREF originalFg = GetTextColor(hdc);
		COLORREF rgbFg = (0 == (stateHighlighted & style)) ? rgbText : rgbHilite;
				
		SetTextColor(hdc, rgbFg);
		INT originalMode = SetBkMode(hdc, TRANSPARENT);
		UINT originalAlign = SetTextAlign(hdc, TA_LEFT | TA_BASELINE);
		
		ExtTextOut(hdc, rect.left + offsetX, rect.top + offsetY, ETO_CLIPPED | ETO_OPAQUE, &rect, pszText, cchText, NULL);
		
		if (TRANSPARENT != originalMode) SetBkMode(hdc, originalMode);
		if ((TA_LEFT | TA_BASELINE) != originalAlign) SetTextAlign(hdc, originalAlign);

		if (0 != (stateHighlighted & style))
		{			
			COLORREF originalBk = SetBkColor(hdc, rgbFg);
			
			RECT partRect;
			::SetRect(&partRect, rect.left + offsetX, rect.top + offsetY + 1, rect.right - offsetX, rect.top + offsetY + 2);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &partRect, NULL, 0, NULL);
			
			SetBkColor(hdc, originalBk);
		}
		
		if (rgbFg != originalFg) 
			SetTextColor(hdc, originalFg);

		return TRUE;
	}

	TOOLBARDRAWICONPARAM param;
	param.hdcDst = hdc;
	param.iconIndex = iconId;
	param.x = rect.left;
	param.y = rect.top;
	param.cx = rect.right - rect.left;
	param.cy = rect.bottom - rect.top;
	param.itemState = state;
	if (Toolbar_DrawIcon(hToolbar, &param))
	{
		if (stateFocused == ((stateFocused | stateNoFocusRect) & state))
		{
			RECT focusRect;
			CopyRect(&focusRect, &rect);
			focusRect.left += 1;
			focusRect.top += 2;
			focusRect.bottom -= 1;
			COLORREF origBk = SetBkColor(hdc, 0x00000000);
			COLORREF origFg = SetTextColor(hdc, 0x00FFFFFF);
			DrawFocusRect(hdc, &focusRect);
			if (origBk != 0x00000000) SetBkColor(hdc, origBk);
			if (origFg != 0x00FFFFFF) SetTextColor(hdc, origFg);
		}
		return TRUE;
	}
	
	return FALSE;
}

INT ToolbarButton::GetTip(LPTSTR pszBuffer, INT cchBufferMax)
{
	if (IS_INTRESOURCE(description))
	{
		if (NULL == pszBuffer || cchBufferMax < 1) 
			return 0;
        Plugin_LoadString((INT)(INT_PTR)description, pszBuffer, cchBufferMax);
		return lstrlen(pszBuffer);
	}
	
	size_t remaining;
	HRESULT hr = StringCchCopyEx(pszBuffer, cchBufferMax, description, NULL, &remaining, STRSAFE_IGNORE_NULLS);
	return SUCCEEDED(hr) ? (cchBufferMax - (INT)remaining) : 0;
	
}

BOOL ToolbarButton::PtInItem(POINT pt)
{
	return (pt.x >= (rect.left + offsetX) && pt.x < (rect.right - offsetX) && rect.bottom != rect.top);
}
INT ToolbarButton::GetCommandId() 
{ 
	return commandId; 
}
void ToolbarButton::MouseMove(HWND hToolbar, UINT mouseFlags, POINT pt)
{
	UINT styleNew = (PtInItem(pt)) ? stateHighlighted  : 0;
	SetStyle(hToolbar, styleNew, stateHighlighted);
	
}

void ToolbarButton::MouseLeave(HWND hToolbar)
{
	if (0 != (stateHighlighted & style))
	{
		SetStyle(hToolbar, 0, stateHighlighted);
	}
}

void ToolbarButton::LButtonDown(HWND hToolbar, UINT mouseFlags, POINT pt)
{
	SetStyle(hToolbar, statePressed, statePressed);
}
void ToolbarButton::LButtonUp(HWND hToolbar, UINT mouseFlags, POINT pt)
{
	SetStyle(hToolbar, 0, statePressed);
}
void ToolbarButton::Click(HWND hToolbar, UINT mouseFlags, POINT pt)
{			
	if (0 != commandId)
	{
		Toolbar_SendCommand(hToolbar, commandId);
	}
}

BOOL ToolbarButton::FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax)
{
	pmii->fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
	pmii->wID = commandId;
	pmii->fState = MFS_UNHILITE;
	pmii->fState |= ((0 == (stateDisabled & style)) ? MFS_ENABLED : MFS_DISABLED);
	pmii->dwTypeData = pszBuffer;

	if (IS_INTRESOURCE(text))
	{			
        Plugin_LoadString((INT)(INT_PTR)text, pszBuffer, cchBufferMax);
	}
	else
	{
		if (FAILED(StringCchCopyEx(pszBuffer, cchBufferMax, text, NULL, NULL, STRSAFE_IGNORE_NULLS)))
			pszBuffer[0] = L'\0';
	}
	return TRUE;
}




BOOL ToolbarButton::KeyDown(HWND hToolbar, INT vKey, UINT flags)
{
	switch(vKey)
	{		
		case VK_SPACE:
		case VK_RETURN:
			SetStyle(hToolbar, statePressed | stateHighlighted, statePressed | stateHighlighted);
			return TRUE;
	}
	return FALSE;
}

BOOL ToolbarButton::KeyUp(HWND hToolbar, INT vKey, UINT flags)
{
	switch(vKey)
	{
		case VK_SPACE:
		case VK_RETURN:
			if (0 != (statePressed & style))
			{
				SetStyle(hToolbar, 0, (statePressed | stateHighlighted));
				if (0 != commandId)
					Toolbar_SendCommand(hToolbar, commandId);
			}
			return TRUE;
	}
	return FALSE;
}

void ToolbarButton::SetFocus(HWND hToolbar, ToolbarItem *focusItem, BOOL fSet)
{
	InvalidateRect(hToolbar, &rect, FALSE);
}
