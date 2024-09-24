#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_TOOLTIP_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_TOOLTIP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"


class SkinnedToolTip : public SkinnedWnd
{

protected:
	SkinnedToolTip(void);
	virtual ~SkinnedToolTip(void);

protected:
	virtual BOOL Attach(HWND hToolTip);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	void OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw);

	HPEN GetBorderPen(void);
	void OnPaint();
	
private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

protected:
	HCURSOR skinCursor;
	wchar_t *buffer;
	size_t bufferSizeCch;
};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_TOOLTIP_HEADER