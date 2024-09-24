#ifndef NULLSOFT_WINAMP_OMBROWSER_OBJECT_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_OBJECT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "bfc/dispatch.h"

// {D5325EAB-9BD7-4382-A31D-38EF603061B3}
static const GUID OBJ_OmBrowser = 
{ 0xd5325eab, 0x9bd7, 0x4382, { 0xa3, 0x1d, 0x38, 0xef, 0x60, 0x30, 0x61, 0xb3 } };

class ifc_ombrowserclass;
class ifc_winamphook;
class ifc_ombrowserregistry;
class ifc_omservice;
class ifc_ombrowserevent;

#define BOSTYLE_NORMAL			0x0000
#define BOSTYLE_SHOWDEBUG		0x0001

#define BOCALLBACK_INIT			0
typedef void (CALLBACK *BROWSEROPTIONSCALLBACK)(HWND /*hDialog*/, UINT /*type*/, ULONG_PTR /*user*/);

class __declspec( novtable ) obj_ombrowser : public Dispatchable
{
protected:
	obj_ombrowser()                                                   {}
	~obj_ombrowser()                                                  {}

public:
	HRESULT Initialize( const wchar_t *pszName, HWND hwndWinamp );
	HRESULT Finish( void );
	HRESULT RegisterWinampHook( ifc_winamphook *hook, UINT *cookieOut );
	HRESULT UnregisterWinampHook( UINT cookie );
	HRESULT GetConfig( const GUID *configIfc, void **configOut );
	HRESULT GetSessionId( LPWSTR pszBuffer, INT cchBufferMax );
	HRESULT GetClientId( LPWSTR pszBuffer, INT cchBufferMax );
	HRESULT GetRegistry( ifc_ombrowserregistry **registryOut );
	HRESULT CreateView( ifc_omservice *service, HWND hParent, LPCWSTR forceUrl, UINT viewStyle, HWND *hView );
	HRESULT CreatePopup( ifc_omservice *service, INT x, INT y, INT cx, INT cy, HWND hOwner, LPCWSTR forceUrl, UINT viewStyle, HWND *hWindow );
	HRESULT IsFinishing( void );
	HRESULT GetClass( ifc_ombrowserclass **instance );
	HRESULT GetVersion( int *major, int *minor );
	HRESULT GetIEVersion( int *major, int *minor, int *build, int *subbuild );
	HRESULT ShowOptions( HWND hOwner, UINT style, BROWSEROPTIONSCALLBACK callback, ULONG_PTR user );

	DISPATCH_CODES
	{
		API_INITIALIZE           =  10,
		API_FINISH               =  20,
		API_REGISTERWINAMPHOOK   =  30,
		API_UNREGISTERWINAMPHOOK =  40,
		API_GETLANGMODULE        =  60,
		API_GETCONFIG            =  70,
		API_GETSESSIONID         =  80,
		API_GETCLIENTID          =  90,
		API_GETREGISTRY          = 100,
		API_CREATEVIEW           = 110,
		API_CREATEPOPUP          = 120,
		API_ISFINISHING          = 130,
		API_GETCLASS             = 140,
		API_GETVERSION           = 150,
		API_GETIEVERSION         = 160,
		API_SHOWOPTIONS          = 170,
	};
};

inline HRESULT obj_ombrowser::Initialize( const wchar_t *pszName, HWND hwndWinamp )
{
	return _call( API_INITIALIZE, (HRESULT)E_NOTIMPL, pszName, hwndWinamp );
}

inline HRESULT obj_ombrowser::Finish( void )
{
	return _call( API_FINISH, (HRESULT)E_NOTIMPL );
}

inline HRESULT obj_ombrowser::RegisterWinampHook( ifc_winamphook *hook, UINT *hookCookie )
{
	return _call( API_REGISTERWINAMPHOOK, (HRESULT)E_NOTIMPL, hook, hookCookie );
}

inline HRESULT obj_ombrowser::UnregisterWinampHook( UINT hookCookie )
{
	return _call( API_UNREGISTERWINAMPHOOK, (HRESULT)E_NOTIMPL, hookCookie );
}

inline HRESULT obj_ombrowser::GetConfig( const GUID *configIfc, void **configOut )
{
	return _call( API_GETCONFIG, (HRESULT)E_NOTIMPL, configIfc, configOut );
}

inline HRESULT obj_ombrowser::GetSessionId( LPWSTR pszBuffer, INT cchBufferMax )
{
	return _call( API_GETSESSIONID, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax );
}

inline HRESULT obj_ombrowser::GetClientId( LPWSTR pszBuffer, INT cchBufferMax )
{
	return _call( API_GETCLIENTID, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax );
}

inline HRESULT obj_ombrowser::GetRegistry( ifc_ombrowserregistry **registryOut )
{
	return _call( API_GETREGISTRY, (HRESULT)E_NOTIMPL, registryOut );
}

inline HRESULT obj_ombrowser::CreateView( ifc_omservice *service, HWND hParent, LPCWSTR forceUrl, UINT viewStyle, HWND *hView )
{
	return _call( API_CREATEVIEW, (HRESULT)E_NOTIMPL, service, hParent, forceUrl, viewStyle, hView );
}

inline HRESULT obj_ombrowser::CreatePopup( ifc_omservice *service, INT x, INT y, INT cx, INT cy, HWND hOwner, LPCWSTR forceUrl, UINT viewStyle, HWND *hWindow )
{
	return _call( API_CREATEPOPUP, (HRESULT)E_NOTIMPL, service, x, y, cx, cy, hOwner, forceUrl, viewStyle, hWindow );
}

inline HRESULT obj_ombrowser::IsFinishing( void )
{
	return _call( API_ISFINISHING, (HRESULT)E_NOTIMPL );
}

inline HRESULT obj_ombrowser::GetClass( ifc_ombrowserclass **instance )
{
	return _call( API_GETCLASS, (HRESULT)E_NOTIMPL, instance );
}

inline HRESULT obj_ombrowser::GetVersion( int *major, int *minor )
{
	return _call( API_GETVERSION, (HRESULT)E_NOTIMPL, major, minor );
}

inline HRESULT obj_ombrowser::GetIEVersion( int *major, int *minor, int *build, int *subbuild )
{
	return _call( API_GETIEVERSION, (HRESULT)E_NOTIMPL, major, minor, build, subbuild );
}

inline HRESULT obj_ombrowser::ShowOptions( HWND hOwner, UINT style, BROWSEROPTIONSCALLBACK callback, ULONG_PTR user )
{
	return _call( API_SHOWOPTIONS, (HRESULT)E_NOTIMPL, hOwner, style, callback, user );
}

#endif //NULLSOFT_WINAMP_OMBROWSER_OBJECT_INTERFACE_HEADER