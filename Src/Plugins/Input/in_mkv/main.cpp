#include "../Winamp/in2.h"
#include "api__in_mkv.h"
#include "MKVInfo.h"
#include "../Winamp/wa_ipc.h"
#include "main.h"
#include "MKVPlayer.h"
#include "MKVDuration.h"
#include "../nu/ns_wc.h"
#include "resource.h"
#include <strsafe.h>

#define MKV_PLUGIN_VERSION L"0.86"

static wchar_t pluginName[256] = {0};
int g_duration=0;
int paused = 0;
static HANDLE play_thread = 0;
static MKVPlayer *player = 0;

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
  {
    0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
  };

void SetFileExtensions(void)
{
	static char fileExtensionsString[256] = {0};	// "MKV\0Matroska Video (MKV)\0"
	char* end = 0;
	size_t remaining;
	StringCchCopyExA(fileExtensionsString, 255, "MKV", &end, &remaining, 0);
	StringCchCopyExA(end+1, remaining-1, WASABI_API_LNGSTRING(IDS_MKV_DESC), &end, &remaining, 0);
	StringCchCopyExA(end+1, remaining-1, "webm", &end, &remaining, 0);
	StringCchCopyExA(end+1, remaining-1, WASABI_API_LNGSTRING(IDS_WEBM_DESC), &end, &remaining, 0);
	plugin.FileExtensions = fileExtensionsString;
}

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
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_MKV_OLD,text,1024);
	StringCchPrintf(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					plugin.description, TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}

int Init()
{
	if (!IsWindow(plugin.hMainWindow))
		return IN_INIT_FAILURE;

	WasabiInit();
	StringCchPrintfW(pluginName,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_MKV),MKV_PLUGIN_VERSION);
	plugin.description = (char*)pluginName;
	SetFileExtensions();

	return IN_INIT_SUCCESS;
}

void Quit()
{
	WasabiQuit();
}

void GetFileInfo(const wchar_t *file, wchar_t *title, int *length_in_ms)
{
	if (title)
		*title=0;
	if (length_in_ms)
	{
		if (file && *file)
		{
			MKVDuration duration;
			if (duration.Open(file))
			{
				if (title)
				{
					const char *mkv_title = duration.GetTitle();
					if (mkv_title)
						MultiByteToWideCharSZ(CP_UTF8, 0, mkv_title, -1, title, GETFILEINFO_TITLE_LENGTH);
				}
				*length_in_ms=duration.GetLengthMilliseconds();
			}
			else
				*length_in_ms=-1000;
		}
		else
			*length_in_ms = g_duration;
	}
}

int InfoBox(const wchar_t *file, HWND hwndParent)
{
	MKVInfo info;
	if (info.Open(file))
	{
		WASABI_API_DIALOGBOXPARAMW(IDD_INFODIALOG, hwndParent, InfoDialog, (LPARAM)&info);
	}
	return INFOBOX_UNCHANGED;
}

int IsOurFile(const wchar_t *fn)
{
	return 0;
}

DWORD CALLBACK MKVThread(LPVOID param);

int Play(const wchar_t *fn)		// return zero on success, -1 on file-not-found, some other value on other (stopping winamp) error
{
	g_duration=-1000;
	delete player;
	player = new MKVPlayer(fn);
	play_thread = CreateThread(0, 0, MKVThread, player, 0, 0);
	SetThreadPriority(play_thread, (int)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	return 0; // success
}


void Pause()
{
	paused = 1;
	plugin.outMod->Pause(1);
}

void UnPause()
{
	paused = 0;
	plugin.outMod->Pause(0);
}

int IsPaused()
{
	return paused;
}

void Stop()
{
	if (player)
	{
		player->Kill();
		if (play_thread) {
			WaitForSingleObject(play_thread, INFINITE);
		}
		play_thread = 0;
		delete player;
		player=0;
	}
}

// time stuff
int GetLength()
{
	return g_duration;
}

int GetOutputTime()
{
	if (plugin.outMod && player)
		return player->GetOutputTime();
	else
		return 0;
}

void SetOutputTime(int time_in_ms)
{
	if (player)
		player->Seek(time_in_ms);
}

void SetVolume(int volume)
{
	plugin.outMod->SetVolume(volume);
}

void SetPan(int pan)
{
	plugin.outMod->SetPan(pan);
}

void EQSet(int on, char data[10], int preamp)
{
}

In_Module plugin = 
{
	IN_VER_RET,
	"nullsoft(in_mkv.dll)",
	NULL, // hMainWindow
	NULL, // hDllInstance
	0  /*"mkv\0Matroska Video\0"*/,
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