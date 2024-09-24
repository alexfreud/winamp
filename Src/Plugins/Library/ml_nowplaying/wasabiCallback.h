#ifndef NULLSOFT_NOWPLAYING_PLUGIN_WASABI_CALLBACK_HEADER
#define NULLSOFT_NOWPLAYING_PLUGIN_WASABI_CALLBACK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <api/syscb/callbacks/syscb.h>
#include <api/syscb/callbacks/browsercb.h>

class WasabiCallback : public SysCallback
{
protected:
	WasabiCallback();
	~WasabiCallback();

public:
	static HRESULT CreateInstance(WasabiCallback **instance);

public:
	/*** Dispatchable ***/
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/*** SysCallback ***/
	FOURCC GetEventType();
    int Notify(int msg, intptr_t param1 = 0, intptr_t param2 = 0);

protected:
	// set *override  = true to prevent the URL from being opened
	// leave it alone otherwise (in case someone else wanted to override it)
	int OpenURL(const wchar_t *url, bool *override);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
};

#endif //NULLSOFT_NOWPLAYING_PLUGIN_WASABI_CALLBACK_HEADER