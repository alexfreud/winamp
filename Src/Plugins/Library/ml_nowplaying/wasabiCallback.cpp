#include "main.h"
#include "./wasabiCallback.h"
#include "./navigation.h"
#include "./service.h"
#include "../replicant/nu/Autowide.h"

WasabiCallback::WasabiCallback()	
	: ref(1)
{
}

WasabiCallback::~WasabiCallback()	
{
}

HRESULT WasabiCallback::CreateInstance(WasabiCallback **instance)
{
	if (NULL == instance) return E_POINTER;

	*instance = new WasabiCallback();
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t WasabiCallback::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t WasabiCallback::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int WasabiCallback::QueryInterface(GUID interface_guid, void **object)
{
	return 0;
}

FOURCC WasabiCallback::GetEventType()
{ 
	return SysCallback::BROWSER; 
}

int WasabiCallback::Notify(int msg, intptr_t param1, intptr_t param2) 
{
	switch (msg) 
	{
		case BrowserCallback::ONOPENURL:
			return OpenURL(reinterpret_cast<const wchar_t*>(param1), reinterpret_cast<bool *>(param2));
	}
	return 0;
}

int WasabiCallback::OpenURL(const wchar_t *url, bool *override)
{
	WCHAR szTemplate[1024] = L"http://client.winamp.com/nowplaying";
	INT cchTemplate = ARRAYSIZE(szTemplate) - 1;
	lstrcpynW(szTemplate, AutoWide(g_config->ReadString("nowplayingurl", "http://client.winamp.com/nowplaying")), ARRAYSIZE(szTemplate));

	if (NULL != url && 
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, url, cchTemplate, szTemplate, cchTemplate))
	{
		if (SUCCEEDED(Navigation_ShowService(SERVICE_ID, url, 
						NAVFLAG_FORCEACTIVE | NAVFLAG_ENSUREMLVISIBLE | NAVFLAG_ENSUREITEMVISIBLE)))
		{
			*override = true;
			return 1;
		}
	}
	return 0;
}

#define CBCLASS WasabiCallback
START_DISPATCH;
  CB(ADDREF, AddRef);
  CB(RELEASE, Release);
  CB(QUERYINTERFACE, QueryInterface);
  CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
  CB(SYSCALLBACK_NOTIFY, Notify);
END_DISPATCH;
#undef CBCLASS