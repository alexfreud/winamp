#include "main.h"
#include "api.h"
#include <multimon.h>

#undef GetSystemMetrics
void getViewport(RECT *r, HWND wnd, int full, RECT *sr)
{
	POINT *p = NULL;
	if (p || sr || wnd)
	{
		static int initted = 0;
		static HMONITOR (WINAPI *Mfp)(POINT pt, DWORD dwFlags);
		static HMONITOR (WINAPI *Mfr)(LPCRECT lpcr, DWORD dwFlags);
		static HMONITOR (WINAPI *Mfw)(HWND wnd, DWORD dwFlags);
		static BOOL (WINAPI *Gmi)(HMONITOR mon, LPMONITORINFOEX lpmi);
		if (!initted)
		{
			HINSTANCE h = LoadLibraryW(L"user32.dll");
			if (h)
			{
				Mfp = (HMONITOR (WINAPI *)(POINT, DWORD)) GetProcAddress(h, "MonitorFromPoint");
				Mfr = (HMONITOR (WINAPI *)(LPCRECT, DWORD)) GetProcAddress(h, "MonitorFromRect");
				Mfw = (HMONITOR (WINAPI *)(HWND, DWORD)) GetProcAddress(h, "MonitorFromWindow");
				Gmi = (BOOL (WINAPI *)(HMONITOR, LPMONITORINFOEX)) GetProcAddress(h, "GetMonitorInfoW");
			}
			initted = 1;
		}

		if (Mfp && Mfr && Mfw && Gmi)
		{
			HMONITOR hm = NULL;

			if (sr)
				hm = Mfr(sr, MONITOR_DEFAULTTONEAREST);
			else if (wnd)
				hm = Mfw(wnd, MONITOR_DEFAULTTONEAREST);
			else if (p)
				hm = Mfp(*p, MONITOR_DEFAULTTONEAREST);

			if (hm)
			{
				MONITORINFOEXW mi;
				memset(&mi, 0, sizeof(mi));
				mi.cbSize = sizeof(mi);

				if (Gmi(hm, &mi))
				{
					if (!full)
						*r = mi.rcWork;
					else
						*r = mi.rcMonitor;

					return ;
				}
			}
		}
	}
	if (full)
	{ // this might be borked =)
		r->top    = r->left = 0;
		r->right  = GetSystemMetrics(SM_CXSCREEN);
		r->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		SystemParametersInfoW(SPI_GETWORKAREA, 0, r, 0);
	}
}

VideoConfig::VideoConfig() : group(0), 
itemYV12(0), itemOverlay(0), itemVsync(0), itemDDraw(0)
{}

bool VideoConfig::yv12()
{
	GetGroup();
	if (!itemYV12)
		return false;

	return itemYV12->GetBool();
}

bool VideoConfig::overlays()
{
	GetGroup();
	if (!itemOverlay)
		return true; // overlay by default

	return itemOverlay->GetBool();
}

bool VideoConfig::vsync()
{
	GetGroup();
	if (!itemVsync)
		return false; // no vsync by default

	return itemVsync->GetBool();
}

bool VideoConfig::ddraw()
{
	GetGroup();
	if (!itemDDraw)
		return true;

	return itemDDraw->GetBool();
}

void VideoConfig::GetGroup()
{
	if (group)
		return ;

	// {2135E318-6919-4bcf-99D2-62BE3FCA8FA6}
	static const GUID videoConfigGroupGUID =
	    { 0x2135e318, 0x6919, 0x4bcf, { 0x99, 0xd2, 0x62, 0xbe, 0x3f, 0xca, 0x8f, 0xa6 } };

	group = AGAVE_API_CONFIG->GetGroup(videoConfigGroupGUID);
	if (group)
	{
		itemYV12 = group->GetItem(L"YV12");
		itemOverlay = group->GetItem(L"overlay");
		itemVsync = group->GetItem(L"vsync");
		itemDDraw = group->GetItem(L"ddraw");
	}
}


VideoConfig config_video;
