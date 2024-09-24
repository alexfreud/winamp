#include <windows.h>
#include "ml.h"
#include "../nu/CGlobalAtom.h"

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

static CGlobalAtom PROP_FLIKERFIX(L"WAFFIX");

typedef struct _FFDATA
{
	WNDPROC oldProc;
	DWORD	mode;
	BOOL	unicode;
	BOOL	forward;
}FFDATA, *PFFDATA;

static LRESULT WINAPI FlickerFixWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	PFFDATA pfd;
	pfd = (PFFDATA)GetPropW(hwnd, PROP_FLIKERFIX);
	if (!pfd) return (IsWindowUnicode(hwnd)) ? DefWindowProcW(hwnd, uMsg, wParam, lParam) : DefWindowProcA(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
			
		case WM_ERASEBKGND: 	
			if (pfd->forward) break;
			
			if (FFM_NOERASE & pfd->mode) return 1;
			if (FFM_ERASEINPAINT & pfd->mode) return 0;
			if (FFM_FORCEERASE & pfd->mode) 
			{
				HBRUSH hb;
				RECT rc;
				hb = CreateSolidBrush(pfd->mode & 0x00FFFFFF);
				GetClientRect(hwnd, &rc);
				FillRect((HDC)wParam, &rc, hb);
				DeleteObject(hb);
				return 1;
			}
			// otherwise unsubclass....
		case WM_NCDESTROY:
			
			if (FlickerFixWndProc == (WNDPROC)(LONG_PTR)GetWindowLongPtrW(hwnd, GWLP_WNDPROC))
			{
				RemovePropW(hwnd, PROP_FLIKERFIX);
				SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pfd->oldProc);
				(pfd->unicode) ? CallWindowProcW(pfd->oldProc, hwnd, uMsg, wParam, lParam) : 
								CallWindowProcA(pfd->oldProc, hwnd, uMsg, wParam, lParam);

				free(pfd);
				return 0;
			}
			else pfd->forward = TRUE; 

			break;
			
	}
	
	return (pfd->unicode) ? CallWindowProcW(pfd->oldProc, hwnd, uMsg, wParam, lParam) : 
									CallWindowProcA(pfd->oldProc, hwnd, uMsg, wParam, lParam);
}


BOOL FlickerFixWindow(HWND hwnd, INT mode)
{
	PFFDATA pfd;

	if (!hwnd || !IsWindow(hwnd))  return FALSE;
	pfd = (PFFDATA)GetPropW(hwnd, PROP_FLIKERFIX);
	if (pfd) pfd->mode = mode;
	else
	{
		pfd = (PFFDATA)calloc(1, sizeof(FFDATA));
		if (pfd)
		{
			pfd->forward = FALSE;
			pfd->mode = mode;
			pfd->unicode = IsWindowUnicode(hwnd);
			pfd->oldProc = (WNDPROC)(LONG_PTR) ((pfd->unicode) ? SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)FlickerFixWndProc) :
																 SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)FlickerFixWndProc));
			if (!pfd->oldProc || !SetPropW(hwnd, PROP_FLIKERFIX, pfd))
			{
				if (pfd->oldProc) 
				{
					((pfd->unicode) ? SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pfd->oldProc) :
														SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pfd->oldProc));
				}
				free(pfd);
				pfd = NULL;
			}
		}
	}
	return (NULL != pfd);
}