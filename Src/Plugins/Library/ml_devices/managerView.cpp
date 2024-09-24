#include "main.h"
#include "./managerView.h"

#define MANAGERVIEW_PROP		L"NullsoftDevicesManagerViewProp"

#define VIEW_OFFSET_LEFT_PX					0
#define VIEW_OFFSET_TOP_PX					0
#define VIEW_OFFSET_RIGHT_PX				2
#define VIEW_OFFSET_BOTTOM_PX				-1

#define DISCOVER_BUTTON_MIN_HEIGHT_PX		18	
#define DISCOVER_BUTTON_MIN_WIDTH_PX		72
#define DISCOVER_BUTTON_EXTRA_SPACE_DLU		8
#define DISCOVER_BUTTON_SPACING_RIGHT_DLU	6

#define ZOOM_SLIDER_SPACING_LEFT_DLU		6

#define BOTTOM_BAR_OFFSET_TOP_DLU			2


#define MANAGERVIEW_WIDGET_ID				10000
#define MANAGERVIEW_STATUSBAR_ID			10001

static ATOM MANAGERVIEW_ATOM = 0;


typedef 
enum ManagerViewState
{
	MANAGERVIEW_STATE_FROZEN_UI = (1 << 0),
} WelcomeViewState;
DEFINE_ENUM_FLAG_OPERATORS(ManagerViewState);

#define MANAGERVIEW_IS_FROZEN(_view) (0 != (MANAGERVIEW_STATE_FROZEN_UI & (_view)->state))
#define MANAGERVIEW_FREEZE(_view) (((_view)->state) |= MANAGERVIEW_STATE_FROZEN_UI)
#define MANAGERVIEW_THAW(_view) (((_view)->state) &= ~MANAGERVIEW_STATE_FROZEN_UI)

#define MANAGERVIEW_DLU_TO_HORZ_PX(_view, _dlu) MulDiv((_dlu), (_view)->unitSize.cx, 4)
#define MANAGERVIEW_DLU_TO_VERT_PX(_view, _dlu) MulDiv((_dlu), (_view)->unitSize.cy, 8)

#define MANAGERVIEW_REGISTER_WIDGET(_widgetWindow) (SetWindowLongPtrW((_widgetWindow), GWLP_ID, MANAGERVIEW_WIDGET_ID))
#define MANAGERVIEW_WIDGET(_viewWindow)	(GetDlgItem((_viewWindow), MANAGERVIEW_WIDGET_ID))
#define MANAGERVIEW_STATUS_BAR(_viewWindow)	(GetDlgItem((_viewWindow), MANAGERVIEW_STATUSBAR_ID))
#define MANAGERVIEW_DISCOVER_BUTTON(_viewWindow)	(GetDlgItem((_viewWindow), IDC_BUTTON_DISCOVER))
#define MANAGERVIEW_ZOOM_SLIDER(_viewWindow)	(GetDlgItem((_viewWindow), IDC_SLIDER_ZOOM))

typedef struct ManagerView
{
	ManagerViewState state;
	WidgetStyle widgetStyle;
	HFONT font;
	HFONT systemFont;
	COLORREF backColor;
	COLORREF textColor;
	COLORREF borderColor;
	HBRUSH backBrush;
	SIZE unitSize;
	BOOL devicesPresent;
	size_t deviceHandler;
	HRGN updateRegion;
	POINT updateOffset;
	unsigned int discoveryStatus;
} ManagerView;

#define MANAGERVIEW(_hwnd) ((ManagerView*)GetPropW((_hwnd), MAKEINTATOM(MANAGERVIEW_ATOM)))
#define MANAGERVIEW_RET_VOID(_view, _hwnd) { (_view) = MANAGERVIEW((_hwnd)); if (NULL == (_view)) return; }
#define MANAGERVIEW_RET_VAL(_view, _hwnd, _error) { (_view) = MANAGERVIEW((_hwnd)); if (NULL == (_view)) return (_error); }

static INT_PTR CALLBACK 
ManagerView_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void CALLBACK
ManagerView_PluginUnloadCb()
{
	if (0 != MANAGERVIEW_ATOM)
	{
		GlobalDeleteAtom(MANAGERVIEW_ATOM);
		MANAGERVIEW_ATOM = 0;
	}
}

HWND ManagerView_CreateWindow(HWND parentWindow)
{
	if (0 == MANAGERVIEW_ATOM)
	{
		MANAGERVIEW_ATOM = GlobalAddAtom(MANAGERVIEW_PROP);
		if (0 == MANAGERVIEW_ATOM)
			return NULL;

		Plugin_RegisterUnloadCallback(ManagerView_PluginUnloadCb);
	}

	HWND hwnd = WASABI_API_CREATEDIALOGPARAMW((INT_PTR)IDD_MANAGER_VIEW, parentWindow,
											  ManagerView_DialogProc, (LPARAM)0L);

	return hwnd;
}

static void 
ManagerView_Layout(HWND hwnd, BOOL redraw)
{
	ManagerView *self = NULL;
	RECT clientRect, elementRect;
	LONG bottomBarHeight = 0, buttonWidth = 0, sliderWidth = 0;

	MANAGERVIEW_RET_VOID(self, hwnd);

	GetClientRect(hwnd, &clientRect);
	clientRect.left += VIEW_OFFSET_LEFT_PX;
	clientRect.top += VIEW_OFFSET_TOP_PX;
	clientRect.right -= VIEW_OFFSET_RIGHT_PX;
	clientRect.bottom -= VIEW_OFFSET_BOTTOM_PX;
	
	HDWP hdwp = BeginDeferWindowPos(4);
	if (NULL == hdwp)
		return;

	UINT swpFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE == redraw)
		swpFlags |= SWP_NOREDRAW;

	HWND buttonWindow = MANAGERVIEW_DISCOVER_BUTTON(hwnd);
	if (NULL != buttonWindow)
	{
		if (FALSE != GetWindowRect(buttonWindow, &elementRect))
		{
			wchar_t buffer[128] = {0};
			GetWindowText(buttonWindow, buffer, ARRAYSIZE(buffer));
			LRESULT idealSize = MLSkinnedButton_GetIdealSize(buttonWindow, buffer);

			hdwp = DeferWindowPos(hdwp, buttonWindow, NULL, clientRect.left + 1,
								  clientRect.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)) - WASABI_API_APP->getScaleY(1),
								  RECTWIDTH(elementRect), WASABI_API_APP->getScaleY(HIWORD(idealSize)), swpFlags);

			bottomBarHeight = WASABI_API_APP->getScaleY(HIWORD(idealSize));
			buttonWidth = RECTWIDTH(elementRect);
		}
	}

	HWND sliderWindow = MANAGERVIEW_ZOOM_SLIDER(hwnd);
	if (NULL != sliderWindow)
	{
		if (0 != (WS_VISIBLE & GetWindowStyle(sliderWindow)) && 
			FALSE != GetWindowRect(sliderWindow, &elementRect))
		{
			hdwp = DeferWindowPos(hdwp, sliderWindow, NULL, clientRect.right - RECTWIDTH(elementRect),
						clientRect.bottom - RECTHEIGHT(elementRect),
						RECTWIDTH(elementRect), RECTHEIGHT(elementRect), swpFlags);
			sliderWidth = RECTWIDTH(elementRect);
		}
	}

	HWND statusBar = MANAGERVIEW_STATUS_BAR(hwnd);
	if (NULL != statusBar)
	{
		long statusBarHeight = 0;
		CopyRect(&elementRect, &clientRect);

		if (0 != buttonWidth)
		{
			elementRect.left += buttonWidth;
			elementRect.left += MANAGERVIEW_DLU_TO_HORZ_PX(self, DISCOVER_BUTTON_SPACING_RIGHT_DLU);
			if (elementRect.left > clientRect.right)
				elementRect.left = clientRect.right;
		}

		if (0 != sliderWidth)
		{
			elementRect.right -= sliderWidth;
			elementRect.right -= MANAGERVIEW_DLU_TO_HORZ_PX(self, ZOOM_SLIDER_SPACING_LEFT_DLU);
			if (elementRect.left > elementRect.right)
				elementRect.right = elementRect.left;
		}

		statusBarHeight = STATUSBAR_GET_IDEAL_HEIGHT(statusBar);
		if (statusBarHeight > bottomBarHeight)
			statusBarHeight = 0;

		elementRect.top = elementRect.bottom - statusBarHeight - WASABI_API_APP->getScaleY(3);
		elementRect.bottom = elementRect.top + statusBarHeight;

		hdwp = DeferWindowPos(hdwp, statusBar, NULL, elementRect.left, elementRect.top,
							  RECTWIDTH(elementRect), RECTHEIGHT(elementRect), swpFlags);
	}

	HWND widgetWindow = MANAGERVIEW_WIDGET(hwnd);
	if (NULL != widgetWindow)
	{
		CopyRect(&elementRect, &clientRect);
		if (0 != bottomBarHeight)
		{
			elementRect.bottom -= (bottomBarHeight + WASABI_API_APP->getScaleY(4));
		}

		Graphics_ClampRect(&elementRect, &clientRect);

		if (NULL != hdwp)
		{
			hdwp = DeferWindowPos(hdwp, widgetWindow, NULL, elementRect.left, elementRect.top, 
						RECTWIDTH(elementRect), RECTHEIGHT(elementRect), swpFlags);
		}
	}

	if (NULL != hdwp)
		EndDeferWindowPos(hdwp);
}

static BOOL
ManagerView_Paint(HWND hwnd, HDC hdc, const RECT *paintRect, BOOL erase)
{
	ManagerView *self;
	MANAGERVIEW_RET_VAL(self, hwnd, FALSE);

	if (FALSE != erase)
	{	
		HRGN fillRegion;
		fillRegion = CreateRectRgnIndirect(paintRect);
		FillRgn(hdc, fillRegion, self->backBrush);			
		DeleteObject(fillRegion);
	}

	return TRUE;
}

static void
ManagerView_UpdateSkin(HWND hwnd)
{
	ManagerView *self;
	MANAGERVIEW_RET_VOID(self, hwnd);

	BOOL styleChanged = FALSE;
	COLORREF color = Graphics_GetSkinColor(WADLG_WNDBG);
	if (color != self->backColor || NULL == self->backBrush)
	{
		self->backColor = color;
		self->backBrush = CreateSolidBrush(self->backColor);
		styleChanged = TRUE;
	}

	color = Graphics_GetSkinColor(WADLG_WNDFG);
	if (self->textColor != color)
	{
		self->textColor = color;
		styleChanged = TRUE;
	}

	color = Graphics_GetSkinColor(WADLG_HILITE);
	if (self->borderColor != color)
	{
		self->borderColor = color;
		styleChanged = TRUE;
	}
	
	if (FALSE != WidgetStyle_UpdateDefaultColors(&self->widgetStyle))
		 styleChanged = TRUE;

	if (FALSE != styleChanged)
	{
		HWND controlWindow = MANAGERVIEW_WIDGET(hwnd);
		if (NULL != controlWindow)
		{
			WIDGET_STYLE_COLOR_CHANGED(controlWindow);
			InvalidateRect(controlWindow, NULL, TRUE);
		}

		controlWindow = MANAGERVIEW_STATUS_BAR(hwnd);
		if (NULL != controlWindow)
		{
			STATUSBAR_SET_TEXT_COLOR(controlWindow, self->textColor, FALSE);
			STATUSBAR_SET_BACK_COLOR(controlWindow, self->backColor, FALSE);
			STATUSBAR_SET_BACK_BRUSH(controlWindow, self->backBrush, FALSE);
		}
	}
}

static BOOL
ManagerView_GetIdealButtonSize(HWND buttonWindow, SIZE *buttonSize)
{
	if (NULL == buttonWindow || NULL == buttonSize)
		return FALSE;

	LRESULT skinSize = MLSkinnedButton_GetIdealSize(buttonWindow, NULL);
	if (0 != skinSize)
	{
		buttonSize->cx = LOWORD(skinSize);
		buttonSize->cy = HIWORD(skinSize);	
		return TRUE;
	}

	buttonSize->cx = 0;
	buttonSize->cy = 0;
	if (FALSE != SendMessageW(buttonWindow, BCM_GETIDEALSIZE, 0, (LPARAM)buttonSize))
		return TRUE;

	return FALSE;
}

static BOOL
ManagerView_UpdateDiscoverButtonFont(HWND hwnd, HFONT font, SIZE *size)
{
	ManagerView *self;
	SIZE buttonSize;

	MANAGERVIEW_RET_VAL(self, hwnd, FALSE);

	HWND buttonWindow = MANAGERVIEW_DISCOVER_BUTTON(hwnd);
	if (NULL == buttonWindow)
		return FALSE;

	SendMessage(buttonWindow, WM_SETFONT, (WPARAM)font, MAKELPARAM(0,0));

	if (FALSE == ManagerView_GetIdealButtonSize(buttonWindow, &buttonSize))
	{
		RECT buttonRect;
		if (FALSE == GetWindowRect(buttonWindow, &buttonRect))
			return FALSE;

		buttonSize.cx = RECTWIDTH(buttonRect);
		buttonSize.cy = RECTHEIGHT(buttonRect);
	}

	buttonSize.cx += MANAGERVIEW_DLU_TO_HORZ_PX(self, DISCOVER_BUTTON_EXTRA_SPACE_DLU);

	if (buttonSize.cx < DISCOVER_BUTTON_MIN_WIDTH_PX)
		buttonSize.cx = DISCOVER_BUTTON_MIN_WIDTH_PX;

	if (buttonSize.cy < DISCOVER_BUTTON_MIN_HEIGHT_PX)
		buttonSize.cy = DISCOVER_BUTTON_MIN_HEIGHT_PX;

	BOOL result = SetWindowPos(buttonWindow, NULL, 0, 0, buttonSize.cx, buttonSize.cy, 
							   SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
	if (FALSE == result)
		return FALSE;

	if (NULL != size)
		*size = buttonSize;

	return TRUE;
}

static void
ManagerView_UpdateFont(HWND hwnd, BOOL redraw)
{
	HWND controlWindow = NULL;
	ManagerView *self = NULL;
	MANAGERVIEW_RET_VOID(self, hwnd);

	if (FALSE == Graphics_GetWindowBaseUnits(hwnd, &self->unitSize.cx, &self->unitSize.cy))
	{
		self->unitSize.cx = 6;
		self->unitSize.cy = 13;
	}

	if (FALSE != WidgetStyle_UpdateDefaultFonts(&self->widgetStyle, self->font, self->unitSize.cx, self->unitSize.cy))
	{
		controlWindow = MANAGERVIEW_WIDGET(hwnd);
		if (NULL != controlWindow)
			WIDGET_STYLE_FONT_CHANGED(controlWindow);
	}

	ManagerView_UpdateDiscoverButtonFont(hwnd, self->font, NULL);

	controlWindow = MANAGERVIEW_STATUS_BAR(hwnd);
	if (NULL != controlWindow)
		SendMessage(controlWindow, WM_SETFONT, (WPARAM)self->font, 0L);

	if (NULL != redraw)
	{
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE |SWP_NOMOVE |SWP_FRAMECHANGED | SWP_NOREDRAW);

		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
	}
}

static void
ManagerView_UpdateDiscoveryStatus(HWND hwnd, BOOL discoveryActive)
{
	ManagerView *self;
	MANAGERVIEW_RET_VOID(self, hwnd);

	HWND statusBar = MANAGERVIEW_STATUS_BAR(hwnd);
	if (NULL == statusBar)
		return;

	if (FALSE != discoveryActive)
	{
		wchar_t buffer[512] = {0};

		WASABI_API_LNGSTRINGW_BUF(IDS_STATUS_DISCOVERY_ACTIVE, buffer, ARRAYSIZE(buffer));

		if (STATUS_ERROR == self->discoveryStatus)
			self->discoveryStatus = STATUSBAR_ADD_STATUS(statusBar, buffer);
		else
		{
			STATUSBAR_SET_STATUS_TEXT(statusBar, self->discoveryStatus, buffer);
			STATUSBAR_MOVE_STATUS(statusBar, self->discoveryStatus, STATUS_MOVE_TOP);
		}
	}
	else
	{
		if (STATUS_ERROR != self->discoveryStatus)
		{
			STATUSBAR_REMOVE_STATUS(statusBar, self->discoveryStatus);
			self->discoveryStatus = STATUS_ERROR;
		}
	}
}

static HWND 
ManagerView_CreateServiceErrorWidget(HWND hwnd)
{
	return InfoWidget_CreateWindow(WIDGET_TYPE_SERVICE_ERROR, 
								   MAKEINTRESOURCE(IDS_INFOWIDGET_TITLE),
								   MAKEINTRESOURCE(IDS_DEVICE_SERVICE_NOT_FOUND),
								   NULL,
								   hwnd, 0, 0, 0, 0, TRUE, 0);
}

static HWND
ManagerView_CreateViewErrorWidget(HWND hwnd)
{
	return InfoWidget_CreateWindow(WIDGET_TYPE_VIEW_ERROR, 
								   MAKEINTRESOURCE(IDS_INFOWIDGET_TITLE),
								   MAKEINTRESOURCE(IDS_CREATE_MANAGER_VIEW_FAILED),
								   NULL,
								   hwnd, 0, 0, 0, 0, TRUE, 0);
}

static HWND
ManagerView_UpdateWidget(HWND hwnd)
{
	ManagerView *self;
	unsigned int widgetType, requiredType;
	unsigned int windowStyle;

	MANAGERVIEW_RET_VAL(self, hwnd, NULL);

	HWND widgetWindow = MANAGERVIEW_WIDGET(hwnd);
	if (NULL == widgetWindow)
		widgetType = WIDGET_TYPE_UNKNOWN;
	else
		widgetType = WIDGET_GET_TYPE(widgetWindow);

	if (FALSE != self->devicesPresent)
		requiredType = WIDGET_TYPE_LIST;
	else
	{
		requiredType = (NULL != WASABI_API_DEVICES) ?
						WIDGET_TYPE_WELCOME :
						WIDGET_TYPE_SERVICE_ERROR;
	}

	if (widgetType == requiredType)
		return widgetWindow;

	windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowStyle(hwnd, windowStyle & ~WS_VISIBLE);

	if (NULL != widgetWindow)
	{
		SetWindowLongPtr(widgetWindow, GWLP_ID, 0);
		WIDGET_FREEZE(widgetWindow);
		DestroyWindow(widgetWindow);
	}

	switch(requiredType)
	{
		case WIDGET_TYPE_SERVICE_ERROR:
			widgetWindow = ManagerView_CreateServiceErrorWidget(hwnd);
			break;
		case WIDGET_TYPE_WELCOME:
			widgetWindow = WelcomeWidget_CreateWindow(hwnd, 0, 0, 0, 0, TRUE, 0);
			break;
		case WIDGET_TYPE_LIST:
			widgetWindow = ListWidget_CreateWindow(hwnd, 0, 0, 0, 0, TRUE, 0);
			break;
		default:
			widgetWindow = NULL;
			break;
	}

	if (NULL == widgetWindow)
		widgetWindow = ManagerView_CreateViewErrorWidget(hwnd);

	if (NULL != widgetWindow)
	{
		if (FALSE != MANAGERVIEW_IS_FROZEN(self))
			WIDGET_FREEZE(widgetWindow);

		MANAGERVIEW_REGISTER_WIDGET(widgetWindow);

		WIDGET_SET_STYLE(widgetWindow, &self->widgetStyle);
	}

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
				 SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_FRAMECHANGED);

	if (NULL != widgetWindow)
	{
		ShowWindow(widgetWindow, SW_SHOWNA);

		SetWindowPos(widgetWindow, HWND_TOP, 0, 0, 0, 0, 
						SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	if (0 != (WS_VISIBLE & windowStyle))
	{
		windowStyle = GetWindowStyle(hwnd);
		if (0 == (WS_VISIBLE & windowStyle))
		{
			windowStyle |= WS_VISIBLE;
			SetWindowStyle(hwnd, windowStyle);
		}
	}

	if (0 != (WS_VISIBLE & windowStyle))
	{
		RECT rect;
		GetClientRect(hwnd, &rect);

		RedrawWindow(hwnd, &rect, NULL, 
			RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
	}

	return widgetWindow;
}

static void
ManagerView_StartDiscovery(HWND hwnd, BOOL silent)
{
	Plugin_BeginDiscovery();
}

static BOOL
ManagerView_CheckDevicesPresent()
{
	ifc_deviceobjectenum *enumerator = 0;
	ifc_deviceobject *object = 0;
	ifc_device *device = 0;

	BOOL devicesPresent = FALSE;

	if (NULL == WASABI_API_DEVICES || 
		FAILED(WASABI_API_DEVICES->DeviceEnumerate(&enumerator)))
	{
		return FALSE;
	}

	while(S_OK == enumerator->Next(&object, 1, NULL))
	{
		if (SUCCEEDED(object->QueryInterface(IFC_Device, (void**)&device)))
		{
			if(FALSE == device->GetHidden() &&
			   // excludes 'cloud' devices from appearing
			   lstrcmpiA(device->GetConnection(), "cloud"))
				devicesPresent = TRUE;
		
			device->Release();
		}
		object->Release();

		if (FALSE != devicesPresent)
			break;
	}
	enumerator->Release();
	
	return devicesPresent;
}

static void
ManagerView_AddDevice(HWND hwnd, ifc_device *device)
{
	ManagerView *self;

	MANAGERVIEW_RET_VOID(self, hwnd);

	if (FALSE != self->devicesPresent)
		return;

	self->devicesPresent = ManagerView_CheckDevicesPresent();
	if (FALSE != self->devicesPresent)
		ManagerView_UpdateWidget(hwnd);
}

static void
ManagerView_RemoveDevice(HWND hwnd, ifc_device *device)
{
	ManagerView *self;
	MANAGERVIEW_RET_VOID(self, hwnd);

	if (FALSE == self->devicesPresent)
		return;

	self->devicesPresent = ManagerView_CheckDevicesPresent();
	if (FALSE == self->devicesPresent)
		ManagerView_UpdateWidget(hwnd);
}

static void
ManagerView_DeviceCb(ifc_device *device, DeviceEvent eventId, void *user)
{
	HWND hwnd = (HWND)user;

	switch(eventId)
	{
		case Event_DeviceAdded:
			ManagerView_AddDevice(hwnd, device);
			break;

		case Event_DeviceRemoved:
			ManagerView_RemoveDevice(hwnd, device);
			break;
	}
}

static void
ManagerView_DiscoveryCb(api_devicemanager *manager, DeviceDiscoveryEvent eventId, void *user)
{
	HWND hwnd = (HWND)user;

	switch(eventId)
	{
		case Event_DiscoveryStarted:
			ManagerView_UpdateDiscoveryStatus(hwnd, TRUE);
			break;

		case Event_DiscoveryFinished:
			ManagerView_UpdateDiscoveryStatus(hwnd, FALSE);
			break;
	}
}

static BOOL
ManagerView_RegisterDeviceHandler(HWND hwnd)
{
	ManagerView *self;
	DeviceEventCallbacks callbacks;

	MANAGERVIEW_RET_VAL(self, hwnd, FALSE);

	if (0 != self->deviceHandler)
		return FALSE;

	HWND eventRelay = Plugin_GetEventRelayWindow();
	if (NULL == eventRelay)
		return FALSE;

	ZeroMemory(&callbacks, sizeof(callbacks));
	callbacks.deviceCb = ManagerView_DeviceCb;
	callbacks.discoveryCb = ManagerView_DiscoveryCb;

	self->deviceHandler = EVENTRELAY_REGISTER_HANDLER(eventRelay, &callbacks, hwnd); 
	return (0 != self->deviceHandler);
}

static void
ManagerView_OnDisplayChanged(HWND hwnd, INT bpp, INT dpi_x, INT dpi_y)
{
	ManagerView *self = NULL;
	MANAGERVIEW_RET_VOID(self, hwnd);

	if (FALSE != MANAGERVIEW_IS_FROZEN(self))
		return;

	ManagerView_UpdateSkin(hwnd);

	RECT rc;
	GetClientRect(hwnd, &rc);
	RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_ERASENOW | RDW_UPDATENOW);

	ManagerView_Layout(hwnd, TRUE);
}

static INT_PTR 
ManagerView_OnInitDialog(HWND hwnd, HWND focusWindow, LPARAM param)
{
	ManagerView *self = (ManagerView*)malloc(sizeof(ManagerView));
	if (NULL != self) 
	{
		ZeroMemory(self, sizeof(ManagerView));
		if (FALSE == SetProp(hwnd, MAKEINTATOM(MANAGERVIEW_ATOM), self))
		{
			free(self);
			self = NULL;
		}
	}

	if (NULL == self)
	{
		DestroyWindow(hwnd);
		return 0;
	}

	MANAGERVIEW_FREEZE(self);

	ManagerView_RegisterDeviceHandler(hwnd);

	self->devicesPresent = ManagerView_CheckDevicesPresent();

	MLSkinWindow2(Plugin_GetLibraryWindow(), hwnd, SKINNEDWND_TYPE_DIALOG, 
				  SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	HWND discoverButton = MANAGERVIEW_DISCOVER_BUTTON(hwnd);
	if (NULL != discoverButton)
	{		
		MLSkinWindow2(Plugin_GetLibraryWindow(), discoverButton, SKINNEDWND_TYPE_BUTTON, 
					  SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

		ShowWindow(discoverButton, SW_SHOW);
		EnableWindow(discoverButton, TRUE);
	}

	HWND statusBar = StatusBar_CreateWindow(0, NULL, WS_VISIBLE, 0, 0, 0, 0, hwnd, MANAGERVIEW_STATUSBAR_ID);
	if (NULL != statusBar)
	{		
		MLSkinWindow2(Plugin_GetLibraryWindow(), statusBar, SKINNEDWND_TYPE_AUTO, 
					  SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);
	}

	self->discoveryStatus = STATUS_ERROR;

	if (S_OK == WASABI_API_DEVICES->IsDiscoveryActive())
		ManagerView_UpdateDiscoveryStatus(hwnd, TRUE);

	ManagerView_UpdateFont(hwnd, FALSE);
	ManagerView_UpdateSkin(hwnd);

	HWND widgetWindow = ManagerView_UpdateWidget(hwnd);
	MANAGERVIEW_THAW(self);

	if (NULL != widgetWindow)
		WIDGET_THAW(widgetWindow);

	PostMessage(hwnd, WM_DISPLAYCHANGE, 0, 0);

	return 0;
}

static void
ManagerView_OnDestroy(HWND hwnd)
{
	ManagerView *self = MANAGERVIEW(hwnd);
	RemoveProp(hwnd, MAKEINTATOM(MANAGERVIEW_ATOM));

	if (NULL == self) 
		return;

	MANAGERVIEW_FREEZE(self);

	if (0 != self->deviceHandler)
	{
		HWND eventRelay = Plugin_GetEventRelayWindow();
		if (NULL != eventRelay)
		{
			EVENTRELAY_UNREGISTER_HANDLER(eventRelay, self->deviceHandler); 
		}
	}

	if (NULL != self->systemFont)
		DeleteObject(self->systemFont);

	if (NULL != self->backBrush)
		DeleteObject(self->backBrush);

	WidgetStyle_Free(&self->widgetStyle);
	free(self);
}

static LRESULT
ManagerView_OnColorDialog(HWND hwnd, HDC hdc)
{
	ManagerView *self;
	self = MANAGERVIEW(hwnd);

	if (NULL == self)
		return DefWindowProcW(hwnd, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hwnd);

	if (NULL != hdc)
	{
		SetTextColor(hdc, self->textColor);
		SetBkColor(hdc, self->backColor);
	}

	return (LRESULT)self->backBrush;
}

static void
ManagerView_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & windowPos->flags) || 
		(SWP_FRAMECHANGED & windowPos->flags))
	{
		ManagerView *self;
		MANAGERVIEW_RET_VOID(self, hwnd);

		if (FALSE != MANAGERVIEW_IS_FROZEN(self))
			return;

		ManagerView_Layout(hwnd, !(SWP_NOREDRAW & windowPos->flags));
	}
}

static void
ManagerView_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;

	if (NULL != BeginPaint(hwnd, &ps))
	{		
		ManagerView_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void 
ManagerView_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	{
		ManagerView_Paint(hwnd, hdc, &clientRect, TRUE);
	}
}

static void
ManagerView_OnSetFont(HWND hwnd, HFONT font, BOOL redraw)
{
	ManagerView *self = NULL;
	LOGFONTW prevFont = {0}, newFont = {0};

	MANAGERVIEW_RET_VOID(self, hwnd);

	if (NULL == self->font || 
		sizeof(LOGFONTW) != GetObjectW(self->font, sizeof(prevFont), &prevFont))
	{
		ZeroMemory(&prevFont, sizeof(prevFont));
	}

	self->font = font;
	if (NULL == self->font)
	{
		if (NULL == self->systemFont)
			self->systemFont = Graphics_CreateSysFont();
	}

	if (NULL == self->font || 
		sizeof(newFont) != GetObjectW(self->font, sizeof(newFont), &newFont))
	{
		ZeroMemory(&newFont, sizeof(newFont));
	}

	if (0 == memcmp(&prevFont, &newFont, sizeof(prevFont)) ||
		FALSE != MANAGERVIEW_IS_FROZEN(self))
	{
		redraw = FALSE;
	}

	ManagerView_UpdateFont(hwnd, redraw);
}

static HFONT
ManagerView_OnGetFont(HWND hwnd)
{
	ManagerView *self;
	MANAGERVIEW_RET_VAL(self, hwnd, NULL);

	return self->font;
}

static void
ManagerView_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND controlWindow)
{
	switch(commandId)
	{
		case IDC_BUTTON_DISCOVER:
			switch(eventId)
			{
				case BN_CLICKED:
					ManagerView_StartDiscovery(hwnd, FALSE);
					break;
			}
			break;
	}
}

static void
ManagerView_OnZoomSliderPosChanging(HWND hwnd, NMTRBTHUMBPOSCHANGING *sliderInfo)
{
	HWND widgetWindow = MANAGERVIEW_WIDGET(hwnd);
	if (NULL != widgetWindow)
		WIDGET_ZOOM_SLIDER_POS_CHANGING(widgetWindow, sliderInfo);
}

static LRESULT
ManagerView_OnNotify(HWND hwnd, NMHDR *pnmh)
{
	return 0;
}

static void
ManagerView_OnHorzScroll(HWND hwnd, INT action, INT trackPosition, HWND senderWindow)
{
	HWND sliderWindow = MANAGERVIEW_ZOOM_SLIDER(hwnd);
	if (NULL != sliderWindow && senderWindow == sliderWindow)
	{
		NMTRBTHUMBPOSCHANGING zoomInfo;
		zoomInfo.hdr.code = TRBN_THUMBPOSCHANGING;
		zoomInfo.hdr.hwndFrom = senderWindow;
		zoomInfo.hdr.idFrom = IDC_SLIDER_ZOOM;
		zoomInfo.nReason = action;
		if (TB_THUMBPOSITION == action ||
			TB_THUMBTRACK == action)
		{
			zoomInfo.dwPos = trackPosition;
		}
		else
			zoomInfo.dwPos = (DWORD)SendMessage(sliderWindow, TBM_GETPOS, 0, 0L);
		
		ManagerView_OnZoomSliderPosChanging(hwnd, &zoomInfo);
	}
}

static BOOL
ManagerView_OnHelp(HWND hwnd, HELPINFO *helpInfo)
{
	HWND widgetWindow = MANAGERVIEW_WIDGET(hwnd);
	if (NULL != widgetWindow)
	{
		wchar_t buffer[4096] = {0};

		if (FALSE != WIDGET_GET_HELP_URL(widgetWindow, buffer, ARRAYSIZE(buffer)) && 
			MediaLibrary_ShowHelp(Plugin_GetLibraryWindow(), buffer))
		{
			return TRUE;
		}
	}

	return Plugin_ShowHelp();
}

static void 
ManagerView_OnSetUpdateRegion(HWND  hwnd, HRGN updateRegion, POINTS regionOffset)
{
	ManagerView *self;
	MANAGERVIEW_RET_VOID(self, hwnd);

	self->updateRegion = updateRegion;
	self->updateOffset.x = regionOffset.x;
	self->updateOffset.y = regionOffset.y;
}

static HWND
ManagerView_OnGetZoomSlider(HWND hwnd)
{
	return MANAGERVIEW_ZOOM_SLIDER(hwnd);
}

static HWND
ManagerView_OnGetStatusBar(HWND hwnd)
{
	return MANAGERVIEW_STATUS_BAR(hwnd);
}

static INT_PTR CALLBACK 
ManagerView_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:		return ManagerView_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		ManagerView_OnDestroy(hwnd); return TRUE;
		case WM_CTLCOLORDLG:	return ManagerView_OnColorDialog(hwnd, (HDC)wParam);
		case WM_PAINT:			ManagerView_OnPaint(hwnd); return TRUE;
		case WM_PRINTCLIENT:	ManagerView_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return TRUE;
		case WM_ERASEBKGND:		DIALOG_RESULT(hwnd, 0);
		case WM_WINDOWPOSCHANGED:	ManagerView_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_DISPLAYCHANGE:	ManagerView_OnDisplayChanged(hwnd, (INT)wParam, LOWORD(lParam), HIWORD(lParam)); return TRUE;
		case WM_SETFONT:		ManagerView_OnSetFont(hwnd, (HFONT)wParam, LOWORD(lParam)); return TRUE;
		case WM_GETFONT:		DIALOG_RESULT(hwnd, ManagerView_OnGetFont(hwnd));
		case WM_COMMAND:		ManagerView_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_NOTIFY:			DIALOG_RESULT(hwnd, ManagerView_OnNotify(hwnd, (NMHDR*)lParam)); 
		case WM_HSCROLL:		ManagerView_OnHorzScroll(hwnd, LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_HELP:			DIALOG_RESULT(hwnd, ManagerView_OnHelp(hwnd, (HELPINFO*)lParam));

		// gen_ml flickerless drawing
		case WM_USER + 0x200:		DIALOG_RESULT(hwnd, 1);
		case WM_USER + 0x201:		ManagerView_OnSetUpdateRegion(hwnd, (HRGN)lParam, MAKEPOINTS(wParam)); return TRUE;

		case MANAGERVIEW_WM_ZOOMSLIDER:			DIALOG_RESULT(hwnd, ManagerView_OnGetZoomSlider(hwnd));
		case MANAGERVIEW_WM_STATUSBAR:			DIALOG_RESULT(hwnd, ManagerView_OnGetStatusBar(hwnd));
	}
	return 0;
}