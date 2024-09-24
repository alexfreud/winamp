#include "main.h"
#include "./welcomeWidget.h"
#include "../winamp/commandLink.h"

#include <strsafe.h>

#define WELCOMEWIDGET_OFFSET_LEFT_DLU			4
#define WELCOMEWIDGET_OFFSET_TOP_DLU			2
#define WELCOMEWIDGET_OFFSET_RIGHT_DLU			4
#define WELCOMEWIDGET_OFFSET_BOTTOM_DLU			2

#define WELCOMEWIDGET_IMAGE_MIN_WIDTH			280
#define WELCOMEWIDGET_IMAGE_MIN_HEIGHT			(424 - 200)

#define WELCOMEWIDGET_TITLE_MAX_HEIGHT			168
#define WELCOMEWIDGET_TITLE_MIN_WIDTH			WELCOMEWIDGET_IMAGE_MIN_WIDTH

#define WELCOMEWIDGET_TEXT_MAX_HEIGHT			66
#define WELCOMEWIDGET_TEXT_MIN_WIDTH			WELCOMEWIDGET_IMAGE_MIN_WIDTH

#define WELCOMEWIDGET_HELP_LINK					1000

typedef struct WelcomeWidget
{
	HDC context;
	long contextWidth;
	long contextHeight;
	HBITMAP previousBitmap;
	HFONT helpFont;
} WelcomeWidget;

static BOOL
WelcomeWidget_GetBestTextRect(HDC hdc, const wchar_t *text, unsigned int length, 
							  RECT *rect, long maxWidth, unsigned int format)
{
	RECT testRect;
	long right;

	if (NULL == rect)
		return FALSE;

	format |= DT_CALCRECT;

	if (length < 0)
		length = (NULL != text) ? lstrlen(text) : 0;
	
	if (0 == length || NULL == text)
	{
		rect->right = rect->left;
		rect->bottom = rect->top;
		return TRUE;
	}


	SetRect(&testRect, rect->left, rect->top, rect->right, rect->top);
	maxWidth += rect->left;
	right = rect->right;

	for(;;)
	{
		if (FALSE == DrawText(hdc, text, length, &testRect, format))
			return FALSE;

		if (testRect.bottom <= rect->bottom || right >= maxWidth)
		{
			CopyRect(rect, &testRect);
			break;
		}

		long increment = maxWidth - right;
		if (increment > 60)
			increment = increment / 4;
		else if (increment > 4)
			increment = increment / 2;

		right += increment;
		testRect.right = right;
		testRect.bottom = rect->top;
	}

	return TRUE;
}

static COLORREF 
WelcomeWidget_GetTextColor(WidgetStyle *style, COLORREF *shadowColor, BOOL titleMode)
{
	COLORREF frontColor, backColor;
	WORD backHue, backLuma, backSat, frontHue, frontLuma, frontSat;

	backColor = WIDGETSTYLE_BACK_COLOR(style);
	frontColor = WIDGETSTYLE_TEXT_COLOR(style);
	
	ColorRGBToHLS(backColor, &backHue, &backLuma, &backSat);
	ColorRGBToHLS(frontColor, &frontHue, &frontLuma, &frontSat);

	if(frontLuma > backLuma)
	{
		if (NULL != shadowColor)
		{
			if (FALSE != titleMode || backLuma > 180)
				*shadowColor = Graphics_BlendColors(0x000000, backColor, 102);
			else
				*shadowColor = backColor;
		}
		
		return Graphics_BlendColors((FALSE != titleMode) ? 0xBBBBBB : 0xEEEEEE, frontColor, 230);
	}

	if (NULL != shadowColor)
	{
		if (FALSE != titleMode || backLuma < 60)
			*shadowColor = Graphics_BlendColors(0xFFFFFF, backColor, 102);
		else
			*shadowColor = backColor;
	}
	
	return Graphics_BlendColors((FALSE != titleMode) ? 0x333333 : 0x111111, frontColor, 230);
}

static HFONT
WelcomeWidget_CreateTitleFont(WidgetStyle *style, HDC hdc)
{
	LOGFONT lf;
	HFONT textFont;
	long height, pixelsY;

	pixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
	height = MulDiv(16, pixelsY, 96);

	textFont = WIDGETSTYLE_TITLE_FONT(style);
	if (NULL != textFont)
	{
		HFONT prevFont;
		TEXTMETRIC tm;

		prevFont = SelectFont(hdc, textFont);
		if (FALSE != GetTextMetricsW(hdc, &tm))
		{
			long test = MulDiv(tm.tmHeight - tm.tmInternalLeading, 96, pixelsY);
			test = MulDiv((test + 2), pixelsY, 72);
			if (test > height)
				height = test;
		}
		SelectFont(hdc, prevFont);
	}

	lf.lfHeight = -height;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_DONTCARE;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = Graphics_GetSysFontQuality();
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"Arial");

	return CreateFontIndirect(&lf);
}

static BOOL
WelcomeWidget_FillContext(HDC targetDC, long width, long height, HDC sourceDC, 
							  HBITMAP backBitmap, long backWidth, long backHeight, WidgetStyle *style)
{
	RECT rect, partRect;
	wchar_t buffer[4096] = {0};
	int bufferLength;
	COLORREF textColor, shadowColor;
	HFONT prevFont;

	if (NULL == targetDC || 
		NULL == sourceDC || 
		NULL == style)
	{
		return FALSE;
	}

	SetRect(&rect, 0, 0, width, height);

	FillRect(targetDC, &rect, WIDGETSTYLE_BACK_BRUSH(style));
	

	if (NULL != backBitmap)
	{
		Image_AlphaBlend(targetDC, &rect, sourceDC, NULL, 255, backBitmap, NULL, 
			AlphaBlend_AlignCenter | AlphaBlend_AlignVCenter, NULL);
	}

	SetBkMode(targetDC, TRANSPARENT);
	
	
	WASABI_API_LNGSTRINGW_BUF(IDS_WELCOMEWIDGET_TITLE, buffer, ARRAYSIZE(buffer));
	if (L'\0' != buffer[0])
	{
		BOOL textMode;

		textMode = TRUE;

		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, buffer, -1, L"WINAMP DEVICE MANAGER", -1))
		{
			HBITMAP titleBitmap;
			titleBitmap = Image_LoadEx(Plugin_GetInstance(), MAKEINTRESOURCE(IDR_WELCOME_WIDGET_TITLE_IMAGE), 
										SRC_TYPE_PNG, ISF_PREMULTIPLY, 0, 0);
			if (NULL != titleBitmap)
			{
				SetRect(&partRect, rect.left, rect.top, rect.right, rect.top + WELCOMEWIDGET_TITLE_MAX_HEIGHT);

				if (FALSE != Image_AlphaBlend(targetDC, &partRect, sourceDC, NULL, 255, titleBitmap, NULL, 
									AlphaBlend_AlignCenter | AlphaBlend_AlignVCenter, NULL))
				{
					textMode = FALSE;
				}

				DeleteObject(titleBitmap);
			}
		}

		if (FALSE != textMode)
		{
			HFONT titleFont;

			bufferLength = lstrlen(buffer);
			
			titleFont = WelcomeWidget_CreateTitleFont(style, targetDC);

			textColor = WelcomeWidget_GetTextColor(style, &shadowColor, TRUE);
			prevFont = SelectFont(targetDC, (NULL != titleFont) ? titleFont : WIDGETSTYLE_TITLE_FONT(style));

			SetRect(&partRect, 0, 0, width, 0);

			if (FALSE != DrawText(targetDC, buffer, bufferLength, &partRect, 
						DT_CALCRECT | DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL | DT_END_ELLIPSIS))
			{
				if (partRect.bottom > WELCOMEWIDGET_TITLE_MAX_HEIGHT)
				{
					TEXTMETRIC textMetrics;
					if (FALSE != GetTextMetrics(targetDC, &textMetrics))
						partRect.bottom = (WELCOMEWIDGET_TITLE_MAX_HEIGHT/textMetrics.tmHeight)*textMetrics.tmHeight;
				}

				OffsetRect(&partRect, 
							rect.left + (RECTWIDTH(rect) - RECTWIDTH(partRect))/2, 
							rect.top  + (WELCOMEWIDGET_TITLE_MAX_HEIGHT - RECTHEIGHT(partRect))/2);
		
				OffsetRect(&partRect, 0, 2);
				SetTextColor(targetDC, shadowColor);
				DrawText(targetDC, buffer, bufferLength, &partRect, 
						DT_CENTER | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL | DT_END_ELLIPSIS);

				OffsetRect(&partRect, 0, -2);
				
				SetTextColor(targetDC, textColor);
				DrawText(targetDC, buffer, bufferLength, &partRect, 
						DT_CENTER | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL | DT_END_ELLIPSIS);
			}

			SelectFont(targetDC, prevFont);
			if (NULL != titleFont)
				DeleteObject(titleFont);
		}
	}

	WASABI_API_LNGSTRINGW_BUF(IDS_WELCOMEWIDGET_TEXT, buffer, ARRAYSIZE(buffer));
	if (L'\0' != buffer[0])
	{
		bufferLength = lstrlen(buffer);

		textColor = WelcomeWidget_GetTextColor(style, &shadowColor, FALSE);
		prevFont = SelectFont(targetDC, WIDGETSTYLE_TEXT_FONT(style));

		SetRect(&partRect, 0, 0, WELCOMEWIDGET_TEXT_MIN_WIDTH, WELCOMEWIDGET_TEXT_MAX_HEIGHT);

		if (FALSE != WelcomeWidget_GetBestTextRect(targetDC, buffer, bufferLength, &partRect, width, 
					DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL | DT_END_ELLIPSIS))
		{
			if (partRect.bottom > WELCOMEWIDGET_TEXT_MAX_HEIGHT)
			{
				TEXTMETRIC textMetrics;
				if (FALSE != GetTextMetrics(targetDC, &textMetrics))
					partRect.bottom = (WELCOMEWIDGET_TEXT_MAX_HEIGHT/textMetrics.tmHeight)*textMetrics.tmHeight;
			}

			OffsetRect(&partRect, 
						rect.left + (RECTWIDTH(rect) - RECTWIDTH(partRect))/2, 
						rect.bottom - WELCOMEWIDGET_TEXT_MAX_HEIGHT);
	
			OffsetRect(&partRect, 0, 1);
			SetTextColor(targetDC, shadowColor);
			DrawText(targetDC, buffer, bufferLength, &partRect, 
					DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL | DT_END_ELLIPSIS);

			OffsetRect(&partRect, 0, -1);
			
			SetTextColor(targetDC, textColor);
			DrawText(targetDC, buffer, bufferLength, &partRect, 
					DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL | DT_END_ELLIPSIS);
		}

		SelectFont(targetDC, prevFont);
	}

	return TRUE;
}

static HDC
WelcomeWidget_CreateContext(HWND hwnd, WidgetStyle *style, HBITMAP *prevBitmapOut, long *widthOut, long *heightOut)
{
	
	HDC windowDC, targetDC, sourceDC;
	HBITMAP backBitmap, targetBitmap;
	HBITMAP prevTargetBitmap;
	long width, height, backWidth, backHeight;

	if (NULL == prevBitmapOut || NULL == style)
		return NULL;

	windowDC = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == windowDC)
		return NULL;

	backBitmap = Image_LoadEx(Plugin_GetInstance(), MAKEINTRESOURCE(IDR_WELCOME_WIDGET_IMAGE), 
							SRC_TYPE_PNG, ISF_PREMULTIPLY, 0, 0);

	width = WELCOMEWIDGET_IMAGE_MIN_WIDTH;
	height = WELCOMEWIDGET_IMAGE_MIN_HEIGHT;
	backWidth = 0;
	backHeight = 0;

	if (NULL != backBitmap)
	{
		BITMAP bi;
		if (sizeof(bi) == GetObject(backBitmap, sizeof(bi), &bi))
		{
			backWidth = bi.bmWidth;
			if (backWidth > width)
				width = backWidth;

			backHeight = ABS(bi.bmHeight);
			if (backHeight > height)
				height = backHeight;
		}
	}

	targetDC = CreateCompatibleDC(windowDC);
	sourceDC = CreateCompatibleDC(windowDC);
	targetBitmap = CreateCompatibleBitmap(windowDC, width, height);
	prevTargetBitmap = NULL;

	ReleaseDC(hwnd, windowDC);

	if (NULL != targetDC && 
		NULL != sourceDC && 
		NULL != targetBitmap)
	{

		HFONT prevTargetFont;
		prevTargetBitmap = SelectBitmap(targetDC, targetBitmap);
		prevTargetFont = GetCurrentFont(targetDC); 
		HBITMAP prevSourceBitmap = GetCurrentBitmap(sourceDC);

		if (FALSE == WelcomeWidget_FillContext(targetDC, width, height, sourceDC, 
											backBitmap, backWidth, backHeight, style))
		{
			SelectBitmap(targetDC, prevTargetBitmap);
			prevTargetBitmap = NULL;
			DeleteObject(targetBitmap);
			targetBitmap = NULL;
		}
		
		SelectFont(targetDC, prevTargetFont);
		SelectBitmap(sourceDC, prevSourceBitmap);
	}

	if (NULL != sourceDC)
		DeleteDC(sourceDC);

	if (NULL != targetDC && NULL == targetBitmap)
	{
		DeleteDC(targetDC);
		targetDC = NULL;
		prevTargetBitmap = NULL;
	}

	if (NULL != backBitmap)
		DeleteObject(backBitmap);

	*prevBitmapOut = prevTargetBitmap;
	if (NULL != widthOut)
		*widthOut = width;
	if (NULL != heightOut)
		*heightOut = height;

	return targetDC;
}

static void
WelcomeWidget_ResetContext(WelcomeWidget * self)
{
	if (NULL == self ||
		NULL == self->context)
	{
		return;
	}

	if (NULL != self->previousBitmap)
	{
		HBITMAP bitmap = SelectBitmap(self->context, self->previousBitmap);
		if (NULL != bitmap)
			DeleteObject(bitmap);
	}

	DeleteDC(self->context);
	self->context = NULL;
	self->previousBitmap = NULL;
	self->contextWidth = 0;
	self->contextHeight = 0;
}

static BOOL
WelcomeWidget_GetViewOrigin(HWND hwnd, POINT *pt)
{
	SCROLLINFO scrollInfo;

	if (NULL == pt)
		return FALSE;

	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_POS;
		
	if (FALSE == GetScrollInfo(hwnd, SB_HORZ, &scrollInfo))
		return FALSE;
	pt->x = -scrollInfo.nPos;

	if (FALSE == GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		return FALSE;
	pt->y = -scrollInfo.nPos;

	return TRUE;
}

static BOOL
WelcomeWidget_UpdateHelpPosition(WelcomeWidget *self, HWND hwnd, const RECT *clientRect, BOOL redraw)
{
	HWND elementWindow;
	RECT elementRect;
	unsigned int positionFlags;
	long left, top;
	POINT origin;

	if (NULL == self || NULL == hwnd || NULL == clientRect)
		return FALSE;

	elementWindow = GetDlgItem(hwnd, WELCOMEWIDGET_HELP_LINK);
	if (NULL == elementWindow)
		return FALSE;
	

	if (FALSE == WelcomeWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}

	if (FALSE == GetWindowRect(elementWindow, &elementRect))
		return FALSE;

	positionFlags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE;
	if (FALSE == redraw)
		positionFlags |= SWP_NOREDRAW;
		
	left = clientRect->right;
	left -= (RECTWIDTH(elementRect) + 2);
	if (left < clientRect->left)
		left = clientRect->left;

	top = clientRect->top;
	top += 2;

	return SetWindowPos(elementWindow, NULL, left, origin.y + top, 0, 0, positionFlags);
}

static BOOL
WelcomeWidget_EnsureHelpVisible(WelcomeWidget *self, HWND hwnd)
{
	HWND elementWindow;
	POINT origin;

	if (NULL == self || NULL == hwnd)
		return FALSE;

	elementWindow = GetDlgItem(hwnd, WELCOMEWIDGET_HELP_LINK);
	if (NULL == elementWindow)
		return FALSE;
	
	if (FALSE != WelcomeWidget_GetViewOrigin(hwnd, &origin) && 
		0 != origin.y)
	{
		return WIDGET_SCROLL(hwnd, 0, origin.y, TRUE);
	}

	return FALSE;
}

static BOOL
WelcomeWidget_GetHelpUrl(wchar_t *buffer, size_t bufferMax)
{
	if (NULL == buffer)
		return FALSE;

	WASABI_API_LNGSTRINGW_BUF(IDS_WELCOMEWIDGET_HELP_URL, buffer, bufferMax);
	return (L'\0' != buffer[0]);
}

static BOOL 
WelcomeWidget_InitCb(HWND hwnd, void **object, void *param)
{
	wchar_t buffer[64] = {0};
	WelcomeWidget *self = (WelcomeWidget*)malloc(sizeof(WelcomeWidget));

	if (NULL == self)
		return FALSE;

	ZeroMemory(self, sizeof(WelcomeWidget));
	WIDGET_ENABLE_CHILDREN_SCROLL(hwnd, FALSE);


	WASABI_API_LNGSTRINGW_BUF(IDS_HELP_LINK, buffer, ARRAYSIZE(buffer));
	CommandLink_CreateWindow(WS_EX_NOPARENTNOTIFY, 
							buffer, 
							WS_CHILD | WS_VISIBLE | WS_TABSTOP |
							/*CLS_ALWAYSUNDERLINE |*/ CLS_HOTTRACK | CLS_HIGHLIGHTCOLOR,
							0, 0, 0, 0, hwnd, WELCOMEWIDGET_HELP_LINK);

	*object = self;

	return TRUE;
}

static void 
WelcomeWidget_DestroyCb(WelcomeWidget *self, HWND hwnd)
{
	if (NULL == self)
		return;

	WelcomeWidget_ResetContext(self);
	if (NULL != self->helpFont)
		DeleteObject(self->helpFont);

	free(self);
}


static void
WelcomeWidget_LayoutCb(WelcomeWidget *self, HWND hwnd, WidgetStyle *style, 
					 const RECT *clientRect, SIZE *viewSize, BOOL redraw)
{
	if (NULL == self || NULL == style)
		return;

	if (NULL == self->context)
	{
		self->context = WelcomeWidget_CreateContext(hwnd, style, &self->previousBitmap, &self->contextWidth, &self->contextHeight);
		if (NULL == self->context)
			return;
	}
	
	viewSize->cx = self->contextWidth + 
					WIDGETSTYLE_DLU_TO_HORZ_PX(style, WELCOMEWIDGET_OFFSET_LEFT_DLU);
	
	viewSize->cy = self->contextHeight + 
					WIDGETSTYLE_DLU_TO_VERT_PX(style, WELCOMEWIDGET_OFFSET_TOP_DLU);
	

	WelcomeWidget_UpdateHelpPosition(self, hwnd, clientRect, redraw);
}


static BOOL 
WelcomeWidget_PaintCb(WelcomeWidget *self, HWND hwnd, WidgetStyle *style, HDC hdc, const RECT *paintRect, BOOL erase)
{
	FillRegion fillRegion;

	FillRegion_Init(&fillRegion, paintRect);
	
	if (NULL != self->context)
	{
		RECT partRect, rect;
		long offsetX, offsetY;

		GetClientRect(hwnd, &rect);

		SetRect(&partRect, 0, 0, self->contextWidth, self->contextHeight);
	
		offsetX = rect.left + WIDGETSTYLE_DLU_TO_HORZ_PX(style, WELCOMEWIDGET_OFFSET_LEFT_DLU);
		offsetY = WIDGETSTYLE_DLU_TO_HORZ_PX(style, WELCOMEWIDGET_OFFSET_RIGHT_DLU);
		if ((offsetX + partRect.right + offsetY) < rect.right)
			offsetX += (rect.right - (offsetX + partRect.right + offsetY))/2;

		offsetY = rect.top + WIDGETSTYLE_DLU_TO_VERT_PX(style, WELCOMEWIDGET_OFFSET_TOP_DLU);

		OffsetRect(&partRect, offsetX, offsetY);
		
		if (FALSE != IntersectRect(&rect, &partRect, paintRect) && 
			FALSE != BitBlt(hdc, rect.left, rect.top, RECTWIDTH(rect), RECTHEIGHT(rect), 
							self->context, rect.left - partRect.left, rect.top - partRect.top, SRCCOPY))
		{
			FillRegion_ExcludeRect(&fillRegion, &rect);
		}

	}


	if (FALSE != erase)
		FillRegion_BrushFill(&fillRegion, hdc, WIDGETSTYLE_BACK_BRUSH(style));

	FillRegion_Uninit(&fillRegion);

	return TRUE;
}

static void
WelcomeWidget_StyleColorChangedCb(WelcomeWidget *self, HWND hwnd, WidgetStyle *style)
{
	HWND elementWindow;

	if (NULL == self)
		return;

	WelcomeWidget_ResetContext(self);
	self->context = WelcomeWidget_CreateContext(hwnd, style, &self->previousBitmap, &self->contextWidth, &self->contextHeight);

	elementWindow = GetDlgItem(hwnd, WELCOMEWIDGET_HELP_LINK);
	if (NULL != elementWindow)
	{
		CommandLink_SetBackColor(elementWindow, WIDGETSTYLE_BACK_COLOR(style));
		CommandLink_SetTextColor(elementWindow, WelcomeWidget_GetTextColor(style, NULL, TRUE));
		CommandLink_SetHighlightColor(elementWindow, WelcomeWidget_GetTextColor(style, NULL, FALSE));
	}
}

static void
WelcomeWidget_StyleFontChangedCb(WelcomeWidget *self, HWND hwnd, WidgetStyle *style)
{
	HWND elementWindow;

	if (NULL == self)
		return;

	WelcomeWidget_ResetContext(self);
	self->context = WelcomeWidget_CreateContext(hwnd, style, &self->previousBitmap, &self->contextWidth, &self->contextHeight);

	elementWindow = GetDlgItem(hwnd, WELCOMEWIDGET_HELP_LINK);
	if (NULL != elementWindow)
	{
		SIZE elementSize;

		if (NULL != self->helpFont)
			DeleteObject(self->helpFont);

		self->helpFont = Graphics_DuplicateFont(WIDGETSTYLE_TEXT_FONT(style), 0, TRUE, TRUE);

		SendMessage(elementWindow, WM_SETFONT, (WPARAM)self->helpFont, 0L);
		if (FALSE != CommandLink_GetIdealSize(elementWindow, &elementSize))
		{
			SetWindowPos(elementWindow, NULL, 0, 0, elementSize.cx, elementSize.cy, 
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		}
	}
}

static void
WelcomeWidget_ScrollCb(WelcomeWidget *self, HWND hwnd, int *dx, int *dy)
{	
	RECT clientRect;
	if (FALSE != GetClientRect(hwnd, &clientRect))
		WelcomeWidget_UpdateHelpPosition(self, hwnd, &clientRect, TRUE);
	
}

static void 
WelcomeWidget_HelpLinkCb(WelcomeWidget *self, HWND hwnd, NMHDR *pnmh, LRESULT *result)
{
	switch(pnmh->code)
	{
		case NM_CLICK:
			{
				wchar_t buffer[4096] = {0};
				if (FALSE != WelcomeWidget_GetHelpUrl(buffer, ARRAYSIZE(buffer)))
					MediaLibrary_ShowHelp(Plugin_GetLibraryWindow(), buffer);
			}
			break;
		case NM_SETFOCUS:
			WelcomeWidget_EnsureHelpVisible(self, hwnd);
			break;

	}
}

static BOOL
WelcomeWidget_NotifyCb(WelcomeWidget *self, HWND hwnd, NMHDR *pnmh, LRESULT *result)
{
	switch(pnmh->idFrom)
	{
		case WELCOMEWIDGET_HELP_LINK:
			WelcomeWidget_HelpLinkCb(self, hwnd, pnmh, result);
			return TRUE;
	}

	return FALSE;
}

static BOOL
WelcomeWidget_HelpCb(WelcomeWidget *self, HWND hwnd, wchar_t *buffer, size_t bufferMax)
{
	return WelcomeWidget_GetHelpUrl(buffer, bufferMax);
}

HWND WelcomeWidget_CreateWindow(HWND parentWindow, int x, int y, int width, int height, BOOL border, unsigned int controlId)
{
	const static WidgetInterface welcomeWidgetInterface =
	{
		(WidgetInitCallback)WelcomeWidget_InitCb,
		(WidgetDestroyCallback)WelcomeWidget_DestroyCb,
		(WidgetLayoutCallback)WelcomeWidget_LayoutCb,
		(WidgetPaintCallback)WelcomeWidget_PaintCb,
		(WidgetStyleCallback)WelcomeWidget_StyleColorChangedCb,
		(WidgetStyleCallback)WelcomeWidget_StyleFontChangedCb,
		NULL /*mouseMove*/,
		NULL /*leftButtonDown*/,
		NULL /*leftButtonUp*/,
		NULL /*leftButtonDblClk*/,
		NULL /*rightButtonDown*/,
		NULL /*rightButtonUp*/,
		NULL /*keyDown*/,
		NULL /*keyUp*/,
		NULL /*character*/,
		NULL /*inputRequest*/,
		NULL /*focusChanged*/,
		NULL /*contextMenu*/,
		NULL /*zoomChanging*/,
		NULL /*scrollBefore*/,
		(WidgetScrollCallback)WelcomeWidget_ScrollCb,
		(WidgetNotifyCallback)WelcomeWidget_NotifyCb,
		(WidgetHelpCallback)WelcomeWidget_HelpCb,

	};

	return Widget_CreateWindow(WIDGET_TYPE_WELCOME,
								&welcomeWidgetInterface, 
								NULL, 
								((FALSE != border) ? WS_EX_CLIENTEDGE : 0) | WS_EX_CONTROLPARENT, 
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
								x, y, width, height, 
								parentWindow, 
								controlId, 0L);
}