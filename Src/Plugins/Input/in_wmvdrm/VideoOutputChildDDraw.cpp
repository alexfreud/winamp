#include "main.h"
#include "VideoOutputChildDDraw.h"
#include <multimon.h>
#include <ddraw.h>

class MonitorFinder
{
public:
	MonitorFinder(HMONITOR hm) : m_monitor_to_find(hm), m_found_devguid(0)
	{}

	HMONITOR m_monitor_to_find;
	int m_found_devguid;
	GUID m_devguid;
};

static BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	MonitorFinder *ovo = (MonitorFinder *)lpContext;
	if (ovo->m_found_devguid) return 1;
	if (hm == ovo->m_monitor_to_find)
	{
		ovo->m_devguid = *lpGUID;
		ovo->m_found_devguid = 1;
	}
	return 1;
}

void VideoOutputChildDDraw::update_monitor_coords()
{
	//find the correct monitor if multiple monitor support is present
	m_mon_x = 0;
	m_mon_y = 0;

	HINSTANCE h = LoadLibrary(L"user32.dll");
	if (h)
	{
		HMONITOR (WINAPI *Mfp)(POINT pt, DWORD dwFlags) = (HMONITOR (WINAPI *)(POINT, DWORD)) GetProcAddress(h, "MonitorFromPoint");
		HMONITOR (WINAPI *Mfr)(LPCRECT lpcr, DWORD dwFlags) = (HMONITOR (WINAPI *)(LPCRECT, DWORD)) GetProcAddress(h, "MonitorFromRect");
		HMONITOR (WINAPI *Mfw)(HWND wnd, DWORD dwFlags) = (HMONITOR (WINAPI *)(HWND, DWORD)) GetProcAddress(h, "MonitorFromWindow");
		BOOL (WINAPI *Gmi)(HMONITOR mon, LPMONITORINFO lpmi) = (BOOL (WINAPI *)(HMONITOR, LPMONITORINFO)) GetProcAddress(h, "GetMonitorInfoA");
		if (Mfp && Mfr && Mfw && Gmi)
		{
			HMONITOR hm = Mfw(parent, 0);
			if (hm)
			{
				HINSTANCE hdd = LoadLibrary(L"ddraw.dll");
				if (hdd)
				{
					typedef BOOL (FAR PASCAL * LPDDENUMCALLBACKEXA)(GUID FAR *, LPSTR, LPSTR, LPVOID, HMONITOR);
					typedef HRESULT (WINAPI * LPDIRECTDRAWENUMERATEEX)( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);
					LPDIRECTDRAWENUMERATEEX lpDDEnumEx;
					lpDDEnumEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(hdd, "DirectDrawEnumerateExW");
					if (lpDDEnumEx)
					{
						MonitorFinder finder(hm);
						
						lpDDEnumEx(&DDEnumCallbackEx, &finder, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
						foundGUID=!!finder.m_found_devguid;
						if (foundGUID)
						{
							m_devguid=finder.m_devguid;
							MONITORINFOEXW mi;
							memset(&mi, 0, sizeof(mi));
							mi.cbSize = sizeof(mi);
							if (Gmi(hm, &mi))
							{
								m_mon_x = mi.rcMonitor.left;
								m_mon_y = mi.rcMonitor.top;
							}
						}
					}
					FreeLibrary(hdd);
				}
			}
		}
		FreeLibrary(h);
	}
}

