#pragma once

#include "./main.h"
#include "./uiUpdatingData.h"
#include "../primo/obj_primo.h"

#define UNITREADYUI_DRIVEREADY				0x000
#define UNITREADYUI_NOTREADY					0x001
#define UNITREADYUI_CANCELED					0x002

#define UNITREADYUI_PRIMOSDKERROR			0x101
#define UNITREADYUI_UNABLETOCREATEDIALOG		0x102
#define UNITREADYUI_MESSAGEPUMPERROR			0x103
#define UNITREADYUI_DRIVENOTSET				0x104
#define UNITREADYUI_PRIMOSDKNOTSET			0x105


class UnitReadyUI
{

public:
	BURNLIB_API UnitReadyUI(void);
	BURNLIB_API ~UnitReadyUI(void);

public:
	BURNLIB_API DWORD Check(obj_primo *primoSDK, DWORD *drive, BOOL showRetry, HWND ownerWnd);
	BURNLIB_API DWORD GetPrimoError(void) { return errPrimo; }

protected:
	DWORD Rescan(void);
	static LRESULT CALLBACK WndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnInitDialog(HWND hwndDlg);
	void OnCancel(void);
	void OnDestroy(void);

protected:
	HWND		hwnd;
	DWORD	*drive;
	obj_primo *primoSDK;
	DWORD	errPrimo;
	DWORD	errReady;
	UpdatingDataUI *updateDlg;

	DWORD statSense;
	DWORD statAsc;
	DWORD statAscQ;
};