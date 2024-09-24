#include "main.h"
#include "./listWidgetInternal.h"

#include <strsafe.h>

#define TOOLTIP_MARGIN_LEFT_DLU			3
#define TOOLTIP_MARGIN_TOP_DLU			1
#define TOOLTIP_MARGIN_RIGHT_DLU		3
#define TOOLTIP_MARGIN_BOTTOM_DLU		1

#define TOOLTIP_DELAY_INITIAL			1000
#define TOOLTIP_DELAY_RESHOW			400			

static ATOM LISTWIDGETTOOLTIP_PROP = 0;

static LRESULT WINAPI 
ListWidgetTooltip_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct ListWidgetTooltip
{
	HWND window;
	HWND owner;
	BOOL active;
	POINT position;
	wchar_t *buffer;
	ListWidgetItem *item;
	ListWidgetItemPart part;
	RECT partRect;
	BOOL blockLocationChange;
	WNDPROC originalProc;
	DWORD showTime;
} ListWidgetTooltip;


static HWND
ListWidget_TooltipCreateWindow(HWND owner, WNDPROC *originalProc, HANDLE windowProperty)
{
	HWND hwnd;
	TOOLINFO ti;

	hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOPARENTNOTIFY | WS_EX_TRANSPARENT | WS_EX_LAYERED, 
							TOOLTIPS_CLASS, NULL, WS_CLIPSIBLINGS | WS_POPUP | TTS_NOANIMATE | TTS_ALWAYSTIP,
							CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
							owner, NULL, NULL, NULL);

	if ( hwnd == NULL )
		return NULL;

	MLSkinWindow2(Plugin_GetLibraryWindow(), hwnd, SKINNEDWND_TYPE_TOOLTIP, 
					SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	if (NULL != originalProc)
	{
		if (0 == LISTWIDGETTOOLTIP_PROP)
			 LISTWIDGETTOOLTIP_PROP = GlobalAddAtom(TEXT("ListWidgetTooltipProp"));

		if (0 != LISTWIDGETTOOLTIP_PROP)
		{
			*originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC,
										(LONGX86)(LONG_PTR)ListWidgetTooltip_WindowProc);
			if (NULL != *originalProc && 
				FALSE == SetProp(hwnd, MAKEINTATOM(LISTWIDGETTOOLTIP_PROP), windowProperty))
			{
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)*originalProc);
				*originalProc = NULL;
			}
		}
		else
			*originalProc = NULL;
	}

	SendMessage(hwnd, CCM_SETVERSION, 6, 0L);
	SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize   = sizeof(ti);
	ti.hwnd     = owner;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	ti.uFlags   = 0/*TTF_TRACK | TTF_ABSOLUTE*/;
	
	SendMessage(hwnd, TTM_ADDTOOL, 0, (LPARAM)&ti);

	return hwnd;
}

ListWidgetTooltip*
ListWidget_TooltipCreate(HWND hwnd)
{
	ListWidgetTooltip *tooltip;

	tooltip = (ListWidgetTooltip*)malloc(sizeof(ListWidgetTooltip));
	if (NULL == tooltip)
		return FALSE;

	ZeroMemory(tooltip, sizeof(ListWidgetTooltip));

	tooltip->window = ListWidget_TooltipCreateWindow(hwnd, &tooltip->originalProc, tooltip);
	if (NULL == tooltip->window)
	{
		ListWidget_TooltipDestroy(tooltip);
		return NULL;
	}

	tooltip->owner = hwnd;
	ListWidget_TooltipFontChanged(tooltip);
	
	return tooltip;
}

void
ListWidget_TooltipDestroy(ListWidgetTooltip *tooltip)
{
	if (NULL != tooltip)
	{
		if (NULL != tooltip->window)
			DestroyWindow(tooltip->window);

		String_Free(tooltip->buffer);
		
		free(tooltip);
	}
}

void
ListWidget_TooltipFontChanged(ListWidgetTooltip *tooltip)
{
	WidgetStyle *style;
	RECT marginsRect;
	HDC hdc;
	long maxWidth;

	if (NULL == tooltip)
		return;

	if (NULL == tooltip->window)
	{
		// attempt to recover...
		tooltip->window = ListWidget_TooltipCreateWindow(tooltip->owner, &tooltip->originalProc, tooltip);
		if (NULL == tooltip->window)
			return;

		tooltip->active = FALSE;
		tooltip->part = ListWidgetItemPart_None;
		SetRectEmpty(&tooltip->partRect);
	}


	style = WIDGET_GET_STYLE(tooltip->owner);
	if (NULL == style)
		return;

	hdc = GetDCEx(tooltip->window, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{
		HFONT font = (HFONT)SendMessage(tooltip->window, WM_GETFONT, 0, 0L);
		HFONT prevFont = SelectFont(hdc, font);

		maxWidth = Graphics_GetAveStrWidth(hdc, 36);

		SelectFont(hdc, prevFont);
		ReleaseDC(tooltip->window, hdc);
	}
	else
		maxWidth = 100;


	SendMessage(tooltip->window, TTM_SETMAXTIPWIDTH, 0, (LPARAM)maxWidth);

	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(marginsRect.left, style, TOOLTIP_MARGIN_LEFT_DLU, 3);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(marginsRect.top, style, TOOLTIP_MARGIN_TOP_DLU, 2);
	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(marginsRect.right, style, TOOLTIP_MARGIN_RIGHT_DLU, 3);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(marginsRect.bottom, style, TOOLTIP_MARGIN_BOTTOM_DLU, 2);

	SendMessage(tooltip->window, TTM_SETMARGIN, 0, (LPARAM)&marginsRect);
}

void
ListWidget_TooltipRelayMouseMessage(ListWidgetTooltip *tooltip, unsigned int message, unsigned int vKeys, const POINT *cursor)
{
	if (NULL != tooltip &&
		NULL != tooltip->window &&
		FALSE != tooltip->active)
	{
		MSG msg;

		msg.hwnd = tooltip->owner;
		msg.message = message;
		msg.wParam = (WPARAM)vKeys;
		msg.lParam = MAKELPARAM(cursor->x, cursor->y);

		SendMessage(tooltip->window, TTM_RELAYEVENT, 0, (LPARAM)&msg);
	}
}

void
ListWidget_TooltipHide(ListWidgetTooltip *tooltip)
{
	if (NULL == tooltip || 
		NULL == tooltip->window ||
		FALSE == tooltip->active)
	{
		return;
	}

	tooltip->active =  FALSE;
	tooltip->part = ListWidgetItemPart_None;
	SetRectEmpty(&tooltip->partRect);

	SendMessage(tooltip->window, TTM_ACTIVATE, FALSE, 0);
}

BOOL
ListWidget_TooltipActivate(ListWidgetTooltip *tooltip, const RECT *rect)
{
	TOOLINFO ti;
	POINT origin;

	if (NULL == tooltip || 
		NULL == tooltip->window)
	{
		return FALSE;
	}
	
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.hwnd = tooltip->owner;
	ti.uId = 0;

			
	if (FALSE == SendMessage(tooltip->window, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
		return FALSE;
	
	if (FALSE != tooltip->active)
	{
		tooltip->active = FALSE;
		SendMessage(tooltip->window, TTM_ACTIVATE, FALSE, 0);
	}
	else
		SendMessage(tooltip->window, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELPARAM(TOOLTIP_DELAY_INITIAL, 0)); 
	
	
	CopyRect(&ti.rect, rect);
	if (FALSE != ListWidget_GetViewOrigin(tooltip->owner, &origin))
		OffsetRect(&ti.rect, origin.x, origin.y);	
	
	ti.lParam = NULL;
	ti.lpszText = LPSTR_TEXTCALLBACK;
		
	SendMessage(tooltip->window, TTM_SETTOOLINFO, 0, (LPARAM)&ti);
	

	if (FALSE == tooltip->active)
	{
		KillTimer(tooltip->window, 4);
		SendMessage(tooltip->window, TTM_ACTIVATE, TRUE, 0);
		tooltip->active = TRUE;
	}

	return TRUE;
}


BOOL
ListWidget_TooltipUpdate(ListWidgetTooltip *tooltip, ListWidgetItem *item, 
						 ListWidgetItemPart part, const RECT *partRect)
{
	BOOL tooltipValid;

	if (NULL == tooltip)
		return FALSE;

	if(FALSE == tooltip->active && NULL == item)
		return FALSE;

	if (0 != (ListWidgetItemPart_Activity & part))
	{
		part &= ~ListWidgetItemPart_Activity;
		part |= ListWidgetItemPart_Frame;
	}

	tooltipValid = (NULL != item && ListWidgetItemPart_None != part);
	if (tooltip->item == item &&
		tooltip->part == part &&
		(FALSE != (FALSE != tooltipValid) ? 
							EqualRect(&tooltip->partRect, partRect) : 
							IsRectEmpty(&tooltip->partRect)))
	{
				
		return FALSE;
	}

	tooltip->item = item;
	tooltip->part = part;

	if (FALSE != tooltipValid)
		CopyRect(&tooltip->partRect, partRect);
	else
		SetRectEmpty(&tooltip->partRect);
	
	if (FALSE == tooltipValid)
	{
		ListWidget_TooltipHide(tooltip);
		return FALSE;
	}

	return ListWidget_TooltipActivate(tooltip, &tooltip->partRect);
}

static BOOL
ListWidget_TooltipFormatTip(ListWidget *self, HWND hwnd, 
							   const RECT *rect, wchar_t *buffer, size_t bufferMax)
{
	WidgetStyle *style;
	ListWidgetItem *item;
	ListWidgetItemPart part;
	ListWidgetItemMetric metrics;
	RECT partRect;
	POINT pt, origin;

	if (NULL == self || NULL == rect)
		return FALSE;
	
	style = WIDGET_GET_STYLE(hwnd);
	if (NULL == style)
		return FALSE;

	if (FALSE == ListWidget_GetItemMetrics(style, &metrics))
		return FALSE;

	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}
	
	pt.x = (rect->left + (rect->right - rect->left)/2) - origin.x;
	pt.y = (rect->top + (rect->bottom - rect->top)/2) - origin.y;

		
	item = ListWidget_GetItemFromPoint(self, pt);
	if (NULL == item)
		return FALSE;

	
	part = ListWidgetItemPart_Frame | 
			ListWidgetItemPart_Command | 
			ListWidgetItemPart_Spacebar | 
			ListWidgetItemPart_Title;
	
	part = ListWidget_GetItemPartFromPoint(self, item, &metrics, pt, part, &partRect);
	switch(part)
	{
		case ListWidgetItemPart_Command:
			CopyRect(&partRect, rect);
			OffsetRect(&partRect, -origin.x - item->rect.left, -origin.y - item->rect.top);
			return ListWidget_FormatItemCommandTip(self, item, &partRect, buffer, bufferMax);
		
		case ListWidgetItemPart_Activity:
		case ListWidgetItemPart_Frame:
			return ListWidget_FormatItemTip(self, item, buffer, bufferMax);

		case ListWidgetItemPart_Spacebar:
			return ListWidget_FormatItemSpaceTip(self, item, buffer, bufferMax);

		case ListWidgetItemPart_Title:
			return ListWidget_FormatItemTitleTip(self, item, buffer, bufferMax);
	}

	return FALSE;
}

static void
ListWidget_TooltipGetDispInfo(ListWidget *self, ListWidgetTooltip *tooltip, NMTTDISPINFO *dispInfo)
{
	TOOLINFO ti;

	if (NULL == dispInfo)
		return;

	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.hwnd = tooltip->owner;
	ti.uId = dispInfo->hdr.idFrom;

	String_Free(tooltip->buffer);
	tooltip->buffer = NULL;
	
	if (FALSE != SendMessage(dispInfo->hdr.hwndFrom, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
	{
		wchar_t buffer[4096] = {0};
		if (FALSE != ListWidget_TooltipFormatTip(self, tooltip->owner, &ti.rect, buffer, ARRAYSIZE(buffer)))
			tooltip->buffer = String_Duplicate(buffer);
	}
	
	dispInfo->lpszText = tooltip->buffer;
	dispInfo->szText[0] = L'\0';
	dispInfo->hinst = NULL;
	dispInfo->uFlags = TTF_DI_SETITEM;
}

BOOL
ListWidget_TooltipProcessNotification(ListWidget *self, ListWidgetTooltip *tooltip, NMHDR *pnmh, LRESULT *result)
{
	if (NULL == tooltip ||
		NULL == pnmh ||
		pnmh->hwndFrom != tooltip->window)
	{
		return FALSE;
	}

	switch(pnmh->code)
	{
		case TTN_GETDISPINFO:
			ListWidget_TooltipGetDispInfo(self, tooltip, (NMTTDISPINFO*)pnmh);
			break;

		case TTN_SHOW:
			if (0 == (WS_VISIBLE & GetWindowStyle(tooltip->window)))
				tooltip->showTime = GetTickCount();

			if (FALSE != tooltip->active)
			{
				SendMessage(tooltip->window, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELPARAM(TOOLTIP_DELAY_RESHOW, 0)); 
			}
			break;
	}

	return TRUE;
}

ListWidgetItem *
ListWidget_TooltipGetCurrent(ListWidgetTooltip *tooltip, ListWidgetItemPart *part, RECT *partRect)
{
	if (NULL == tooltip ||
		FALSE == tooltip->active ||
		NULL == tooltip->item)
	{
		return NULL;
	}
	
	if (NULL != part)
		*part = tooltip->part;

	if (NULL != partRect)
		CopyRect(partRect, &tooltip->partRect);

	return tooltip->item;
}

BOOL
ListWidget_TooltipGetChanged(ListWidgetTooltip *tooltip, ListWidgetItem *item,
							 ListWidgetItemPart part, const RECT *partRect)
{
	if (NULL == tooltip)
		return FALSE;

	if (tooltip->item != item)
		return TRUE;

	if (NULL == item)
		return FALSE;

	if (tooltip->part != part)
		return TRUE;

	if (FALSE == EqualRect(&tooltip->partRect, partRect))
		return TRUE;

	return FALSE;
}

BOOL
ListWidget_TooltipUpdateText(ListWidget *self, ListWidgetTooltip *tooltip, ListWidgetItem *item, TooltipUpdateReason reason)
{
	TOOLINFO ti;
	ListWidgetItemPart mask;
	unsigned int windowStyle;
	unsigned long showTimeout, currentTime;

	if (NULL == self ||
		NULL == tooltip ||
		NULL == tooltip->active ||
		NULL == tooltip->window)
	{
		return FALSE;
	}

	if (NULL != item && tooltip->item != item)
		return FALSE;

	windowStyle = GetWindowStyle(tooltip->window);
	if (0 == (WS_VISIBLE & windowStyle))
		return FALSE;

	showTimeout = (unsigned long)SendMessage(tooltip->window, TTM_GETDELAYTIME, (WPARAM)TTDT_AUTOPOP, 0L);
	currentTime = GetTickCount();
	if (currentTime < tooltip->showTime)
		return FALSE;
	
	currentTime -= tooltip->showTime;
	if (showTimeout > (currentTime + 10))
		showTimeout -= currentTime;
	else
		return FALSE;
	
	KillTimer(tooltip->window, 4);

	switch(reason)
	{
		case Tooltip_DeviceTitleChanged:
		case Tooltip_DeviceModelChanged:
		case Tooltip_DeviceStatusChanged:
			mask = ListWidgetItemPart_Frame | 
				   ListWidgetItemPart_Activity |
				   ListWidgetItemPart_Title;

			if (0 == (mask & tooltip->part))
				return FALSE;

			break;

		case Tooltip_DeviceSpaceChanged:
			mask = ListWidgetItemPart_Frame | 
				   ListWidgetItemPart_Activity |
				   ListWidgetItemPart_Spacebar;

			if (0 == (mask & tooltip->part))
				return FALSE;

			break;

		/*case Tooltip_DeviceActivityChanged:
			mask = ListWidgetItemPart_Frame | 
				   ListWidgetItemPart_Activity;

			if (0 == (mask & tooltip->part))
				return FALSE;

			break;*/
	}


	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.hwnd = tooltip->owner;
	ti.uId = 0;
	ti.lpszText = LPSTR_TEXTCALLBACK;

	tooltip->blockLocationChange = TRUE;
	SendMessage(tooltip->window, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	tooltip->blockLocationChange = FALSE;

	SetTimer(tooltip->window, 4, showTimeout, 0);
	return TRUE;
}

static LRESULT WINAPI 
ListWidgetTooltip_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ListWidgetTooltip *tooltip;
	
	tooltip = (ListWidgetTooltip*)GetProp(hwnd, MAKEINTATOM(LISTWIDGETTOOLTIP_PROP));
	if (NULL == tooltip ||
		NULL == tooltip->originalProc)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	switch(uMsg)
	{
		case WM_DESTROY:
			RemoveProp(hwnd, MAKEINTATOM(LISTWIDGETTOOLTIP_PROP));
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)tooltip->originalProc);
			CallWindowProc(tooltip->originalProc, hwnd, uMsg, wParam, lParam);
			tooltip->originalProc = NULL;
			tooltip->window = NULL;
			break;
		case WM_WINDOWPOSCHANGING:
			if (FALSE != tooltip->blockLocationChange)
			{
				WINDOWPOS *pwp;
				pwp = (WINDOWPOS*)lParam;
				if (NULL != pwp)
				{
					pwp->flags |= SWP_NOMOVE;
				}
			}
			break;
	}
	
	return CallWindowProc(tooltip->originalProc, hwnd, uMsg, wParam, lParam);
}