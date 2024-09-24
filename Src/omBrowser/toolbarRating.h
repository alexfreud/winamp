#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_RATING_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_RATING_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./toolbarItem.h"

class ifc_skinnedrating;

class ToolbarRating : public ToolbarItem
{
protected:
	ToolbarRating(LPCSTR pszName, UINT nStyle, LPCWSTR pszText, LPCWSTR pszDescription);
	~ToolbarRating();

public:
	static ToolbarItem* CALLBACK CreateInstance(ToolbarItem::Template *itemTemplate);

public:
	BOOL AdjustRect(HWND hToolbar, RECT *proposedRect);
	BOOL Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state);

	void MouseMove(HWND hToolbar, UINT mouseFlags, POINT pt);
	void MouseLeave(HWND hToolbar);
	void LButtonDown(HWND hToolbar, UINT mouseFlags, POINT pt);
	void LButtonUp(HWND hToolbar, UINT mouseFlags, POINT pt);
	void Click(HWND hToolbar, UINT mouseFlags, POINT pt);
	void UpdateSkin(HWND hToolbar);
	BOOL PtInItem(POINT pt);

	INT GetTip(LPTSTR pszBuffer, INT cchBufferMax);
	BOOL FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax);

	BOOL SetValueInt(HWND hToolbar, INT value);

	void SetFocus(HWND hToolbar, ToolbarItem *focusItem, BOOL fSet);
	BOOL KeyDown(HWND hToolbar, INT vKey, UINT flags);
	BOOL KeyUp(HWND hToolbar, INT vKey, UINT flags);

protected:
	void SendRating(HWND hToolbar, INT ratingValue);

protected:
	BYTE rating;
	BYTE highlighted;
	BYTE focused;
	RECT ratingRect;
	RECT textRect;
	INT baseLine;
	ifc_skinnedrating *skinnedRating;
};

#endif //NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_RATING_HEADER
