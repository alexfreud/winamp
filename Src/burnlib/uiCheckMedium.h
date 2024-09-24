#pragma once

#include "./main.h"
#include "./primosdk.h"
#define CHECKMEDIUMUI_MATCH						0x000
#define CHECKMEDIUMUI_NOMATCH					0x001
#define CHECKMEDIUMUI_CANCELED					0x002

#define CHECKMEDIUMUI_PRIMOSDKERROR				0x101
#define CHECKMEDIUMUI_UNABLETOCREATEDIALOG		0x102
#define CHECKMEDIUMUI_MESSAGEPUMPERROR			0x103
#define CHECKMEDIUMUI_DRIVENOTSET				0x104
#define CHECKMEDIUMUI_PRIMOSDKNOTSET				0x105
#define CHECKMEDIUMUI_DISCNOTSET					0x106
#define CHECKMEDIUMUI_DRIVENOTREADY				0x107
#define CHECKMEDIUMUI_DEADLOOP					0x108



class CheckMediumUI
{

public:
	BURNLIB_API CheckMediumUI(void);
	BURNLIB_API ~CheckMediumUI(void);

public:
	BURNLIB_API DWORD Check(obj_primo *primoSDK, DWORD *drive, WAMEDIUMINFO *medium, const wchar_t *description, BOOL disableAIN, BOOL showErase, HWND ownerWnd);
	
		
	BURNLIB_API DWORD GetPrimoError(void) { return errPrimo; }

protected:
	DWORD Rescan(void);
	static LRESULT CALLBACK WndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnInitDialog(HWND hwndDlg);
	void OnCancel(void);
	void OnDestroy(void);
	void OnEraseClicked(void);
	wchar_t* GetMediumInfoText(wchar_t *buffer, unsigned int cchBuffer, WAMEDIUMINFO *info);
	wchar_t* GetSizeText(wchar_t *buffer, unsigned int cchBuffer, unsigned int sectors);

protected:
	HWND				hwnd;
	HWND				ownerWnd;
	WAMEDIUMINFO		*desiredMedium;
	DWORD			*drive;
	obj_primo			*primoSDK;
	DWORD			errPrimo;
	DWORD			errReady;
	BOOL			disableAIN;
	BOOL			showErase;
	DWORD			deadLoop;
	const wchar_t *description;
};