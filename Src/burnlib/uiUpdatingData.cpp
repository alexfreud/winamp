#include "./uiUpdatingData.h"
#include "./resource.h"
#include <strsafe.h>

#define TIMER_SHOWDIALOG_ID			1979
#define TIMER_ANIMATION_ID			1978
#define TIMER_ANIMATION_INTERVAL	500

#define ANIMATION_CUBE_SIDE			16
#define ANIMATION_CUBE_SPACING		4
UpdatingDataUI::UpdatingDataUI(void)
{
	hwnd = NULL;
	hThread = NULL;
	evntExit = NULL;
	evntStarted = NULL;
	text[0] = 0x0000;
}


UpdatingDataUI::~UpdatingDataUI(void)
{
	Hide();
}

void UpdatingDataUI::Show(int delay,HWND ownerWnd)
{
	wchar_t buffer[128] = {0};
	LoadStringW(hResource, IDS_UPDATINGDATA, buffer, 128);
	Show(delay, buffer, TRUE, ownerWnd);
}
void UpdatingDataUI::Show(int delay, const wchar_t* text, int animation, HWND ownerWnd)
{
	this->delay = delay;
	this->ownerWnd = ownerWnd;
	StringCchCopyW(this->text, 128, text);
	this->animation = animation;
	animStep = 0;
	
	evntExit = CreateEvent(NULL, FALSE, FALSE, NULL);
	evntStarted = CreateEvent(NULL, FALSE, FALSE, NULL);
	DWORD id;
	hThread = CreateThread(NULL, 0, MessagePump, this, 0, &id);
	WaitForSingleObject(evntStarted, INFINITE);
	CloseHandle(evntStarted);
	evntStarted = NULL;
}

void UpdatingDataUI::Hide(void)
{
	if (hwnd) 	PostMessage(hwnd, WM_CLOSE, 0, 0);
	
	if (evntExit) 
	{
		WaitForSingleObject(evntExit, INFINITE);
		CloseHandle(evntExit);
		evntExit = NULL;
	}
	if (hThread)
	{	
		CloseHandle(hThread);
		hThread = NULL;
	}
}

void UpdatingDataUI::OnInitDialog(HWND hwndDlg)
{
	hwnd = hwndDlg;
	SetTimer(hwnd, TIMER_SHOWDIALOG_ID, delay, NULL);
	SetEvent(evntStarted);
}

void UpdatingDataUI::OnShowTimer(void)
{
	KillTimer(hwnd, TIMER_SHOWDIALOG_ID);
	RECT rect;
	if (ownerWnd)
	{
		RECT ownerRect;
		GetWindowRect(hwnd, &rect);
		GetWindowRect(ownerWnd, &ownerRect);
		SetWindowPos(hwnd, HWND_TOPMOST, ownerRect.left + ((ownerRect.right - ownerRect.left) - (rect.right - rect.left))/2,
										ownerRect.top + ((ownerRect.bottom - ownerRect.top) - (rect.bottom - rect.top))/2,
										0,0, SWP_NOSIZE);
	}
	SetDlgItemTextW(hwnd, IDC_LBL_TEXT, text);
	if (animation) 
	{
		GetClientRect(hwnd, &rect);
		int width = (rect.right - rect.left);
		animStep = 0;
		SetRect(&animRect, 10,  (rect.bottom - rect.top) - ANIMATION_CUBE_SIDE - 8,  width - 8, (rect.bottom - rect.top) - 10);
		animMaxStep = ((animRect.right - animRect.left) + ANIMATION_CUBE_SPACING) / (ANIMATION_CUBE_SIDE + ANIMATION_CUBE_SPACING);
		SetTimer(hwnd, TIMER_ANIMATION_ID, TIMER_ANIMATION_INTERVAL, NULL);
	}
	else
	{
		SetRect(&animRect, 0, 0, 0, 0);
		animMaxStep = 0;
	}
	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);
}
void UpdatingDataUI::OnAnimationTimer(void)
{
	animStep++;
	if (animStep == animMaxStep) animStep = 0;
	InvalidateRect(hwnd, &animRect, FALSE);
}

void UpdatingDataUI::OnDestroy(void)
{
	hwnd = NULL;
}

void UpdatingDataUI::OnPaint(PAINTSTRUCT *ps)
{
	if (RectVisible(ps->hdc, &animRect))
	{
		HBRUSH br = GetSysColorBrush(COLOR_3DFACE);
		HBRUSH sbr = CreateSolidBrush(RGB(254, 172, 1));
		HBRUSH oldBrush;
		HPEN oldPen, pen = CreatePen(PS_SOLID, 2, RGB(21, 72, 9));
		oldPen = (HPEN)SelectObject(ps->hdc, pen);
		oldBrush = (HBRUSH)SelectObject(ps->hdc, br);
		RECT cube = {animRect.left, animRect.top, animRect.left + ANIMATION_CUBE_SIDE, animRect.top + ANIMATION_CUBE_SIDE};
		
		for (int i = 0; i < animMaxStep; i++)
		{
			SelectObject(ps->hdc, (i == animStep) ? sbr : br);
			Rectangle(ps->hdc, cube.left, cube.top, cube.right, cube.bottom);
			cube.left = cube.right + ANIMATION_CUBE_SPACING;
			cube.right = cube.left + ANIMATION_CUBE_SIDE; 
		}
		SelectObject(ps->hdc, oldPen);
		SelectObject(ps->hdc,oldBrush);
	//	DeleteObject(br);
		DeleteObject(sbr);
	}
}


DWORD UpdatingDataUI::MessagePump(void *param)
{
	UpdatingDataUI *object = (UpdatingDataUI*)param;

	LPCDLGTEMPLATE templ = NULL;
	HRSRC hres = FindResourceExW(hResource,  MAKEINTRESOURCEW(5), MAKEINTRESOURCEW(IDD_DLG_UPDATING), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)); 
	if (hres) templ = (LPCDLGTEMPLATE)LoadResource(hResource, hres);
	HWND dlgWnd = CreateDialogIndirectParamW(dllInstance, templ, NULL, (DLGPROC)WndProc, (LPARAM)object);
	if (!dlgWnd) return 1;
	
	MSG msg;
	BOOL ret;
	while( 0 != (ret = GetMessageW(&msg, dlgWnd, 0, 0)))
	{	 
		if (ret == -1) 	break;
		if (IsDialogMessage(dlgWnd, &msg)) continue;
		TranslateMessage(&msg); 
		DispatchMessageW(&msg); 
	}
	SetEvent(object->evntExit);
	return 0;
}
LRESULT UpdatingDataUI::WndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static UpdatingDataUI *object = NULL;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			object = (UpdatingDataUI*)lParam;
			object->OnInitDialog(hwndDlg);
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				if (BeginPaint(hwndDlg, &ps))
				{
					object->OnPaint(&ps);
					EndPaint(hwndDlg, &ps);
				}
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;
		case WM_DESTROY:
			ShowWindow(hwndDlg, SW_HIDE);
			PostQuitMessage(1);
			object->OnDestroy();
			break;
		case WM_TIMER:
			switch(wParam)
			{
				case TIMER_SHOWDIALOG_ID:
					object->OnShowTimer();
					break;
				case TIMER_ANIMATION_ID:
					object->OnAnimationTimer();
					break;
			}
			break;

	}
	return 0;
}