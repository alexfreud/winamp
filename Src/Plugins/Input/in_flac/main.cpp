/*
** Copyright © 2007-2014 Winamp SA
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/
#include "main.h"
#include "api__in_flv.h"
#include "Metadata.h"
#include "../Agave/Language/api_language.h"
#include <api/service/waServiceFactory.h>
#include "../Winamp/wa_ipc.h"
#include "../nu/Singleton.h"
#include "../nu/Autochar.h"
#include <shlwapi.h>
#include "resource.h"
#include "AlbumArt.h"
#include "RawReader.h"
#include <strsafe.h>
#include "nswasabi/ReferenceCounted.h"
#include "mkv_flac_decoder.h"

api_config *AGAVE_API_CONFIG = 0;
api_memmgr *WASABI_API_MEMMGR=0;
AlbumArtFactory albumArtFactory;
static RawMediaReaderService raw_media_reader_service;
static SingletonServiceFactory<svc_raw_media_reader, RawMediaReaderService> raw_factory;
static MKVDecoder mkv_service;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoder> mkv_factory;
// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

HANDLE killswitch=0;
HANDLE playThread=0;

const wchar_t *winampINI=0;
void Config(HWND hwndParent);
void About(HWND hwndParent);
wchar_t pluginName[256] = {0};

int Init()
{
	if (!IsWindow(plugin.hMainWindow))
		return IN_INIT_FAILURE;

	killswitch = CreateEvent(0, TRUE, FALSE, 0);
	plugin.service->service_register(&albumArtFactory);
	raw_factory.Register(plugin.service, &raw_media_reader_service);
	mkv_factory.Register(plugin.service, &mkv_service);

	waServiceFactory *sf = (waServiceFactory *)plugin.service->service_getServiceByGuid(AgaveConfigGUID);
	if (sf)	AGAVE_API_CONFIG= (api_config *)sf->getInterface();
	sf = (waServiceFactory *)plugin.service->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf)	WASABI_API_MEMMGR= (api_memmgr *)sf->getInterface();

	// loader so that we can get the localisation service api for use
	sf = plugin.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,InFlacLangGUID);

	StringCchPrintfW(pluginName,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_FLAC_DECODER),PLUGIN_VER);
	plugin.description = (char*)pluginName;

	winampINI = (const wchar_t *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILEW);

	wchar_t exts[1024] = {0};
	GetPrivateProfileStringW(L"in_flac", L"extensions", DEFAULT_EXTENSIONSW, exts, 1024, winampINI);
	plugin.FileExtensions = BuildExtensions(AutoChar(exts));

	config_average_bitrate = !!GetPrivateProfileIntW(L"in_flac", L"average_bitrate", 1, winampINI);

	plugin.UsesOutputPlug|=8;
	return IN_INIT_SUCCESS;
}

void Quit()
{
	CloseHandle(killswitch);
	waServiceFactory *sf = (waServiceFactory *)plugin.service->service_getServiceByGuid(AgaveConfigGUID);
	if (sf)	sf->releaseInterface(AGAVE_API_CONFIG);
	sf = (waServiceFactory *)plugin.service->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf)	sf->releaseInterface(WASABI_API_MEMMGR);

	plugin.service->service_deregister(&albumArtFactory);
	raw_factory.Deregister(plugin.service);
}

void GetFileInfo(const in_char *file, in_char *title, int *length_in_ms)
{
	if (length_in_ms)
	{
		if (!file || !*file && currentSongLength != -1000)
			*length_in_ms = currentSongLength;
		else
		{
			FLACMetadata metadata;
			unsigned __int64 length_in_msec;
			if (metadata.Open(file) && metadata.GetLengthMilliseconds(&length_in_msec))
				*length_in_ms = (int)length_in_msec;
			else
				*length_in_ms=-1000;
		}
	}
	if (title) *title=0;
}

int InfoBox(const in_char *file, HWND hwndParent) { return 0; }

int IsOurFile(const in_char *fn)
{
	return 0;
}

wchar_t *lastfn=0;
HANDLE threadStarted;
extern FLAC__uint64 lastoutputtime;
extern volatile int bufferCount;
int Play(const in_char *fn)
{
	free(lastfn);
	lastfn=_wcsdup(fn);
	currentSongLength=-1000;
	plugin.is_seekable = 0;
	plugin.SetInfo(0,0,0,0);
	lastoutputtime=0;
	bufferCount=0;
	ResetEvent(killswitch);
	DWORD threadId;
	threadStarted = CreateEvent(0, TRUE, FALSE, 0);

	ReferenceCountedNXString filename_nx;
	nx_uri_t filename_uri;
	NXStringCreateWithUTF16(&filename_nx, fn);
	NXURICreateWithNXString(&filename_uri, filename_nx);

	playThread=CreateThread(0, 0, FLACThread, filename_uri, 0, &threadId);
	SetThreadPriority(playThread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	WaitForSingleObject(threadStarted, INFINITE);
	CloseHandle(threadStarted);
	return 0;
}

int localPause=0;
void Pause()
{
	localPause=1;
	QueueUserAPC(APCPause, playThread, (ULONG_PTR)1);
}

void UnPause()
{
	localPause=0;
	QueueUserAPC(APCPause, playThread, (ULONG_PTR)0);
}

int IsPaused()
{
	return localPause;
}

void Stop()
{
	SetEvent(killswitch);
	WaitForSingleObject(playThread, INFINITE);
	currentSongLength=-1000;
	plugin.outMod->Close();
	plugin.SAVSADeInit();
	free(lastfn);
	lastfn=0;
}

int GetLength()
{
	return currentSongLength;
}


int GetOutputTime()
{
	if (bufferCount)
		return bufferCount;

	if (plugin.outMod)
	{
		return (int)lastoutputtime + (plugin.outMod->GetOutputTime() - plugin.outMod->GetWrittenTime());
	}
	else
		return 0;
}

void SetOutputTime(int time_in_ms)
{
	lastoutputtime=time_in_ms; // cheating a bit here :)
	QueueUserAPC(APCSeek, playThread, (ULONG_PTR)time_in_ms);
}

int pan = 0, volume = -666;
void SetVolume(int _volume)
{
	volume = _volume;
	if (plugin.outMod)
		plugin.outMod->SetVolume(volume);
}

void SetPan(int _pan)
{
	pan = _pan;
	if (plugin.outMod)
		plugin.outMod->SetPan(pan);
}

void EQSet(int on, char data[10], int preamp)
{}

In_Module plugin =
{
	IN_VER_RET,
	"nullsoft(in_flac.dll)",
	0,
	0,
	"FLAC\0FLAC Files\0",
	1,
	1,
	Config,
	About,
	Init,
	Quit,
	GetFileInfo,
	InfoBox,
	IsOurFile,
	Play,
	Pause,
	UnPause,
	IsPaused,
	Stop,
	GetLength,
	GetOutputTime,
	SetOutputTime,
	SetVolume,
	SetPan,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	EQSet,
	0,
	0
};

extern "C"	__declspec(dllexport) In_Module * winampGetInModule2()
{
	return &plugin;
}