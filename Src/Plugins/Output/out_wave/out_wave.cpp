#include "out_wave.h"
#include "api.h"
#include "resource.h"
#include "waveout.h"
#include "../winamp/wa_ipc.h"
#include "../nu/AutoWide.h"

#ifdef HAVE_SSRC
#include "ssrc\ssrc.h"
static Resampler_base * pSSRC;
#endif

static bool gapless_stop;
static WaveOut * pWO;
static __int64 total_written;
static int pos_delta;
static UINT canwrite_hack;

// wasabi based services for localisation support
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
api_application *WASABI_API_APP = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

void _init();

//cfg_prebuf now in ms !
UINT cfg_dev = 0, cfg_buf_ms = 2000, cfg_prebuf = 200, cfg_trackhack = 200;
bool cfg_volume = 1, cfg_altvol = 0, cfg_resetvol = 0;
static int fmt_sr, fmt_bps, fmt_nch;

static int volume = 255, pan = 0;


#ifdef HAVE_SSRC
UINT cfg_dither = 1, cfg_resample_freq = 48000, cfg_resample_bps = 1;
bool cfg_fast = 1;
UINT cfg_pdf = 1;
UINT bps_tab[3] = {8, 16, 24};
static bool finished, use_finish;

static UINT resample_freq;
static UINT resample_bps;

static void do_ssrc_create()
{
	pSSRC = SSRC_create(fmt_sr, resample_freq, fmt_bps, resample_bps, fmt_nch, cfg_dither, cfg_pdf, cfg_fast, 0);
	finished = 0;
	use_finish = cfg_trackhack == 0 ? 1 : 0;
}
#endif

void do_cfg(bool s);

static CRITICAL_SECTION sync; //various funky time consuming stuff going on, better protect ourselves with a critical section here, resampler doesnt have its own one
#define SYNC_IN EnterCriticalSection(&sync);
#define SYNC_OUT LeaveCriticalSection(&sync);


void Config(HWND);

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMSW),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCEW(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirectW(&msgbx);
}

void About(HWND hwndParent)
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_WAVEOUT_OLD,text,1024);
	wsprintfW(message, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
			  mod.description, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}

static void Init()
{
	if (!IsWindow(mod.hMainWindow))
		return;

	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1) WASABI_API_SVC = NULL;
	if (!WASABI_API_SVC || WASABI_API_SVC == (api_service *)1)
		return;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,OutWaveLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_WAVEOUT),OUT_WAVE_VER);
	mod.description = (char*)szDescription;
}

static int inited;

static void Quit()
{
	if (inited)
	{
		if (pWO)
		{
			delete pWO;
			pWO = 0;
		}
#ifdef HAVE_SSRC
		if (pSSRC)
		{
			delete pSSRC;
			pSSRC = 0;
		}
#endif
		do_cfg(1);
		inited = 0;
	}
}

static void reset_stuff()
{
	canwrite_hack = 0;
	pos_delta = 0;
	total_written = 0;
	gapless_stop = 0;
}


void _init()
{
	if (!inited)
	{
		inited = 1;
		do_cfg(0);
		if (cfg_dev > waveOutGetNumDevs()) cfg_dev = 0;
	}
}

static void _write(char * data, int size);

int Open(int sr, int nch, int bps, int bufferlenms, int prebufferms)
{
	_init();
	SYNC_IN;

	if (pWO)	//"gapless" track change (or someone forgot to close output)
	{
		pWO->SetCloseOnStop(0);	//turn off self-destruct on out-of-PCM-data
		if (!pWO->IsClosed())	//has it run out of PCM data or not ? if yes, we can only delete and create new one
		{
			if (sr != fmt_sr || nch != fmt_nch || bps != fmt_bps) //tough shit, new pcm format
			{ //wait-then-close, dont cut previous track
#ifdef HAVE_SSRC
				if (!pSSRC && !finished)
				{
					use_finish = 1;
					_write(0, 0);
				}
#endif
				while (pWO->GetLatency() > 0) Sleep(1);
			}
			else
			{ //successful gapless track change. yay.
				reset_stuff();
#ifdef HAVE_SSRC
				if (pSSRC)
				{
					if (finished)
					{
						delete pSSRC;
						do_ssrc_create();
					}
					else use_finish = cfg_trackhack == 0 ? 1 : 0;
				}
#endif
				int r = pWO->GetMaxLatency();
				SYNC_OUT;
				return r;
			}
		}
#ifdef HAVE_SSRC
		if (pSSRC)
		{
			delete pSSRC;
			pSSRC = 0;
		}
#endif
		delete pWO;
	}

	WaveOutConfig * cfg = new WaveOutConfig; //avoid crazy crt references with creating cfg on stack, keep TINY_DLL config happy
	cfg->SetBuffer(cfg_buf_ms, cfg_prebuf);
	cfg->SetDevice(cfg_dev);
	cfg->SetVolumeSetup(cfg_volume, cfg_altvol, cfg_resetvol);

	fmt_sr = sr;
	fmt_nch = nch;
	fmt_bps = bps;

#ifdef HAVE_SSRC
	resample_freq = cfg_resample_freq;
	if (resample_freq < 6000) resample_freq = 6000;
	else if (resample_freq > 192000) resample_freq = 192000;
	resample_bps = bps_tab[cfg_resample_bps];

	if (fmt_sr == (int)resample_freq && fmt_bps == (int)resample_bps)
	{
		cfg->SetPCM(fmt_sr, fmt_nch, fmt_bps);
	}
	else
	{
		do_ssrc_create();
	}


	if (!pSSRC) cfg->SetPCM(sr, nch, bps);
	else cfg->SetPCM(resample_freq, nch, resample_bps);

#else//!HAVE_SSRC
	cfg->SetPCM(sr, nch, bps);
#endif

	pWO = WaveOut::Create(cfg);
	if (!pWO)
	{
		const WCHAR *error = cfg->GetError();
		if (error)
		{
			WCHAR err[128] = {0}, temp[128] = {0};
			swprintf(err,128,WASABI_API_LNGSTRINGW(IDS_ERROR),WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_WAVEOUT_OLD,temp,128));
			MessageBoxW(mod.hMainWindow, error, err, MB_ICONERROR);
		}
	}
	else
	{
		reset_stuff();
	}

	delete cfg;
	if (pWO)
	{
		int r = pWO->GetMaxLatency();
		SYNC_OUT;
		return r;
	}
	else
	{
#ifdef HAVE_SSRC
		if (pSSRC)
		{
			delete pSSRC;
			pSSRC = 0;
		}
#endif
		SYNC_OUT;
		return -1;
	}
}

#ifdef HAVE_SSRC
static UINT ssrc_extra_latency;

static void _write(char * data, int size)
{
	if (pWO)
	{
		if (pSSRC)
		{
			if (!finished)
			{
				UINT nsiz;
				if (data > 0) pSSRC->Write(data, (UINT)size);
				else if (use_finish)
				{
					finished = 1;
					pSSRC->Finish();
				}
				data = (char*)pSSRC->GetBuffer(&nsiz);
				UINT nsiz1 = nsiz;
				while (nsiz) //ugly
				{
					int wr = pWO->WriteData(data, nsiz);
					if (wr > 0)
					{
						data += wr;
						nsiz -= wr;
						if (!nsiz) break;
					}
					ssrc_extra_latency = MulDiv(nsiz, 1000, resample_freq * fmt_nch * (resample_bps >> 3));
					SYNC_OUT;
					Sleep(1); //shouldnt happen anymore since canwrite works correctly
					SYNC_IN;
				}
				pSSRC->Read(nsiz1);
				ssrc_extra_latency = 0;
			}
		}
		else
		{
			pWO->WriteData(data, size);
		}
		total_written += size / ((fmt_bps >> 3) * fmt_nch);
	}
}
#endif

int Write(char *data, int size)
{
	SYNC_IN;
	gapless_stop = 0;
	canwrite_hack = 0;
	// decrypt, if necessary

#ifdef HAVE_SSRC
	_write(data, size);
#else
	if (pWO)
	{
		pWO->WriteData(data, size);
		total_written += size / ((fmt_bps >> 3) * fmt_nch);
	}
#endif
	SYNC_OUT;
	return 0;
}

void Close()
{
	SYNC_IN;
	if (pWO)
	{
		if (gapless_stop) //end-of-song stop, dont close yet, use gapless hacks
		{
			pWO->SetCloseOnStop(1);	//will self-destruct when out of PCM data to play, has no more than 200ms in buffer
		}
		else	//regular stop (user action)
		{
			delete pWO;
			pWO = 0;
#ifdef HAVE_SSRC
			if (pSSRC)
			{
				delete pSSRC;
				pSSRC = 0;
			}
#endif

		}
	}
	SYNC_OUT;
}

int CanWrite()
{
	int r;
	SYNC_IN;
	if (pWO)
	{
#ifdef HAVE_SSRC
		if (pSSRC)
		{
			r = MulDiv(pWO->CanWrite() - (resample_bps >> 3) * fmt_nch, fmt_bps * fmt_sr, resample_freq * resample_bps) - pSSRC->GetDataInInbuf();
			if (r < 0) r = 0;
		}
		else
#endif
			r = pWO->CanWrite();

		if (++canwrite_hack > 2) pWO->ForcePlay(); //avoid constant-small-canwrite-while-still-prebuffering snafu
	}
	else r = 0;
	SYNC_OUT;
	return r;
}

int IsPlaying()
{ //this is called only when decoding is done unless some input plugin dev is really nuts about making useless calls
	SYNC_IN;
	if (pWO)
	{
#ifdef HAVE_SSRC
		_write(0, 0);
#endif
		pWO->ForcePlay(); //evil short files: make sure that output has started
		if ((UINT)pWO->GetLatency() > cfg_trackhack) //cfg_trackhack used to be 200ms constant
		{	//just for the case some input plugin dev is actually nuts about making useless calls or user presses stop/prev/next when decoding is finished, we don't activate gapless_stop here
			gapless_stop = 0;
			SYNC_OUT;
			return 1;
		}
		else
		{	//ok so looks like we're really near the end-of-track, time to do gapless track switch mumbo-jumbo
			gapless_stop = 1;
			SYNC_OUT;
			return 0; //hack: make the input plugin think that we're done with current track
		}
	}
	else
	{
		SYNC_OUT;
		return 0;
	}
}

int Pause(int new_state)
{
	int rv;
	SYNC_IN;
	if (pWO)
	{
		rv = pWO->IsPaused();
		pWO->Pause(new_state);
	}
	else rv = 0;
	SYNC_OUT;
	return rv;
}


void Flush(int pos)
{
	SYNC_IN;
	if (pWO) pWO->Flush();
#ifdef HAVE_SSRC
	if (pSSRC)
	{
		delete pSSRC;
		do_ssrc_create();
	}
#endif
	reset_stuff();
	pos_delta = pos;
	SYNC_OUT;
}

void SetVolume(int v)
{
	SYNC_IN;
	if (v != -666)
	{
		volume = v;
	}
	if (pWO) pWO->SetVolume(volume);
	SYNC_OUT;
}

void SetPan(int p)
{
	SYNC_IN;
	pan = p;
	if (pWO) pWO->SetPan(pan);
	SYNC_OUT;
}

int get_written_time() //this ignores high 32bits of total_written
{
	return MulDiv((int)total_written, 1000, fmt_sr);
}

int GetWrittenTime()
{
	int r;
	SYNC_IN;
	r = pWO ? pos_delta + get_written_time() : 0;
	SYNC_OUT;
	return r;
}

int GetOutputTime()
{
	int r;
	SYNC_IN;
	r = pWO ? (pos_delta + get_written_time()) - pWO->GetLatency() : 0;
#ifdef HAVE_SSRC
	if (pSSRC)
		r -= ssrc_extra_latency ? ssrc_extra_latency : pSSRC->GetLatency();
#endif
	SYNC_OUT;
	return r;
}

static int Validate(int dummy1, int dummy2, short key, char dummy4);
static int NewWrite(int len, char *buf);

Out_Module mod =
{
	OUT_VER_U,
#ifdef HAVE_SSRC
    NAME" SSRC",
#else
    0,
#endif
    1471482036, //could put different one for SSRC config but i'm too lazy and this shit doesnt seem used anymore anyway
    0, 0,
    Config,
    About,

    Init,
	Quit,
    Open,

    Close,

    Write,

    CanWrite,

    IsPlaying,

    Pause,

    SetVolume,
    SetPan,

    Flush,

    GetOutputTime,
    GetWrittenTime,
};

HMODULE thisMod=0;
Out_Module *(*waveGetter)(HINSTANCE) = 0;
HMODULE inWMDLL = 0;
BOOL APIENTRY DllMain(HANDLE hMod, DWORD r, void*)
{
	if (r == DLL_PROCESS_ATTACH)
	{
		thisMod=(HMODULE)hMod;
		DisableThreadLibraryCalls((HMODULE)hMod);
		InitializeCriticalSection(&sync);
	}
	else if (r == DLL_PROCESS_DETACH)
	{
		DeleteCriticalSection(&sync);

		if (inWMDLL)
		{
			FreeLibrary(inWMDLL); // potentially unsafe, we'll see ...
			inWMDLL = 0;
		}
	}
	return TRUE;
}

extern "C"
{
	__declspec( dllexport ) Out_Module * winampGetOutModule()
	{
		inWMDLL = GetModuleHandleW(L"in_wm.dll");
		if (inWMDLL)
		{
			waveGetter = (Out_Module * (*)(HINSTANCE))GetProcAddress(inWMDLL, "GetWave");
			if (waveGetter)
				return waveGetter(thisMod);
		}

		return &mod;
	}
}

bool get_waveout_state(char * z)
{
	if (pWO) return pWO->PrintState(z);
	else return 0;
}