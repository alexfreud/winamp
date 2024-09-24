#include "main.h"
#include "api.h"
#include "../Winamp/wa_ipc.h"
#include "../Winamp/strutil.h"
#include <shlwapi.h>
#include "FLVExternalInterface.h"
#include <api/service/waServiceFactory.h>
#include <strsafe.h>
#include "resource.h"

#define SWF_PLUGIN_VERSION L"1.15"

FLVExternalInterface flashExternalInterface;
IVideoOutput *videoOutput=0;
int playPosition=0;
int playLength=-1000;
api_application *WASABI_API_APP = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
wchar_t pluginName[256] = {0}, status[256] = {0};
Nullsoft::Utility::LockGuard statusGuard;

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
	if (plugin.service)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

void SetFileExtensions(void)
{
	static char fileExtensionsString[1200] = {0};	// "SWF\0Shockwave Flash Files\0"
	char* end = 0;
	StringCchCopyExA(fileExtensionsString, 1200, "SWF", &end, 0, 0);
	StringCchCopyExA(end+1, 1200, WASABI_API_LNGSTRING(IDS_SWF_FILES), 0, 0, 0);
	plugin.FileExtensions = fileExtensionsString;
}

int Init()
{
	if (!IsWindow(plugin.hMainWindow))
		return IN_INIT_FAILURE;

	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);

	WASABI_API_START_LANG(plugin.hDllInstance,InSwfLangGUID);
	StringCchPrintfW(pluginName,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_SWF),SWF_PLUGIN_VERSION);
	plugin.description = (char*)pluginName;
	SetFileExtensions();

	return IN_INIT_SUCCESS;
}

void Quit()
{
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
}

void GetFileInfo(const in_char *file, in_char *title, int *length_in_ms)
{
	if (length_in_ms)
	{
		if (file && *file)
			*length_in_ms=-1000;
		else
			*length_in_ms = playLength;
	}

	if (title)
	{
		if (file && *file)
			*title=0;
		else
		{
			Nullsoft::Utility::AutoLock autolock(statusGuard);
			if (status[0])
				StringCchPrintf(title, GETFILEINFO_TITLE_LENGTH, L"[%s]", status);
			else
				*title=0;
		}
	}
}

int InfoBox(const in_char *file, HWND hwndParent)
{
	return INFOBOX_UNCHANGED;
}

int IsOurFile(const in_char *fn)
{
	if (!_wcsnicmp(fn, L"rtmp://", 7))
		return 1;
	return 0;
}

static bool isFLV = false;
static int PlaySWF(BSTR filename)
{
#ifdef WIN64
	if (!activeContainer || (unsigned long long)activeContainer < 65536)
	{
		isFLV = false;
		return 1;
	}
#else
	if (!activeContainer || (unsigned long)activeContainer < 65536)
	{
		isFLV = false;
		return 1;
}
#endif

	isFLV = false;
	activeContainer->externalInterface = &flashExternalInterface;
	activeContainer->flash->DisableLocalSecurity();
	activeContainer->flash->put_BackgroundColor(0); 
	activeContainer->flash->put_EmbedMovie(FALSE);
	activeContainer->flash->put_Scale(L"showAll");
	activeContainer->flash->put_AllowScriptAccess(L"always");

	HRESULT hr = activeContainer->flash->LoadMovie(0, filename);

	activeContainer->setVisible(TRUE);

	plugin.is_seekable = 0; // not seekable to start, we'll find out after opening if it's really seekable or not
	return 0;
}

static int PlayFLV(const wchar_t *filename)
{
#ifdef WIN64
	if (!activeContainer || (unsigned long long)activeContainer < 65536)
	{
		isFLV = false;
		return 1;
}
# else
	if (!activeContainer || (unsigned long)activeContainer < 65536)
	{
		isFLV = false;
		return 1;
	}
#endif // 

//	if (!activeContainer || (unsigned long)activeContainer < 65536)
//	{
//		isFLV = false;
//		return 1;
//	}

	isFLV = true;
	activeContainer->externalInterface = &flashExternalInterface;
	activeContainer->flash->DisableLocalSecurity();
	activeContainer->flash->put_BackgroundColor(0); 
	activeContainer->flash->put_EmbedMovie(FALSE);
	activeContainer->flash->put_Scale(L"showAll");
	activeContainer->flash->put_AllowScriptAccess(L"always");
	
	static wchar_t pluginPath[MAX_PATH] = {0}, swfPath[MAX_PATH+7] = {0};
	if (!pluginPath[0] && !swfPath[0])
	{
		lstrcpynW(pluginPath, (wchar_t*)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORYW), MAX_PATH);
		PathAppend(pluginPath, L"winampFLV.swf");
		for (wchar_t *itr = pluginPath; *itr; itr++)
		{
			if (*itr == '\\')
				*itr = '/';
		}
		StringCchPrintf(swfPath, MAX_PATH+7, L"file://%s", pluginPath);
	}
	
	HRESULT hr = activeContainer->flash->LoadMovie(0, swfPath);
	
	activeContainer->setVisible(TRUE);

	// TODO: make filename XML-safe
	wchar_t funcCall[1024] = {0};
	StringCchPrintf(funcCall, 1024, L"<invoke name=\"PlayFLV\" returntype=\"xml\"><arguments><string>%s</string></arguments></invoke>", filename);
	BSTR bstr_ret = 0;
	activeContainer->flash->CallFunction(funcCall, &bstr_ret);
	SetVolume(volume);
	SetPan(pan);

	plugin.is_seekable = 1; // not seekable to start, we'll find out after opening if it's really seekable or not
	return 0;
}

int Play(const in_char *filename)
{
	status[0]=0;
	playPosition=0;
	playLength=-1000;

	if (!filename || !*filename)
		return 1;

	if (!videoOutput)
		videoOutput = (IVideoOutput *)SendMessage(plugin.hMainWindow,WM_WA_IPC,0,IPC_GET_IVIDEOOUTPUT);

	if (!videoOutput)
		return 1;

	HWND videoWnd = (HWND)videoOutput->extended(VIDUSER_GET_VIDEOHWND, 0, 0); // ask for the video hwnd

	wchar_t *mangledFilename = 0;
	if (PathIsURL(filename))
		mangledFilename = const_cast<wchar_t *>(filename);
	else
	{
		mangledFilename = (wchar_t *)malloc((MAX_PATH + 7)*sizeof(wchar_t));
		StringCchPrintf(mangledFilename, MAX_PATH+7, L"file://%s", filename);
	}
	videoOutput->open(0, 0, 0, 1.0, 'ENON');
	activeContainer = new SWFContainer(videoWnd);
	if (!activeContainer->flash)
	{
		delete activeContainer;
		activeContainer=0;
		if (mangledFilename != filename)
			free(mangledFilename);
		return 1; // failed
	}

	oldVidProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(videoWnd, GWLP_WNDPROC, (LONG)(LONG_PTR)WndProc);

	wchar_t ext[16]=L"";
	extension_exW(filename, ext, 16);
	if (!_wcsicmp(ext, L"swf"))
	{
		if (PlaySWF(mangledFilename))
		{
			return 1; // failed
		}
	}
	else
	{
		if (PlayFLV(mangledFilename))
		{
			return 1; // failed
		}
	}

	HRESULT hr = activeContainer->flash->Play();

	if (mangledFilename != filename)
		free(mangledFilename);

	return 0;
}

int localPause=0;
void Pause()
{
	localPause=1;

	if (isFLV)
	{
		BSTR bstr_ret;
		activeContainer->flash->CallFunction(L"<invoke name=\"Pause\" returntype=\"xml\"><arguments></arguments></invoke>", &bstr_ret);	
	}
}

void UnPause()
{
	localPause=0;
	if (isFLV)
	{
		BSTR bstr_ret;
		activeContainer->flash->CallFunction(L"<invoke name=\"Resume\" returntype=\"xml\"><arguments></arguments></invoke>", &bstr_ret);	
	}
}

int IsPaused()
{
	return localPause;
}

void Stop()
{
	videoOutput->close();
	HWND videoWnd = (HWND)videoOutput->extended(VIDUSER_GET_VIDEOHWND, 0, 0); // ask for the video hwnd
	SetWindowLongPtr(videoWnd, GWLP_WNDPROC, (LONG)(LONG_PTR)oldVidProc);
	activeContainer->close();
	activeContainer->Release();
	activeContainer=0;
}

int GetLength()
{
	return playLength;
}

int GetOutputTime()
{
	return playPosition;
}

void SetOutputTime(int time_in_ms)
{
	if (activeContainer)
	{
		if (isFLV)
		{
			double seconds = time_in_ms;
			seconds/=1000.0;

			wchar_t funcCall[1024] = {0};
			StringCchPrintf(funcCall, 1024, L"<invoke name=\"Seek\" returntype=\"xml\"><arguments><number>%.3f</number></arguments></invoke>", seconds);
			BSTR bstr_ret;
			activeContainer->flash->CallFunction(funcCall, &bstr_ret);
		}
		else
		{
			// TODO: maybe change the frame?
		}
	}
}

int pan = 0, volume = 255;
void SetVolume(int _volume)
{
	volume = _volume;
	if (activeContainer)
	{
		if (isFLV)
		{
			int newVolume = (volume * 100) / 255;
	
			wchar_t funcCall[1024] = {0};
			StringCchPrintf(funcCall, 1024, L"<invoke name=\"SetVolume\" returntype=\"xml\"><arguments><number>%u</number></arguments></invoke>", newVolume);
			BSTR bstr_ret;
			activeContainer->flash->CallFunction(funcCall, &bstr_ret);	
		}
	}
}

void SetPan(int _pan)
{
	pan = _pan;
	if (activeContainer)
	{
		if (isFLV)
		{
			int left = 100;
			int right = 100;
			if (pan < 0)
				left += (pan * 100)/127;
			if (pan>0)
				right-=(pan*100)/127;

			wchar_t funcCall[1024] = {0};
			StringCchPrintf(funcCall, 1024, L"<invoke name=\"SetPan\" returntype=\"xml\"><arguments><number>%u</number><number>%u</number></arguments></invoke>", left, right);
			BSTR bstr_ret = 0;
			activeContainer->flash->CallFunction(funcCall, &bstr_ret);	
		}
	}
}

void EQSet(int on, char data[10], int preamp)
{}

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

void About(HWND hwndParent);
In_Module plugin =
{
	IN_VER_RET,
	"nullsoft(in_swf.dll)",
	0,
	0,
	0  /*"SWF\0Shockwave Flash Files\0"*/,
	1,
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

void About(HWND hwndParent)
{
	wchar_t message[1024] = {0}, text[1024] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_SWF_OLD,text,1024);
	StringCchPrintf(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					plugin.description, TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}

extern "C"	__declspec(dllexport) In_Module * winampGetInModule2()
{
	return &plugin;
}