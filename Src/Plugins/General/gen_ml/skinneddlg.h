#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_DIALOG_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_DIALOG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedscrollwnd.h"
#include <commctrl.h>

class SkinnedDialog : public SkinnedWnd
{

protected:
	SkinnedDialog(void);
	virtual ~SkinnedDialog(void);

protected:
	virtual BOOL Attach(HWND hwndDialog);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	virtual HBRUSH OnColorDialog(HDC hdc);
private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

protected:

};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_DIALOG_HEADER