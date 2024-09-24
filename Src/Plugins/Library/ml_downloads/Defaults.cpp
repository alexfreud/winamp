#include "main.h"
#include "Defaults.h"
#include <shlobj.h>
wchar_t defaultDownloadPath[MAX_PATH] = {0};
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

static void BuildDefaultDownloadPath( HWND hwnd )
{
	wchar_t defaultPath[ MAX_PATH ] = L"";
	if ( !UtilGetSpecialFolderPath( hwnd, defaultPath, CSIDL_MYMUSIC ) )
		UtilGetSpecialFolderPath( hwnd, defaultPath, CSIDL_PERSONAL );

	lstrcpyn( defaultDownloadPath, defaultPath, MAX_PATH );
}

/*
Requires an HWND because some of the shell functions require one.  Theoretically it could be NULL
*/
void BuildDefaults( HWND hwnd )
{
	BuildDefaultDownloadPath( hwnd );
}

