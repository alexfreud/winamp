#include "jnetcom.h"
#include "../nu/AutoChar.h"
/* --- Jnetlib COM object --- */
extern "C" extern HANDLE DuplicateCurrentThread();

JNetCOM::JNetCOM( IDispatch *_dispatch )
{
	refCount     = 1;
	token        = 0;
	dispatch     = _dispatch;
	threadId     = GetCurrentThreadId();
	threadHandle = DuplicateCurrentThread();
	retained     = false;

	if ( NULL != dispatch )
		dispatch->AddRef();
}

JNetCOM::~JNetCOM()
{
	if ( retained )
	{
		if ( NULL != WAC_API_DOWNLOADMANAGER )
			WAC_API_DOWNLOADMANAGER->ReleaseDownload( token );
	}

	CloseHandle( threadHandle );

	if ( NULL != dispatch )
		dispatch->Release();
}

enum
{
	DISP_JNETCOM_ABORT,
	DISP_JNETCOM_ADDHEADER,
	DISP_JNETCOM_CONNECT,
	DISP_JNETCOM_GETCONTENT,
	DISP_JNETCOM_GETCONTENTASSTRING,
	DISP_JNETCOM_GETERRORSTRING,
	DISP_JNETCOM_GETHEADER,
	DISP_JNETCOM_GETREPLY,
	DISP_JNETCOM_GETREPLYCODE,
	DISP_JNETCOM_GETURL,
	DISP_JNETCOM_SETPOSTSTRING,
};

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT JNetCOM::GetIDsOfNames( REFIID riid, OLECHAR FAR *FAR *rgszNames, unsigned int cNames, LCID lcid, DISPID FAR *rgdispid )
{
	bool unknowns = false;
	for ( unsigned int i = 0; i != cNames; i++ )
	{
		CHECK_ID( "Abort",              DISP_JNETCOM_ABORT );
		CHECK_ID( "AddHeader",          DISP_JNETCOM_ADDHEADER );
		CHECK_ID( "Connect",            DISP_JNETCOM_CONNECT );
		CHECK_ID( "GetContent",         DISP_JNETCOM_GETCONTENT );
		CHECK_ID( "GetContentAsString", DISP_JNETCOM_GETCONTENTASSTRING );
		CHECK_ID( "GetErrorString",     DISP_JNETCOM_GETERRORSTRING );
		CHECK_ID( "GetHeader",          DISP_JNETCOM_GETHEADER );
		CHECK_ID( "GetReply",           DISP_JNETCOM_GETREPLY );
		CHECK_ID( "GetReplyCode",       DISP_JNETCOM_GETREPLYCODE );
		CHECK_ID( "GetURL",             DISP_JNETCOM_GETURL );
		CHECK_ID( "SetPOSTString",      DISP_JNETCOM_SETPOSTSTRING );

		rgdispid[ i ] = DISPID_UNKNOWN;
		unknowns = true;
	}

	if ( unknowns )
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JNetCOM::GetTypeInfo( unsigned int itinfo, LCID lcid, ITypeInfo FAR *FAR *pptinfo )
{
	return E_NOTIMPL;
}

HRESULT JNetCOM::GetTypeInfoCount( unsigned int FAR *pctinfo )
{
	return E_NOTIMPL;
}

HRESULT JNetCOM::Invoke( DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR *pexecinfo, unsigned int FAR *puArgErr )
{
	switch ( dispid )
	{
		case DISP_JNETCOM_ABORT:
			return Abort();
		case DISP_JNETCOM_ADDHEADER:
			return AddHeader( pdispparams->rgvarg[ 0 ].bstrVal );
		case DISP_JNETCOM_CONNECT:
			if ( pdispparams->cArgs == 2 )
				return Connect( pdispparams->rgvarg[ 1 ].bstrVal, pdispparams->rgvarg[ 0 ].bstrVal );
			else
				return Connect( pdispparams->rgvarg[ 0 ].bstrVal, L"GET" );
		case DISP_JNETCOM_GETCONTENT:
			return GetContent( pvarResult );
		case DISP_JNETCOM_GETCONTENTASSTRING:
			return GetContentAsString( pvarResult );
		case DISP_JNETCOM_GETERRORSTRING:
			return GetErrorString( pvarResult );
		case DISP_JNETCOM_GETHEADER:
			return GetHeader( pdispparams->rgvarg[ 0 ].bstrVal, pvarResult );
		case DISP_JNETCOM_GETREPLY:
			return GetReply( pvarResult );
		case DISP_JNETCOM_GETREPLYCODE:
			return GetReplyCode( pvarResult );
		case DISP_JNETCOM_GETURL:
			return GetUrl( pvarResult );
		case DISP_JNETCOM_SETPOSTSTRING:
			break;
	}

	return DISP_E_MEMBERNOTFOUND;
}


STDMETHODIMP JNetCOM::QueryInterface( REFIID riid, PVOID *ppvObject )
{
	if ( !ppvObject )
		return E_POINTER;
	else if ( IsEqualIID( riid, IID_IDispatch ) )
		*ppvObject = (IDispatch *)this;
	else if ( IsEqualIID( riid, IID_IUnknown ) )
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;

		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}

ULONG JNetCOM::AddRef( void )
{
	return InterlockedIncrement( &refCount );
}

ULONG JNetCOM::Release( void )
{
	LONG lRef = InterlockedDecrement( &refCount );
	if ( lRef == 0 )
		delete this;

	return lRef;
}

/* ---- */
HRESULT JNetCOM::Abort()
{
	if ( NULL != WAC_API_DOWNLOADMANAGER )
		WAC_API_DOWNLOADMANAGER->CancelDownload( token );

	return S_OK;
}

HRESULT JNetCOM::AddHeader( LPCWSTR header )
{
	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
	if ( http )
		http->addheader( AutoChar( header, CP_UTF8 ) );

	return S_OK;
}

HRESULT JNetCOM::Connect( LPCWSTR url, LPCWSTR requestMethod )
{
	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	AddRef();

	token = WAC_API_DOWNLOADMANAGER->DownloadEx( AutoChar( url, CP_UTF8 ), this, api_downloadManager::DOWNLOADEX_BUFFER );

	return S_OK;
}

HRESULT JNetCOM::GetContent( VARIANT *variant )
{
	char    dummy[ 1 ] = { 0 };
	size_t  sourcelen  = 0;
	void   *source     = 0;

	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	WAC_API_DOWNLOADMANAGER->GetBuffer( token, &source, &sourcelen );

	if ( !sourcelen || !source )
	{
		source = dummy;
		sourcelen = 1;
	}

	SAFEARRAY *bufferArray = SafeArrayCreateVector( VT_UI1, 0, (ULONG)sourcelen );
	void *data;
	SafeArrayAccessData( bufferArray, &data );
	memcpy( data, source, sourcelen );
	SafeArrayUnaccessData( bufferArray );
	VariantInit( variant );

	V_VT( variant )    = VT_ARRAY | VT_UI1;
	V_ARRAY( variant ) = bufferArray;

	return S_OK;
}

HRESULT JNetCOM::GetContentAsString( VARIANT *variant )
{
	// TODO: try to determine character encoding
	size_t  sourcelen = 0;
	void   *source    = 0;

	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	if ( WAC_API_DOWNLOADMANAGER->GetBuffer( token, &source, &sourcelen ) == 0 )
	{
		if ( source && sourcelen )
		{
			int  len = MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)source, (int)sourcelen, 0, 0 );
			BSTR str = SysAllocStringLen( 0, len );

			MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)source, (int)sourcelen, str, len );

			VariantInit( variant );

			V_VT( variant )   = VT_BSTR;
			V_BSTR( variant ) = str;

			return S_OK;
		}
		else
		{
			VariantInit( variant );

			V_VT( variant )   = VT_BSTR;
			V_BSTR( variant ) = SysAllocString( L"" );

			return S_OK;
		}
	}
	else
		return E_FAIL;
}

HRESULT JNetCOM::GetErrorString( VARIANT *variant )
{
	const char *source = 0;

	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
	if ( http )
		source = http->geterrorstr();

	if ( !source )
		source = "";

	int  sourcelen = (int)strlen( source );
	int  len       = MultiByteToWideChar( CP_ACP, 0, source, sourcelen, 0, 0 );
	BSTR str       = SysAllocStringLen( 0, len );

	MultiByteToWideChar( CP_ACP, 0, source, sourcelen, str, len );
	VariantInit( variant );

	V_VT( variant )   = VT_BSTR;
	V_BSTR( variant ) = str;

	return S_OK;
}

HRESULT JNetCOM::GetHeader( LPCWSTR header, VARIANT *variant )
{
	const char *source = 0;

	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
	if ( http )
		source = http->getheader( AutoChar( header, CP_UTF8 ) );

	if ( !source )
		source = "";

	int  sourcelen = (int)strlen( source );
	int  len       = MultiByteToWideChar( CP_ACP, 0, source, sourcelen, 0, 0 );
	BSTR str       = SysAllocStringLen( 0, len );

	MultiByteToWideChar( CP_ACP, 0, source, sourcelen, str, len );
	VariantInit( variant );

	V_VT( variant )   = VT_BSTR;
	V_BSTR( variant ) = str;

	return S_OK;
}


HRESULT JNetCOM::GetReply( VARIANT *variant )
{
	const char *source = 0;

	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
	if ( http )
		source = http->GetReply();

	if ( !source )
		source = "";

	int  sourcelen = (int)strlen( source );
	int  len       = MultiByteToWideChar( CP_ACP, 0, source, sourcelen, 0, 0 );
	BSTR str       = SysAllocStringLen( 0, len );

	MultiByteToWideChar( CP_ACP, 0, source, sourcelen, str, len );
	VariantInit( variant );

	V_VT( variant )   = VT_BSTR;
	V_BSTR( variant ) = str;

	return S_OK;
}

HRESULT JNetCOM::GetReplyCode( VARIANT *variant )
{
	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	int code = 0;
	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
	if ( http )
		code = http->getreplycode();

	VariantInit( variant );

	V_VT( variant )  = VT_UI4;
	V_UI4( variant ) = code;

	return S_OK;
}

HRESULT JNetCOM::GetUrl( VARIANT *variant )
{
	if ( NULL == WAC_API_DOWNLOADMANAGER )
		return E_POINTER;

	const char *source = 0;
	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
	if ( http )
		source = http->get_url();

	if ( !source )
		source = "";

	int  sourcelen = (int)strlen( source );
	int  len       = MultiByteToWideChar( 1252, 0, source, sourcelen, 0, 0 );
	BSTR str       = SysAllocStringLen( 0, len );

	MultiByteToWideChar( 1252, 0, source, sourcelen, str, len );
	VariantInit( variant );

	V_VT( variant )   = VT_BSTR;
	V_BSTR( variant ) = str;

	return S_OK;
}

extern void CallDispatchMethod( IDispatch *dispatch, DISPPARAMS &params, OLECHAR *name );

struct APCWait
{
	IDispatch *dispatch;
	HANDLE     hEvent;
};

#define AutoAPC(name) \
	static VOID CALLBACK name ## APC(ULONG_PTR param) {\
	APCWait *wait = (APCWait *)param;\
\
	DISPPARAMS params;\
	params.cArgs             = 0;\
	params.cNamedArgs        = 0;\
	params.rgdispidNamedArgs = 0;\
	params.rgvarg            = 0;\
	if (wait->dispatch != NULL)\
		CallDispatchMethod(wait->dispatch, params, L ## #name);\
	if (wait->hEvent)\
		SetEvent(wait->hEvent);\
}

AutoAPC( OnFinish );
AutoAPC( OnTick );
AutoAPC( OnError );
AutoAPC( OnCancel );
AutoAPC( OnConnect );
AutoAPC( OnInit );

void JNetCOM::Call( PAPCFUNC func )
{
	DWORD curThreadId = GetCurrentThreadId();

	if ( curThreadId == threadId )
	{
		APCWait wait;
		wait.dispatch = dispatch;
		wait.hEvent   = 0;

		func( (ULONG_PTR)&wait );
	}
	else
	{
		if ( threadHandle )
		{
			APCWait wait;
			wait.dispatch = dispatch;
			wait.hEvent   = CreateEvent( NULL, FALSE, FALSE, NULL );

			if ( QueueUserAPC( func, threadHandle, (ULONG_PTR)&wait ) != 0 )
				WaitForSingleObject( wait.hEvent, INFINITE );

			CloseHandle( wait.hEvent );
		}
	}

}

void JNetCOM::OnFinish( DownloadToken token )
{
	if ( NULL != WAC_API_DOWNLOADMANAGER )
		WAC_API_DOWNLOADMANAGER->RetainDownload( token );

	retained = true;

	Call( OnFinishAPC );

	token = 0;

	Release();
}

void JNetCOM::OnTick( DownloadToken token )
{
	//Call(OnTickAPC);
}

void JNetCOM::OnError( DownloadToken token, int error )
{
	if ( NULL != WAC_API_DOWNLOADMANAGER )
		WAC_API_DOWNLOADMANAGER->RetainDownload( token );

	retained = true;

	Call( OnErrorAPC );

	token = 0;

	Release();
}

void JNetCOM::OnCancel( DownloadToken token )
{
	if ( NULL != WAC_API_DOWNLOADMANAGER )
		WAC_API_DOWNLOADMANAGER->RetainDownload( token );

	retained = true;

	Call( OnCancelAPC );

	token = 0;

	Release();
}

void JNetCOM::OnConnect( DownloadToken token )
{
	Call( OnConnectAPC );
}

void JNetCOM::OnInit( DownloadToken token )
{
	Call( OnInitAPC );
}

size_t JNetCOM::Dispatchable_AddRef()
{
	return InterlockedIncrement( &refCount );
}

size_t JNetCOM::Dispatchable_Release()
{
	LONG lRef = InterlockedDecrement( &refCount );
	if ( lRef == 0 )
		delete this;

	return lRef;
}


#define CBCLASS JNetCOM
START_DISPATCH;
CB( ADDREF,  Dispatchable_AddRef )
CB( RELEASE, Dispatchable_Release )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONFINISH,  OnFinish )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONTICK,    OnTick )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONERROR,   OnError )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL,  OnCancel )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT, OnConnect )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONINIT,    OnInit )
END_DISPATCH;
#undef CBCLASS
