#ifndef NULLSOFT_WMCALLBACK
#define NULLSOFT_WMCALLBACK

#include <wmsdk.h>
#include <deque>
#include "WMHandler.h"


class WMCallback :  public IWMReaderCallback, public IWMReaderCallbackAdvanced, public IWMCredentialCallback
{

public:
	WMCallback() : refCount(0), handler(0)
	{
		AddRef();
	}

	~WMCallback()
	{
	}

	WMHandler &operator >> (WMHandler *_handler)
	{
		handler = _handler;
		return *handler;
	}


private:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE OnStatus(WMT_STATUS Status, HRESULT hr, WMT_ATTR_DATATYPE dwType, BYTE __RPC_FAR *pValue, void __RPC_FAR *pvContext);
	virtual HRESULT STDMETHODCALLTYPE OnSample(DWORD dwOutputNum, QWORD cnsSampleTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer __RPC_FAR *pSample, void __RPC_FAR *pvContext);

	/* IWMReaderCallbackAdvanced */
	HRESULT STDMETHODCALLTYPE OnStreamSample(WORD wStreamNum, QWORD cnsSampleTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer *pSample, void *pvContext);
	HRESULT STDMETHODCALLTYPE OnTime(QWORD cnsCurrentTime, void *pvContext);
	HRESULT STDMETHODCALLTYPE OnStreamSelection(WORD wStreamCount, WORD *pStreamNumbers, WMT_STREAM_SELECTION *pSelections, void *pvContext);
	HRESULT STDMETHODCALLTYPE OnOutputPropsChanged(DWORD dwOutputNum, WM_MEDIA_TYPE *pMediaType, void *pvContext);
	HRESULT STDMETHODCALLTYPE AllocateForStream(WORD wStreamNum, DWORD cbBuffer, INSSBuffer **ppBuffer, void *pvContext);
	HRESULT STDMETHODCALLTYPE AllocateForOutput(DWORD dwOutputNum, DWORD cbBuffer, INSSBuffer **ppBuffer, void *pvContext);

	/* IWMCredentialCallback */
	HRESULT STDMETHODCALLTYPE AcquireCredentials(WCHAR* pwszRealm, WCHAR* pwszSite, WCHAR* pwszUser, DWORD cchUser, WCHAR*  pwszPassword, DWORD cchPassword, HRESULT hrStatus,  DWORD* pdwFlags);

	long refCount;
		WMHandler *handler;

};

#endif
