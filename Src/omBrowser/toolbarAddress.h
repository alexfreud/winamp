#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_ADDRESS_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_ADDRESS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./toolbarItem.h"
#include "./toolbarEditbox.h"

class ToolbarAddress : public ToolbarItem, public ToolbarEditboxHost
{
public:
	typedef enum
	{
		styleAddressReadonly	= 0x00010000,
		styleAddressShowReal	= 0x00020000,
	} addressStyles;

protected:
	ToolbarAddress(LPCSTR pszName, UINT nStyle);
	~ToolbarAddress();

public:
	static ToolbarItem* CALLBACK CreateInstance(ToolbarItem::Template *itemTemplate);

public:
	BOOL AdjustRect(HWND hToolbar, RECT *proposedRect);
	BOOL Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state);
	INT GetTip(LPTSTR pszBuffer, INT cchBufferMax);
	BOOL SetDescription(HWND hToolbar, LPCWSTR pszDescription);
	BOOL SetRect(const RECT *prc);
	BOOL SetRectEmpty();
	UINT GetStyle();
	void SetStyle(HWND hToolbar, UINT newStyle, UINT styleMask); 
	HRESULT GetText(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetTextLength(size_t *pcchLength);

	void MouseMove(HWND hToolbar, UINT mouseFlags, POINT pt);
	void MouseLeave(HWND hToolbar);
	void LButtonDown(HWND hToolbar, UINT mouseFlags, POINT pt);
	void SetFocus(HWND hToolbar, ToolbarItem *focusItem, BOOL fSet);
	BOOL SetCursor(HWND hToolbar, HWND hCursor, UINT hitTest, UINT messageId);

	void UpdateSkin(HWND hToolbar);
	BOOL FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax);
	void CommandSent(HWND hToolbar, INT commandId);

	BOOL SetValueStr(HWND hToolbar, LPCWSTR value);

	BOOL DisplayContextMenu(HWND hToolbar, INT x, INT y);

	//ToolbarEditboxHost
	void EditboxDestroyed(HWND hwnd);
	BOOL EditboxKillFocus(HWND hwnd, HWND hFocus);
	void EditboxResetText(HWND hwnd);
	void EditboxNavigateNextCtrl(HWND hwnd, BOOL fForward);
	void EditboxAcceptText(HWND hwnd);
	BOOL EditboxKeyDown(HWND hwnd, UINT vKey, UINT state);
	BOOL EditboxKeyUp(HWND hwnd, UINT vKey, UINT state);
	BOOL EditboxPreviewChar(HWND hwnd, UINT vKey, UINT state);

protected:
	BOOL ActivateEditor(HWND hToolbar, const POINT *ppt);
	BOOL SetClipboardText(HWND hToolbar, LPCWSTR pszText);

protected:
	UINT minWidth;
	HBITMAP bitmap;
	INT iconHeight;
	INT iconWidth;
	INT textHeight;
	INT textMargin;
	COLORREF rgbText;
	COLORREF rgbBk;
	HWND hEditor;
};

#endif //NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_STATIC_HEADER