#include "main.h"
#include "./widgetHost.h"


#define WIDGETHOST_WINDOW_CLASS		L"NullsoftDevicesWidgetHost"
#define WIDGETHOST_WIDGET_ID		1000

typedef 
enum WidgetHostState
{
	WIDGETHOST_STATE_FROZEN_UI = (1 << 0),
} WidgetHostState;
DEFINE_ENUM_FLAG_OPERATORS(WidgetHostState);

typedef struct WidgetHost
{
	WidgetHostState state;
	WidgetStyle widgetStyle;
	HFONT font;
	HRGN updateRegion;
	POINT updateOffset;
} WidgetHost;

typedef struct WidgetHostCreateParam
{
	WidgetCreateProc widgetCreate;
	void *widgetCreateParam;
} WidgetHostCreateParam;

#define WIDGETHOST(_hwnd) ((WidgetHost*)(LONGX86)GetWindowLongPtrW((_hwnd), 0))
#define WIDGETHOST_RET_VOID(_self, _hwnd) {(_self) = WIDGETHOST((_hwnd)); if (NULL == (_self)) return;}
#define WIDGETHOST_RET_VAL(_self, _hwnd, _error) {(_self) = WIDGETHOST((_hwnd)); if (NULL == (_self)) return (_error);}

#define WIDGETHOST_WIDGET(_hostWindow)	(GetDlgItem((_hostWindow), WIDGETHOST_WIDGET_ID))

#define WIDGETHOST_IS_FROZEN(_self) (0 != (WIDGETHOST_STATE_FROZEN_UI & (_self)->state))
#define WIDGETHOST_FREEZE(_self) (((_self)->state) |= WIDGETHOST_STATE_FROZEN_UI)
#define WIDGETHOST_THAW(_self) (((_self)->state) &= ~WIDGETHOST_STATE_FROZEN_UI)


static LRESULT CALLBACK 
WidgetHost_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);

static ATOM 
WidgetHost_GetClassAtom(HINSTANCE instance)
{
	WNDCLASSEXW klass;
	ATOM klassAtom;

	klassAtom = (ATOM)GetClassInfoExW(instance, WIDGETHOST_WINDOW_CLASS, &klass);
	if (0 != klassAtom)
		return klassAtom;

	memset(&klass, 0, sizeof(klass));
	klass.cbSize = sizeof(klass);
	klass.style = CS_DBLCLKS;
	klass.lpfnWndProc = WidgetHost_WindowProc;
	klass.cbClsExtra = 0;
	klass.cbWndExtra = sizeof(WidgetHost*);
	klass.hInstance = instance;
	klass.hIcon = NULL;
	klass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	klass.hbrBackground = NULL;
	klass.lpszMenuName = NULL;
	klass.lpszClassName = WIDGETHOST_WINDOW_CLASS;
	klass.hIconSm = NULL;
	klassAtom = RegisterClassExW(&klass);
	
	return klassAtom;
}


HWND 
WidgetHost_Create(unsigned int windowStyle, int x, int y, int width, int height, 
				  HWND parentWindow, WidgetCreateProc createProc, void *createParam)
{
	HINSTANCE instance;
	ATOM klassAtom;
	HWND hwnd;
	WidgetHostCreateParam hostParam;
		

	if (NULL == createProc)
		return NULL;

	instance = GetModuleHandleW(NULL);
	klassAtom = WidgetHost_GetClassAtom(instance);
	if (0 == klassAtom)
		return NULL;
	
	hostParam.widgetCreate = createProc;
	hostParam.widgetCreateParam = createParam;
	

	hwnd = CreateWindowExW(WS_EX_NOPARENTNOTIFY |WS_EX_CONTROLPARENT, 
							(LPCWSTR)MAKEINTATOM(klassAtom), NULL,
							WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | windowStyle,
							x, y, width, height, 
							parentWindow, NULL, instance, &hostParam);

	return hwnd;
}

static void 
WidgetHost_Layout(HWND hwnd, BOOL redraw)
{
	WidgetHost *self;
	RECT rect;
	WIDGETHOST_RET_VOID(self, hwnd);

	if (FALSE == GetClientRect(hwnd, &rect))
		return;

	HWND widgetWindow = WIDGETHOST_WIDGET(hwnd);
	if (NULL != widgetWindow)
	{
		unsigned int flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER;
		if (FALSE == redraw)
			flags |= SWP_NOREDRAW;

		SetWindowPos(widgetWindow, NULL, 0, 0, RECTWIDTH(rect), RECTHEIGHT(rect), flags);
	}
}

static void
WidgetHost_Paint(HWND hwnd, HDC hdc, const RECT *paintRect, BOOL erase)
{
	if (FALSE != erase)
	{
		COLORREF backColor, prevBackColor;
		backColor = Graphics_GetSkinColor(WADLG_WNDBG);
		prevBackColor = SetBkColor(hdc, backColor);

		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, paintRect, NULL, 0, NULL);
		SetBkColor(hdc, prevBackColor);
	}
}

static void
WidgetHost_UpdateSkin(HWND hwnd)
{
	WidgetHost *self;
	WIDGETHOST_RET_VOID(self, hwnd);
	BOOL styleChanged = FALSE;

	if (FALSE != WidgetStyle_UpdateDefaultColors(&self->widgetStyle))
		 styleChanged = TRUE;

	if (FALSE != styleChanged)
	{
		HWND widgetWindow = WIDGETHOST_WIDGET(hwnd);
		if (NULL != widgetWindow)
		{
			WIDGET_STYLE_COLOR_CHANGED(widgetWindow);
			InvalidateRect(widgetWindow, NULL, TRUE);
		}
	}

}

static void
WidgetHost_UpdateFont(HWND hwnd, BOOL redraw)
{
	WidgetHost *self;
	BOOL styleChanged = FALSE;
	long unitWidth, unitHeight;

	WIDGETHOST_RET_VOID(self, hwnd);

	if (FALSE == Graphics_GetWindowBaseUnits(hwnd, &unitWidth, &unitHeight))
	{
		unitWidth = 6;
		unitHeight = 13;
	}

	if (FALSE != WidgetStyle_UpdateDefaultFonts(&self->widgetStyle, self->font, unitWidth, unitHeight))
		styleChanged = TRUE;

	if (FALSE != styleChanged)
	{
		HWND widgetWindow = WIDGETHOST_WIDGET(hwnd);
		if (NULL != widgetWindow)
		{
			WIDGET_STYLE_COLOR_CHANGED(widgetWindow);
			InvalidateRect(widgetWindow, NULL, TRUE);
		}
	}

}

static LRESULT
WidgetHost_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{	
	WidgetHost *self;
	HWND widgetWindow;
	WidgetHostCreateParam *createParam;
	
	if (NULL == createStruct)
		return -1;

	createParam = (WidgetHostCreateParam*)createStruct->lpCreateParams;
	if (NULL == createParam)
		return -1;

	self = (WidgetHost*)malloc(sizeof(WidgetHost));
	if (NULL == self)
		return -1;

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)self) && ERROR_SUCCESS != GetLastError())
		return -1;

	memset(self, 0, sizeof(WidgetHost));

	WIDGETHOST_FREEZE(self);

	MLSkinWindow2(Plugin_GetLibraryWindow(), hwnd, SKINNEDWND_TYPE_WINDOW, 
					SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	WidgetHost_UpdateFont(hwnd, FALSE);
	WidgetHost_UpdateSkin(hwnd);

	widgetWindow = NULL;
	if (NULL != createParam->widgetCreate)
		widgetWindow = createParam->widgetCreate(hwnd, createParam->widgetCreateParam);

	if (NULL == widgetWindow)
		return  -1;

	SetWindowLongPtrW(widgetWindow, GWLP_ID, WIDGETHOST_WIDGET_ID);

	WIDGET_SET_STYLE(widgetWindow, &self->widgetStyle);

	SetWindowPos(widgetWindow, NULL, 0, 0, 0, 0, 
		SWP_NOSIZE  | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_FRAMECHANGED);
	
	ShowWindow(widgetWindow, SW_SHOWNA);

	SetWindowPos(widgetWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	
	WIDGETHOST_THAW(self);

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
		SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);

	return 0;
}

static void
WidgetHost_OnDestroy(HWND hwnd)
{
	WidgetHost *self;

	self = WIDGETHOST(hwnd);
	SetWindowLongPtr(hwnd, 0, 0);
	
	if (NULL == self)
		return;

	WIDGETHOST_FREEZE(self);

	WidgetStyle_Free(&self->widgetStyle);
	free(self);
}

static void
WidgetHost_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	
	if (NULL != BeginPaint(hwnd, &ps))
	{		
		WidgetHost_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void 
WidgetHost_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	{
		WidgetHost_Paint(hwnd, hdc, &clientRect, TRUE);
	}
}

static void
WidgetHost_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED) & windowPos->flags))
	{
		WidgetHost *self;
		WIDGETHOST_RET_VOID(self, hwnd);

		if (FALSE != WIDGETHOST_IS_FROZEN(self))
			return;

		WidgetHost_Layout(hwnd, 0 == (SWP_NOREDRAW & windowPos->flags));
	}
}

static void
WidgetHost_OnDisplayChanged(HWND hwnd, INT bpp, INT dpi_x, INT dpi_y)
{
	WidgetHost *self;
	WIDGETHOST_RET_VOID(self, hwnd);

	if (FALSE != WIDGETHOST_IS_FROZEN(self))
		return;
	
	WidgetHost_UpdateSkin(hwnd);
	InvalidateRect(hwnd, NULL, TRUE);
}

static void
WidgetHost_OnSetFont(HWND hwnd, HFONT font, BOOL redraw)
{
	WidgetHost *self;
	LOGFONTW prevFont, newFont;
	
	WIDGETHOST_RET_VOID(self, hwnd);
	
	if (NULL == self->font || 
		sizeof(LOGFONTW) != GetObjectW(self->font, sizeof(prevFont), &prevFont))
	{
		ZeroMemory(&prevFont, sizeof(prevFont));
	}

	self->font = font;


	if (NULL == self->font || 
		sizeof(newFont) != GetObjectW(self->font, sizeof(newFont), &newFont))
	{
		ZeroMemory(&newFont, sizeof(newFont));
	}

	if (0 != memcmp(&prevFont, &newFont, sizeof(prevFont)) &&
		FALSE == WIDGETHOST_IS_FROZEN(self))
	{
		WidgetHost_UpdateFont(hwnd, redraw);
	}
}

static HFONT
WidgetHost_OnGetFont(HWND hwnd)
{
	WidgetHost *self;
	WIDGETHOST_RET_VAL(self, hwnd, NULL);
	
	return self->font;
}

static void
WidgetHost_OnSetUpdateRegion(HWND hwnd, HRGN updateRegion, POINTS regionOffset)
{
	WidgetHost *self;
	WIDGETHOST_RET_VOID(self, hwnd);

	self->updateRegion = updateRegion;
	self->updateOffset.x = regionOffset.x;
	self->updateOffset.y = regionOffset.y;
}


static LRESULT CALLBACK 
WidgetHost_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return WidgetHost_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			WidgetHost_OnDestroy(hwnd); return 0;
		case WM_PAINT:				WidgetHost_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		WidgetHost_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_PRINT:				return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_WINDOWPOSCHANGED:	WidgetHost_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_MOVE:				return 0;
		case WM_DISPLAYCHANGE:		WidgetHost_OnDisplayChanged(hwnd, (INT)wParam, LOWORD(lParam), HIWORD(lParam)); return 0;
		case WM_SETFONT:			WidgetHost_OnSetFont(hwnd, (HFONT)wParam, LOWORD(lParam)); return 0;
		case WM_GETFONT:			return (LRESULT)WidgetHost_OnGetFont(hwnd);

		// gen_ml flickerless drawing
		case WM_USER + 0x200:		return 1;
		case WM_USER + 0x201:		WidgetHost_OnSetUpdateRegion(hwnd, (HRGN)lParam, MAKEPOINTS(wParam)); return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}