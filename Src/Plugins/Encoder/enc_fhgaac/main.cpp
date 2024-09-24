#pragma comment(linker, "-nodefaultlib:libmmd.lib")
#pragma message(__FILE__": telling linker to ignore libmmd.lib")

#include "FhGAACEncoder.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "../nsv/enc_if.h"
#include "config.h"
#include "preferences.h"
#include "resource.h"
#include <api/service/waservicefactory.h>
#include "../Agave/Language/api_language.h"
#include "../winamp/wa_ipc.h"
#include "../nu/AutoWideFn.h"
#include <strsafe.h>
#include "../nu/AutoWideFn.h"
#include "encoder_common.h"
#include "ADTSAACEncoder.h"

#define ENC_VERSION "1.08"

HWND winampwnd = 0;
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
api_application *WASABI_API_APP = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
HINSTANCE enc_fhg_HINST = 0;

static HINSTANCE GetMyInstance()
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
		return (HINSTANCE)mbi.AllocationBase;
	return NULL;
}

void GetLocalisationApiService(void)
{
	if (!enc_fhg_HINST)
		enc_fhg_HINST = GetMyInstance();

	if(winampwnd && !WASABI_API_LNG)
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

		if (!WASABI_API_SVC)
			return;

		if(!WASABI_API_APP)
		{
			waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
			if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());
		}

		if(!WASABI_API_LNG)
		{
			waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
			if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());
		}

		// need to have this initialised before we try to do anything with localisation features
		WASABI_API_START_LANG(GetMyInstance(),EncFhgAacLangGUID); 
	}
}

extern "C" 
{

	unsigned int __declspec(dllexport) GetAudioTypes3(int idx, char *desc)
	{
		switch(idx)
		{
		case 0:
			GetLocalisationApiService();
			StringCchPrintfA(desc, 1024, WASABI_API_LNGSTRING(IDS_ENC_FHGAAC_DESC), ENC_VERSION);
			return ENCODER_TYPE_MPEG4;
		case 1:
			GetLocalisationApiService();
			StringCchPrintfA(desc, 1024, WASABI_API_LNGSTRING(IDC_ENC_ADTS_DESC), ENC_VERSION);
			return ENCODER_TYPE_ADTS;
		default:
			return 0;
		}
	}

	AudioCoder __declspec(dllexport) *CreateAudio3(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile)
	{
		if (srct == mmioFOURCC('P','C','M',' '))
		{
			switch(*outt)
			{
			case ENCODER_TYPE_MPEG4:
				{
					FhGAACEncoder *t=0;
					AACConfigurationFile *wr = AACConfig_Create(ENCODER_TYPE_MPEG4, configfile);
					if (!wr)
						return 0;

					t = FhGAACEncoder::CreateDecoder(&wr->config, nch, srate, bps);
					free(wr);
					return t;
				}

			case ENCODER_TYPE_ADTS:
				{
					ADTSAACEncoder *t=0;
					AACConfigurationFile *wr = AACConfig_Create(ENCODER_TYPE_ADTS, configfile);
					if (!wr)
						return 0;

					t = ADTSAACEncoder::CreateDecoder(&wr->config, nch, srate, bps);
					free(wr);
					return t;
				}
			}
		}
		return NULL;
	}

	HWND __declspec(dllexport) ConfigAudio3(HWND hwndParent, HINSTANCE hinst, unsigned int outt, const char *configfile)
	{
		switch(outt)
		{
		case ENCODER_TYPE_MPEG4:
			{
				AACConfigurationFile *wr = AACConfig_Create(ENCODER_TYPE_MPEG4, configfile);
				if (!wr)
					return 0;

				wr->channels = 2; /* dummy defaults */
				wr->sample_rate = 44100; /* dummy defaults */
				GetLocalisationApiService();
				return WASABI_API_CREATEDIALOGPARAMW(IDD_PREFERENCES_MP4, hwndParent, Preferences_MP4_DlgProc, (LPARAM)wr);
			}
		case ENCODER_TYPE_ADTS:
			{
				AACConfigurationFile *wr = AACConfig_Create(ENCODER_TYPE_ADTS, configfile);
				if (!wr)
					return 0;

				wr->channels = 2; /* dummy defaults */
				wr->sample_rate = 44100; /* dummy defaults */
				GetLocalisationApiService();
				return WASABI_API_CREATEDIALOGPARAMW(IDD_PREFERENCES_ADTS, hwndParent, Preferences_ADTS_DlgProc, (LPARAM)wr);
			}

		}
		return NULL;
	}

#if 0 // TODO
	HWND __declspec(dllexport) ConfigAudio4(HWND hwndParent, HINSTANCE hinst, unsigned int outt, const char *configfile, unsigned int channels, unsigned int sample_rate)
	{
		switch(outt)
		{
		case ENCODER_TYPE_MPEG4:
			{
				AACConfigurationFile *wr = (AACConfigurationFile*)calloc(1, sizeof(AACConfigurationFile));
				if (!wr)
					return 0;

				if (configfile) 
					lstrcpynA(wr->config_file, configfile, MAX_PATH);
				else 
					wr->config_file[0] = 0;

				wr->channels = channels;
				wr->sample_rate = sample_rate; 
				AACConfig_Load(wr);
				GetLocalisationApiService();
				return WASABI_API_CREATEDIALOGPARAMW(IDD_PREFERENCES_MP4, hwndParent, Preferences_MP4_DlgProc, (LPARAM)wr);
			}
		case ENCODER_TYPE_ADTS:
			{
				AACConfigurationFile *wr = (AACConfigurationFile*)calloc(1, sizeof(AACConfigurationFile));
				if (!wr)
					return 0;

				if (configfile) 
					lstrcpynA(wr->config_file, configfile, MAX_PATH);
				else 
					wr->config_file[0] = 0;

				wr->channels = channels; 
				wr->sample_rate = sample_rate;
				AACConfig_Load(wr);
				GetLocalisationApiService();
				return WASABI_API_CREATEDIALOGPARAMW(IDD_PREFERENCES_ADTS, hwndParent, Preferences_ADTS_DlgProc, (LPARAM)wr);
			}

		}
		return NULL;
	}
#endif

	void __declspec(dllexport) PrepareToFinish(const char *filename, AudioCoder *coder)
	{
		((EncoderCommon*)coder)->PrepareToFinish();
	}

	void __declspec(dllexport) PrepareToFinishW(const wchar_t *filename, AudioCoder *coder)
	{
		((EncoderCommon*)coder)->PrepareToFinish();
	}

	void __declspec(dllexport) FinishAudio3(const char *filename, AudioCoder *coder)
	{
		((EncoderCommon*)coder)->Finish(AutoWideFn(filename));
	}

	void __declspec(dllexport) FinishAudio3W(const wchar_t *filename, AudioCoder *coder)
	{
		((EncoderCommon*)coder)->Finish(filename);
	}

	int __declspec(dllexport) GetConfigItem(unsigned int outt, const char *item, char *data, int len, char *configfile)
	{
		if (outt == mmioFOURCC('A','A','C','f'))
		{
			if (!lstrcmpiA(item,"extension")) 
				lstrcpynA(data,"m4a",len);
			return 1;
		}
		else if (outt == mmioFOURCC('A','D','T','S'))
		{
			if (!lstrcmpiA(item,"extension")) 
			{
				lstrcpynA(data,"aac",len);
			}
			else if (!lstrcmpiA(item,"aot"))
			{
					AACConfigurationFile *wr = AACConfig_Create(ENCODER_TYPE_ADTS, configfile);
					if (wr)
					{
						StringCbPrintfA(data, len, "%u", AACConfig_GetAOT(&wr->config));
					}
					else
					{
						StringCbPrintfA(data, len, "%u", AUD_OBJ_TYP_LC);
					}
					free(wr);
			}
			return 1;
		}
		return 0;
	}

	void __declspec(dllexport) SetWinampHWND(HWND hwnd)
	{
		winampwnd = hwnd;

	}
}