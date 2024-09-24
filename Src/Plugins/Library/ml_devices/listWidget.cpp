#include "main.h"
#include "./listWidget.h"
#include "./listWidgetInternal.h"
#include <vector>

#include <strsafe.h>

#define LISTWIDGET_OFFSET_LEFT_DLU				0
#define LISTWIDGET_OFFSET_TOP_DLU				1
#define LISTWIDGET_OFFSET_BOTTOM_DLU			2

#define LISTWIDGET_CATEGORY_OFFSET_TOP_DLU		0
#define LISTWIDGET_CATEGORY_OFFSET_LEFT_DLU		1
#define LISTWIDGET_CATEGORY_SPACING_DLU			2
#define LISTWIDGET_CATEGORY_SPACING_COLLAPSED_DLU	0

#define LISTWIDGET_ITEM_OFFSET_TOP_DLU			1
#define LISTWIDGET_ITEM_OFFSET_LEFT_DLU			2
#define LISTWIDGET_ITEM_SPACING_HORZ_DLU		2
#define LISTWIDGET_ITEM_SPACING_VERT_DLU		10
#define LISTWIDGET_ITEM_TITLE_MAX_LINES			3

#define LISTWIDGET_IMAGE_MIN_HEIGHT				48
#define LISTWIDGET_IMAGE_MAX_HEIGHT				256
#define LISTWIDGET_IMAGE_DEFAULT_HEIGHT			160//128
#define LISTWIDGET_IMAGE_DEFAULT_WIDTH			160//96

#define LISTWIDGETTIMER_SHOW_COMMANDS_ID		3
#define LISTWIDGETTIMER_SHOW_COMMANDS_DELAY		75
#define LISTWIDGETTIMER_PROGRESS_TICK_ID		4
#define LISTWIDGETTIMER_PROGRESS_TICK_DELAY		130
#define LISTWIDGETTIMER_EDIT_TITLE_ID			5

#define LISTWIDGET_CONNECTION_MIN_HEIGHT		20
#define LISTWIDGET_CONNECTION_MAX_HEIGHT		48
#define LISTWIDGET_CONNECTION_DEFAULT_HEIGHT	36

#define LISTWIDGET_PRIMARYCOMMAND_MIN_HEIGHT		16
#define LISTWIDGET_PRIMARYCOMMAND_MAX_HEIGHT		48
#define LISTWIDGET_PRIMARYCOMMAND_DEFAULT_HEIGHT	36

#define LISTWIDGET_SECONDARYCOMMAND_MIN_HEIGHT		14
#define LISTWIDGET_SECONDARYCOMMAND_MAX_HEIGHT		36
#define LISTWIDGET_SECONDARYCOMMAND_DEFAULT_HEIGHT	20

#define LISTWIDGET_ACTIVITY_MIN_HEIGHT			16
#define LISTWIDGET_ACTIVITY_MAX_HEIGHT			48
#define LISTWIDGET_ACTIVITY_DEFAULT_HEIGHT		36

#define LISTWIDGET_PROGRESS_MIN_HEIGHT			16

#define LISTWIDGET_PROGRESS_FRAME_COUNT			9/*12*/

typedef std::vector<ifc_device*> DeviceList;


static ListWidgetCategory *
ListWidget_CreateCategoryHelper(const char *name, int titleId, wchar_t *buffer, size_t bufferMax)
{
	WASABI_API_LNGSTRINGW_BUF(titleId, buffer, bufferMax);
	return ListWidget_CreateCategory(name,
						buffer, 
						Config_ReadBool("CollapsedCategories", name, FALSE));
}

static void
ListWidget_CreateDefaultCategories(ListWidget *self)
{
	ListWidgetCategory *category;
	wchar_t buffer[512] = {0};

	category = ListWidget_CreateCategoryHelper("attached", IDS_CATEGORY_ATTACHED, buffer, ARRAYSIZE(buffer));
	if (NULL != category)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_CATEGORY_ATTACHED_EMPTY_TEXT, buffer, ARRAYSIZE(buffer));
		ListWidget_SetCategoryEmptyText(category, buffer);
		self->categories.push_back(category);
	}

	category = ListWidget_CreateCategoryHelper("discovered", IDS_CATEGORY_DISCOVERED, buffer, ARRAYSIZE(buffer));
	if (NULL != category)
		self->categories.push_back(category);
}

BOOL
ListWidget_GetViewOrigin(HWND hwnd, POINT *pt)
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


static HBITMAP
ListWidget_CreateSpacebarBitmap(HBITMAP sourceBitmap, HWND hwnd, long width, long height)
{
	HDC windowDC, sourceDC, resultDC;
	HBITMAP resultBitmap;
	BITMAP sourceInfo;
	RECT resultRect, sourceRect;

	if (NULL == sourceBitmap ||
		sizeof(sourceInfo) != GetObject(sourceBitmap, sizeof(sourceInfo), &sourceInfo))
	{
		return FALSE;
	}
	
	windowDC = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == windowDC)
		return NULL;

	sourceDC = CreateCompatibleDC(windowDC);
	resultDC = CreateCompatibleDC(windowDC);
	resultBitmap = CreateCompatibleBitmap(windowDC, width, height*2);

	ReleaseDC(hwnd, windowDC);

	if (NULL != sourceDC &&
		NULL != resultDC &&
		NULL != resultBitmap)
	{
		HBITMAP prevSourceBitmap, prevResultBitmap;
		prevSourceBitmap = SelectBitmap(sourceDC, sourceBitmap);
		prevResultBitmap = SelectBitmap(resultDC, resultBitmap);
	
	
		SetRect(&resultRect, 0, 0, width, height);
		SetRect(&sourceRect, 0, 0, sourceInfo.bmWidth, ABS(sourceInfo.bmHeight)/2);

		Image_FillBorder(resultDC, &resultRect, sourceDC, &sourceRect, TRUE, 255);
		
		OffsetRect(&resultRect, 0, height);
		OffsetRect(&sourceRect, 0, RECTHEIGHT(sourceRect));

		Image_FillBorder(resultDC, &resultRect, sourceDC, &sourceRect, TRUE, 255);


		SelectBitmap(sourceDC, prevSourceBitmap);
		SelectBitmap(resultDC, prevResultBitmap);

	}
	
	if (NULL != sourceDC)
		DeleteDC(sourceDC);
	if (NULL != resultDC)
		DeleteDC(resultDC);

	if (NULL != resultBitmap)
	{
		RECT imageRect;
		SetRect(&imageRect, 0, 0, width, height);
		Image_Premultiply(resultBitmap, &imageRect);
	}

	return resultBitmap;
}

HBITMAP
ListWidget_GetSpacebarBitmap(ListWidget *self, WidgetStyle *style, HWND hwnd, long width, long height)
{

	if (NULL == self)
		return NULL;

	if (NULL != self->spacebarBitmap)
	{
		BITMAP bi;
		if (sizeof(bi) != GetObject(self->spacebarBitmap, sizeof(bi), &bi) ||
			bi.bmWidth != width || 
			bi.bmHeight/2 != height)
		{
			DeleteObject(self->spacebarBitmap);
			self->spacebarBitmap = NULL;
		}
	}
	
	if (NULL == self->spacebarBitmap && 
		NULL != style)
	{
		HBITMAP baseBitmap;
		baseBitmap = Image_Load(MAKEINTRESOURCE(IDR_SPACEBAR_IMAGE), SRC_TYPE_PNG, 0, 0, 0); 
							
		//WIDGETSTYLE_IMAGE_BACK_COLOR(style),  
		//					WIDGETSTYLE_IMAGE_FRONT_COLOR(style),
		//					WIDGETSTYLE_BACK_COLOR(style));
		
		if (NULL != baseBitmap)
		{
			DIBSECTION bitmapData;
						
			if (sizeof(bitmapData) == GetObjectW(baseBitmap, sizeof(bitmapData), &bitmapData))
			{
				BITMAP *bi;
				long bitmapHeight;
				void *pixels;
				WORD backHue, backLuma, backSat, frontHue, frontLuma, frontSat;

				bi = &bitmapData.dsBm;
				bitmapHeight = ABS(bi->bmHeight);
										
				pixels = ((BYTE*)bi->bmBits) + (bi->bmWidthBytes * (bitmapHeight - bitmapHeight/2));

				ColorRGBToHLS(WIDGETSTYLE_IMAGE_BACK_COLOR(style), &backHue, &backLuma, &backSat);
				ColorRGBToHLS(WIDGETSTYLE_IMAGE_FRONT_COLOR(style), &frontHue, &frontLuma, &frontSat);
				
				if (backLuma > frontLuma)
				{
					COLORREF backColor;

					frontLuma = 25;
					backColor = ColorHLSToRGB(frontHue, frontLuma, frontSat);

					Image_FilterEx(pixels, bi->bmWidth, bitmapHeight/2, bi->bmBitsPixel, 
									0, 
									backColor, 
									WIDGETSTYLE_IMAGE_BACK_COLOR(style),
									WIDGETSTYLE_BACK_COLOR(style));

				}
				else
				{
					Image_FilterEx(pixels, bi->bmWidth, bitmapHeight/2, bi->bmBitsPixel, 
									0, 
									WIDGETSTYLE_IMAGE_BACK_COLOR(style), 
									WIDGETSTYLE_IMAGE_FRONT_COLOR(style), 
									WIDGETSTYLE_BACK_COLOR(style));
				}
			}
		

			Image_Premultiply(baseBitmap, NULL);
			self->spacebarBitmap = ListWidget_CreateSpacebarBitmap(baseBitmap, hwnd, width, height);
			DeleteObject(baseBitmap);
		}
	}

	return self->spacebarBitmap;
}

static HBITMAP
ListWidget_CreateBorderBitmap(HBITMAP sourceBitmap, HWND hwnd, long width, long height)
{
	HDC windowDC, sourceDC, resultDC;
	HBITMAP resultBitmap;
	BITMAP sourceInfo;
	RECT resultRect, sourceRect;
	
	if (NULL == sourceBitmap ||
		sizeof(sourceInfo) != GetObject(sourceBitmap, sizeof(sourceInfo), &sourceInfo))
	{
		return FALSE;
	}

	SetRect(&resultRect, 0, 0, width, height);
	SetRect(&sourceRect, 0, 0, sourceInfo.bmWidth, ABS(sourceInfo.bmHeight));
		
	windowDC = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == windowDC)
		return NULL;

	sourceDC = CreateCompatibleDC(windowDC);
	resultDC = CreateCompatibleDC(windowDC);
	resultBitmap = CreateCompatibleBitmap(windowDC, width, height);

	ReleaseDC(hwnd, windowDC);

	if (NULL != sourceDC &&
		NULL != resultDC &&
		NULL != resultBitmap)
	{
		HBITMAP prevSourceBitmap, prevResultBitmap;
		prevSourceBitmap = SelectBitmap(sourceDC, sourceBitmap);
		prevResultBitmap = SelectBitmap(resultDC, resultBitmap);
	
		Image_FillBorder(resultDC, &resultRect, sourceDC, &sourceRect, TRUE, 255);
		
		SelectBitmap(sourceDC, prevSourceBitmap);
		SelectBitmap(resultDC, prevResultBitmap);

	}
	
	if (NULL != sourceDC)
		DeleteDC(sourceDC);
	if (NULL != resultDC)
		DeleteDC(resultDC);
	
	return resultBitmap;
}

static HBITMAP
ListWidget_GetBorderBitmap(HBITMAP bitmap, const wchar_t *path, WidgetStyle *style, 
						   HWND hwnd, long width, long height, BOOL disableSkin,
						   COLORREF colorBack, COLORREF colorFront)
{
	
	if (NULL != bitmap)
	{
		BITMAP bi;
		if (sizeof(bi) != GetObject(bitmap, sizeof(bi), &bi) ||
			bi.bmWidth != width || 
			bi.bmHeight != height)
		{
			DeleteObject(bitmap);
			bitmap = NULL;
		}
	}

	if (NULL == bitmap && 
		NULL != style)
	{

		HBITMAP baseBitmap;

		if (FALSE == disableSkin)
		{
			baseBitmap = Image_LoadSkinned(path, 
								SRC_TYPE_PNG, 
								IMAGE_FILTER_NORMAL,
								0, 0, 
								colorBack, colorFront, 
								WIDGETSTYLE_BACK_COLOR(style));
		}
		else
		{
			baseBitmap = Image_Load(path, SRC_TYPE_PNG, 0, 0, 0);
		}
		if (NULL != baseBitmap)
		{
			Image_Premultiply(baseBitmap, NULL);
			bitmap = ListWidget_CreateBorderBitmap(baseBitmap, hwnd, width, height);
			DeleteObject(baseBitmap);
		}
	}

	return bitmap;
}
HBITMAP
ListWidget_GetHoverBitmap(ListWidget *self, WidgetStyle *style, HWND hwnd, long width, long height)
{
	if (NULL == self)
		return NULL;
	
	self->hoverBitmap = ListWidget_GetBorderBitmap(self->hoverBitmap, 
								MAKEINTRESOURCE(IDR_ITEM_HOVER_IMAGE),
								style, hwnd, width, height, FALSE,
								WIDGETSTYLE_SELECT_BACK_COLOR(style),
								WIDGETSTYLE_SELECT_FRONT_COLOR(style));

	return self->hoverBitmap;
}

HBITMAP
ListWidget_GetSelectBitmap(ListWidget *self, WidgetStyle *style, HWND hwnd, long width, long height)
{

	if (NULL == self)
		return NULL;
	
	self->selectBitmap = ListWidget_GetBorderBitmap(self->selectBitmap, 
								MAKEINTRESOURCE(IDR_ITEM_SELECT_IMAGE),
								style, hwnd, width, height, FALSE,
								WIDGETSTYLE_SELECT_BACK_COLOR(style),
								WIDGETSTYLE_SELECT_FRONT_COLOR(style));

	return self->selectBitmap;
}

HBITMAP
ListWidget_GetInactiveSelectBitmap(ListWidget *self, WidgetStyle *style, HWND hwnd, long width, long height)
{

	if (NULL == self)
		return NULL;
	
	self->inactiveSelectBitmap = ListWidget_GetBorderBitmap(self->inactiveSelectBitmap, 
								MAKEINTRESOURCE(IDR_ITEM_SELECT_IMAGE),
								style, hwnd, width, height, FALSE,
								WIDGETSTYLE_INACTIVE_SELECT_BACK_COLOR(style),
								WIDGETSTYLE_INACTIVE_SELECT_FRONT_COLOR(style));

	return self->inactiveSelectBitmap;
}

HBITMAP
ListWidget_GetLargeBadgeBitmap(ListWidget *self, WidgetStyle *style, HWND hwnd, long width, long height)
{

	if (NULL == self)
		return NULL;
	
	self->largeBadgeBitmap = ListWidget_GetBorderBitmap(self->largeBadgeBitmap, 
								MAKEINTRESOURCE(IDR_COMMAND_BACKGROUND_IMAGE),
								style, hwnd, width, height, TRUE,
								WIDGETSTYLE_IMAGE_BACK_COLOR(style),
								WIDGETSTYLE_IMAGE_FRONT_COLOR(style));

	return self->largeBadgeBitmap;
}

HBITMAP
ListWidget_GetSmallBadgeBitmap(ListWidget *self, WidgetStyle *style, HWND hwnd, long width, long height)
{

	if (NULL == self)
		return NULL;
	
	self->smallBadgeBitmap = ListWidget_GetBorderBitmap(self->smallBadgeBitmap, 
								MAKEINTRESOURCE(IDR_COMMAND_SECONDARY_BACKGROUND_IMAGE),
								style, hwnd, width, height, TRUE,
								WIDGETSTYLE_IMAGE_BACK_COLOR(style),
								WIDGETSTYLE_IMAGE_FRONT_COLOR(style));

	return self->smallBadgeBitmap;
}

HBITMAP
ListWidget_GetArrowsBitmap(ListWidget *self, WidgetStyle *style, HWND hwnd)
{
	if (NULL == self)
		return NULL;

	if (NULL == self->arrowsBitmap && 
		NULL != style)
	{
		self->arrowsBitmap = Image_LoadSkinned(MAKEINTRESOURCE(IDR_CATEGORY_ARROWS_IMAGE),
										SRC_TYPE_PNG, 
										IMAGE_FILTER_NORMAL,
										0, 0, 
										WIDGETSTYLE_CATEGORY_BACK_COLOR(style),  
										WIDGETSTYLE_CATEGORY_TEXT_COLOR(style),
										WIDGETSTYLE_CATEGORY_BACK_COLOR(style));
		
		if (NULL != self->arrowsBitmap)
			Image_Premultiply(self->arrowsBitmap, NULL);
	}

	return self->arrowsBitmap;

}

HBITMAP
ListWidget_GetUnknownCommandLargeBitmap(ListWidget *self, WidgetStyle *style, long width, long height)
{
	if (NULL == self)
		return NULL;

	if (NULL == self->unknownCommandLargeImage && 
		NULL != style)
	{
		self->unknownCommandLargeImage = DeviceImageCache_GetImage(Plugin_GetImageCache(), 
				MAKEINTRESOURCE(IDR_UNKNOWN_COMMAND_LARGE_IMAGE), 
				width, height, 
				NULL, NULL);
	}

	return DeviceImage_GetBitmap(self->unknownCommandLargeImage, DeviceImage_Normal);
}

HBITMAP
ListWidget_GetUnknownCommandSmallBitmap(ListWidget *self, WidgetStyle *style, long width, long height)
{
	if (NULL == self)
		return NULL;

	if (NULL == self->unknownCommandSmallImage && 
		NULL != style)
	{
		self->unknownCommandSmallImage = DeviceImageCache_GetImage(Plugin_GetImageCache(), 
				MAKEINTRESOURCE(IDR_UNKNOWN_COMMAND_LARGE_IMAGE), 
				width, height, 
				NULL, NULL);
	}

	return DeviceImage_GetBitmap(self->unknownCommandSmallImage, DeviceImage_Normal);
}

HBITMAP
ListWidget_GetActivityProgressBitmap(ListWidget *self, WidgetStyle *style)
{
	if (NULL == self)
		return NULL;

	if (NULL == self->activityProgressImage && 
		NULL != style)
	{
		self->activityProgressImage = DeviceImageCache_GetImage(Plugin_GetImageCache(), 
				MAKEINTRESOURCE(IDR_PROGRESS_SMALL_IMAGE), 
				self->activityMetrics.progressWidth, self->activityMetrics.progressHeight * LISTWIDGET_PROGRESS_FRAME_COUNT, 
				NULL, NULL);
		
	}

	return DeviceImage_GetBitmap(self->activityProgressImage, DeviceImage_Normal);
}

HBITMAP
ListWidget_GetActivityBadgeBitmap(ListWidget *self, WidgetStyle *style, HWND hwnd, long width, long height)
{	
	if (NULL == self)
		return NULL;
	
	self->activityBadgeBitmap = ListWidget_GetBorderBitmap(self->activityBadgeBitmap, 
								MAKEINTRESOURCE(IDR_ACTION_BACKGROUND_IMAGE),
								style, hwnd, width, height, TRUE,
								WIDGETSTYLE_IMAGE_BACK_COLOR(style),
								WIDGETSTYLE_IMAGE_FRONT_COLOR(style));

	return self->activityBadgeBitmap;
}


static BOOL
ListWidget_UpdateCommandsLayout(ListWidget *self, HWND hwnd)
{
	WidgetStyle *style;
	ListWidgetItemMetric metrics;
	RECT rect;
	size_t index, indexMax;
	long spacing;

	if (NULL == self)
		return FALSE;

	style = WIDGET_GET_STYLE(hwnd);
	if (NULL == style)
		return FALSE;
	
	if (0 == self->commandsCount)
		return TRUE;
		
	if (FALSE == ListWidget_GetItemMetrics(style, &metrics))
		return FALSE;
		
	indexMax = self->commandsCount;

	
	if ((NULL == self->hoveredItem || NULL == self->hoveredItem->activity) &&
		FALSE != ListWidget_GetCommandPrimary(self->commands[0]))
	{
		rect.bottom = self->imageSize.cy /*+ metrics.imageOffsetBottom*/;
		rect.bottom += metrics.offsetTop + metrics.imageOffsetTop;
		rect.top = rect.bottom - self->primaryCommandSize.cy;
		
		rect.left = (self->itemWidth - self->primaryCommandSize.cx)/2;
		rect.right = rect.left + self->primaryCommandSize.cx;
		
		ListWidget_SetCommandRect(self->commands[0], &rect);
		indexMax--;
	}
	
	rect.top = metrics.offsetTop + metrics.imageOffsetTop;
	rect.bottom = rect.top + self->secondaryCommandSize.cy;
	rect.right = self->itemWidth - metrics.offsetRight - metrics.imageOffsetRight;
	rect.left = rect.right - self->secondaryCommandSize.cx;
	
	spacing = self->secondaryCommandSize.cx/16 + 1;
	for(index = 0; index < indexMax; index++)
	{
		ListWidget_SetCommandRect(self->commands[self->commandsCount - index - 1], &rect);
		OffsetRect(&rect, -(self->secondaryCommandSize.cx + spacing), 0);
	}

	return TRUE;
}


static BOOL
ListWidget_UpdateActiveCommands(ListWidget *self, HWND hwnd)
{
	ListWidgetCommand **clone;
	size_t cloneSize, index;
	BOOL invalidated;
	POINT origin, pt;
	RECT rect;
		
	if (NULL == self)
		return FALSE;

	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}

	if (NULL == self->hoveredItem)
	{		
		ListWidget_DestroyAllCommands(self->commands, self->commandsCount);
		self->commandsCount = 0;
		return TRUE;
	}
	
	origin.x += self->hoveredItem->rect.left;
	origin.y += self->hoveredItem->rect.top;
	invalidated = FALSE;
	
	if (0 != self->commandsCount)
	{			
		for(index = 0; index < self->commandsCount; index++)
		{
			if (FALSE != ListWidget_GetCommandRect(self->commands[index], &rect))
			{
				OffsetRect(&rect, origin.x, origin.y);
				if (FALSE != InvalidateRect(hwnd, &rect, FALSE))
					invalidated = TRUE;
			}
		}

		clone = (ListWidgetCommand**)malloc(sizeof(ListWidgetCommand*) * self->commandsCount);
		if (NULL != clone)
		{
			cloneSize = self->commandsCount;
			CopyMemory(clone, self->commands, sizeof(ListWidgetCommand*) * cloneSize);
		}
		else
		{
			cloneSize = 0;
			ListWidget_DestroyAllCommands(self->commands, self->commandsCount);
		}
	}
	else
	{
		clone = NULL;
		cloneSize = 0;
	}

	self->commandsCount = ListWidget_GetItemCommands(self->hoveredItem, 
									self->commands, self->commandsMax);
	
	if (0 != self->commandsCount)
	{
		ListWidget_UpdateCommandsLayout(self, hwnd);
		ListWidgetItem_SetInteractive(self->hoveredItem);

		for(index = 0; index < self->commandsCount; index++)
		{
			if (FALSE != ListWidget_GetCommandRect(self->commands[index], &rect))
			{
				OffsetRect(&rect, origin.x, origin.y);
				if (FALSE != InvalidateRect(hwnd, &rect, FALSE))
					invalidated = TRUE;
			}
		}
	}
	else
		ListWidgetItem_UnsetInteractive(self->hoveredItem);

	
	if (NULL != clone)
	{
		ListWidget_DestroyAllCommands(clone, cloneSize);
		free(clone);
	}

	if (FALSE != GetCursorPos(&pt))
	{
		ListWidgetItem *tooltipItem;
		ListWidgetItemPart tooltipItemPart;
		RECT tooltipItemPartRect;

		tooltipItem = ListWidget_TooltipGetCurrent(self->tooltip, &tooltipItemPart, &tooltipItemPartRect);

		MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
		ListWidget_UpdateHoverEx(self, hwnd, &pt);

		if (FALSE != ListWidget_TooltipGetChanged(self->tooltip, tooltipItem, 
											tooltipItemPart, &tooltipItemPartRect))
		{
			ListWidget_TooltipRelayMouseMessage(self->tooltip, WM_MOUSEMOVE, 0, &pt);
		}
	}
	
	if (FALSE != invalidated)
		UpdateWindow(hwnd);

	return TRUE;
}

static void CALLBACK
ListWidget_ShowCommandTimerCb(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD elpasedTime)
{
	ListWidget *self;
	
	KillTimer(hwnd, eventId);

	self = WIDGET_GET_SELF(hwnd, ListWidget);
	if (NULL == self)
		return;

	if (NULL != self->hoveredItem && 
		FALSE == ListWidgetItem_IsInteractive(self->hoveredItem) &&
		NULL == self->activeMenu)
	{
		ListWidget_UpdateActiveCommands(self, hwnd);
	}
}

static void CALLBACK
ListWidget_ProgressTickCb(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD elpasedTime)
{
	ListWidget *self = WIDGET_GET_SELF(hwnd, ListWidget);
	if (NULL == self)
	{
		KillTimer(hwnd, eventId);
		return;
	}

	size_t index = self->activeItems.size();
	if (index > 0)
	{
		POINT origin;
		RECT rect;
		ListWidgetItemMetric metrics, *metrics_ptr;
		WidgetStyle *style;

		if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
		{
			origin.x = 0;
			origin.y = 0;
		}

		style = WIDGET_GET_STYLE(hwnd);
		if (NULL != style && FALSE != ListWidget_GetItemMetrics(style, &metrics))
			metrics_ptr = &metrics;
		else
			metrics_ptr = NULL;

		while(index--)
		{
			ListWidgetItem *item = self->activeItems[index];
			item->activity->step++;
			if (item->activity->step >= LISTWIDGET_PROGRESS_FRAME_COUNT)
				item->activity->step = 1;
			
			if (FALSE != ListWidget_GetItemActivityProgressRect(self, NULL, item, metrics_ptr, &rect))
			{
				OffsetRect(&rect, origin.x, origin.y);
				InvalidateRect(hwnd, &rect, FALSE);
			}
		}

	}
	else
	{
		KillTimer(hwnd, eventId);
		self->activityTimerEnabled = FALSE;
	}
}


static void CALLBACK
ListWidget_BeginTitleEditTimerCb(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD elpasedTime)
{
	ListWidget *self;

	self = WIDGET_GET_SELF(hwnd, ListWidget);

	KillTimer(hwnd, eventId);

	if (NULL != self && 
		NULL != self->titleEditItem)
	{

		if (self->titleEditItem == self->selectedItem)
		{
			if (NULL != self->titleEditor)
			{
				DestroyWindow(self->titleEditor);
				self->titleEditor = NULL;
			}

			self->titleEditor = ListWidget_BeginItemTitleEdit(self, hwnd, self->titleEditItem);
		}
		
		self->titleEditItem = NULL;
	}
}

BOOL
ListWidget_UpdateHoverEx(ListWidget *self, HWND hwnd, const POINT *cursor)
{
	ListWidgetItem *hoveredItem;
	ListWidgetItemMetric metrics, *metrics_ptr;
	WidgetStyle *style;
	ListWidgetItemPart hoveredPart = ListWidgetItemPart_None;
	RECT rect, hoveredPartRect;
	POINT pt, origin;

	if (NULL == cursor)
		return FALSE;

	metrics_ptr = NULL;

	pt = *cursor;

	if (NULL != self->pressedCategory ||
		NULL != self->activeMenu || 
		0 != (0x8000 & GetAsyncKeyState(VK_LBUTTON)) ||
		0 != (0x8000 & GetAsyncKeyState(VK_RBUTTON)))
	{
		return FALSE;
	}
	
	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}

	if (FALSE != GetClientRect(hwnd, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		
		pt.x -= origin.x;
		pt.y -= origin.y;
		hoveredItem = ListWidget_GetItemFromPoint(self, pt);
		if (NULL != hoveredItem)
		{
			if (NULL == metrics_ptr)
			{
				style = WIDGET_GET_STYLE(hwnd);
			
				if (NULL != style && 
					FALSE != ListWidget_GetItemMetrics(style, &metrics))
				{
					metrics_ptr = &metrics;
				}
			}

			hoveredPart =	ListWidgetItemPart_Frame | 
							ListWidgetItemPart_Command | 
							ListWidgetItemPart_Spacebar | 
							ListWidgetItemPart_Title;

			hoveredPart = ListWidget_GetItemPartFromPoint(self, hoveredItem, metrics_ptr, pt, hoveredPart, &hoveredPartRect);
		}
	}
	else
		hoveredItem = NULL;
	
	if (NULL == hoveredItem)
		hoveredPart= ListWidgetItemPart_None;

	ListWidget_TooltipUpdate(self->tooltip, hoveredItem, hoveredPart, &hoveredPartRect);
	
	if (ListWidgetItemPart_None == ((ListWidgetItemPart_Frame | ListWidgetItemPart_Command | ListWidgetItemPart_Activity) & hoveredPart))
		hoveredItem = NULL;
	
	if (self->hoveredItem == hoveredItem)
		return FALSE;
	
	if (NULL == metrics_ptr)
	{
		style = WIDGET_GET_STYLE(hwnd);
	
		if (NULL != style && 
			FALSE != ListWidget_GetItemMetrics(style, &metrics))
		{
			metrics_ptr = &metrics;
		}
	}
	
	if (NULL != self->hoveredItem)
	{
		ListWidgetItem_UnsetHovered(self->hoveredItem);
		ListWidgetItem_UnsetInteractive(self->hoveredItem);
		
		if (NULL == metrics_ptr || 
			FALSE == ListWidget_GetItemFrameRect(self, self->hoveredItem, metrics_ptr, &rect))
		{
			CopyRect(&rect, &self->hoveredItem->rect);
		}
		
		OffsetRect(&rect, origin.x, origin.y);
		InvalidateRect(hwnd, &rect, FALSE);
	}
	
	if (NULL != hoveredItem)
	{
		ListWidgetItem_SetHovered(hoveredItem);

		if (NULL == metrics_ptr || 
			FALSE == ListWidget_GetItemFrameRect(self, hoveredItem, metrics_ptr, &rect))
		{
			CopyRect(&rect, &hoveredItem->rect);
		}

		OffsetRect(&rect, origin.x, origin.y);
		InvalidateRect(hwnd, &rect, FALSE);

		SetTimer(hwnd, LISTWIDGETTIMER_SHOW_COMMANDS_ID, 
					LISTWIDGETTIMER_SHOW_COMMANDS_DELAY, 
					ListWidget_ShowCommandTimerCb);
	}
	else
		KillTimer(hwnd, LISTWIDGETTIMER_SHOW_COMMANDS_ID);

	self->hoveredItem = hoveredItem;
		
	return TRUE;
}

BOOL
ListWidget_UpdateHover(ListWidget *self, HWND hwnd)
{
	POINT pt;
	if (FALSE == GetCursorPos(&pt))
		return FALSE;
	
	MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
	return ListWidget_UpdateHoverEx(self, hwnd, &pt);
}

BOOL
ListWidget_RemoveHover(ListWidget *self, HWND hwnd, BOOL invalidate)
{
	if (NULL == self)
		return FALSE;

	if (NULL == self->hoveredItem)
		return FALSE;

	ListWidgetItem_UnsetHovered(self->hoveredItem);
	ListWidgetItem_UnsetInteractive(self->hoveredItem);
	KillTimer(hwnd, LISTWIDGETTIMER_SHOW_COMMANDS_ID);

	if (FALSE != invalidate)
	{
		RECT rect;
		POINT origin;
		ListWidgetItemMetric metrics;
		WidgetStyle *style;
		
		style = WIDGET_GET_STYLE(hwnd);
		if (NULL == style ||
			FALSE == ListWidget_GetItemMetrics(style, &metrics) ||
			FALSE == ListWidget_GetItemFrameRect(self, self->hoveredItem, &metrics, &rect))
		{
			CopyRect(&rect, &self->hoveredItem->rect);
		}

		if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
			OffsetRect(&rect, origin.x, origin.y);

		InvalidateRect(hwnd, &rect, FALSE);
	}

	self->hoveredItem = NULL;

	return TRUE;
}


void
ListWidget_UpdateSelectionStatus(ListWidget *self, HWND hwnd, BOOL ensureVisible)
{
	HWND statusWindow;

	if (NULL == self)
		return;

	statusWindow = GetParent(hwnd);
	if (NULL == statusWindow)
		return;

	statusWindow = MANAGERVIEW_GET_STATUS_BAR(statusWindow);
	if (NULL == statusWindow)
		return;
		
	if (NULL != self->selectedItem)
	{
		wchar_t buffer[2048] = {0};
		const wchar_t *statusString;

		if (FALSE == ListWidget_FormatItemStatus(self, self->selectedItem, buffer, ARRAYSIZE(buffer)) ||
			L'\0' == *buffer)
		{
			statusString = NULL;
		}
		else
			statusString = buffer;
		
		if (STATUS_ERROR == self->selectionStatus)
		{
			self->selectionStatus = STATUSBAR_ADD_STATUS(statusWindow, statusString);
			if (STATUS_ERROR != self->selectionStatus)
			{
				if (FALSE == ListWidget_FormatItemSpaceStatus(self, self->selectedItem, buffer, ARRAYSIZE(buffer)) ||
					L'\0' == *buffer)
				{
					statusString = NULL;
				}
				else
					statusString = buffer;

				STATUSBAR_SET_STATUS_RTEXT(statusWindow, self->selectionStatus, statusString); 
			}
		}
		else
		{
			STATUSBAR_SET_STATUS_TEXT(statusWindow, self->selectionStatus, statusString);

			if (FALSE == ListWidget_FormatItemSpaceStatus(self, self->selectedItem, buffer, ARRAYSIZE(buffer)) ||
					L'\0' == *buffer)
			{
				statusString = NULL;
			}
			else
				statusString = buffer;

			STATUSBAR_SET_STATUS_RTEXT(statusWindow, self->selectionStatus, statusString); 

			if (FALSE != ensureVisible)
			{
				STATUSBAR_MOVE_STATUS(statusWindow, self->selectionStatus, STATUS_MOVE_TOP);
			}
		}
	}
	else
	{
		if (STATUS_ERROR != self->selectionStatus)
		{
			STATUSBAR_REMOVE_STATUS(statusWindow, self->selectionStatus);
			self->selectionStatus = STATUS_ERROR;
		}
	}
}

void
ListWidget_UpdateSelectionSpaceStatus(ListWidget *self, HWND hwnd, BOOL ensureVisible)
{
	HWND statusWindow;

	if (NULL == self)
		return;

	statusWindow = GetParent(hwnd);
	if (NULL == statusWindow)
		return;

	statusWindow = MANAGERVIEW_GET_STATUS_BAR(statusWindow);
	if (NULL == statusWindow)
		return;
		
	if (NULL != self->selectedItem &&
		STATUS_ERROR != self->selectionStatus)
	{
		wchar_t buffer[2048] = {0};
		const wchar_t *statusString;

		if (FALSE == ListWidget_FormatItemSpaceStatus(self, self->selectedItem, buffer, ARRAYSIZE(buffer)) ||
				L'\0' == *buffer)
		{
			statusString = NULL;
		}
		else
			statusString = buffer;

		STATUSBAR_SET_STATUS_RTEXT(statusWindow, self->selectionStatus, statusString); 

		if (FALSE != ensureVisible)
			STATUSBAR_MOVE_STATUS(statusWindow, self->selectionStatus, STATUS_MOVE_TOP);
	}
}

BOOL
ListWidget_SelectItem(ListWidget *self, HWND hwnd, ListWidgetItem *item, BOOL ensureVisible)
{
	BOOL invalidated;
	
	if (NULL == self)
		return FALSE;

	invalidated = FALSE;

	if (self->selectedItem != item)
	{
		RECT rect;
		POINT origin;
		ListWidgetItemMetric metrics, *metrics_ptr;
		WidgetStyle *style;

		if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
		{
			origin.x = 0;
			origin.y = 0;
		}

		style = WIDGET_GET_STYLE(hwnd);
		if (NULL != style && 
			FALSE != ListWidget_GetItemMetrics(style, &metrics))
		{
			metrics_ptr = &metrics;
		}
		else
			metrics_ptr = NULL;

		if (NULL != self->selectedItem)
		{
			ListWidgetItem_UnsetSelected(self->selectedItem);

			if (NULL == metrics_ptr ||
				FALSE == ListWidget_GetItemFrameRect(self, self->selectedItem, metrics_ptr, &rect))
			{
				CopyRect(&rect, &self->selectedItem->rect);
			}

			OffsetRect(&rect, origin.x, origin.y);
			if (FALSE != InvalidateRect(hwnd, &rect, FALSE))
				invalidated = TRUE;
		}
		
		if (NULL != item)
		{
			ListWidgetItem_SetSelected(item);
			
			if (NULL == metrics_ptr ||
				FALSE == ListWidget_GetItemFrameRect(self, item, metrics_ptr, &rect))
			{
				CopyRect(&rect, &item->rect);
			}

			OffsetRect(&rect, origin.x, origin.y);
			if (FALSE != InvalidateRect(hwnd, &rect, FALSE))
				invalidated = TRUE;
		}
		self->selectedItem = item;
	}

	if (FALSE != ensureVisible)
	{
		if (FALSE != ListWidget_EnsureItemVisisble(self, hwnd, item, VISIBLE_NORMAL))
			invalidated = TRUE;
	}

	if (FALSE != invalidated)
		UpdateWindow(hwnd);

	ListWidget_UpdateSelectionStatus(self, hwnd, TRUE);

	
	return TRUE;
}

static void 
ListWidget_EnsureFocused(ListWidget *self, HWND hwnd, BOOL forceSelect)
{
	HWND hDialog, hParent;

	if (NULL == self || NULL == hwnd)
		return;

	if (GetFocus() == hwnd)
		return;
	
	hDialog = NULL;
	hParent = hwnd;

	while(NULL != (hParent = GetAncestor(hParent, GA_PARENT)))
	{
		if (32770 != GetClassLongPtr(hParent, GCW_ATOM) ||
			0 == (WS_EX_CONTROLPARENT & GetWindowStyleEx(hParent)))
		{
			break;
		}
		
		hDialog = hParent;
	}

	self->flags |= ListWidgetFlag_NoFocusSelect;

	if (NULL != hDialog)
		SendMessage(hDialog, WM_NEXTDLGCTL, (WPARAM)hwnd, TRUE);
	else 
		SetFocus(hwnd);

	self->flags &= ~ListWidgetFlag_NoFocusSelect;
}

static BOOL
ListWidget_EnsureTopVisible(ListWidget *self, HWND hwnd)
{
	POINT pt;
	
	if (NULL == self || NULL == hwnd)
		return FALSE;

	
	if (FALSE == ListWidget_GetViewOrigin(hwnd, &pt) || 
		(0 == pt.x && 0 == pt.y))
	{
		return FALSE;
	}

	if (FALSE == WIDGET_SCROLL(hwnd, pt.x, pt.y, TRUE))
		return FALSE;
	
	ListWidget_UpdateHover(self, hwnd);
	
	return TRUE;
}

static BOOL
ListWidget_EnsureBottomVisible(ListWidget *self, HWND hwnd)
{
	SCROLLINFO scrollInfo;
	POINT pt;

	if (NULL == self || NULL == hwnd)
		return FALSE;

	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		
	if (FALSE != GetScrollInfo(hwnd, SB_HORZ, &scrollInfo))
		pt.x = -scrollInfo.nPos;
	else
		pt.x = 0;

	if (FALSE != GetScrollInfo(hwnd, SB_VERT, &scrollInfo) && 0 != scrollInfo.nPage)
		pt.y = (scrollInfo.nMax + 1 - scrollInfo.nPage) - scrollInfo.nPos;
	else
		pt.y = 0;

	if (0 == pt.x && 0 == pt.y)
		return FALSE;
	
	if (FALSE == WIDGET_SCROLL(hwnd, pt.x, pt.y, TRUE))
		return FALSE;
	
	ListWidget_UpdateHover(self, hwnd);
	
	return TRUE;
}

BOOL
ListWidget_UpdateLayout(HWND hwnd, ListWidgetLayoutFlags layoutFlags)
{
	BOOL result;
	unsigned int flags;
	ListWidget *self;
	POINT anchorPoint;
	ListWidgetItem *anchorItem;
	SCROLLINFO scrollInfo;
	HWND managerWindow, focusWindow;
	POINT pt, menuAnchor, hoverAnchor;

	self = WIDGET_GET_SELF(hwnd, ListWidget);
	if (NULL == self)
		return FALSE;

	if (NULL == self->activeMenu ||
		NULL == self->selectedItem ||
		FALSE == ListWidget_GetViewItemPos(hwnd, self->selectedItem, &menuAnchor))
	{
		menuAnchor.x = 0x7FFFFFFF;
		menuAnchor.y = 0x7FFFFFFF;
	}

	if (NULL != self->hoveredItem ||
		FALSE == ListWidget_GetViewItemPos(hwnd, self->hoveredItem, &hoverAnchor))
	{
		hoverAnchor.x = 0x7FFFFFFF;
		hoverAnchor.y = 0x7FFFFFFF;
	}

	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_POS;

	anchorItem = NULL;

	managerWindow = GetAncestor(hwnd, GA_PARENT);
	if (NULL == managerWindow)
		managerWindow = hwnd;

	focusWindow = GetFocus();

	if (0 != (ListWidgetLayout_KeepStable & layoutFlags) &&
		(managerWindow == focusWindow || IsChild(managerWindow, focusWindow)))
	{
		if (NULL != self->selectedItem)
		{
			RECT clientRect;
	
			GetClientRect(hwnd, &clientRect);

			anchorPoint.x = (FALSE != GetScrollInfo(hwnd, SB_HORZ, &scrollInfo)) ?
							-scrollInfo.nPos : 0;
			anchorPoint.y = (FALSE != GetScrollInfo(hwnd, SB_VERT, &scrollInfo)) ?
							-scrollInfo.nPos : 0;
			OffsetRect(&clientRect, -anchorPoint.x, -anchorPoint.y);

			if (clientRect.left <= self->selectedItem->rect.left &&
				clientRect.top <= self->selectedItem->rect.top &&
				clientRect.right >= self->selectedItem->rect.right &&
				clientRect.bottom >= self->selectedItem->rect.bottom)
			{

				anchorItem = self->selectedItem;
				anchorPoint.x += self->selectedItem->rect.left;
				anchorPoint.y += self->selectedItem->rect.top;
			}
		}
	}

	flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | 
			SWP_FRAMECHANGED | SWP_NOREDRAW;
	
	result = SetWindowPos(hwnd, NULL, 0, 0, 0, 0, flags);

	if (NULL != anchorItem)
	{
		long positionY;
	
		positionY = anchorItem->rect.top;

		if (FALSE != GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
			positionY -= scrollInfo.nPos;
		
		if (positionY != anchorPoint.y)
			WIDGET_SET_SCROLL_POS(hwnd, 0, positionY - anchorPoint.y, FALSE);
	}

	if (0x7FFFFFFF != menuAnchor.x &&
		NULL != self->selectedItem &&
		FALSE != ListWidget_GetViewItemPos(hwnd, self->selectedItem, &pt) &&
		menuAnchor.x != pt.x && menuAnchor.y != pt.y)
	{
		if (NULL != self->activeMenu)
			EndMenu();
	}

	if (0x7FFFFFFF != hoverAnchor.x &&
		NULL != self->hoveredItem &&
		FALSE != ListWidget_GetViewItemPos(hwnd, self->hoveredItem, &pt) &&
		hoverAnchor.x != pt.x && hoverAnchor.y != pt.y)
	{
		ListWidget_UpdateHover(self, hwnd);
	}

	if (0 == (ListWidgetLayout_NoRedraw & layoutFlags))
	{
		flags = RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME | RDW_ERASE;
		
		if (0 != (ListWidgetLayout_UpdateNow & layoutFlags))
			flags |= (RDW_ERASENOW | RDW_UPDATENOW);

		RedrawWindow(hwnd, NULL, NULL, flags);
	}

	return result;
}


double 
ListWidget_GetZoomRatio(ListWidget *self)
{
	if (NULL == self)
		return 1.0;
	
	if (self->imageSize.cy > LISTWIDGET_IMAGE_DEFAULT_HEIGHT)
	{
		return (1.0 + (double)(self->imageSize.cy - LISTWIDGET_IMAGE_DEFAULT_HEIGHT)/
						(LISTWIDGET_IMAGE_MAX_HEIGHT - LISTWIDGET_IMAGE_DEFAULT_HEIGHT));
	}
	
	return (double)(self->imageSize.cy - LISTWIDGET_IMAGE_MIN_HEIGHT)/
			(LISTWIDGET_IMAGE_DEFAULT_HEIGHT - LISTWIDGET_IMAGE_MIN_HEIGHT);
	
}


long
ListWidget_GetZoomedValue(double zoomRatio, long normal, long min, long max)
{
	if (zoomRatio < 1.0)
		return min + (long)floor((double)(normal - min) * zoomRatio);
	else if (zoomRatio > 1.0)
		return normal + (long)ceil((double)(max - normal) * (zoomRatio - 1.0));

	return normal;
}

static BOOL 
ListWidget_UpdateActivityLayout(ListWidget *self, HWND hwnd, double zoomRatio)
{
	LOGFONTW lf;
	HDC windowDC;
	long elementHeight, titleMinWidth, workHeight;
	ListWidgetItem *item;
	ListWidgetActivityMetric *activityMetrics;

	// always reset items activity size cache
	size_t index = (self ? self->activeItems.size() : NULL);
	while(index--)
	{
		item = self->activeItems[index];
		if (NULL != item)
			SetSizeEmpty(&item->activity->titleSize);
	}

	if (NULL == self)
		return FALSE;


	activityMetrics = &self->activityMetrics;

	activityMetrics->offsetLeft = self->activityMetrics.height/16;
	if (activityMetrics->offsetLeft < 1)
		activityMetrics->offsetLeft = 1;
	activityMetrics->offsetRight = activityMetrics->offsetLeft;

	activityMetrics->offsetTop = self->activityMetrics.height/9;
	if (activityMetrics->offsetTop < 3)
		activityMetrics->offsetTop = 3;
	activityMetrics->offsetBottom = activityMetrics->offsetTop;

	activityMetrics->spacing = 4 + self->activityMetrics.height/16;

	workHeight = activityMetrics->height - (activityMetrics->offsetTop + activityMetrics->offsetBottom);

	activityMetrics->fontHeight = workHeight/2;
	if (activityMetrics->fontHeight < 8)
		activityMetrics->fontHeight = 8;

	if (NULL == self->activityFont ||
		sizeof(lf) != GetObject(self->activityFont, sizeof(lf), &lf) ||
		lf.lfHeight != activityMetrics->fontHeight)
	{
		if (NULL != self->activityFont)
			DeleteObject(self->activityFont);

		lf.lfHeight = activityMetrics->fontHeight;
		lf.lfWidth = 0;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		lf.lfWeight = FW_NORMAL;
		lf.lfItalic = FALSE;
		lf.lfUnderline = FALSE;
		lf.lfStrikeOut = FALSE;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_TT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = Graphics_GetSysFontQuality();
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"Arial"/*L"Times New Roman"*/);

		self->activityFont = CreateFontIndirect(&lf); 
	}

	activityMetrics->titleHeight = workHeight;
	if (activityMetrics->titleHeight < 0)
		activityMetrics->titleHeight = 0;

	activityMetrics->titleWidth = 0;
	activityMetrics->percentWidth = 0;
	activityMetrics->percentHeight = 0;
	titleMinWidth = 0;

	windowDC = GetWindowDC(hwnd);
	if (NULL != windowDC)
	{
		SIZE textSize;
		HFONT prevFont;

		prevFont = SelectFont(windowDC, self->activityFont);

		if (FALSE == GetTextExtentPoint32(windowDC, L"00%", 3, &textSize))
			SetSizeEmpty(&textSize);

		if (textSize.cy <= workHeight)
		{
			activityMetrics->fontHeight = textSize.cy;
			activityMetrics->percentHeight = textSize.cy;
			activityMetrics->percentWidth = textSize.cx;
		}

		titleMinWidth = Graphics_GetAveStrWidth(windowDC, 6);
		activityMetrics->spacing = Graphics_GetAveStrWidth(windowDC, 1) - 2;
	}
	
	if (NULL != self->activityBadgeBitmap)
	{
		DeleteObject(self->activityBadgeBitmap);
		self->activityBadgeBitmap = NULL;
	}

	elementHeight = (long)(18.0 * (double)workHeight/28.0);

	if (activityMetrics->progressWidth != elementHeight ||
		activityMetrics->progressHeight != elementHeight)
	{
		activityMetrics->progressWidth = elementHeight;
		activityMetrics->progressHeight = elementHeight;

		if (NULL != self->activityProgressImage)
		{
			DeviceImage_Release(self->activityProgressImage);
			self->activityProgressImage = NULL;
		}
	}

	for(;;)
	{
		activityMetrics->titleWidth = activityMetrics->width - 
									(activityMetrics->offsetLeft + activityMetrics->offsetRight);

		if(0 != activityMetrics->progressWidth)
			activityMetrics->titleWidth -= (activityMetrics->progressWidth + activityMetrics->spacing);

		if(0 != activityMetrics->percentWidth)
			activityMetrics->titleWidth -= (activityMetrics->percentWidth + activityMetrics->spacing);

		if (activityMetrics->titleWidth < titleMinWidth)
		{
			if (0 != activityMetrics->percentWidth)
			{
				activityMetrics->percentWidth = 0;
				activityMetrics->percentHeight = 0;
				continue;
			}

			if (0 != activityMetrics->progressWidth)
			{
				activityMetrics->progressWidth = 0;
				activityMetrics->progressHeight = 0;
				continue;
			}

			activityMetrics->titleWidth = 0;
			activityMetrics->titleHeight = 0;
		}

		break;
	}

	return TRUE;
}

BOOL
ListWidget_SetImageSize(ListWidget *self, HWND hwnd, int imageWidth, int imageHeight, BOOL redraw)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	long elementWidth, elementHeight;
	double zoomRatio;

	if (NULL == self)
		return FALSE;

	SetSize(&self->imageSize, imageWidth, imageHeight);

	zoomRatio = ListWidget_GetZoomRatio(self);

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
		for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
		{
			group = category->groups[iGroup];
			for(iItem = 0; iItem < group->items.size(); iItem++)
			{
				item = group->items[iItem];
				if (NULL != item->image)
				{
					DeviceImage_Release(item->image);
					item->image = NULL;
				}
				SetSize(&item->titleSize, -1, -1);
			}
		}
	}

	// connetion size
	elementHeight = ListWidget_GetZoomedValue(zoomRatio, LISTWIDGET_CONNECTION_DEFAULT_HEIGHT, 
						LISTWIDGET_CONNECTION_MIN_HEIGHT, LISTWIDGET_CONNECTION_MAX_HEIGHT);

	elementWidth = elementHeight;
	if (self->connectionSize.cx != elementWidth ||
		self->connectionSize.cy != elementHeight)
	{
		SetSize(&self->connectionSize, elementWidth, elementHeight);
		int index = (int)self->connections.size();
		while (index--)
		{
			ListWidget_UpdateConnectionImageSize(self->connections[index], 
				self->connectionSize.cx, self->connectionSize.cy);
		}
	}


	// primary command
	elementHeight = ListWidget_GetZoomedValue(zoomRatio, LISTWIDGET_PRIMARYCOMMAND_DEFAULT_HEIGHT, 
						LISTWIDGET_PRIMARYCOMMAND_MIN_HEIGHT, LISTWIDGET_PRIMARYCOMMAND_MAX_HEIGHT);
	elementWidth =  self->imageSize.cx;

	if (self->primaryCommandSize.cx != elementWidth ||
		self->primaryCommandSize.cy != elementHeight)
	{
		SetSize(&self->primaryCommandSize, elementWidth, elementHeight);
	}

	// secondary command
	elementHeight = ListWidget_GetZoomedValue(zoomRatio, LISTWIDGET_SECONDARYCOMMAND_DEFAULT_HEIGHT, 
						LISTWIDGET_SECONDARYCOMMAND_MIN_HEIGHT, LISTWIDGET_SECONDARYCOMMAND_MAX_HEIGHT);
	elementWidth = elementHeight;
	if (self->secondaryCommandSize.cx != elementWidth ||
		self->secondaryCommandSize.cy != elementHeight)
	{
		SetSize(&self->secondaryCommandSize, elementWidth, elementHeight);
	}

	// activity
	elementHeight = ListWidget_GetZoomedValue(zoomRatio, LISTWIDGET_ACTIVITY_DEFAULT_HEIGHT, 
						LISTWIDGET_ACTIVITY_MIN_HEIGHT, LISTWIDGET_ACTIVITY_MAX_HEIGHT);
	elementWidth = self->imageSize.cx;

	if (self->activityMetrics.width != elementWidth ||
		self->activityMetrics.height != elementHeight)
	{
		self->activityMetrics.width = elementWidth;
		self->activityMetrics.height = elementHeight;
	}
	
	ListWidget_UpdateLayout(hwnd, ListWidgetLayout_NoRedraw);
	
	if (NULL != self->hoveredItem && 
		FALSE != ListWidgetItem_IsInteractive(self->hoveredItem))
	{
		ListWidget_UpdateActiveCommands(self, hwnd);
	}

	ListWidget_UpdateActivityLayout(self, hwnd, zoomRatio);

	if (FALSE != redraw)	
	{
		RedrawWindow(hwnd, NULL, NULL, 
			RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	
	return TRUE;
}

BOOL
ListWidget_DisplayContextMenu(ListWidget *self, HWND hostWindow, POINT pt)
{
	return FALSE;
}

BOOL
ListWidget_RegisterActiveItem(ListWidget *self, HWND hwnd, ListWidgetItem *item)
{
	size_t index;
	if (NULL == self || NULL == item)
		return FALSE;

	index = self->activeItems.size();
	while(index--)
	{
		if (item == self->activeItems[index])
			return FALSE;
	}

	self->activeItems.push_back(item);
	if (1 == self->activeItems.size())
	{
		if (0 != SetTimer(hwnd, LISTWIDGETTIMER_PROGRESS_TICK_ID, 
					LISTWIDGETTIMER_PROGRESS_TICK_DELAY, ListWidget_ProgressTickCb))
		{
			self->activityTimerEnabled = TRUE;
		}
	}

	return TRUE;
}

BOOL
ListWidget_UnregisterActiveItem(ListWidget *self, HWND hwnd, ListWidgetItem *item)
{
	size_t index;

	if (NULL == self || NULL == item)
		return FALSE;

	index = self->activeItems.size();
	while(index--)
	{
		if (item == self->activeItems[index])
		{
			self->activeItems.erase(self->activeItems.begin() + index);
			if (0 == self->activeItems.size())
			{
				KillTimer(hwnd, LISTWIDGETTIMER_PROGRESS_TICK_ID);
				self->activityTimerEnabled = FALSE;
			}
			return TRUE;
		}
	}

	return FALSE;
}

static void 
ListWidget_CallItemAction(ListWidget *self, HWND hwnd, ListWidgetCategory *category, ListWidgetItem *item)
{
	ifc_device *device;

	if (NULL == self || NULL == item)
		return;

	if (NULL == category)
	{
		if (FALSE == ListWidget_GetItemOwner(self, item, &category) ||
			NULL == category)
		{
			return;
		}
	}

	if (NULL == WASABI_API_DEVICES ||
		S_OK != WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		return;
	}

	if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, category->name, -1, "discovered", -1))
	{
		if (FALSE == device->GetAttached())
			device->Attach(NULL);
	}

	if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, category->name, -1, "attached", -1))
	{
		Navigation_SelectDevice(device->GetName());
	}

	device->Release();
}
static BOOL
ListWidget_AddDevicesToCategory(ListWidget *self, ListWidgetCategory *category, DeviceList *list)
{
	ListWidgetGroup *group;
	ListWidgetItem *item;
	ifc_device *device;
	size_t index, groupCount, categoryCount;

	if (NULL == list || NULL == category)
		return FALSE;

	categoryCount = category->groups.size();

	index = list->size();
	if (0 == index)
		return FALSE;

	do
	{
		index--;
		device = list->at(index);
		const char *groupName = device->GetType();
		group = ListWidget_FindGroupEx(category, groupName, categoryCount);
		if (NULL == group)
		{
			group = ListWidget_CreateGroup(groupName);
			if (NULL != group)
				ListWidget_AddGroup(category, group);
		}
		
		if (NULL != group)
		{
			groupCount = group->items.size();
			item = ListWidget_FindGroupItemEx(group, device->GetName(), groupCount);
			if (NULL == item)
			{
				item = ListWidget_CreateItemFromDevice(self, device);
				if (NULL != item)
				{
					ListWidget_AddItem(group, item);
					if (NULL != item->activity)
						self->activeItems.push_back(item);
				}
			}
		}
		else
			groupCount = 0;

		list->erase(list->begin() + index);
		device->Release();

		while(index--)
		{
			device = list->at(index);
			if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, groupName, -1, device->GetType(), -1))
			{
				if (NULL != group)
				{
					item = ListWidget_FindGroupItemEx(group, device->GetName(), groupCount);
					if (NULL == item)
					{
						item = ListWidget_CreateItemFromDevice(self, device);
						if (NULL != item)
						{
							ListWidget_AddItem(group, item);
							if (NULL != item->activity)
							{
								self->activeItems.push_back(item);
							}
						}
					}
				}
				list->erase(list->begin() + index);
				device->Release();
			}
		}

		if (NULL != group)
			ListWidget_SortGroup(group);

		index = list->size();

	} while(0 != index);

	ListWidget_SortCategory(category);

	return TRUE;
}

static void
ListWidget_AddExistingDevices(ListWidget *self, HWND hwnd)
{
	DeviceList discoveredList, attachedList;
	ifc_device *device;
	ListWidgetCategory *category;
	size_t index, activeItemsCount;
	BOOL devicesAdded;

	if (NULL == self)
		return;

	activeItemsCount = self->activeItems.size();

	if (NULL != WASABI_API_DEVICES)
	{
		ifc_deviceobjectenum *enumerator;
		if (SUCCEEDED(WASABI_API_DEVICES->DeviceEnumerate(&enumerator)))
		{
			size_t reserveSize;
			ifc_deviceobject *object;

			if (SUCCEEDED(enumerator->GetCount(&reserveSize)))
			{
				discoveredList.reserve(reserveSize);
				attachedList.reserve(reserveSize);
			}

			while(S_OK == enumerator->Next(&object, 1, NULL))
			{
				if (SUCCEEDED(object->QueryInterface(IFC_Device, (void**)&device)))
				{
					if (FALSE == device->GetHidden() &&
						// excludes 'cloud' devices from appearing
						lstrcmpiA(device->GetConnection(), "cloud"))
					{
						if (FALSE == device->GetAttached())
							discoveredList.push_back(device);
						else
							attachedList.push_back(device);
					}
					else
						device->Release();
				}
				object->Release();
			}
			enumerator->Release();
		}
	}

	devicesAdded = FALSE;
	if (0 != attachedList.size())
	{
		category = ListWidget_FindCategory(self, "attached");
		if (NULL != category)
		{
			if (FALSE != ListWidget_AddDevicesToCategory(self, category, &attachedList))
			{
				ListWidget_ResetCategoryCounter(category);
				devicesAdded = TRUE;
			}
		}
		index = attachedList.size();
		while(index--)
			attachedList[index]->Release();
	}

	if (0 != discoveredList.size())
	{
		category = ListWidget_FindCategory(self, "discovered");
		if (NULL != category)
		{
			if (FALSE != ListWidget_AddDevicesToCategory(self, category, &discoveredList))
			{
				ListWidget_ResetCategoryCounter(category);
				devicesAdded = TRUE;
			}
		}
		index = discoveredList.size();
		while(index--)
			discoveredList[index]->Release();
	}

	if (self->activeItems.size() > 0)
	{
		if (0 == activeItemsCount)
		{
			if (0 != SetTimer(hwnd, LISTWIDGETTIMER_PROGRESS_TICK_ID, 
						LISTWIDGETTIMER_PROGRESS_TICK_DELAY, ListWidget_ProgressTickCb))
			{
				self->activityTimerEnabled = TRUE;
			}
		}
	}
	else
	{
		if (0 != activeItemsCount)
		{
			KillTimer(hwnd, LISTWIDGETTIMER_PROGRESS_TICK_ID);
			self->activityTimerEnabled = FALSE;
		}
	}

	if (FALSE != devicesAdded)
	{
		ListWidget_UpdateLayout(hwnd, ListWidgetLayout_Normal);
	}
}


void 
ListWidget_UpdateTitleEditorColors(HWND editor, WidgetStyle *style)
{
	if (NULL == editor || NULL == style)
		return;

	EMBEDDEDEDITOR_SET_TEXT_COLOR(editor, WIDGETSTYLE_TEXT_COLOR(style));
	EMBEDDEDEDITOR_SET_BACK_COLOR(editor, WIDGETSTYLE_BACK_COLOR(style));
	EMBEDDEDEDITOR_SET_BORDER_COLOR(editor, WIDGETSTYLE_TEXT_EDITOR_BORDER_COLOR(style));

	InvalidateRect(editor, NULL, TRUE);
}

static BOOL 
ListWidget_DeviceAdd(HWND hwnd, ifc_device *device, DeviceImage *deviceImage)
{
	ListWidget *self;
	ListWidgetCategory *category;
	ListWidgetGroup *group;
	ListWidgetItem *item;

	self = WIDGET_GET_SELF(hwnd, ListWidget);
	if (NULL == self)
		return FALSE;

	if (NULL == self || 
		NULL == device || 
		FALSE != device->GetHidden() &&
		// excludes 'cloud' devices from appearing
		lstrcmpiA(device->GetConnection(), "cloud"))
	{
		return FALSE;
	}

	category = ListWidget_FindCategory(self, (FALSE != device->GetAttached()) ? "attached" : "discovered");
	if (NULL == category)
		return FALSE;

	group = ListWidget_FindGroup(category, device->GetType());
	if (NULL == group)
	{
		group = ListWidget_CreateGroup(device->GetType());
		if (NULL == group)
			return FALSE;

		ListWidget_AddGroup(category, group);
		ListWidget_SortCategory(category);
	}

	item = ListWidget_FindGroupItem(group, device->GetName());
	if (NULL != item)
		return FALSE;

	item = ListWidget_CreateItemFromDevice(self, device);
	if (NULL == item)
		return NULL;

	if (NULL == item->image && NULL != deviceImage)
	{
		item->image = deviceImage;
		DeviceImage_AddRef(item->image);
	}

	ListWidget_AddItem(group, item);
	ListWidget_SortGroup(group);

	if (NULL != item->activity)
		ListWidget_RegisterActiveItem(self, hwnd, item);

	ListWidget_ResetCategoryCounter(category);

	if (FALSE == category->collapsed)
	{
		ListWidget_UpdateLayout(hwnd, 
					ListWidgetLayout_UpdateNow | ListWidgetLayout_KeepStable);
	}
	else
	{
		RECT rect;
		POINT origin;

		CopyRect(&rect, &category->rect);
		if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
			OffsetRect(&rect, origin.x, origin.y);
		
		InvalidateRect(hwnd, &rect, FALSE);
	}
	return TRUE;
}

static BOOL 
ListWidget_DeviceRemove(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	ListWidget_RemoveItem(self, hwnd, device->GetName());
	ListWidget_UpdateHover(self, hwnd);

	return TRUE;
}


static BOOL
ListWidget_DeviceAttach(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetCategory *category;
	ListWidgetItem *item;
	DeviceImage *image;
	BOOL result;

	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	image = NULL;

	category = ListWidget_FindCategory(self, "discovered");
	if (NULL != category)
	{
		ListWidgetGroup *group = ListWidget_FindGroup(category, device->GetType());
		if (NULL != group)
		{
			item = ListWidget_FindGroupItem(group, device->GetName());
			if (NULL != item)
			{
				image = item->image;
				if (NULL != image)
					DeviceImage_AddRef(image);
				ListWidget_RemoveItem(self, hwnd, device->GetName());
			}
		}
	}

	result = ListWidget_DeviceAdd(hwnd, device, image);

	if (NULL != image)
		DeviceImage_Release(image);

	return result;
}

static BOOL
ListWidget_DeviceDetach(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetCategory *category;
	ListWidgetItem *item;
	DeviceImage *image;
	BOOL result;

	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	image = NULL;

	category = ListWidget_FindCategory(self, "attached");
	if (NULL != category)
	{
		ListWidgetGroup *group = ListWidget_FindGroup(category, device->GetType());
		if (NULL != group)
		{
			item = ListWidget_FindGroupItem(group, device->GetName());
			if (NULL != item)
			{
				image = item->image;
				if (NULL != image)
					DeviceImage_AddRef(image);
				ListWidget_RemoveItem(self, hwnd, device->GetName());
			}
		}
	}

	result = ListWidget_DeviceAdd(hwnd, device, image);
	
	if (NULL != image)
		DeviceImage_Release(image);

	return result;
}

static BOOL
ListWidget_DeviceTitleChanged(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;
	ListWidgetCategory *category;
	wchar_t buffer[1024] = {0};

	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), &category, NULL);
	if (NULL == item)
		return FALSE;

	if (FAILED(device->GetDisplayName(buffer, ARRAYSIZE(buffer))))
		return FALSE;

	ListWidget_SetItemTitle(item, buffer);

	if (NULL == category ||
		FALSE == category->collapsed)
	{
		ListWidget_UpdateLayout(hwnd, ListWidgetLayout_UpdateNow | ListWidgetLayout_KeepStable);
		ListWidget_TooltipUpdateText(self, self->tooltip, item, Tooltip_DeviceTitleChanged);

		if (STATUS_ERROR != self->selectionStatus &&
			item == self->selectedItem)
		{
			ListWidget_UpdateSelectionStatus(self, hwnd, FALSE);
		}
	}

	return TRUE;
}

static BOOL
ListWidget_DeviceSpaceChanged(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;
	ListWidgetCategory *category;
	
	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), &category, NULL);
	if (NULL == item)
		return FALSE;

	if (FAILED(device->GetTotalSpace(&item->spaceTotal)))
		return FALSE;

	if (FAILED(device->GetUsedSpace(&item->spaceUsed)))
		return FALSE;

	if (NULL == category ||
		FALSE == category->collapsed)
	{
		RECT rect;
		POINT origin;
		ListWidgetItemMetric metrics;
		WidgetStyle *style;

		CopyRect(&rect, &item->rect);
		if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
			OffsetRect(&rect, origin.x, origin.y);

		style = WIDGET_GET_STYLE(hwnd);
		if (NULL != style && FALSE != ListWidget_GetItemMetrics(style, &metrics))
		{
			rect.top += metrics.offsetTop + 
						metrics.imageOffsetTop + 
						self->imageSize.cy + 
						metrics.imageOffsetBottom + 
						metrics.spacebarOffsetTop;
			rect.left += metrics.offsetLeft;
			rect.right -= metrics.offsetRight;
			rect.bottom = rect.top + metrics.spacebarHeight;
		}

		InvalidateRect(hwnd, &rect, FALSE);
		UpdateWindow(hwnd);

		ListWidget_TooltipUpdateText(self, self->tooltip, item, Tooltip_DeviceSpaceChanged);

		if (STATUS_ERROR != self->selectionStatus &&
			item == self->selectedItem)
		{
			ListWidget_UpdateSelectionSpaceStatus(self, hwnd, FALSE);
		}
	}
	

	return TRUE;
}

static BOOL
ListWidget_DeviceIconChanged(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;
	ListWidgetCategory *category;
	DeviceImage *previousImage;

	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), &category, NULL);
	if (NULL == item)
		return FALSE;

	previousImage = item->image;
	item->image = NULL;

	if (NULL == category ||
		FALSE == category->collapsed)
	{
		RECT rect;
		POINT origin;
		ListWidgetItemMetric metrics;
		WidgetStyle *style;
		
		
		style = WIDGET_GET_STYLE(hwnd);
		if (NULL == style ||
			FALSE == ListWidget_GetItemMetrics(style, &metrics) ||
			FALSE == ListWidget_GetItemImageRect(self, item, &metrics, &rect))
		{
			CopyRect(&rect, &item->rect);
		}

		if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
			OffsetRect(&rect, origin.x, origin.y);

		InvalidateRect(hwnd, &rect, FALSE);
		UpdateWindow(hwnd);
	}

	if (NULL != previousImage)
		DeviceImage_Release(previousImage);

	return TRUE;
}

static BOOL
ListWidget_DeviceCommandChanged(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;


	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), NULL, NULL);
	if (NULL == item)
		return FALSE;

	if (item == self->hoveredItem && 
		NULL == self->activeMenu)
	{
		ListWidget_UpdateActiveCommands(self, hwnd);
	}
		
	return TRUE;
}

static BOOL
ListWidget_DeviceActivityStarted(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;
	ListWidgetCategory *category;
	size_t index;

	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), &category, NULL);
	if (NULL == item)
		return FALSE;


	index = self->activeItems.size();
	while(index--)
	{
		if (item == self->activeItems[index])
			return FALSE;
	}

	if (FALSE != ListWidget_CreateItemActivity(item))
	{
		ifc_deviceactivity *activity;

		ListWidget_RegisterActiveItem(self, hwnd, item);
		
		if(S_OK == device->GetActivity(&activity) && 
			NULL != activity)
		{
			ListWidget_UpdateItemActivity(item, activity);
			activity->Release();
		}
	}
		
	
	if (NULL == category ||
		FALSE == category->collapsed)
	{
		ListWidget_InvalidateItemImage(self, hwnd, item);

		if (STATUS_ERROR != self->selectionStatus &&
			item == self->selectedItem)
		{
			ListWidget_UpdateSelectionStatus(self, hwnd, FALSE);
		}

	}

	if (item == self->hoveredItem &&
		NULL == self->activeMenu)
	{
		ListWidget_UpdateActiveCommands(self, hwnd);
	}

	return TRUE;
}

static BOOL
ListWidget_DeviceActivityFinished(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;
	ListWidgetCategory *category;
	BOOL activityChanged;

	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), &category, NULL);
	if (NULL == item)
		return FALSE;

	activityChanged = FALSE;

	if (FALSE != ListWidget_UnregisterActiveItem(self, hwnd, item))
		activityChanged = TRUE;
	if (FALSE != ListWidget_DeleteItemActivity(item))
		activityChanged = TRUE;

	if (FALSE != activityChanged)
	{
		if (NULL == category ||
			FALSE == category->collapsed)
		{
			ListWidget_InvalidateItemImage(self, hwnd, item);
		}

		if (item == self->hoveredItem &&
			NULL == self->activeMenu)
		{
			ListWidget_UpdateActiveCommands(self, hwnd);
		}

		if (STATUS_ERROR != self->selectionStatus &&
			item == self->selectedItem)
		{
			ListWidget_UpdateSelectionStatus(self, hwnd, FALSE);
		}
	}

	return TRUE;
}

static BOOL
ListWidget_DeviceActivityChanged(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;
	ListWidgetCategory *category;
	ifc_deviceactivity *activity;
	ListWidgetActivityChange changes;

	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), &category, NULL);
	if (NULL == item)
		return FALSE;

	if (S_OK == device->GetActivity(&activity) && 
		NULL != activity)
	{
		changes = ListWidget_UpdateItemActivity(item, activity);
		activity->Release();
	}
	else
		changes = ListWidgetActivityChanged_Nothing;

	//if (FALSE != self->activityTimerEnabled)
	//	changes &= ~ListWidgetActivityChanged_Percent;

	if (ListWidgetActivityChanged_Nothing != changes &&
		(NULL == category || FALSE == category->collapsed))
	{
		ListWidget_InvalidateItemActivity(self, hwnd, item, changes);
		ListWidget_TooltipUpdateText(self, self->tooltip, item, Tooltip_DeviceActivityChanged);

		if (0 != (ListWidgetActivityChanged_Title & changes) && 
			STATUS_ERROR != self->selectionStatus &&
			item == self->selectedItem)
		{
			ListWidget_UpdateSelectionStatus(self, hwnd, FALSE);
		}

	}
	

	return TRUE;
}

static BOOL
ListWidget_DeviceModelChanged(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;
	ListWidgetCategory *category;
	
	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), &category, NULL);
	if (NULL == item)
		return FALSE;

	
	if (NULL == category || 
		FALSE == category->collapsed)
	{		
		ListWidget_TooltipUpdateText(self, self->tooltip, item, Tooltip_DeviceModelChanged);

		if (STATUS_ERROR != self->selectionStatus &&
			item == self->selectedItem)
		{
			ListWidget_UpdateSelectionStatus(self, hwnd, FALSE);
		}
	}
	return TRUE;
}

static BOOL
ListWidget_DeviceStatusChanged(HWND hwnd, ifc_device *device)
{
	ListWidget *self;
	ListWidgetItem *item;
	ListWidgetCategory *category;
	
	self = WIDGET_GET_SELF(hwnd, ListWidget);

	if (NULL == self || NULL == device)
		return FALSE;

	item = ListWidget_FindItem(self, device->GetName(), &category, NULL);
	if (NULL == item)
		return FALSE;

	
	if (NULL == category || 
		FALSE == category->collapsed)
	{		
		ListWidget_TooltipUpdateText(self, self->tooltip, item, Tooltip_DeviceStatusChanged);

		if (STATUS_ERROR != self->selectionStatus &&
			item == self->selectedItem)
		{
			ListWidget_UpdateSelectionStatus(self, hwnd, FALSE);
		}
	}
	return TRUE;
}

static void
ListWidget_DeviceCb(ifc_device *device, DeviceEvent eventId, void *user)
{
	HWND hwnd;
	hwnd = (HWND)user;
	
	switch(eventId)
	{
		case Event_DeviceAdded:
			ListWidget_DeviceAdd(hwnd, device, NULL);
			break;
		case Event_DeviceRemoved:
			ListWidget_DeviceRemove(hwnd, device);
			break;
		case Event_DeviceHidden:
			ListWidget_DeviceRemove(hwnd, device);
			break;
		case Event_DeviceShown:
			ListWidget_DeviceAdd(hwnd, device, NULL);
			break;
		case Event_DeviceAttached:
			ListWidget_DeviceAttach(hwnd, device);
			break;
		case Event_DeviceDetached:
			ListWidget_DeviceDetach(hwnd, device);
			break;
		case Event_DeviceDisplayNameChanged:
			ListWidget_DeviceTitleChanged(hwnd, device);
			break;
		case Event_DeviceTotalSpaceChanged:
			ListWidget_DeviceSpaceChanged(hwnd, device);
			break;
		case Event_DeviceUsedSpaceChanged:
			ListWidget_DeviceSpaceChanged(hwnd, device);
			break;
		case Event_DeviceIconChanged:
			ListWidget_DeviceIconChanged(hwnd, device);
			break;
		case Event_DeviceActivityStarted:
			ListWidget_DeviceActivityStarted(hwnd, device);
			break;
		case Event_DeviceActivityFinished:
			ListWidget_DeviceActivityFinished(hwnd, device);
			break;
		case Event_DeviceActivityChanged:
			ListWidget_DeviceActivityChanged(hwnd, device);
			break;
		case Event_DeviceCommandChanged:
			ListWidget_DeviceCommandChanged(hwnd, device);
			break;
		case Event_DeviceModelChanged:
			ListWidget_DeviceModelChanged(hwnd, device);
			break;
		case Event_DeviceStatusChanged:
			ListWidget_DeviceStatusChanged(hwnd, device);
			break;

	}
}


static BOOL
ListWidget_RegisterDeviceHandler(ListWidget *self, HWND hwnd)
{
	HWND eventRelay;
	DeviceEventCallbacks callbacks;

	if (NULL == self)
		return FALSE;

	if (0 != self->deviceHandler)
		return FALSE;

	eventRelay = Plugin_GetEventRelayWindow();
	if (NULL == eventRelay)
		return FALSE;
	
	ZeroMemory(&callbacks, sizeof(callbacks));
	callbacks.deviceCb = ListWidget_DeviceCb;

	self->deviceHandler = EVENTRELAY_REGISTER_HANDLER(eventRelay, &callbacks, hwnd); 
	return (0 != self->deviceHandler);
}

static BOOL 
ListWidget_InitCb(HWND hwnd, void **object, void *param)
{
	ListWidget *self;
	HWND sliderWindow;
	int imageHeight, imageWidth;

	self = new ListWidget();
	if (NULL == self)
		return FALSE;

	self->flags = (ListWidgetFlags)0;
	self->hoveredItem = NULL;
	self->selectedItem = NULL;
	self->titleEditItem = NULL;
	self->pressedCategory = NULL;
	self->itemWidth = 0;
	self->spacebarBitmap = NULL;
	self->hoverBitmap = NULL;
	self->selectBitmap = NULL;
	self->inactiveSelectBitmap = NULL;
	self->largeBadgeBitmap = NULL;
	self->smallBadgeBitmap = NULL;
	self->arrowsBitmap = NULL;
	self->itemsPerLine = 0;
	self->deviceHandler = 0;
	self->activeMenu = NULL;
	self->previousMouse.x = -1;
	self->previousMouse.y = -1;
	self->commands = NULL;
	self->commandsCount = 0;
	self->commandsMax = 0;
	self->unknownCommandLargeImage = NULL;
	self->unknownCommandSmallImage = NULL;

	ZeroMemory(&self->activityMetrics, sizeof(ListWidgetActivityMetric));
	self->activityFont = NULL;
	self->activityProgressImage = NULL;
	self->activityBadgeBitmap = NULL;
	self->activityTimerEnabled = FALSE;

	SetSizeEmpty(&self->connectionSize);
	SetSizeEmpty(&self->primaryCommandSize);
	SetSizeEmpty(&self->secondaryCommandSize);

	self->selectionStatus = STATUS_ERROR;

	self->titleEditor = NULL;

	BackBuffer_Initialize(&self->backBuffer, hwnd);

	imageHeight = Config_ReadInt("View", "imageHeight", LISTWIDGET_IMAGE_DEFAULT_HEIGHT);
	if (imageHeight < LISTWIDGET_IMAGE_MIN_HEIGHT)
		imageHeight = LISTWIDGET_IMAGE_MIN_HEIGHT;
	else if (imageHeight > LISTWIDGET_IMAGE_MAX_HEIGHT)
		imageHeight = LISTWIDGET_IMAGE_MAX_HEIGHT;

	imageWidth = (imageHeight * LISTWIDGET_IMAGE_DEFAULT_WIDTH)/LISTWIDGET_IMAGE_DEFAULT_HEIGHT;

	ListWidget_SetImageSize(self, hwnd, imageWidth, imageHeight, FALSE);

	ListWidget_CreateDefaultCategories(self);

	*object = self;

	sliderWindow = MANAGERVIEW_GET_ZOOM_SLIDER(GetParent(hwnd));
	if (NULL != sliderWindow)
	{
		int pos;

		SendMessage(sliderWindow, TBM_SETPAGESIZE, 0, 10);
		SendMessage(sliderWindow, TBM_SETLINESIZE, 0, 10);
		SendMessage(sliderWindow, TBM_SETRANGE, TRUE, MAKELPARAM(-100, 100));
		SendMessage(sliderWindow, TBM_SETTIC, 0, 0);

		if (imageHeight == LISTWIDGET_IMAGE_DEFAULT_HEIGHT)
			pos = 0;
		else if (imageHeight > LISTWIDGET_IMAGE_DEFAULT_HEIGHT)
			pos = (100 * (imageHeight- LISTWIDGET_IMAGE_DEFAULT_HEIGHT))/(LISTWIDGET_IMAGE_MAX_HEIGHT - LISTWIDGET_IMAGE_DEFAULT_HEIGHT);
		else 
			pos = (100 * (imageHeight - LISTWIDGET_IMAGE_MIN_HEIGHT))/(LISTWIDGET_IMAGE_DEFAULT_HEIGHT - LISTWIDGET_IMAGE_MIN_HEIGHT) - 100;

		SendMessage(sliderWindow, TBM_SETPOS, TRUE, pos);
	}

	self->tooltip = ListWidget_TooltipCreate(hwnd);

	ListWidget_AddExistingDevices(self, hwnd);
	ListWidget_RegisterDeviceHandler(self, hwnd);
	return TRUE;
}

static void 
ListWidget_DestroyCb(ListWidget *self, HWND hwnd)
{
	HWND sliderWindow;
	size_t index;

	if (NULL == self)
		return;


	Config_WriteInt("View", "imageHeight", self->imageSize.cy);

	if (0 != self->deviceHandler)
	{
		HWND eventRelay;
		eventRelay = Plugin_GetEventRelayWindow();
		if (NULL != eventRelay)
		{
			EVENTRELAY_UNREGISTER_HANDLER(eventRelay, self->deviceHandler); 
		}
	}

	index = self->activeItems.size();
	if (0 != index)
	{
		KillTimer(hwnd, LISTWIDGETTIMER_PROGRESS_TICK_ID);
		self->activityTimerEnabled = FALSE;
		while(index--)
			ListWidget_DeleteItemActivity(self->activeItems[index]);

	}

	index = self->categories.size();
	if (0 != index)
	{
		while(index--)
		{
			ListWidgetCategory *category = self->categories[index];
			Config_WriteBool("CollapsedCategories", category->name, category->collapsed);
			ListWidget_DestroyCategory(category);
		}
		self->categories.clear();
	}

	ListWidget_RemoveAllConnections(self);

	BackBuffer_Uninitialize(&self->backBuffer);

	if (NULL != self->spacebarBitmap)
		DeleteObject(self->spacebarBitmap);

	if (NULL != self->hoverBitmap)
		DeleteObject(self->hoverBitmap);

	if (NULL != self->selectBitmap)
		DeleteObject(self->selectBitmap);

	if (NULL != self->inactiveSelectBitmap)
		DeleteObject(self->inactiveSelectBitmap);

	if (NULL != self->largeBadgeBitmap)
		DeleteObject(self->largeBadgeBitmap);

	if (NULL != self->smallBadgeBitmap)
		DeleteObject(self->smallBadgeBitmap);

	if (NULL != self->unknownCommandLargeImage)
		DeviceImage_Release(self->unknownCommandLargeImage);

	if (NULL != self->unknownCommandSmallImage)
		DeviceImage_Release(self->unknownCommandSmallImage);

	if (NULL != self->arrowsBitmap)
		DeleteObject(self->arrowsBitmap);

	if (NULL != self->activityProgressImage)
		DeviceImage_Release(self->activityProgressImage);

	if (NULL != self->activityBadgeBitmap)
		DeleteObject(self->activityBadgeBitmap);

	if (NULL != self->activityFont)
		DeleteObject(self->activityFont);

	ListWidget_TooltipDestroy(self->tooltip);

	if (NULL != self->commands)
	{
		ListWidget_DestroyAllCommands(self->commands, self->commandsCount);
		free(self->commands);
	}

	free(self);

	sliderWindow = MANAGERVIEW_GET_ZOOM_SLIDER(GetParent(hwnd));
	if (NULL != sliderWindow)
	{
		ShowWindow(sliderWindow, SW_HIDE);
	}
}

static void
ListWidget_LayoutCb(ListWidget *self, HWND hwnd, WidgetStyle *style, 
					 const RECT *clientRect, SIZE *viewSize, BOOL redraw)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	ListWidgetCategoryMetric categoryMetrics;
	size_t itemsInLine;
	long viewWidth, itemHeight, lineHeight;
	long itemTextWidth, itemTextHeightMax, textHeight, fontHeight;
	long categoryHeight, categorySpacing, categorySpacingCollapsed;
	SIZE itemSpacing, widgetSize, itemSize;
	POINT widgetOffset, categoryOffset, itemOffset;
	RECT elementRect;
	HDC targetDC;
	HFONT targetPrevFont;
	TEXTMETRIC textMetrics;

	if (NULL == style)
		return;

	viewWidth = RECTWIDTH(*clientRect);

	if (FALSE == ListWidget_CalculateItemBaseSize(self, style, &itemSize, &itemTextWidth))
		SetSizeEmpty(&itemSize);

	self->itemWidth = itemSize.cx;

	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(widgetOffset.x, style, LISTWIDGET_OFFSET_LEFT_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(widgetOffset.y, style, LISTWIDGET_OFFSET_TOP_DLU, 1);
	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(categoryOffset.x, style, LISTWIDGET_CATEGORY_OFFSET_LEFT_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(categoryOffset.y, style, LISTWIDGET_CATEGORY_OFFSET_TOP_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(categorySpacing, style, LISTWIDGET_CATEGORY_SPACING_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(categorySpacingCollapsed, style, LISTWIDGET_CATEGORY_SPACING_COLLAPSED_DLU, 1);

	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(itemOffset.x, style, LISTWIDGET_ITEM_OFFSET_LEFT_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(itemOffset.y, style, LISTWIDGET_ITEM_OFFSET_TOP_DLU, 1);

	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(itemSpacing.cx, style, LISTWIDGET_ITEM_SPACING_HORZ_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(itemSpacing.cy, style, LISTWIDGET_ITEM_SPACING_VERT_DLU, 1);

	self->itemsPerLine = ((viewWidth - (widgetOffset.x + itemOffset.x))/* + itemSpacing.cx*/) / (itemSize.cx + itemSpacing.cx);
	self->itemsPerLine = MAX(self->itemsPerLine, 1);

	itemsInLine = 0;
	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				if (itemsInLine < group->items.size())
					itemsInLine = group->items.size();
			}
		}
	}

	if (self->itemsPerLine < itemsInLine && self->itemsPerLine > 1)
		itemSpacing.cx = (LONG)(((viewWidth - (widgetOffset.x + itemOffset.x)) - (itemSize.cx * self->itemsPerLine))/self->itemsPerLine);

	if (itemsInLine < self->itemsPerLine)
		self->itemsPerLine = itemsInLine;

	widgetSize.cx = widgetOffset.x + itemOffset.x + itemSize.cx;
	widgetSize.cy = widgetOffset.y;

	targetDC = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != targetDC)
	{
		targetPrevFont = SelectFont(targetDC, WIDGETSTYLE_CATEGORY_FONT(style));
		categoryHeight = Graphics_GetFontHeight(targetDC);

		SelectFont(targetDC, WIDGETSTYLE_TEXT_FONT(style));
		fontHeight = Graphics_GetFontHeight(targetDC);
		itemTextHeightMax = LISTWIDGET_ITEM_TITLE_MAX_LINES * fontHeight;
	}
	else
	{
		targetPrevFont = NULL;
		itemTextHeightMax = 0;
		categoryHeight = 0;
		fontHeight = 0;
	}

	if (NULL == targetDC ||
		FALSE == GetTextMetrics(targetDC, &textMetrics))
	{
		ZeroMemory(&textMetrics, sizeof(textMetrics));
	}

	if (FALSE != ListWidget_GetCategoryMetrics(style, &categoryMetrics))
	{
		if (categoryHeight < categoryMetrics.minHeight)
			categoryHeight = categoryMetrics.minHeight;

		categoryHeight += categoryMetrics.offsetTop + categoryMetrics.offsetBottom +
						  categoryMetrics.lineHeight + categoryMetrics.lineOffsetTop;
	}

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];

		SetRect(&category->rect, 0, 0, viewWidth, categoryHeight);

		long offsetX = clientRect->left + categoryOffset.x;
		long offsetY = clientRect->top + widgetSize.cy;
		if (0 == iCategory)
			offsetY += categoryOffset.y;
		else
		{
			if (FALSE == self->categories[iCategory - 1]->collapsed)
				offsetY += categorySpacing;
			else
				offsetY += categorySpacingCollapsed;
		}

		OffsetRect(&category->rect, offsetX, offsetY);

		widgetSize.cy = category->rect.bottom - clientRect->top;
		size_t itemsInCategory = 0;

		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				itemsInLine = 1;
				lineHeight = 0;
				
				if (0 != group->items.size())
				{
					if (0 == iGroup)
						widgetSize.cy += itemOffset.y;
					else
						widgetSize.cy += itemSpacing.cy;

					for(iItem = 0; iItem < group->items.size(); iItem++, itemsInLine++)
					{
						item = group->items[iItem];
						if (itemsInLine > self->itemsPerLine)
						{
							widgetSize.cy += lineHeight + itemSpacing.cy;
							lineHeight = 0;
							itemsInLine = 1;
						}

						itemHeight = itemSize.cy;
						itemsInCategory++;

						if (-1 == item->titleSize.cy)
						{
							if (FALSE == IS_STRING_EMPTY(item->title))
							{
								SetRect(&elementRect, 0, 0, itemTextWidth - textMetrics.tmAveCharWidth/2, 0);
								if (FALSE != DrawText(targetDC, item->title, -1, &elementRect, 
														DT_NOPREFIX | DT_CALCRECT | DT_EDITCONTROL | DT_WORDBREAK))
								{
									SetSize(&item->titleSize, RECTWIDTH(elementRect), RECTHEIGHT(elementRect));
									item->titleSize.cx += textMetrics.tmAveCharWidth/2;
									if (item->titleSize.cx > itemTextWidth)
										item->titleSize.cx = itemTextWidth;


								}
								else
									SetSizeEmpty(&item->titleSize);
							}
							else
							{
								SetSize(&item->titleSize, 0, textMetrics.tmHeight);
							}
						}

						textHeight = item->titleSize.cy;
						if (textHeight > itemTextHeightMax)
						{
							textHeight = itemTextHeightMax;
							ListWidgetItem_SetTextTruncated(item);
						}
						itemHeight += textHeight;
																	
						SetRect(&item->rect, 0, 0, itemSize.cx, itemHeight);

						offsetX = long(clientRect->left + itemOffset.x +
								  (itemsInLine - 1)*(itemSize.cx + itemSpacing.cx));
						offsetY = clientRect->top + widgetSize.cy;

						OffsetRect(&item->rect, offsetX, offsetY);
						
						if (lineHeight < itemHeight)
							lineHeight = itemHeight;
					}

					if (0 != lineHeight)
						widgetSize.cy += lineHeight;
				}
			}

			if (0 == itemsInCategory && 
				FALSE == IS_STRING_EMPTY(category->emptyText))
			{
				SetRect(&category->emptyTextRect, 0, 0, viewWidth - textMetrics.tmAveCharWidth/2, 0);
				if (FALSE != DrawText(targetDC, category->emptyText, -1, &category->emptyTextRect, 
										DT_NOPREFIX | DT_CALCRECT | DT_EDITCONTROL | DT_WORDBREAK))
				{
					category->emptyTextRect.right += textMetrics.tmAveCharWidth/2;
					if (category->emptyTextRect.right > viewWidth)
						category->emptyTextRect.right = viewWidth;

					offsetX = clientRect->left + categoryOffset.x + 
							 (viewWidth - category->emptyTextRect.right)/2;

					offsetY = clientRect->top + widgetSize.cy + itemOffset.y + fontHeight/2;

					OffsetRect(&category->emptyTextRect, offsetX, offsetY);

					widgetSize.cy += RECTHEIGHT(category->emptyTextRect) + itemOffset.y + fontHeight;
				}
				else
					SetRectEmpty(&category->emptyTextRect);
			}
		}
		else
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++, itemsInLine++)
				{
					item = group->items[iItem];
					SetRectEmpty(&item->rect);
					itemsInCategory++;
				}
			}
		}
	}

	widgetSize.cy += WIDGETSTYLE_DLU_TO_VERT_PX(style, LISTWIDGET_OFFSET_BOTTOM_DLU);
	viewSize->cx = widgetSize.cx;
	viewSize->cy = widgetSize.cy;

	size_t commandsMax = 1;
	if (self->commandsMax != commandsMax)
	{
		if (NULL != self->commands)
		{
			ListWidget_DestroyAllCommands(self->commands, self->commandsCount);
			free(self->commands);
		}

		self->commandsCount = 0;
		self->commands = (ListWidgetCommand**)malloc(sizeof(ListWidgetCommand*) * commandsMax);

		if (NULL != self->commands)
			self->commandsMax = commandsMax;
		else
			self->commandsMax = 0;

	}

	if (NULL != targetDC)
	{
		SelectFont(targetDC, targetPrevFont);
		ReleaseDC(hwnd, targetDC);
	}
}

static BOOL 
ListWidget_PaintCb(ListWidget *self, HWND hwnd, WidgetStyle *style, HDC hdc, const RECT *paintRect, BOOL erase)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	HFONT prevFont;
	FillRegion fillRegion;
	ListWidgetPaint paint;
	size_t itemsInCategory;

	if (FALSE == ListWidgetPaint_Initialize(&paint, self, style, hwnd, hdc, paintRect, erase))
		return FALSE;

	SetTextColor(hdc, style->textColor);
	SetBkColor(hdc, style->backColor);
	SetBkMode(hdc, TRANSPARENT);
	prevFont = SelectFont(hdc, style->textFont);

	FillRegion_Init(&fillRegion, paintRect);

	if (FALSE != erase)
	{
		for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
		{
			category = self->categories[iCategory];
			
			FillRegion_ExcludeRect(&fillRegion, &category->rect);

			if (FALSE == category->collapsed)
			{
				itemsInCategory = 0;
				for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
				{
					group = category->groups[iGroup];
					for(iItem = 0; iItem < group->items.size(); iItem++)
					{
						item = group->items[iItem];
						itemsInCategory++;
						FillRegion_ExcludeRect(&fillRegion, &item->rect);
					}
				}

				if (0 == itemsInCategory &&
					FALSE == IS_STRING_EMPTY(category->emptyText))
				{
					FillRegion_ExcludeRect(&fillRegion, &category->emptyTextRect);
				}
			}
		}
		FillRegion_BrushFill(&fillRegion, hdc, style->backBrush);
		FillRegion_SetEmpty(&fillRegion);
	}

	
	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
		
		if (FALSE == ListWidgetPaint_DrawCategory(&paint, category))
			FillRegion_AppendRect(&fillRegion, &category->rect);

		if (FALSE == category->collapsed)
		{
			itemsInCategory = 0;
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++)
				{
					item = group->items[iItem];
					itemsInCategory++;
					if (FALSE == ListWidgetPaint_DrawItem(&paint, item))
						FillRegion_AppendRect(&fillRegion, &item->rect);
				}
			}

			if (0 == itemsInCategory &&
				FALSE == IS_STRING_EMPTY(category->emptyText))
			{
				if (FALSE == ListWidgetPaint_DrawEmptyCategoryText(&paint, category))
					FillRegion_AppendRect(&fillRegion, &category->emptyTextRect);
			}
		}
	}
	
	FillRegion_BrushFill(&fillRegion, hdc, style->backBrush);
	FillRegion_Uninit(&fillRegion);

	SelectFont(hdc, prevFont);

	ListWidgetPaint_Uninitialize(&paint);

	return TRUE;
}

static void
ListWidget_StyleColorChangedCb(ListWidget *self, HWND hwnd, WidgetStyle *style)
{

	if (NULL == self)
		return;

	if (NULL != self->spacebarBitmap)
	{
		DeleteObject(self->spacebarBitmap);
		self->spacebarBitmap = NULL;
	}

	if (NULL != self->hoverBitmap)
	{
		DeleteObject(self->hoverBitmap);
		self->hoverBitmap = NULL;
	}

	if (NULL != self->selectBitmap)
	{
		DeleteObject(self->selectBitmap);
		self->selectBitmap = NULL;
	}
	if (NULL != self->inactiveSelectBitmap)
	{
		DeleteObject(self->inactiveSelectBitmap);
		self->inactiveSelectBitmap = NULL;
	}
	if (NULL != self->arrowsBitmap)
	{
		DeleteObject(self->arrowsBitmap);
		self->arrowsBitmap = NULL;
	}

	
	
	ListWidget_ResetConnnectionsColors(self, style);
	
	if (NULL != self->titleEditor)
		ListWidget_UpdateTitleEditorColors(self->titleEditor, style);

}

static void
ListWidget_StyleFontChangedCb(ListWidget *self, HWND hwnd, WidgetStyle *style)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;

	if (NULL == self)
		return;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
		category->countWidth = -1;
		category->titleWidth = -1;
		SetRect(&category->emptyTextRect, -1, -1, -1, -1);

		for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
		{
			group = category->groups[iGroup];
			for(iItem = 0; iItem < group->items.size(); iItem++)
			{
				item = group->items[iItem];
				SetSize(&item->titleSize, -1, -1);
			}
		}
	}

	ListWidget_TooltipFontChanged(self->tooltip);

	if (NULL != self->titleEditor)
		SendMessage(self->titleEditor, WM_SETFONT, (WPARAM)WIDGETSTYLE_TEXT_FONT(style), TRUE);
	
}

static BOOL 
ListWidget_MouseMoveCb(ListWidget *self, HWND hwnd, unsigned int vKeys, const POINT *cursor)
{
	if (self->previousMouse.x == cursor->x && 
		self->previousMouse.y == cursor->y)
	{
		return TRUE;
	}

	self->previousMouse = *cursor;

	if (FALSE != ListWidget_UpdateHoverEx(self, hwnd, cursor))
		UpdateWindow(hwnd);

	ListWidget_TooltipRelayMouseMessage(self->tooltip, WM_MOUSEMOVE, vKeys, cursor);
	return TRUE;
}

static BOOL 
ListWidget_LeftButtonDownCb(ListWidget *self, HWND hwnd, unsigned int vKeys, const POINT *cursor)
{
	ListWidgetItem *selectedItem;
	ListWidgetCategory *pressedCategory;
	ListWidgetItemPart pressedPart;
	ListWidgetCommand *command, *pressedCommand;
	WidgetStyle *style; 
	RECT rect, pressedPartRect;
	POINT pt, origin;
	size_t index;

	style = WIDGET_GET_STYLE(hwnd);
	if (NULL == style)
		return FALSE;
	
	pt = *cursor;
	
	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}

	selectedItem = NULL;
	pressedCategory = NULL;
	pressedPart = ListWidgetItemPart_None;
	pressedCommand = NULL;

	if (FALSE != GetClientRect(hwnd, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		
		pt.x -= origin.x;
		pt.y -= origin.y;

		pressedCategory = ListWidget_GetCategoryFromPoint(self, pt);
		
		if (NULL == pressedCategory)
		{
			selectedItem = ListWidget_GetItemFromPoint(self, pt);
			if (NULL != selectedItem)
			{
				ListWidgetItemMetric metrics;
				if (FALSE != ListWidget_GetItemMetrics(style, &metrics))
				{
					pressedPart = ListWidgetItemPart_Command;
					if (selectedItem == self->selectedItem)
							pressedPart |= ListWidgetItemPart_Title;

					pressedPart = ListWidget_GetItemPartFromPoint(self, selectedItem, &metrics, pt,
																	pressedPart, &pressedPartRect);

					if (ListWidgetItemPart_Title == pressedPart &&
						self->titleEditItem != selectedItem &&
						hwnd == GetFocus() &&
						FALSE == ListWidgetItem_IsTextEdited(selectedItem))
					{						
						self->titleEditItem = selectedItem;

						SetTimer(hwnd, LISTWIDGETTIMER_EDIT_TITLE_ID, 
									GetDoubleClickTime() + 1, 
									ListWidget_BeginTitleEditTimerCb);
									
					}
				}
			}
		
		}
	}

	if (NULL != pressedCategory)
	{
		ListWidgetCategoryMetric metrics;
		if (FALSE == ListWidget_GetCategoryMetrics(style, &metrics))
			SetRectEmpty(&rect);
		else
		{
			CopyRect(&rect, &pressedCategory->rect);
			rect.left += metrics.offsetLeft;
			rect.top += metrics.offsetTop;
			rect.right = rect.left + metrics.iconWidth;
			rect.bottom -= (metrics.offsetBottom + metrics.lineHeight + metrics.lineOffsetTop);
		}

		if (FALSE == PtInRect(&rect, pt))
			pressedCategory = NULL;
	}

	self->pressedCategory = pressedCategory;

		
	ListWidget_SelectItem(self, hwnd, selectedItem, FALSE);
	
	ListWidget_EnsureFocused(self, hwnd, FALSE);


	if (ListWidgetItemPart_Command == pressedPart &&
		NULL != selectedItem)
	{
		OffsetRect(&pressedPartRect, -selectedItem->rect.left, -selectedItem->rect.top);
	}

	index = self->commandsCount;

	while(index--)
	{
		command = self->commands[index];
		if (NULL == pressedCommand &&
			ListWidgetItemPart_Command == pressedPart &&
			ListWidget_GetCommandRectEqual(command, &pressedPartRect))
		{
			pressedCommand = command;
		}

		if (FALSE != ListWidget_GetCommandPressed(command) || 
			pressedCommand == command)
		{
			if (FALSE != ListWidget_SetCommandPressed(pressedCommand, (pressedCommand == command)) &&
				FALSE != ListWidget_GetCommandRect(command, &rect) && 
				NULL != selectedItem)
			{
				OffsetRect(&rect, selectedItem->rect.left + origin.x, selectedItem->rect.top + origin.y);
				InvalidateRect(hwnd, &rect, FALSE);
			}
		}
	}

	if (NULL != pressedCommand)
		self->flags |= ListWidgetFlag_LButtonDownOnCommand;
	else
		self->flags &= ~ListWidgetFlag_LButtonDownOnCommand;

	
	if (GetCapture() != hwnd)
		SetCapture(hwnd);

	UpdateWindow(hwnd);
	
	ListWidget_TooltipHide(self->tooltip);

	return TRUE;
}

static BOOL 
ListWidget_LeftButtonUpCb(ListWidget *self, HWND hwnd, unsigned int vKeys, const POINT *cursor)
{	
	WidgetStyle *style; 
	RECT rect;
	POINT pt, origin;
	ListWidgetCategory *pressedCategory;
	ListWidgetCommand *pressedCommand;
	size_t index;
	BOOL updateWindow;

	style = WIDGET_GET_STYLE(hwnd);

	pt = *cursor;

	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}
		
	pressedCategory = NULL;
	pressedCommand = NULL;
	updateWindow = FALSE;

	if (FALSE != GetClientRect(hwnd, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		
		pt.x -= origin.x;
		pt.y -= origin.y;

		pressedCategory = ListWidget_GetCategoryFromPoint(self, pt);

		if (NULL == pressedCategory)
		{			
			if (NULL != self->selectedItem && 
				FALSE != ListWidgetItem_IsInteractive(self->selectedItem))
			{
				
				index = self->commandsCount;
				while(index--)
				{
					if (FALSE != ListWidget_GetCommandRect(self->commands[index], &rect))
					{
						OffsetRect(&rect, self->selectedItem->rect.left, self->selectedItem->rect.top);
						if (PtInRect(&rect, pt))
						{
							pressedCommand = self->commands[index];
							if (FALSE != ListWidget_GetCommandDisabled(pressedCommand) ||
								FALSE == ListWidget_GetCommandPressed(pressedCommand))
							{
								pressedCommand = NULL;
							}
							break;
						}
					}
				}
			}
		}
	}
	
	if (NULL != self->pressedCategory)
	{
		if (NULL != pressedCategory)
		{		
			ListWidgetCategoryMetric metrics;
			if (FALSE == ListWidget_GetCategoryMetrics(style, &metrics))
				SetRectEmpty(&rect);
			else
			{
				CopyRect(&rect, &pressedCategory->rect);
				rect.left += metrics.offsetLeft;
				rect.top += metrics.offsetTop;
				rect.right = rect.left + metrics.iconWidth;
				rect.bottom -= (metrics.offsetBottom + metrics.lineHeight + metrics.lineOffsetTop);
			}

			if (FALSE == PtInRect(&rect, pt))
				pressedCategory = NULL;
		}

		if(self->pressedCategory == pressedCategory)
		{
			ListWidget_ToggleCategory(pressedCategory, hwnd);
		}
		self->pressedCategory = NULL;
	}

	if (NULL != self->selectedItem)
	{
		if (NULL != pressedCommand && NULL != self->selectedItem)
		{
			ListWidget_SendItemCommand(self->selectedItem->name, 
										ListWidget_GetCommandName(pressedCommand), 
										hwnd, 0, TRUE);
		}

		index = self->commandsCount;
		while(index--)
		{
			if (FALSE != ListWidget_GetCommandPressed(self->commands[index]))
			{
				if (FALSE != ListWidget_SetCommandPressed(self->commands[index], FALSE) &&
					FALSE != ListWidget_GetCommandRect(self->commands[index], &rect))
				{
					OffsetRect(&rect, self->selectedItem->rect.left + origin.x, self->selectedItem->rect.top + origin.y);
					InvalidateRect(hwnd, &rect, FALSE);
					updateWindow = TRUE;
				}
				break;
			}
		}

		if (FALSE != ListWidget_EnsureItemVisisble(self, hwnd, self->selectedItem, VISIBLE_NORMAL))
			updateWindow = TRUE;
	}

	if (FALSE != ListWidget_UpdateHoverEx(self, hwnd, cursor))
		updateWindow = TRUE;
		
	if (GetCapture() == hwnd)
		ReleaseCapture();

	if (FALSE != updateWindow)
		UpdateWindow(hwnd);

	ListWidget_TooltipRelayMouseMessage(self->tooltip, WM_LBUTTONUP, vKeys, cursor);

	return TRUE;
}

static BOOL 
ListWidget_LeftButtonDblClkCb(ListWidget *self, HWND hwnd, unsigned int vKeys, const POINT *cursor)
{
	RECT rect;
	POINT pt = *cursor, origin;
	ListWidgetCategory *category;

	if (NULL != self->titleEditItem)
	{
		KillTimer(hwnd, LISTWIDGETTIMER_EDIT_TITLE_ID);
		self->titleEditItem = NULL;
	}

	if (0 != (ListWidgetFlag_LButtonDownOnCommand & self->flags))
		return FALSE;

	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}

	if (FALSE != GetClientRect(hwnd, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		pt.x -= origin.x;
		pt.y -= origin.y;

		category = ListWidget_GetCategoryFromPoint(self, pt);
		if (NULL != category)
		{
			ListWidget_ToggleCategory(category, hwnd);
			self->pressedCategory = NULL;
			return TRUE;
		}

		ListWidgetItem *item = ListWidget_GetItemFromPointEx(self, pt, &category, NULL);
		if (NULL != item)
		{
			if (NULL != item && 
				FALSE != ListWidgetItem_IsInteractive(item))
			{
				size_t index;
				index = self->commandsCount;
				while(index--)
				{
					if (FALSE != ListWidget_GetCommandRect(self->commands[index], &rect))
					{
						OffsetRect(&rect, item->rect.left, item->rect.top);
						if (PtInRect(&rect, pt))
						{
							item = NULL;
							break;
						}
					}
				}

			}

			if (NULL != item)
			{
				ListWidget_CallItemAction(self, hwnd, category, item);
				return TRUE;
			}

		}
	}

	return FALSE;
}



static BOOL
ListWidget_RightButtonDownCb(ListWidget *self, HWND hwnd, unsigned int vKeys, const POINT *cursor)
{
	ListWidgetItem *selectedItem;
	RECT rect;
	POINT pt, origin;

	pt = *cursor;
	
	if (NULL != self->titleEditItem)
	{
		KillTimer(hwnd, LISTWIDGETTIMER_EDIT_TITLE_ID);
		self->titleEditItem = NULL;
	}

	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}

	selectedItem = NULL;

	if (FALSE != GetClientRect(hwnd, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		pt.x -= origin.x;
		pt.y -= origin.y;
		selectedItem = ListWidget_GetItemFromPoint(self, pt);
	}

	ListWidget_SelectItem(self, hwnd, selectedItem, TRUE);
	
	ListWidget_EnsureFocused(self, hwnd, FALSE);

	if (GetCapture() != hwnd)
		SetCapture(hwnd);
	
	UpdateWindow(hwnd);

	ListWidget_TooltipRelayMouseMessage(self->tooltip, WM_RBUTTONDOWN, vKeys, cursor);
	
	return FALSE; // allow defwindowproc to get this message
}

static BOOL 
ListWidget_RightButtonUpCb(ListWidget *self, HWND hwnd, unsigned int vKeys, const POINT *cursor)
{
	RECT rect;

	if (FALSE != ListWidget_UpdateHoverEx(self, hwnd, cursor))
		UpdateWindow(hwnd);

	if (GetCapture() == hwnd)
		ReleaseCapture();

	ListWidget_TooltipRelayMouseMessage(self->tooltip, WM_RBUTTONUP, vKeys, cursor);


	if (FALSE != GetClientRect(hwnd, &rect) &&
		FALSE != PtInRect(&rect, *cursor))
	{
		ListWidgetItem *item;
		ListWidgetItemMetric metrics;
		POINT pt;

		if (FALSE == ListWidget_GetViewOrigin(hwnd, &pt))
		{
			pt.x = 0;
			pt.y = 0;
		}

		pt.x = cursor->x - pt.x;
		pt.y = cursor->y - pt.y;

		item = ListWidget_GetItemFromPoint(self, pt);
		if (NULL != item)
		{
			WidgetStyle *style = WIDGET_GET_STYLE(hwnd);
			if (NULL != style && 
				FALSE != ListWidget_GetItemMetrics(style, &metrics))
			{
				ListWidgetItemPart part;
				part = ListWidgetItemPart_Command | ListWidgetItemPart_Activity;
				part = ListWidget_GetItemPartFromPoint(self, item, &metrics, pt, part, NULL);
				if (ListWidgetItemPart_None != part)
					return TRUE;
			}
		}
	}

	return FALSE; // allow defwindowproc to get this message
}


static BOOL
ListWidget_KeyDownCb(ListWidget *self, HWND hwnd, unsigned int vKey, unsigned int flags)
{
	ListWidgetItem *selectedItem;
	ListWidgetVisibleFlags visibleFlags;

	selectedItem = NULL;
	visibleFlags = VISIBLE_NORMAL/*VISIBLE_PARTIAL_OK*/;

	switch(vKey)
	{
		case VK_HOME:
			ListWidget_EnsureTopVisible(self, hwnd);
			visibleFlags |= VISIBLE_ALIGN_TOP | VISIBLE_ALIGN_ALWAYS;
			selectedItem = ListWidget_GetFirstItem(self);
			break;
		case VK_END:
			ListWidget_EnsureBottomVisible(self, hwnd);
			visibleFlags |= VISIBLE_ALIGN_BOTTOM | VISIBLE_ALIGN_ALWAYS;
			selectedItem = ListWidget_GetLastItem(self);
			break;
		case VK_LEFT:
			selectedItem = (NULL != self->selectedItem) ?
							ListWidget_GetPreviousItem(self, self->selectedItem) :
							ListWidget_GetLastItem(self);
			if (NULL == selectedItem)
				ListWidget_EnsureTopVisible(self, hwnd);
			break;
		case VK_RIGHT:
			selectedItem = (NULL != self->selectedItem) ?
							ListWidget_GetNextItem(self, self->selectedItem) :
							ListWidget_GetFirstItem(self);
			if (NULL == selectedItem)
				ListWidget_EnsureBottomVisible(self, hwnd);
			break;
		case VK_UP:
			selectedItem = (NULL != self->selectedItem) ?
							ListWidget_GetPreviousLineItem(self, self->selectedItem) :
							ListWidget_GetLastItem(self);
			if (NULL == selectedItem)
				ListWidget_EnsureTopVisible(self, hwnd);
			break;
		case VK_DOWN:
			selectedItem = (NULL != self->selectedItem) ?
							ListWidget_GetNextLineItem(self, self->selectedItem) :
							ListWidget_GetFirstItem(self);
			if (NULL == selectedItem)
				ListWidget_EnsureBottomVisible(self, hwnd);
			break;
		case VK_PRIOR:
			visibleFlags |= VISIBLE_ALIGN_BOTTOM;
			selectedItem = (NULL != self->selectedItem) ?
							ListWidget_GetPreviousPageItem(self, hwnd, self->selectedItem) :
							ListWidget_GetLastItem(self);
			if (NULL == selectedItem)
				ListWidget_EnsureTopVisible(self, hwnd);
			break;

		case VK_NEXT:
			visibleFlags |= VISIBLE_ALIGN_TOP;
			selectedItem = (NULL != self->selectedItem) ?
							ListWidget_GetNextPageItem(self, hwnd, self->selectedItem) :
							ListWidget_GetFirstItem(self);
			if (NULL == selectedItem)
				ListWidget_EnsureBottomVisible(self, hwnd);

			break;
		case VK_RETURN:
			if (NULL != self->selectedItem)
			{
				ListWidget_CallItemAction(self, hwnd, NULL, self->selectedItem);
				ListWidget_EnsureItemVisisble(self, hwnd, self->selectedItem, visibleFlags);
			}
			break;
		case VK_F2:
			if (NULL != self->selectedItem)
			{
				if (NULL != self->titleEditor)
					DestroyWindow(self->titleEditor);
				
				self->titleEditor = ListWidget_BeginItemTitleEdit(self, hwnd, self->selectedItem);
			}
			break;
		
		default:
			return FALSE;
	}

	if (NULL != selectedItem)
	{
		ListWidget_SelectItem(self, hwnd, selectedItem, FALSE);
		ListWidget_EnsureItemVisisble(self, hwnd, self->selectedItem, visibleFlags);
	}
	return TRUE;
}

static BOOL
ListWidget_KeyUpCb(ListWidget *self, HWND hwnd, unsigned int vKey, unsigned int flags)
{
	return FALSE;
}

static BOOL
ListWidget_CharacterCb(ListWidget *self, HWND hwnd, unsigned int vKey, unsigned int flags)
{
	return FALSE;
}

static INT
ListWidget_InputRequestCb(ListWidget *self, HWND hwnd, unsigned int vKey, MSG *message)
{
	INT result;

	if (NULL == message)
		return DLGC_WANTALLKEYS;
	
	ListWidget_TooltipHide(self->tooltip);

	result = DLGC_WANTCHARS;

	switch(vKey)
	{
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
		case VK_RETURN:
			result |= DLGC_WANTALLKEYS;
			break;
	}
	return result;
}


static void
ListWidget_FocusChangedCb(ListWidget *self, HWND hwnd, HWND focusWindow, BOOL focusReceived)
{
	if (NULL == self)
		return;

	if (NULL == self->selectedItem)
	{
		if (FALSE != focusReceived && 0 == (ListWidgetFlag_NoFocusSelect & self->flags))
		{
			BOOL disableSelect;

			disableSelect = FALSE;

			if (NULL != focusWindow)
			{
				HWND ancestorWindow;
				ancestorWindow = hwnd;
				while(NULL != ancestorWindow)
				{
					ancestorWindow = GetAncestor(ancestorWindow, GA_PARENT);
					if (focusWindow == ancestorWindow)
					{
						wchar_t buffer[64] = {0};
						if (0 != GetClassName(focusWindow, buffer, ARRAYSIZE(buffer)) &&
							CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, buffer, -1, L"Winamp Gen", -1))
						{
							disableSelect = TRUE;
						}
						break;
					}
				}
			}

			if (FALSE == disableSelect)
			{
				ListWidgetItem *item;
				item = ListWidget_GetFirstItem(self);
				if (NULL != item)
					ListWidget_SelectItem(self, hwnd, item, FALSE);
			}
		}
	}
	else
	{
		POINT origin;
		RECT rect;

		CopyRect(&rect, &self->selectedItem->rect);
		if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
			OffsetRect(&rect, origin.x, origin.y);

		InvalidateRect(hwnd, &rect, FALSE);
		UpdateWindow(hwnd);
	}
}

static BOOL
ListWidget_ContextMenuCb(ListWidget *self, HWND hwnd, HWND targetWindow, const POINT *cursor)
{
	POINT pt;
	ListWidgetItem *item;

	if (NULL == self)
		return FALSE;


	if (NULL == cursor || 
		(-1 == cursor->x && -1 == cursor->y))
	{
		if (hwnd != targetWindow)
			return FALSE;

		item = self->selectedItem;
		if (NULL != item)
		{
			POINT origin;
			pt.x = RECTWIDTH(item->rect);
			pt.x = item->rect.left + pt.x/2 + pt.x%2;
			pt.y = RECTHEIGHT(item->rect);
			pt.y = item->rect.top + pt.y/2 + pt.y%2;

			if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
			{
				pt.x += origin.x;
				pt.y += origin.y;
			}
		}
		else
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			pt.x = rect.left + 2;
			pt.y = rect.top + 2;
		}

		MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);
	}
	else
	{
		POINT test;
		pt = *cursor;
		
		if (FALSE == ListWidget_GetViewOrigin(hwnd, &test))
		{
			test = pt;
		}
		else
		{
			test.x = pt.x - test.x;
			test.y = pt.y - test.y;
		}
		MapWindowPoints(HWND_DESKTOP, hwnd, &test, 1);
		item = ListWidget_GetItemFromPoint(self, test);
	}

	if (NULL == item)
		ListWidget_DisplayContextMenu(self, hwnd, pt);
	else
		ListWidget_DisplayItemContextMenu(self, hwnd, item, pt);
	
	return TRUE;
};



static void 
ListWidget_ZoomChangingCb(ListWidget *self, HWND hwnd, NMTRBTHUMBPOSCHANGING *zoomInfo)
{
	double pos, height, width;
	int cx, cy;
		
	if (NULL == self)
		return;

	pos = (double)(int)zoomInfo->dwPos;
	if (0 == pos)
	{
		height = LISTWIDGET_IMAGE_DEFAULT_HEIGHT;
	}
	else if (pos < 0)
	{
		height = ((LISTWIDGET_IMAGE_DEFAULT_HEIGHT - LISTWIDGET_IMAGE_MIN_HEIGHT) * (100.0 + pos))/100.0;
		height += LISTWIDGET_IMAGE_MIN_HEIGHT;
		height = floor(height);
	}
	else
	{
		height = ((LISTWIDGET_IMAGE_MAX_HEIGHT - LISTWIDGET_IMAGE_DEFAULT_HEIGHT) * pos)/100.0;
		height += LISTWIDGET_IMAGE_DEFAULT_HEIGHT;
		height = ceil(height);

	}

	width = (height * LISTWIDGET_IMAGE_DEFAULT_WIDTH)/ LISTWIDGET_IMAGE_DEFAULT_HEIGHT;

	cx = (int)(width + 0.5);
	cy = (int)(height + 0.5);

	if (self->imageSize.cx == cx &&
		self->imageSize.cy == cy)
	{
		return;
	}

	ListWidget_SetImageSize(self, hwnd, cx, cy, TRUE);
	
//	aTRACE_FMT("zoom changing: pos = %d, width = %d, height = %d\r\n", zoomInfo->dwPos, cx, cy);
}

static void
ListWidget_ScrollCb(ListWidget *self, HWND hwnd, int *dx, int *dy)
{	
	if (NULL != self->titleEditor)
	{
		DestroyWindow(self->titleEditor);
		self->titleEditor = NULL;
	}

	if (FALSE != ListWidget_UpdateHover(self, hwnd))
		UpdateWindow(hwnd);
	
	
	ListWidget_TooltipHide(self->tooltip);
}


static BOOL
ListWidget_NotifyCb(ListWidget *self, HWND hwnd, NMHDR *pnmh, LRESULT *result)
{
	if (FALSE != ListWidget_TooltipProcessNotification(self, self->tooltip, pnmh, result))
		return TRUE;

	return FALSE;
}
HWND 
ListWidget_CreateWindow(HWND parentWindow, int x, int y, int width, int height, BOOL border, unsigned int controlId)
{
	const static WidgetInterface widgetInterface =
	{
		(WidgetInitCallback)ListWidget_InitCb,
		(WidgetDestroyCallback)ListWidget_DestroyCb,
		(WidgetLayoutCallback)ListWidget_LayoutCb,
		(WidgetPaintCallback)ListWidget_PaintCb,
		(WidgetStyleCallback)ListWidget_StyleColorChangedCb,
		(WidgetStyleCallback)ListWidget_StyleFontChangedCb,
		(WidgetMouseCallback)ListWidget_MouseMoveCb,
		(WidgetMouseCallback)ListWidget_LeftButtonDownCb,
		(WidgetMouseCallback)ListWidget_LeftButtonUpCb,
		(WidgetMouseCallback)ListWidget_LeftButtonDblClkCb,
		(WidgetMouseCallback)ListWidget_RightButtonDownCb,
		(WidgetMouseCallback)ListWidget_RightButtonUpCb,
		(WidgetKeyCallback)ListWidget_KeyDownCb,
		(WidgetKeyCallback)ListWidget_KeyUpCb,
		(WidgetKeyCallback)ListWidget_CharacterCb,
		(WidgetInputCallback)ListWidget_InputRequestCb,
		(WidgetFocusCallback)ListWidget_FocusChangedCb,
		(WidgetMenuCallback)ListWidget_ContextMenuCb,
		(WidgetZoomCallback)ListWidget_ZoomChangingCb,
		(WidgetScrollCallback)NULL, /*scrollBefore*/
		(WidgetScrollCallback)ListWidget_ScrollCb,
		(WidgetNotifyCallback)ListWidget_NotifyCb,
	};

	return Widget_CreateWindow(WIDGET_TYPE_LIST,
								&widgetInterface, 
								NULL,
								(FALSE != border) ? WS_EX_CLIENTEDGE : 0, 
								WS_TABSTOP, 
								x, y, width, height, 
								parentWindow, 
								controlId, 0L);
}