#ifndef NULLSOFT_PLAYLIST_M3U_LOADER_H
#define NULLSOFT_PLAYLIST_M3U_LOADER_H

#include "ifc_playlistloader.h"
#include "ifc_playlistloadercallback.h"
#include <stdio.h>

static wchar_t *_INFO_NAME_MEDIA_HASH    = L"mediahash";
static wchar_t *_INFO_NAME_META_HASH     = L"metahash";
static wchar_t *_INFO_NAME_CLOUD_ID      = L"cloud_id";
static wchar_t *_INFO_NAME_CLOUD_STATUS  = L"cloud_status";
static wchar_t *_INFO_NAME_CLOUD_DEVICES = L"cloud_devices";

class M3ULoader : public ifc_playlistloader
{
public:
	M3ULoader();
	virtual ~M3ULoader( void );

	int Load( const wchar_t *filename, ifc_playlistloadercallback *playlist );
	int OnFileHelper( ifc_playlistloadercallback *playlist, const wchar_t *trackName, const wchar_t *title, int length, const wchar_t *rootPath, ifc_plentryinfo *extraInfo );


protected:
	RECVS_DISPATCH;

	bool    _utf8;
	wchar_t wideFilename[ FILENAME_SIZE ];
	wchar_t wideTitle[ FILETITLE_SIZE ];
};
#endif