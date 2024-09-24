#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_DIVIDER_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_DIVIDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"


class SkinnedDivider : public SkinnedWnd
{

protected:
	SkinnedDivider(void);
	virtual ~SkinnedDivider(void);

protected:
	virtual BOOL Attach(HWND hwndButton);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	virtual BOOL OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult);
	void OnPaint(void);

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

protected:
	MLDIVIDERMOVED callback;
	LPARAM userParam;
	int clickoffs;

};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_BUTTON_HEADER