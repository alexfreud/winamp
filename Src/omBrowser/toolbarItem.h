#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBARITEM_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBARITEM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define ICON_NONE				(-1)

class __declspec(novtable) ToolbarItem
{
public:
	typedef enum
	{
		stateHidden = 0x0001,
		stateDisabled = 0x0002,
        statePressed = 0x0004,
		stateHighlighted = 0x0008,
		stateFocused = 0x0010,
		stateNoFocusRect = 0x0020,
		
		styleChevronOnly	= 0x0100,
		styleNoChevron	= 0x0200,
		stylePopup = 0x0400,

		styleFlexible = 0x1000, // item can change it width
		styleStatic = 0x2000,	// item not reacting on mouse or keyboard events
		styleWantKey = 0x4000,
		styleTabstop = 0x8000,	// item wants to have it's own tabstop (not TBS_TABSTOP)
	} Styles;

	typedef struct __Template
	{
		LPCSTR name;
		LPCWSTR text;
		LPCWSTR description;
		INT iconId;
		INT commandId;
		UINT style;
	} Template;

protected:
	ToolbarItem(LPCSTR pszName, UINT nStyle, INT nIcon, LPCWSTR pszText, LPCWSTR pszDescription);
	virtual ~ToolbarItem();

public:
	ULONG AddRef();
	ULONG Release();
	
	LPCSTR GetName();
	virtual UINT GetStyle();
	virtual void SetStyle(HWND hToolbar, UINT newStyle, UINT styleMask); // if NULL != hToolbar - item will be invalidated
	
	virtual BOOL SetRect(const RECT *prc);
	BOOL GetRect(RECT *prc);
	BOOL OffsetRect(INT dx, INT dy);
	virtual BOOL SetRectEmpty();
	BOOL IsRectEmpty();
	BOOL PtInRect(POINT pt);
	
	BOOL IntersectRect(RECT *prcDst, const RECT *prcSrc);
	
	BOOL IsEqual(LPCSTR pszName, INT cchName);

	virtual HRESULT GetText(LPWSTR pszBuffer, UINT cchBufferMax);
	virtual HRESULT GetTextLength(size_t *pcchLength) {  return E_NOTIMPL; }
	virtual HRESULT GetDescription(LPWSTR pszBuffer, UINT cchBufferMax);
	virtual BOOL SetDescription(HWND hToolbar, LPCWSTR pszDescription);

	virtual BOOL AdjustRect(HWND hToolbar, RECT *proposedRect) { return FALSE; }
	virtual BOOL Paint(HWND hToolbar, HDC hdc, const RECT *paintRect, UINT state) { return FALSE;}
	virtual INT GetTip(LPTSTR pszBuffer, INT cchBufferMax) { return 0; }
	virtual void MouseMove(HWND hToolbar, UINT mouseFlags, POINT pt) {}
	virtual void MouseLeave(HWND hToolbar) {}
	virtual void LButtonDown(HWND hToolbar, UINT mouseFlags, POINT pt) {}
	virtual void LButtonUp(HWND hToolbar, UINT mouseFlags, POINT pt) {}
	virtual void Click(HWND hToolbar, UINT mouseFlags, POINT pt) {}

	virtual BOOL SetValueInt(HWND hToolbar, INT value) { return FALSE; }
	virtual BOOL SetValueStr(HWND hToolbar, LPCWSTR value) { return FALSE; }

	virtual INT GetCommandId() { return 0; }

	virtual void UpdateSkin(HWND hToolbar) {}
	virtual BOOL PtInItem(POINT pt);

	virtual BOOL FillMenuInfo(HWND hToolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax) { return FALSE; }
	virtual BOOL KeyDown(HWND hToolbar, INT vKey, UINT flags) { return FALSE; }
	virtual BOOL KeyUp(HWND hToolbar, INT vKey, UINT flags) { return FALSE; }
	virtual void SetFocus(HWND hToolbar, ToolbarItem *focusItem, BOOL fSet) {}
	virtual BOOL SetCursor(HWND hToolbar, HWND hCursor, UINT hitTest, UINT messageId) { return FALSE; }
	virtual void CommandSent(HWND hToolbar, INT commandId) {}
	virtual BOOL DisplayContextMenu(HWND hToolbar, INT x, INT y) { return FALSE; }

protected:
	ULONG ref;
	LPSTR name;
	UINT style;
	RECT rect;
	INT iconId;
	LPWSTR text;
	LPWSTR description;
};

#endif // NULLSOFT_WINAMP_OMBROWSER_TOOLBARITEM_HEADER