#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_COMBOBOX_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_COMBOBOX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"


class SkinnedCombobox : public SkinnedWnd
{

protected:
	SkinnedCombobox(void);
	virtual ~SkinnedCombobox(void);
public:
	virtual BOOL SetStyle(UINT newStyle, BOOL bRedraw);

protected:
	virtual BOOL Attach(HWND hwndCombo);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	virtual void OnPaint(void);
	BOOL IsButtonDown(DWORD windowStyle);
	virtual void OnSkinUpdated(BOOL bNotifyChildren, BOOL bRedraw);
	virtual INT OnNcHitTest(POINTS pts);
	virtual void OnMouseLeave(void);
	
public:
	static void DrawButton(HDC hdc, RECT *prcButton, BOOL bPressed, BOOL bActive);

private:
	LRESULT SilenceMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);


protected:
	BOOL activeBorder;

};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_COMBOBOX_HEADER