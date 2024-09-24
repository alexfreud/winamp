//#define USE_LOG

#include "out_ds.h"
#include "ds2.h"
#include <dsound.h>
#include <math.h>
#include "ds_ipc.h"
#include "../winamp/wa_ipc.h"
#include "res_wa2/resource.h"
#include <shlwapi.h>

extern Out_Module mod;

// wasabi based services for localisation support
api_service *WASABI_API_SVC = 0;
api_application *WASABI_API_APP = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
HINSTANCE cfg_orig_dll = 0;
static wchar_t szDescription[256];

class FORMATSPEC
{
public:
	UINT freq, nch, bps;
	FORMATSPEC(UINT f, UINT n, UINT b) {freq = f;nch = n;bps = b;}
	FORMATSPEC() {freq = 0;nch = 0;bps = 0;}
	bool operator==(FORMATSPEC & foo) { return foo.freq == freq && foo.nch == nch && foo.bps == bps;}
	bool operator!=(FORMATSPEC & foo) { return !(*this == foo);}
	FORMATSPEC & operator=(FORMATSPEC &foo) {freq = foo.freq;bps = foo.bps;nch = foo.nch; return *this;}
	UINT Size() { return nch*(bps >> 3);}
	//	long B2T(long b) {return MulDiv(b,1000,freq*Size());}
	//	long T2B(long t) {return MulDiv(t,freq*Size(),1000);}
};

static FORMATSPEC dataspec;

cfg_struct_t<GUID> cfg_dev2("cfg_dev2", 0);

cfg_int cfg_buf_ms("cfg_buf_ms", 2000);
cfg_int cfg_prebuf2("cfg_prebuf2", 500);
cfg_int cfg_sil_db("cfg_sil_db", 400);
cfg_int cfg_trackhack("cfg_trackhack", 500);
cfg_int cfg_oldpause("cfg_oldpause", 0);
cfg_int cfg_killsil("cfg_killsil", 0);
cfg_int cfg_wait("cfg_wait", 1);
cfg_int cfg_createprimary("cfg_createprimary", (GetVersion()&0x80000000) ? 1 : 0);
cfg_int cfg_volume("cfg_volume", 1);
cfg_int cfg_fadevol("cfg_fadevol", 1);
cfg_int cfg_autocpu("cfg_autocpu", 0);
cfg_int cfg_volmode("cfg_volmode", 0);
cfg_int cfg_logvol_min("cfg_logvol_min", 100);
cfg_int cfg_logfades("cfg_logfades", 0);
cfg_struct_t<__int64> cfg_total_time("cfg_total_time", 0);
cfg_int cfg_hw_mix("cfg_hw_mix", 1);
cfg_int cfg_override("cfg_override", 0);
cfg_int cfg_override_freq("cfg_override_freq", 44100);
cfg_int cfg_override_bps("cfg_override_bps", 16);
cfg_int cfg_override_nch("cfg_override_nch", 2);
cfg_int cfg_refresh("cfg_refresh", 10);
cfg_int cfg_cur_tab("cfg_cur_tab", 0);

void preCreateIPC();
void createIPC();
void destroyIPC();

static int hack_canwrite_count;

static bool is_playing = 0;

#ifdef HAVE_SSRC

cfg_int cfg_use_resample("cfg_use_resample", 0);

#include "../ssrc/ssrc.h"

static Resampler_base* pSSRC;


cfg_int cfg_dither("cfg_dither", 1);
cfg_int cfg_resample_freq("cfg_resample_freq", 48000);
cfg_int cfg_resample_bps("cfg_resample_bps2", 16);

cfg_int cfg_fast("cfg_fast", 1);
cfg_int cfg_pdf("cfg_pdf", 1);

static FORMATSPEC resampled;

#define KILL_SSRC {if (pSSRC) {delete pSSRC;pSSRC=0;}}

static bool finished, use_finish;

static void CREATE_SSRC(FORMATSPEC & out)
{
	//todo: freq/bps range checks ?
	if (pSSRC) {delete pSSRC;pSSRC = 0;}
	if (out != dataspec) pSSRC = SSRC_create(dataspec.freq, out.freq, dataspec.bps, out.bps, dataspec.nch, cfg_dither, cfg_pdf, cfg_fast, 0);
	if (!pSSRC)
	{
		resampled = dataspec;
	}
	else
	{
		if (&resampled != &out) resampled = out;
		finished = 0;
		use_finish = cfg_trackhack == 0 ? 1 : 0;
	}
}


#else

#define KILL_SSRC
#define CREATE_SSRC(X)

#endif

#ifdef HAVE_JOY
void wa2_hack_joy_update();
void wa2_hack_joy_init();
void wa2_hack_joy_deinit();
#endif

static CriticalSection sync; //class from ds2.h
#define SYNC_IN sync.Enter();
#define SYNC_OUT sync.Leave();

#ifdef USE_LOG
#include <iostream>
static void log_write(char* msg)
{
	/*
	char tmp[512];
	SYSTEMTIME st;
	GetSystemTime(&st);
	wsprintf(tmp, "wa2: %u:%02u:%02u.%03u   %s\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, msg);
	OutputDebugString(tmp);
	*/
	std::cout << msg << std::endl;
}

#else

#define log_write(x)

#endif


static UINT fadetimehack;

static int wa2_hint;
enum
{
	HINT_NONE, HINT_EOF, HINT_EOF_GAPLESS
};


void Config(HWND w);

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
	wchar_t message[1024], text[1024];
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_DS_OUTPUT_OLD,text,1024);
	wsprintfW(message, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
			  szDescription, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}

static DS2* pDS2;

static char INI_FILE[MAX_PATH];
static char APP_NAME[MAX_PATH];
void Init()
{
	if (!IsWindow(mod.hMainWindow))
		return;

	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (!WASABI_API_SVC || WASABI_API_SVC == (api_service *)1)
		return;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,OutDSLangGUID);
	cfg_orig_dll = mod.hDllInstance;

	swprintf(szDescription, 256, WASABI_API_LNGSTRINGW(IDS_NULLSOFT_DS_OUTPUT), DS2_ENGINE_VER);
	mod.description = (char*)szDescription;

	log_write("init");
	SYNC_IN;
	char *p;
	if ((p = (char *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE))
		&& p!= (char *)1)
	{
		lstrcpynA(INI_FILE, p, MAX_PATH);
	}
	else
	{
		GetModuleFileNameA(NULL, INI_FILE, sizeof(INI_FILE));
		p = INI_FILE + strlen(INI_FILE);
		while (p >= INI_FILE && *p != '.') p--;
		lstrcpyA(++p, "ini");
	}

	char temp[MAX_PATH];
	GetModuleFileNameA(mod.hDllInstance, temp, sizeof(temp));
	p = temp +strlen(temp);
	while (p && *p != '\\' && p >= temp)
	{
		if (*p == '.')
			*p = 0;
		p = CharPrevA(temp, p);
	}
	if (p != nullptr)
	{
		p = CharNextA(p);
		lstrcpyA(APP_NAME, p);
	}	

	cfg_var::config_read(INI_FILE, APP_NAME);
	DS2::SetTotalTime(cfg_total_time);
	preCreateIPC();
	SYNC_OUT;
}

void Quit()
{
	log_write("quit");
	SYNC_IN;
	destroyIPC();
	if (pDS2)
	{
		pDS2->Release();
		pDS2 = 0;
	}
	KILL_SSRC;

	if (cfg_wait)
	{
		while (DS2::InstanceCount() > 0) Sleep(1);
	}

	cfg_total_time = DS2::GetTotalTime();

	DS2::Quit();

	cfg_var::config_write(INI_FILE,APP_NAME/* "out_ds"*/);

	SYNC_OUT;
#ifdef HAVE_JOY
	wa2_hack_joy_deinit();
#endif
}

int Pause(int);

static int volume = 255, pan=0;
static __int64 pos_delta;
static __int64 samples_written;

static int paused;

void setup_config(DS2config * cfg)
{
#ifdef HAVE_SSRC
	cfg->SetPCM(resampled.freq, resampled.bps, resampled.nch);
#else
	cfg->SetPCM(dataspec.freq, dataspec.bps, dataspec.nch);
#endif
	cfg->SetCreatePrimary(!!cfg_createprimary);
	cfg->SetWindow(mod.hMainWindow);
	cfg->SetDeviceGUID(cfg_dev2);
	int crossfadetime = cfg_fade_stop.usedef ? cfg_def_fade : cfg_fade_stop.time;
	int buffersize = cfg_fade_stop.on ? (crossfadetime > cfg_buf_ms ? crossfadetime : cfg_buf_ms) : cfg_buf_ms;
	cfg->SetBuffer(buffersize, cfg_prebuf2);
	if (cfg_killsil) cfg->SetSilence((float)cfg_sil_db*(float)0.1);
	cfg->SetVolMode(cfg_volmode, cfg_logvol_min, !!cfg_logfades);
	cfg->SetMixing(cfg_hw_mix ? 0 : 2);
	if (cfg_override)
	{
		cfg->SetPrimaryOverride(1);
		cfg->SetPrimaryOverrideFormat(cfg_override_freq, cfg_override_bps, cfg_override_nch);
	}
	cfg->SetCpuManagement(!!cfg_autocpu);
	cfg->SetRefresh(cfg_refresh);
	//	cfg->SetCoop(0);
#ifdef HAVE_JOY
	cfg->SetHavePitch(1);
#endif
}

__int64 get_written_time();

static void do_ssrc_write(char * buf, int len);

int CanResample(int sfrq, int dfrq);

int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{ //messy

	log_write("open");
	SYNC_IN;
#ifdef HAVE_JOY
	wa2_hack_joy_init();
#endif

	is_playing = 0;

	FORMATSPEC newformat(samplerate, numchannels, bitspersamp);
#ifdef HAVE_SSRC
	FORMATSPEC newresampled(cfg_resample_freq, numchannels, cfg_resample_bps);
	if (!cfg_use_resample) newresampled = newformat;
#endif

	DS2 * wait = 0;
	bool nofadein = pDS2 ? 1 : 0;
	bool nosetvol = nofadein;

	if (pDS2)
	{
		pDS2->SetCloseOnStop(0);
		if (pDS2->IsClosed())
		{
			pDS2->Release();pDS2 = 0;
			KILL_SSRC;
		}
		else
		{
			log_write("got ds2");
#ifdef HAVE_SSRC
			if (dataspec != newformat
				&& cfg_fade_stop <= 0 && cfg_fade_start <= 0
				&& resampled == newresampled
				&& CanResample(newformat.freq, newresampled.freq)
				)
			{ //reinit ssrc, dont reinit output
				if (pSSRC)
				{
					use_finish = 1;
					do_ssrc_write(0, 0);
					delete pSSRC;
					pSSRC = 0;
				}

				dataspec = newformat;
				CREATE_SSRC(newresampled); //resampled spec doesn't change, canresample was checked, this cant fail
			}
			else
#endif
				if (dataspec != newformat
#ifdef HAVE_SSRC
					|| resampled != newresampled
#endif
					|| cfg_fade_stop > 0 || cfg_fade_start > 0
					)
				{
#ifdef HAVE_SSRC
					if (pSSRC)
					{
						use_finish = 1;
						do_ssrc_write(0, 0);
						delete pSSRC;
						pSSRC = 0;
					}
#endif
					log_write("using wait");
					wait = pDS2;
					pDS2 = 0;
				}
		}
	}

	if (!pDS2)
	{
		nosetvol = 0;
		log_write("doing new ds2 instance");
		dataspec = newformat;
#ifdef HAVE_SSRC
		CREATE_SSRC(newresampled);
#endif

		DS2config cfg;
		setup_config(&cfg);
		pDS2 = DS2::Create(&cfg);
		if (!pDS2)
		{
			log_write("bork bork");
			if (wait) wait->Release();
			const TCHAR* moo = cfg.GetError();
			if (moo)
			{
				TCHAR errStr[128];
				wsprintf(errStr,WASABI_API_LNGSTRINGW(IDS_ERROR),mod.description);
				MessageBox(0, moo, errStr, MB_ICONERROR);
			}
			KILL_SSRC;
			SYNC_OUT;
			return -1;
		}
	}
	else
	{ //reusing old DS2
#ifdef HAVE_SSRC
		if (pSSRC)
		{
			if (finished)
			{
				//				KILL_SSRC;
				CREATE_SSRC(resampled);
			}
			else use_finish = cfg_trackhack == 0 ? 1 : 0;
		}
#endif
		pDS2->StartNewStream();
		pos_delta -= get_written_time();
	}

	if (!cfg_volume) volume = 255;
	pDS2->SetPan_Int(pan);
	UINT ft = DS2::InstanceCount() > 1 ? cfg_fade_start : cfg_fade_firststart;
	if (ft && !nofadein)
	{
		log_write("fadein");
		pDS2->SetVolume_Int(0);
		pDS2->Fade_Int(ft, volume);
	}
	else if (!nosetvol) pDS2->SetVolume_Int(volume);

	if (wait) pDS2->WaitFor(wait, 0);

	pos_delta = 0;
	samples_written = 0;
	paused = 0;
	log_write("done opening");
	wa2_hint = HINT_NONE;
	hack_canwrite_count = 0;
	is_playing = 1;

#ifdef HAVE_JOY
	wa2_hack_joy_update();
#endif

	int crossfadetime = cfg_fade_stop.usedef ? cfg_def_fade : cfg_fade_stop.time;
	int buffersize = cfg_fade_stop.on ? (crossfadetime > cfg_buf_ms ? crossfadetime : cfg_buf_ms) : cfg_buf_ms;
	int rv = buffersize;
	SYNC_OUT;
	log_write("~open");
	return rv;
}

void Close()
{
	log_write("close");
	SYNC_IN;
	if (pDS2)
	{
		log_write("got ds2");
		pDS2->KillEndGap();
		switch (wa2_hint)
		{
		case HINT_NONE:
			pDS2->FadeAndForget(cfg_fade_pause);
			pDS2 = 0;
			KILL_SSRC;
			break;
		case HINT_EOF:
			pDS2->FadeAndForget(cfg_fade_stop);
			pDS2 = 0;
			KILL_SSRC;
			break;
		case HINT_EOF_GAPLESS:
			if (pDS2->GetLatency() > 0) pDS2->SetCloseOnStop(1);
			else {pDS2->Release();pDS2 = 0;}
			break;
		}
	}
	is_playing = 0;
	SYNC_OUT;
	log_write("done closing");
}

static void make_new_ds2()
{
#ifdef HAVE_SSRC
	//	KILL_SSRC;
	CREATE_SSRC(resampled);
#endif

	DS2config cfg;
	setup_config(&cfg);
	pDS2 = DS2::Create(&cfg);

	if (pDS2)
	{
		pDS2->SetPan_Int(pan);
		pDS2->SetVolume_Int(0);
		pDS2->Fade_Int(fadetimehack, volume);
		fadetimehack = 0;
	}
}

#ifdef HAVE_SSRC

static void do_ssrc_write(char * buf, int len)
{
	if (!finished && pSSRC)
	{
		UINT nsiz;
		if (len > 0) pSSRC->Write(buf, (UINT)len);
		else if (use_finish)
		{
			finished = 1;
			pSSRC->Finish();
		}

		char * data = (char*)pSSRC->GetBuffer(&nsiz);
		if (nsiz) pDS2->ForceWriteData(data, nsiz);
		pSSRC->Read(nsiz);
	}
}
#endif


int Write(char *buf, int len)
{
	log_write("write");
	SYNC_IN;
	hack_canwrite_count = 0;
	wa2_hint = 0;
	if (paused)
	{
		SYNC_OUT;
		return 1;
	}
	if (!pDS2)
	{
		log_write("write: need new object");
		make_new_ds2();
		if (!pDS2 || !buf || !len)
		{
			SYNC_OUT;
			return 0;
		}
	}
	samples_written += len / dataspec.Size();
	int rv = 0;
	if (buf && len > 0)
	{

#ifdef HAVE_SSRC
		if (pSSRC) do_ssrc_write(buf, len);
		else
#endif
			rv = !pDS2->ForceWriteData(buf, len); //flood warning
	}
	SYNC_OUT;
	return rv;
}

int CanWrite()
{
	log_write("canwrite");
	int rv = 0;
	SYNC_IN;
	if (!paused)
	{
		if (!pDS2)
		{
			make_new_ds2();
			hack_canwrite_count = -1;
		}
		if (pDS2)
		{
#ifdef HAVE_SSRC
			if (pSSRC)
			{
				rv = MulDiv(pDS2->CanWrite() - resampled.Size(), dataspec.bps * dataspec.freq, resampled.bps * resampled.freq) - pSSRC->GetDataInInbuf();
			}
			else
#endif
				rv = pDS2->CanWrite();
			if (rv < 0) rv = 0;
			if (++hack_canwrite_count > 2 && pDS2->BufferStatusPercent() > 50) pDS2->ForcePlay(); //big prebuffer hack
		}
	}
	SYNC_OUT;
	return rv;
}

int IsPlaying()
{
	log_write("isplaying");
	int rv = 0;
	SYNC_IN;
	if (pDS2)
	{
		int foo = cfg_fade_stop;
		pDS2->KillEndGap();
		pDS2->ForcePlay();
		int lat = pDS2->GetLatency();
		wa2_hint = HINT_EOF;
		if (foo > 0)
		{
			rv = lat > foo;
		}
		else if (lat > (int)cfg_trackhack)
		{
			rv = 1;
		}
		else
		{
			wa2_hint = HINT_EOF_GAPLESS;
			rv = 0;
		}
	}
	SYNC_OUT;
	return rv;
}

int Pause(int new_state)
{
	log_write("pause");
	SYNC_IN;
	int rv = paused;
	paused = new_state;
	if (new_state)
	{
		if (pDS2)
		{
			UINT ft = cfg_fade_pause;
			if (!ft)
			{
				pDS2->Pause(1);
			}
			else if (cfg_oldpause)
			{
				pDS2->FadeAndForget(ft);
				pDS2 = 0;
				KILL_SSRC;
				fadetimehack = ft;
			}
			else
			{
				pDS2->FadePause(ft);
			}
		}
	}
	else
	{
		if (pDS2) pDS2->Pause(0);
	}
	SYNC_OUT;
	return rv;
}

void SetVolume(int _volume) // volume is 0-255
{
	log_write("setvolume");
	SYNC_IN;
	if (_volume != -666 && cfg_volume)
	{
		volume = _volume;
		if (pDS2)
		{
			if (cfg_fadevol) pDS2->FadeX_Int(150, _volume);
			else pDS2->SetVolume_Int(_volume);
		}
	}
	SYNC_OUT;
}

void SetPan(int _pan) // pan is -128 to 128
{
	log_write("setpan");
	SYNC_IN;
	if (cfg_volume)
	{
		pan = _pan;
		if (pDS2) pDS2->SetPan_Int(pan);
	}
	SYNC_OUT;
}

void Flush(int t)
{
	log_write("flush");
	SYNC_IN;
	if (pDS2)
	{
		UINT t = cfg_fade_seek;
		pDS2->FadeAndForget(t);
		pDS2 = 0;
		fadetimehack = t;
	}
#ifdef HAVE_SSRC
	//	KILL_SSRC;
	CREATE_SSRC(resampled);
#endif
	samples_written = 0;
	pos_delta = t;
	SYNC_OUT;
}

__int64 get_written_time()
{
	return dataspec.freq ? samples_written*1000 / (__int64)dataspec.freq : 0;
}

static int GetWrittenTime()
{
	log_write("getwrittentime");
	int rv;
	SYNC_IN;
	rv = is_playing ? (int)(pos_delta + get_written_time()) : 0;
	SYNC_OUT;
	log_write("~getwrittentime");
	return rv;
}

static __int64 GetOutputTime64()
{
	if (!is_playing) return 0;
	__int64 rv = get_written_time();
	if (pDS2) rv -= pDS2->GetLatency();
#ifdef HAVE_SSRC
	if (pSSRC) rv -= pSSRC->GetLatency();
#endif
	if (rv < 0) rv = 0;
	return rv;
}

static int GetOutputTime()
{
	log_write("getoutputtime");
	SYNC_IN;
	int rv = (int)(pos_delta + GetOutputTime64());
	SYNC_OUT;
	log_write("!getoutputtime");
	return rv;
}



Out_Module mod =
{
	OUT_VER_U,
	0
	/*NAME
#ifdef HAVE_SSRC
	" SSRC"
#endif
#ifdef HAVE_JOY
	" JOY"
#endif*/
	,
	203968848,
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
 
static HMODULE inWMDLL = 0;
BOOL APIENTRY DllMain(HANDLE hMod, DWORD r, void*)
{
	if (r == DLL_PROCESS_ATTACH)
	{
		thisMod=(HMODULE)hMod;
		DisableThreadLibraryCalls((HMODULE)hMod);
	}

	if (r == DLL_PROCESS_DETACH)
	{
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
	__declspec(dllexport) Out_Module * winampGetOutModule()
	{
		HMODULE in_wm = GetModuleHandleW(L"in_wm.dll");
		if (in_wm)
		{
			Out_Module *(*dsGetter)(HINSTANCE) = (Out_Module * (*)(HINSTANCE))GetProcAddress(in_wm, "GetDS");
			if (dsGetter) {
				Out_Module *in_wm_ds = dsGetter(thisMod);
				if (in_wm_ds) {
					inWMDLL = in_wm;
					return in_wm_ds;
				}
			}
		}
		return &mod;
	}
}

bool wa2_GetRealtimeStat(DS2_REALTIME_STAT * stat) //for config
{
	bool rv = 0;
	SYNC_IN;
	if (pDS2 && !pDS2->IsClosed())
	{
		rv = 1;
		pDS2->GetRealtimeStat(stat);
	}
	SYNC_OUT;
	return rv;
}

#ifdef HAVE_SSRC
bool wa2_IsResampling(RESAMPLING_STATUS *foo)
{
	bool rv;
	SYNC_IN;
	if (pSSRC)
	{
		foo->src_freq = dataspec.freq;
		foo->src_bps = dataspec.bps;
		foo->dst_freq = resampled.freq;
		foo->dst_bps = resampled.bps;
		rv = 1;
	}
	else rv = 0;
	SYNC_OUT;
	return rv;
}
#endif

#ifdef HAVE_JOY
void wa2_hack_setpitch(double d)
{
	SYNC_IN;
	if (pDS2) pDS2->SetPitch(d);
	SYNC_OUT;
}
#endif

void wa2_sync_in() {SYNC_IN;}
void wa2_sync_out() {SYNC_OUT;}

HWND ipcWnd = NULL;

extern void set_buffer(HWND wnd, UINT b);
extern void update_buf(HWND wnd);
extern HWND buffer_config_wnd;
extern HWND fades_config_wnd;
extern UINT cur_buffer;
extern void update_prebuf_range(HWND wnd);

typedef struct
{
	const wchar_t * name;
	int on, usedef;
	int time;
}
FadeCfgCopy;

extern void format_fade(wchar_t * txt, FadeCfgCopy * c, int idx);

LRESULT CALLBACK ipcProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DS_IPC:
		switch (lParam)
		{
		case DS_IPC_CB_CFGREFRESH:
			// trap me !
			return 0;
		case DS_IPC_CB_ONSHUTDOWN:
			// trap me !
			return 0;
		case DS_IPC_SET_CROSSFADE:
			wa2_sync_in();
			cfg_fade_stop.on = (int)wParam;
			// update the config wnd if it is showing the fades page
			if (fades_config_wnd)
			{
				HWND list = GetDlgItem(fades_config_wnd, IDC_LIST);
				int cursel = (int)SendMessage(list, LB_GETCURSEL, 0, 0);
				FadeCfgCopy * c = (FadeCfgCopy*)SendMessage(list, LB_GETITEMDATA, 2, 0);
				c->on = (int)wParam;
				c->usedef = cfg_fade_stop.usedef;
				c->time = cfg_fade_stop.time;
				wchar_t txt[256];
				format_fade(txt, c, 2);
				SendMessage(list, LB_DELETESTRING, 2, 0);
				SendMessage(list, LB_INSERTSTRING, 2, (LPARAM)txt);
				SendMessage(list, LB_SETITEMDATA, 2, (LPARAM)c);
				if (cursel == 2)
				{
					CheckDlgButton(fades_config_wnd, IDC_FADE_ENABLED, c->on);
					CheckDlgButton(fades_config_wnd, IDC_USE_CUSTOM_FADE, !c->usedef);
					SetDlgItemInt(fades_config_wnd, IDC_CUSTOM_FADE, c->time, 0);
				}
				SendMessage(list, LB_SETCURSEL, cursel, 0);
			}
			wa2_sync_out();
			return 0;
		case DS_IPC_SET_CROSSFADE_TIME:
			wa2_sync_in();
			cfg_fade_stop.usedef = 0;
			cfg_fade_stop.time = (int)wParam;
			wa2_sync_out();
			return 0;
		case DS_IPC_GET_CROSSFADE:
			return cfg_fade_stop.on;
		case DS_IPC_GET_CROSSFADE_TIME:
			if (cfg_fade_stop.usedef) return cfg_def_fade;
			return cfg_fade_stop.time;
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

VOID CALLBACK preCreateIPCTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(NULL, idEvent);
	createIPC();
}


void preCreateIPC()
{
	SetTimer(NULL, 1, 1, preCreateIPCTimerProc);
}

void createIPC()
{
	WNDCLASSA wc;
	if ( !GetClassInfoA( mod.hDllInstance, DS_IPC_CLASSNAME, &wc ) )
	{
		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = ipcProc;
		wc.hInstance = mod.hDllInstance;
		wc.lpszClassName = DS_IPC_CLASSNAME;
		wc.style = 0;
		ATOM atom = RegisterClassA( &wc );
	}

	ipcWnd = CreateWindowA(DS_IPC_CLASSNAME, NULL, WS_CHILD, 0, 0, 1, 1, mod.hMainWindow, NULL, mod.hDllInstance, NULL);
	PostMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_CB_OUTPUTCHANGED);
}

void destroyIPC()
{
	if (ipcWnd)
	{
		if (IsWindow(mod.hMainWindow))
			DestroyWindow(ipcWnd); ipcWnd = NULL;
	}
	// this is disabled because otherwise win98 can fail the next registerclass,
	// so at creation, we just check if the class exists or not
	// UnregisterClass(DS_IPC_CLASSNAME, mod.hDllInstance);
}