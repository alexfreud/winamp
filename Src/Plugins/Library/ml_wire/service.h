#ifndef NULLSOFT_PODCAST_PLUGIN_SERVICE_HEADER
#define NULLSOFT_PODCAST_PLUGIN_SERVICE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <atomic>

#include "../omBrowser/ifc_omservice.h"

class OmService;
typedef HWND (CALLBACK *SVCWNDCREATEPROC)(HWND /*hParent*/, OmService* /*service*/);

class OmService : public ifc_omservice
{
public:
	typedef enum
	{
		flagRoot  = 0x00000001,
		flagLocal = 0x00000002,
	} Flags;

protected:
	OmService( UINT nId );
	~OmService();

public:
	static HRESULT CreateRemote( UINT nId, LPCWSTR pszName, LPCWSTR pszIcon, LPCWSTR pszUrl, OmService **instance );
	static HRESULT CreateLocal( UINT nId, LPCWSTR pszName, LPCWSTR pszIcon, SVCWNDCREATEPROC windowCreator, OmService **instance );

	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int    QueryInterface( GUID interface_guid, void **object );

	/* ifc_omservice */
	unsigned int GetId();
	HRESULT GetName( wchar_t *pszBuffer, int cchBufferMax );
	HRESULT GetUrl( wchar_t *pszBuffer, int cchBufferMax );
	HRESULT GetExternal( IDispatch **ppDispatch );
	HRESULT GetIcon( wchar_t *pszBuffer, int cchBufferMax );

	HRESULT SetName( LPCWSTR pszName );
	HRESULT SetUrl( LPCWSTR pszUrl );
	HRESULT SetIcon( LPCWSTR pszIcon );

	void    SetFlags( UINT mask, UINT newFlags );
	UINT    GetFlags( void );

	HRESULT SetWindowCreator( SVCWNDCREATEPROC proc );
	HRESULT GetWindowCreator( SVCWNDCREATEPROC *proc );

	HRESULT CreateView( HWND hParent, HWND *hView );

protected:
	RECVS_DISPATCH;

	std::atomic<std::size_t> _ref           = 1;
	UINT                     id             = 0;
	LPWSTR                   name           = NULL;
	LPWSTR                   url            = NULL;
	SVCWNDCREATEPROC         windowCreator  = NULL;
	LPWSTR                   icon           = NULL;
	UINT                     flags          = 0;
};

#endif //NULLSOFT_PODCAST_PLUGIN_SERVICE_HEADER
