#include <shlobj.h>
#include <strsafe.h>

#include "main.h"
#include "api__playlist.h"
#include "../nu/ns_wc.h"

HRESULT ResolveShortCut(HWND hwnd, LPCWSTR pszShortcutFile, LPWSTR pszPath)
{
	IShellLinkW      *psl = 0;
	WIN32_FIND_DATAW  wfd = {0};

	*pszPath = 0;   // assume failure

	HRESULT hres = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void **)&psl );
	if ( SUCCEEDED( hres ) )
	{
		IPersistFile *ppf = 0;
		hres = psl->QueryInterface( &ppf ); // OLE 2!  Yay! --YO
		if ( SUCCEEDED( hres ) )
		{
			hres = ppf->Load( pszShortcutFile, STGM_READ );
			if ( SUCCEEDED( hres ) )
			{
				hres = psl->Resolve( hwnd, SLR_ANY_MATCH );
				if ( SUCCEEDED( hres ) )
				{
					wchar_t szGotPath[ MAX_PATH ] = { 0 };
					StringCchCopyW( szGotPath, MAX_PATH, pszShortcutFile );
					hres = psl->GetPath( szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH );
					StringCchCopyW( pszPath, MAX_PATH, szGotPath );
				}
			}

			ppf->Release();
		}

		psl->Release();
	}

	return SUCCEEDED(hres);
}

bool IsUrl(const wchar_t *url)
{
	return !!wcsstr(url, L"://");
}

void SetUserAgent(api_httpreceiver *http)
{
	char agent[256] = {0};
	StringCchPrintfA(agent, 256, "User-Agent: %S/%S", WASABI_API_APP->main_getAppName(), WASABI_API_APP->main_getVersionNumString());
	http->addheader(agent);
}

const char *GetProxy()
{
	static char proxy[ 256 ] = "";
	// {C0A565DC-0CFE-405a-A27C-468B0C8A3A5C}
	const GUID internetConfigGroupGUID =
	{ 0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c } };
	ifc_configgroup *group = AGAVE_API_CONFIG->GetGroup( internetConfigGroupGUID );
	if ( group )
	{
		ifc_configitem *item = group->GetItem( L"Proxy" );
		if ( item )
		{
			const wchar_t *wideProxy = item->GetString();
			if ( wideProxy )
			{
				WideCharToMultiByteSZ( CP_ACP, 0, wideProxy, -1, proxy, 256, 0, 0 );
			}
		}
	}

	return proxy;
}