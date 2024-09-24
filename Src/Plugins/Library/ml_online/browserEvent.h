#ifndef NULLSOFT_ONLINEMEDIA_PLUGIN_BROWSER_EVENT_HANDLER_HEADER
#define NULLSOFT_ONLINEMEDIA_PLUGIN_BROWSER_EVENT_HANDLER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <ifc_ombrowserevent.h>

class BrowserEvent : public ifc_ombrowserevent
{

protected:
	BrowserEvent();
	~BrowserEvent();

public:
	static HRESULT CreateInstance(BrowserEvent **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_ombrowserevent */
	void WindowCreate(HWND hwnd, const GUID *windowType);
	void WindowClose(HWND hwnd, const GUID *windowType);


protected:
	ULONG ref;

protected:
	RECVS_DISPATCH;
};




#endif //NULLSOFT_ONLINEMEDIA_PLUGIN_BROWSER_EVENT_HANDLER_HEADER