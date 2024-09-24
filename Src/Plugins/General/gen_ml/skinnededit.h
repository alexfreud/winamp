#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_EDIT_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_EDIT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"


class SkinnedEdit : public SkinnedWnd
{
protected:
	SkinnedEdit(void);
	virtual ~SkinnedEdit(void);

protected:
	virtual BOOL Attach(HWND hwndEdit);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	virtual void OnPaint(void);
	virtual void OnSkinUpdated(BOOL bNotifyChildren, BOOL bRedraw);
	virtual void OnWindowPosChanged(WINDOWPOS *pwp);
	virtual void OnSetFont(HFONT hFont, BOOL fRedraw);
	void FontChanged();

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);
	void EraseBckGnd(HDC hdc, RECT *prc, RECT *prcText, BOOL fEraseAll, HBRUSH hBrush);
	void DrawText(HDC hdc, RECT *prc, RECT *prcText, LPCWSTR pszText, INT cchText);
	LRESULT OverrideDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

	typedef struct __SELECTION
	{
		INT first;
		INT last;
		LONG leftX;
		LONG rightX;
	} SELECTION;

	BOOL GetSelection(SELECTION *selection, INT cchText, const RECT *clientRect);

protected:
	int firstVisible;
	int lastVisible;
	int firstSelected;
	int lastSelected;
	INT maxCharWidth;
	WPARAM mouseWParam;
	LPARAM mouseLParam;
	int	cx;
	int cy;
};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_EDIT_HEADER