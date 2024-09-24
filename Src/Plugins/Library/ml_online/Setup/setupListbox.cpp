#include "./setupPage.h"
#include "./setupListbox.h"
#include "./setupGroupList.h"
#include "./setupImage.h"
#include "../common.h"
#include "../resource.h"
#include "../api__ml_online.h"

#include "../../nu/windowsTheme.h"

#include <vssym32.h>

//#include <tmschema.h>

static ATOM SERVICELIST_PROP = 0;

#define SLF_UNICODE  0x0001
#define SLF_DRAGMOVE  0x0002

typedef BOOL (SetupListboxItem::*ITEMMOUSEPROC)(SetupListbox*, const RECT*, UINT, POINT);

class Listbox : public SetupListbox
{

protected:
	Listbox(HWND hListbox, SetupGroupList *groupList);
	~Listbox();

public:
	static HRESULT AttachToWindow(HWND hwndListbox, SetupGroupList *groupList, SetupListbox **pInstance);

public:
	HWND GetHwnd() { return hwnd; }
	HFONT GetFont() { return (HFONT)::SendMessage(hwnd, WM_GETFONT, 0, 0L); }
	LRESULT CallDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CallPrevProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL MeasureItem(INT itemId, UINT *cx, UINT *cy);
	BOOL DrawItem(HDC hdc, const RECT *itemRect, INT itemId, UINT itemState, UINT itemAction);
	INT_PTR KeyToItem(INT vKey, INT caretPos);
	INT_PTR CharToItem(INT vKey, INT caretPos);

	BOOL DrawCheckbox(HDC hdc, BOOL checked, UINT state, const RECT *pRect, const RECT *pClipRect);
	BOOL GetCheckboxMetrics(HDC hdc, BOOL checked, UINT state, SIZE *pSize);

	INT GetCheckboxThemeState(BOOL checked, UINT state);
	INT HitTest(POINT pt, RECT *prcItem);

	void SetCapture(SetupListboxItem *item);
	SetupListboxItem *GetCapture();
	void ReleaseCapture();
	BOOL InvalidateRect(const RECT *prcInvalidate, BOOL fErase);
	BOOL InvalidateItem(INT itemId, BOOL fErase);
	void UpdateCount();

	BOOL DrawExpandbox(HDC hdc, BOOL fExpanded, const RECT *pRect, COLORREF rgbBk, COLORREF rgbFg);
	BOOL GetExpandboxMetrics(HDC hdc, BOOL fExpanded, SIZE *pSize);

	INT GetPageCount();
	INT GetNextEnabledItem(INT iItem, SetupListboxItem **itemOut);
	INT GetPrevEnabledItem(INT iItem, SetupListboxItem **itemOut);

	SetupListboxItem *GetSelection();
	BOOL SetSelection(SetupListboxItem *item);
	BOOL GetIndex(SetupListboxItem *item, INT *iItem);

	BOOL DoDragAndDrop(UINT mouseEvent, UINT mouseFlags, POINT pt);
	HMENU GetContextMenu(UINT menuId);

protected:
	friend static LRESULT WINAPI Listbox_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnDestroy();
	void GetItemRect(INT itemId, RECT *prcItem);

	SetupImage *GetExpandboxImage(HDC hdc, BOOL fExpanded);

	void OnMouseEvent(UINT mouseEvent, UINT mouseFlags, POINTS pts, BOOL fDefaultHandler, ITEMMOUSEPROC proc);
	void OnMouseLeave();
	void OnEraseBkGround(HDC hdc);
	void OnCaptureChanged(HWND hwndGained);
	void NotifyReleaseCapture(INT itemId, SetupListboxItem *itemGain);
	void OnCommand(INT commandId, INT eventId, HWND hControl);

protected:
	HWND hwnd;
	UINT flags;
	WNDPROC originalProc;
	UXTHEME buttonTheme;
	SetupGroupList *groups;
	INT mouseoverId;
	INT capturedId;
	SetupImage *expandedImage;
	SetupImage *collapsedImage;
};

#define GetList(__hwnd) ((Listbox*)GetPropW((__hwnd), MAKEINTATOM(SERVICELIST_PROP)))




HRESULT SetupListbox::CreateInstance(HWND hListbox, SetupGroupList *groupList, SetupListbox **pInstance)
{
	return Listbox::AttachToWindow(hListbox, groupList, pInstance);
}

SetupListbox *SetupListbox::GetInstance(HWND hListbox)
{
	 return GetList(hListbox);
}

Listbox::Listbox(HWND hListbox, SetupGroupList *groupList) 
	: hwnd(hListbox), flags(0), originalProc(NULL), buttonTheme(NULL), groups(groupList),
	mouseoverId(LB_ERR), capturedId(LB_ERR), expandedImage(NULL), collapsedImage(NULL)
{
	if (IsWindowUnicode(hwnd)) 
		flags |= SLF_UNICODE;

	buttonTheme = (UxIsAppThemed()) ? UxOpenThemeData(hwnd, L"Button") : NULL;
	
	groups->AddRef();
	UpdateCount();
	
}

Listbox::~Listbox()
{
	if (NULL != hwnd)
	{
		RemoveProp(hwnd, MAKEINTATOM(SERVICELIST_PROP));
		if (NULL != originalProc)
		{
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)originalProc);
			CallPrevProc(WM_DESTROY, 0, 0L);
		}
	}

	if (NULL != groups)
		groups->Release();

	if (NULL != buttonTheme)
		UxCloseThemeData(buttonTheme);

	if (NULL != expandedImage)
		expandedImage->Release();

	if (NULL != collapsedImage)
		collapsedImage->Release();
}

HRESULT Listbox::AttachToWindow(HWND hListbox, SetupGroupList *groupList, SetupListbox **pInstance)
{	
	if (0 == SERVICELIST_PROP)
	{
		 SERVICELIST_PROP = GlobalAddAtom(TEXT("omSetupListbox"));
		 if (0 == SERVICELIST_PROP) return E_UNEXPECTED;
	}

		
	if(NULL == hListbox || !IsWindow(hListbox) || NULL == groupList) 
		return E_INVALIDARG;

	Listbox *list = new Listbox(hListbox, groupList);
	if (NULL == list) 
		return E_OUTOFMEMORY;
	
	list->originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hListbox, GWLP_WNDPROC,
							(LONGX86)(LONG_PTR)Listbox_WindowProc);
	
	if (NULL == list->originalProc || !SetProp(hListbox, MAKEINTATOM(SERVICELIST_PROP), list))
	{
		if (NULL != list->originalProc)
			SetWindowLongPtr(hListbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)list->originalProc);
		delete(list);
		return E_FAIL;
	}

	
	if (NULL != pInstance)
		*pInstance = list;
	
	return S_OK;
}

LRESULT Listbox::CallDefaultProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (0 != (SLF_UNICODE & flags)) ? 
				DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

LRESULT Listbox::CallPrevProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	return (0 != (SLF_UNICODE & flags)) ? 
			CallWindowProcW(originalProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(originalProc, hwnd, uMsg, wParam, lParam);
}


void Listbox::UpdateCount()
{
	SendMessage(hwnd, WM_SETREDRAW, FALSE, 0L);

	INT iSelected = (INT)SendMessage(hwnd, LB_GETCURSEL, 0, 0L);

	size_t recordCount = (NULL != groups) ? groups->GetListboxCount() : 0;
	SendMessage(hwnd, LB_SETCOUNT, (WPARAM)recordCount, 0L);

	SetupListboxItem *item;
	UINT cy, maxCY = 0;
	for (size_t i = 0; i < recordCount; i++)
	{
		if (SUCCEEDED(groups->FindListboxItem(i, &item)) && 
			item->MeasureItem(this, NULL, &cy) && cy > maxCY)
		{
			maxCY = cy;
		}
	}

	SendMessage(hwnd, LB_SETITEMHEIGHT, (WPARAM)0, (LPARAM)maxCY);

	if (recordCount > 0)
	{
		if (iSelected < 0) 
			iSelected = 0;
		
		if ((size_t)iSelected >= recordCount)
			iSelected = (INT)(recordCount - 1);
		
		SendMessage(hwnd, LB_SETCARETINDEX, (WPARAM)iSelected, TRUE);
		SendMessage(hwnd, LB_SETANCHORINDEX, (WPARAM)iSelected, 0L);
		SendMessage(hwnd, LB_SETCURSEL, (WPARAM)iSelected, 0L);
	}

	SendMessage(hwnd, WM_SETREDRAW, TRUE, 0L);
}

BOOL Listbox::MeasureItem(INT itemId, UINT *cx, UINT *cy)
{
	SetupListboxItem *item;
	HRESULT hr = groups->FindListboxItem(itemId, &item);
	return (SUCCEEDED(hr)) ? item->MeasureItem(this, cx, cy) : FALSE;
}

BOOL Listbox::DrawItem(HDC hdc, const RECT *prcItem, INT itemId, UINT itemState, UINT itemAction)
{
	SetupListboxItem *item;
	HRESULT hr = groups->FindListboxItem(itemId, &item);
	if (FAILED(hr)) return FALSE;

	if (0 != (ODS_SELECTED & itemState) && GetFocus() != hwnd)
		itemState |= ODS_INACTIVE;

	if (item->IsDisabled())
		itemState |= ODS_DISABLED;

	return item->DrawItem(this, hdc, prcItem, itemState);
}

INT Listbox::GetPageCount()
{
	RECT clientRect;
	if (NULL == hwnd || !GetClientRect(hwnd, &clientRect))
		return 0;
	
	INT itemHeight = (INT)SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0L);
	return (clientRect.bottom - clientRect.top) / itemHeight;
}

INT Listbox::GetNextEnabledItem(INT iItem, SetupListboxItem **itemOut)
{
	INT iLast = (INT)SendMessage(hwnd, LB_GETCOUNT, 0, 0L);
	if (iLast >= 0) iLast--;

	SetupListboxItem *testItem;
	while(iItem++ < iLast)
	{
		if (SUCCEEDED(groups->FindListboxItem(iItem, &testItem)))
		{
			if (FALSE == testItem->IsDisabled()) break;
		}
	}
	if (NULL != itemOut)
	{
		*itemOut = (iItem <= iLast) ? testItem : NULL;
	}
	return (iItem <= iLast) ? iItem : LB_ERR;
}

INT Listbox::GetPrevEnabledItem(INT iItem, SetupListboxItem **itemOut)
{	
	SetupListboxItem *testItem;
	while(iItem-- > 0)
	{
		if (SUCCEEDED(groups->FindListboxItem(iItem, &testItem)))
		{
			if (FALSE == testItem->IsDisabled()) break;
		}
	}

	if (NULL != itemOut)
	{
		*itemOut = (iItem >= 0) ? testItem : NULL;
	}
	return (iItem >= 0) ? iItem : LB_ERR;
}

SetupListboxItem *Listbox::GetSelection()
{
	INT iSelected = (INT)SendMessage(hwnd, LB_GETCURSEL, 0, 0L);
	if (LB_ERR == iSelected)
		return NULL;

	SetupListboxItem *item;
	return (SUCCEEDED(groups->FindListboxItem(iSelected, &item))) ? item : NULL;
}

BOOL Listbox::SetSelection(SetupListboxItem *item)
{
	INT iItem = LB_ERR;
	if (NULL != item) 
	{
		iItem = groups->GetListboxItem(item);
		if (LB_ERR == iItem) 
			return FALSE;
	}
	
	BOOL resultOk = (LB_ERR != SendMessage(hwnd, LB_SETCURSEL, (WPARAM)iItem, 0L));
	if (LB_ERR == iItem) resultOk = TRUE;

	if (LB_ERR != iItem)
	{
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent)
			SendMessage(hParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), LBN_SELCHANGE), (LPARAM)hwnd);
	}
	return resultOk;
}

BOOL Listbox::GetIndex(SetupListboxItem *item, INT *iItem)
{
	if (NULL == iItem) 
		return FALSE;
		
	*iItem = (NULL != item && NULL != groups) ? groups->GetListboxItem(item) : LB_ERR;
	return (LB_ERR != *iItem);
}

INT_PTR Listbox::KeyToItem(INT vKey, INT iCaret)
{
	SetupListboxItem *item;
	HRESULT hr = groups->FindListboxItem(iCaret, &item);
	if (FAILED(hr)) return -1;

	RECT itemRect;
	GetItemRect(iCaret, &itemRect);
	INT_PTR result = item->KeyToItem(this, &itemRect, vKey);
	if (-1 != result) return result;

	INT iTarget, iCount;
	
	switch(vKey)
	{
		case VK_UP:
		case VK_LEFT:
			iTarget = GetPrevEnabledItem(iCaret, NULL);
			if (LB_ERR != iTarget) return iTarget;

			SendMessage(hwnd, LB_SETTOPINDEX, (WPARAM)0, 0L);
			SendMessage(hwnd, LB_SETCARETINDEX, (WPARAM)iCaret, FALSE);
			return -2;
			
		case VK_DOWN:
		case VK_RIGHT:
			iTarget = GetNextEnabledItem(iCaret, NULL);
			if (LB_ERR != iTarget) return iTarget;
			
			SendMessage(hwnd, LB_SETTOPINDEX, (WPARAM)iCaret, 0L);
			return -2;
				
		case VK_HOME:
			if (iCaret > 0)
			{
				SendMessage(hwnd, LB_SETTOPINDEX, (WPARAM)0, 0L);
				iTarget = GetNextEnabledItem(-1, NULL);
				if (iTarget >= iCaret) iTarget = LB_ERR;
				if (LB_ERR != iTarget) return iTarget;
                SendMessage(hwnd, LB_SETCARETINDEX, (WPARAM)iCaret, FALSE);
			}
			return -2;

		case VK_PRIOR:
			if (iCaret > 0)
			{
				INT iTop = (INT)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0L);
				if (iTop == iCaret)
				{
					INT iPage = iCaret - GetPageCount() + 1;
					iTop =  (iPage <= 0) ? 0 : (iPage - 1);
				}

				iTarget = GetPrevEnabledItem(iTop + 1, NULL);

				if (LB_ERR == iTarget)
				{
					SendMessage(hwnd, LB_SETTOPINDEX, (WPARAM)0, 0L);
					iTarget = GetNextEnabledItem(iTop, NULL);
					if (iTarget > iCaret) iTarget = LB_ERR;
				}
				
				if (LB_ERR != iTarget) return iTarget;
				SendMessage(hwnd, LB_SETCARETINDEX, (WPARAM)iCaret, FALSE);
			}
			return -2;	

		case VK_END:
			iCount  = (INT)SendMessage(hwnd, LB_GETCOUNT, 0, 0L);
			if (iCount > 0 && iCaret != (iCount - 1))
			{				
				SendMessage(hwnd, LB_SETTOPINDEX, (WPARAM)(iCount - 1), 0L);
				iTarget = GetPrevEnabledItem(iCount, NULL);
				if (iTarget <= iCaret) iTarget = LB_ERR;
				if (LB_ERR != iTarget) return iTarget;
				SendMessage(hwnd, LB_SETCARETINDEX, (WPARAM)iCaret, FALSE);
			}
			return -2;

		case VK_NEXT:
			iCount  = (INT)SendMessage(hwnd, LB_GETCOUNT, 0, 0L);
			if (iCount > 0 && iCaret != (iCount - 1))
			{
				INT iPage = GetPageCount();
				INT iBottom = (INT)SendMessage(hwnd, LB_GETTOPINDEX, 0, 0L) + iPage - 1;
				if (iBottom == iCaret)
				{
					iBottom += iPage;
					if (iBottom >= iCount) iBottom = iCount -1;
				}
				iTarget = GetNextEnabledItem(iBottom - 1, NULL);
				if (LB_ERR == iTarget)
				{
					SendMessage(hwnd, LB_SETTOPINDEX, (WPARAM)(iCount - 1), 0L);
					iTarget = GetPrevEnabledItem(iBottom, NULL);
					if (iTarget < iCaret) iTarget = LB_ERR;
				}

				if (LB_ERR != iTarget) return iTarget;
				SendMessage(hwnd, LB_SETCARETINDEX, (WPARAM)iCaret, FALSE);
			}
			return -2;
	}
	
	return result;
}

INT_PTR Listbox::CharToItem(INT vKey, INT caretPos)
{
	return -2;
	//SetupListboxItem *item;
	//HRESULT hr = groups->FindListboxItem(caretPos, &item);
	//return (SUCCEEDED(hr)) ? item->CharToItem(this, vKey) : -1;
}

INT Listbox::GetCheckboxThemeState(BOOL checked, UINT state)
{	
	if (FALSE != checked)
	{
		if (0 != (ODS_DISABLED & state)) return CBS_CHECKEDDISABLED;
		if (0 != (ODS_HOTLIGHT & state)) return CBS_CHECKEDHOT;
		if (0 != (ODS_SELECTED & state)) return CBS_CHECKEDPRESSED;
		return CBS_CHECKEDNORMAL;
	}
	
	if (0 != (ODS_DISABLED & state)) return  CBS_UNCHECKEDDISABLED;
	if (0 != (ODS_HOTLIGHT & state)) return  CBS_UNCHECKEDHOT;
	if (0 != (ODS_SELECTED & state)) return  CBS_UNCHECKEDPRESSED;
	return CBS_UNCHECKEDNORMAL;
}

BOOL Listbox::DrawCheckbox(HDC hdc, BOOL checked, UINT state, const RECT *pRect, const RECT *pClipRect)
{
	if (NULL != buttonTheme)
	{
		INT stateId = GetCheckboxThemeState(checked, state);
		if (SUCCEEDED(UxDrawThemeBackground(buttonTheme, hdc, BP_CHECKBOX, stateId, pRect, pClipRect)))
			return TRUE;
	}

	UINT stateId = DFCS_BUTTONCHECK;
	if (FALSE != checked) stateId |= DFCS_CHECKED;
	if (0 != (ODS_DISABLED & state)) stateId |= DFCS_INACTIVE;
	if (0 != (ODS_HOTLIGHT & state)) stateId |= DFCS_HOT;
	if (0 != (ODS_SELECTED & state)) stateId |= DFCS_PUSHED;

	return DrawFrameControl(hdc, (LPRECT)pRect,DFC_BUTTON, stateId);
}

BOOL Listbox::GetCheckboxMetrics(HDC hdc, BOOL checked, UINT state, SIZE *pSize)
{
	if (NULL != buttonTheme)
	{
		HDC hdcMine = NULL;

		if (NULL == hdc)
		{
			hdcMine = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
			hdc = hdcMine;
		}

		INT stateId = GetCheckboxThemeState(checked, state);
		HRESULT hr = UxGetThemePartSize(buttonTheme, hdc, BP_CHECKBOX, 
								stateId, NULL, TS_DRAW, pSize);

		if (NULL != hdcMine)
			ReleaseDC(hwnd, hdcMine);
		if (SUCCEEDED(hr)) return TRUE;
	}
	pSize->cx = 13;
	pSize->cy = 13;
	return TRUE;
}

void Listbox::OnDestroy()
{
	delete(this);
}


INT Listbox::HitTest(POINT pt, RECT *prcItem)
{
	RECT itemRect;
	INT itemId = (INT)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELONG(pt.x, pt.y));

	if (LB_ERR == itemId ||
		LB_ERR == SendMessage(hwnd, LB_GETITEMRECT, (WPARAM)itemId, (LPARAM)&itemRect) ||
		FALSE == PtInRect(&itemRect, pt))
	{
		return LB_ERR;
	}

	if (NULL != prcItem)
		CopyRect(prcItem, &itemRect);

	return itemId;
}

void Listbox::GetItemRect(INT itemId, RECT *prcItem)
{
	if (LB_ERR == ::SendMessage(hwnd, LB_GETITEMRECT, (WPARAM)itemId, (LPARAM)prcItem))
		SetRectEmpty(prcItem);
}

BOOL Listbox::DoDragAndDrop(UINT mouseEvent, UINT mouseFlags, POINT pt)
{
	if (WM_MOUSEMOVE == mouseEvent && 
		0 != ((MK_LBUTTON | MK_MBUTTON | MK_RBUTTON) & mouseFlags))
	{
		flags |= SLF_DRAGMOVE;
	}
	else
	{
		flags &= ~SLF_DRAGMOVE;
	}
	return (0 != (SLF_DRAGMOVE & flags));
}

void Listbox::OnMouseEvent(UINT mouseEvent, UINT mouseFlags, POINTS pts, BOOL fDefaultHandler, ITEMMOUSEPROC proc)
{
	POINT pt;
	POINTSTOPOINT(pt, pts);
	SetupListboxItem *item;
	RECT itemRect;

	if (LB_ERR != capturedId)
	{
		if (SUCCEEDED(groups->FindListboxItem(capturedId, &item)))
		{
			GetItemRect(capturedId, &itemRect);
			if (FALSE == ((item->*proc)(this, &itemRect, mouseFlags, pt)))
			{
				CallPrevProc(mouseEvent, (WPARAM)mouseFlags, *((LPARAM*)&pts));
			}
			return;
		}
		capturedId = LB_ERR;
	}

	if (DoDragAndDrop(mouseEvent, mouseFlags, pt))
		return;
	
	INT itemId = HitTest(pt, &itemRect);
	if (mouseoverId != itemId)
	{
		if (SUCCEEDED(groups->FindListboxItem(mouseoverId, &item)))
		{
			RECT leaveRect;
			GetItemRect(mouseoverId, &leaveRect);
			item->MouseLeave(this, &leaveRect);
		}
		mouseoverId = itemId;

		TRACKMOUSEEVENT tm;
		tm.cbSize = sizeof(TRACKMOUSEEVENT);
		tm.hwndTrack = hwnd;
		tm.dwFlags = TME_LEAVE;
		if (LB_ERR == mouseoverId)
			tm.dwFlags |= TME_CANCEL;
		TrackMouseEvent(&tm);
	}

	if (LB_ERR != mouseoverId)
	{
		if (SUCCEEDED(groups->FindListboxItem(mouseoverId, &item)))
		{		
			BOOL callListbox = FALSE;
			if (FALSE != item->IsDisabled())
			{
				switch(mouseEvent)
				{
					case WM_LBUTTONUP:
					case WM_RBUTTONUP:
					case WM_MBUTTONUP:
					case 0x020C /*WM_XBUTTONUP*/:
						callListbox = TRUE;
						break;
				}
			}
			else 
			{
				if (FALSE == ((item->*proc)(this, &itemRect, mouseFlags, pt)))
				{
					callListbox = TRUE;
				}
			}
			
			if (FALSE != callListbox)
			{
				CallPrevProc(mouseEvent, (WPARAM)mouseFlags, *((LPARAM*)&pts));
			}
			return;
		}
	}

	if (FALSE != fDefaultHandler)
	{
		CallPrevProc(mouseEvent, (WPARAM)mouseFlags, *((LPARAM*)&pts));
	}
}

void Listbox::OnMouseLeave()
{
	if (LB_ERR != mouseoverId)
	{
		SetupListboxItem *item;
		INT itemId = mouseoverId;
		mouseoverId = LB_ERR;
		if (SUCCEEDED(groups->FindListboxItem(itemId, &item)))
		{
			RECT itemRect;
			GetItemRect(itemId, &itemRect);
			if (item->MouseLeave(this, &itemRect))
				return;
		}
	}
	CallPrevProc(WM_MOUSELEAVE, 0, 0L);
}

void Listbox::SetCapture(SetupListboxItem *item)
{	
	INT prevCapturedId = capturedId;
	capturedId = (NULL != item) ? groups->GetListboxItem(item) : LB_ERR;

	NotifyReleaseCapture(prevCapturedId, item);
	
	if (LB_ERR != capturedId && ::GetCapture() != hwnd)
		::SetCapture(hwnd);
}

SetupListboxItem *Listbox::GetCapture()
{
	SetupListboxItem *capturedItem;
	if (LB_ERR == capturedId || FAILED(groups->FindListboxItem(capturedId, &capturedItem)))
		capturedItem = NULL;
	
	return capturedItem;
}

void Listbox::ReleaseCapture()
{
	if (LB_ERR != capturedId)
	{
		INT prevCapturedId = capturedId;
		capturedId = LB_ERR;
		
		NotifyReleaseCapture(prevCapturedId, NULL);
		if (::GetCapture() == hwnd)
			::ReleaseCapture();
	}
}

void Listbox::NotifyReleaseCapture(INT itemId, SetupListboxItem *itemGain)
{
	if (LB_ERR == itemId)
		return;
	
	SetupListboxItem *item;
	if (SUCCEEDED(groups->FindListboxItem(itemId, &item)))
	{
		RECT itemRect;
		GetItemRect(itemId, &itemRect);
		item->CaptureChanged(this, &itemRect, itemGain);
	}
}

void Listbox::OnCaptureChanged(HWND hwndGained)
{
	if (hwnd != hwndGained && LB_ERR != capturedId)
	{
		INT prevCapturedId = capturedId;
		capturedId = LB_ERR;
		NotifyReleaseCapture(prevCapturedId, NULL);
	}
}

BOOL Listbox::InvalidateRect(const RECT *prcInvalidate, BOOL fErase)
{
	return ::InvalidateRect(hwnd, prcInvalidate, fErase);
}

BOOL Listbox::InvalidateItem(INT itemId, BOOL fErase)
{	
	if (itemId < 0) return FALSE;

	RECT itemRect;
	if (LB_ERR == SendMessage(hwnd, LB_GETITEMRECT, (WPARAM)itemId, (LPARAM)&itemRect))
		return FALSE;
	
	return ::InvalidateRect(hwnd, &itemRect, fErase);
}

void Listbox::OnEraseBkGround(HDC hdc)
{
	INT iCount = (INT)SendMessage(hwnd, LB_GETCOUNT, 0, 0L);
	RECT clientRect, itemRect;
	
	GetClientRect(hwnd, &clientRect);
	if (iCount > 0 && 
		LB_ERR != SendMessage(hwnd, LB_GETITEMRECT, (WPARAM)(iCount - 1), (LPARAM)&itemRect))
	{
		clientRect.top = itemRect.top;
	}
	
	if (clientRect.top < clientRect.bottom)
	{
		HBRUSH hb = NULL;
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent)
		{
			hb = (HBRUSH)SendMessage(hParent, WM_CTLCOLORLISTBOX, (WPARAM)hdc, (LPARAM)hwnd);
		}
		if (NULL == hb)
		{
			hb = GetSysColorBrush(COLOR_WINDOW);
		}
		FillRect(hdc, &clientRect, hb);
	}
}

void Listbox::OnCommand(INT commandId, INT eventId, HWND hControl)
{
	if (NULL == hControl)
	{
		SetupListboxItem *item = GetSelection();
		if (NULL != item)
		{
			item->Command(this, commandId, eventId);
		}

	}
}
SetupImage *Listbox::GetExpandboxImage(HDC hdc, BOOL fExpanded)
{
	if (fExpanded)
	{
		if (NULL == expandedImage)
			expandedImage = SetupImage::CreateFromPluginBitmap(hdc, L"gen_ml.dll", MAKEINTRESOURCE(137), 3);
		return expandedImage;
	}
		
	if (NULL == collapsedImage)
		collapsedImage = SetupImage::CreateFromPluginBitmap(hdc, L"gen_ml.dll", MAKEINTRESOURCE(135), 3);
	return collapsedImage;
}

BOOL Listbox::DrawExpandbox(HDC hdc, BOOL fExpanded, const RECT *pRect, COLORREF rgbBk, COLORREF rgbFg)
{
	SetupImage *image = GetExpandboxImage(hdc, fExpanded);
	return (NULL != image) ? 
			image->DrawImage(hdc, pRect->left, pRect->top, 
							pRect->right - pRect->left, pRect->bottom- pRect->top, 
							0, 0, rgbBk, rgbFg) 
			: FALSE;
}

BOOL Listbox::GetExpandboxMetrics(HDC hdc, BOOL fExpanded, SIZE *pSize)
{
	SetupImage *image = GetExpandboxImage(hdc, fExpanded);
	return (NULL != image) ? image->GetSize(pSize) : FALSE;
}

HMENU Listbox::GetContextMenu(UINT menuId)
{
	HMENU baseMenu = WASABI_API_LOADMENUW(IDR_SETUPMENU);
	if (NULL == baseMenu) 
		return NULL;

	switch(menuId)
	{
		case menuGroupContext:
			return GetSubMenu(baseMenu, 0);
	}
	return NULL;
}

LRESULT Listbox::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:
			OnDestroy();
			return 0;
		case WM_MOUSEMOVE:		
			OnMouseEvent(uMsg, (UINT)wParam, MAKEPOINTS(lParam), TRUE, &SetupListboxItem::MouseMove); 
			return 0;
		case WM_LBUTTONDOWN:
			OnMouseEvent(uMsg, (UINT)wParam, MAKEPOINTS(lParam), FALSE, &SetupListboxItem::LButtonDown);
			return 0;
		case WM_LBUTTONUP:
			OnMouseEvent(uMsg, (UINT)wParam, MAKEPOINTS(lParam), TRUE, &SetupListboxItem::LButtonUp);
			return 0;
		case WM_LBUTTONDBLCLK:
			OnMouseEvent(uMsg, (UINT)wParam, MAKEPOINTS(lParam), FALSE, &SetupListboxItem::LButtonDblClk);
			return 0;
		case WM_RBUTTONDOWN:
			OnMouseEvent(uMsg, (UINT)wParam, MAKEPOINTS(lParam), FALSE, &SetupListboxItem::RButtonDown);
			return 0;
		case WM_RBUTTONUP:
			OnMouseEvent(uMsg, (UINT)wParam, MAKEPOINTS(lParam), TRUE, &SetupListboxItem::RButtonUp);
			return 0;
		case WM_MOUSELEAVE:
			OnMouseLeave();
			return 0;
		case WM_CAPTURECHANGED:
			OnCaptureChanged((HWND)lParam);
			break;
		case WM_ERASEBKGND:
			OnEraseBkGround((HDC)wParam);
			return TRUE;
		case WM_COMMAND:
			OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			return 0;
	}

	return CallPrevProc(uMsg, wParam, lParam);
}

static LRESULT WINAPI Listbox_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	Listbox *list = GetList(hwnd);
	if (NULL != list)
		return list->WindowProc(uMsg, wParam, lParam);

	return (IsWindowUnicode(hwnd)) ? 
		DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
		DefWindowProcA(hwnd, uMsg, wParam, lParam);

}
