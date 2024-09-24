#if 0
#include <windows.h>
#include "../Winamp/wa_ipc.h"
#include "Main.h"
#include <shlwapi.h>

static WNDPROC waProc=0;
static bool winampisUnicode=false;

static LRESULT WINAPI StatusHookProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	if (msg == WM_WA_IPC && lParam == IPC_HOOK_TITLESW)
	{
		LRESULT downTheLine = winampisUnicode?CallWindowProcW(waProc, hwnd, msg, wParam, lParam):CallWindowProcA(waProc, hwnd, msg, wParam, lParam);
		waHookTitleStructW *hook = (waHookTitleStructW *)wParam;
		if (!PathIsURLW(hook->filename) && winamp.GetStatusHook(hook->title, 2048, hook->filename))
		{
			return TRUE;
		}
		else
			return downTheLine;
	}

	if (waProc)
	{
		if (winampisUnicode)
			return CallWindowProcW(waProc, hwnd, msg, wParam, lParam);
		else 
			return CallWindowProcA(waProc, hwnd, msg, wParam, lParam);
	}
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Hook(HWND winamp)
{
	if (winamp)
	{
		winampisUnicode = !!IsWindowUnicode(winamp);
		if (winampisUnicode) 
			waProc = (WNDPROC)SetWindowLongPtrW(winamp, GWLP_WNDPROC, (LONG_PTR)StatusHookProc);
		else 
			waProc = (WNDPROC)SetWindowLongPtrA(winamp, GWLP_WNDPROC, (LONG_PTR)StatusHookProc);
	}
}

void Unhook(HWND winamp)
{
//	if (winamp && GetWindowLongA(winamp,GWL_WNDPROC) == (LONG)StatusHookProc)
		//SetWindowLong(winamp, GWL_WNDPROC, (LONG)waProc);
	//waProc=0;
}
#endif