#include "main.h"
#include "../winamp/wa_ipc.h"
#include "api__in_avi.h"
#include "../nsavi/nsavi.h"
#include "win32_avi_reader.h"
#include "resource.h"
#include <strsafe.h>

#define AVI_PLUGIN_VERSION L"0.78"

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

void SetFileExtensions(void)
{
	static char fileExtensionsString[256] = {0};	// "AVI\0Audio/Video Interleave (AVI)\0"
	char* end = 0;
	size_t remaining = 0;
	StringCchCopyExA(fileExtensionsString, 256, "AVI", &end, &remaining, 0);
	StringCchCopyExA(end+1, remaining-1, WASABI_API_LNGSTRING(IDS_AVI_DESC), 0, 0, 0);
	plugin.FileExtensions = fileExtensionsString;
}

HANDLE killswitch = 0, seek_event = 0;
volatile LONG seek_position=-1;
int g_duration = -1;
nu::VideoClock video_clock;
int paused = 0;
static HANDLE play_thread = 0;
int video_only=0;
In_Module *dshow_mod = 0;
HMODULE in_dshow=0;
wchar_t pluginName[256] = {0};

static int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
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
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_AVI_OLD,text,1024);
	StringCchPrintf(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					plugin.description, TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}

int Init()
{
	if (!IsWindow(plugin.hMainWindow))
		return IN_INIT_FAILURE;

	WasabiInit();

	StringCchPrintfW(pluginName, ARRAYSIZE(pluginName),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_AVI), AVI_PLUGIN_VERSION);
	plugin.description = (char*)pluginName;
	SetFileExtensions();
	return IN_INIT_SUCCESS;
}

void Quit()
{
	WasabiQuit();
	if (in_dshow)
	{
		FreeLibrary(in_dshow);
		in_dshow = NULL;
	}
}

void GetFileInfo(const wchar_t *file, wchar_t *title, int *length_in_ms)
{
	if (title)
		*title=0;
	if (length_in_ms)
	{
		if (file && *file)
		{
			*length_in_ms = -1000; // fallback if anything fails
			AVIReaderWin32 reader;
			if (reader.Open(file) == nsavi::READ_OK)
			{
				nsavi::Duration duration(&reader);
				duration.GetDuration(length_in_ms);
				reader.Close();
			}
		}
		else
		{
			if (dshow_mod)
			{
				dshow_mod->GetFileInfo(file, title, length_in_ms);
			}
			else
			{

				*length_in_ms=g_duration;
			}
		}
	}


}

int InfoBox(const wchar_t *file, HWND hwndParent)
{
	AVIReaderWin32 reader;
	if (reader.Open(file) == nsavi::READ_OK)
	{
		nsavi::Metadata metadata(&reader);
		uint32_t type;
		if (metadata.GetRIFFType(&type) == nsavi::READ_OK && type == ' IVA')
		{
			WASABI_API_DIALOGBOXPARAMW(IDD_INFODIALOG, hwndParent, InfoDialog, (LPARAM)&metadata);
		}
		reader.Close();
	}

	return INFOBOX_UNCHANGED;
}

int IsOurFile(const wchar_t *fn)
{
	return 0;
}


int Play(const wchar_t *fn)		// return zero on success, -1 on file-not-found, some other value on other (stopping winamp) error
{
	g_duration = -1;
	seek_position = -1;
	video_clock.Reset();
	video_only=false;
	paused=0;
	if (!killswitch)
		killswitch = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!seek_event)
		seek_event = CreateEvent(NULL, TRUE, FALSE, NULL);

	ResetEvent(killswitch);
	ResetEvent(seek_event);

	play_thread = CreateThread(0, 0, AVIPlayThread, _wcsdup(fn), 0, 0);
	SetThreadPriority(play_thread, (int)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	return 0; // success
}


void Pause()
{
	paused = 1;

	if (dshow_mod)
	{
		dshow_mod->Pause();
	}
	else 	if (video_only)
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
	if (dshow_mod)
	{
		dshow_mod->UnPause();
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
	if (dshow_mod)
		return dshow_mod->IsPaused();

	return paused;
}

void Stop()
{
	if (dshow_mod)
	{
		dshow_mod->Stop();
		dshow_mod=0;
	}
	else if (play_thread)
	{
		SetEvent(killswitch);
		WaitForSingleObject(play_thread, INFINITE);
		ResetEvent(killswitch);
		play_thread=0;
	}
}

// time stuff
int GetLength()
{
	if (dshow_mod)
		return dshow_mod->GetLength();
	else
		return g_duration;
}

int GetOutputTime()
{
	if (dshow_mod)
		return dshow_mod->GetOutputTime();
	else if (video_only)
		return video_clock.GetOutputTime();
	if (plugin.outMod)
		return plugin.outMod->GetOutputTime();
	else
		return 0;
}

void SetOutputTime(int time_in_ms)
{
	if (dshow_mod)
	{
		dshow_mod->SetOutputTime(time_in_ms);
	}
	else if (seek_event)
	{
		InterlockedExchange(&seek_position, time_in_ms);
		SetEvent(seek_event);
	}
}

void SetVolume(int volume)
{
	if (dshow_mod)
	{
		dshow_mod->SetVolume(volume);
	}
	else
	{
		plugin.outMod->SetVolume(volume);
	}
}

void SetPan(int pan)
{	
	if (dshow_mod)
	{
		dshow_mod->SetPan(pan);
	}
	else
	{
		plugin.outMod->SetPan(pan);
	}
}

void EQSet(int on, char data[10], int preamp)
{
	if (dshow_mod)
	{
		dshow_mod->EQSet(on, data, preamp);
		return;
	}
}

In_Module plugin = 
{
	IN_VER_RET,
	"nullsoft(in_avi.dll)",
	NULL, // hMainWindow
	NULL, // hDllInstance
	0  /*"AVI\0Audio/Video Interleave (AVI)\0"*/,
	1, // is seekable
	IN_MODULE_FLAG_USES_OUTPUT_PLUGIN, //UsesOutputPlug
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