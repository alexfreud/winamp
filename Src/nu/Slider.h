#pragma once
#include <windows.h>
#include <commctrl.h>
class Slider
{
public:
	
	Slider(HWND hwndDlg, int id)
	{
		slider_hwnd = GetDlgItem(hwndDlg, id);
	}

	void SetRange(WORD low, WORD high, BOOL redraw = TRUE)
	{
		SendMessage(slider_hwnd, TBM_SETRANGE, redraw, MAKELONG(low,high));
	}

	void SetPosition(LPARAM position, BOOL redraw = TRUE)
	{
		SendMessage(slider_hwnd, TBM_SETPOS, redraw, position);
	}

	void SetTickFrequency(LPARAM frequency)
	{
		SendMessage(slider_hwnd, TBM_SETTICFREQ, frequency, 0);		 
	}

	enum
	{
		NO_REDRAW = FALSE,
		REDRAW = TRUE,
	};
private:
	HWND slider_hwnd;
};