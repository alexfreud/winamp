/*
** nsv_coder_lame: main.cpp - LAME mp3 encoder plug-in
** (requires lame_enc.dll)
** 
** Copyright (C) 2001-2006 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty.  
** In no event will the authors be held liable for any damages arising from the use of this software.
**
** Permission is granted to anyone to use this software for any purpose, including commercial 
** applications, and to alter it and redistribute it freely, subject to the following restrictions:
**  1. The origin of this software must not be misrepresented; you must not claim that you wrote the 
**     original software. If you use this software in a product, an acknowledgment in the product 
**     documentation would be appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be misrepresented as 
**     being the original software.
**  3. This notice may not be removed or altered from any source distribution.
*/

#define ENC_VERSION "v1.38"

#include <stdio.h>
#include "resource.h"
#include "BladeMP3EncDLL.h"
#include "MP3Coder.h"
#include <strsafe.h>
#include <shlwapi.h>
#include <lame/lame.h>

// wasabi based services for localisation support
#include <api/service/waServiceFactory.h>
#include "../Agave/Language/api_language.h"
#include "../winamp/wa_ipc.h"

HWND winampwnd = 0;
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

int g_valid_bitrates[] = { 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1 };

static wchar_t lamedll[MAX_PATH]=L"";
HINSTANCE g_lamedll = 0;

//BEINITSTREAM beInitStream=0;
//BECLOSESTREAM beCloseStream=0;
//BEENCODECHUNK beEncodeChunk=0;
//BEDEINITSTREAM beDeinitStream=0;
//BEWRITEVBRHEADER beWriteVBRHeader=0;
//BEVERSION beVersion=0;
//BEENCODECHUNKFLOATS16NI beEncodeChunkFloatS16NI=0;

static HINSTANCE GetMyInstance()
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
		return (HINSTANCE)mbi.AllocationBase;
	return NULL;
}

void GetLocalisationApiService(void)
{
	if(!WASABI_API_LNG)
	{
		// loader so that we can get the localisation service api for use
		if(!WASABI_API_SVC)
		{
			WASABI_API_SVC = (api_service*)SendMessage(winampwnd, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
			if (WASABI_API_SVC == (api_service*)1)
			{
				WASABI_API_SVC = NULL;
				return;
			}
		}

		if(!WASABI_API_LNG)
		{
			waServiceFactory *sf;
			sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
			if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());
		}

		// need to have this initialised before we try to do anything with localisation features
		WASABI_API_START_LANG(GetMyInstance(),EncLameLangGUID);
	}
}
/*
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}*/

typedef struct
{
	configtype cfg;
	char configfile[MAX_PATH];
}
configwndrec;

void BuildVersionString(wchar_t version[128])
{
	BE_VERSION ver;
	beVersion(&ver);

	if (ver.byBetaLevel)
		StringCchPrintf(version, 128, L"%u.%ub%u", (unsigned int)ver.byMajorVersion, (unsigned int)ver.byMinorVersion, (unsigned int)ver.byBetaLevel);
	else if (ver.byAlphaLevel)
		StringCchPrintf(version, 128, L"%u.%ua%u", (unsigned int)ver.byMajorVersion, (unsigned int)ver.byMinorVersion, (unsigned int)ver.byAlphaLevel);
	else
		StringCchPrintf(version, 128, L"%u.%u", (unsigned int)ver.byMajorVersion, (unsigned int)ver.byMinorVersion);
}

void readconfig(char *configfile, configtype *cfg)
{
	cfg->bitrate = 128;
	cfg->vbr_max_bitrate = 320;
	cfg->abr_bitrate = 128;

	cfg->stereo_mode = 1;
	cfg->quality = LQP_FAST_STANDARD;

	cfg->vbr = 2;
	cfg->vbr_method = VBR_METHOD_NEW;

	if (configfile) 
		GetPrivateProfileStructA("audio_mp3l", "conf", cfg, sizeof(configtype), configfile);
}

void writeconfig(char* configfile, configtype *cfg)
{
	if (configfile)
		WritePrivateProfileStructA("audio_mp3l", "conf", cfg, sizeof(configtype), configfile);
}

extern "C"
{
	unsigned int __declspec(dllexport) GetAudioTypes3(int idx, char *desc)
	{
		//if ((g_lamedll != NULL) && (beInitStream != NULL) && (beCloseStream != NULL) && (beEncodeChunk != NULL) && (beDeinitStream != NULL) && (beWriteVBRHeader != NULL) && (beVersion != NULL))
			if (idx == 0)
			{
				GetLocalisationApiService();
				StringCchPrintfA(desc, 1024, WASABI_API_LNGSTRING(IDS_ENC_LAME_DESC), ENC_VERSION);
				return mmioFOURCC('M', 'P', '3', 'l');
			}
		return 0;
	}

	AudioCoder __declspec(dllexport) *CreateAudio3(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile)
	{
		if (nch > 2 || srate > 48000)
			return NULL;

		if (srct == mmioFOURCC('P', 'C', 'M', ' ') && *outt == mmioFOURCC('M', 'P', '3', 'l'))
		{
			configtype cfg;
			readconfig(configfile, &cfg);
			*outt = mmioFOURCC('M', 'P', '3', ' ');
			AudioCoderMP3 *t=0;
			if (bps != 16)
				t = new AudioCoderMP3_24(nch, srate, bps, &cfg);
			else
				t = new AudioCoderMP3(nch, srate, bps, &cfg);
			if (t->GetLastError())
			{
				delete t;
				return NULL;
			}
			return t;
		}
		return NULL;
	}

	void __declspec(dllexport) FinishAudio3(const char *filename, AudioCoder *coder)
	{
		//beWriteVBRHeader(filename);
		((AudioCoderMP3*)coder)->setVbrFilename((char *)filename); //apparently this needs to be called after beCloseStream
	}

	void __declspec(dllexport) PrepareToFinish(const char *filename, AudioCoder *coder)
	{
		((AudioCoderMP3*)coder)->PrepareToFinish();
	}

	static void setBitrates(HWND hwndDlg, int mi, int ma)
	{
		int i = 0;
		while (g_valid_bitrates[i] > 0)
		{
			if (g_valid_bitrates[i] == mi) SendDlgItemMessage(hwndDlg, IDC_BITRATE, CB_SETCURSEL, i, 0);
			if (g_valid_bitrates[i] == ma) SendDlgItemMessage(hwndDlg, IDC_MAXBITRATE, CB_SETCURSEL, i, 0);
			i++;
		}
	}

	BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_USER + 666)
		{
			int vbr_meth = (int)SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_GETCURSEL, 0, 0);
			if (vbr_meth == CB_ERR) vbr_meth = 0;
			vbr_meth--;
			EnableWindow(GetDlgItem(hwndDlg, IDC_VBRQ), vbr_meth != -1 && vbr_meth != 4);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VBRQ_TEXT), vbr_meth != -1 && vbr_meth != 4);
			SetDlgItemTextW(hwndDlg, IDC_BITRATE_TEXT1, WASABI_API_LNGSTRINGW((vbr_meth == -1 ? IDS_BITRATE : vbr_meth == 4 ? IDS_ABR_MIN_BITRATE : IDS_VBR_MIN_BITRATE)));
			SetDlgItemTextW(hwndDlg, IDC_MAXBITRATE_TEXT1, WASABI_API_LNGSTRINGW((vbr_meth == -1 ? IDS_N_A : vbr_meth == 4 ? IDS_ABR_MAX_BITRATE : IDS_VBR_MAX_BITRATE)));
			SetDlgItemTextW(hwndDlg, IDC_AVGBITRATE_TEXT1, WASABI_API_LNGSTRINGW((vbr_meth != 4 ? IDS_N_A : IDS_AVERAGE_BITRATE)));

			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXBITRATE_TEXT1), vbr_meth != -1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXBITRATE_TEXT2), vbr_meth != -1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXBITRATE), vbr_meth != -1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVGBITRATE_TEXT1), vbr_meth == 4);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVGBITRATE_TEXT2), vbr_meth == 4);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVGBITRATE), vbr_meth == 4);

			int qual = (int)SendDlgItemMessage(hwndDlg, IDC_QUALITY, CB_GETCURSEL, 0, 0);
			if (qual == CB_ERR) qual = 0;
			switch (qual)
			{
			case LQP_R3MIX:
			case LQP_STANDARD:
			case LQP_FAST_STANDARD:
			case LQP_EXTREME:
			case LQP_FAST_EXTREME:
			case LQP_INSANE:
			case LQP_MEDIUM:
			case LQP_FAST_MEDIUM:
			case LQP_ABR:
			case LQP_CBR:
				EnableWindow(GetDlgItem(hwndDlg, IDC_STEREOMODE), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_VBRMETHOD), 0);
				break;
			default:
				EnableWindow(GetDlgItem(hwndDlg, IDC_STEREOMODE), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_VBRMETHOD), 1);
				break;
			}
			if (qual == 4) EnableWindow(GetDlgItem(hwndDlg, IDC_VBRQ), 0);
		}

		if (uMsg == WM_USER + 667)
		{
			int qual = (int)SendDlgItemMessage(hwndDlg, IDC_QUALITY, CB_GETCURSEL, 0, 0);
			if (qual == CB_ERR) qual = 0;
			switch (qual)
			{
			case LQP_R3MIX:
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 3, 0);
				setBitrates(hwndDlg, 96, 224);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, 1, 0);
				break;
			case LQP_STANDARD:  /* check for alt-preset standard */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 2, 0);
				setBitrates(hwndDlg, 32, 320);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, 2, 0);
				break;
			case LQP_FAST_STANDARD:  /* check for alt-preset fast standard  */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 3, 0);
				setBitrates(hwndDlg, 32, 320);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, 2, 0);
				break;
				case LQP_MEDIUM:  /* check for alt-preset medium */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 2, 0);
				setBitrates(hwndDlg, 32, 320);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, 2, 0);
				break;
			case LQP_FAST_MEDIUM:  /* check for alt-preset fast medium  */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 3, 0);
				setBitrates(hwndDlg, 32, 320);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, 2, 0);
				break;
			case LQP_EXTREME:  /* check for alt-preset extreme  */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 2, 0);
				setBitrates(hwndDlg, 32, 320);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, 2, 0);
				break;
			case LQP_FAST_EXTREME:  /* check for alt-preset fast extreme */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 3, 0);
				setBitrates(hwndDlg, 32, 320);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, 2, 0);
				break;
			case LQP_INSANE:  /* check for alt-preset fast insane  */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 0, 0);
				setBitrates(hwndDlg, 320, 320);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				break;
			case LQP_ABR:  /* check for alt-preset fast insane  */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 5, 0);
				setBitrates(hwndDlg, 64, 320);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, 2, 0);
				break;
			case LQP_CBR:  /* check for alt-preset fast insane  */
				SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, 1, 0);
				break;
			}
			SendMessage(hwndDlg, WM_USER + 666, 0, 0);
		}

		if (uMsg == WM_INITDIALOG)
		{
#if defined (_WIN64)
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
#else
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
#endif
			wchar_t versionText[128] = {0};
			BuildVersionString(versionText);
			SetDlgItemText(hwndDlg, IDC_LAMEVERSION, versionText);
			if (lParam)
			{
				configwndrec *wc = (configwndrec*)lParam;

				// -1=none, 0=default, 1=old, 2=new, 3=mtrh, 4=abr
				SendDlgItemMessageW(hwndDlg, IDC_VBRMETHOD, CB_ADDSTRING, 0, (LPARAM)L"CBR");
				SendDlgItemMessageW(hwndDlg, IDC_VBRMETHOD, CB_ADDSTRING, 0, (LPARAM)L"VBR default");
				SendDlgItemMessageW(hwndDlg, IDC_VBRMETHOD, CB_ADDSTRING, 0, (LPARAM)L"VBR old");
				SendDlgItemMessageW(hwndDlg, IDC_VBRMETHOD, CB_ADDSTRING, 0, (LPARAM)L"VBR new");
				SendDlgItemMessageW(hwndDlg, IDC_VBRMETHOD, CB_ADDSTRING, 0, (LPARAM)L"VBR mtrh");
				SendDlgItemMessageW(hwndDlg, IDC_VBRMETHOD, CB_ADDSTRING, 0, (LPARAM)L"ABR");
				SendDlgItemMessageW(hwndDlg, IDC_VBRMETHOD, CB_SETCURSEL, wc->cfg.vbr_method + 1, 0);

				//0=stereo,1=jstereo,2=mchannel,3=mono
				SendDlgItemMessageW(hwndDlg, IDC_STEREOMODE, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_STEREO));
				SendDlgItemMessageW(hwndDlg, IDC_STEREOMODE, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_JOINT_STEREO));
				SendDlgItemMessageW(hwndDlg, IDC_STEREOMODE, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_MULTI_CHANNEL));
				SendDlgItemMessageW(hwndDlg, IDC_STEREOMODE, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_MONO));
				SendDlgItemMessageW(hwndDlg, IDC_STEREOMODE, CB_SETCURSEL, wc->cfg.stereo_mode, 0);

				{
					int i = 0;
					while (g_valid_bitrates[i] > 0)
					{
						wchar_t buf[64] = {0};
						StringCchPrintf(buf, 64, L"%d", g_valid_bitrates[i]);
						SendDlgItemMessage(hwndDlg, IDC_BITRATE , CB_ADDSTRING, 0, (LPARAM)buf);
						SendDlgItemMessage(hwndDlg, IDC_MAXBITRATE, CB_ADDSTRING, 0, (LPARAM)buf);
						SendDlgItemMessage(hwndDlg, IDC_AVGBITRATE, CB_ADDSTRING, 0, (LPARAM)buf);
						SendDlgItemMessage(hwndDlg, IDC_BITRATE , CB_SETITEMDATA, i, g_valid_bitrates[i]);
						SendDlgItemMessage(hwndDlg, IDC_MAXBITRATE, CB_SETITEMDATA, i, g_valid_bitrates[i]);
						SendDlgItemMessage(hwndDlg, IDC_AVGBITRATE, CB_SETITEMDATA, i, g_valid_bitrates[i]);
						i++;
					};

					i = 0;
					while (g_valid_bitrates[i] > 0)
					{
						if (g_valid_bitrates[i] == wc->cfg.bitrate ) SendDlgItemMessage(hwndDlg, IDC_BITRATE , CB_SETCURSEL, i, 0);
						if (g_valid_bitrates[i] == wc->cfg.abr_bitrate ) SendDlgItemMessage(hwndDlg, IDC_AVGBITRATE, CB_SETCURSEL, i, 0);
						if (g_valid_bitrates[i] == wc->cfg.vbr_max_bitrate) SendDlgItemMessage(hwndDlg, IDC_MAXBITRATE, CB_SETCURSEL, i, 0);
						i++;
					}
				}

				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_NORMAL));
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_LOW));
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_HIGH));
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_VOICE));
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_R3MIX));
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_VERY_HIGH));
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset standard");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset fast standard");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset extreme");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset fast extreme");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset insane");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset abr");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset cbr");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset medium");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_ADDSTRING, 0, (LPARAM)L"--alt-preset fast medium");
				SendDlgItemMessageW(hwndDlg, IDC_QUALITY, CB_SETCURSEL, wc->cfg.quality, 0);

				int x;
				for (x = 0; x < 10; x ++)
				{
					wchar_t buf[123] = {0};
					StringCchPrintfW(buf, 123, L"%d%s", x, x == 0 ? WASABI_API_LNGSTRINGW(IDS_HIGH_BRACKET) : x == 9 ? WASABI_API_LNGSTRINGW(IDS_LOW_BRACKET) : L"");
					SendDlgItemMessageW(hwndDlg, IDC_VBRQ, CB_ADDSTRING, 0, (LPARAM)buf);
				}
				SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_SETCURSEL, wc->cfg.vbr, 0);

				SendMessage(hwndDlg, WM_USER + 666, 0, 0);
			}
		}

		if (uMsg == WM_COMMAND)
		{
			if (LOWORD(wParam) == IDC_VBRMETHOD && HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(hwndDlg, WM_USER + 666, 0, 0);
			if (LOWORD(wParam) == IDC_QUALITY && HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(hwndDlg, WM_USER + 667, 0, 0);
		}

		if (uMsg == WM_DESTROY)
		{
#if defined (_WIN64)
			configwndrec *wc = (configwndrec*)SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
#else
			configwndrec* wc = (configwndrec*)SetWindowLong(hwndDlg, GWL_USERDATA, 0);
#endif
			if (wc)
			{
				wc->cfg.bitrate = (int)SendDlgItemMessage(hwndDlg, IDC_BITRATE , CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_BITRATE , CB_GETCURSEL, 0, 0), 0);
				wc->cfg.vbr_max_bitrate = (int)SendDlgItemMessage(hwndDlg, IDC_MAXBITRATE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_MAXBITRATE, CB_GETCURSEL, 0, 0), 0);
				wc->cfg.abr_bitrate = (int)SendDlgItemMessage(hwndDlg, IDC_AVGBITRATE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_AVGBITRATE, CB_GETCURSEL, 0, 0), 0);
				int vbr_meth = (int)SendDlgItemMessage(hwndDlg, IDC_VBRMETHOD, CB_GETCURSEL, 0, 0);
				if (vbr_meth == CB_ERR) vbr_meth = 0;
				wc->cfg.vbr_method = vbr_meth - 1;
				wc->cfg.stereo_mode = (int)SendDlgItemMessage(hwndDlg, IDC_STEREOMODE, CB_GETCURSEL, 0, 0);
				wc->cfg.quality = (int)SendDlgItemMessage(hwndDlg, IDC_QUALITY, CB_GETCURSEL, 0, 0);
				wc->cfg.vbr = (int)SendDlgItemMessage(hwndDlg, IDC_VBRQ, CB_GETCURSEL, 0, 0);
				writeconfig(wc->configfile, &wc->cfg);
				free(wc);
			}
		}
		return 0;
	}

	HWND __declspec(dllexport) ConfigAudio3(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char*configfile)
	{
		if (outt == mmioFOURCC('M', 'P', '3', 'l'))
		{
			configwndrec *wr = (configwndrec*)malloc(sizeof(configwndrec));
			if (configfile) lstrcpynA(wr->configfile, configfile, MAX_PATH);
			else wr->configfile[0] = 0;

			readconfig(configfile, &wr->cfg);
			GetLocalisationApiService();
			return WASABI_API_CREATEDIALOGPARAMW(IDD_MP3, hwndParent, DlgProc, (LPARAM)wr);
		}
		return NULL;
	}

	int __declspec(dllexport) SetConfigItem(unsigned int outt, wchar_t*item, char *data, char* configfile)
	{
		if (outt == mmioFOURCC('M', 'P', '3', 'l'))
		{
			configtype cfg;
			readconfig(configfile, &cfg);
			if (!lstrcmpi(item, L"bitrate")) // assume CBR bitrate
			{
				cfg.abr_bitrate = cfg.bitrate = atoi(data);
				cfg.vbr_method = -1;
			}
			writeconfig(configfile, &cfg);
			return 1;
		}
		return 0;
	}

	int __declspec(dllexport) GetConfigItem(unsigned int outt, wchar_t *item, wchar_t *data, int len, char* configfile)
	{
		if (outt == mmioFOURCC('M', 'P', '3', 'l'))
		{
			configtype cfg;
			readconfig(configfile, &cfg);
			if (!lstrcmpi(item, L"bitrate"))
			{
				int bitrate;
				if(cfg.vbr_method == -1) bitrate = cfg.bitrate;
				else if(cfg.vbr_method == 4) bitrate = cfg.abr_bitrate;
				else bitrate = (cfg.bitrate + cfg.vbr_max_bitrate) / 2;
				StringCchPrintf(data,len,L"%d",bitrate);
			}
			return 1;
		}
		return 0;
	}

	void __declspec(dllexport) SetWinampHWND(HWND hwnd)
	{
		winampwnd = hwnd;

		// this is called when the encoder is needed (and is slightly better than the dllmain loading from before)

		/*if (!g_lamedll)
		{
			if (!lamedll[0])
			{
				PathCombineW(lamedll, (wchar_t*)SendMessage(hwnd, WM_WA_IPC, 0, IPC_GETSHAREDDLLDIRECTORYW),L"lame_enc.dll");
			}
			g_lamedll = LoadLibraryW(lamedll);
		}*/

		/*if (g_lamedll)
		{
			beInitStream	= (BEINITSTREAM) &beInitStream;
			beCloseStream	= (BECLOSESTREAM) GetProcAddress(g_lamedll, TEXT_BECLOSESTREAM);
			beEncodeChunk	= (BEENCODECHUNK) GetProcAddress(g_lamedll, TEXT_BEENCODECHUNK);
			beDeinitStream	= (BEDEINITSTREAM) GetProcAddress(g_lamedll, TEXT_BEDEINITSTREAM);
			beWriteVBRHeader = (BEWRITEVBRHEADER) GetProcAddress(g_lamedll, TEXT_BEWRITEVBRHEADER);
			beVersion = (BEVERSION) GetProcAddress(g_lamedll, TEXT_BEVERSION);
			beEncodeChunkFloatS16NI = (BEENCODECHUNKFLOATS16NI)GetProcAddress(g_lamedll, TEXT_BEENCODECHUNKFLOATS16NI);
		}*/
	}
};