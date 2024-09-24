#define ENC_VERSION "v1.02a"

#include "Config.h"
#include "resource.h"
#include "../nsv/enc_if.h"
#include "ACMEncoder.h"
#include "WAVEncoder.h"
#include "../nu/AutoWideFn.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return TRUE;
}

// wasabi based services for localisation support
#include <api/service/waServiceFactory.h>
#include "../Agave/Language/api_language.h"
#include "../winamp/wa_ipc.h"

#include <strsafe.h>
HWND winampwnd = 0;
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static const WAVEFORMATEX wfx_default =
{
	WAVE_FORMAT_PCM,
		2,
		44100,
		44100 * 4,
		4,
		16,
		0
};

static void ReadConfig(ACMConfig *config, char *INI_FILE)
{
	int l = GetPrivateProfileIntA("enc_wav", "fmtsize", 0, INI_FILE);

	EXT_WFX convert_wfx_temp;
	if (GetPrivateProfileStructA("enc_wav", "fmt", &convert_wfx_temp, sizeof(WAVEFORMATEX) + l, INI_FILE))
		memcpy(&config->convert_wfx, &convert_wfx_temp, sizeof(config->convert_wfx));
	else
		config->convert_wfx.wfx = wfx_default; 

	GetPrivateProfileStringA("enc_wav", "ext", "WAV", config->wav_ext, sizeof(config->wav_ext), INI_FILE);
	config->header = !!GetPrivateProfileIntA("enc_wav", "header", 1, INI_FILE);
	config->convert = !!GetPrivateProfileIntA("enc_wav", "convert", 0, INI_FILE);
}

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
		WASABI_API_START_LANG(GetMyInstance(),EncWavLangGUID);
	}
}

extern "C"
{
	AudioCoder __declspec(dllexport) *CreateAudio3(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile)
	{
		if (srct == mmioFOURCC('P','C','M',' '))
		{
			if (*outt == mmioFOURCC('A','C','M',' '))
			{
				ACMConfig config;
				ReadConfig(&config, configfile);
				if (config.convert)
				{
				ACMEncoder *encoder = new ACMEncoder(srate, nch, bps, &config);
				if (encoder->GetLastError())
				{
					delete encoder;
					encoder=0;
				}
				return encoder;
				}
				else
				{
					return new WAVEncoder(nch, srate, bps, &config);
				}
			}
			else if (*outt == mmioFOURCC('W','A','V',' '))
			{
				ACMConfig config;
				ReadConfig(&config, configfile);
				return new WAVEncoder(nch, srate, bps, &config);
			}
		}
		return 0;
	}

	unsigned int __declspec(dllexport) GetAudioTypes3(int idx, char *desc)
	{
		switch(idx)
		{
		case 0:
			GetLocalisationApiService();
			StringCchPrintfA(desc, 1024, WASABI_API_LNGSTRING(IDS_ENC_WAV_DESC), ENC_VERSION);
			return mmioFOURCC('A','C','M',' ');
		}
		return 0;
	}

	HWND __declspec(dllexport) ConfigAudio3(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char *configfile)
	{
		if (outt == mmioFOURCC('A', 'C','M',' '))
		{	
			ConfigWnd configwnd;
			ReadConfig(&configwnd.cfg, configfile);
			configwnd.configfile = configfile;
			GetLocalisationApiService();
			return WASABI_API_CREATEDIALOGPARAMW(IDD_CONFIG, hwndParent, DlgProc, (LPARAM)&configwnd);
		}
		return 0;
	}

	void __declspec(dllexport) FinishAudio3(const char *filename, AudioCoder *coder)
	{
		((AudioCommon*)coder)->FinishAudio(AutoWideFn(filename));
	}

	void __declspec(dllexport) FinishAudio3W(const wchar_t *filename, AudioCoder *coder)
	{
		((AudioCommon*)coder)->FinishAudio(filename);
	}

	int __declspec(dllexport) GetConfigItem(unsigned int outt, char *item, char *data, int len, char *configfile)
	{
		if (outt==mmioFOURCC('A','C','M',' '))
		{
			ACMConfig config;
			ReadConfig(&config, configfile);
			if (!_stricmp(item, "extension"))
			{
				lstrcpynA(data, config.wav_ext, len);
				return 1;
			}
		}
		return 0;
	}

	void __declspec(dllexport) SetWinampHWND(HWND hwnd)
	{
		winampwnd = hwnd;
	}
};