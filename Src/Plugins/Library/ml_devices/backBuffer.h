#ifndef _NULLSOFT_WINAMP_ML_DEVICES_BACK_BUFFER_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_BACK_BUFFER_HEADER

typedef struct BackBuffer
{
	HBITMAP bitmap;
	HWND	hwnd;
	HDC		hdc;
	HBITMAP previous;
} BackBuffer;

BOOL 
BackBuffer_Initialize(BackBuffer *self, HWND hwnd);

void 
BackBuffer_Uninitialize(BackBuffer *self);

BOOL 
BackBuffer_EnsureSize(BackBuffer *self, 
					  long width, 
					  long height);

BOOL 
BackBuffer_EnsureSizeEx(BackBuffer *self, 
					  long width, 
					  long height,
					  long allocWidth, 
					  long allocHeight);

HDC 
BackBuffer_GetDC(BackBuffer *self);

BOOL
BackBuffer_Copy(BackBuffer *self, 
				HDC hdc, 
				long x, 
				long y, 
				long width, 
				long height);

void 
BackBuffer_Reset(BackBuffer *self);

BOOL
BackBuffer_DrawText(BackBuffer *self, 
					HDC hdc, 
					const wchar_t *string, 
					int length, 
					RECT *rect, 
					unsigned int format);

BOOL
BackBuffer_DrawTextEx(BackBuffer *self, 
					HDC hdc, 
					const wchar_t *string, 
					int length, 
					RECT *rect, 
					unsigned int format,
					HFONT font, 
					COLORREF backColor, 
					COLORREF textColor, 
					int backMode);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_BACK_BUFFER_HEADER