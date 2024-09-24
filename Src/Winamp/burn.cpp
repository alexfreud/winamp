/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "resource.h"
#include <windows.h>
#include <strsafe.h>
#include <time.h>
#include <math.h>

#include "./api.h"
#include "./wa_ipc.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoWide.h"
#include "../Agave/Language/api_language.h"
#include <api/service/waservicefactory.h>
#ifdef BURN_SUPPORT
#include "../primo/obj_primo.h"
#include "../burnlib/burnlib.h"

#endif

static void code(long* v, long* k)
{
	unsigned long y = v[0], z = v[1], sum = 0, delta = 0x9e3779b9, n = 32 ;  /* key schedule constant*/
	while (n-- > 0)
	{                                       /* basic cycle start */
		sum += delta;
		y += ((z << 4) + k[0]) ^ (z + sum) ^ ((z >> 5) + k[1]);
		z += ((y << 4) + k[2]) ^ (y + sum) ^ ((y >> 5) + k[3]); /* end cycle */
	}
	v[0] = y; v[1] = z;
}

int getRegVer(HWND waWnd)
{
	int *x = (int*)malloc(32);
	long s[3];
	long ss[2] = {(long)GetTickCount64(), (long)((int)x + (int)s)};
	long tealike_key[4] = { 31337, 0xf00d, 0xdead, 0xbeef};
	free(x);
	s[0] = ss[0];
	s[1] = ss[1];
	s[2] = 0;

	SendMessageW(waWnd, WM_WA_IPC, (WPARAM)s, IPC_GETREGISTEREDVERSION);
	code(ss, tealike_key);
	return (memcmp(s, ss, 8)) ? 0 : s[2];
}

#ifdef BURN_SUPPORT
int burn_start(burnCDStruct *param)
{
	char buf[MAX_PATH] = "\"";
	STARTUPINFO si = {sizeof(si), };
	PROCESS_INFORMATION pi;
	GetModuleFileName(NULL, buf + 1, sizeof(buf) - 1);
	StringCchCat(buf, MAX_PATH, "\"");
	StringCchPrintf(buf + lstrlen(buf), MAX_PATH - lstrlen(buf), " /BURN=%c,\"%s\",%d", param->cdletter, param->playlist_file, param->callback_hwnd);
	return (CreateProcess(NULL, buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) ? pi.dwProcessId : 0;
}
extern "C"
{
	typedef int (*BurnFunction)(const wchar_t*, HWND, DWORD, DWORD, DWORD , const wchar_t*);
}

#define BURNER_OK				0x0000
#define BURNER_FAILED			0x0001
#define BURNER_PRIMOFAILED		0x0FF0
#define BURNER_BADPLAYLISTPATH	0x0FF1
#define BURNER_BADTEMPPATH		0x0FF2
#define BURNER_BADCONFIGPATH	0x0FF3
#define BURNER_BADCOMMANDLINE	0x0FF4
#define BURNER_BADDRIVELETTER	0x0FF5
#define BURNER_EMPTYPLAYLIST	0x0FF6
#define BURNER_DIALOGFAILED		0x0FF7

void ReportError(HWND ownerWnd, int errorCode)
{
	wchar_t description[128] = {0};

	switch (errorCode)
	{
	case BURNER_OK: return ;
	case BURNER_PRIMOFAILED: getStringW(IDS_BURN_LIBRARY_INIT_FAILED,description,128); break;
	case BURNER_BADPLAYLISTPATH: getStringW(IDS_BURN_INVALID_PLAYLIST_PATH,description,128); break;
	case BURNER_BADTEMPPATH: getStringW(IDS_BURN_INVALID_TEMP_PATH,description,128); break;
	case BURNER_BADCONFIGPATH: getStringW(IDS_BURN_INVALID_CONFIG_PATH,description,128); break;
	case BURNER_BADCOMMANDLINE: getStringW(IDS_BURN_INVALID_CMDLINE,description,128); break;
	case BURNER_BADDRIVELETTER: getStringW(IDS_BURN_BAD_DRIVE_LETTER,description,128); break;
	case BURNER_EMPTYPLAYLIST: getStringW(IDS_BURN_LOAD_FROM_PLAYLIST_FAIL,description,128); break;
	case BURNER_DIALOGFAILED: getStringW(IDS_BURN_CREATE_DIALOG_FAIL,description,128); break;
	default: getStringW(IDS_BURN_UNKNOWN_ERROR,description,128); break;
	}

	wchar_t message[1024] = {0};
	if (S_OK != StringCchPrintfW(message, 1024, getStringW(IDS_BURN_INIT_ERROR_REASON,NULL,0), description))
	{
		StringCchCopyW(message, 1024, getStringW(IDS_BURN_INIT_ERROR_UNKNOWN,NULL,0));
	}
	MessageBoxW(ownerWnd, message, getStringW(IDS_BURN_NULLSOFT_STR,NULL,0), MB_OK | MB_ICONSTOP);
}

unsigned int burn_doBurn(char *cmdline, HWND winampWnd, HINSTANCE winampInstance)
{
//	PrimoSDK::Trace(FALSE);

#ifdef _DEBUG
	MessageBox(NULL, "Starting burner", "Debug", MB_OK);
#endif

	HWND	callbackWnd = 0;
	DWORD	speed = 0;
	DWORD	cdrom = 0;
	DWORD	flags = PRIMOSDK_CLOSEDISC;
	DWORD	errorCode = BURNER_OK;
	wchar_t	tmppath[4*MAX_PATH] = {0};
	wchar_t	in_wm[MAX_PATH] = {0};
	wchar_t	playlist[4096] = {0};

	// get temp path
	if (BURNER_OK == errorCode && !GetTempPathW(MAX_PATH, tmppath)) errorCode = BURNER_BADTEMPPATH;

	if (BURNER_OK == errorCode) //// parse parameters
	{
		if (lstrlenA(cmdline) < 1) errorCode = BURNER_BADCOMMANDLINE;

		if (BURNER_OK == errorCode)
		{
			// drive letter
			CharUpperBuff(cmdline, 1);
			cdrom = cmdline[0];
			for (int i = 0; i < 2; i++) cmdline = CharNext(cmdline);
			if (cdrom < 'A' || cdrom > 'Z') errorCode = BURNER_BADDRIVELETTER;
		}

		if (BURNER_OK == errorCode)
		{
			// callback window
			char *current = cmdline + lstrlenA(cmdline);
			while (current != cmdline && *current != ',') current = CharPrevA(cmdline, current);
			callbackWnd = (current != cmdline) ? (HWND)atoi(CharNext(current)) : NULL;

			// playlist path
			size_t cchLen = current - cmdline;
			if (cchLen == 0) errorCode = BURNER_BADPLAYLISTPATH;

			if (BURNER_OK == errorCode)
			{
				if (!MultiByteToWideCharSZ(CP_ACP, 0, cmdline, current - cmdline, playlist, 4096)) errorCode = BURNER_BADPLAYLISTPATH;
				else playlist[cchLen] = 0x0000;
			}
		}

		if (BURNER_OK == errorCode)
		{
			speed = GetPrivateProfileIntW(L"gen_ml_config", L"cdburnspeed", PRIMOSDK_MIN, ML_INI_FILE);
			DWORD maxspeed = GetPrivateProfileIntW(L"gen_ml_config", L"cdburnmaxspeed", PRIMOSDK_MIN, ML_INI_FILE);
			if (!getRegVer(winampWnd)) speed = PRIMOSDK_MIN;
			if (!speed) speed = maxspeed;
			if (!speed) speed = PRIMOSDK_MIN;

			flags |= (GetPrivateProfileIntW(L"gen_ml_config", L"cdburntestmode", 0, ML_INI_FILE)) ? PRIMOSDK_TEST : PRIMOSDK_WRITE;
			flags |= (GetPrivateProfileIntW(L"gen_ml_config", L"cdburnproof", 0, ML_INI_FILE)) ? PRIMOSDK_BURNPROOF : 0;
		}
	}

	if (BURNER_OK != errorCode)
	{
		if (playlist[0] != 0x0000) DeleteFileW(playlist);
		ReportError(callbackWnd, errorCode);
		return errorCode;
	}

	// try in_wm version first
	PathCombineW(in_wm, PLUGINDIR, L"in_wm.dll");
	BurnFunction burnFunc = NULL;
	HMODULE lib = LoadLibraryW(in_wm);
	if (lib)
	{
		burnFunc = (BurnFunction)GetProcAddress(lib, "burn_doBurn");
		if (burnFunc)
		{
			errorCode = burnFunc(playlist, callbackWnd, cdrom, speed, flags, tmppath);
		}
	}
	if (lib) FreeLibrary(lib);
	lib = NULL;

	if (!burnFunc)
	{
		InitializeBurningLibrary(WASABI_API_SVC, hMainInstance, winampWnd);
		BurnerPlaylist burnPL;
		burnPL.Load(playlist);
		if (!burnPL.GetCount()) errorCode = BURNER_EMPTYPLAYLIST;
		else
		{
			obj_primo *primo=0;
			waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
			if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
			if (!primo)
				errorCode = BURNER_PRIMOFAILED;
			else
			{
				BurnPlaylistUI burnDlg;
				errorCode = burnDlg.Burn(primo, cdrom, speed, flags, &burnPL, tmppath, callbackWnd);
				sf->releaseInterface(primo);
			}
		}
	}

	DeleteFileW(playlist);
	if (BURNER_OK != errorCode) ReportError(callbackWnd, errorCode);
	return errorCode;
}
#endif