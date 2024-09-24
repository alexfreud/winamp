#pragma once
/*#ifndef _WA_DPI_H
#define _WA_DPI_H*/

#ifdef __cplusplus

extern "C"
{
#endif

#include <windows.h>

// DPI awareness based on http://msdn.microsoft.com/en-US/library/dd464660.aspx
// Definition: relative pixel = 1 pixel at 96 DPI and scaled based on actual DPI.

// Get screen DPI.
int GetDPIX();
int GetDPIY();

// Convert between raw pixels and relative pixels.
int ScaleX(int x);
int ScaleY(int y);
int UnscaleX(int x);
int UnscaleY(int y);

int _ScaledSystemMetricX(int nIndex);
int _ScaledSystemMetricY(int nIndex);

// Determine the screen dimensions in relative pixels.
int ScaledScreenWidth();
int ScaledScreenHeight();

// Scale rectangle from raw pixels to relative pixels.
void ScaleRect(__inout RECT *pRect);

// Determine if screen resolution meets minimum requirements in relative pixels.
BOOL IsResolutionAtLeast(int cxMin, int cyMin);

// Convert a point size (1/72 of an inch) to raw pixels.
int PointsToPixels(int pt);

#ifdef __cplusplus
} // extern "C"
#endif

//#endif