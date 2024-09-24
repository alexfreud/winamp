#include "main.h"
#include "WinampPlaylist.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "api.h"

WinampDirectoryLoad::WinampDirectoryLoad( bool _forceRecurse, char *_exts ) : forceRecurse( _forceRecurse ), exts( _exts ), needsFree( false ), listStart( 0 )
{
	listStart = PlayList_getlength();
	if ( !exts )
	{
		exts = in_getextlist();
		needsFree = false;
	}
}

WinampDirectoryLoad::~WinampDirectoryLoad()
{
	if ( config_rofiob & 1 )
		PlayList_sort( 2, listStart );

	if ( needsFree )
		GlobalFree( (HGLOBAL)exts );
}

WinampPlaylist::WinampPlaylist( const wchar_t *_base, bool _loadDirectories ) : base( _base ), terminateOnEnd( false ), loadDirectories( _loadDirectories )
{}

bool WhitelistOK( wchar_t *url )
{
	bool validurl = false;
	wchar_t *p = url;
	// Whitelist the url...
	if ( *p == L'a' ) p++; // get passed the ad-fullscreen blob.
	if ( *p )
	{
		wchar_t *whitelist;
		whitelist = wcsstr( p, L"http://www.winamp.com/" );
		if ( whitelist && whitelist == p )
		{
			validurl = true;
		}

		if ( validurl == false )	// Check for javscript command
		{
			whitelist = wcsstr( p, L"javascript:onCommand(" );
			if ( whitelist && whitelist == p )
			{
				wchar_t *semi = wcsstr( whitelist, L";" );
				if ( semi )
				{
					semi++;
					if ( *semi )
						*semi = 0;
				}

				validurl = true;
			}

		}
	}

	return validurl;
}

WinampPlaylist::~WinampPlaylist()
{
	//if ( terminateOnEnd )
	//{
	//}
}

void WinampPlaylist::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMs, ifc_plentryinfo *info)
{
	BOOL hidden = false;
	int playcount = 0;
	unsigned long starttime = 0, endtime = 0;
	wchar_t fcurtain[FILENAME_SIZE] = L"";
	wchar_t fileExt[ 10 ] = L"";
	//wchar_t fbrowser[FILENAME_SIZE] = L"";

	if (LoadPlaylist(filename, 1, 0) == 0) //(playlistManager->Load(filename, this) == PLAYLISTMANAGER_SUCCESS) // if it's another playlist file, load recursively
		return ;

	// check for trailing backslash
	/*
	if (filename[lstrlenW(filename)] == '\\')
	{
					WinampDirectoryLoad dir(true);
				playlistManager->LoadDirectory(filename, this, &dir);
				return;
	}
	*/

	// see if it's a directory
	if ( loadDirectories && !PathIsURLW( filename ) && !PathIsNetworkPathW( filename ) )
	{
		HANDLE h;
		WIN32_FIND_DATAW d;

		h = FindFirstFileW( filename, &d );
		if ( h != INVALID_HANDLE_VALUE )
		{
			FindClose( h );
			if ( d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				WinampDirectoryLoad dir( true );
				playlistManager->LoadDirectory( filename, this, &dir );
				return;
			}
		}
	}

	if (info)
	{
		const wchar_t *ext = 0;
		ext = info->GetExtendedInfo(L"Repeat");
		if (ext) playcount = _wtoi(ext);

		ext = info->GetExtendedInfo(L"Start");
		if (ext) starttime = _wtoi(ext);

		ext = info->GetExtendedInfo(L"End");
		if (ext) endtime = _wtoi(ext);

		ext = info->GetExtendedInfo(L"Hidden");
		if (ext)
			hidden = _wtoi(ext);
		else
		{
			ext = info->GetExtendedInfo(L"Context");
			if (ext)
			{
				if (!lstrcmpiW(ext, L"radio"))
				{
					hidden = true;
					terminateOnEnd = true;
				}
			}
		}

		ext = info->GetExtendedInfo(L"Curtain");
		if (ext)
		{
			StringCchCopyW(fcurtain, FILENAME_SIZE, ext);
			if (!WhitelistOK(fcurtain))
				fcurtain[0] = 0;
		}

		ext = info->GetExtendedInfo( L"ext" );
		if ( ext )
		{
			StringCchCopyW( fileExt, 10, ext );
		}
		/*
		ext = info->GetExtendedInfo(L"Browser");
		if (ext)
		{
			StringCchCopyW(fbrowser, FILENAME_SIZE, ext);
			if (!WhitelistOK(fbrowser))
				fbrowser[0] = 0;
		}
		*/
	}

	if ( lengthInMs != -1 )
		lengthInMs /= 1000;

	if ( !hidden )
	{
		PlayList_append_withinfo_curtain( filename, title, lengthInMs, fcurtain[ 0 ] ? AutoChar( fcurtain ) : 0, fileExt, 0 );
	}
	else
	{
		PlayList_append_withinfo_hidden( filename, title, lengthInMs, fcurtain[ 0 ] ? AutoChar( fcurtain ) : 0/*,		                                fbrowser[0] ? AutoChar(fbrowser) : 0*/ );
	}

	PlayList_SetLastItem_RepeatCount(playcount);
	PlayList_SetLastItem_Range(starttime, endtime);
}

const wchar_t *WinampPlaylist::GetBasePath()
{
	return base;
}

bool WinampDirectoryLoad::ShouldRecurse(const wchar_t *path)
{
	if (config_rofiob&1)
		PlayList_sort(2, listStart);

	listStart = PlayList_getlength();

	if (forceRecurse)
		return true;
	else
		return (config_rofiob&2) ? false : true;
}

bool WinampDirectoryLoad::ShouldLoad(const wchar_t *filename)
{
	if (!exts)
		return true;
	const wchar_t *ext = extensionW(filename);
	char *a = exts;
	while (a && *a)
	{
		if (!_wcsicmp(AutoWide(a), ext))
			return true;
		a += lstrlenA(a) + 1;
	}
	return false;

}

#define CBCLASS WinampPlaylist
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
CB(IFC_PLAYLISTLOADERCALLBACK_GETBASEPATH, GetBasePath)
END_DISPATCH;

#undef CBCLASS
#define CBCLASS WinampDirectoryLoad
START_DISPATCH;
CB(IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDRECURSE, ShouldRecurse)
CB(IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDLOAD, ShouldLoad)
END_DISPATCH;
#undef CBCLASS
