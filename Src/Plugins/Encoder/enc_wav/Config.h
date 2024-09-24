#ifndef NULLSOFT_ENC_ACM_CONFIG_H
#define NULLSOFT_ENC_ACM_CONFIG_H

#include <windows.h>
#include <mmreg.h>
#include <msacm.h>

#define WFSIZ 0x800
struct EXT_WFX
{
	WAVEFORMATEX wfx;
	BYTE crap[WFSIZ];
};

struct ACMConfig
{
	EXT_WFX convert_wfx;
	char wav_ext[32];
	bool header;
	bool convert;
};

struct ConfigWnd
{
	ACMConfig cfg;
	char *configfile;
};

INT_PTR WINAPI DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif