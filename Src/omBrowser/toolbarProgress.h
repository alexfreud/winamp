#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_PROGRESS_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_PROGRESS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./toolbarItem.h"

class ToolbarProgress : public ToolbarItem
{
protected:
	ToolbarProgress(LPCSTR pszName, UINT nStyle, LPCWSTR pszText, LPCWSTR pszDescription);
	~ToolbarProgress();

public:
	static ToolbarItem* CALLBACK CreateInstance(ToolbarItem::Template *itemTemplate);

public:
	void SetStyle(HWND hToolbar, UINT newStyle, UINT styleMask);
	BOOL AdjustRect(HWND hToolbar, RECT *proposedRect);
	BOOL Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state);
	void UpdateSkin(HWND hToolbar);


	BOOL Animate(HWND hToolbar, BOOL fAnimate);
	
protected:
    HBITMAP bitmap;
	INT  frame;
	HWND hTimer;
};

#endif //NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_PROGRESS_HEADER
