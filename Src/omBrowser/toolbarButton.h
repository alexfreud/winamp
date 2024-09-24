#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_BUTTON_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_BUTTON_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./toolbarItem.h"

class ToolbarButton : public ToolbarItem
{
public:
	typedef enum
	{
		stylePushbutton  = 0x00000000, 
		styleCommandLink = 0x00010000,
	} buttonStyles;

protected:
	ToolbarButton(LPCSTR  pszName, INT nCommand, UINT nStyle, INT nIcon, LPCWSTR pszText, LPCWSTR pszDescription);
	virtual ~ToolbarButton() {}

public:
	static ToolbarItem* CALLBACK CreateInstance(ToolbarItem::Template *itemTemplate);

public:
	BOOL AdjustRect(HWND hToolbar, RECT *proposedRect);
	BOOL Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state);
	INT GetTip(LPTSTR pszBuffer, INT cchBufferMax);

	void MouseMove(HWND hToolbar, UINT mouseFlags, POINT pt);
	void MouseLeave(HWND hToolbar);
	void LButtonDown(HWND hToolbar, UINT mouseFlags, POINT pt);
	void LButtonUp(HWND hToolbar, UINT mouseFlags, POINT pt);
	void Click(HWND hToolbar, UINT mouseFlags, POINT pt);
	void UpdateSkin(HWND hToolbar);
	BOOL PtInItem(POINT pt);

	INT GetCommandId();

	BOOL FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax);

	void SetFocus(HWND hToolbar, ToolbarItem *focusItem, BOOL fSet);
	BOOL KeyDown(HWND hToolbar, INT vKey, UINT flags);
	BOOL KeyUp(HWND hToolbar, INT vKey, UINT flags);
	

protected:
	UINT	commandId;
	INT		offsetX;
	INT		offsetY;
	COLORREF rgbText;
	COLORREF rgbHilite;
};

#endif //NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_BUTTON_HEADER
