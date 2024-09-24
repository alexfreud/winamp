#include "main.h"
#include "./service.h"
#include "api__ml_wire.h"
#include "./util.h"
#include "./resource.h"
#include "./externalCOM.h"

#include "../winamp/wa_ipc.h"
#include <strsafe.h>

#define IS_INVALIDISPATCH(__disp) (((IDispatch *)1) == (__disp) || NULL == (__disp))

OmService::OmService( UINT nId ) : id( nId )
{}

OmService::~OmService()
{
	Plugin_FreeResString( name );
	Plugin_FreeResString( url );
	Plugin_FreeResString( icon );
}


HRESULT OmService::CreateRemote( UINT nId, LPCWSTR pszName, LPCWSTR pszIcon, LPCWSTR pszUrl, OmService **instance )
{
	if ( instance == NULL )
		return E_POINTER;

	*instance = NULL;
	
	if ( nId == 0 || pszName == NULL )
		return E_INVALIDARG;

	OmService *service = new OmService( nId );
	if ( service == NULL )
		return E_OUTOFMEMORY;

	service->SetName( pszName );
	service->SetIcon( pszIcon );
	service->SetUrl( pszUrl );
    	
	*instance = service;

	return S_OK;
}

HRESULT OmService::CreateLocal( UINT nId, LPCWSTR pszName, LPCWSTR pszIcon, SVCWNDCREATEPROC windowCreator, OmService **instance )
{
	if ( instance == NULL )
		return E_POINTER;

	*instance = NULL;
	
	if ( nId == 0 || pszName == NULL )
		return E_INVALIDARG;

	OmService *service = new OmService( nId );
	if ( service == NULL )
		return E_OUTOFMEMORY;

	service->SetFlags( flagLocal, flagLocal );
	service->SetName( pszName );
	service->SetIcon( pszIcon );
	service->SetWindowCreator( windowCreator );
    	
	*instance = service;

	return S_OK;
}


size_t OmService::AddRef()
{
	return _ref.fetch_add( 1 );
}

size_t OmService::Release()
{
	if ( _ref.load() == 0 )
		return _ref.load();
	
	LONG r = _ref.fetch_sub( 1 );
	if ( r == 0 )
		delete( this );
	
	return r;
}


int OmService::QueryInterface( GUID interface_guid, void **object )
{
	if ( object == NULL )
		return E_POINTER;

	if ( IsEqualIID( interface_guid, IFC_OmService ) )
		*object = static_cast<ifc_omservice *>( this );
	else
	{
		*object = NULL;

		return E_NOINTERFACE;
	}

	if ( *object == NULL )
		return E_UNEXPECTED;

	AddRef();
	
	return S_OK;
}


unsigned int OmService::GetId()
{
	return id;
}

HRESULT OmService::GetName( wchar_t *pszBuffer, int cchBufferMax )
{
	return Plugin_CopyResString( pszBuffer, cchBufferMax, name );
}

HRESULT OmService::GetUrl( wchar_t *pszBuffer, int cchBufferMax )
{
	return Plugin_CopyResString( pszBuffer, cchBufferMax, url );
}

HRESULT OmService::GetIcon( wchar_t *pszBuffer, int cchBufferMax )
{
	if ( icon != NULL && IS_INTRESOURCE( icon ) )
	{
		WCHAR szPath[ 2 * MAX_PATH ] = { 0 };
		if ( GetModuleFileName( plugin.hDllInstance, szPath, ARRAYSIZE( szPath ) ) == 0 )
			return E_FAIL;

		return StringCchPrintf( pszBuffer, cchBufferMax, L"res://%s/#%d/#%d", szPath, RT_RCDATA, icon );
	}

	return StringCchCopyEx( pszBuffer, cchBufferMax, icon, NULL, NULL, STRSAFE_IGNORE_NULLS );
}

HRESULT OmService::GetExternal( IDispatch **ppDispatch )
{
	if ( ppDispatch == NULL )
		return E_POINTER;
	
	*ppDispatch = NULL;

	HWND hWinamp = plugin.hwndWinampParent;
	if ( hWinamp == NULL )
		return E_UNEXPECTED;
		
	//////*ppDispatch = (IDispatch*)SENDWAIPC(hWinamp, IPC_GET_DISPATCH_OBJECT, 0);

	WCHAR szBuffer[ 64 ] = { 0 };
	if ( SUCCEEDED( StringCchPrintfW( szBuffer, ARRAYSIZE( szBuffer ), L"%u", id ) ) )
		*ppDispatch = (IDispatch *) SENDWAIPC( hWinamp, IPC_JSAPI2_GET_DISPATCH_OBJECT, (WPARAM) szBuffer );


	if (IS_INVALIDISPATCH(*ppDispatch) && FAILED(ExternalCOM::CreateInstance((ExternalCOM**)ppDispatch)))
	{
		*ppDispatch = NULL;
		return E_FAIL;
	}
	

	return S_OK;
}


HRESULT OmService::SetName( LPCWSTR pszName )
{
	Plugin_FreeResString( name );
	name = Plugin_DuplicateResString( pszName );

	return S_OK;
}

HRESULT OmService::SetUrl( LPCWSTR pszUrl )
{
	Plugin_FreeResString( url );
	url = Plugin_DuplicateResString( pszUrl );

	return S_OK;
}

HRESULT OmService::SetIcon( LPCWSTR pszIcon )
{
	Plugin_FreeResString( icon );
	icon = Plugin_DuplicateResString( pszIcon );

	return S_OK;
}

void OmService::SetFlags( UINT mask, UINT newFlags )
{
	flags = ( flags & ~mask ) | ( mask & newFlags );
}

UINT OmService::GetFlags( void )
{
	return flags;
}


HRESULT OmService::SetWindowCreator( SVCWNDCREATEPROC proc )
{
	windowCreator = proc;

	return S_OK;
}

HRESULT OmService::GetWindowCreator( SVCWNDCREATEPROC *proc )
{
	if ( proc == NULL )
		return E_INVALIDARG;

	*proc = windowCreator;

	return S_OK;
}


HRESULT OmService::CreateView( HWND hParent, HWND *hView )
{
	if ( hView == NULL )
		return E_POINTER;

	*hView = NULL;
	HRESULT hr = S_OK;

	if ( ( flagLocal & flags ) != 0 )
	{
		if ( windowCreator != NULL )
		{
			*hView = windowCreator( hParent, this );
			if ( *hView == NULL )
				hr = E_FAIL;
		}
		else
			hr = E_INVALIDARG;
	}
	else
	{
		if ( OMBROWSERMNGR != NULL )
		{
			hr = OMBROWSERMNGR->Initialize( NULL, plugin.hwndWinampParent );
			if ( SUCCEEDED( hr ) )
				hr = OMBROWSERMNGR->CreateView( this, hParent, NULL, 0, hView );
		}
		else
			hr = E_UNEXPECTED;

	}

	return hr;
}


#define CBCLASS OmService
START_DISPATCH;
CB( ADDREF,          AddRef )
CB( RELEASE,         Release )
CB( QUERYINTERFACE,  QueryInterface )
CB( API_GETID,       GetId )
CB( API_GETNAME,     GetName )
CB( API_GETURL,      GetUrl )
CB( API_GETICON,     GetIcon )
CB( API_GETEXTERNAL, GetExternal )
END_DISPATCH;
#undef CBCLASS

	