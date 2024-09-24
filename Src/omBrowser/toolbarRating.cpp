#include "main.h"
#include "./toolbarRating.h"
#include "./toolbar.h"
#include "./graphics.h"
#include "./resource.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedrating.h"
#include "./menu.h"
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"
#include <strsafe.h>

#define RATING_SPACECX_PX		4
#define RATING_SPACECX_UNIT		2

#define RATING_STARSTYLE			(RDS_LEFT | RDS_TOP)

	BYTE rating;
	BYTE highlighted;
	BYTE focused;
	RECT ratingRect;
	RECT textRect;
	INT baseLine;
	ifc_skinnedrating *skinnedRating;

ToolbarRating::ToolbarRating(LPCSTR pszName, UINT nStyle, LPCWSTR pszText, LPCWSTR pszDescription) :
	ToolbarItem(pszName, nStyle, ICON_NONE, pszText, pszDescription), 
		rating(0), highlighted(0), focused(0), baseLine(0), skinnedRating(NULL)
{
	ifc_skinhelper *skinHelper;
	if (SUCCEEDED(Plugin_GetSkinHelper(&skinHelper)))
	{
		if (FAILED(skinHelper->QueryInterface(IFC_SkinnedRating, (void**)&skinnedRating)))
			skinnedRating = NULL;

		skinHelper->Release();
	}
}

ToolbarRating::~ToolbarRating()
{
	if (NULL != skinnedRating)
		skinnedRating->Release();
}

ToolbarItem* CALLBACK ToolbarRating::CreateInstance(ToolbarItem::Template *item)
{
	if (NULL == item) 
		return NULL;

	return new ToolbarRating( (NULL != item->name) ? item->name : TOOLCLS_RATING,
							item->style,
							item->text, 
							item->description);

}
static BOOL ToolbarRating_GetTextSize(LPCWSTR pszText, HWND hToolbar, SIZE *textSize)
{
	BOOL result = FALSE;
	WCHAR szText[64] = {0};

	if (IS_INTRESOURCE(pszText))
	{   
		Plugin_LoadString((INT)(INT_PTR)pszText, szText, ARRAYSIZE(szText));
		pszText = szText;
	}
	
	INT cchText = (NULL != pszText) ? lstrlenW(pszText) : 0;
	
	if (0 != cchText)
	{
		HDC hdc = GetDCEx(hToolbar, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			HFONT font = (HFONT)SendMessage(hToolbar, WM_GETFONT, 0, 0L);
			HFONT originalFont = (HFONT)SelectObject(hdc, font);
			result = GetTextExtentPoint32(hdc, pszText, cchText, textSize);
			SelectObject(hdc, originalFont); 
			ReleaseDC(hToolbar, hdc);
		}
	}

	return result;
}

BOOL ToolbarRating::AdjustRect(HWND hToolbar, RECT *proposedRect)
{
	TOOLBARTEXTMETRIC ttm;

	if (NULL == skinnedRating || FAILED(skinnedRating->CalcMinRect(5, &ratingRect)))
		::SetRectEmpty(&ratingRect);

	if (!Toolbar_GetTextMetrics(hToolbar, &ttm))
			ZeroMemory(&ttm, sizeof(TOOLBARTEXTMETRIC));

	::SetRectEmpty(&textRect);
	ToolbarRating_GetTextSize(text, hToolbar, ((SIZE*)&textRect) + 1);

	INT spaceCX = MulDiv(RATING_SPACECX_UNIT, ttm.aveCharWidth, 4);

	LONG cx = (ratingRect.right - ratingRect.left) + 2*spaceCX + (textRect.right - textRect.left) + 4;

	::OffsetRect(&textRect, spaceCX, ttm.origY - proposedRect->top);
	baseLine = ttm.baseY;

	::OffsetRect(&ratingRect, 
		spaceCX + (textRect.right - textRect.left) + 4, 
		(ttm.origY - proposedRect->top)  + baseLine -  (ratingRect.bottom - ratingRect.top) + 1);

	proposedRect->right = proposedRect->left + cx;
	return TRUE;
}


BOOL ToolbarRating::Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state)
{
	RECT cotrolRect;
	CopyRect(&cotrolRect, &ratingRect);
	::OffsetRect(&cotrolRect, rect.left, rect.top);

	INT trackingVal = (0 != (stateHighlighted & style)) ? highlighted : rating;
		
	UINT fStyle = RATING_STARSTYLE | RDS_OPAQUE;
	if (0 == rating || 0 != ((stateFocused | stateHighlighted) & state))
		fStyle |= RDS_SHOWEMPTY;
	
	if (0 == (stateDisabled & style))
		fStyle |= RDS_HOT;

	HRGN rgn = CreateRectRgnIndirect(&rect);
	HRGN rgn2 = CreateRectRgnIndirect(&textRect);
	OffsetRgn(rgn2, rect.left, rect.top);
	CombineRgn(rgn, rgn, rgn2, RGN_DIFF);
	SetRectRgn(rgn2, cotrolRect.left, cotrolRect.top, cotrolRect.right, cotrolRect.bottom);
	CombineRgn(rgn, rgn, rgn2, RGN_DIFF);
	HBRUSH hb = Toolbar_GetBkBrush(hToolbar);
	FillRgn(hdc, rgn, hb);
	DeleteObject(rgn);
	DeleteObject(rgn2);

	if (!::IsRectEmpty(&textRect))
	{
		WCHAR szText[64], *pszText(text);
		
		if (IS_INTRESOURCE(pszText))
		{
			Plugin_LoadString((INT)(INT_PTR)pszText, szText, ARRAYSIZE(szText));
			pszText = szText;
		}
		
		INT cchText = lstrlenW(pszText);
		if (0 != cchText)
		{
			UINT originalAlign = SetTextAlign(hdc, TA_LEFT | TA_BASELINE);
			RECT rc;
			CopyRect(&rc, &textRect);
			::OffsetRect(&rc, rect.left, rect.top);
			ExtTextOut(hdc, rc.left, rc.top + baseLine, ETO_OPAQUE, &rc, pszText, cchText, NULL);

			if ((TA_LEFT | TA_BASELINE) != originalAlign) SetTextAlign(hdc, originalAlign);
		}
	}

	if (NULL == skinnedRating || FAILED(skinnedRating->Draw(hdc, 5, rating, trackingVal, &cotrolRect, fStyle)))
	{
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &cotrolRect, NULL, 0, NULL);
	}
	else
	{
		if (stateFocused == ((stateFocused | stateNoFocusRect) & state) && focused > 0 && focused < 6)
		{
			RECT focusRect;
			CopyRect(&focusRect, &cotrolRect);
			INT starWidth = (cotrolRect.right - cotrolRect.left)/5;
			
			focusRect.left += starWidth * (focused - 1);
			focusRect.right = focusRect.left + starWidth;
			InflateRect(&focusRect, 1, 1);

			COLORREF origBk = SetBkColor(hdc, 0x00000000);
			COLORREF origFg = SetTextColor(hdc, 0x00FFFFFF);
			DrawFocusRect(hdc, &focusRect);
			if (origBk != 0x00000000) SetBkColor(hdc, origBk);
			if (origFg != 0x00FFFFFF) SetTextColor(hdc, origFg);
		}
	}

	return TRUE;
}

void ToolbarRating::MouseMove(HWND hToolbar, UINT mouseFlags, POINT pt)
{
	BYTE tracking = 0;
	UINT state = 0;
	if (PtInItem(pt))
	{	
		state = stateHighlighted;

		RECT controlRect;
		CopyRect(&controlRect, &ratingRect);
		::OffsetRect(&controlRect, rect.left, rect.top);

		POINT ptTest = pt;
		ptTest.y = controlRect.top;

		LONG result;
		if (NULL == skinnedRating || FAILED(skinnedRating->HitTest(pt, 5, &controlRect, RATING_STARSTYLE, &result)))
			result = 0;
		
		UINT hitTest = HIWORD(result);
		if (0 != ((RHT_ONVALUE | RHT_ONVALUEABOVE | RHT_ONVALUEBELOW) & hitTest))
			tracking = (BYTE)LOWORD(result);
	}
	
	BOOL invalidate = FALSE;

	if (tracking != highlighted)
	{
		highlighted = tracking;
		invalidate = TRUE;
	}

	if ((stateHighlighted & style) != (stateHighlighted & state))
	{
		style |= ((style & ~stateHighlighted) | state);
		invalidate = TRUE;
	}

	if (FALSE != invalidate)
	{
		RECT invalidRect;
		CopyRect(&invalidRect, &ratingRect);
		::OffsetRect(&invalidRect, rect.left, rect.top);
		InvalidateRect(hToolbar, &invalidRect, FALSE);
		Toolbar_UpdateTip(hToolbar);
	}
}

void ToolbarRating::MouseLeave(HWND hToolbar)
{
	BOOL invalidate = FALSE;
	if (highlighted != 0)
	{
		highlighted = 0;
		invalidate = TRUE;
	}

	if (0 != (stateHighlighted & style))
	{
		style &= ~stateHighlighted;
		invalidate = TRUE;
	}

	if (FALSE != invalidate)
	{
		RECT invalidRect;
		CopyRect(&invalidRect, &ratingRect);
		::OffsetRect(&invalidRect, rect.left, rect.top);
		InvalidateRect(hToolbar, &invalidRect, FALSE);
	}
}
void ToolbarRating::LButtonDown(HWND hToolbar, UINT mouseFlags, POINT pt)
{
	style |= statePressed;
}
void ToolbarRating::LButtonUp(HWND hToolbar, UINT mouseFlags, POINT pt)
{	
	style &= ~statePressed;
}

void ToolbarRating::Click(HWND hToolbar, UINT mouseFlags, POINT pt)
{
	if (0 == (ToolbarItem::statePressed & style))
		return;
		
	RECT controlRect;
	CopyRect(&controlRect, &ratingRect);
	::OffsetRect(&controlRect, rect.left, rect.top);

	POINT ptTest = pt;
	ptTest.y = controlRect.top;

	LONG result;
	if (NULL == skinnedRating || FAILED(skinnedRating->HitTest(pt, 5, &controlRect, RATING_STARSTYLE, &result)))
		result = 0;
		
	if (0 != ((RHT_ONVALUE | RHT_ONVALUEABOVE | RHT_ONVALUEBELOW) & HIWORD(result)) && LOWORD(result) > 0)
	{
		SendRating(hToolbar, LOWORD(result));
	}
}

void ToolbarRating::SendRating(HWND hToolbar, INT ratingValue)
{
	INT commandId = 0;
	switch(ratingValue)
	{
		case 1: commandId = ID_RATING_VALUE_1; break;
		case 2: commandId = ID_RATING_VALUE_2; break;
		case 3: commandId = ID_RATING_VALUE_3; break;
		case 4: commandId = ID_RATING_VALUE_4; break;
		case 5: commandId = ID_RATING_VALUE_5; break;
	}
	if (0 != commandId)
		Toolbar_SendCommand(hToolbar, commandId);

}
void ToolbarRating::UpdateSkin(HWND hToolbar)
{	

}

BOOL ToolbarRating::PtInItem(POINT pt)
{
	return (pt.x >= (rect.left + textRect.left) && pt.x < (rect.left + ratingRect.right) && rect.bottom != rect.top);
}

BOOL ToolbarRating::SetValueInt(HWND hToolbar, INT value)
{
	if (value < 0) value = 0;
	if (value > 5) value = 5;
	if (rating != value)
	{
		rating = value;

		RECT invalidRect;
		CopyRect(&invalidRect, &ratingRect);
		::OffsetRect(&invalidRect, rect.left, rect.top);
		InvalidateRect(hToolbar, &invalidRect, FALSE);
		
	}
	return TRUE;
}

static LPCWSTR ToolbarRating_FormatRating(INT ratingValue, LPWSTR pszBuffer, INT cchBufferMax)
{
	INT stringId;
	switch(ratingValue)
	{
		case 5: stringId = IDS_RATING_5; break;
		case 4: stringId = IDS_RATING_4; break;
		case 3: stringId = IDS_RATING_3; break;
		case 2: stringId = IDS_RATING_2; break;
		case 1: stringId = IDS_RATING_1; break;
		default: stringId = IDS_RATING_0; break;
	}
	
	Plugin_LoadString(stringId, pszBuffer, cchBufferMax);
	return pszBuffer;
}

INT ToolbarRating::GetTip(LPTSTR pszBuffer, INT cchBufferMax)
{
	WCHAR szText[64] = {0}, szRated[32] = {0};
	size_t remaining = 0;
	LPWSTR cursor;
	
	Plugin_LoadString(IDS_RATING_CURRENT, szText, ARRAYSIZE(szText));
	ToolbarRating_FormatRating(rating, szRated, ARRAYSIZE(szRated));

	HRESULT hr = StringCchPrintfEx(pszBuffer, cchBufferMax, &cursor, &remaining, STRSAFE_IGNORE_NULLS, TEXT("%s  %s "), szText, szRated);
	if (FAILED(hr)) return 0;

	if (NULL != highlighted)
	{
		Plugin_LoadString(IDS_RATING_CHANGETO, szText, ARRAYSIZE(szText));
		ToolbarRating_FormatRating(highlighted, szRated, ARRAYSIZE(szRated));
		hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_IGNORE_NULLS, TEXT("\r\n%s  %s "), szText, szRated);
		if (FAILED(hr)) return 0;
	}
	
	return cchBufferMax - (INT)remaining;
	
}

BOOL ToolbarRating::FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax)
{
	pmii->fMask = MIIM_STRING | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
	pmii->wID = 0;
	pmii->fState = MFS_ENABLED;
	pmii->dwTypeData = pszBuffer;
	pmii->hSubMenu = Menu_GetMenu(MENU_RATING, RATINGTOMCF(rating));

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

void ToolbarRating::SetFocus(HWND hToolbar, ToolbarItem *focusItem, BOOL fSet)
{
	
	if (FALSE != fSet)
	{
		focused = 1;
		if (NULL != focusItem) 
		{
			INT mineIndex = Toolbar_FindItem(hToolbar, name);
			INT otherIndex = Toolbar_FindItem(hToolbar, focusItem->GetName());
			if (ITEM_ERR != mineIndex && 
				ITEM_ERR != otherIndex &&
				otherIndex > mineIndex)
			{
				focused = 5;
			}
		}
	}
	else
	{
		focused = 0;	
	}
	InvalidateRect(hToolbar, &rect, FALSE);
}

BOOL ToolbarRating::KeyDown(HWND hToolbar, INT vKey, UINT flags)
{
	switch(vKey)
	{		
		case VK_LEFT:
			if (focused > 1) 
			{
				focused--;
				InvalidateRect(hToolbar, &rect, FALSE);
				return TRUE;
			}
			break;
		case VK_RIGHT:
			if (focused < 5) 
			{
				focused++;
				InvalidateRect(hToolbar, &rect, FALSE);
				return TRUE;
			}
			break;
		case VK_SPACE:
		case VK_RETURN:
			SetStyle(hToolbar, statePressed, statePressed);
			return TRUE;
	}
	return FALSE;
}
BOOL ToolbarRating::KeyUp(HWND hToolbar, INT vKey, UINT flags)
{
	switch(vKey)
	{		
		case VK_SPACE:
		case VK_RETURN:
			if (0 != (statePressed & style) && 0 != focused)
			{
				SetStyle(hToolbar, 0, statePressed);
				SendRating(hToolbar, focused);
			}
			return TRUE;
	}
	return FALSE;
}