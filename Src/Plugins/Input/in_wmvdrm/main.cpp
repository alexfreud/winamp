#include "Main.h"
#include "api.h"
#include "loadini.h"
#include "FileTypes.h"
#include <commctrl.h>
#include "../nu/Config.h"
#include "../nu/CCVersion.h"
#include "resource.h"

wchar_t INI_FILE[MAX_PATH] = L"";
IDispatch *winampExternal = 0;
Nullsoft::Utility::Config wmConfig;
WMDRM mod;
HINSTANCE WASABI_API_LNG_HINST_WAV = 0;
HINSTANCE WASABI_API_LNG_HINST_DS = 0;

int Init()
{
	if (!IsWindow(plugin.hMainWindow))
		return IN_INIT_FAILURE;

	if (!LoadWasabi())
		return IN_INIT_FAILURE;

	plugin.UsesOutputPlug |= 8;

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,InWmLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_WINDOWS_MEDIA_DECODER),WMDRM_VERSION);
	plugin.description = (char*)szDescription;

	IniFile(plugin.hMainWindow);
	wmConfig.SetFile(INI_FILE, L"in_wm");
	ReadConfig();
	fileTypes.ReadConfig();
	if (NULL == winampExternal)
	{
		winampExternal = (IDispatch *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GET_DISPATCH_OBJECT); // ask for winamp's
		if (winampExternal == (IDispatch *)1)
			winampExternal = 0;
	}

	mod.Init(); 
	return IN_INIT_SUCCESS;
}

void Quit() 
{ 
	mod.Quit();  
	UnloadWasabi();
	fileTypes.types.clear();

	if (NULL != winampExternal)
	{
		winampExternal->Release();
		winampExternal = NULL;
	}
}

void Config(HWND parent)
{
	mod.Config(parent);
	fileTypes.SaveConfig();
	WriteConfig();
}

void About(HWND parent) 
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_WINDOWS_MEDIA_DECODER_OLD,text,1024);
	wsprintfW(message, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
			  plugin.description, TEXT(__DATE__));
	DoAboutMessageBox(parent,text,message);
}

void GetFileInfo(const in_char *file, wchar_t *title, int *length_in_ms) { mod.GetFileInfo(file, title, length_in_ms); }
int InfoDialog(const in_char *file, HWND parent) { return mod.InfoBox(file, parent); }
int IsOurFile(const in_char *fn) { return mod.IsOurFile(fn); }
int Play(const in_char *fn) {return mod.Play(fn); }
void Pause() { mod.Pause(); }
void Resume() { mod.UnPause(); }
int IsPaused() { return mod.IsPaused(); }
void Stop() { mod.Stop(); }
int GetLength() { return mod.GetLength(); }
int GetOutputTime() { return mod.GetOutputTime(); }
void SetOutputTime(int time_in_ms) { return mod.SetOutputTime(time_in_ms); }
void SetVolume(int volume) { mod.SetVolume(volume); }
void SetPan(int pan) { mod.SetPan(pan); }
void EQSet(int on, char data[10], int preamp) { mod.EQSet(on, data, preamp); }

In_Module plugin =
{
	IN_VER_RET,	// defined in IN2.H
	"nullsoft(in_wm.dll)",
	0,	// hMainWindow (filled in by winamp)
	0,  // hDllInstance (filled in by winamp)
	0,	// this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
	0,	// is_seekable
	1,	// uses output plug-in system
	Config,
	About,
	Init,
	Quit,
	GetFileInfo,
	InfoDialog,
	IsOurFile,
	Play,
	Pause,
	Resume,
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

extern "C" __declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &plugin;
}