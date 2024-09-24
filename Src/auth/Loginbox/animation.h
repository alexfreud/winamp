#ifndef NULLSOFT_AUTH_LOGINBOX_ANIMATION_HEADER
#define NULLSOFT_AUTH_LOGINBOX_ANIMATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef struct __ANIMATIONDATA
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER completion;
	LARGE_INTEGER stepBegin;
	LARGE_INTEGER stepEnd;
} ANIMATIONDATA;

BOOL Animation_Initialize(ANIMATIONDATA *animation, UINT durationMs);
BOOL Animation_BeginStep(ANIMATIONDATA *animation);
BOOL Animation_EndStep(ANIMATIONDATA *animation, size_t stepsRemaining);
BOOL Animation_SetWindowPos(HWND hwnd, INT x, INT y, INT cx, INT cy, UINT flags, HDC hdc, INT contextX, INT contextY);

#endif //NULLSOFT_AUTH_LOGINBOX_ANIMATION_HEADER