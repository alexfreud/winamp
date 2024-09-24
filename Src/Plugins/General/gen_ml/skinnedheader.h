#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_HEADER_CONTROL_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_HEADER_CONTROL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"
#include <commctrl.h>


#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200
#endif // !HDF_SORTUP


// size rules
#define SHS_SIZERULE_WINDOWS		0x00	// Use windows default size rule.
#define SHS_SIZERULE_ADJUSTONE		0x01	// Resize column and adjust adjacent column.
#define SHS_SIZERULE_ADJUSTALL		0x03	// Resize column and adjust adjacent column when column reach minimum adjust next one.
#define SHS_SIZERULE_PROPORTIONAL	0x02	// Resize column and adjust all columns proportionately.


class SkinnedHeader : public SkinnedWnd
{

protected:
	SkinnedHeader(void);
	virtual ~SkinnedHeader(void);
	
	void SetHeight(INT nHeight); // if nHeight == -1 control will calculate height required to fit current font
	
protected:
	virtual BOOL Attach(HWND hwndHeader);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnReflectedNotify(INT idCtrl, REFLECTPARAM *rParam);
	virtual BOOL OnCustomDraw(HWND hwndFrom, NMCUSTOMDRAW *pnmcd, LRESULT *pResult);
	virtual void OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw);
	virtual void OnPaint(void);
	virtual BOOL OnLayout(HDLAYOUT *pLayout);
	virtual BOOL OnSetCursor(HWND hwdCursor, UINT hitTest, UINT message);
	virtual BOOL OnBeginTrack(HWND hwndFrom, NMHEADERW *phdn, LRESULT *pResult);
	virtual BOOL OnEndTrack(HWND hwndFrom, NMHEADERW *phdn);
	virtual BOOL OnItemChanging(HWND hwndFrom, NMHEADERW *phdn, LRESULT *pResult);
	virtual BOOL OnItemChanged(HWND hwndFrom, NMHEADERW *phdn);
	virtual void OnTimer(UINT_PTR nIDEvent, TIMERPROC lpTimerFunc);
	virtual BOOL OnCursorNotify(HWND hwndFrom, NMMOUSE *pnm, LRESULT *pResult);
	virtual BOOL OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult);
	virtual LRESULT OnSetItem(INT iIndex, HDITEMW *phdItem, BOOL bUnicode);

private:
	void DrawHeaderItem(LPNMCUSTOMDRAW pnmcd);
	UINT SizeRuleAdjustOne(HDITEMW *phdi, INT index, UINT uMsg);
	UINT SizeRuleProportional(HDITEMW *phdi, INT index, UINT uMsg);
	void BlockRedraw(BOOL bBlock, UINT unblockDelay);
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

public:
	static DWORD GetSortArrowSize(void);
	static BOOL DrawSortArrow(HDC hdc, RECT *prc, COLORREF rgbBk, COLORREF rgbFg, BOOL bAscending);
	static BOOL DrawCloudIcon(HDC hdc, RECT *prc, COLORREF rgbBk, COLORREF rgbFg);

protected:
	DWORD		hdrFlags;
	DWORD		hdrSizeRule;
	HCURSOR		hcurNormal;
	INT			cloudColumn;
};

#endif // NULLOSFT_MEDIAIBRARY_SKINNED_HEADER_CONTROL_HEADER