#include "pluginloader.h"
#include "../winamp/wa_ipc.h"
#include "nu/AutoWide.h"
#include <shlwapi.h>
#include "../nu/MediaLibraryInterface.h"

extern winampMediaLibraryPlugin plugin;

C_ItemList m_plugins;

extern HWND mainMessageWindow;

int wmDeviceChange(WPARAM wParam, LPARAM lParam) {
	int ret=0;
	for(int i=0; i < m_plugins.GetSize(); i++) {
		PMPDevicePlugin * plugin = (PMPDevicePlugin *)m_plugins.Get(i);
    /*
		if(plugin->wmDeviceChange)
		{
			if(plugin->wmDeviceChange(wParam, lParam) == BROADCAST_QUERY_DENY)
				ret = BROADCAST_QUERY_DENY;
		}
    */
		if(plugin->MessageProc)
		{
			if(plugin->MessageProc(PMP_DEVICECHANGE,wParam,lParam,0) == BROADCAST_QUERY_DENY)
				ret = BROADCAST_QUERY_DENY;
		}
	}
	return ret;
}

PMPDevicePlugin * loadPlugin(wchar_t * file)
{
	HINSTANCE m=LoadLibrary(file);
	if(m)
	{
		PMPDevicePlugin *(*gp)();
		gp=(PMPDevicePlugin *(__cdecl *)(void))GetProcAddress(m,"winampGetPMPDevicePlugin");
		if(!gp)
		{
			FreeLibrary(m);
			return NULL;
		}

		PMPDevicePlugin *devplugin=gp();
		if(!devplugin || devplugin->version != PMPHDR_VER)
		{
			FreeLibrary(m);
			return NULL;
		}

		devplugin->hDllInstance=m;
		devplugin->hwndLibraryParent=plugin.hwndLibraryParent;
		devplugin->hwndWinampParent=plugin.hwndWinampParent;
		devplugin->hwndPortablesParent=mainMessageWindow;
		devplugin->service = plugin.service;

		if(devplugin->init())
		{
			FreeLibrary(m);
		}
		else
		{
			m_plugins.Add((void *)devplugin);
			return devplugin;
		}
	}
	return NULL;
}

BOOL loadDevPlugins(int *count)
{
	BOOL loaded = FALSE;
	wchar_t tofind[MAX_PATH] = {0};
	LPCWSTR dir = (LPCWSTR)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETPLUGINDIRECTORYW);
	PathCombine(tofind, dir, L"pmp_*.dll"); 

	WIN32_FIND_DATA d = {0};
	HANDLE h = FindFirstFile(tofind,&d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t file[MAX_PATH] = {0};
			PathCombine(file, dir, d.cFileName);
			loaded += (!!loadPlugin(file));
		}
		while(FindNextFile(h,&d));
		FindClose(h);
	}

	if (count) *count = m_plugins.GetSize();
	return loaded;
}

BOOL testForDevPlugins()
{
	BOOL found = FALSE;
	wchar_t tofind[MAX_PATH] = {0};
	LPCWSTR dir = (LPCWSTR)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETPLUGINDIRECTORYW);
	PathCombine(tofind, dir, L"pmp_*.dll"); 

	WIN32_FIND_DATA d = {0};
	HANDLE h = FindFirstFile(tofind,&d);
	if (h != INVALID_HANDLE_VALUE)
	{
		found = TRUE;
		FindClose(h);
	}
	return found;
}

extern int profile;
static HANDLE hProfile = INVALID_HANDLE_VALUE;
HANDLE GetProfileFileHandle()
{
	if (profile)
	{
		if (hProfile == INVALID_HANDLE_VALUE)
		{
			wchar_t profileFile[MAX_PATH] = {0};
			PathCombineW(profileFile, mediaLibrary.GetIniDirectoryW(), L"profile.txt");
			hProfile = CreateFileW(profileFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hProfile != INVALID_HANDLE_VALUE)
			{
				// just to make sure we don't over-write things
				SetFilePointer(hProfile, NULL, NULL, FILE_END);
			}
		}
		return hProfile;
	}
	return INVALID_HANDLE_VALUE;
}

void unloadPlugin(PMPDevicePlugin *devplugin, int n=-1)
{
	if(n == -1) for(int i=0; i<m_plugins.GetSize(); i++) if(m_plugins.Get(i) == (void*)devplugin) n=i;
	devplugin->quit();
	//if (devplugin->hDllInstance) FreeLibrary(devplugin->hDllInstance);
	m_plugins.Del(n);
}

void unloadDevPlugins()
{
	int i=m_plugins.GetSize();
	HANDLE hProfile = GetProfileFileHandle();
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	if (hProfile != INVALID_HANDLE_VALUE)
	{
		DWORD written = 0;
		WriteFile(hProfile, L"\r\n", 2, &written, NULL);
	}

	while (i-->0)  // reverse order to aid in not fucking up subclassing shit
	{
	    PMPDevicePlugin *devplugin=(PMPDevicePlugin *)m_plugins.Get(i);
		wchar_t profile[MAX_PATH*2] = {0}, filename[MAX_PATH] = {0};
		LARGE_INTEGER starttime, endtime;
		if (hProfile != INVALID_HANDLE_VALUE)
		{
			GetModuleFileNameW(devplugin->hDllInstance, filename, MAX_PATH);
			QueryPerformanceCounter(&starttime);
		}

		unloadPlugin(devplugin,i);

		if (hProfile != INVALID_HANDLE_VALUE)
		{
			QueryPerformanceCounter(&endtime);

			DWORD written = 0;
			unsigned int ms = (UINT)((endtime.QuadPart - starttime.QuadPart) * 1000 / freq.QuadPart);
			int len = swprintf(profile, L"Portable\t%s\t[%s]\t%dms\r\n", filename, devplugin->description, ms);
			WriteFile(hProfile, profile, len*sizeof(wchar_t), &written, NULL);
		}
	}

	if (hProfile != INVALID_HANDLE_VALUE)
	{
		DWORD written = 0;
		WriteFile(hProfile, L"\r\n", 2, &written, NULL);
		CloseHandle(hProfile);
	}
}