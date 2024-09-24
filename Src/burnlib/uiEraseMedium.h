#pragma once

#include "./main.h"
#include "./eraseMedium.h"

#define ERASEMEDIUMUI_OK						0x000
#define ERASEMEDIUMUI_ERROR					0x001
#define ERASEMEDIUMUI_CANCELED				0x002

#define ERASEMEDIUMUI_PRIMOSDKERROR			0x101
#define ERASEMEDIUMUI_UNABLETOCREATEDIALOG	0x102
#define ERASEMEDIUMUI_MESSAGEPUMPERROR		0x103
#define ERASEMEDIUMUI_DRIVENOTSET			0x104
#define ERASEMEDIUMUI_PRIMOSDKNOTSET		0x105
#define ERASEMEDIUMUI_UNABLETOCREATEOBJECT	0x106

class EraseMediumUI
{
public:
	BURNLIB_API EraseMediumUI(void);
	BURNLIB_API ~EraseMediumUI(void);
public:
	BURNLIB_API DWORD Erase(DWORD drive, BOOL discCheck, HWND ownerWnd);
	BURNLIB_API DWORD SetEject(int ejectmode);

protected:
	static LRESULT CALLBACK PrepareWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK EraseWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnPrepareInit(HWND hwndDlg);
	void OnPrepareOk();
	void OnEraseInit(HWND hwndDlg);
	void OnEraseTimerClock(void);
	void OnEraseClose(DWORD exitCode);
	static DWORD CALLBACK OnEraseNotify(void *sender, void *param, DWORD eraseCode, DWORD primoCode);



protected:

	DWORD			drive;
	HWND				prepareWnd;
	HWND				eraseWnd;
	EraseMedium		*eraseMedium;
	DWORD			eraseMode;
	DWORD			startTick;	
	unsigned int	actualTime;
	unsigned int	estimateTime;
	DWORD			eraseCode;
	DWORD			primoCode;
	BOOL			discCheck;
};
