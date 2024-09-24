#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_LISTBOX_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_LISTBOX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedscrollwnd.h"


class SkinnedListbox : public SkinnedScrollWnd
{

protected:
	SkinnedListbox(void);
	virtual ~SkinnedListbox(void);

protected:
	virtual BOOL Attach(HWND hwndListbox);
	virtual void OnDrawItem(DRAWITEMSTRUCT *pdis);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); 
	virtual void OnSkinUpdated(BOOL bNotifyChildren, BOOL bRedraw);
	virtual HPEN GetBorderPen(void);
	virtual void OnPrint(HDC hdc, UINT options);
public:
	static void DrawItem(DRAWITEMSTRUCT *pdis, LPCWSTR pszText, INT cchText);
	static void MeasureItem(HWND hwnd, LPCWSTR pszText, INT cchText, UINT *pWidth, UINT *pHeight);

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);
};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_LISTBOX_HEADER