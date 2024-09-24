#include "PlaylistsXML.h"
#include "api__playlist.h"
#include "../nu/AutoLock.h"
#include "Playlists.h"
#include <shlwapi.h>
#include <stdint.h>

using namespace Nullsoft::Utility;

void PlaylistsXML::StartTag( const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params )
{
	const wchar_t *filename    = params->getItemValue( L"filename" );
	const wchar_t *title       = params->getItemValue( L"title" );
	const wchar_t *countString = params->getItemValue( L"songs" );

	size_t numItems = 0;

	if ( countString && *countString )
		numItems = _wtoi( countString );

	const wchar_t *lengthString = params->getItemValue( L"seconds" );
	size_t length = 0;
	if ( lengthString && *lengthString )
		length = _wtoi( lengthString );

	const wchar_t *iTunesIDString = params->getItemValue( L"iTunesID" );
	uint64_t iTunesID = 0;
	if ( iTunesIDString && *iTunesIDString )
		iTunesID = _wtoi64( iTunesIDString );

	const wchar_t *cloudString = params->getItemValue( L"cloud" );
	size_t cloud = 0;
	if ( cloudString && *cloudString )
		cloud = _wtoi( cloudString );

	// parse GUID
	GUID guid = INVALID_GUID;
	const wchar_t *guidString = params->getItemValue( L"id" );
	if ( guidString && *guidString )
	{
		int Data1, Data2, Data3;
		int Data4[ 8 ];

		int n = swscanf( guidString, L" { %08x - %04x - %04x - %02x%02x - %02x%02x%02x%02x%02x%02x } ",
						 &Data1, &Data2, &Data3, Data4 + 0, Data4 + 1,
						 Data4 + 2, Data4 + 3, Data4 + 4, Data4 + 5, Data4 + 6, Data4 + 7 );

		if ( n == 11 ) // GUID was
		{
			// Cross assign all the values
			guid.Data1      = Data1;
			guid.Data2      = Data2;
			guid.Data3      = Data3;
			guid.Data4[ 0 ] = Data4[ 0 ];
			guid.Data4[ 1 ] = Data4[ 1 ];
			guid.Data4[ 2 ] = Data4[ 2 ];
			guid.Data4[ 3 ] = Data4[ 3 ];
			guid.Data4[ 4 ] = Data4[ 4 ];
			guid.Data4[ 5 ] = Data4[ 5 ];
			guid.Data4[ 6 ] = Data4[ 6 ];
			guid.Data4[ 7 ] = Data4[ 7 ];
		}
	}

	if ( PathIsFileSpecW( filename ) )
	{
		wchar_t playlistFilename[ MAX_PATH ] = { 0 };
		PathCombineW( playlistFilename, rootPath, filename );
		playlists->AddPlaylist_internal( playlistFilename, title, guid, numItems, length, iTunesID, cloud );
	}
	else
	{
		playlists->AddPlaylist_internal( filename, title, guid, numItems, length, iTunesID, cloud );
	}
}

int PlaylistsXML::LoadFile( const wchar_t *filename )
{
	if ( !parser )
		return PLAYLISTSXML_NO_PARSER; // no sense in continuing if there's no parser available

	HANDLE file = CreateFileW( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL );

	if ( file == INVALID_HANDLE_VALUE )
		return PLAYLISTSXML_NO_FILE;

	while ( true )
	{
		int8_t data[ 1024 ] = { 0 };
		DWORD bytesRead = 0;
		if ( ReadFile( file, data, 1024, &bytesRead, NULL ) && bytesRead )
		{
			if ( parser->xmlreader_feed( data, bytesRead ) != API_XML_SUCCESS )
			{
				CloseHandle( file );
				return PLAYLISTSXML_XML_PARSE_ERROR;
			}
		}
		else
			break;
	}

	CloseHandle( file );
	parser->xmlreader_feed( 0, 0 );

	return PLAYLISTSXML_SUCCESS;
}

PlaylistsXML::PlaylistsXML( Playlists *_playlists ) : playlists( _playlists )
{
	const wchar_t *g_path = WASABI_API_APP->path_getUserSettingsPath();
	PathCombineW( rootPath, g_path, L"plugins\\ml\\playlists" );

	playlists->AddRef();

	parserFactory = WASABI_API_SVC->service_getServiceByGuid( obj_xmlGUID );
	if ( parserFactory )
		parser = (obj_xml *)parserFactory->getInterface();

	if ( parser )
	{
		parser->xmlreader_setCaseSensitive();
		parser->xmlreader_registerCallback( L"Winamp:Playlists\fplaylist", this );
		parser->xmlreader_registerCallback( L"playlists\fplaylist", this );
		parser->xmlreader_open();
	}
}

PlaylistsXML::~PlaylistsXML()
{
	playlists->Release();
	if ( parser )
	{
		parser->xmlreader_unregisterCallback( this );
		parser->xmlreader_close();
	}

	if ( parserFactory && parser )
		parserFactory->releaseInterface( parser );

	parserFactory = 0;
	parser        = 0;
}

#define CBCLASS PlaylistsXML
START_DISPATCH;
VCB( ONSTARTELEMENT, StartTag )
END_DISPATCH;
#undef CBCLASS