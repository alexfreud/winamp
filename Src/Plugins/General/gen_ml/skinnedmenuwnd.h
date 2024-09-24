#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_MENU_WINDOW_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_MENU_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"
#include "./skinnedmenuthreadinfo.h"

#define MENU_BUTTON_SCROLLUP		0x0001
#define MENU_BUTTON_SCROLLDOWN		0x0002

#define MENU_BUTTON_STATE_DISABLED	0x0001
#define MENU_BUTTON_STATE_PRESSED	0x0002

class SkinnedMenuWnd : public SkinnedWnd
{
protected:
	SkinnedMenuWnd(UINT menuExStyle, HMLIMGLST hmlil, INT forcedWidth, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);
	virtual ~SkinnedMenuWnd(void);

public:
	HMENU GetMenuHandle();
	HWND GetOwnerWindow();
	HWND SetOwnerWindow(HWND hwndOwner);

protected:
	virtual BOOL Attach(HWND hwndMenu, HWND hwndOwner);
	virtual BOOL AttachMenu(HMENU hMenuToAttach);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	virtual HPEN GetBorderPen(void);
	BOOL OnReflectedDrawItem(DRAWITEMSTRUCT *pdis);
	BOOL OnReflectedMeasureItem(MEASUREITEMSTRUCT *pmis);
	HFONT GetMenuFont(BOOL fBold);
	INT GetLineHeight();

	virtual LRESULT OnEraseBackground(HDC hdc);
	virtual void OnPrint(HDC hdc, UINT options);
	virtual void OnNcPaint(HRGN rgnUpdate);

	virtual INT OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *pncsp);
	virtual void DrawBorder(HDC hdc);
	
	BOOL IsSkinnedItem(UINT itemId);

	BOOL DrawScrollButton(HDC hdc, UINT scrollButton);
	void PaintScrollButton(HDC hdc, const RECT *prc, UINT scrollButton, BOOL buttonState);
	LRESULT OnMenuSelect(UINT selectedItem);
	LRESULT CallHookedWindowProc(UINT uItem, BOOL fByPosition, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT FindHiliteItem(HMENU hMenu);

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);
	friend class SkinnedMenu;

protected:
	typedef struct SkinnedItemRecord
	{
		unsigned int itemId;
		unsigned int originalId;
		BOOL skinned;
		BOOL failed;
	}SkinnedItemRecord;

protected:
	SkinnedMenuThreadInfo *threadInfo;
	HWND		hOwner;
	HMENU		hMenu;
	UINT		menuExStyle;
	HMLIMGLST	hmlil;
	INT			lineWidth = 0;
	INT			lineHeight = 0;
	INT			imageWidth = 0;
	INT			imageHeight = 0;
	INT			shortcutCX = 0;
	INT			textCX = 0;
	BOOL		bRestoreShadow;
	HFONT		hBoldFont;
	HBRUSH		backBrush;
	HPEN		borderPen;
	HBRUSH		menuOrigBrush;

	SkinnedItemRecord	*skinnedItems;
	INT			skinnedItemCount;
	INT			skinnedItemCursor;
	INT			prevSelectedItem;
	HBITMAP		scrollBitmap;
	HBITMAP		disabledScrollBitmap;

	UINT		menuFlags;

	MENUCUSTOMIZEPROC customProc;
	ULONG_PTR customParam;
};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_MENU_WINDOW_HEADER