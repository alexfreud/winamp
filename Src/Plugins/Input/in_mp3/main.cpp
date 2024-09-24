//#define PLUGIN_NAME "Nullsoft MPEG Audio Decoder"
#include "main.h"
#include <time.h>
#include "DecodeThread.h"
#include "api__in_mp3.h"
#include "../Winamp/wa_ipc.h"
#include "../nu/ServiceBuilder.h"
#include "config.h"
#include "AlbumArt.h"
#include "MetadataFactory.h"
#include "../nu/Singleton.h"
#include "RawMediaReader.h"

char lastfn_status[256] = {0};
int lastfn_status_err = 0;
CRITICAL_SECTION g_lfnscs;
CRITICAL_SECTION streamInfoLock;

int lastfn_data_ready;

int config_fastvis=0;
unsigned char config_miscopts=0;
unsigned char allow_sctitles=1;
unsigned char sctitle_format=1;
unsigned char config_eqmode=4,config_http_proxynonport80=1;
unsigned int winampVersion=0x00005010; // default version # to use if winamp version is 5.1 or less (and therefore doesn't have a valid HWND during Init)
char config_http_save_dir[MAX_PATH] = "C:\\";
int config_http_buffersize=64, config_http_prebuffer=40, config_http_prebuffer_underrun=10;
unsigned char config_downmix=0, config_downsample=0, allow_scartwork=1;

int config_max_bufsize_k=128;
int config_gapless=1;
char INI_FILE[MAX_PATH] = {0};

wchar_t lastfn[8192] = {0};	// currently playing file (used for getting info on the current file)

// Used for correcting DSP plug-in pitch changes
int paused = 0;				// are we paused?

int m_is_stream = 0;
bool m_is_stream_seekable = true;

volatile int killDecodeThread=0;			// the kill switch for the decode thread
HANDLE thread_handle=INVALID_HANDLE_VALUE;	// the handle to the decode thread

DWORD WINAPI DecodeThread(LPVOID b); // the decode thread procedure

extern char *getfileextensions();


#include "api/service/waservicefactory.h"

#include "FactoryHelper.h"

// wasabi based services for localisation support
HINSTANCE        WASABI_API_LNG_HINST  = 0;
HINSTANCE        WASABI_API_ORIG_HINST = 0;

api_language    *WASABI_API_LNG        = 0;
api_application *WASABI_API_APP        = 0;
api_config      *AGAVE_API_CONFIG      = 0;
api_memmgr      *WASABI_API_MEMMGR     = 0;

AlbumArtFactory albumArtFactory;
MetadataFactory metadataFactory;

static RawMediaReaderService raw_media_reader_service;
static SingletonServiceFactory<svc_raw_media_reader, RawMediaReaderService> raw_factory;

int init()
{
	if (!IsWindow(mod.hMainWindow))
		return IN_INIT_FAILURE;

	winampVersion = (unsigned int)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETVERSION);
	mod.service->service_register(&metadataFactory);
	mod.service->service_register(&albumArtFactory);
	raw_factory.Register(mod.service, &raw_media_reader_service);

	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,InMp3LangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription, 256, WASABI_API_LNGSTRINGW(IDS_NULLSOFT_MPEG_AUDIO_DECODER), PLUGIN_VERSION);
	mod.description = (char*)szDescription;

	InitializeCriticalSection(&g_lfnscs);
	InitializeCriticalSection(&streamInfoLock);
	mod.UsesOutputPlug|=2;
	config_read();
	mod.FileExtensions=getfileextensions();
	return IN_INIT_SUCCESS;
}

void quit()
{
	DeleteCriticalSection(&g_lfnscs);
	DeleteCriticalSection(&streamInfoLock);
	ServiceRelease(mod.service, AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(mod.service, WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(mod.service, WASABI_API_MEMMGR, memMgrApiServiceGuid);
	mod.service->service_deregister(&albumArtFactory);
	raw_factory.Deregister(mod.service);
}

int g_eq_ok;

int isourfile(const wchar_t *fn)
{
	if (!_wcsnicmp(fn,L"uvox://",7)) return 1;
	if (!_wcsnicmp(fn,L"icy://",6)) return 1;
	if (!_wcsnicmp(fn,L"sc://",5)) return 1;
	if (!_wcsnicmp(fn,L"shoutcast://",12)) return 1;
	return 0;
}


int m_force_seek=-1;

// called when winamp wants to play a file
int play(const in_char *fn)
{
	DWORD thread_id;
	lastfn_status_err=0;
	paused=0;
	g_length=-1000;
	decode_pos_ms=0;
	seek_needed=m_force_seek;
	m_force_seek=-1;
	m_is_stream = 0;
	m_is_stream_seekable = false;
	killDecodeThread=0;
	g_sndopened=0;
	lastfn_data_ready=0;
	lastfn_status[0]=0;
	g_bufferstat=0;
	g_closeaudio=0;
	lstrcpynW(lastfn,fn, 8192);
	mod.is_seekable = 0;
	mod.SetInfo(0,0,0,0);

	g_ds=config_downsample;

	g_eq_ok=1;
	// launch decode thread
	thread_handle = (HANDLE)CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) DecodeThread,NULL,0,&thread_id);
	SetThreadPriority(thread_handle, (int)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));

	return 0;
}

// standard pause implementation
void pause()
{
	paused=1;
	if (g_sndopened)
		mod.outMod->Pause(1);
}

void unpause()
{
	paused=0;
	if (g_sndopened)
		mod.outMod->Pause(0);
}

int ispaused()
{
	return paused;
}

// stop playing.
void stop()
{
	killDecodeThread=1;
	WaitForSingleObject(thread_handle,INFINITE);
	CloseHandle(thread_handle);
	g_eq_ok=0;
	thread_handle = INVALID_HANDLE_VALUE;
	g_length=-1000;
	lastfn[0]=0;
	if (g_closeaudio)
	{
		g_closeaudio=0;
		mod.outMod->Close();
		mod.SAVSADeInit();
	}
	g_sndopened=0;
	m_force_seek=-1;
}


// returns length of playing track
int getlength()
{
	return g_length;
}


// returns current output position, in ms.
// you could just use return mod.outMod->GetOutputTime(),
// but the dsp plug-ins that do tempo changing tend to make
// that wrong.
int getoutputtime()
{
	if (g_bufferstat)
		return g_bufferstat;

	if (!lastfn_data_ready||!g_sndopened)
		return 0;

	if (seek_needed!=-1)
		return seek_needed;

	return decode_pos_ms +
	       (mod.outMod->GetOutputTime()-mod.outMod->GetWrittenTime());
}


// called when the user releases the seek scroll bar.
// usually we use it to set seek_needed to the seek
// point (seek_needed is -1 when no seek is needed)
// and the decode thread checks seek_needed.
void setoutputtime(int time_in_ms)
{


	if (m_is_stream == 0 || (m_is_stream !=0 && m_is_stream_seekable))
	{
		seek_needed=time_in_ms;
		m_force_seek=-1;
	}
}


// standard volume/pan functions
void setvolume(int volume)
{
	mod.outMod->SetVolume(volume);
}
void setpan(int pan)
{
	mod.outMod->SetPan(pan);
}


// this is an odd function. it is used to get the title and/or
// length of a track.
// if filename is either NULL or of length 0, it means you should
// return the info of lastfn. Otherwise, return the information
// for the file in filename.
// if title is NULL, no title is copied into it.
// if length_in_ms is NULL, no length is copied into it.

static int memcmpv(char *d, char v, int l)
{
	while (l--)
		if (*d++ != v) return 1;
	return 0;
}

void eq_set(int on, char data[10], int preamp)
{
	int x;
	eq_preamp = preamp;
	eq_enabled = on;
	for (x = 0; x < 10; x ++)
		eq_tab[x] = data[x];

	// if eq zeroed out, dont use eq
	if (eq_enabled && preamp==31 && !memcmpv(data,31,10))
		eq_enabled=0;
}



// render 576 samples into buf.
// this function is only used by DecodeThread.

// note that if you adjust the size of sample_buffer, for say, 1024
// sample blocks, it will still work, but some of the visualization
// might not look as good as it could. Stick with 576 sample blocks
// if you can, and have an additional auxiliary (overflow) buffer if
// necessary..


// module definition.

extern In_Module mod =
{
	IN_VER_RET,	// defined in IN2.H
	"nullsoft(in_mp3.dll)",
	0,	// hMainWindow (filled in by winamp)
	0,  // hDllInstance (filled in by winamp)
	0,
	// this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
	0,	// is_seekable
	1,	// uses output plug-in system
	config,
	about,
	init,
	quit,
	getfileinfo,
	id3Dlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,

	getlength,
	getoutputtime,
	setoutputtime,

	setvolume,
	setpan,

	0,0,0,0,0,0,0,0,0, // visualization calls filled in by winamp

	0,0, // dsp calls filled in by winamp

	eq_set,

	NULL,		// setinfo call filled in by winamp

	0, // out_mod filled in by winamp
};

// exported symbol. Returns output module.
extern "C"
{
	__declspec(dllexport) In_Module * winampGetInModule2()
	{
		return &mod;
	}
}