#define PLUGIN_NAME "Nullsoft LineIn Plug-in"
#define PLUGIN_VERSION L"3.16"

#include "main.h"
#include <windows.h>
#include "LineIn.h"
#include <api/service/waServiceFactory.h>
#include "../Agave/Language/api_language.h"
#include "../winamp/wa_ipc.h"
#include "resource.h"

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

int lstrcmpni(const char *a, const char *b, int count)
{
	if (!a || !b) return -1;

	switch (CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, a, min(lstrlenA(a),count), b, min(lstrlenA(b),count)))
	{
		case CSTR_LESS_THAN: return -1;
		case CSTR_GREATER_THAN: return 1;
		case CSTR_EQUAL: return 0;
	}
	return -1;
}
LineIn lineIn;

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

void about(HWND hwndParent)
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_LINEIN_PLUGIN_OLD,text,1024);
	wsprintfW(message, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
			  line.description, __DATE__);
	DoAboutMessageBox(hwndParent,text,message);
}

int init()
{
	if (!IsWindow(line.hMainWindow))
		return IN_INIT_FAILURE;

	waServiceFactory *sf = line.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(line.hDllInstance,InLineInLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_LINEIN_PLUGIN),PLUGIN_VERSION);
	line.description = (char*)szDescription;
	return IN_INIT_SUCCESS;
}

void quit() {}

int play(const char *fn)
{
	line.is_seekable=0;
	if (!lstrcmpni(fn, "linein://", 9))
	{
		lineIn.Play();
		return 0;
	}
	return 1;
}

int isourfile(const char *fn)
{
	if (!lstrcmpni(fn, "linein://", 9)) return 1;
	return 0;
}

void pause()
{
	lineIn.Pause();
}

void unpause()
{
	lineIn.Unpause();
}

int ispaused()
{
	return lineIn.IsPaused();
}

void stop()
{
	lineIn.Stop();
	line.SAVSADeInit();
}

int getlength()
{
	return lineIn.GetLength();
}

int getoutputtime()
{
	return lineIn.GetOutputTime();
}
void setoutputtime(int time_in_ms)
{}

void setvolume(int volume)
{
	line.outMod->SetVolume(volume);
}

void setpan(int pan)
{
	line.outMod->SetPan(pan);
}

int infoDlg(const char *fn, HWND hwnd)
{
	return 0;
}

void getfileinfo(const char *filename, char *title, int *length_in_ms)
{
	if (length_in_ms)
		*length_in_ms = -1000;
	if (title)
		WASABI_API_LNGSTRING_BUF(IDS_LINE_INPUT,title,GETFILEINFO_TITLE_LENGTH);
}

void eq_set(int on, char data[10], int preamp)
{}

In_Module line =
    {
		IN_VER_RET,
        "nullsoft(in_line.dll)",
        0,    	// hMainWindow
        0,      // hDllInstance
        "",
        0,    	// is_seekable
        //0,  // uses output plugins
        1,      // uses output plugins
        about,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0,     // dsp shit
        eq_set,
        NULL,    		// setinfo
        NULL // out_mod
    };

BOOL APIENTRY DllMain(HANDLE hMod, DWORD r, void*)
{
	if (r == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls((HMODULE)hMod);

	return TRUE;
}

extern "C"	__declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &line;
}