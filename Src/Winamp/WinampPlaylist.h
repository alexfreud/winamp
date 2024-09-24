#ifndef NULLSOFT_WINAMP_WINAMP_PLAYLIST_H
#define NULLSOFT_WINAMP_WINAMP_PLAYLIST_H

// this class is for the main playlist editor
#include "../playlist/ifc_playlistloadercallback.h"
#include "../playlist/ifc_playlistdirectorycallback.h"

class WinampPlaylist : public ifc_playlistloadercallback
{
public:
	WinampPlaylist( const wchar_t *_base = 0, bool _loadDirectories = false );
	~WinampPlaylist();

	void OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info );
	const wchar_t *GetBasePath();

protected:
	RECVS_DISPATCH;

	const wchar_t *base;
	bool terminateOnEnd;
	bool loadDirectories;
};

class WinampDirectoryLoad : public ifc_playlistdirectorycallback
{
public:
	WinampDirectoryLoad( bool _forceRecurse, char *exts = 0 );
	~WinampDirectoryLoad();

	bool ShouldRecurse( const wchar_t *path );
	bool ShouldLoad( const wchar_t *filename );

	bool forceRecurse;
	char *exts;
	bool needsFree;
	int listStart;

protected:
	RECVS_DISPATCH;
};

#endif