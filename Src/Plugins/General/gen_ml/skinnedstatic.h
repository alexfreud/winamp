#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_STATIC_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_STATIC_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"

#define STATIC_TEXT_MAX 1024

class SkinnedStatic : public SkinnedWnd
{
protected:
	SkinnedStatic(void);
	virtual ~SkinnedStatic(void);

protected:
	virtual BOOL Attach(HWND hwndStatic);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	virtual BOOL OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult);
	virtual LRESULT GetIdealSize(LPCWSTR pszText);

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

protected:
};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_STATIC_HEADER