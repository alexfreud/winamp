#include "main.h"
#include "./backBuffer.h"


BOOL 
BackBuffer_Initialize(BackBuffer *self, HWND hwnd)
{
	if (NULL == self)
		return FALSE;

	ZeroMemory(self, sizeof(BackBuffer));

	self->hwnd = hwnd;

	return TRUE;
}

void 
BackBuffer_Uninitialize(BackBuffer *self)
{
	BackBuffer_Reset(self);
}

void 
BackBuffer_Reset(BackBuffer *self)
{
	if (NULL == self)
		return;

	if (NULL != self->hdc)
	{
		if (NULL != self->previous)
			SelectBitmap(self->hdc, self->previous);
		
		DeleteDC(self->hdc);
	}
	
	if (NULL != self->bitmap)
	{
		DeleteObject(self->bitmap);
	}

	ZeroMemory(self, sizeof(BackBuffer));
}

BOOL 
BackBuffer_EnsureSize(BackBuffer *self, long width, long height)
{
	return BackBuffer_EnsureSizeEx(self, width, height, width, height);
}

BOOL 
BackBuffer_EnsureSizeEx(BackBuffer *self, long width, long height, long allocWidth, long allocHeight)
{
	BOOL result;
	HDC windowDC;
	HBITMAP bitmap;
	long bitmapWidth, bitmapHeight;

	if (NULL == self)
		return FALSE;

	if (width < 0)
		width = 0;

	if (height < 0)
		height = 0;
	
	if (NULL != self->bitmap)
	{
		BITMAP bitmapInfo;
		if (sizeof(bitmapInfo) == GetObject(self->bitmap, sizeof(bitmapInfo), &bitmapInfo))
		{
			if (bitmapInfo.bmWidth >= width && bitmapInfo.bmHeight >= height)
				return TRUE;

			bitmapWidth = bitmapInfo.bmWidth;
			bitmapHeight = bitmapInfo.bmHeight;
		}
		else
		{
			bitmapWidth = 0;
			bitmapHeight = 0;
		}
	}
	else
	{
		bitmapWidth = 0;
		bitmapHeight = 0;
	}
			
	result = FALSE;
	bitmap = NULL;

	windowDC = GetDCEx(self->hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if(NULL != windowDC)
	{
		if (allocWidth < width)
			allocWidth = width;

		if (allocWidth < bitmapWidth)
			allocWidth = bitmapWidth;

		if (allocHeight < height)
			allocHeight = height;
		
		if (allocHeight < bitmapHeight)
			allocHeight = bitmapHeight;

		bitmap = CreateCompatibleBitmap(windowDC, allocWidth, allocHeight);
		ReleaseDC(self->hwnd, windowDC);
	}

	if (NULL != bitmap)
	{
		if (NULL != self->hdc)
			SelectBitmap(self->hdc, bitmap);
	
		if (NULL != self->bitmap)
			DeleteObject(self->bitmap);

		self->bitmap = bitmap;
		result = TRUE;
	}

	return result;
}

HDC 
BackBuffer_GetDC(BackBuffer *self)
{
	if (NULL == self)
		return FALSE;

	if (NULL == self->hdc)
	{
		HDC windowDC;
		windowDC = GetDCEx(self->hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != windowDC)
		{
			self->hdc = CreateCompatibleDC(windowDC);
			ReleaseDC(self->hwnd, windowDC);
			
			if (NULL != self->hdc)
			{
				if (NULL != self->bitmap)
					self->previous = SelectBitmap(self->hdc, self->bitmap);
				else
					self->previous = GetCurrentBitmap(self->hdc);
			}
		}
	}

	return self->hdc;
}

BOOL
BackBuffer_Copy(BackBuffer *self, HDC hdc, long x, long y, long width, long height)
{
	HDC sourceDC;

	if (NULL == self || NULL == self->bitmap)
		return FALSE;

	sourceDC = BackBuffer_GetDC(self);
	if (NULL == sourceDC)
		return FALSE;

	return BitBlt(hdc, x, y, width, height, sourceDC, 0, 0, SRCCOPY);
}

BOOL
BackBuffer_DrawTextEx(BackBuffer *self, HDC hdc, const wchar_t *string, 
					int length, RECT *rect, unsigned int format,
					HFONT font, COLORREF backColor, COLORREF textColor, int backMode)
{
	BOOL result = FALSE;
	RECT bufferRect;

	if (NULL == hdc || NULL == rect)
		return FALSE;

	SetRect(&bufferRect, 0, 0, RECTWIDTH(*rect), RECTHEIGHT(*rect));

	if (NULL != self &&
		FALSE != BackBuffer_EnsureSize(self, bufferRect.right, bufferRect.bottom))
	{
		HDC bufferDC = BackBuffer_GetDC(self);
		if (NULL != bufferDC)
		{
			HFONT prevFont;
			prevFont = SelectFont(bufferDC, font);
			SetTextColor(bufferDC, textColor);
			SetBkColor(bufferDC, backColor);
			SetBkMode(bufferDC, backMode);
			
			if (OPAQUE == backMode)
				ExtTextOut(bufferDC, 0, 0, ETO_OPAQUE, &bufferRect, NULL, 0, NULL);
			
			if (FALSE != DrawText(bufferDC, string, length, &bufferRect, format))
			{
				result = BackBuffer_Copy(self, hdc, rect->left, rect->top, 
								bufferRect.right, bufferRect.bottom);
			}

			SelectFont(bufferDC, prevFont);
		}
	}

	
	if (FALSE == result)
	{
		HFONT prevFont;
		COLORREF  prevBackColor, prevTextColor;
		int prevBkMode;

		prevFont = SelectFont(hdc, font);
		prevTextColor = SetTextColor(hdc, textColor);
		prevBackColor = SetBkColor(hdc, backColor);
		prevBkMode= SetBkMode(hdc, backMode);

		result = DrawText(hdc, string, length, rect, format);

		SelectFont(hdc, prevFont);
		SetTextColor(hdc, prevTextColor);
		SetBkColor(hdc, prevBackColor);
		SetBkMode(hdc, prevBkMode);
	}
	
	return result;
}

BOOL
BackBuffer_DrawText(BackBuffer *self, HDC hdc, const wchar_t *string, 
					int length, RECT *rect, unsigned int format)
{
	BOOL result = FALSE;
	RECT bufferRect;

	if (NULL == hdc || NULL == rect)
		return FALSE;

	SetRect(&bufferRect, 0, 0, RECTWIDTH(*rect), RECTHEIGHT(*rect));

	if (NULL != self &&
		FALSE != BackBuffer_EnsureSize(self, bufferRect.right, bufferRect.bottom))
	{
		HDC bufferDC = BackBuffer_GetDC(self);
		if (NULL != bufferDC)
		{
			HFONT prevFont;
			int backMode;
			prevFont = SelectFont(bufferDC, GetCurrentFont(hdc));
			SetTextColor(bufferDC, GetTextColor(hdc));
			SetBkColor(bufferDC, GetBkColor(hdc));
			backMode = GetBkMode(hdc);
			SetBkMode(bufferDC, backMode);
			
			if (OPAQUE == backMode)
				ExtTextOut(bufferDC, 0, 0, ETO_OPAQUE, &bufferRect, NULL, 0, NULL);

			if (FALSE != DrawText(bufferDC, string, length, &bufferRect, format))
			{
				result = BackBuffer_Copy(self, hdc, rect->left, rect->top, 
								bufferRect.right, bufferRect.bottom);
			}

			SelectFont(bufferDC, prevFont);
		}
	}

	if (FALSE == result)
		result = DrawText(hdc, string, length, rect, format);

	return result;
}
