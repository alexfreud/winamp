#include "main.h"
#include <shlwapi.h>
#include "api__in_flv.h"
#include <stdio.h>
#include <api/service/waservicefactory.h>
#include "../Winamp/wa_ipc.h"
#include "resource.h"
#include "FileProcessor.h"
#include "FLVCOM.h"
#include "VideoThread.h"
#include "../f263/flv_f263_decoder.h"
#include <strsafe.h>

#define FLV_PLUGIN_VERSION L"1.47"

template <class api_T>
static void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (plugin.service)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
static void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (plugin.service && api_t)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

/* Wasabi services */
api_application *WASABI_API_APP=0;
api_config *AGAVE_API_CONFIG=0;
api_language *WASABI_API_LNG = 0;

In_Module *swf_mod = 0;
HMODULE in_swf = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
wchar_t pluginName[256] = {0};
wchar_t *playFile=0;
HANDLE killswitch, playthread=0;
int g_length=-1000;
bool video_only=false;
int paused = 0;
nu::VideoClock video_clock;
int m_need_seek=-1;
extern uint32_t last_timestamp;
int pan = 0, volume = -666;
wchar_t *stream_title=0;
Nullsoft::Utility::LockGuard stream_title_guard;

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

void SetFileExtensions(void)
{
	static char fileExtensionsString[256] = {0};	// "FLV\0Flash Video\0"
	char* end = 0;
	size_t remaining;
	StringCchCopyExA(fileExtensionsString, 256, "FLV", &end, &remaining, 0);
	StringCchCopyExA(end+1, remaining-1, WASABI_API_LNGSTRING(IDS_FLASH_VIDEO), 0, 0, 0);
	plugin.FileExtensions = fileExtensionsString;
}

int Init()
{
	if (!IsWindow(plugin.hMainWindow))
		return IN_INIT_FAILURE;

	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,InFlvLangGUID);
	
	if (plugin.service->service_getServiceByGuid(flv_h263_guid))
		StringCchPrintfW(pluginName,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_FLV),FLV_PLUGIN_VERSION L" (h)");
	else
		StringCchPrintfW(pluginName,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_FLV),FLV_PLUGIN_VERSION);
	plugin.description = (char*)pluginName;
	SetFileExtensions();

	DispatchInfo flvDisp = { L"FLV", &flvCOM };
	SendMessage(plugin.hMainWindow, WM_WA_IPC, (WPARAM)&flvDisp, IPC_ADD_DISPATCH_OBJECT);

	killswitch = CreateEvent(NULL, TRUE, FALSE, NULL);
	return IN_INIT_SUCCESS;
}

void Quit()
{
	CloseHandle(killswitch);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	if (in_swf)
		FreeLibrary(in_swf);
}

void GetFileInfo(const wchar_t *file, wchar_t *title, int *length_in_ms)
{
	if (!file || !*file)
	{
		if (swf_mod)
		{
			swf_mod->GetFileInfo(file, title, length_in_ms);
			return;
		}

		file = playFile;
	}

	// no title support as it's not always stored in a standard way in FLV
	if (title)
	{
		Nullsoft::Utility::AutoLock stream_lock(stream_title_guard);
		if (stream_title && (!file || !file[0]))
		{
			lstrcpyn(title, stream_title, GETFILEINFO_TITLE_LENGTH);
		}
		else
		{
			lstrcpyn(title, file ? file : L"", GETFILEINFO_TITLE_LENGTH);
			PathStripPath(title);
		}
	}

	// calculate length
	if (file == playFile) // currently playing song?
		*length_in_ms = g_length; // easy!
	else if (PathIsURLW(file)) // don't calculate lengths for URLs since we'd have to connect
	{
		*length_in_ms = -1;
	}
	else
	{
		// open the file and find the "duration" metadata
		FileProcessor processor(file);

		size_t frameIndex=0;
		FrameData frameData;
		int length=-1;
		bool found=false;
		do
		{
			// enumerate the frames as we process
			// this function will fail the first few times
			// because Process() hasn't been called enough
			if (processor.GetFrame(frameIndex, frameData))
			{
				if (frameData.header.type == FLV::FRAME_TYPE_METADATA)
				{
					if (processor.Seek(frameData.location + 15) == -1)
						break;

					size_t dataSize = frameData.header.dataSize;
					uint8_t *metadatadata= (uint8_t *)calloc(dataSize, sizeof(uint8_t));
					if (metadatadata)
					{
						size_t bytesRead = processor.Read(metadatadata, dataSize);
						if (bytesRead != dataSize)
						{
							free(metadatadata);
							break;
						}
						FLVMetadata metadata;
						metadata.Read(metadatadata, dataSize);

						for ( FLVMetadata::Tag *tag : metadata.tags )
						{
							if ( !_wcsicmp( tag->name.str, L"onMetaData" ) )
							{
								AMFType *duration = tag->parameters->array[ L"duration" ];
								if ( duration )
								{
									length = (int)( AMFGetDouble( duration ) * 1000.0 );
									found = true;
								}
							}
						}
						free(metadatadata);
					}
					else
						break;
				}
				frameIndex++;
			}
		}
		while (!found && processor.Process() == FLV_OK); // for local files, any return value other than FLV_OK is a failure
		*length_in_ms = length;
	}
}

int InfoBox(const wchar_t *file, HWND hwndParent)
{
	return INFOBOX_UNCHANGED;
}

int IsOurFile(const wchar_t *file)
{
	return 0;
}

int Play(const wchar_t *file)
{
	{
		Nullsoft::Utility::AutoLock stream_lock(stream_title_guard);
		free(stream_title);
		stream_title=0;
	}
	if (!videoOutput)
		videoOutput = (IVideoOutput *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GET_IVIDEOOUTPUT);

	video_only=false;
	m_need_seek = -1;
	video_clock.Reset();
	paused=0;
	ResetEvent(killswitch);
	free(playFile);
	playFile = _wcsdup(file);
	playthread=CreateThread(0, 0, PlayProcedure, 0, 0, 0);
	SetThreadPriority(playthread, (INT)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	return 0; // success
}


void Pause()
{
	paused = 1;
	if (swf_mod)
	{
		swf_mod->Pause();
	}
	else if (video_only)
	{
		video_clock.Pause();
	}
	else
	{
		plugin.outMod->Pause(1);
	}
}

void UnPause()
{
	paused = 0;
	if (swf_mod)
	{
		swf_mod->UnPause();
	}
	else if (video_only)
	{
		video_clock.Unpause();
	}
	else
	{
		plugin.outMod->Pause(0);
	}

}

int IsPaused()
{
	if (swf_mod)
		return swf_mod->IsPaused();

	return paused;
}

void Stop()
{
	if (swf_mod)
	{
		swf_mod->Stop();
		swf_mod=0;
		return;
	}
	SetEvent(killswitch);
	WaitForSingleObject(playthread, INFINITE);
	playthread=0;
	plugin.outMod->Close();
	plugin.SAVSADeInit();
	paused=0;
}

int GetLength()
{
	if (swf_mod)
	{
		return swf_mod->GetLength();
	}
	return g_length;
}

int GetOutputTime()
{
	if (swf_mod)
	{
		return swf_mod->GetOutputTime();
	}
	else if (video_only)
	{
		return video_clock.GetOutputTime();
	}
	else if (plugin.outMod)
	{
		return plugin.outMod->GetOutputTime();
	}
	else
		return 0;
}

void SetOutputTime(int time_in_ms)
{
	if (swf_mod)
	{
		swf_mod->SetOutputTime(time_in_ms);
		return ;
	}
	m_need_seek=time_in_ms;
}


void SetVolume(int _volume)
{
	if (swf_mod)
	{
		swf_mod->SetVolume(_volume);
		return ;
	}
	volume = _volume;
	if (plugin.outMod)
		plugin.outMod->SetVolume(volume);
}

void SetPan(int _pan)
{
	if (swf_mod)
	{
		swf_mod->SetPan(_pan);
		return ;
	}
	pan = _pan;
	if (plugin.outMod)
		plugin.outMod->SetPan(pan);
}

void EQSet(int on, char data[10], int preamp)
{
	if (swf_mod)
	{
		swf_mod->EQSet(on, data, preamp);
		return;
	}
}

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

void About(HWND hwndParent)
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_FLV_OLD,text,1024);
	StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					 plugin.description, TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}

In_Module plugin =
{
	IN_VER_RET,
	"nullsoft(in_flv.dll)",
	0,
	0,
	0  /*"FLV\0Flash Video\0"*/,
	1, // not seekable, for now
	1,
	About,
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