#include "main.h"
#include "./graphics.h"

BYTE 
Graphics_GetSysFontQuality()
{
	BOOL smoothingEnabled;
	if (FALSE == SystemParametersInfoW(SPI_GETFONTSMOOTHING, 0, &smoothingEnabled, 0) ||
		FALSE == smoothingEnabled)
	{
		return DEFAULT_QUALITY;
	}
    
    OSVERSIONINFOW vi;
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (FALSE == GetVersionExW(&vi)) 
		return DEFAULT_QUALITY;

	if (vi.dwMajorVersion > 5 || (vi.dwMajorVersion == 5 && vi.dwMinorVersion >= 1))
	{
		UINT smootingType;
	    if (FALSE == SystemParametersInfoW(SPI_GETFONTSMOOTHINGTYPE, 0, &smootingType, 0))
			return DEFAULT_QUALITY;
	    
	    if (FE_FONTSMOOTHINGCLEARTYPE == smootingType)
			return CLEARTYPE_NATURAL_QUALITY/*CLEARTYPE_QUALITY*/;
	}

	return ANTIALIASED_QUALITY;
}

HFONT 
Graphics_CreateSysFont()
{
	LOGFONTW lf;
	HFONT font;

	if (FALSE == SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0))
		return NULL;
	
	lf.lfQuality = Graphics_GetSysFontQuality();
	font = CreateFontIndirectW(&lf);
	
	return font;
}

HFONT 
Graphics_DuplicateFont(HFONT sourceFont, INT heightDeltaPt, BOOL forceBold, BOOL systemQuality)
{
	LOGFONTW lf;

	if (NULL == sourceFont) 
		return NULL;
	
	
	if (sizeof(lf) != GetObjectW(sourceFont, sizeof(lf), &lf))
		return NULL;

	if (0 != heightDeltaPt)
	{
		HDC hdc, hdcTmp;
		
		hdc = GetDCEx(NULL, NULL, DCX_WINDOW | DCX_CACHE | DCX_NORESETATTRS);
		hdcTmp = NULL;

		if (NULL != hdc)
		{
			hdcTmp = CreateCompatibleDC(hdc);
			ReleaseDC(NULL, hdc);
		}
		
		if (NULL == hdcTmp)
			return NULL;
		
		LONG pixelsY = GetDeviceCaps(hdcTmp, LOGPIXELSY);
		HFONT prevFont = SelectFont(hdcTmp, sourceFont);
		
		TEXTMETRICW tm;
		if (FALSE != GetTextMetricsW(hdcTmp, &tm))
		{
			INT basePt = MulDiv(tm.tmHeight - tm.tmInternalLeading, 72, pixelsY);
			lf.lfHeight = -MulDiv((basePt + heightDeltaPt), pixelsY, 72);

		}

		SelectObject(hdcTmp, prevFont);
		DeleteDC(hdcTmp);
	}

	if (FALSE != systemQuality)
		lf.lfQuality = Graphics_GetSysFontQuality();
	if (FALSE != forceBold && lf.lfWeight < FW_BOLD)
		lf.lfWeight = FW_BOLD;

	return CreateFontIndirectW(&lf);
}

long 
Graphics_GetFontHeight(HDC hdc)
{
	TEXTMETRICW tm;
	if (FALSE == GetTextMetricsW(hdc, &tm))
		return 0;

	return tm.tmHeight;
}

long
Graphics_GetAveStrWidth(HDC hdc, UINT cchLen)
{
	const char szTest[] = 
	{ 
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P', 'Q','R','S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l', 'm','n','o','p','q','r','s','t','u','v','w','x','y','z'
	};

	SIZE textSize;
	if (FALSE == GetTextExtentPointA(hdc, szTest, ARRAYSIZE(szTest) -1, &textSize))
		return 0;

	LONG result;
	if (1 == cchLen)
	{
		result = (textSize.cx + ARRAYSIZE(szTest)/2)/ARRAYSIZE(szTest);
	}
	else
	{
		result = MulDiv(cchLen, textSize.cx + ARRAYSIZE(szTest)/2, ARRAYSIZE(szTest));
		if (0 != result)
		{
			TEXTMETRICW tm;
			if (FALSE != GetTextMetricsW(hdc, &tm))
				result += tm.tmOverhang;
		}
	}
	return result;
}

BOOL 
Graphics_GetWindowBaseUnits(HWND hwnd, LONG *baseUnitX, LONG *baseUnitY)
{
	BOOL result;
	result = FALSE;

	if (NULL != hwnd)
	{		
		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			TEXTMETRICW tm;
			HFONT font, prevFont;
			
			font = (HFONT)SNDMSG(hwnd, WM_GETFONT, 0, 0L);
			prevFont = SelectFont(hdc, font);
			
			if (FALSE != GetTextMetricsW(hdc, &tm))
			{
				if (NULL != baseUnitX) 
					*baseUnitX = Graphics_GetAveStrWidth(hdc, 1);

				if (NULL != baseUnitY) 
					*baseUnitY = tm.tmHeight;

				result = TRUE;
			}

			SelectFont(hdc, prevFont);
			ReleaseDC(hwnd, hdc); 
		}
	}
	return result;
}


typedef int (*SkinColorFunc)(int idx);

COLORREF Graphics_GetSkinColor(unsigned int colorIndex)
{
	static SkinColorFunc GetSkinColor = NULL;
	
	if (NULL == GetSkinColor)
	{
		GetSkinColor = (SkinColorFunc)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SKIN_WADLG_GETFUNC, 1);
		if (NULL == GetSkinColor)
			return RGB(255, 0, 255);
	}

	return GetSkinColor(colorIndex);
}

COLORREF 
Graphics_BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha)
{
	if (alpha > 254) return rgbTop;
	if (alpha < 0) return rgbBottom;

	WORD k = (WORD)(((255 - alpha)*255 + 127)/255);
	
	return RGB( (GetRValue(rgbTop)*alpha + k*GetRValue(rgbBottom) + 127)/255, 
				(GetGValue(rgbTop)*alpha + k*GetGValue(rgbBottom) + 127)/255, 
				(GetBValue(rgbTop)*alpha + k*GetBValue(rgbBottom) + 127)/255);
}

INT 
Graphics_GetColorDistance(COLORREF rgb1, COLORREF rgb2)
{
	return (1000 * ((GetRValue(rgb1) - GetRValue(rgb2)) + 
			(GetGValue(rgb1) - GetGValue(rgb2)) +
			(GetBValue(rgb1) - GetBValue(rgb2))))/ (3 * 255);
}

void 
Graphics_ClampRect(RECT *rect, const RECT *boxRect)
{
	if (rect->left < boxRect->left)
		rect->left = boxRect->left;
	
	if (rect->top < boxRect->top)
		rect->top = boxRect->top;

	if (rect->right > boxRect->right)
		rect->right = boxRect->right;
	
	if (rect->bottom > boxRect->bottom)
		rect->bottom = boxRect->bottom;
}

void 
Graphics_NormalizeRect(RECT *rect)
{
	if (rect->top > rect->bottom)
		rect->bottom = rect->top;

	if (rect->left > rect->right)
		rect->right = rect->left;
}

void 
Graphics_GetRectSizeNormalized(const RECT *rect, SIZE *size)
{
	size->cx = rect->right - rect->left;
	if (size->cx < 0) 
		size->cx = 0;

	size->cy = rect->bottom - rect->top;
	if (size->cy < 0) 
		size->cy = 0;
}

BOOL 
Graphics_IsRectFit(const RECT *rect, const RECT *boxRect)
{
	if (rect->left < boxRect->left ||
		rect->top < boxRect->top ||
		rect->right > boxRect->right ||
		rect->bottom > boxRect->bottom)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL SetSizeEmpty(SIZE *size)
{
	if (NULL == size)
		return FALSE;

	ZeroMemory(size, sizeof(SIZE));
	return TRUE;
}

BOOL IsSizeEmpty(SIZE *size)
{
	return (NULL == size || 0 == size->cx  || 0 == size->cy);
}

BOOL SetSize(SIZE *size, long width, long height)
{
	if (NULL == size)
		return FALSE;

	size->cx = width;
	size->cy = height;

	return TRUE;
}

BOOL SetPoint(POINT *pt, long x, long y)
{
	if (NULL == pt)
		return FALSE;

	pt->x = x;
	pt->y = y;

	return TRUE;
}

BOOL MakeRectPolygon(POINT vertices[4], long left, long top, long right, long bottom)
{
	if (NULL == vertices)
		return FALSE;

	vertices[0].x = left;
	vertices[0].y = top;
	vertices[1].x = right;
	vertices[1].y = top;
	vertices[2].x = right;
	vertices[2].y = bottom;
	vertices[3].x = left;
	vertices[3].y = bottom;

	return TRUE;
}
