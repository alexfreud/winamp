#include "../Winamp/IN2.h"
#include "api__in_mod.h"
#include "../nu/ServiceBuilder.h"
#include "resource.h"
#include <strsafe.h>
#include "MODPlayer.h"
#include "../nu/AutoWide.h"
#include <libopenmpt/libopenmpt.h>

static MODPlayer *player;
DWORD CALLBACK MODThread(LPVOID param);
extern In_Module plugin;

HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
int g_duration=0;
int paused = 0;
static HANDLE play_thread = 0;

static const wchar_t *MOD_PLUGIN_VERSION = L"3.05";

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{
	0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
};

static wchar_t plugin_name[256];

/* Wasabi services */
api_application *WASABI_API_APP=0;
api_config *AGAVE_API_CONFIG=0;
api_language *WASABI_API_LNG = 0;

static int Init()
{
	if (!IsWindow(plugin.hMainWindow)) {
		return IN_INIT_FAILURE;
	}

	ServiceBuild(plugin.service, AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(plugin.service, WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(plugin.service, WASABI_API_LNG, languageApiGUID);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance, InModMPTLangGUID);
	StringCbPrintfW(plugin_name,sizeof(plugin_name),WASABI_API_LNGSTRINGW(IDS_NULLSOFT_MOD), MOD_PLUGIN_VERSION);
	plugin.description = (char *)plugin_name;


	static char fileExtensionsString[2048] = "";
	char* end = fileExtensionsString;
	size_t remaining=sizeof(fileExtensionsString);
	const char *extensions = openmpt_get_supported_extensions();
	char *next_token;
	for (const char *extension = strtok_s((char *)extensions, ";", &next_token); extension; extension = strtok_s(NULL, ";", &next_token)) {
		StringCbCopyExA(end, remaining, extension, &end, &remaining, 0);
		const char *tracker = openmpt_get_tracker_name(extension);
		StringCbCopyExA(end+1, remaining-1, tracker, &end, &remaining, 0);
		openmpt_free_string(tracker);
		end++; remaining--;

	}
	plugin.FileExtensions = fileExtensionsString;
	*end = 0;	

	openmpt_free_string(extensions);
	return IN_INIT_SUCCESS;
}

static void Quit()
{
	ServiceRelease(plugin.service, AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(plugin.service, WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(plugin.service, WASABI_API_LNG, languageApiGUID);
}

static int InfoBox(const wchar_t *file, HWND hwndParent)
{
	return INFOBOX_UNCHANGED;
}

static int IsOurFile(const wchar_t *file)
{
	return 0;
}

static void GetFileInfo(const wchar_t *file, wchar_t *title, int *length_in_ms)
{
}



static int Play(const wchar_t *file)
{
	g_duration=-1000;
	player = new MODPlayer(file);
	play_thread = CreateThread(0, 0, MODThread, player, 0, 0);
	SetThreadPriority(play_thread, (int)AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	return 0; // success
}

static void Pause()
{
	paused = 1;
	plugin.outMod->Pause(1);
}

static void UnPause()
{
	paused = 0;
	plugin.outMod->Pause(0);
}

static int IsPaused()
{
	return paused;
}

static void Stop()
{
	if (player)	{
		player->Kill();
		if (play_thread) {
			WaitForSingleObject(play_thread, INFINITE);
		}
		play_thread = 0;
		delete player;
		player=0;
	}
}

static int GetLength()
{
	return g_duration;
}

static int GetOutputTime()
{
	if (plugin.outMod && player) {
		return player->GetOutputTime();
	} else {
		return 0;
	}
}

static void SetOutputTime(int time_in_ms)
{
	if (player) {
		player->Seek(time_in_ms);
	}
}

static void SetVolume(int _volume)
{
	plugin.outMod->SetVolume(_volume);
}

static void SetPan(int _pan)
{
	plugin.outMod->SetPan(_pan);
}

static void EQSet(int on, char data[10], int preamp)
{
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

static void About(HWND hwndParent)
{
	wchar_t message[1024], text[1024];
	char license_trim[1024];
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_MOD_OLD,text,1024);
	const char *library_version = openmpt_get_string("library_version");
	const char *license = openmpt_get_string("license");

	// trim the license string
	StringCbCopyA(license_trim, sizeof(license_trim), license);
	char * trim = license_trim;
	for (int i=0;i<4;i++) {
		trim = strchr(trim, '\n');
		if (trim) {
			trim++;
		}
	}
	*trim=0;

	StringCchPrintfW(message, 1024, 
		WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
		plugin.description, 
		TEXT(__DATE__),
		library_version,
		license_trim		
		);


	DoAboutMessageBox(hwndParent,text,message);
}


extern In_Module plugin =
{
	IN_VER_RET,	// defined in IN2.H
	"nullsoft(in_mod.dll)",
	0,	// hMainWindow (filled in by winamp)
	0,  // hDllInstance (filled in by winamp)
	"S3M\0Scream Tracker\0", 	// this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
	1,	// is_seekable
	1,	// uses output plug-in system
	About, // TODO(benski) config
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

	0,0,0,0,0,0,0,0,0, // visualization calls filled in by winamp

	0,0, // dsp calls filled in by winamp

	EQSet,

	NULL,		// setinfo call filled in by winamp

	0, // out_mod filled in by winamp
};

// exported symbol. Returns output module.
extern "C"
{
	__declspec(dllexport) In_Module * winampGetInModule2()
	{
		return &plugin;
	}
}