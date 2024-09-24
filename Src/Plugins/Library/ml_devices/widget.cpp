#include "main.h"
#include "./widget.h"

typedef 
enum WidgetState
{
	WIDGET_STATE_MOUSE_MOVE_TRACKED = (1 << 0),
	WIDGET_STATE_DISABLE_CHILDREN_SCROLL = (1 << 1),
} WidgetState;

DEFINE_ENUM_FLAG_OPERATORS(WidgetState);

#define WIDGET_IS_FROZEN(_widget) (0 != (_widget)->freezer)

#define WIDGET_IS_MOUSE_MOVE_TRACKED(_widget) (0 != (WIDGET_STATE_MOUSE_MOVE_TRACKED & (_widget)->state))
#define WIDGET_SET_MOUSE_MOVE_TRACK(_widget) (((_widget)->state) |= WIDGET_STATE_MOUSE_MOVE_TRACKED)
#define WIDGET_UNSET_MOUSE_MOVE_TRACK(_widget) (((_widget)->state) &= ~WIDGET_STATE_MOUSE_MOVE_TRACKED)


#define WIDGET_IS_CHILDREN_SCROLL_DISABLED(_widget) (0 != (WIDGET_STATE_DISABLE_CHILDREN_SCROLL & (_widget)->state))
#define WIDGET_SET_DISABLE_CHILDREN_SCROLL(_widget) (((_widget)->state) |= WIDGET_STATE_DISABLE_CHILDREN_SCROLL)
#define WIDGET_UNSET_DISABLE_CHILDREN_SCROLL(_widget) (((_widget)->state) &= ~WIDGET_STATE_DISABLE_CHILDREN_SCROLL)

typedef struct Widget
{
	unsigned int type;
	WidgetState state;
	const WidgetInterface *callbacks;
	void *object;
	WidgetStyle *style;
	wchar_t *text;
	HFONT font;
	SIZE viewSize;
	POINT viewOrigin;
	int	wheelCarryover;
	size_t freezer;
} Widget;

typedef struct WidgetCreateParam
{
	unsigned int type;
	const WidgetInterface *callbacks;
	void *param;
	const wchar_t *text;
} WidgetCreateParam;

#define WIDGET(_hwnd) ((Widget*)(LONGX86)GetWindowLongPtrW((_hwnd), 0))
#define WIDGET_RET_VOID(_view, _hwnd) {(_view) = WIDGET((_hwnd)); if (NULL == (_view)) return;}
#define WIDGET_RET_VAL(_view, _hwnd, _error) {(_view) = WIDGET((_hwnd)); if (NULL == (_view)) return (_error);}

#define WIDGETSTYLE(_widget) (((Widget*)(_widget))->style)
#define WIDGETOBJECT(_widget) (((Widget*)(_widget))->object)
#define WIDGETCALLBACKS(_widget) (((Widget*)(_widget))->callbacks)

static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;

static LRESULT CALLBACK 
Widget_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);


static ATOM 
Widget_GetClassAtom(HINSTANCE instance)
{
	WNDCLASSEXW klass;
	ATOM klassAtom;

	klassAtom = (ATOM)GetClassInfoExW(instance, WIDGET_WINDOW_CLASS, &klass);
	if (0 != klassAtom)
		return klassAtom;

	memset(&klass, 0, sizeof(klass));
	klass.cbSize = sizeof(klass);
	klass.style = CS_DBLCLKS;
	klass.lpfnWndProc = Widget_WindowProc;
	klass.cbClsExtra = 0;
	klass.cbWndExtra = sizeof(Widget*);
	klass.hInstance = instance;
	klass.hIcon = NULL;
	klass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	klass.hbrBackground = NULL;
	klass.lpszMenuName = NULL;
	klass.lpszClassName = WIDGET_WINDOW_CLASS;
	klass.hIconSm = NULL;
	klassAtom = RegisterClassExW(&klass);
	
	return klassAtom;
}

HWND 
Widget_CreateWindow(unsigned int type, const WidgetInterface *callbacks,
					const wchar_t *text, unsigned long windowExStyle, unsigned long windowStyle, 
					int x, int y, int width, int height, 
					HWND parentWindow, unsigned int controlId, void *param)
{
	HINSTANCE instance;
	ATOM klassAtom;
	HWND hwnd;
	WidgetCreateParam createParam;
		
	if (NULL == parentWindow || FALSE == IsWindow(parentWindow))
		return NULL;

	instance = GetModuleHandleW(NULL);
	klassAtom = Widget_GetClassAtom(instance);
	if (0 == klassAtom)
		return NULL;
	
	createParam.type = type;
	createParam.param = param;
	createParam.callbacks = callbacks;
	createParam.text = text;

	hwnd = CreateWindowExW(WS_EX_NOPARENTNOTIFY | windowExStyle, (LPCWSTR)MAKEINTATOM(klassAtom), NULL,
							WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | windowStyle,
							x, y, width, height, 
							parentWindow, (HMENU)controlId, instance, &createParam);

	return hwnd;
}


static LRESULT
Widget_DefWindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	Widget *self;
	
	self = WIDGET(hwnd);

	if (NULL != self && NULL != self->callbacks->messageProc)
	{
		LRESULT result;
		result = 0;
		if (FALSE != self->callbacks->messageProc(self->object, 
							hwnd, uMsg, wParam, lParam, &result))
		{
			return result;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void 
Widget_Freeze(Widget *self)
{
	if (NULL != self)
		self->freezer++;
}

static void 
Widget_Thaw(Widget *self)
{
	if (NULL != self && 0 != self->freezer)
		self->freezer--;
}

static INT
Widget_ScrollBarOffsetPos(HWND hwnd, INT barType, INT delta, BOOL redraw)
{
	Widget *self;
	INT position;
	SCROLLINFO scrollInfo;
	
	self = WIDGET(hwnd);

	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;

	if (FALSE == GetScrollInfo(hwnd, barType, &scrollInfo))
		return 0;

	position = scrollInfo.nPos + delta;

	if (position < scrollInfo.nMin)
		position = scrollInfo.nMin;
	else if (position > (scrollInfo.nMax - (INT)scrollInfo.nPage))
		position = scrollInfo.nMax - (INT)scrollInfo.nPage + 1;
	
	delta = position - scrollInfo.nPos;

	scrollInfo.fMask = SIF_POS;
	scrollInfo.nPos = position;
	SetScrollInfo(hwnd, barType, &scrollInfo, redraw);

	if (NULL != self)
	{
		if (SB_HORZ == barType)
			self->viewOrigin.x = -position;
		else 
			self->viewOrigin.y = -position;
	}

	return delta;
}

static BOOL 
Widget_ScrollContent(HWND hwnd, int dx, int dy, BOOL redraw)
{
	Widget *self;
	UINT scrollFlags;
	HRGN invalidRgn;
	INT scrollError;

	if (0 == dx && 0 == dy)
		return FALSE;

	self = WIDGET(hwnd);
	if (NULL != self &&
		NULL != self->callbacks &&
		NULL != self->callbacks->scrollBefore)
	{
		self->callbacks->scrollBefore(WIDGETOBJECT(self), hwnd, &dx, &dy);
		if (0 == dx && 0 == dy)
			return FALSE;
	}


	scrollFlags = (FALSE == WIDGET_IS_CHILDREN_SCROLL_DISABLED(self)) ? SW_SCROLLCHILDREN : 0;
	if (FALSE != redraw)
	{
		invalidRgn = CreateRectRgn(0, 0, 0, 0);
		scrollFlags |= SW_INVALIDATE | SW_ERASE;
	}
	else
	{
		invalidRgn = NULL;
	}
	
	scrollError = ScrollWindowEx(hwnd, -dx, -dy, NULL, NULL, invalidRgn, NULL, scrollFlags);
	if (ERROR != scrollError)
	{
		if (NULL != self && 
			NULL != self->callbacks &&
			NULL != self->callbacks->scroll)
		{
			self->callbacks->scroll(WIDGETOBJECT(self), hwnd, &dx, &dy);
		}

		if (FALSE != redraw && NULLREGION != scrollError)
		{
			RedrawWindow(hwnd, NULL, invalidRgn, 
							RDW_ERASENOW | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}

	if (NULL != invalidRgn)
		DeleteObject(invalidRgn);

	return (ERROR != scrollError);
}

static BOOL
Widget_SyncContentOrigin(HWND hwnd, BOOL redraw)
{
	Widget *self;
	RECT clientRect;
	SCROLLINFO scrollInfo;
	INT dx, dy;

	WIDGET_RET_VAL(self, hwnd, FALSE);
	
	
	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_POS;

	if (FALSE == GetClientRect(hwnd, &clientRect))
		SetRectEmpty(&clientRect);

	if (self->viewSize.cx < RECTWIDTH(clientRect))
	{
		scrollInfo.nPos = 0;
		dx = scrollInfo.nPos + self->viewOrigin.x;
		self->viewOrigin.x = 0;
	}
	else if (FALSE != GetScrollInfo(hwnd, SB_HORZ, &scrollInfo))
	{
		dx = scrollInfo.nPos + self->viewOrigin.x;
		self->viewOrigin.x = -scrollInfo.nPos;
	}
	else
		dx = 0;

	if (FALSE != GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
	{
		dy = scrollInfo.nPos + self->viewOrigin.y;
		self->viewOrigin.y = -scrollInfo.nPos;
	}
	else
		dy = 0;

	if (0 == dx && 0 == dy)
		return FALSE;

	return Widget_ScrollContent(hwnd, dx, dy, redraw);
}


static BOOL
Widget_ScrollWindow(HWND hwnd, INT dx, INT dy, BOOL redraw)
{		
	if (0 != dx)
		dx = Widget_ScrollBarOffsetPos(hwnd, SB_HORZ, dx, redraw);
	
	if (0 != dy)
		dy = Widget_ScrollBarOffsetPos(hwnd, SB_VERT, dy, redraw);

	return Widget_ScrollContent(hwnd, dx, dy, redraw);
}

static BOOL
Widget_ScrollBarAction(HWND hwnd, INT barType, INT actionLayout, INT line, BOOL redraw)
{
	INT delta;
	SCROLLINFO scrollInfo;

	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	
	if (FALSE == GetScrollInfo(hwnd, barType, &scrollInfo))
		return FALSE;
	
	switch(actionLayout)
	{
		case SB_BOTTOM:
			delta = scrollInfo.nMax - scrollInfo.nPos;
			break;
		case SB_TOP:
			delta = scrollInfo.nMin - scrollInfo.nPos;
			break;
		case SB_LINEDOWN:
			delta = line;
			break;
		case SB_LINEUP:
			delta = -line;
			break;
		case SB_PAGEDOWN:
			delta = (INT)scrollInfo.nPage;
			break;
		case SB_PAGEUP:
			delta = -(INT)scrollInfo.nPage;
			break;
		case SB_THUMBTRACK:
			delta = scrollInfo.nTrackPos - scrollInfo.nPos;
			break;
		case SB_THUMBPOSITION:
		case SB_ENDSCROLL:
			delta = 0;
			break;
		default:
			return FALSE;
	}
	
	if(0 != delta)
	{
		Widget_ScrollWindow(hwnd,  
					(SB_HORZ == barType) ? delta : 0, 
					(SB_VERT == barType) ? delta : 0,
					redraw);
	}
	else
		Widget_SyncContentOrigin(hwnd, redraw);

	return TRUE;
}

static BOOL
Widget_ScrollBarUpdate(HWND hwnd, INT barType, UINT page, INT max, BOOL redraw)
{
	Widget *self;
	SCROLLINFO scrollInfo;
	UINT windowStyle, styleFilter;

	WIDGET_RET_VAL(self, hwnd, FALSE);

	windowStyle = GetWindowStyle(hwnd);
	
	switch(barType)
	{
		case SB_HORZ:	styleFilter = WS_HSCROLL; break;
		case SB_VERT:	styleFilter = WS_VSCROLL; break;
		default:		return FALSE;
	}
	
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_PAGE | SIF_RANGE;

	if (page >= (UINT)max)
	{
		if (0 == (styleFilter & windowStyle))
			return FALSE;
		
		scrollInfo.nPage = page + 1;
		scrollInfo.nMin = 0;
		scrollInfo.nMax = max;
		scrollInfo.nPos = scrollInfo.nMin;
		scrollInfo.nTrackPos = scrollInfo.nPos;
		scrollInfo.fMask |= (SIF_POS | SIF_TRACKPOS);

		Widget_Freeze(self);
		SetScrollInfo(hwnd, barType, &scrollInfo, redraw);
		Widget_Thaw(self);

		windowStyle = GetWindowStyle(hwnd);
		if (0 != (styleFilter & windowStyle))
			SetWindowStyle(hwnd, windowStyle & ~styleFilter);

		return TRUE;
	}

	

	if (FALSE == GetScrollInfo(hwnd, barType, &scrollInfo))
	{
		if (ERROR_NO_SCROLLBARS == GetLastError())
		{			
			scrollInfo.nPage = 0;
			scrollInfo.nMax = 0;
			scrollInfo.nMin = 0;
			scrollInfo.nPos = scrollInfo.nMin;
			scrollInfo.nTrackPos = scrollInfo.nPos;
		}
		else
			return FALSE;
	}
	
	scrollInfo.fMask = 0;
	
	if (scrollInfo.nPage != page)
	{
		scrollInfo.nPage = page;
		scrollInfo.fMask |= SIF_PAGE;
	}
	
	if (scrollInfo.nMax != max)
	{
		scrollInfo.nMax = max;
		scrollInfo.fMask |= SIF_RANGE;
	}
	
	if (0 == (styleFilter & windowStyle))
	{
		scrollInfo.fMask |= (SIF_POS | SIF_TRACKPOS);
		scrollInfo.nPos = scrollInfo.nMin;
		scrollInfo.nTrackPos = scrollInfo.nMin;
	}

	if (0 == scrollInfo.fMask)
		return FALSE;
	
	Widget_Freeze(self);
	SetScrollInfo(hwnd, barType, &scrollInfo, redraw);
	Widget_Thaw(self);

	
	if (0 == (styleFilter & windowStyle))
	{
		windowStyle = GetWindowStyle(hwnd);
		if (0 == (styleFilter & windowStyle))
			SetWindowStyle(hwnd, windowStyle | styleFilter);

		return TRUE;
	}

	return FALSE;
}

static void 
Widget_Layout(HWND hwnd, BOOL redraw)
{
	Widget *self;
	RECT rect;
	size_t iteration;

	WIDGET_RET_VOID(self, hwnd);

	iteration = 0;
	do
	{
		if (iteration++ > 2)
			break;

		if (FALSE == GetClientRect(hwnd, &rect))
			break;

		SetSize(&self->viewSize, 0, 0);
	
		if (NULL != self->callbacks->layout)
		{
			self->callbacks->layout(self->object, hwnd, self->style, &rect, &self->viewSize, redraw);
		}

		if (FALSE != IsSizeEmpty(&self->viewSize))
			SetSize(&self->viewSize, RECTWIDTH(rect), RECTHEIGHT(rect));

	}
	while(FALSE != Widget_ScrollBarUpdate(hwnd, SB_HORZ, RECTWIDTH(rect), self->viewSize.cx, redraw) ||
		  FALSE != Widget_ScrollBarUpdate(hwnd, SB_VERT, RECTHEIGHT(rect), self->viewSize.cy, redraw));

	Widget_SyncContentOrigin(hwnd, redraw);
}

static BOOL
Widget_Paint(HWND hwnd, HDC hdc, const RECT *paintRect, BOOL erase)
{
	Widget *self;
	BOOL result;
		
	self = WIDGET(hwnd);
	if (NULL == self || NULL == self->style)
		return FALSE;
			
	
	if (NULL != self->callbacks->paint)
	{
		POINT prevOrigin;
		RECT viewRect;

		CopyRect(&viewRect, paintRect);
		OffsetRect(&viewRect, -self->viewOrigin.x, -self->viewOrigin.y);

		OffsetViewportOrgEx(hdc, self->viewOrigin.x, self->viewOrigin.y, &prevOrigin);

		result = self->callbacks->paint(self->object, hwnd, 
							self->style, hdc, &viewRect, erase);

		SetViewportOrgEx(hdc, prevOrigin.x, prevOrigin.y, NULL);
	}
	else
		result = FALSE;
	
	if (FALSE == result)
	{
		if (FALSE != erase)
			result = FillRect(hdc, paintRect, WIDGETSTYLE_BACK_BRUSH(self->style));
		else
			result = TRUE;
	}

	return result;
}

static void
Widget_FocusChanged(HWND hwnd, HWND focusWindow, BOOL focusReceived)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	if (NULL != self->callbacks && 
		NULL != self->callbacks->focusChanged)
	{
		self->callbacks->focusChanged(self->object, hwnd, focusWindow, focusReceived);
	}
}

static LRESULT
Widget_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{	
	Widget *self;
	WidgetCreateParam *createParam;
	
	if (NULL == createStruct)
		return -1;

	createParam = (WidgetCreateParam*)createStruct->lpCreateParams;
	if (NULL == createParam)
		return -1;

	self = (Widget*)malloc(sizeof(Widget));
	if (NULL == self)
		return -1;

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)self) && ERROR_SUCCESS != GetLastError())
		return -1;

	memset(self, 0, sizeof(Widget));

	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
		WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");

	self->type = createParam->type;
	self->callbacks = createParam->callbacks;
	
	Widget_Freeze(self);

	if (NULL != createParam->text)
		SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)createParam->text);

	MLSkinWindow2(Plugin_GetLibraryWindow(), hwnd, SKINNEDWND_TYPE_SCROLLWND, 
				SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	if (NULL != self->callbacks->init &&
		FALSE == self->callbacks->init(hwnd, &self->object, createParam->param))
	{
		return -1;
	}
	
	Widget_Thaw(self);

	return 0;
}

static void
Widget_OnDestroy(HWND hwnd)
{
	Widget *self;

	self = WIDGET(hwnd);
	SetWindowLongPtr(hwnd, 0, 0);
	
	if (NULL == self)
		return;
	
	Widget_Freeze(self);

	if (NULL != self->callbacks->destroy)
		self->callbacks->destroy(self->object, hwnd);

	String_Free(self->text);

	free(self);
}

static void
Widget_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	
	if (NULL != BeginPaint(hwnd, &ps))
	{		
		if (FALSE == Widget_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase))
		{
			COLORREF backColor, prevBackColor;

			backColor = Graphics_GetSkinColor(WADLG_WNDBG);
			prevBackColor = SetBkColor(ps.hdc, backColor);

			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, NULL, 0, NULL);
			SetBkColor(ps.hdc, prevBackColor);
		}
		EndPaint(hwnd, &ps);
	}
}

static void 
Widget_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	{
		Widget_Paint(hwnd, hdc, &clientRect, TRUE);
	}
}

static void
Widget_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED) & windowPos->flags))
	{
		Widget *self;
		WIDGET_RET_VOID(self, hwnd);

		if (FALSE != WIDGET_IS_FROZEN(self))
			return;

		Widget_Layout(hwnd, 0 == (SWP_NOREDRAW & windowPos->flags));
	}
}

static LRESULT 
Widget_OnSetText(HWND hwnd, LPCWSTR text)
{
	Widget *self;
	WIDGET_RET_VAL(self, hwnd, FALSE);
	
	String_Free(self->text);

	if (NULL == text)
		self->text = NULL;
	else if (FALSE != IS_INTRESOURCE(text))
	{
		WCHAR buffer[4096] = {0};
		ResourceString_CopyTo(buffer, text, ARRAYSIZE(buffer));
		self->text = String_Duplicate(buffer);
	}
	else
		self->text = String_Duplicate(text);

	return TRUE;
}

static LRESULT 
Widget_OnGetText(HWND hwnd, LPWSTR buffer, size_t bufferMax)
{
	Widget *self;

	WIDGET_RET_VAL(self, hwnd, 0);

	return String_CopyTo(buffer, self->text, bufferMax);
}

static LRESULT 
Widget_OnGetTextLength(HWND hwnd)
{
	Widget *self;
	WIDGET_RET_VAL(self, hwnd, 0);

	return ( NULL != self->text) ? lstrlenW(self->text) : 0;
}


static void 
Widget_OnSetFont(HWND hwnd, HFONT font, BOOL redraw)
{
	Widget *self;

	WIDGET_RET_VOID(self, hwnd);

	self->font = font;

	if (NULL != redraw)
		InvalidateRect(hwnd, NULL, TRUE);
}

static HFONT
Widget_OnGetFont(HWND hwnd)
{
	Widget *self;
	WIDGET_RET_VAL(self, hwnd, NULL);

	return self->font;
}

static void
Widget_OnVertScroll(HWND hwnd, INT actionLayout, INT trackPosition, HWND scrollBar)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	Widget_ScrollBarAction(hwnd, SB_VERT, actionLayout, self->style->unitSize.cy, TRUE);
}

static void
Widget_OnHorzScroll(HWND hwnd, INT actionLayout, INT trackPosition, HWND scrollBar)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	Widget_ScrollBarAction(hwnd, SB_HORZ, actionLayout, self->style->unitSize.cx, TRUE);
}

static void
Widget_OnMouseWheel(HWND hwnd, INT virtualKeys, INT distance, LONG pointer_s)
{
	Widget *self;
	UINT wheelScroll;
	INT  scrollLines;
	UINT windowStyle;
	INT barType;

	WIDGET_RET_VOID(self, hwnd);

	windowStyle = GetWindowStyle(hwnd);

	if (0 != (WS_VSCROLL & windowStyle))
		barType = SB_VERT;
	else if (0 != (WS_HSCROLL & windowStyle))
		barType = SB_HORZ;
	else
		return;

	if (FALSE == SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &wheelScroll, 0))
        wheelScroll = 3; 

	if (0 == wheelScroll)
		return;
    
	if (WHEEL_PAGESCROLL == wheelScroll)
	{
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		if (SB_VERT == barType)
			wheelScroll = RECTHEIGHT(clientRect)/self->style->unitSize.cy;
		else
			wheelScroll = RECTWIDTH(clientRect)/self->style->unitSize.cx;
    }

	distance += self->wheelCarryover; 
	scrollLines = distance * (INT)wheelScroll / WHEEL_DELTA;

	self->wheelCarryover = distance - scrollLines * WHEEL_DELTA / (INT)wheelScroll;
	
	if (FALSE != Widget_ScrollWindow(hwnd, 
						(SB_HORZ == barType) ? -(scrollLines * self->style->unitSize.cx) : 0,
						(SB_VERT == barType) ? -(scrollLines * self->style->unitSize.cy) : 0,
						TRUE))
	{
		
	}

}


static void
Widget_OnMouseMove(HWND hwnd, unsigned int vKeys, long cursor_s)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	if (NULL != self->callbacks && NULL != self->callbacks->mouseMove)
	{
		BOOL processed;
		POINT cursor;
		POINTSTOPOINT(cursor, cursor_s);
		
		
		processed = self->callbacks->mouseMove(self->object, hwnd, vKeys, &cursor);

		if (FALSE == WIDGET_IS_MOUSE_MOVE_TRACKED(self))
		{
			TRACKMOUSEEVENT trackMouse;
			trackMouse.cbSize = sizeof(trackMouse);
			trackMouse.dwFlags = TME_LEAVE;
			trackMouse.hwndTrack = hwnd;
			if (FALSE != TrackMouseEvent(&trackMouse))
				WIDGET_SET_MOUSE_MOVE_TRACK(self);
		}

		if (FALSE != processed)
			return;
	}

	DefWindowProc(hwnd, WM_MOUSEMOVE, (WPARAM)vKeys, (LPARAM)cursor_s);
}

static void
Widget_OnMouseLeave(HWND hwnd)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	WIDGET_UNSET_MOUSE_MOVE_TRACK(self);

	if (NULL != self->callbacks && NULL != self->callbacks->mouseMove)
	{
		POINT cursor;
		cursor.x = 0xEFFFFFFF;
		cursor.y = 0xEFFFFFFF;
			
		if (FALSE != self->callbacks->mouseMove(self->object, hwnd, 0, &cursor))
			return;
	}
	
	DefWindowProc(hwnd, WM_MOUSELEAVE, 0, 0L);

}

static void
Widget_OnLeftButtonDown(HWND hwnd, unsigned int vKeys, long cursor_s)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);
	
	if (NULL != self->callbacks && NULL != self->callbacks->leftButtonDown)
	{
		POINT cursor;
		POINTSTOPOINT(cursor, cursor_s);
		if (FALSE != self->callbacks->leftButtonDown(self->object, hwnd, vKeys, &cursor))
			return;
	}

	DefWindowProc(hwnd, WM_LBUTTONDOWN, (WPARAM)vKeys, (LPARAM)cursor_s);

}

static void
Widget_OnLeftButtonUp(HWND hwnd, unsigned int vKeys, long cursor_s)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	if (NULL != self->callbacks && NULL != self->callbacks->leftButtonUp)
	{
		POINT cursor;
		POINTSTOPOINT(cursor, cursor_s);
		if (FALSE != self->callbacks->leftButtonUp(self->object, hwnd, vKeys, &cursor))
			return;
	}

	DefWindowProc(hwnd, WM_LBUTTONUP, (WPARAM)vKeys, (LPARAM)cursor_s);
}

static void
Widget_OnLeftButtonDblClk(HWND hwnd, unsigned int vKeys, long cursor_s)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);
	
	if (NULL != self->callbacks && NULL != self->callbacks->leftButtonDblClk)
	{
		POINT cursor;
		POINTSTOPOINT(cursor, cursor_s);
		if (FALSE != self->callbacks->leftButtonDblClk(self->object, hwnd, vKeys, &cursor))
			return;
	}
	DefWindowProc(hwnd, WM_LBUTTONDBLCLK, (WPARAM)vKeys, (LPARAM)cursor_s);
}

static void
Widget_OnRightButtonDown(HWND hwnd, unsigned int vKeys, long cursor_s)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);
	
	if (NULL != self->callbacks && NULL != self->callbacks->rightButtonDown)
	{
		POINT cursor;
		POINTSTOPOINT(cursor, cursor_s);
		if (FALSE != self->callbacks->rightButtonDown(self->object, hwnd, vKeys, &cursor))
			return;
	}

	DefWindowProc(hwnd, WM_RBUTTONDOWN, (WPARAM)vKeys, (LPARAM)cursor_s);

}

static void
Widget_OnRightButtonUp(HWND hwnd, unsigned int vKeys, long cursor_s)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	if (NULL != self->callbacks && NULL != self->callbacks->rightButtonUp)
	{
		POINT cursor;
		POINTSTOPOINT(cursor, cursor_s);
		if (FALSE != self->callbacks->rightButtonUp(self->object, hwnd, vKeys, &cursor))
			return;
	}

	DefWindowProc(hwnd, WM_RBUTTONUP, (WPARAM)vKeys, (LPARAM)cursor_s);
}

static void
Widget_OnKeyDown(HWND hwnd, unsigned int vKey, unsigned int flags)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	if (NULL != self->callbacks && 
		NULL != self->callbacks->keyDown &&
		FALSE != self->callbacks->keyDown(self->object, hwnd, vKey, flags))
	{
		return;
	}

	DefWindowProc(hwnd, WM_KEYDOWN, (WPARAM)vKey, (LPARAM)flags);

}

static void
Widget_OnKeyUp(HWND hwnd, unsigned int vKey, unsigned int flags)
{
	Widget *self;
	self = WIDGET(hwnd);

	if (NULL !=  self &&
		NULL != self->callbacks && 
		NULL != self->callbacks->keyUp &&
		FALSE != self->callbacks->keyUp(self->object, hwnd, vKey, flags))
	{
		return;
	}

	DefWindowProc(hwnd, WM_KEYUP, (WPARAM)vKey, (LPARAM)flags);
}

static void
Widget_OnChar(HWND hwnd, unsigned int vKey, unsigned int flags)
{
	Widget *self;
	self = WIDGET(hwnd);

	if (NULL !=  self &&
		NULL != self->callbacks && 
		NULL != self->callbacks->character &&
		FALSE != self->callbacks->character(self->object, hwnd, vKey, flags))
	{
		return;
	}

	DefWindowProc(hwnd, WM_CHAR, (WPARAM)vKey, (LPARAM)flags);
}

static unsigned int
Widget_OnGetDlgCode(HWND hwnd, unsigned int vKey, MSG *message)
{
	Widget *self;
	self = WIDGET(hwnd);
	
	if (NULL != self &&
		NULL != self->callbacks && 
		NULL != self->callbacks->inputRequest)
	{
		return self->callbacks->inputRequest(self->object, hwnd, vKey, message);
	}

	return (unsigned int)DefWindowProc(hwnd, WM_GETDLGCODE, (WPARAM)vKey, (LPARAM)message);
}


static void
Widget_OnSetFocus(HWND hwnd, HWND focusWindow)
{
	Widget_FocusChanged(hwnd, focusWindow, TRUE);
}

static void
Widget_OnKillFocus(HWND hwnd, HWND focusWindow)
{
	Widget_FocusChanged(hwnd, focusWindow, FALSE);
}

static void
Widget_OnContextMenu(HWND hwnd, HWND targetWindow, long cursor_s)
{
	BOOL processed;
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);
	
	if (NULL != self->callbacks && NULL != self->callbacks->contextMenu)
	{
		POINT cursor;
		POINTSTOPOINT(cursor, cursor_s);
		processed = self->callbacks->contextMenu(self->object, hwnd, targetWindow, &cursor);
	}
	else
		processed = FALSE;

	if (FALSE == processed)
		Widget_DefWindowProc(hwnd, WM_CONTEXTMENU, (WPARAM)targetWindow, (LPARAM)cursor_s);
}

static unsigned int
Widget_OnGetType(HWND hwnd)
{
	Widget *self;
	WIDGET_RET_VAL(self, hwnd, WIDGET_TYPE_UNKNOWN);

	return self->type;
}

static void*
Widget_OnGetSelf(HWND hwnd)
{
	Widget *self;
	WIDGET_RET_VAL(self, hwnd, NULL);
	
	return self->object;
}

static BOOL
Widget_OnSetStyle(HWND hwnd, WidgetStyle *style)
{
	Widget *self;
	BOOL styleChanged;

	WIDGET_RET_VAL(self, hwnd, FALSE);

	styleChanged = (self->style != style);

	self->style = style;

	if (FALSE != styleChanged)
	{
		if (NULL != WIDGETCALLBACKS(self))
		{			
			if (NULL != WIDGETCALLBACKS(self)->styleColorChanged)
				 WIDGETCALLBACKS(self)->styleColorChanged(WIDGETOBJECT(self), hwnd, WIDGETSTYLE(self));

			if (NULL != WIDGETCALLBACKS(self)->styleFontChanged)
				 WIDGETCALLBACKS(self)->styleFontChanged(WIDGETOBJECT(self), hwnd, WIDGETSTYLE(self));
		}
	}
	
	return TRUE;
}

static WidgetStyle *
Widget_OnGetStyle(HWND hwnd)
{
	Widget *self;
	WIDGET_RET_VAL(self, hwnd, NULL);

	return self->style;
}

static void 
Widget_OnStyleColorChanged(HWND hwnd)
{
	Widget *self;
	
	WIDGET_RET_VOID(self, hwnd);

	if (FALSE != WIDGET_IS_FROZEN(self))
			return;

	if (NULL != WIDGETCALLBACKS(self) &&
		NULL != WIDGETCALLBACKS(self)->styleColorChanged)
	{
		 WIDGETCALLBACKS(self)->styleColorChanged(WIDGETOBJECT(self), hwnd, WIDGETSTYLE(self));
	}
}

static void
Widget_OnStyleFontChanged(HWND hwnd)
{
	Widget *self;
	
	WIDGET_RET_VOID(self, hwnd);

	if (FALSE != WIDGET_IS_FROZEN(self))
			return;

	if (NULL != WIDGETCALLBACKS(self) &&
		NULL != WIDGETCALLBACKS(self)->styleFontChanged)
	{
		 WIDGETCALLBACKS(self)->styleFontChanged(WIDGETOBJECT(self), hwnd, WIDGETSTYLE(self));
	}
}

static void
Widget_OnFreeze(HWND hwnd, BOOL freeze)
{
	Widget *self;
	
	WIDGET_RET_VOID(self, hwnd);

	if (FALSE == freeze)
		Widget_Thaw(self);
	else
		Widget_Freeze(self);
}

static BOOL
Widget_OnScroll(HWND hwnd, int dx, int dy, BOOL redraw)
{
	return Widget_ScrollWindow(hwnd, dx, dy, redraw);
}

static LRESULT
Widget_OnSetScrollPos(HWND hwnd, int dx, int dy, BOOL redraw)
{
	if (0 != dx)
		dx = Widget_ScrollBarOffsetPos(hwnd, SB_HORZ, dx, redraw);
	
	if (0 != dy)
		dy = Widget_ScrollBarOffsetPos(hwnd, SB_VERT, dy, redraw);

	return (LRESULT)MAKELONG(dx, dy);

}

static void
Widget_OnZoomSliderPosChanging(HWND hwnd, NMTRBTHUMBPOSCHANGING *sliderInfo)
{
	Widget *self;
	WIDGET_RET_VOID(self, hwnd);

	if (NULL != WIDGETCALLBACKS(self) &&
		NULL != WIDGETCALLBACKS(self)->zoomChanging)
	{
		WIDGETCALLBACKS(self)->zoomChanging(WIDGETOBJECT(self), hwnd, sliderInfo);
	}
}

static LRESULT
Widget_OnNotify(HWND hwnd, NMHDR *notification)
{
	Widget *self;

	self = WIDGET(hwnd);
	if (NULL != self && 
		NULL != self->callbacks && 
		NULL != self->callbacks->notify)
	{
		LRESULT result;
		if (FALSE != self->callbacks->notify(WIDGETOBJECT(self), hwnd, notification, &result))
			return result;
	}
	
	return Widget_DefWindowProc(hwnd, WM_NOTIFY, 
				(WPARAM)notification->idFrom, (LPARAM)notification);
}

static BOOL
Widget_OnEnableChildrenScroll(HWND hwnd, BOOL enable)
{
	Widget *self;
	BOOL previous;

	WIDGET_RET_VAL(self, hwnd, FALSE);

	previous = (FALSE == WIDGET_IS_CHILDREN_SCROLL_DISABLED(self));
	
	if (FALSE != enable)
		WIDGET_UNSET_DISABLE_CHILDREN_SCROLL(self);
	else
		WIDGET_SET_DISABLE_CHILDREN_SCROLL(self);

	return previous;
}

static BOOL
Widget_OnGetChildrenScrollEnabled(HWND hwnd)
{
	Widget *self;
	WIDGET_RET_VAL(self, hwnd, FALSE);

	return (FALSE == WIDGET_IS_CHILDREN_SCROLL_DISABLED(self));

}
static LRESULT CALLBACK 
Widget_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return Widget_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			Widget_OnDestroy(hwnd); return 0;
		case WM_PAINT:				Widget_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		Widget_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_PRINT:				return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_WINDOWPOSCHANGED:	Widget_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_MOVE:				return 0;
		case WM_SETTEXT:			return Widget_OnSetText(hwnd, (LPCWSTR)lParam);
		case WM_GETTEXT:			return Widget_OnGetText(hwnd, (LPWSTR)lParam, (INT)wParam);
		case WM_GETTEXTLENGTH:		return Widget_OnGetTextLength(hwnd);
		case WM_SETFONT:			Widget_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); return 0;
		case WM_GETFONT:			return (LRESULT)Widget_OnGetFont(hwnd);
		case WM_VSCROLL:			Widget_OnVertScroll(hwnd, LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); return 0;
		case WM_HSCROLL:			Widget_OnHorzScroll(hwnd, LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); return 0;
		case WM_MOUSEWHEEL:			Widget_OnMouseWheel(hwnd, LOWORD(wParam), (short)HIWORD(wParam), (LONG)lParam); return 0;
		case WM_MOUSEMOVE:			Widget_OnMouseMove(hwnd, (unsigned int)wParam, (long)lParam); return 0;
		case WM_MOUSELEAVE:			Widget_OnMouseLeave(hwnd); return 0;
		case WM_LBUTTONDOWN:		Widget_OnLeftButtonDown(hwnd, (unsigned int)wParam, (long)lParam); return 0;
		case WM_LBUTTONUP:			Widget_OnLeftButtonUp(hwnd, (unsigned int)wParam, (long)lParam); return 0;
		case WM_LBUTTONDBLCLK:		Widget_OnLeftButtonDblClk(hwnd, (unsigned int)wParam, (long)lParam); return 0;
		case WM_RBUTTONDOWN:		Widget_OnRightButtonDown(hwnd, (unsigned int)wParam, (long)lParam); return 0;
		case WM_RBUTTONUP:			Widget_OnRightButtonUp(hwnd, (unsigned int)wParam, (long)lParam); return 0;
		case WM_KEYDOWN:			Widget_OnKeyDown(hwnd, (unsigned int)wParam, (unsigned int)lParam); return 0;			
		case WM_KEYUP:				Widget_OnKeyUp(hwnd, (unsigned int)wParam, (unsigned int)lParam); return 0;
		case WM_CHAR:				Widget_OnChar(hwnd, (unsigned int)wParam, (unsigned int)lParam); return 0;
		case WM_GETDLGCODE:			return Widget_OnGetDlgCode(hwnd, (unsigned int)wParam, (MSG*)lParam);
		case WM_SETFOCUS:			Widget_OnSetFocus(hwnd, (HWND)wParam); return 0;
		case WM_KILLFOCUS:			Widget_OnKillFocus(hwnd, (HWND)wParam); return 0;
		case WM_CONTEXTMENU:		Widget_OnContextMenu(hwnd, (HWND)wParam, (long)lParam); return 0;
		case WM_NOTIFY:				return Widget_OnNotify(hwnd, (NMHDR*)lParam);

		case WIDGET_WM_GET_TYPE:			return (LRESULT)Widget_OnGetType(hwnd);
		case WIDGET_WM_GET_SELF:			return (LRESULT)Widget_OnGetSelf(hwnd);
		case WIDGET_WM_SET_STYLE:			return Widget_OnSetStyle(hwnd, (WidgetStyle*)lParam); 
		case WIDGET_WM_GET_STYLE:			return (LRESULT)Widget_OnGetStyle(hwnd);
		case WIDGET_WM_STYLE_COLOR_CHANGED: Widget_OnStyleColorChanged(hwnd); return 0;
		case WIDGET_WM_STYLE_FONT_CHANGED:	Widget_OnStyleFontChanged(hwnd); return 0;
		case WIDGET_WM_FREEZE:				Widget_OnFreeze(hwnd, (BOOL)wParam); return 0;
		case WIDGET_WM_SET_SCROLL_POS:		return Widget_OnSetScrollPos(hwnd, (short)LOWORD(lParam), (short)HIWORD(lParam), (BOOL)wParam);
		case WIDGET_WM_SCROLL:				return Widget_OnScroll(hwnd, (short)LOWORD(lParam), (short)HIWORD(lParam), (BOOL)wParam);
		case WIDGET_WM_ZOOM_SLIDER_POS_CHANGING: Widget_OnZoomSliderPosChanging(hwnd, (NMTRBTHUMBPOSCHANGING*)lParam); return 0;
		case WIDGET_WM_ENABLE_CHILDREN_SCROLL:		return Widget_OnEnableChildrenScroll(hwnd, (BOOL)lParam);
		case WIDGET_WM_GET_CHILDREN_SCROLL_ENABLED:	return Widget_OnGetChildrenScrollEnabled(hwnd);
	}

	if (WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg && 
		WM_NULL != WINAMP_WM_DIRECT_MOUSE_WHEEL)
	{
		Widget_OnMouseWheel(hwnd, LOWORD(wParam), (SHORT)HIWORD(wParam), (LONG)lParam);
		return TRUE;
	}

	return Widget_DefWindowProc(hwnd, uMsg, wParam, lParam);
}

