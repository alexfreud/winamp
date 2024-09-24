#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_PROGRESSBAR_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_PROGRESSBAR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"


class SkinnedProgressBar : public SkinnedWnd
{

protected:
	SkinnedProgressBar(void);
	virtual ~SkinnedProgressBar(void);

protected:
	virtual BOOL Attach(HWND hProgressBar);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	void OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw);

	void OnPaint(void);
	UINT GetBorderType(void);

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

protected:
	HCURSOR skinCursor;
};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_PROGRESSBAR_HEADER