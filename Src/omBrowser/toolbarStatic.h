#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_STATIC_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_STATIC_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./toolbarItem.h"

class ToolbarStatic : public ToolbarItem
{
public:
	typedef enum
	{
		styleLabel		= 0x00000000,
		styleSpacer		= 0x00010000,
		styleSeparator	= 0x00020000,
		
	} staticStyles;

protected:
	ToolbarStatic(LPCSTR pszName, UINT nStyle, INT nIcon, LPCWSTR pszText, LPCWSTR pszDescription);
	~ToolbarStatic() {}

public:
	static ToolbarItem* CALLBACK CreateInstance(ToolbarItem::Template *itemTemplate);

public:
	BOOL AdjustRect(HWND hToolbar, RECT *proposedRect);
	BOOL Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state);
	INT GetTip(LPTSTR pszBuffer, INT cchBufferMax);
	void UpdateSkin(HWND hToolbar);
	BOOL FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax);

protected:
	INT spaceWidth;
};

#endif //NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_STATIC_HEADER
