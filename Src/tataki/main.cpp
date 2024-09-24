#include "api__tataki.h"
#include <api/service/waservicefactory.h>
#include <tataki/export.h>
#include <tataki/canvas/bltcanvas.h>
#include "../nu/ServiceWatcher.h"

api_service *WASABI_API_SVC=0;
api_application *WASABI_API_APP=0;
api_syscb *WASABI_API_SYSCB=0;
api_font *WASABI_API_FONT=0;
wnd_api *WASABI_API_WND=0;
api_config *WASABI_API_CONFIG=0;
imgldr_api *WASABI_API_IMGLDR=0;
api_memmgr *WASABI_API_MEMMGR=0;
api_skin *WASABI_API_SKIN=0;
api_random *AGAVE_API_RANDOM=0;
api_palette *WASABI_API_PALETTE=0;
DWORD bitmap_cache_tls=TLS_OUT_OF_INDEXES;

ServiceWatcher serviceWatcher;
template <class api_t>
api_t *GetService(GUID serviceGUID)
{	
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(serviceGUID);
	if (sf)
		return reinterpret_cast<api_t *>( sf->getInterface() );
	else
		return 0;

}

template <class api_t>
inline void ReleaseService(GUID serviceGUID, api_t *&service)
{
	if (service)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(serviceGUID);
		if (sf)
			sf->releaseInterface(service);

		service=0;
	}
}

static size_t refCount=0;
TATAKIAPI size_t Tataki::Init(api_service *_serviceApi)
{
	refCount++;
	if (bitmap_cache_tls == TLS_OUT_OF_INDEXES)
		bitmap_cache_tls=TlsAlloc();

	if (!WASABI_API_SVC)
	{
		WASABI_API_SVC = _serviceApi;
		WASABI_API_SYSCB = GetService<api_syscb>(syscbApiServiceGuid);
		AGAVE_API_RANDOM = GetService<api_random>(randomApiGUID);
		WASABI_API_PALETTE = GetService<api_palette>(PaletteManagerGUID);

		serviceWatcher.WatchWith(WASABI_API_SVC);
		serviceWatcher.WatchFor(&WASABI_API_APP, applicationApiServiceGuid);
		serviceWatcher.WatchFor(&WASABI_API_FONT, fontApiServiceGuid);
		serviceWatcher.WatchFor(&WASABI_API_WND, wndApiServiceGuid);
		serviceWatcher.WatchFor(&WASABI_API_CONFIG, configApiServiceGuid);
		serviceWatcher.WatchFor(&WASABI_API_IMGLDR, imgLdrApiServiceGuid);
		serviceWatcher.WatchFor(&WASABI_API_MEMMGR, memMgrApiServiceGuid);
		serviceWatcher.WatchFor(&WASABI_API_SKIN, skinApiServiceGuid);

		// register for service callbacks in case any of these don't exist yet
		WASABI_API_SYSCB->syscb_registerCallback(&serviceWatcher);
	}
	return refCount;
}

TATAKIAPI size_t Tataki::Quit()
{
	if (!--refCount)
	{
		serviceWatcher.StopWatching();
		serviceWatcher.Clear();

		ReleaseService(syscbApiServiceGuid,	WASABI_API_SYSCB);
		ReleaseService(applicationApiServiceGuid, WASABI_API_APP);
		ReleaseService(fontApiServiceGuid,WASABI_API_FONT);
		ReleaseService(wndApiServiceGuid, WASABI_API_WND);
		ReleaseService(configApiServiceGuid, WASABI_API_CONFIG);
		ReleaseService(imgLdrApiServiceGuid, WASABI_API_IMGLDR);
		ReleaseService(memMgrApiServiceGuid, WASABI_API_MEMMGR);
		ReleaseService(skinApiServiceGuid, WASABI_API_SKIN);
		ReleaseService(randomApiGUID, AGAVE_API_RANDOM);
		ReleaseService(PaletteManagerGUID, WASABI_API_PALETTE);

		// unregister callbacks
		// release any services we have
		WASABI_API_SVC = 0;
	}
	return refCount;
}

extern "C" BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved
)
{
	Wasabi::Std::Initialize();
	if (fdwReason == DLL_PROCESS_DETACH && bitmap_cache_tls!=TLS_OUT_OF_INDEXES)
	{
		TlsFree(bitmap_cache_tls);
		bitmap_cache_tls=TLS_OUT_OF_INDEXES;
	}
	else if (fdwReason == DLL_THREAD_DETACH && bitmap_cache_tls!=TLS_OUT_OF_INDEXES)
	{
		BltCanvas *cache_canvas = (BltCanvas *)TlsGetValue(bitmap_cache_tls);
		if (cache_canvas)
		{
			delete cache_canvas;
			TlsSetValue(bitmap_cache_tls, 0); // this is probably unnecessary but just in case
		}

	}
	return TRUE;
}
