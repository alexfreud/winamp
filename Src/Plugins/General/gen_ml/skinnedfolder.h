#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_FOLDERBROWSER_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_FOLDERBROWSER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedscrollwnd.h"


class SkinnedFolderBrowser : public SkinnedScrollWnd
{

protected:
	SkinnedFolderBrowser(void);
	virtual ~SkinnedFolderBrowser(void);
public:
	virtual BOOL SetStyle(UINT newStyle, BOOL bRedraw);

protected:
	virtual BOOL Attach(HWND hwndFB);
	virtual void OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw);


private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

protected:

};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_FOLDERBROWSER_HEADER