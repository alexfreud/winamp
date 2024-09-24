#include "main.h"
#include "WMPlaylist.h"

WMPlaylist activePlaylist;

void WMPlaylist::OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info )
{
	//if (playstring.empty())
	if ( playstring )
		free( playstring );

	playstring = _wcsdup( filename );
}

const wchar_t *WMPlaylist::GetFileName()
{
	return ( playstring ? playstring : L"" );
}

const wchar_t *WMPlaylist::GetOriginalFileName()
{
	return ( playlistFilename ? playlistFilename : L"" );
}

bool WMPlaylist::IsMe( const char *filename )
{
	return IsMe( (const wchar_t *)AutoWide( filename ) );
}

bool WMPlaylist::IsMe( const wchar_t *filename )
{
	if ( playlistFilename && !_wcsicmp( playlistFilename, filename ) )
		return true;

	if ( playstring && !_wcsicmp( playstring, filename ) )
		return true;

	return false;
}

#define CBCLASS WMPlaylist
START_DISPATCH;
VCB( IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile )
END_DISPATCH;
#undef CBCLASS