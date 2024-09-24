#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_LISTVIEW_CONTROL_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_LISTVIEW_CONTROL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedscrollwnd.h"
#include <commctrl.h>

extern int config_use_alternate_colors;

class SkinnedListView : public SkinnedScrollWnd
{

protected:
	SkinnedListView(void);
	virtual ~SkinnedListView(void);


public:
	virtual BOOL SetStyle(UINT newStyle, BOOL bRedraw);
protected:
	virtual BOOL Attach(HWND hwndListView);
	virtual void TryAttachHeader(void);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnReflectedNotify(HWND hwndFrom, INT idCtrl, NMHDR *pnmh, LRESULT *pResult);
	virtual BOOL OnCustomDraw(HWND hwndFrom, NMLVCUSTOMDRAW *plvcd, LRESULT *pResult);
	virtual void OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw);
	virtual LRESULT OnEraseBackground(HDC hdc);
	virtual BOOL OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult);
	virtual void OnLButtonDown(UINT uFlags, POINTS pts);
	virtual void OnRButtonDown(UINT uFlags, POINTS pts);
	virtual void OnKeyDown(UINT vkCode, UINT flags);
	virtual LRESULT OnGetDlgCode(UINT vkCode, MSG* pMsg);

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

private:
	UINT	listFlags;
	COLORREF currentColor;
	DWORD currentItem;
};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_LISTVIEW_CONTROL_HEADER