#include "main.h"
#include "Defaults.h"
#include <shlobj.h>
wchar_t defaultDownloadPath[MAX_PATH] = {0},
		serviceUrl[1024] = {0};
__time64_t updateTime = 60 /* 1 minute */ * 60 /* 1 hour */ * 24 /* 1 day */;
int autoDownloadEpisodes = 1;
bool autoUpdate = true;
bool autoDownload = true;
bool updateOnLaunch = false;
bool needToMakePodcastsView=true;

static BOOL UtilGetSpecialFolderPath( HWND hwnd, wchar_t *path, int folder )
{
	ITEMIDLIST *pidl; // Shell Item ID List ptr
	IMalloc *imalloc; // Shell IMalloc interface ptr
	BOOL result; // Return value

	if ( SHGetSpecialFolderLocation( hwnd, folder, &pidl ) != NOERROR )
		return FALSE;

	result = SHGetPathFromIDList( pidl, path );

	if ( SHGetMalloc( &imalloc ) == NOERROR )
	{
		imalloc->Free( pidl );
		imalloc->Release();
	}

	return result;
}

void BuildDefaultDownloadPath( HWND hwnd )
{
	wchar_t defaultPath[ MAX_PATH ] = L"";
	if ( !UtilGetSpecialFolderPath( hwnd, defaultPath, CSIDL_MYMUSIC ) )
		UtilGetSpecialFolderPath( hwnd, defaultPath, CSIDL_PERSONAL );

	lstrcpyn( defaultDownloadPath, defaultPath, MAX_PATH );
}