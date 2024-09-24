#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_DOWNLOADCALLBACKT_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_DOWNLOADCALLBACKT_H

#include "wac_downloadManager_api.h"

#define WIN32_LEAN_AND_MEAN

//#include <windows.h> // for InterlockedIncrememt/Decrement
#include <atomic>

/* DownloadCallbackT is reference counted for you. if you don't like that, inherit from ifc_downloadManagerCallback yourself */
template <class T>
class DownloadCallbackT : public ifc_downloadManagerCallback
{
public:
	size_t AddRef() 
	{
		return _ref_count.fetch_add( 1 );
	}

	size_t Release() 
	{
		if (_ref_count.load() == 0 )
			return _ref_count.load();

		LONG l_ref_count = _ref_count.fetch_sub( 1 );
		if ( !l_ref_count )
			delete( static_cast<T *>( this ) );

		return l_ref_count;
	}

protected:
	DownloadCallbackT()                                               {}

	void OnFinish( DownloadToken token )                              { Release(); }
	void OnTick( DownloadToken token )                                {}
	void OnError( DownloadToken token, int error )                    { Release(); }
	void OnCancel( DownloadToken token )                              { Release(); }
	void OnConnect( DownloadToken token )                             {}
	void OnInit( DownloadToken token )                                {}
	void OnData( DownloadToken token, void *data, size_t datalen )    {}

	int GetSource(wchar_t *source, size_t source_cch)                 { return 1;}
	int GetTitle(wchar_t *title, size_t title_cch)                    { return 1;}
	int GetLocation(wchar_t *location, size_t location_cch)           { return 1;}

#define CBCLASS T
#define CBCLASST DownloadCallbackT<T>
	START_DISPATCH_INLINE;
	CBT(ADDREF, AddRef);
	CBT(RELEASE, Release);
	VCBT(IFC_DOWNLOADMANAGERCALLBACK_ONFINISH,    OnFinish);
	VCBT(IFC_DOWNLOADMANAGERCALLBACK_ONTICK,      OnTick);
	VCBT(IFC_DOWNLOADMANAGERCALLBACK_ONERROR,     OnError);
	VCBT(IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL,    OnCancel);
	VCBT(IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT,   OnConnect);
	VCBT(IFC_DOWNLOADMANAGERCALLBACK_ONINIT,      OnInit);
	VCBT(IFC_DOWNLOADMANAGERCALLBACK_ONDATA,      OnData);
	CBT( IFC_DOWNLOADMANAGERCALLBACK_GETSOURCE,   GetSource);
	CBT( IFC_DOWNLOADMANAGERCALLBACK_GETTITLE,    GetTitle);
	CBT( IFC_DOWNLOADMANAGERCALLBACK_GETLOCATION, GetLocation);
	END_DISPATCH;
#undef CBCLASS
#undef CBCLASST

private:
	std::atomic<std::size_t> _ref_count;
};

#endif // !NULLSOFT_WAC_DOWNLOAD_MANAGER_DOWNLOADCALLBACKT_H
