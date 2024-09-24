#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLISTBOX_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLISTBOX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

class SetupGroupList;
class SetupListboxItem;

class __declspec(novtable) SetupListbox
{
public:
	typedef enum
	{
		menuGroupContext = 0,
	} contextMenu;

public:
	static HRESULT CreateInstance(HWND hListbox, SetupGroupList *groupList, SetupListbox **pInstance);
	static SetupListbox *GetInstance(HWND hListbox);

public:
	virtual HWND GetHwnd() = 0;
	virtual HFONT GetFont() = 0;
	virtual LRESULT CallDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual LRESULT CallPrevProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	virtual BOOL MeasureItem(INT itemId, UINT *cx, UINT *cy) = 0;
	virtual BOOL DrawItem(HDC hdc, const RECT *prcItem, INT itemId, UINT itemState, UINT itemAction) = 0;
	virtual INT_PTR KeyToItem(INT vKey, INT caretPos) = 0;
	virtual INT_PTR CharToItem(INT vKey, INT caretPos) = 0;

	virtual BOOL DrawCheckbox(HDC hdc, BOOL checked, UINT state, const RECT *pRect, const RECT *pClipRect) = 0;
	virtual BOOL GetCheckboxMetrics(HDC hdc, BOOL checked, UINT state, SIZE *pSize) = 0;
	virtual void SetCapture(SetupListboxItem *item) = 0;
	virtual SetupListboxItem *GetCapture() = 0;
	virtual void ReleaseCapture() = 0;
	
	virtual BOOL InvalidateRect(const RECT *prcInvalidate, BOOL fErase) = 0;
	virtual BOOL InvalidateItem(INT iItem, BOOL fErase) = 0;
	virtual void UpdateCount() = 0;
	

	virtual BOOL DrawExpandbox(HDC hdc, BOOL fExpanded, const RECT *pRect, COLORREF rgbBk, COLORREF rgbFg) = 0;
	virtual BOOL GetExpandboxMetrics(HDC hdc, BOOL fExpanded, SIZE *pSize) = 0;

	virtual INT GetPageCount() = 0;
	virtual INT GetNextEnabledItem(INT iItem, SetupListboxItem **itemOut) = 0;
	virtual INT GetPrevEnabledItem(INT iItem, SetupListboxItem **itemOut) = 0;
	virtual SetupListboxItem *GetSelection() = 0;
	virtual BOOL SetSelection(SetupListboxItem *item) = 0;
	virtual BOOL GetIndex(SetupListboxItem *item, INT *iItem) = 0;

	virtual HMENU GetContextMenu(UINT menuId) = 0;


};

class __declspec(novtable) SetupListboxItem
{

public:
	virtual BOOL MeasureItem(SetupListbox *instance, UINT *cx, UINT *cy) = 0;
	virtual BOOL DrawItem(SetupListbox *instance, HDC hdc, const RECT *prc, UINT state) = 0;
	virtual INT_PTR KeyToItem(SetupListbox *instance, const RECT *prcItem, INT vKey) = 0;
	virtual BOOL MouseMove(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt) = 0;
	virtual BOOL MouseLeave(SetupListbox *instance, const RECT *prcItem) = 0;
	virtual BOOL LButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt) = 0;
	virtual BOOL LButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt) = 0;
	virtual BOOL LButtonDblClk(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt) = 0;
	virtual BOOL RButtonDown(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt) = 0;
	virtual BOOL RButtonUp(SetupListbox *instance, const RECT *prcItem, UINT mouseFlags, POINT pt) = 0;
	virtual void CaptureChanged(SetupListbox *instance, const RECT *prcItem, SetupListboxItem *captured) = 0;
	virtual BOOL IsDisabled() = 0;
	virtual void Command(SetupListbox *instance, INT commandId, INT eventId) = 0;

	virtual HWND CreateDetailsView(HWND hParent) = 0;
	virtual BOOL GetUniqueName(LPWSTR pszBuffer, UINT cchBufferMax) = 0;
};

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPLISTBOX_HEADER