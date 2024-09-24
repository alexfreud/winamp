#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_SCROLLWINDOW_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_SCROLLWINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"

typedef struct _SCROLLBAR SCROLLBAR;

// scroll modes
#define SCROLLMODE_STANDARD_I 0x00
#define SCROLLMODE_LISTVIEW_I 0x01
#define SCROLLMODE_TREEVIEW_I 0x02
#define SCROLLMODE_COMBOLBOX_I 0x03

class SkinnedScrollWnd : public SkinnedWnd
{
public:
	typedef enum InvalidateFlags
	{
		InvalidateFlag_Normal = 0,
		InvalidateFlag_RedrawNow = (1 << 0),
		InvalidateFlag_Frame = (1 << 1),
		InvalidateFlag_VertBarAppeared = (1 << 2),
		InvalidateFlag_VertBarRemoved = (1 << 3),
		InvalidateFlag_HorzBarAppeared = (1 << 4),
		InvalidateFlag_HorzBarRemoved = (1 << 5),
	} InvalidateFlags;
	
protected:
	SkinnedScrollWnd(BOOL bIsDialog);
	virtual ~SkinnedScrollWnd(void);

public:
	void UpdateScrollBars(BOOL fInvalidate);
	void ShowHorzScroll(BOOL fEnable);
	void ShowVertScroll(BOOL fEnable);
	BOOL IsHorzBarHidden();
	BOOL IsVertBarHidden();
	BOOL SetMode(UINT nMode);
	UINT GetMode();
	void DisableNoScroll(BOOL bDisable);
	BOOL IsNoScrollDisabled();
	
protected:
	virtual BOOL Attach(HWND hwndHeader);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual INT OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *pncsp);
	virtual INT OnNcHitTest(POINTS pts);
	virtual void OnNcPaint(HRGN rgnUpdate);
	virtual void OnNcMouseMove(UINT nHitTest, POINTS pts);
	virtual void OnNcLButtonDown(UINT nHitTest, POINTS pts);
	virtual void OnNcMouseLeave();
	virtual void OnStyleChanged(INT styleType, STYLESTRUCT *pss);
	virtual void OnLButtonUp(UINT nFlags, POINTS pts);
	virtual void OnMouseMove(UINT nFlags, POINTS pts);
	virtual void OnTimer(UINT_PTR idEvent, TIMERPROC fnTimer);
	virtual LRESULT OnEraseBackground(HDC hdc);
	virtual void OnPrint(HDC hdc, UINT options); 
	virtual void OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw);


	virtual BOOL OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult);	

	BOOL ShowScrollBar(int wBar, BOOL fShow);
	void UpdateScrollBar(SCROLLBAR *psb, InvalidateFlags *invalidateFlags);
	void InvalidateNC(InvalidateFlags invalidate, UINT bars);
	void PaintNonClient(HDC hdc);
	INT AdjustHover(UINT nHitTest, POINTS pts);
	void UpdateFrame();

	
	LRESULT OnListViewScroll(INT dx, INT dy);
	void OnVertScroll(UINT code, UINT pos, HWND hwndSB);
	void OnMouseWheel(INT delta, UINT vtKey, POINTS pts);

	void Emulate_LeftButtonUp(UINT nFlags, POINTS pts, BOOL forwardMessage);

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);
	friend static BOOL GetHScrollRect(SkinnedScrollWnd *pWnd, RECT *prc);
	friend static BOOL GetVScrollRect(SkinnedScrollWnd *pWnd, RECT *prc);


private:
    
	SCROLLBAR	*psbHorz;		
	SCROLLBAR	*psbVert;		
	UINT		scrollFlags;
	UINT scrollPortionHover;
	int wheelCarryover;
};
 
DEFINE_ENUM_FLAG_OPERATORS(SkinnedScrollWnd::InvalidateFlags);

#endif //NULLOSFT_MEDIALIBRARY_SKINNED_SCROLLWINDOW_HEADER
