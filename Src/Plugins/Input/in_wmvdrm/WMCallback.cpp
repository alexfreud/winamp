#include "Main.h"
#include "WMCallback.h"
#include <algorithm>
#include "WMHandler.h"
#include "util.h"
#include <cassert>

#define CAST_TO(x) if (riid== IID_##x) { *ppvObject=static_cast<x *>(this); AddRef(); return S_OK; }
#define CAST_TO_VIA(x,y) if (riid== IID_##x) { *ppvObject=static_cast<y *>(this); AddRef(); return S_OK; }
HRESULT WMCallback::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject )
{
	CAST_TO(IWMReaderCallback);
	CAST_TO_VIA(IUnknown, IWMReaderCallback); 
	CAST_TO(IWMReaderCallbackAdvanced);
#ifdef _DEBUG
	CAST_TO(IWMCredentialCallback);
#endif
	
	*ppvObject = NULL;
	return E_NOINTERFACE;
}

ULONG WMCallback::AddRef()
{
	return InterlockedIncrement(&refCount);
}

ULONG WMCallback::Release()
{
	if (InterlockedDecrement(&refCount) == 0)
	{
		delete this;
		return 0;
	}

	return refCount;
}


HRESULT WMCallback::OnStatus(WMT_STATUS Status, HRESULT hr, WMT_ATTR_DATATYPE dwType,
                             BYTE __RPC_FAR *pValue, void __RPC_FAR *pvContext)
{
	if (!handler)
		return S_OK;

	switch (Status)
	{
		WMTCASE(WMT_INIT_PLAYLIST_BURN)
			handler->InitPlaylistBurn();
			break;

		WMTCASE(WMT_NO_RIGHTS)
		handler->NoRights((wchar_t *)pValue);
		break;

		WMTCASE(WMT_NO_RIGHTS_EX)
		handler->NoRightsEx((WM_GET_LICENSE_DATA *&)pValue);
		break;

		WMTCASE(WMT_NEEDS_INDIVIDUALIZATION)
		handler->Individualize();
		break;

		WMTCASE(WMT_END_OF_STREAMING)
		break;

		WMTCASE(WMT_LICENSEURL_SIGNATURE_STATE)
		handler->SignatureState((WMT_DRMLA_TRUST *&)pValue);
		break;

		WMTCASE(WMT_ACQUIRE_LICENSE)
		WMT_SHOW_HR_CODE(hr)
		switch (hr)
		{
		case NS_S_DRM_LICENSE_ACQUIRED:
			handler->LicenseAcquired();
			break;
		case NS_S_DRM_MONITOR_CANCELLED:
			handler->MonitorCancelled();
			break;
		case NS_S_DRM_ACQUIRE_CANCELLED:
			handler->SilentCancelled();
			break;
		default:
			handler->AcquireLicense((WM_GET_LICENSE_DATA *&)pValue);
		}
		break;

		WMTCASE(WMT_INDIVIDUALIZE)
		handler->IndividualizeStatus((WM_INDIVIDUALIZE_STATUS *)pValue);
		break;

		//the file has been opened
		WMTCASE(WMT_OPENED)
		if (SUCCEEDED(hr))
			handler->Opened();
		else
		{
			switch (hr)
			{
				WMTCASE(NS_E_DRM_APPCERT_REVOKED)
				WMTCASE(NS_E_DRM_LICENSE_APP_NOTALLOWED)
				handler->DRMExpired();
			WMTCASE(NS_E_LICENSE_REQUIRED)
			handler->LicenseRequired();
			break;
			WMTCASE(NS_E_DRM_NEEDS_INDIVIDUALIZATION)
			handler->NeedsIndividualization();
			break;
				WMTCASE(E_ACCESSDENIED)
				handler->AccessDenied();
				break;
			default:
				WMT_SHOW_HR_CODE(hr);
				handler->Error();
				return S_OK;
			}
			handler->OpenCalled();
		}

		break;

		// Playback of the opened file has begun.
		WMTCASE( WMT_STARTED)
		if (SUCCEEDED(hr))
			handler->Started();
		else
		{
			switch (hr)
			{
				WMTCASE(E_ABORT)
				//handler->OpenFailed();
				break;
				WMTCASE(NS_E_DRM_REOPEN_CONTENT)
				handler->Error();
				break;
			default:
				WMT_SHOW_HR_CODE(hr);
				handler->Error();
				break;
			}
		}
		break;

		WMTCASE( WMT_NEW_METADATA)
		if (SUCCEEDED(hr))
			handler->NewMetadata();
		break;

		// The previously playing reader has stopped.
		WMTCASE( WMT_STOPPED)
		if (SUCCEEDED(hr))
			handler->Stopped();
		else
		{
			WMT_SHOW_HR_CODE(hr);
			handler->Error();
		}
		break;

		// The previously playing reader has stopped.
		WMTCASE( WMT_CLOSED)
		if (SUCCEEDED(hr))
			handler->Closed();
		else
		{
			WMT_SHOW_HR_CODE(hr);
			handler->Error();
		}
		break;

		WMTCASE(WMT_ERROR)
		WMT_SHOW_HR_CODE(hr);
		//handler->Error();

		break;

		WMTCASE( WMT_BUFFERING_START)
		if (SUCCEEDED(hr))
			handler->BufferingStarted();
		break;

		WMTCASE( WMT_BUFFERING_STOP)
		if (SUCCEEDED(hr))
			handler->BufferingStopped();
		break;

		WMTCASE( WMT_EOF)
			WMT_SHOW_HR_CODE(hr);
		handler->EndOfFile();
		break;

		WMTCASE( WMT_LOCATING)
			handler->Locating();
		break;

		WMTCASE( WMT_CONNECTING)
			handler->Connecting();
		break;

		WMTCASE( WMT_PREROLL_READY)
		break;

		WMTCASE( WMT_PREROLL_COMPLETE)
		break;
		WMTCASE(WMT_NEW_SOURCEFLAGS)
		break;
		WMTCASE(WMT_MISSING_CODEC)

		WMT_SHOW_HR_CODE(hr);
#ifdef DEBUG
		std::cerr << dwType << std::endl;
		std::wcerr << GuidString(*(GUID *)pValue) << std::endl;
#endif
		break;
	default:
		#ifdef DEBUG
		std::cerr << "unknown message = " << Status << std::endl;
		#endif
		break;
	};
	return S_OK;
}

HRESULT WMCallback::OnStreamSelection(WORD wStreamCount, WORD *pStreamNumbers, WMT_STREAM_SELECTION *pSelections, void *pvContext)
{
	#ifdef DEBUG
	std::cerr << "OnStreamSelection" << std::endl;
	#endif
	return E_NOTIMPL;
}

HRESULT WMCallback::OnOutputPropsChanged(DWORD dwOutputNum, WM_MEDIA_TYPE *pMediaType, void *pvContext)
{
				#ifdef DEBUG
		std::cerr << "OnOutputPropsChanged" << std::endl;
		#endif

	return E_NOTIMPL;
}

HRESULT WMCallback::AllocateForStream(WORD wStreamNum, DWORD cbBuffer, INSSBuffer **ppBuffer, void *pvContext)
{
	return E_NOTIMPL;
}

HRESULT WMCallback::AllocateForOutput(DWORD dwOutputNum, DWORD cbBuffer, INSSBuffer **ppBuffer, void *pvContext)
{
	return E_NOTIMPL;
}

HRESULT WMCallback::OnStreamSample(WORD wStreamNum, QWORD cnsSampleTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer *pSample, void *pvContext)
{
	return E_NOTIMPL;
}

// 0x5A, 0xA5, 0x00, 0x03, 0x74, 0x00, 0x01, 0x01, 0x77,
//------------------------------------------------------------------------------
// Name: CWAPlugin::OnSample()
// Desc: IWMReaderCallback method to process samples.
//------------------------------------------------------------------------------
HRESULT WMCallback::OnSample(DWORD dwOutputNum, QWORD cnsSampleTime,
                             QWORD cnsSampleDuration, DWORD dwFlags,
                             INSSBuffer __RPC_FAR *pSample, void __RPC_FAR *pvContext)
{
	if (!handler)
		return S_OK;
	handler->SampleReceived(cnsSampleTime, cnsSampleDuration, dwOutputNum, dwFlags, pSample);
	return S_OK;
}

HRESULT WMCallback::OnTime(QWORD cnsCurrentTime, void *pvContext)
{
	if (!handler)
		return S_OK;
	handler->TimeReached(cnsCurrentTime);
	return S_OK;
}


HRESULT WMCallback::AcquireCredentials(WCHAR* pwszRealm, WCHAR* pwszSite,
																			 WCHAR* pwszUser, DWORD cchUser, 
																			 WCHAR*  pwszPassword, DWORD cchPassword,
																			 HRESULT hrStatus,  DWORD* pdwFlags)
{
#ifdef _DEBUG
	std::cout << "WMCallback::AcquireCredentials" << std::endl;
	std::wcout << pwszRealm << std::endl;
	std::wcout << pwszSite << std::endl;
	std::wcout << HRErrorCode(hrStatus) << std::endl;
	return S_OK;
#endif
	return E_NOTIMPL;
}