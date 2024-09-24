#pragma once

#include "./main.h"

class UpdatingDataUI
{
public:
	BURNLIB_API UpdatingDataUI(void);
	BURNLIB_API ~UpdatingDataUI(void);

public:
	BURNLIB_API void Show(int delay, HWND ownerWnd);
	BURNLIB_API void Show(int delay, const wchar_t* text, int animation, HWND ownerWnd);
	BURNLIB_API void Hide(void);

protected:
	static LRESULT CALLBACK WndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnInitDialog(HWND hwndDlg);
	void OnDestroy(void);
	void OnShowTimer(void);
	void OnAnimationTimer(void);
	void OnPaint(PAINTSTRUCT *ps);
	static DWORD WINAPI MessagePump(void* param);

protected:
	HWND				hwnd;
	HWND				ownerWnd;
	HANDLE			hThread;
	HANDLE			evntExit;
	HANDLE			evntStarted;
	int				delay;
	wchar_t			text[128];
	int				animation;
	int				animStep;
	int				animMaxStep;
	RECT			animRect;



};