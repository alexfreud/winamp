#include "main.h"
#include "./toolbarAddress.h"
#include "./toolbar.h"
#include "./toolbarEditbox.h"
#include "./resource.h"
#include "./graphics.h"
#include "./menu.h"
#include "./ifc_imageloader.h"
#include "./addressEncoder.h"


#include <wininet.h>
//#include <shlwapi.h>
#include <strsafe.h>

#define ADDRESSBAR_MINWIDTH		96

	UINT minWidth;
	HBITMAP bitmap;
	INT iconHeight;
	INT iconWidth;
	INT textHeight;
	INT textMargin;
	COLORREF rgbText;
	COLORREF rgbBk;
	HWND hEditor;

ToolbarAddress::ToolbarAddress(LPCSTR pszName, UINT nStyle) :
	ToolbarItem(pszName, nStyle, ICON_NONE, NULL, NULL), 
	minWidth(ADDRESSBAR_MINWIDTH), bitmap(NULL), iconHeight(0), iconWidth(0),
	textHeight(12), textMargin(0), rgbText(0x000000), rgbBk(0xFFFFFF), hEditor(NULL)
{
}

ToolbarAddress::~ToolbarAddress()
{
	if (NULL != bitmap)
		DeleteObject(bitmap);
	if (NULL != hEditor)
		DestroyWindow(hEditor);
}

ToolbarItem* CALLBACK ToolbarAddress::CreateInstance(ToolbarItem::Template *item)
{
	if (NULL == item) 
		return NULL;

	return new ToolbarAddress( (NULL != item->name) ? item->name : TOOLCLS_ADDRESSBAR,
								item->style);
}

BOOL ToolbarAddress::ActivateEditor(HWND hToolbar, const POINT *ppt)
{
	if (NULL == hEditor)
	{
		UINT editorStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP |
							ES_LEFT | ES_NOHIDESEL | ES_AUTOHSCROLL;
		
		if (0 != (styleAddressReadonly & style))
			editorStyle |= ES_READONLY;
	
		hEditor = CreateWindowEx(WS_EX_NOPARENTNOTIFY, L"Edit", text, editorStyle, 0, 0, 0, 0, hToolbar, NULL, NULL, 0);

		if (NULL == hEditor)
			return FALSE;
		
		ToolbarEditbox_AttachWindow(hEditor, this);

		HFONT hFont = (HFONT)SendMessage(hToolbar, WM_GETFONT, 0, 0L);
		SendMessage(hEditor, WM_SETFONT, (WPARAM)hFont, 0L);
		SendMessage(hEditor, EM_SETMARGINS, (WPARAM)(EC_LEFTMARGIN | EC_RIGHTMARGIN), MAKELPARAM(textMargin, textMargin));
	}

	UINT editorStyle = GetWindowStyle(hEditor);
	if (0 == (WS_VISIBLE & editorStyle))
	{
		RECT textRect;
		CopyRect(&textRect, &rect);
		InflateRect(&textRect, -((iconWidth -1)/2), -((iconHeight -1)/2));
		textRect.top = textRect.bottom - textHeight;

		SetWindowPos(hEditor, NULL, textRect.left, textRect.top, 
				textRect.right - textRect.left, textRect.bottom - textRect.top, 
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
		
		UINT editorStyle = GetWindowStyle(hEditor);
		if (0 == (WS_VISIBLE & editorStyle))
			SetWindowLongPtr(hEditor, GWL_STYLE, editorStyle | WS_VISIBLE);
		
	}

	if (IsWindowVisible(hEditor))
	{		
		if (hEditor != GetFocus())
			::SetFocus(hEditor);

		if (NULL != ppt)
		{
			POINT editorPt = *ppt;
			MapWindowPoints(hToolbar, hEditor, &editorPt, 1);
			PostMessage(hEditor, WM_LBUTTONDOWN, (WPARAM)(MK_LBUTTON), MAKELPARAM(editorPt.x, editorPt.y));
		}
		else
		{
			SendMessage(hEditor, NTEBM_SELECTALL, 0, 0);
		}
		
		InvalidateRect(hEditor, NULL, FALSE);
	}

	InvalidateRect(hToolbar, &rect, FALSE);
	return TRUE;
}


UINT ToolbarAddress::GetStyle()
{
	UINT filteredStyle = style;
	if (0 != (styleAddressReadonly & filteredStyle))
		filteredStyle &= ~styleTabstop;

	return filteredStyle;
}

void ToolbarAddress::SetStyle(HWND hToolbar, UINT newStyle, UINT styleMask)
{
	ToolbarItem::SetStyle(hToolbar, newStyle, styleMask);

	if (NULL != hEditor)
	{
		if (0 != (stateHidden & styleMask) && 0 != (stateHidden & style))
		{
			if (GetFocus() == hEditor)
				EditboxNavigateNextCtrl(hEditor, TRUE);
			else
				DestroyWindow(hEditor);
		}
	
		if (0 != (styleAddressReadonly & styleMask))
		{
			UINT editorStyle = GetWindowStyle(hEditor);
			if (0 == (styleAddressReadonly & style) != 0 == (ES_READONLY & editorStyle))
				SendMessage(hEditor, EM_SETREADONLY, (WPARAM)(0 != (styleAddressReadonly & style)), 0L);
		}
	}
}

BOOL ToolbarAddress::SetRect(const RECT *prc)
{
	BOOL result = ToolbarItem::SetRect(prc);
	if (FALSE == result) return result;
	
	if (NULL != hEditor)
	{
		UINT editorStyle = GetWindowStyle(hEditor);
		if (0 != (WS_VISIBLE & editorStyle))
		{
			RECT textRect;
			CopyRect(&textRect, &rect);
			InflateRect(&textRect, -((iconWidth -1)/2), -((iconHeight -1)/2));
			textRect.top = textRect.bottom - textHeight;
			
			SetWindowPos(hEditor, NULL, textRect.left, textRect.top, 
					textRect.right - textRect.left, textRect.bottom - textRect.top, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
		}
	}

	return result;
}

BOOL ToolbarAddress::SetRectEmpty()
{
	BOOL result = ToolbarItem::SetRectEmpty();
	if (FALSE == result) return result;


	if (NULL != hEditor)
	{
		if (GetFocus() == hEditor)
			EditboxNavigateNextCtrl(hEditor, TRUE);
		else
			DestroyWindow(hEditor);
	}

	return result;
}

BOOL ToolbarAddress::SetDescription(HWND hToolbar, LPCWSTR pszDescription)
{
	if (NULL != pszDescription && FALSE == IS_INTRESOURCE(pszDescription))
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, L"about:blank", -1, pszDescription, -1))
			pszDescription = NULL;
	}
	
	BOOL result = ToolbarItem::SetDescription(hToolbar, pszDescription);
	if (NULL != hToolbar)
		InvalidateRect(hToolbar, &rect, TRUE);
	return result;
}

BOOL ToolbarAddress::AdjustRect(HWND hToolbar, RECT *proposedRect)
{	
	if (0 == (styleFlexible & style) || 
		proposedRect->right < proposedRect->left ||
		((UINT)(proposedRect->right - proposedRect->left)) < minWidth)
	{
		proposedRect->right = proposedRect->left + minWidth;
	}
	//INT iconCY = Toolbar_GetImageListHeight(hToolbar);
	INT height = textHeight + iconHeight;
	INT t = ((proposedRect->bottom - proposedRect->top) - height);
	proposedRect->top += (t/2 + t%2);
	proposedRect->bottom = proposedRect->top + height;

	
	return TRUE;
}

BOOL ToolbarAddress::Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state)
{
	
	if (NULL == bitmap)
		return FALSE;
		
	RECT blitRect;
	if (!IntersectRect(&blitRect, paintRect))
		return TRUE;
	
	BOOL success = FALSE;
	HDC hdcSrc = CreateCompatibleDC(hdc);

	UINT editorStyle = (NULL != hEditor) ? GetWindowStyle(hEditor) : 0;
	INT offsetY;
	if (0 != (stateDisabled & state) || 0 != (styleAddressReadonly & style))
		offsetY = 2* iconHeight;
	else if (WS_VISIBLE == ((WS_VISIBLE | ES_READONLY | WS_DISABLED) & editorStyle))
		offsetY = 1 * iconHeight;
	else
		offsetY = 0;

	INT partCY = (iconHeight - 1)/2;
	INT partCX = (iconWidth - 1)/2;

	RECT textRect;
	CopyRect(&textRect, &rect);
	InflateRect(&textRect, -partCX, -partCY);

	INT clipRgnCode = -1;
	HRGN clipRgn = CreateRectRgn(0, 0, 0, 0);
	if (NULL != clipRgn)
	{
		clipRgnCode = GetClipRgn(hdc, clipRgn);
		ExcludeClipRect(hdc, textRect.left, textRect.top, textRect.right, textRect.bottom);
	}

	if (NULL != hdcSrc)
	{
		success = TRUE;
		HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcSrc, bitmap);

		// left-top
		BitBlt(hdc, rect.left, rect.top, partCX, partCY, hdcSrc, 0, offsetY + 0, SRCCOPY);
		// right-top
		BitBlt(hdc, rect.right - partCX, rect.top, partCX, partCY, hdcSrc, iconWidth - partCX, offsetY + 0, SRCCOPY);
		// right-bottom
		BitBlt(hdc, rect.right - partCX, rect.bottom - partCY, partCX, partCY, hdcSrc, iconWidth - partCX, offsetY + (iconHeight - partCY), SRCCOPY);
		// left-bottom
		BitBlt(hdc, rect.left, rect.bottom - partCY, partCX, partCY, hdcSrc, 0, offsetY + (iconHeight - partCY), SRCCOPY);
		
		INT stretchMode = SetStretchBltMode(hdc, COLORONCOLOR);

		StretchBlt(hdc, rect.left, rect.top + partCY, partCX, (rect.bottom - rect.top) - 2*partCY, hdcSrc, 0, offsetY + partCY, partCX, 1, SRCCOPY);
		StretchBlt(hdc, rect.right - partCX, rect.top + partCY, partCX, (rect.bottom - rect.top) - 2*partCY, hdcSrc, iconWidth - partCX, offsetY + partCY, partCX, 1, SRCCOPY);
		StretchBlt(hdc, rect.left + partCX, rect.top, (rect.right - rect.left) - 2 * partCX, partCY, hdcSrc, partCX, offsetY + 0, 1, partCY, SRCCOPY);
		StretchBlt(hdc, rect.left + partCX, rect.bottom - partCY, (rect.right - rect.left) - 2 * partCX, partCY, hdcSrc, partCX, offsetY + (iconHeight - partCY), 1, partCY, SRCCOPY);
		

		if (COLORONCOLOR != stretchMode)
				SetStretchBltMode(hdc, stretchMode);


		SelectObject(hdcSrc, hbmpOld);
		DeleteDC(hdcSrc);
	}
	
	if (clipRgnCode >= 0)
		SelectClipRgn(hdc, ( 0 != clipRgnCode) ? (clipRgn ? clipRgn : NULL) : NULL);
	
	if (NULL != clipRgn)
		DeleteObject(clipRgn);

	COLORREF rgbTextOrig, rgbBkOrig;
	
	if (0 == (stateDisabled & state) && 0 == (styleAddressReadonly & style))
	{
		rgbTextOrig = SetTextColor(hdc, rgbText);
		rgbBkOrig = SetBkColor(hdc, rgbBk);
	}
	else
	{
		rgbTextOrig = GetTextColor(hdc);
		rgbBkOrig = GetBkColor(hdc);
	}

	LPCWSTR pszText;
	pszText = ( 0 == (styleAddressShowReal & style) && NULL != description) ? description : text;

	
	UINT cchText = (NULL != pszText && FALSE == IS_INTRESOURCE(pszText)) ? lstrlen(pszText) : 0;
	INT textAlign = SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
	ExtTextOut(hdc, textRect.left + textMargin, textRect.bottom, ETO_OPAQUE | ETO_CLIPPED, &textRect, pszText, cchText, NULL);

	if (textAlign != (TA_LEFT | TA_BOTTOM)) SetTextAlign(hdc, textAlign);
	if (rgbText != rgbTextOrig) SetTextColor(hdc, rgbTextOrig);
	if (rgbBk != rgbBkOrig) SetBkColor(hdc, rgbBkOrig);

	
	
	return success;
}

INT ToolbarAddress::GetTip(LPTSTR pszBuffer, INT cchBufferMax)
{
	return 0;
}

HRESULT ToolbarAddress::GetText(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	if (0 == cchBufferMax) return E_OUTOFMEMORY;
	if (NULL == text)
	{
		*pszBuffer = L'\0';
		return S_OK;
	}

	if (IS_INTRESOURCE(text))
		return Plugin_CopyResString(pszBuffer, cchBufferMax, text);
	
	return AddressEncoder_EncodeString(text, pszBuffer,(size_t*)&cchBufferMax, ICU_BROWSER_MODE);
}

HRESULT ToolbarAddress::GetTextLength(size_t *pcchLength)
{
	if (NULL == pcchLength)
		return E_POINTER;

	if (NULL == text)
	{
		*pcchLength = 0;
		return S_OK;
	}

	if (IS_INTRESOURCE(text))
	{
		WCHAR szBuffer[8192] = {0};
		HRESULT hr = Plugin_CopyResString(szBuffer, ARRAYSIZE(szBuffer), text);
		*pcchLength = (FAILED(hr)) ? 0 : lstrlen(szBuffer);
		return hr;
	}

	WCHAR szBuffer[1] = {0};
	size_t cchBufferMax = ARRAYSIZE(szBuffer);
	HRESULT hr = AddressEncoder_EncodeString(text, szBuffer, &cchBufferMax, ICU_BROWSER_MODE);
	if (SUCCEEDED(hr) || ENC_E_INSUFFICIENT_BUFFER == hr)
	{
		if (0 != cchBufferMax) cchBufferMax--;
		*pcchLength = cchBufferMax;
		hr = S_OK;
	}
	else
	{
		hr = E_FAIL;
		*pcchLength = 0;

	}

	return hr;

}
void ToolbarAddress::MouseMove(HWND hToolbar, UINT mouseFlags, POINT pt)
{
//	UINT styleNew = (PtInItem(pt)) ? stateHighlighted  : 0;
//	SetStyle(hToolbar, styleNew, stateHighlighted);
	
}

void ToolbarAddress::MouseLeave(HWND hToolbar)
{
	if (0 != (stateHighlighted & style))
	{
		SetStyle(hToolbar, 0, stateHighlighted);
	}
}

void ToolbarAddress::LButtonDown(HWND hToolbar, UINT mouseFlags, POINT pt)
{
	ActivateEditor(hToolbar, &pt);
}

void ToolbarAddress::UpdateSkin(HWND hToolbar)
{
	if (NULL != bitmap)
	{
		DeleteObject(bitmap);
		bitmap = NULL;
	}
			
	iconHeight = 0;
	iconWidth = 0;

	COLORREF rgbToolBk = Toolbar_GetBkColor(hToolbar);
	rgbText = Toolbar_GetEditColor(hToolbar);
	rgbBk = Toolbar_GetEditBkColor(hToolbar);

	ifc_omimageloader *loader;
	if (SUCCEEDED(Plugin_QueryImageLoader(Plugin_GetInstance(), MAKEINTRESOURCE(IDR_TOOLBARADDRESS_IMAGE), TRUE, &loader)))
	{
		BITMAPINFOHEADER headerInfo;
		BYTE *pixelData;
		if (SUCCEEDED(loader->LoadBitmapEx(&bitmap, &headerInfo, (void**)&pixelData)))
		{
			if (headerInfo.biHeight < 0) headerInfo.biHeight = -headerInfo.biHeight;
			iconHeight = headerInfo.biHeight/3;
			iconWidth  = headerInfo.biWidth;
			
			Image_Colorize(pixelData, headerInfo.biWidth, headerInfo.biHeight, headerInfo.biBitCount, 
				rgbToolBk, Toolbar_GetFgColor(hToolbar), FALSE);
			
			// disabled
			Image_BlendOnColorEx(pixelData, headerInfo.biWidth, headerInfo.biHeight, 
					0, 0, headerInfo.biWidth, iconHeight, headerInfo.biBitCount, FALSE, rgbToolBk);
			// highlighted
			Image_BlendOnColorEx(pixelData, headerInfo.biWidth, headerInfo.biHeight, 
					0, 1*iconHeight, headerInfo.biWidth, iconHeight, headerInfo.biBitCount, FALSE, rgbBk);
			// normal
			Image_BlendOnColorEx(pixelData, headerInfo.biWidth, headerInfo.biHeight, 
					0, 2*iconHeight, headerInfo.biWidth, iconHeight, headerInfo.biBitCount, FALSE, rgbBk);

		}
		loader->Release();
	}

	textHeight = 12;
	HFONT toolbarFont = (HFONT)SendMessage(hToolbar, WM_GETFONT, 0, 0L);

	if (NULL != toolbarFont)
	{
		HDC hdc = GetDCEx(hToolbar, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{	
			HFONT originalFont = (HFONT)SelectObject(hdc, toolbarFont);

			TEXTMETRIC tm;
			if (GetTextMetrics(hdc, &tm))
			{
				textHeight = tm.tmHeight;
				textMargin = tm.tmAveCharWidth/2 + tm.tmAveCharWidth%2;
			}
			
			SelectObject(hdc, originalFont);
			ReleaseDC(hToolbar, hdc);
		}

		if (NULL != hEditor)
		{
			UINT editorStyle = GetWindowStyle(hEditor);
			if (0 != (WS_VISIBLE & editorStyle))
			{
				SendMessage(hEditor, WM_SETFONT, (WPARAM)toolbarFont, 0L);
				SendMessage(hEditor, EM_SETMARGINS, (WPARAM)(EC_LEFTMARGIN | EC_RIGHTMARGIN), MAKELPARAM(textMargin, textMargin));
			}
		}
	}
}

BOOL ToolbarAddress::FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax)
{
	return FALSE;
}
void ToolbarAddress::CommandSent(HWND hToolbar, INT commandId)
{
	if (NULL != hEditor && ID_ADDRESSBAR_CHANGED != commandId)
	{
		if (GetFocus() == hEditor)
			EditboxNavigateNextCtrl(hEditor, TRUE);
		else
			DestroyWindow(hEditor);
	}
}



BOOL ToolbarAddress::SetValueStr(HWND hToolbar, LPCWSTR value)
{
	Plugin_FreeResString(text);
	
	if (NULL != value)
	{
		if (FALSE != IS_INTRESOURCE(value) || FAILED(AddressEncoder_DecodeString(value, &text)))
			text = Plugin_DuplicateResString(value);
	}
	else
		text = NULL;
	
	RECT textRect;
	CopyRect(&textRect, &rect);
	InflateRect(&textRect, -((iconWidth -1)/2), -(iconHeight - 1)/2);

	InvalidateRect(hToolbar, &textRect, TRUE);

	return TRUE;
}

BOOL ToolbarAddress::SetClipboardText(HWND hToolbar, LPCWSTR pszText)
{
	if(FALSE == OpenClipboard(hToolbar))
		return FALSE;

	EmptyClipboard(); 
	
	INT cchLen;
	HGLOBAL hMemory;
	
	cchLen = (NULL != pszText) ? lstrlenW(pszText) : 0; 
	hMemory = (0 != cchLen) ? GlobalAlloc(GMEM_MOVEABLE, (cchLen + 1) * sizeof(WCHAR)) : NULL; 
	if (NULL != hMemory) 
	{ 
		LPWSTR pszDest = (LPWSTR)GlobalLock(hMemory); 
		CopyMemory(pszDest, pszText, (cchLen + 1) * sizeof(WCHAR));
		GlobalUnlock(hMemory); 	
	} 
	SetClipboardData(CF_UNICODETEXT, hMemory); 
	

	LPSTR pszTextAnsi = (NULL != pszText) ? Plugin_WideCharToMultiByte(CP_ACP, 0, pszText, -1, NULL, NULL) : NULL;
	cchLen = (NULL != pszTextAnsi) ? lstrlenA(pszTextAnsi) : 0; 
	hMemory = (0 != cchLen) ? GlobalAlloc(GMEM_MOVEABLE, (cchLen + 1) * sizeof(CHAR)) : NULL; 
	if (NULL != hMemory) 
	{ 
		LPWSTR pszDest = (LPWSTR)GlobalLock(hMemory); 
		CopyMemory(pszDest, pszTextAnsi, (cchLen + 1) * sizeof(CHAR));
		GlobalUnlock(hMemory); 	
	} 
	SetClipboardData(CF_TEXT, hMemory); 

	CloseClipboard(); 
	return TRUE;
}

BOOL ToolbarAddress::DisplayContextMenu(HWND hToolbar, INT x, INT y)
{
	HMENU hMenu = Menu_GetMenu(MENU_ADDRESSBAR, 0);
	if (NULL != hMenu)
	{	
		UINT fEnable = MF_BYCOMMAND;
		if (0 != (styleAddressReadonly & style))
			fEnable |= (MF_DISABLED | MF_GRAYED);
		else
			fEnable |= MF_ENABLED;

		EnableMenuItem(hMenu, ID_ADDRESSBAR_EDITADDRESS, fEnable);

		INT commandId = Menu_TrackPopup(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, x, y, hToolbar, NULL);
		switch(commandId)
		{
			case ID_ADDRESSBAR_COPY:
				if (0 == (styleAddressShowReal & style) && NULL != description)
				{
					UINT cchTextMax = (IS_INTRESOURCE(description)) ? 2048 : (lstrlen(description) + 1);
					LPWSTR pszText = Plugin_MallocString(cchTextMax);
					if (NULL != pszText && FAILED(GetDescription(pszText, cchTextMax)))
					{
						Plugin_FreeString(pszText);
						pszText = NULL;
					}
					SetClipboardText(hToolbar, pszText);
					Plugin_FreeString(pszText);
					break;
				}
				// let it go
			case ID_ADDRESSBAR_COPYADDRESS:
				{
					size_t cchTextMax;
					if (SUCCEEDED(GetTextLength(&cchTextMax)))
					{
						cchTextMax++;
						LPWSTR pszText = Plugin_MallocString(cchTextMax);
						if (NULL != pszText && FAILED(GetText(pszText, (UINT)cchTextMax)))
						{
							Plugin_FreeString(pszText);
							pszText = NULL;
						}
							
						SetClipboardText(hToolbar, pszText);
						Plugin_FreeString(pszText);
					}
				}
				break;
			case ID_ADDRESSBAR_EDITADDRESS:
				ActivateEditor(hToolbar,  NULL);
				break;
		}

		Menu_ReleaseMenu(hMenu, MENU_ADDRESSBAR);
	}
	return TRUE;
}

void ToolbarAddress::SetFocus(HWND hToolbar, ToolbarItem *focusItem, BOOL fSet)
{
	ActivateEditor(hToolbar,  NULL);
}

BOOL ToolbarAddress::SetCursor(HWND hToolbar, HWND hCursor, UINT hitTest, UINT messageId)
{
	RECT textRect;
	CopyRect(&textRect, &rect);
	InflateRect(&textRect, -((iconWidth -1)/2), -(iconHeight - 1)/2);
	UINT messagePos = GetMessagePos();
	POINTS messagePts = MAKEPOINTS(messagePos);
	POINT messagePt;
	POINTSTOPOINT(messagePt, messagePts);
	MapWindowPoints(HWND_DESKTOP, hToolbar, &messagePt, 1);
	if (::PtInRect(&textRect, messagePt))
	{
		HCURSOR cursor = LoadCursor(NULL, IDC_IBEAM);
		if (NULL != cursor)
		{
			::SetCursor(cursor);
			return TRUE;
		}
	}
	return FALSE;
}

void ToolbarAddress::EditboxDestroyed(HWND hwnd)
{
	if (hEditor == hwnd)
	{
		hEditor = NULL;
	
		HWND hToolbar = GetParent(hwnd);
		if (NULL != hToolbar) 
		{
			InvalidateRect(hToolbar, &rect, FALSE);
			Toolbar_CheckHide(hToolbar, TRUE);
		}
	}
}

BOOL ToolbarAddress::EditboxKillFocus(HWND hwnd, HWND hFocus)
{
	DestroyWindow(hwnd);
	return TRUE;
}

void ToolbarAddress::EditboxResetText(HWND hwnd)
{
	UINT editboxStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & editboxStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, editboxStyle & ~WS_VISIBLE);

	SetWindowText(hwnd, text);
	SendMessage(hEditor, NTEBM_SELECTALL, 0, 0L);
	
	if (0 != (WS_VISIBLE & editboxStyle))
	{
		UINT editboxStyle = GetWindowStyle(hwnd);
		if (0 == (WS_VISIBLE & editboxStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, editboxStyle | WS_VISIBLE);
		
		InvalidateRect(hwnd, NULL, FALSE);
	}

}

void ToolbarAddress::EditboxNavigateNextCtrl(HWND hwnd, BOOL fForward)
{
	HWND hToolbar = GetParent(hwnd);
	if (NULL != hToolbar) 
	{
		INT itemIndex = Toolbar_GetNextTabItem(hToolbar, name, (FALSE == fForward));
		if (ITEM_ERR != itemIndex)
		{
			if (FALSE != Toolbar_NextItem(hToolbar, MAKEINTRESOURCE(itemIndex), TRUE))
			{
				::SetFocus(hToolbar);
				return;
			}
		}

		HWND hParent = GetAncestor(hToolbar, GA_PARENT);
		while(NULL != hParent &&
				0 != (WS_CHILD & GetWindowStyle(hParent)) &&
				0 != (WS_EX_CONTROLPARENT & GetWindowStyleEx(hParent)))
		{
			HWND hTest = GetAncestor(hParent, GA_PARENT);
			if (NULL == hTest) break;
			hParent = hTest;
		}
		
		if (NULL != hParent)
		{
			HWND hFocus = GetNextDlgTabItem(hParent, hToolbar, (FALSE == fForward));
			if (NULL != hFocus) ::SetFocus(hFocus);
		}
	}
}

void ToolbarAddress::EditboxAcceptText(HWND hwnd)
{
	UINT cchLenMax = GetWindowTextLength(hwnd);
	if (0 == cchLenMax) return;

	cchLenMax++;
	Plugin_FreeResString(text);
	text = Plugin_MallocString(cchLenMax);
	if (NULL != text)
	{
		INT cchText = GetWindowText(hwnd, text, cchLenMax);
		if (0 != cchText)
		{
			LPCWSTR begin, end;
			begin = text;
			end = text + cchText + 1;
			while (L' ' == *begin && begin != end) begin++;
			while (L' ' == *end && begin != end) end--;
			if (end <= begin) return;
		}
	}
	
	HWND hToolbar = GetParent(hwnd);

	if (NULL != hToolbar) 
	{
		Toolbar_SendCommand(hToolbar, ID_ADDRESSBAR_CHANGED);
	}

	EditboxNavigateNextCtrl(hwnd, TRUE);
}
BOOL ToolbarAddress::EditboxKeyDown(HWND hwnd, UINT vKey, UINT state)
{
	switch(vKey)
	{
		case VK_UP:
		case VK_DOWN:
			return TRUE;
	}
	return FALSE;
}
BOOL ToolbarAddress::EditboxKeyUp(HWND hwnd, UINT vKey, UINT state)
{
	return FALSE;
}
BOOL ToolbarAddress::EditboxPreviewChar(HWND hwnd, UINT vKey, UINT state)
{
	return FALSE;
}