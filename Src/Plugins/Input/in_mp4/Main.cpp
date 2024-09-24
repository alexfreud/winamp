#define PLUGIN_VERSION L"2.70"
#include "Main.h"
#include <windows.h>
#include <stdio.h>
#include <locale.h>
#include "resource.h"
#include "../Winamp/in2.h"
#include "../Winamp/wa_ipc.h"
#include "../nu/AutoChar.h"
#include "api__in_mp4.h"

#include <api/service/waservicefactory.h>

#pragma warning(disable:4786)
#include "mpeg4audio.h"

#include <shlwapi.h>
#include <malloc.h>
#include "VirtualIO.h"
#include "AlbumArt.h"
#include <assert.h>
#include "../in_wmvdrm/Remaining.h"
#include "VideoThread.h"
#include "RawMediaReader.h"
#include "../nu/Singleton.h"
#include <strsafe.h>

Remaining remaining;
nu::VideoClock video_clock;

AlbumArtFactory albumArtFactory;

wchar_t m_ini[MAX_PATH] = {0};
int infoDlg(const wchar_t *fn, HWND hwnd);
#define WM_WA_IPC WM_USER
#define WM_WA_MPEG_EOF WM_USER+2

HANDLE hThread;
static DWORD WINAPI playProc(LPVOID lpParameter);
static int paused, m_kill;
HANDLE killEvent, seekEvent, pauseEvent;
static int m_opened;


static RawMediaReaderService raw_media_reader_service;
static SingletonServiceFactory<svc_raw_media_reader, RawMediaReaderService> raw_factory;
static void stop();

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

api_config *AGAVE_API_CONFIG = 0;
api_memmgr *WASABI_API_MEMMGR = 0;
api_application *WASABI_API_APP = 0;

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMS msgbx = {sizeof(MSGBOXPARAMS),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCE(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirect(&msgbx);
}

void about(HWND hwndParent)
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_MPEG4_AUDIO_DECODER_OLD,text,1024);
	StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					 mod.description, TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}

static const wchar_t defaultExtensions_nonpro[] = {L"M4A;MP4"};
static const wchar_t defaultExtensions_pro[] = {L"M4A;MP4;M4V"};
const wchar_t *defaultExtensions = defaultExtensions_pro;


// the return pointer has been malloc'd.  Use free() when you are done.
char *BuildExtensions(const char *extensions)
{
	char name[64] = {0};
	WASABI_API_LNGSTRING_BUF(IDS_MP4_FILE,name,64);
	size_t length = strlen(extensions) + 1 + strlen(name) + 2;
	char *newExt = (char *)calloc(length, sizeof(char));
	char *ret = newExt; // save because we modify newExt

	// copy extensions
	StringCchCopyExA(newExt, length, extensions, &newExt, &length, 0);
	newExt++;
	length--;

	// copy description
	StringCchCopyExA(newExt, length, name, &newExt, &length, 0);
	newExt++;
	length--;

	// double null terminate
	assert(length == 1);
	*newExt = 0;

	return ret;
}

int init()
{
	if (!IsWindow(mod.hMainWindow))
		return IN_INIT_FAILURE;

	killEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	seekEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	pauseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

	mod.service->service_register(&albumArtFactory);
	raw_factory.Register(mod.service, &raw_media_reader_service);

	waServiceFactory *sf = mod.service->service_getServiceByGuid(AgaveConfigGUID);
	if (sf)
		AGAVE_API_CONFIG = (api_config *)sf->getInterface();
	sf = mod.service->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf)
		WASABI_API_APP = (api_application *)sf->getInterface();
	sf = mod.service->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf)
		WASABI_API_MEMMGR = (api_memmgr *)sf->getInterface();
	sf = mod.service->service_getServiceByGuid(DownloadManagerGUID);
	if (sf)
		WAC_API_DOWNLOADMANAGER = (api_downloadManager *)sf->getInterface();
	sf = mod.service->service_getServiceByGuid(ThreadPoolGUID);
	if (sf)
		WASABI_API_THREADPOOL = (api_threadpool *)sf->getInterface();

	// loader so that we can get the localisation service api for use
	sf = mod.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,InMp4LangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_MPEG4_AUDIO_DECODER),PLUGIN_VERSION);
	mod.description = (char*)szDescription;

	const wchar_t *inipath = (wchar_t *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETINIDIRECTORYW);

	PathCombineW(m_ini, inipath, L"Plugins");
	CreateDirectoryW(m_ini, NULL);
	PathAppendW(m_ini, L"in_mp4.ini");

	wchar_t exts[1024] = {0};
	GetPrivateProfileStringW(L"in_mp4", L"extensionlist", defaultExtensions, exts, 1024, m_ini);
	mod.FileExtensions = BuildExtensions(AutoChar(exts));
	return IN_INIT_SUCCESS;
}

void quit()
{
	CloseHandle(killEvent);
	CloseHandle(seekEvent);

	raw_factory.Deregister(mod.service);
	waServiceFactory *sf = mod.service->service_getServiceByGuid(AgaveConfigGUID);
	if (sf)
		sf->releaseInterface(AGAVE_API_CONFIG);
	sf = mod.service->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf)
		sf->releaseInterface(WASABI_API_APP);
	sf = mod.service->service_getServiceByGuid(DownloadManagerGUID);
	if (sf)
		sf->releaseInterface( WAC_API_DOWNLOADMANAGER );
	mod.service->service_deregister(&albumArtFactory);

	free(mod.FileExtensions);
}

int isourfile(const wchar_t *fn)
{
	return 0;
}

void config(HWND hwndParent);

void setoutputtime(int time_in_ms)
{
	m_needseek = time_in_ms;
	SetEvent(seekEvent);
}

MP4TrackId GetVideoTrack(MP4FileHandle infile)
{
	int numTracks = MP4GetNumberOfTracks(infile, NULL,      /* subType */ 0);

	for (int i = 0; i < numTracks; i++)
	{
		MP4TrackId trackId = MP4FindTrackId(infile, i, NULL,      /* subType */ 0);
		const char* trackType = MP4GetTrackType(infile, trackId);

		if (!lstrcmpA(trackType, MP4_VIDEO_TRACK_TYPE))
			return trackId;
	}

	/* can't decode this */
	return MP4_INVALID_TRACK_ID;
}

MP4TrackId GetAudioTrack(MP4FileHandle infile)
{
	int ret = MP4_INVALID_TRACK_ID;
	__try
	{
			/* find AAC track */
	int numTracks = MP4GetNumberOfTracks(infile, NULL,      /* subType */ 0);

	for (int i = 0; i < numTracks; i++)
	{
		MP4TrackId trackId = MP4FindTrackId(infile, i, NULL,      /* subType */ 0);
		if (trackId != MP4_INVALID_TRACK_ID)
		{
			const char* trackType = MP4GetTrackType(infile, trackId);

			if (trackType && !lstrcmpA(trackType, MP4_AUDIO_TRACK_TYPE))
				return trackId;
		}

	}

	/* can't decode this */
	return MP4_INVALID_TRACK_ID;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return MP4_INVALID_TRACK_ID;
	}
	return ret;
}

MP4SampleId numSamples, numVideoSamples;
MP4FileHandle MP4hFile;
MP4TrackId audio_track, video_track;

double m_length;
volatile int m_needseek = -1;
unsigned int audio_srate, audio_nch, audio_bps, audio_bitrate=0;
unsigned int video_bitrate=0;

wchar_t lastfn[MAX_PATH*4] = L"";

MP4AudioDecoder *audio = 0;
waServiceFactory *audioFactory = 0, *videoFactory = 0;
MP4VideoDecoder *video = 0;

uint32_t m_timescale = 0, m_video_timescale = 0;
static void *reader = 0;
bool audio_chunk = false;
enum
{
	READER_UNICODE=0,
	READER_HTTP=1,
};
int reader_type=READER_UNICODE;

bool open_mp4(const wchar_t *fn)
{
	audio = 0;
	video = 0;
	if (!_wcsnicmp(fn, L"http://", 7) || !_wcsnicmp(fn, L"https://", 8))
	{
		reader = CreateReader(fn, killEvent);
		reader_type=READER_HTTP;
		MP4hFile = MP4ReadEx(fn, reader, &HTTPIO);
	}
	else
	{
		reader = CreateUnicodeReader(fn);
		if (!reader)
			return false;
		reader_type=READER_UNICODE;
		MP4hFile = MP4ReadEx(fn, reader, &UnicodeIO);
	}

	if (!MP4hFile)
	{
		return false;
	}

	m_opened = 1;

	unsigned int output_bits = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
	if (output_bits  >= 24)	
		output_bits  = 24;
	else	
		output_bits  = 16;

	unsigned int max_channels;
		// get max channels
	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"surround", true))
		max_channels = 6;
	else if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false))
		max_channels = 1;
	else
		max_channels = 2;

	audio_track = GetAudioTrack(MP4hFile);
	if (audio_track == MP4_INVALID_TRACK_ID || !CreateDecoder(MP4hFile, audio_track, audio, audioFactory)	|| audio->OpenMP4(MP4hFile, audio_track, output_bits, max_channels, false) != MP4_SUCCESS)
	{
		audio = 0;
		video_clock.Start();
	}

	video_track = GetVideoTrack(MP4hFile);
	if (video_track != MP4_INVALID_TRACK_ID)
	{
		CreateVideoDecoder(MP4hFile, video_track, video, videoFactory);
		if (video)
			video->Open(MP4hFile, video_track);
	}
	else
		video=0;

	if (!audio && !video)
	{
		return false;
	}

	numVideoSamples = MP4GetTrackNumberOfSamples(MP4hFile, video_track);
	m_video_timescale = MP4GetTrackTimeScale(MP4hFile, video_track);
	unsigned __int64 trackDuration;

	double lengthAudio = 0;
	double lengthVideo = 0;
	
	if (audio_track != MP4_INVALID_TRACK_ID)
	{
		if (audio)
		{
			ConfigureDecoderASC(MP4hFile, audio_track, audio);
			audio_chunk = !!audio->RequireChunks();
		}
		else
			audio_chunk = false;

		numSamples = audio_chunk?MP4GetTrackNumberOfChunks(MP4hFile, audio_track):MP4GetTrackNumberOfSamples(MP4hFile, audio_track);
		m_timescale = MP4GetTrackTimeScale(MP4hFile, audio_track);
		trackDuration = MP4GetTrackDuration(MP4hFile, audio_track);
		lengthAudio = (double)(__int64)trackDuration / (double)m_timescale;
	}
	else
	{
		numSamples = numVideoSamples;
		trackDuration = MP4GetTrackDuration(MP4hFile, video_track);
		lengthVideo = (double)(__int64)trackDuration / (double)m_video_timescale;
	}
	
	/* length in Sec. */
	m_length = max(lengthAudio, lengthVideo); //(double)(__int64)trackDuration / (double)m_timescale;

	audio_bitrate = MP4GetTrackBitRate(MP4hFile, audio_track) / 1000;
	if (video)
		video_bitrate = MP4GetTrackBitRate(MP4hFile, video_track) / 1000;
	else
	video_bitrate = 0;

	if (audio && audio->SetGain(GetGain(MP4hFile)) == MP4_SUCCESS)
		mod.UsesOutputPlug |= 8;
	else
		mod.UsesOutputPlug &= ~8;

	return true;
}

int play(const wchar_t *fn)
{
	video_clock.Reset();
	if (!videoOutput) // grab this now while we're on the main thread
		videoOutput = (IVideoOutput *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_IVIDEOOUTPUT);
	audio = 0;
	video = 0;
	paused = 0;
	m_kill = 0;
	ResetEvent(killEvent);
	m_length = 0;
	ResetEvent(seekEvent);
	m_needseek = -1;
	SetEvent(pauseEvent);
	if (m_force_seek != -1)
	{
		setoutputtime(m_force_seek);
	}
	m_opened = 0;

	lstrcpynW(lastfn, fn, MAX_PATH*4);

	DWORD thread_id;
	HANDLE threadCreatedEvent = CreateEvent(0, FALSE, FALSE, 0);
	hThread = CreateThread(NULL, NULL, PlayProc, (LPVOID)threadCreatedEvent, NULL, &thread_id);
	SetThreadPriority(hThread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	WaitForSingleObject(threadCreatedEvent, INFINITE);
	CloseHandle(threadCreatedEvent);

	return 0;
}

static inline wchar_t *IncSafe(wchar_t *val, int x)
{
	while (x--)
	{
		if (*val)
			val++;
	}

	return val;
}

void GetGaps(MP4FileHandle mp4, unsigned __int32 &pre, unsigned __int32 &post)
{
	wchar_t gap_data[128] = {0};
	if (GetCustomMetadata(mp4, "iTunSMPB", gap_data, 128) && gap_data[0])
	{
		wchar_t *itr = IncSafe(gap_data, 9);

		pre = wcstoul(itr, 0, 16);

		itr = IncSafe(itr, 9);
		post = wcstoul(itr, 0, 16);

		// don't care about total number of samples, really
		/*
		itr+=9;
		unsigned int numSamples = wcstoul(itr, 0, 16);*/

	}
	else
	{
		pre = 0;
		post = 0;
	}
}

float GetGain(MP4FileHandle mp4, bool allowDefault)
{
	if (AGAVE_API_CONFIG && AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain", false))
	{
		float dB = 0, peak = 1.0f;
		wchar_t gain[128] = L"", peakVal[128] = L"";
		_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();

		switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_source", 0))
		{
		case 0:   // track
			if ((!GetCustomMetadata(mp4, "replaygain_track_gain", gain, 128) || !gain[0])
			    && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				GetCustomMetadata(mp4, "replaygain_album_gain", gain, 128);

			if ((!GetCustomMetadata(mp4, "replaygain_track_peak", peakVal, 128) || !peakVal[0])
			    && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				GetCustomMetadata(mp4, "replaygain_album_peak", peakVal, 128);
			break;
		case 1:
			if ((!GetCustomMetadata(mp4, "replaygain_album_gain", gain, 128) || !gain[0])
			    && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				GetCustomMetadata(mp4, "replaygain_track_gain", gain, 128);

			if ((!GetCustomMetadata(mp4, "replaygain_album_peak", peakVal, 128) || !peakVal[0])
			    && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				GetCustomMetadata(mp4, "replaygain_track_peak", peakVal, 128);
			break;
		}

		if (gain[0])
		{
			if (gain[0] == L'+')
				dB = (float)_wtof_l(&gain[1],C_locale);
			else
				dB = (float)_wtof_l(gain,C_locale);
		}
		else if (allowDefault)
		{
			dB = AGAVE_API_CONFIG->GetFloat(playbackConfigGroupGUID, L"non_replaygain", -6.0);
			return powf(10.0f, dB / 20.0f);
		}

		if (peakVal[0])
		{
			peak = (float)_wtof_l(peakVal,C_locale);
		}

		switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_mode", 1))
		{
		case 0:   // apply gain
			return powf(10.0f, dB / 20.0f);
		case 1:   // apply gain, but don't clip
			return min(powf(10.0f, dB / 20.0f), 1.0f / peak);
		case 2:   // normalize
			return 1.0f / peak;
		case 3:   // prevent clipping
			if (peak > 1.0f)
				return 1.0f / peak;
			else
				return 1.0f;
		}

	}

	return 1.0f; // no gain
}




bool first;

void pause()
{
	paused = 1;
	if (audio)
	{
		mod.outMod->Pause(1);
	}
	else
	{
		video_clock.Pause();
	}
	ResetEvent(pauseEvent); // pauseEvent signal state is opposite of pause state
}

void unpause()
{
	paused = 0;
	if (audio)
	{
		mod.outMod->Pause(0);
	}
	else
	{
		video_clock.Unpause();
	}
	SetEvent(pauseEvent); // pauseEvent signal state is opposite of pause state
}

int ispaused()
{
	return paused;
}

void stop()
{
	if (reader && reader_type==READER_HTTP) StopReader(reader);

	lastfn[0] = 0;
	SetEvent(killEvent);
	m_kill = 1;
	WaitForSingleObject(hThread, INFINITE);

	mod.outMod->Close();
	mod.SAVSADeInit();

	if (m_opened) MP4Close(MP4hFile);
	MP4hFile = 0;
	m_opened = 0;

	if (audio)
	{
		audio->Close();
		audioFactory->releaseInterface(audio);
	}

	audioFactory = 0;
	audio = 0;

	if (video)
	{
		video->Close();
		videoFactory->releaseInterface(video);
	}
	videoFactory=0;
	video = 0;

	if (reader) 
	{
		if (reader_type == READER_HTTP)
			DestroyReader(reader);
		else
			DestroyUnicodeReader(reader);
	}
	reader = 0;
}

int getlength()
{
	return (int)(m_length*1000);
}

int getoutputtime()
{
	if (m_needseek == -1)
			return (int)GetClock();
	else
		return m_needseek; // this prevents the seekbar from jumping around while the playthread is seeking
}

void setvolume(int volume)
{
	mod.outMod->SetVolume(volume);
}
void setpan(int pan)
{
	mod.outMod->SetPan(pan);
}
/*
void FillInfo(HWND hwndDlg, MP4FileHandle hMp4);
void CALLBACK CurrentlyPlayingInfoBox(ULONG_PTR param)
{
	ThreadInfoBox *threadInfo = (ThreadInfoBox *)param;
	FillInfo(threadInfo->hwndDlg, MP4hFile);
	SetEvent(threadInfo->completionEvent);
}
*/
void getfileinfo(const wchar_t *filename, wchar_t *title, int *length_in_ms)
{
	if (!filename || !*filename)  // currently playing file
	{
		if (length_in_ms) *length_in_ms = getlength();
		if (title) // get non-path portion.of filename
		{
			lstrcpynW(title, lastfn, GETFILEINFO_TITLE_LENGTH);
			PathStripPathW(title);
			PathRemoveExtensionW(title);
		}
	}
	else // some other file
	{
		if (length_in_ms) // calculate length
		{
			*length_in_ms = -1000; // the default is unknown file length (-1000).

			MP4FileHandle hMp4 = MP4Read(filename);
			if (hMp4)
			{
				double lengthAudio = 0;
				double lengthVideo = 0;
				MP4TrackId audio_track = GetAudioTrack(hMp4);
				if (audio_track != -1)
				{
					int timescale = MP4GetTrackTimeScale(hMp4, audio_track);
					unsigned __int64 trackDuration = MP4GetTrackDuration(hMp4, audio_track);
					lengthAudio = (double)(__int64)trackDuration / (double)timescale;
				}
				MP4TrackId video_track = GetVideoTrack(hMp4);
				if (video_track != -1)
				{
					int timescale = MP4GetTrackTimeScale(hMp4, video_track);
					unsigned __int64 trackDuration = MP4GetTrackDuration(hMp4, video_track);
					lengthVideo = (double)(__int64)trackDuration / (double)timescale;
				}
				*length_in_ms = (int)(max(lengthAudio, lengthVideo) * 1000);
				MP4Close(hMp4);
			}
		}
		if (title) // get non path portion of filename
		{
			lstrcpynW(title, filename, GETFILEINFO_TITLE_LENGTH);
			PathStripPathW(title);
			PathRemoveExtensionW(title);
		}
	}
}

void eq_set(int on, char data[10], int preamp)
{}

// module definition.

In_Module mod =
{
	IN_VER_RET, // defined in IN2.H
	"nullsoft(in_mp4.dll)",		//"Nullsoft MPEG-4 Audio Decoder v1.22"
	0,    	// hMainWindow (filled in by winamp)
	0,      // hDllInstance (filled in by winamp)
	0,	    // this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
	1,    	// is_seekable
	1,    	// uses output plug-in system
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
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

	0, 0, 0, 0, 0, 0, 0, 0, 0,     // visualization calls filled in by winamp

	0, 0,     // dsp calls filled in by winamp

	eq_set,

	NULL,    		// setinfo call filled in by winamp

	0 // out_mod filled in by winamp
};

extern "C"
{
	__declspec(dllexport) In_Module * winampGetInModule2()
	{
		return &mod;
	}
}