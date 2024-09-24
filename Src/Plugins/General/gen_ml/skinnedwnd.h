#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_WINDOW_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "./ml.h"
#include "./ml_ipc_0313.h"
#include "./reflectmsg.h"
#include "./stockobjects.h"


#define MLWM_SKINCHANGED			(WM_APP + 0x2FFF)

#define BORDER_NONE		0
#define BORDER_SUNKEN	1
#define BORDER_FLAT		2

typedef enum SkinnedWndRedraw
{
	SWR_NONE = 0,
	SWR_INVALIDATE = (1 << 0),
	SWR_ERASE = (1 << 1),
	SWR_UPDATE = (1 << 2),
	SWR_ALLCHILDREN = (1 << 3),
}SkinnedWndRedraw;

DEFINE_ENUM_FLAG_OPERATORS(SkinnedWndRedraw);


class SkinnedWnd
{
protected:
	SkinnedWnd(BOOL bIsDialog);
	virtual ~SkinnedWnd(void);

public:
	static SkinnedWnd *GetFromHWND(HWND hwndSkinned);
	static BOOL IsDwmCompositionEnabled();

	BOOL IsUnicode(void);
	BOOL IsAttached(void);
	INT GetType(void) { return (INT)LOWORD(wnddata); }
	HWND GetHWND(void) { return hwnd; }
	void SkinChanged(BOOL bNotifyChildren, BOOL bRedraw);
	void EnableReflection(BOOL bEnable);
	virtual BOOL SetStyle(UINT newStyle, BOOL bRedraw);
	virtual UINT GetStyle(void) { return style; }
	void SetMinMaxInfo(MLSKINNEDMINMAXINFO *minMax);

protected:
	virtual BOOL Attach(HWND hwndToSkin);	
	void SetType(INT newType) { ((wnddata & 0xFFFF0000) | (newType & 0xFFFF)); }
	LRESULT CallPrevWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CallDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw);
	virtual void OnSkinUpdated(BOOL bNotifyChildren, BOOL bRedraw);
	virtual BOOL OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult);

	virtual INT OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *pncsp);
	virtual INT OnNcHitTest(POINTS pts);
	virtual void OnNcPaint(HRGN rgnUpdate);
	virtual void OnPrint(HDC hdc, UINT options); 
	virtual void DrawBorder(HDC hdc);
	virtual void OnDwmCompositionChanged(void);
	virtual UINT GetBorderType(void);
	virtual HPEN GetBorderPen(void);
	void OnUpdateUIState(UINT uAction, UINT uState);
	virtual void OnStyleChanged(INT styleType, STYLESTRUCT *pss);

	void DisableRedraw();
	void EnableRedraw(SkinnedWndRedraw redrawFlags);

	virtual BOOL OnDirectMouseWheel(INT delta, UINT vtKey, POINTS pts);
	virtual void OnGetMinMaxInfo(MINMAXINFO *minMax);
	
public:
	static void DrawBorder(HDC hdc, RECT *prc, UINT type, HPEN pen);

private:
	static LRESULT WINAPI WindowProcReal(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);
	friend BOOL UnskinWindow(HWND hwndToUnskin);

protected:
	HWND	hwnd;
	UINT	style;
	WORD	uiState;
	ULONG	redrawLock;
	SIZE	minSize;
	SIZE	maxSize;
private:
	WNDPROC	fnWndProc;
	LONG	wnddata;
};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_WINDOW_HEADER