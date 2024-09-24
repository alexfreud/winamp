#include "main.h"
#include "./infoWidget.h"

#define INFOWIDGET_OFFSET_LEFT_DLU				4
#define INFOWIDGET_OFFSET_TOP_DLU				2
#define INFOWIDGET_OFFSET_RIGHT_DLU			4
#define INFOWIDGET_OFFSET_BOTTOM_DLU			2

#define INFOWIDGET_MIN_WIDTH_DLU				(8*4)
#define INFOWIDGET_MAX_WIDTH_DLU				(48*4)

#define INFOWIDGET_TITLE_OFFSET_BOTTOM_DLU		24
#define INFOWIDGET_IMAGE_OFFSET_BOTTOM_DLU		24


typedef struct InfoWidget
{
	wchar_t *title;
	wchar_t *text;
	wchar_t *imagePath;
	HBITMAP image;
	RECT titleRect;
	RECT textRect;
	RECT imageRect;
	BackBuffer backBuffer;
} InfoWidget;

typedef struct InfoWidgetParam
{
	const wchar_t *title;
	const wchar_t *text;
	const wchar_t *imagePath;
} InfoWidgetParam;

static BOOL 
InfoWidget_InitCb(HWND hwnd, void **object, void *param)
{
	InfoWidget *self;
	const InfoWidgetParam *createParam;

	self = (InfoWidget*)malloc(sizeof(InfoWidget));
	if (NULL == self)
		return FALSE;

	ZeroMemory(self, sizeof(InfoWidget));


	createParam = (InfoWidgetParam*)param;

	if (NULL != createParam)
	{
		wchar_t buffer[4096] = {0};

		if (FALSE != IS_INTRESOURCE(createParam->title))
		{
			if (NULL != WASABI_API_LNG)
			{
				WASABI_API_LNGSTRINGW_BUF((int)(INT_PTR)createParam->title, buffer, ARRAYSIZE(buffer));
				self->title = String_Duplicate(buffer);
			}
			else
				self->title = NULL;
		}
		else
			self->title = String_Duplicate(createParam->title);

		if (FALSE != IS_INTRESOURCE(createParam->text))
		{
			if (NULL != WASABI_API_LNG)
			{
				WASABI_API_LNGSTRINGW_BUF((int)(INT_PTR)createParam->text, buffer, ARRAYSIZE(buffer));
				self->text = String_Duplicate(buffer);
			}
			else
				self->text = NULL;
		}
		else
			self->text = String_Duplicate(createParam->text);

		self->imagePath = ResourceString_Duplicate(createParam->imagePath);
	}

	BackBuffer_Initialize(&self->backBuffer, hwnd);

	*object = self;

	return TRUE;
}

static void 
InfoWidget_DestroyCb(InfoWidget *self, HWND hwnd)
{
	if (NULL == self)
		return;
	
	String_Free(self->title);
	String_Free(self->text);
	ResourceString_Free(self->imagePath);
	
	if (NULL != self->image)
		DeleteObject(self->image);

	BackBuffer_Uninitialize(&self->backBuffer);
	free(self);
}


static HBITMAP
InfoWidget_GetImage(InfoWidget *self, WidgetStyle *style)
{
	if (NULL == self->image)
	{
		unsigned int flags;

		flags = IMAGE_FILTER_BLEND;

		if (FALSE == IS_INTRESOURCE(self->imagePath))
			flags |= ISF_LOADFROMFILE;
					
		self->image = Image_LoadSkinned(self->imagePath, SRC_TYPE_PNG, flags,
							0, 0, 
							WIDGETSTYLE_IMAGE_BACK_COLOR(style), 
							WIDGETSTYLE_IMAGE_FRONT_COLOR(style), 
							WIDGETSTYLE_BACK_COLOR(style));
			
	}

	return self->image;
}

static BOOL
InfoWidget_GetImageSize(InfoWidget *self, WidgetStyle *style, SIZE *size)
{
	HBITMAP image;
	BITMAP imageInfo;

	image = InfoWidget_GetImage(self, style);
	if (NULL == image)
		return FALSE;
			
	if (sizeof(imageInfo) != GetObject(image, sizeof(imageInfo), &imageInfo))
		return FALSE;

	size->cx = imageInfo.bmWidth;
	size->cy = imageInfo.bmHeight;
	if (size->cy < 0)
		size->cy = -size->cy;
	
	return TRUE;
}

static BOOL
InfoWidget_GetTextSize(HDC hdc, HFONT font, const wchar_t *text, long width, 
						unsigned int format, SIZE *size)
{
	RECT rect;
	BOOL result;
	HFONT prevFont;

	if (FALSE != IS_STRING_EMPTY(text))
	{
		size->cx = 0;
		size->cy = 0;
		return TRUE;
	}
	
	prevFont = SelectFont(hdc, font);
	
	SetRect(&rect, 0, 0, width, 0);
	result = DrawText(hdc, text, -1, &rect, DT_CALCRECT | format);
	if (FALSE != result)
	{
		size->cx = RECTWIDTH(rect);
		size->cy = RECTHEIGHT(rect);
	}
	
	SelectFont(hdc, prevFont);

	return result;	
}

static long
InfoWidget_GetClientWidth(WidgetStyle *style, long viewWidth)
{
	long test;

	viewWidth -= (WIDGETSTYLE_DLU_TO_HORZ_PX(style, INFOWIDGET_OFFSET_LEFT_DLU) + 
				  WIDGETSTYLE_DLU_TO_HORZ_PX(style, INFOWIDGET_OFFSET_RIGHT_DLU));
	
	test = WIDGETSTYLE_DLU_TO_HORZ_PX(style, INFOWIDGET_MIN_WIDTH_DLU);
	if (viewWidth < test)
		return test;

	test = WIDGETSTYLE_DLU_TO_HORZ_PX(style, INFOWIDGET_MAX_WIDTH_DLU);
	if (viewWidth > test)
		return test;

	return viewWidth;
}

static void
InfoWidget_LayoutCb(InfoWidget *self, HWND hwnd, WidgetStyle *style, 
					 const RECT *clientRect, SIZE *viewSize, BOOL redraw)
{
	HDC windowDC;
	LONG offsetX, offsetY;
	SIZE widgetSize;
	RECT offsetRect;

	if (NULL == self || NULL == style)
		return;

	windowDC = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == windowDC)
		return;
	
	offsetRect.left = WIDGETSTYLE_DLU_TO_HORZ_PX(style, INFOWIDGET_OFFSET_LEFT_DLU);
	offsetRect.top = WIDGETSTYLE_DLU_TO_VERT_PX(style, INFOWIDGET_OFFSET_TOP_DLU);
	offsetRect.right = WIDGETSTYLE_DLU_TO_HORZ_PX(style, INFOWIDGET_OFFSET_RIGHT_DLU);
	offsetRect.bottom = WIDGETSTYLE_DLU_TO_VERT_PX(style, INFOWIDGET_OFFSET_BOTTOM_DLU);

	widgetSize.cx = InfoWidget_GetClientWidth(style, RECTWIDTH(*clientRect));
	widgetSize.cy = 0;

	SetRectEmpty(&self->imageRect);
	if (FALSE != InfoWidget_GetImageSize(self, style, ((SIZE*)&self->imageRect) + 1))
	{
		if (widgetSize.cx < self->imageRect.right)
			widgetSize.cx = self->imageRect.right;

		widgetSize.cy += self->imageRect.bottom;
		if (0 != self->imageRect.bottom)
			widgetSize.cy += WIDGETSTYLE_DLU_TO_VERT_PX(style, INFOWIDGET_IMAGE_OFFSET_BOTTOM_DLU);
	}
		
	SetRectEmpty(&self->titleRect);
	if (FALSE != InfoWidget_GetTextSize(windowDC, style->titleFont, self->title, widgetSize.cx, 
					DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK, ((SIZE*)&self->titleRect) + 1))
	{
		widgetSize.cy += self->titleRect.bottom;
		if (0 != self->titleRect.bottom)
			widgetSize.cy += WIDGETSTYLE_DLU_TO_VERT_PX(style, INFOWIDGET_TITLE_OFFSET_BOTTOM_DLU);
	}

	SetRectEmpty(&self->textRect);
	if (FALSE != InfoWidget_GetTextSize(windowDC, style->textFont, self->text, widgetSize.cx, 
			DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK, ((SIZE*)&self->textRect) + 1))
	{
		widgetSize.cy += self->textRect.bottom;
	}

	
	if ((widgetSize.cx + (offsetRect.left + offsetRect.right)) < RECTWIDTH(*clientRect))
		offsetX = clientRect->left + (RECTWIDTH(*clientRect) - widgetSize.cx)/2;
	else
		offsetX = clientRect->left + offsetRect.left;

	if ((widgetSize.cy + (offsetRect.top + offsetRect.bottom)) < RECTHEIGHT(*clientRect))
		offsetY = clientRect->top + (RECTHEIGHT(*clientRect) - widgetSize.cy)/2;
	else
		offsetY = clientRect->top + offsetRect.top;
		
				
	if (FALSE == IsRectEmpty(&self->titleRect))
	{		
		OffsetRect(&self->titleRect, offsetX + (widgetSize.cx - self->titleRect.right)/2, offsetY);
		offsetY = self->titleRect.bottom;
		offsetY += WIDGETSTYLE_DLU_TO_VERT_PX(style, INFOWIDGET_TITLE_OFFSET_BOTTOM_DLU);
	}

	if (FALSE == IsRectEmpty(&self->imageRect))
	{
		OffsetRect(&self->imageRect, offsetX + (widgetSize.cx - self->imageRect.right)/2, offsetY);
		offsetY = self->imageRect.bottom;
		offsetY += WIDGETSTYLE_DLU_TO_VERT_PX(style, INFOWIDGET_IMAGE_OFFSET_BOTTOM_DLU);
	}

	if (FALSE == IsRectEmpty(&self->textRect))
	{
		OffsetRect(&self->textRect, offsetX + (widgetSize.cx - self->textRect.right)/2, offsetY);
	}

	ReleaseDC(hwnd, windowDC);

	viewSize->cx = widgetSize.cx + offsetRect.left + offsetRect.right;
	viewSize->cy = widgetSize.cy + offsetRect.top + offsetRect.bottom;
}


static BOOL 
InfoWidget_PaintCb(InfoWidget *self, HWND hwnd, WidgetStyle *style, HDC hdc, const RECT *paintRect, BOOL erase)
{
	RECT  intersectRect;
	FillRegion fillRegion;

	FillRegion_Init(&fillRegion, paintRect);

	if (FALSE == IS_STRING_EMPTY(self->title) && 
		FALSE != IntersectRect(&intersectRect, &self->titleRect, paintRect))
	{
		if (FALSE != BackBuffer_DrawTextEx(&self->backBuffer, hdc, self->title, -1, &self->titleRect, 
							DT_CENTER | DT_NOPREFIX | DT_WORDBREAK, 
							style->titleFont, style->backColor, style->titleColor, OPAQUE))
		{
			FillRegion_ExcludeRect(&fillRegion, &intersectRect);
		}
	}

	if (NULL != self->image	&& 
		FALSE != IntersectRect(&intersectRect, &self->imageRect, paintRect))
	{
		HDC windowDC = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != windowDC)
		{
			HDC sourceDC = CreateCompatibleDC(windowDC);
			if (NULL != sourceDC)
			{
				HBITMAP prevBitmap = SelectBitmap(sourceDC, self->image);
				if (FALSE != BitBlt(hdc, intersectRect.left, intersectRect.top, 
									RECTWIDTH(intersectRect), RECTHEIGHT(intersectRect), 
									sourceDC, 
									intersectRect.left - self->imageRect.left, 
									intersectRect.top - self->imageRect.top,
									SRCCOPY))
				{
					FillRegion_ExcludeRect(&fillRegion, &intersectRect);
				}
				
				SelectBitmap(sourceDC, prevBitmap);
				DeleteDC(sourceDC);
			}
			ReleaseDC(hwnd, windowDC);
		}
	}

	if (FALSE == IS_STRING_EMPTY(self->text) && 
		FALSE != IntersectRect(&intersectRect, &self->textRect, paintRect))
	{
		if (FALSE != BackBuffer_DrawTextEx(&self->backBuffer, hdc, self->text, -1, &self->textRect, 
							DT_CENTER | DT_NOPREFIX | DT_WORDBREAK,
							style->textFont, style->backColor, style->textColor, OPAQUE))
		{
			FillRegion_ExcludeRect(&fillRegion, &intersectRect);
		}
	}


	if (FALSE != erase)
		FillRegion_BrushFill(&fillRegion, hdc, style->backBrush);

	FillRegion_Uninit(&fillRegion);

	return TRUE;
}

static void
InfoWidget_StyleColorChangedCb(InfoWidget *self, HWND hwnd, WidgetStyle *style)
{
	if (NULL == self)
		return;

	if (NULL != self->image)
	{
		if (NULL != self->image)
			DeleteObject(self->image);
		
		self->image = NULL;
		InfoWidget_GetImage(self, style);
	}
}

HWND InfoWidget_CreateWindow(unsigned int type, const wchar_t *title, const wchar_t *text, 
							 const wchar_t *imagePath, HWND parentWindow, 
							 int x, int y, int width, int height, BOOL border, unsigned int controlId)
{
	const static WidgetInterface infoWidgetInterface =
	{
		(WidgetInitCallback)InfoWidget_InitCb,
		(WidgetDestroyCallback)InfoWidget_DestroyCb,
		(WidgetLayoutCallback)InfoWidget_LayoutCb,
		(WidgetPaintCallback)InfoWidget_PaintCb,
		(WidgetStyleCallback)InfoWidget_StyleColorChangedCb,
	};

	InfoWidgetParam param;

	param.title = title;
	param.text = text;
	param.imagePath = imagePath;

	return Widget_CreateWindow(type,
								&infoWidgetInterface, 
								NULL,
								(FALSE != border) ? WS_EX_CLIENTEDGE : 0,
								0, 
								x, y, width, height, 
								parentWindow, 
								controlId, &param);
}