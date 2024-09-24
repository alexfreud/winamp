#ifndef NULLSOFT_IN_WMVDRM_WMPLAYLIST_H
#define NULLSOFT_IN_WMVDRM_WMPLAYLIST_H

#include "../playlist/ifc_playlistloadercallback.h"

class WMPlaylist : public ifc_playlistloadercallback
{
public:
	WMPlaylist()                                                      {}

	~WMPlaylist()
	{
		if ( playstring )
			free( playstring );

		if ( playlistFilename )
			free( playlistFilename );
	}

	void Clear()
	{
		if ( playstring )
			free( playstring );

		playstring = 0;

		if ( playlistFilename )
			free( playlistFilename );

		playlistFilename = 0;
	}

	void OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info );

	const wchar_t *GetFileName();
	const wchar_t *GetOriginalFileName();
	/* TODO: need something like these, just not sure exact what yet
	bool ForceStartTime(int &);
	bool ForceLength(int &);
	bool ForceNoSeek();
	*/
	bool IsMe( const char *filename );
	bool IsMe( const wchar_t *filename );


protected:
	RECVS_DISPATCH;

public:
	wchar_t *playstring       = 0;
	wchar_t *playlistFilename = 0;
};

extern WMPlaylist activePlaylist;

#endif  // !NULLSOFT_IN_WMVDRM_WMPLAYLIST_H