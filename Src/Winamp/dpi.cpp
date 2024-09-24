#include "main.h"
#include "dpi.h"

// DPI awareness based on http://msdn.microsoft.com/en-US/library/dd464660.aspx
// Definition: relative pixel = 1 pixel at 96 DPI and scaled based on actual DPI.

BOOL _fInitialized = FALSE;
int _dpiX = 96, _dpiY = 96;

void _Init()
{
    if (!_fInitialized)
    {
        HDC hdc = GetDC(NULL);
        if (hdc)
        {
            _dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            _dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
        }
        _fInitialized = TRUE;
    }
}

// Get screen DPI.
int GetDPIX()
{
	_Init();
	return _dpiX;
}

int GetDPIY()
{
	_Init();
	return _dpiY;
}

// Convert between raw pixels and relative pixels.
int ScaleX(int x)
{
	_Init();
	return MulDiv(x, _dpiX, 96);
}

int ScaleY(int y)
{
	_Init();
	return MulDiv(y, _dpiY, 96);
}

int UnscaleX(int x)
{
	_Init();
	return MulDiv(x, 96, _dpiX);
}

int UnscaleY(int y)
{
	_Init();
	return MulDiv(y, 96, _dpiY);
}

int _ScaledSystemMetricX(int nIndex)
{
    _Init();
    return MulDiv(GetSystemMetrics(nIndex), 96, _dpiX);
}

int _ScaledSystemMetricY(int nIndex)
{
    _Init();
    return MulDiv(GetSystemMetrics(nIndex), 96, _dpiY);
}

// Determine the screen dimensions in relative pixels.
int ScaledScreenWidth()
{
	return _ScaledSystemMetricX(SM_CXSCREEN);
}

int ScaledScreenHeight()
{
	return _ScaledSystemMetricY(SM_CYSCREEN);
}

// Scale rectangle from raw pixels to relative pixels.
void ScaleRect(__inout RECT *pRect)
{
    pRect->left = ScaleX(pRect->left);
    pRect->right = ScaleX(pRect->right);
    pRect->top = ScaleY(pRect->top);
    pRect->bottom = ScaleY(pRect->bottom);
}

// Determine if screen resolution meets minimum requirements in relative pixels.
BOOL IsResolutionAtLeast(int cxMin, int cyMin)
{
    return (ScaledScreenWidth() >= cxMin) && (ScaledScreenHeight() >= cyMin);
}

// Convert a point size (1/72 of an inch) to raw pixels.
int PointsToPixels(int pt)
{
	_Init();
	return MulDiv(pt, _dpiY, 72);
}