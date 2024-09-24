#include "main.h"
#include "ReplayGain.h"
#include <api/service/waservicefactory.h>
static obj_replaygain *replayGain = 0;
HANDLE rgThread = 0;
static HANDLE killSwitch = 0;
extern HWND m_extract_wnd;

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (plugin.service)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (plugin.service && api_t)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

DWORD WINAPI RGProc(void *data)
{
	while (WaitForSingleObjectEx(killSwitch, INFINITE, TRUE) != WAIT_OBJECT_0);

	return 0;
}

void CreateGain()
{
	killSwitch = CreateEvent(0, FALSE, FALSE, 0);
	DWORD dummy;
	rgThread = CreateThread(0, 0, RGProc, 0, 0, &dummy);
}

void CALLBACK StartGain(ULONG_PTR data)
{
	int mode = (int)data;

	ServiceBuild(replayGain, RGGUID);
	if (replayGain)
		replayGain->Open(mode);
}

void CALLBACK WriteGain(ULONG_PTR data)
{
	if (replayGain)
		replayGain->Write();

	HANDLE notifyHandle =(HANDLE)data;
	if (notifyHandle)
		SetEvent(notifyHandle);

	PostMessage(m_extract_wnd, WM_APP+4, 0, 0);
}

void CALLBACK CalculateGain(ULONG_PTR data)
{
	wchar_t *lastfn = (wchar_t *)data;
	if (replayGain)
	{
		PostMessage(m_extract_wnd, WM_APP+2, 0, 0);
		replayGain->ProcessTrack(lastfn);
	}
	free(lastfn);
	PostMessage(m_extract_wnd, WM_APP+3, 0, 0);
}

void CALLBACK CloseGain(ULONG_PTR data)
{
	if (replayGain)
	{
		replayGain->Close();
		ServiceRelease(replayGain, RGGUID);
		replayGain = 0;
	}
}

void CALLBACK QuitThread(ULONG_PTR data)
{
	if (rgThread)
	{
		SetEvent(killSwitch);
	}
}
