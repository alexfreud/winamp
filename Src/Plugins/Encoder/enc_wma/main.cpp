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

#define ENC_VERSION "v1.23"

#include <windows.h>
#include <stdio.h>
#include <wmsdk.h>

#include <mmreg.h>
#include <msacm.h>

#include "AudioCoderWMA.h"
#include "../nu/AutoWideFn.h"

// wasabi based services for localisation support
#include <api/service/waServiceFactory.h>
#include "../Agave/Language/api_language.h"
#include "../winamp/wa_ipc.h"

#include <strsafe.h>

HWND winampwnd = 0;
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}

int getwrittentime();

HINSTANCE g_hinst;

int g_srate, g_numchan, g_bps;
volatile int writtentime, w_offset;

// LGIVEN Mod 4-10-05
void readconfig(char *configfile, configtype *cfg);
void writeconfig(char *configfile, configtype *cfg);

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
		WASABI_API_START_LANG(GetMyInstance(),EncWMALangGUID);
	}
}


// ==================================================================
//
// Published enc_wma methods.
//
// ==================================================================
#include <cassert>

extern "C"
{

	unsigned int __declspec(dllexport) GetAudioTypes3(int idx, char *desc)
	{
		if ( idx == 0 )
		{
			GetLocalisationApiService();
			StringCchPrintfA(desc, 1024, WASABI_API_LNGSTRING(IDS_ENC_WMA_DESC), ENC_VERSION);
			return mmioFOURCC('W', 'M', 'A', ' ');
		}
		return 0;
	}

	AudioCoder __declspec(dllexport) *CreateAudio3(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile)
	{
		if (srct == mmioFOURCC('P', 'C', 'M', ' ') && *outt == mmioFOURCC('W', 'M', 'A', ' '))
		{
			GetLocalisationApiService();
			configtype cfg;
			readconfig(configfile, &cfg);

			AudioCoderWMA *t = new AudioCoderWMA(nch, srate, bps, &cfg, configfile);

			if ( t && t->GetLastError())
			{
				delete t;
				return NULL;
			}
			else return t;
		}
		return NULL;
	}

	void __declspec(dllexport) FinishAudio3(const char *filename, AudioCoder *coder)
	{
		((AudioCoderWMA*)coder)->OnFinished(AutoWideFn(filename));
	}

	void __declspec(dllexport) FinishAudio3W(const wchar_t *filename, AudioCoder *coder)
	{
		((AudioCoderWMA*)coder)->OnFinished(filename);
	}

	void __declspec(dllexport) PrepareToFinish(const char *filename, AudioCoder *coder)
	{
		((AudioCoderWMA*)coder)->PrepareToFinish();
	}

	void __declspec(dllexport) PrepareToFinishW(const wchar_t *filename, AudioCoder *coder)
	{
		((AudioCoderWMA*)coder)->PrepareToFinish();
	}

	HWND __declspec(dllexport) ConfigAudio3(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char *configfile)
	{
		if (outt == mmioFOURCC('W', 'M', 'A', ' '))
		{
			configwndrec *wr = (configwndrec*)malloc(sizeof(configwndrec));
			if (configfile)
			{
				wr->configfile = _strdup(configfile);
			}
			else
			{
				wr->configfile = 0;
			}
			readconfig(configfile, &wr->cfg);
			GetLocalisationApiService();
			return WASABI_API_CREATEDIALOGPARAMW(IDD_DIALOG1, hwndParent, ConfigProc, (LPARAM)wr);
		}
		return NULL;
	}

	int __declspec(dllexport) SetConfigItem(unsigned int outt, char *item, char *data, char *configfile)
	{
		if (outt == mmioFOURCC('W', 'M', 'A', ' '))
		{
			configtype cfg;
			readconfig(configfile, &cfg);
			if (!lstrcmpiA(item, "bitrate"))
			{
				//cfg.config_bitrate = atoi(data) * 1000;
			}
			writeconfig(configfile, &cfg);
			return 1;
		}
		return 0;
	}

	int __declspec(dllexport) GetConfigItem(unsigned int outt, char *item, char *data, int len, char *configfile)
	{
		if (outt == mmioFOURCC('W', 'M', 'A', ' '))
		{
			configtype cfg;
			readconfig(configfile, &cfg);
			if (!lstrcmpiA(item, "bitrate"))
			{
				StringCchPrintfA(data, len, "%d", cfg.config_bitrate / 1000);
			}
			return 1;
		}
		return 0;
	}

	void __declspec(dllexport) SetWinampHWND(HWND hwnd)
	{
		winampwnd = hwnd;
	}
};