#include "./animation.h"
#include "./common.h"

BOOL Animation_Initialize(ANIMATIONDATA *animation, UINT durationMs)
{
	if (NULL == animation)
		return FALSE;
	
	if (FALSE == QueryPerformanceFrequency(&animation->frequency))
		return FALSE;

	QueryPerformanceCounter(&animation->completion);
	animation->completion.QuadPart += animation->frequency.QuadPart*durationMs/1000LL;
	return TRUE;
}

BOOL Animation_BeginStep(ANIMATIONDATA *animation)
{
	if (NULL == animation || FALSE == QueryPerformanceCounter(&animation->stepBegin))
		return FALSE;

	return TRUE;
}

BOOL Animation_EndStep(ANIMATIONDATA *animation, size_t stepsRemaining)
{
	if (NULL == animation || FALSE == QueryPerformanceCounter(&animation->stepEnd))
		return FALSE;
	
	if (0 == stepsRemaining || animation->stepEnd.QuadPart >= animation->completion.QuadPart)
		return TRUE;
			
	LARGE_INTEGER sleep;
	sleep.QuadPart = (animation->completion.QuadPart - animation->stepEnd.QuadPart) -
						(stepsRemaining * (animation->stepEnd.QuadPart - animation->stepBegin.QuadPart));
	
	if (stepsRemaining > 1)
		sleep.QuadPart /= (stepsRemaining -1);

	if (sleep.QuadPart <= 0)
		return TRUE;
							
	sleep.QuadPart += animation->stepEnd.QuadPart;
	do
	{
		SleepEx(0, FALSE);
		QueryPerformanceCounter(&animation->stepEnd);
	} while(sleep.QuadPart > animation->stepEnd.QuadPart);
								
	return TRUE;
}

BOOL Animation_SetWindowPos(HWND hwnd, INT x, INT y, INT cx, INT cy, UINT flags, HDC hdc, INT contextX, INT contextY)
{
	if (NULL == hwnd || 
		FALSE == SetWindowPos(hwnd, NULL, x, y, cx, cy, 
					flags | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS))
	{
		return FALSE;
	}
	
	UINT windowStyle = GetWindowStyle(hwnd);
	
	POINT origPoint;
	SetViewportOrgEx(hdc, contextX, contextY, &origPoint);
	if (0 == (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);

	if (FALSE == LoginBox_PrintWindow(hwnd, hdc, 0))
		SendMessage(hwnd, WM_PRINT, (WPARAM)hdc, (LPARAM)(PRF_CLIENT | PRF_ERASEBKGND | PRF_CHILDREN | PRF_NONCLIENT));
							
	if (0 == (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

	SetViewportOrgEx(hdc, origPoint.x, origPoint.y, NULL);
	return TRUE;
}