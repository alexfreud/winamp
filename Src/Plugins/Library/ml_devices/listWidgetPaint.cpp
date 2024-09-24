#include "main.h"
#include "./listWidgetInternal.h"

#include <strsafe.h>

static BOOL
ListWidgetPaintSpacebar_Initialize(ListWidgetPaintSpacebar *self, ListWidget *widget, 
								   WidgetStyle *style, HWND hwnd, long width, long height)
{
	if (NULL == self)
		return FALSE;

	self->bitmap = ListWidget_GetSpacebarBitmap(widget, style, hwnd, width, height);
	if (NULL == self->bitmap)
	{
		self->width = 0;
		self->height = 0;
		self->emptyBarOffset = 0;
		self->filledBarOffset = 0;
		return FALSE;
	}
		
	self->width = width;
	self->height = height;
	self->emptyBarOffset = 0;
	self->filledBarOffset = height;
	return TRUE;
}

static void
ListWidgetPaintSpacebar_Uninitialize(ListWidgetPaintSpacebar *self)
{
	if (NULL == self)
		return;
}

static BOOL
ListWidgetPaintArrow_Initialize(ListWidgetPaintArrow *self, ListWidget *widget, 
								WidgetStyle *style, HWND hwnd)
{
	BITMAP bitmapInfo;

	if (NULL == self)
		return FALSE;

	self->bitmap = ListWidget_GetArrowsBitmap(widget, style, hwnd);
	if (NULL == self->bitmap || 
		sizeof(bitmapInfo) != GetObject(self->bitmap, sizeof(bitmapInfo), &bitmapInfo))
	{
		ZeroMemory(self, sizeof(ListWidgetPaintArrow));
		return FALSE;
	}

	self->width = bitmapInfo.bmWidth;
	self->height = bitmapInfo.bmHeight/2;
	if (self->height < 0)
		self->height = -self->height;

	self->collapsedOffset = 0;
	self->expandedOffset = self->height;
	
	return TRUE;
}

static void
ListWidgetPaintArrow_Uninitialize(ListWidgetPaintArrow *self)
{
	if (NULL == self)
		return;
}

BOOL 
ListWidgetPaint_Initialize(ListWidgetPaint *self, ListWidget *widget, WidgetStyle *style, 
						   HWND hwnd, HDC hdc, const RECT *paintRect, BOOL erase)
{
	HDC windowDC;

	if (NULL == self)
		return FALSE;

	if (NULL == widget || NULL == style)
		return FALSE;

	self->widget = widget;
	self->style = style;
	self->hwnd = hwnd;
	self->hdc = hdc;
	self->erase = erase;
	self->paintRect = paintRect;
	
	windowDC = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != windowDC)
	{
		self->sourceDC = CreateCompatibleDC(windowDC);
		ReleaseDC(hwnd, windowDC);
	}
	
	if (NULL == self->sourceDC)
		return FALSE;
	
	if (FALSE == ListWidget_GetItemMetrics(style, &self->itemMetrics))
		return FALSE;

	if (FALSE == ListWidget_GetCategoryMetrics(style, &self->categoryMetrics))
		return FALSE;

	if (FALSE == ListWidgetPaintSpacebar_Initialize(&self->spacebar, widget, style, hwnd, 
					widget->itemWidth - (self->itemMetrics.offsetLeft + self->itemMetrics.offsetRight),
					self->itemMetrics.spacebarHeight))
	{
	//	ListWidgetPaint_Uninitialize(self);
	//	return FALSE;
	}


	if (FALSE == ListWidgetPaintArrow_Initialize(&self->arrow, widget, style, hwnd))
	{
	//	ListWidgetPaint_Uninitialize(self);
	//	return FALSE;
	}
	
	return TRUE;
}

void
ListWidgetPaint_Uninitialize(ListWidgetPaint *self)
{
	if (NULL == self)
		return;

	if (NULL != self->sourceDC)
		DeleteDC(self->sourceDC);

	ListWidgetPaintSpacebar_Uninitialize(&self->spacebar);
	ListWidgetPaintArrow_Uninitialize(&self->arrow);
}


static BOOL
ListWidgetPaint_DrawSpacebar(ListWidgetPaint *self, HDC hdc, int x, int y,
							  uint64_t totalSpace, uint64_t usedSpace)
{
	RECT *partRect, sourceRect;
	BOOL succeeded;
	long usedWidth;
	ListWidgetPaintSpacebar *spacebar;
		
	if (NULL == self)
		return FALSE;

	partRect = &self->partRect;
	spacebar = &self->spacebar;
	if (NULL == spacebar->bitmap)
		return FALSE;
	
	succeeded = TRUE;

	if (usedSpace > totalSpace)
		usedSpace = totalSpace;
	
	if (0 == totalSpace)
		usedWidth = 0;
	else
		usedWidth = (long)(spacebar->width * (double)usedSpace/totalSpace);
	
	
	SetRect(partRect, x, y, x + spacebar->width, y + spacebar->height);
	SetRect(&sourceRect, 0, 0, spacebar->width, spacebar->height);
	OffsetRect(&sourceRect, 0, spacebar->emptyBarOffset);
	
	if (FALSE == Image_AlphaBlend(hdc, partRect, self->sourceDC, &sourceRect, 255, 
										spacebar->bitmap, self->paintRect, AlphaBlend_Normal, NULL))
	{
			succeeded = FALSE;
	}
	
	if (0 != usedWidth)
	{
		SetRect(partRect, x, y, x + usedWidth, y + spacebar->height);
		SetRect(&sourceRect, 0, 0, usedWidth, spacebar->height);
		OffsetRect(&sourceRect, 0, spacebar->filledBarOffset);

		if (FALSE == Image_AlphaBlend(hdc, partRect, self->sourceDC, &sourceRect, 255, 
										spacebar->bitmap, self->paintRect, AlphaBlend_Normal, NULL))
		{
			succeeded = FALSE;
		}
	}

	return succeeded;
}

void
ListWidgetPaint_DrawItemAction(ListWidgetPaint *self, HDC hdc, ListWidgetItem *item, ListWidgetActivity *activity)
{
	HDC sourceDC;
	WidgetStyle *style;
	ListWidget *widget;
	ListWidgetItemMetric *metrics;
	RECT *partRect, sourceRect;
	HBITMAP bitmap, prevSourceBitmap;
	COLORREF prevTextColor;
	HFONT prevFont;
	int stringLength;
	
	if (NULL == activity ||
		FALSE == ListWidget_GetItemActivityRect(self->widget, item, &self->itemMetrics, &self->partRect) ||
		FALSE == IntersectRect(&sourceRect, &self->partRect, self->paintRect))
	{
		return;
	}

	
	style = self->style;
	widget = self->widget;
	metrics = &self->itemMetrics;
	sourceDC = self->sourceDC;
	partRect = &self->partRect;
	prevSourceBitmap = GetCurrentBitmap(sourceDC);
	prevTextColor = GetTextColor(hdc);
	prevFont = GetCurrentFont(hdc);

	if (NULL != widget->activityFont)
		SelectFont(hdc, widget->activityFont);

	bitmap = ListWidget_GetActivityBadgeBitmap(widget, style, self->hwnd, 
							RECTWIDTH(*partRect), RECTHEIGHT(*partRect));
	if (NULL != bitmap)
	{		
		Image_AlphaBlend(hdc, partRect, sourceDC, NULL, 255, bitmap, self->paintRect, 
								AlphaBlend_Normal, NULL);
	}


	if (FALSE == IS_STRING_EMPTY(activity->title) &&
		FALSE != ListWidget_GetItemActivityTitleRect(widget, hdc, item, metrics, partRect) &&
		FALSE != IntersectRect(&sourceRect, partRect, self->paintRect))
	{
		stringLength = lstrlen(activity->title);
	
			OffsetRect(partRect, 0, 1);
		SetTextColor(hdc, 0x000000);
		DrawText(hdc, activity->title, stringLength, partRect, 
			DT_LEFT | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL | DT_WORD_ELLIPSIS);

		OffsetRect(partRect, 0, -1);
		SetTextColor(hdc, 0xFFFFFF);
		DrawText(hdc, activity->title, stringLength, partRect, 
			DT_LEFT | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL | DT_WORD_ELLIPSIS);
	
	}

	if (FALSE != ListWidget_GetItemActivityProgressRect(widget, hdc, item, metrics, partRect) &&
		FALSE != IntersectRect(&sourceRect, partRect, self->paintRect))
	{
		bitmap = ListWidget_GetActivityProgressBitmap(widget, style);
		if (NULL != bitmap)
		{		
			SetRect(&sourceRect, 0, 0, 
					widget->activityMetrics.progressWidth, widget->activityMetrics.progressHeight);
			OffsetRect(&sourceRect, 0, widget->activityMetrics.progressHeight * activity->step);

			Image_AlphaBlend(hdc, partRect, sourceDC, &sourceRect, 255, bitmap, self->paintRect, 
										AlphaBlend_Normal, NULL);
		}
	}

	if ((unsigned int)-1 != activity->percent &&
		FALSE != ListWidget_GetItemActivityPercentRect(widget, hdc, item, metrics, partRect) &&
		FALSE != IntersectRect(&sourceRect, partRect, self->paintRect))
	{
		wchar_t buffer[6] = {0};
		
		if (FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), L"%d%%", activity->percent)))
			stringLength = 0;
		else
			stringLength = lstrlen(buffer);
		
		OffsetRect(partRect, 0, 1);
		SetTextColor(hdc, 0x000000);
		DrawText(hdc, buffer, stringLength, partRect, 
			DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);

		OffsetRect(partRect, 0, -1);
		SetTextColor(hdc, 0xFFFFFF);
		DrawText(hdc, buffer, stringLength, partRect, 
			DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);
	}

	SetTextColor(hdc, prevTextColor);
	SelectFont(hdc, prevFont);
}

BOOL
ListWidgetPaint_DrawItem(ListWidgetPaint *self, ListWidgetItem *item)
{
	RECT frameRect;
	long frameHeight, frameWidth;
	HDC hdc, sourceDC, bufferDC, targetDC;
	WidgetStyle *style;
	ListWidget *widget;
	ListWidgetItemMetric *metrics;
	RECT *partRect, paintRect;
	HBITMAP bitmap, prevSourceBitmap;
	
	if (NULL == self || NULL == item)
		return FALSE;

	hdc = self->hdc;
	style = self->style;
	widget = self->widget;
	metrics = &self->itemMetrics;
	sourceDC = self->sourceDC;
	partRect = &self->partRect;
	prevSourceBitmap = GetCurrentBitmap(sourceDC);



	if (FALSE == ListWidget_GetItemFrameRect(widget, item, metrics, &frameRect))
		return FALSE;

	frameWidth = RECTWIDTH(frameRect);
	frameHeight = RECTHEIGHT(frameRect);

	if (FALSE == IntersectRect(&paintRect, &item->rect, self->paintRect))
		return TRUE;

	CopyRect(partRect, &paintRect);

	if (FALSE != BackBuffer_EnsureSizeEx(&widget->backBuffer, 
							RECTWIDTH(paintRect), RECTHEIGHT(paintRect), 
							RECTWIDTH(item->rect), RECTHEIGHT(item->rect)))
	{

		bufferDC = BackBuffer_GetDC(&widget->backBuffer);
		if (NULL != bufferDC)
		{
			SetViewportOrgEx(bufferDC, -paintRect.left, -paintRect.top, NULL);
			SetTextColor(bufferDC, GetTextColor(hdc));
			SetBkColor(bufferDC, GetBkColor(hdc));
			SetBkMode(bufferDC, GetBkMode(hdc));
			SelectFont(bufferDC, GetCurrentFont(hdc));

			targetDC = hdc;
			hdc = bufferDC;
		}
	}
	else
		bufferDC = NULL;
	

	if (FALSE != self->erase)
	{	
		FillRect(hdc, partRect, style->backBrush);
	}
		

	if (FALSE != ListWidgetItem_IsHovered(item))
	{			
		bitmap = ListWidget_GetHoverBitmap(widget, style, self->hwnd, 
												frameWidth, frameHeight);
		
		Image_AlphaBlend(hdc, &frameRect, sourceDC, NULL, 255, bitmap, &paintRect, 
							AlphaBlend_Normal, NULL);
	}

	if (FALSE != ListWidgetItem_IsSelected(item))
	{
		bitmap = (GetFocus() == self->hwnd) ?
					ListWidget_GetSelectBitmap(widget, style, self->hwnd, frameWidth, frameHeight) :
					ListWidget_GetInactiveSelectBitmap(widget, style, self->hwnd, frameWidth, frameHeight);

		Image_AlphaBlend(hdc, &frameRect, sourceDC, NULL, 255, bitmap, &paintRect, 
							AlphaBlend_Normal, NULL);
	}

		
	bitmap = ListWidget_GetItemImage(widget, style, item);
	if (NULL != bitmap)
	{
		partRect->left =  frameRect.left + metrics->offsetLeft + metrics->imageOffsetLeft;
		partRect->top = frameRect.top + metrics->offsetTop + metrics->imageOffsetTop;
		partRect->right = frameRect.right - (metrics->offsetRight + metrics->imageOffsetRight);
		partRect->bottom = frameRect.bottom - metrics->imageOffsetBottom;

		Image_AlphaBlend(hdc, partRect, sourceDC, NULL, 255, bitmap, &paintRect, 
							AlphaBlend_ScaleSource | AlphaBlend_AlignBottom, NULL);
	}

	bitmap = ListWidget_GetConnectionImage(style, item->connection, 
						widget->connectionSize.cx, widget->connectionSize.cy);
	if (NULL != bitmap && 
		FALSE != ListWidget_GetItemConnectionRect(widget, item, metrics, partRect))
	{		
		
		Image_AlphaBlend(hdc, partRect, sourceDC, NULL, 255, bitmap, &paintRect, 
						AlphaBlend_AlignCenter | AlphaBlend_AlignBottom, NULL);
	}
		
	ListWidgetPaint_DrawItemAction(self, hdc, item, item->activity);
	
	if (FALSE != ListWidgetItem_IsInteractive(item) && 0 != widget->commandsCount)
	{
		
		HBITMAP commandBitmap;
		size_t index;
		RECT commandPaintRect;
					
		bitmap = NULL;
		for(index = 0; index < widget->commandsCount; index++)
		{				
			long offset;
			BYTE sourceAlpha;
			ListWidget_GetCommandRect(widget->commands[index], partRect);
			OffsetRect(partRect, frameRect.left, frameRect.top);

			if (FALSE == IntersectRect(&commandPaintRect, partRect, &paintRect))
				continue;
				
			if (FALSE != ListWidget_GetCommandPressed(widget->commands[index]))
				sourceAlpha = 200;
			else if (FALSE != ListWidget_GetCommandDisabled(widget->commands[index]))
				sourceAlpha = 32;
			else 
				sourceAlpha = 255;

			if (NULL == item->activity && 
				FALSE != ListWidget_GetCommandPrimary(widget->commands[index]))
			{
				bitmap =  ListWidget_GetLargeBadgeBitmap(widget, style, self->hwnd, 
									RECTWIDTH(*partRect), RECTHEIGHT(*partRect));

				Image_AlphaBlend(hdc, partRect, sourceDC, NULL, 255, bitmap, &commandPaintRect, 
								AlphaBlend_AlignCenter, NULL);

				bitmap = NULL;

				offset = widget->primaryCommandSize.cy/6;
				if (offset < 3) 
					offset = 3;

				InflateRect(partRect, -offset, -offset);
				commandBitmap = ListWidget_GetCommandLargeBitmap(style, widget->commands[index], 
									RECTWIDTH(*partRect), RECTHEIGHT(*partRect));
				if (NULL == commandBitmap)
					commandBitmap = ListWidget_GetUnknownCommandLargeBitmap(widget, style,
										RECTWIDTH(*partRect), RECTHEIGHT(*partRect));
			}
			else 
			{
				if (NULL == bitmap)
				{
					bitmap = ListWidget_GetSmallBadgeBitmap(widget, style, self->hwnd, 
							RECTWIDTH(*partRect), RECTHEIGHT(*partRect));
				}

				Image_AlphaBlend(hdc, partRect, sourceDC, NULL, 255, bitmap, &commandPaintRect, 
								AlphaBlend_AlignCenter, NULL);

				offset = widget->secondaryCommandSize.cy/6;
				if (offset < 3) 
					offset = 3;

				InflateRect(partRect, -offset, -offset);
				commandBitmap = ListWidget_GetCommandSmallBitmap(style, widget->commands[index], 
									RECTWIDTH(*partRect), RECTHEIGHT(*partRect));

				if (NULL == commandBitmap)
					commandBitmap = ListWidget_GetUnknownCommandSmallBitmap(widget, style,
										RECTWIDTH(*partRect), RECTHEIGHT(*partRect));
			}
			
			if (NULL != commandBitmap)
			{
				Image_AlphaBlend(hdc, partRect, sourceDC, NULL, sourceAlpha, commandBitmap, &commandPaintRect,
								AlphaBlend_Normal, NULL);
			}

		}

	}
	
	if (FALSE != ListWidget_GetItemSpacebarRect(widget, item, metrics, partRect))
	{
		ListWidgetPaint_DrawSpacebar(self, hdc, 
			partRect->left,
			partRect->top,
			item->spaceTotal, item->spaceUsed);
	}

	
	if (FALSE == ListWidgetItem_IsTextEdited(item) &&
		FALSE == IS_STRING_EMPTY(item->title) &&
		FALSE != ListWidget_GetItemTitleRect(widget, item, metrics, TRUE, partRect))
	{		
		DrawText(hdc, item->title, -1, partRect, 
			DT_CENTER | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL | DT_END_ELLIPSIS);
	}

	SelectBitmap(sourceDC, prevSourceBitmap);

	if (NULL != bufferDC)
	{
		hdc  = targetDC;

		SetViewportOrgEx(bufferDC, 0, 0, NULL);

		BackBuffer_Copy(&widget->backBuffer, hdc, 
			paintRect.left, paintRect.top, RECTWIDTH(paintRect), RECTHEIGHT(paintRect));
	}

	return TRUE;
}

BOOL
ListWidgetPaint_DrawCategory(ListWidgetPaint *self, ListWidgetCategory *category)
{
	HDC hdc;
	RECT elementRect, *partRect;
	WidgetStyle *style;
	ListWidgetCategoryMetric *metrics;

	if (NULL == self || NULL == category)
		return FALSE;

	hdc = self->hdc;
	style = self->style;
	partRect = &self->partRect;
	metrics = &self->categoryMetrics;

	if (FALSE == IntersectRect(partRect, &category->rect, self->paintRect))
		return TRUE;

	CopyRect(partRect, &category->rect);
	partRect->right -= metrics->offsetRight;
	if (FALSE != IntersectRect(partRect, partRect, self->paintRect))
		FillRect(hdc, partRect, WIDGETSTYLE_CATEGORY_BRUSH(style));

	partRect->left = category->rect.right - metrics->offsetRight;
	partRect->right = category->rect.right;
	if (FALSE != IntersectRect(partRect, partRect, self->paintRect))
		FillRect(hdc, partRect, WIDGETSTYLE_BACK_BRUSH(style));

	if (NULL != self->arrow.bitmap)
	{
		long limit;
		SetRect(&elementRect, 0, 0, self->arrow.width, self->arrow.height);
		OffsetRect(&elementRect, 
					0, 
					(FALSE == category->collapsed) ? 
						self->arrow.expandedOffset : 
						self->arrow.collapsedOffset);


		CopyRect(partRect, &category->rect);
		partRect->left += metrics->offsetLeft;
		partRect->right = partRect->left + self->arrow.width;
	
		limit = (RECTHEIGHT(category->rect) - self->arrow.height - metrics->offsetTop);
		partRect->top += metrics->offsetTop + limit/2 + limit%2;
		partRect->bottom = partRect->top + self->arrow.height;

		limit = category->rect.bottom - metrics->offsetBottom - metrics->lineHeight;
		if (partRect->bottom > limit)
			OffsetRect(partRect, 0, (limit - partRect->bottom));
		
		limit = category->rect.top + metrics->offsetTop;
		if (partRect->top < limit)
			OffsetRect(partRect, 0, (limit - partRect->top));
				
		Image_AlphaBlend(hdc, partRect, self->sourceDC, &elementRect, 
				255, self->arrow.bitmap, self->paintRect, 
				AlphaBlend_Normal, NULL);
	}
	
	CopyRect(&elementRect, &category->rect);
	elementRect.left += metrics->offsetLeft + metrics->iconWidth + metrics->titleOffsetLeft;
	elementRect.top += metrics->offsetTop;
	elementRect.right -= metrics->offsetRight;
	elementRect.bottom -= (metrics->offsetBottom + metrics->lineHeight + metrics->lineOffsetTop);

	if (FALSE != IntersectRect(partRect, &elementRect, self->paintRect))
	{
		COLORREF prevTextColor = SetTextColor(hdc, WIDGETSTYLE_CATEGORY_TEXT_COLOR(style));
		HFONT prevFont = SelectFont(hdc, WIDGETSTYLE_CATEGORY_FONT(style));

		if (NULL == category->countString)
		{
			size_t count, index;
			wchar_t buffer[64] = {0};

			count = 0;
			index = category->groups.size();
			while(index--)
			{
				count += category->groups[index]->items.size();
			}
			
			if (SUCCEEDED(StringCchPrintf(buffer, ARRAYSIZE(buffer), L" (%u)", count)))
				category->countString = String_Duplicate(buffer);
		}

		if (-1 == category->titleWidth)
		{
			category->titleWidth = 0;

			if (FALSE == IS_STRING_EMPTY(category->title))
			{
				SetRect(partRect, 0, 0, 0, RECTHEIGHT(elementRect));
				if (FALSE != DrawText(hdc, category->title, -1, partRect, 
							DT_CALCRECT | DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE))
				{
					category->titleWidth = RECTWIDTH(*partRect);
				}
			}
		}	
		
		if (-1 == category->countWidth)
		{
			category->countWidth = 0;
			if (FALSE == IS_STRING_EMPTY(category->countString))
			{
				SetRect(partRect, 0, 0, 0, RECTHEIGHT(elementRect));
				if (FALSE != DrawText(hdc, category->countString, -1, partRect, 
							DT_CALCRECT | DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE))
				{
					category->countWidth = RECTWIDTH(*partRect);
				}
			}
		}

		if (0 != category->titleWidth)
		{
			CopyRect(partRect, &elementRect);
		
			if (partRect->right < (partRect->left + category->titleWidth + category->countWidth))
				partRect->right = partRect->right - category->countWidth;
				
			if (partRect->right > partRect->left)
			{
				DrawText(hdc, category->title, -1, partRect, 
						DT_LEFT | DT_BOTTOM | DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
			}
		}

		if (0 != category->countWidth)
		{
			CopyRect(partRect, &elementRect);
			partRect->left += category->titleWidth;
			
			if (partRect->left > (partRect->right - category->countWidth))
			{
				partRect->left = partRect->right - category->countWidth;
				if (partRect->left < elementRect.left)
					partRect->left = elementRect.left;
			}

			if (partRect->right > partRect->left)
			{
				DrawText(hdc, category->countString, -1, partRect, 
						DT_LEFT | DT_BOTTOM | DT_NOPREFIX | DT_SINGLELINE);
			}
		}

		SetTextColor(hdc, prevTextColor);
		SelectFont(hdc, prevFont);
	}

	if (0 != metrics->lineHeight)
	{				
		CopyRect(partRect, &category->rect);
		partRect->right -= metrics->offsetRight;
		partRect->bottom -= metrics->offsetBottom;
		partRect->top = partRect->bottom - metrics->lineHeight;
	
		if (FALSE != IntersectRect(partRect, partRect, self->paintRect))
		{
			COLORREF prevBackColor;

			prevBackColor = SetBkColor(hdc, WIDGETSTYLE_CATEGORY_LINE_COLOR(style));
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, partRect, NULL, 0, NULL);

			SetBkColor(hdc, prevBackColor);
		}
	}

	return TRUE;
}

BOOL
ListWidgetPaint_DrawEmptyCategoryText(ListWidgetPaint *self, ListWidgetCategory *category)
{
	HDC hdc;
	WidgetStyle *style;
	RECT *partRect;
	BOOL result;
	COLORREF prevTextColor;

	if (NULL == self || NULL == category)
		return FALSE;

	hdc = self->hdc;
	style = self->style;
	partRect = &self->partRect;
	
	
	if (FALSE == IntersectRect(partRect, self->paintRect, &category->emptyTextRect))
		return TRUE;

	if (FALSE != self->erase)
	{	
		FillRect(hdc, partRect, style->backBrush);
	}

	if (FALSE != IS_STRING_EMPTY(category->emptyText))
		return TRUE;
	
	prevTextColor = SetTextColor(hdc, WIDGETSTYLE_CATEGORY_EMPTY_TEXT_COLOR(style));
	
	result = DrawText(hdc, category->emptyText, -1, &category->emptyTextRect, 
				DT_CENTER | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL);
	
	SetTextColor(hdc, prevTextColor);
	return result;
}