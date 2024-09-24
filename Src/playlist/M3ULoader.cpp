#include <stdio.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <fstream>
#include <string>

#include "M3ULoader.h"
#include "../nu/ns_wc.h"

#include "../WAT/WAT.h"


M3ULoader::M3ULoader() : _utf8( false )
{
	wideTitle[ 0 ] = wideFilename[ 0 ] = 0;
}

M3ULoader::~M3ULoader( void )
{
	//Close();
}

struct cmpWchar_t {
	bool operator()(const wchar_t* a, const wchar_t* b) const {
		return wcscmp(a, b) < 0;
	}
};

class M3UInfo : public ifc_plentryinfo
{
public:
	M3UInfo()                                                         {}
	M3UInfo( wchar_t *_mediahash, wchar_t *_metahash, wchar_t *_cloud_id, wchar_t *_cloud_status, wchar_t *_cloud_devices )
	{
		_extended_infos.emplace( _wcsdup( _INFO_NAME_MEDIA_HASH ),    _wcsdup( _mediahash) );
		_extended_infos.emplace( _wcsdup( _INFO_NAME_META_HASH ),     _wcsdup( _metahash ) );

		_extended_infos.emplace( _wcsdup( _INFO_NAME_CLOUD_ID ),      _wcsdup( _cloud_id ) );
		_extended_infos.emplace( _wcsdup( _INFO_NAME_CLOUD_STATUS ),  _wcsdup( _cloud_status ) );
		_extended_infos.emplace( _wcsdup( _INFO_NAME_CLOUD_DEVICES ), _wcsdup( _cloud_devices ) );
	}

	~M3UInfo()
	{
		for ( auto l_extended_infos_iterator = _extended_infos.begin(); l_extended_infos_iterator != _extended_infos.end(); ++l_extended_infos_iterator )
		{
			free( ( *l_extended_infos_iterator ).first );
			free( ( *l_extended_infos_iterator ).second );
		}

		_extended_infos.clear();
	}

	void SetExtendedInfo( const wchar_t *p_parameter_name, const wchar_t *p_parameter_value )
	{
		_extended_infos.emplace( _wcsdup( p_parameter_name ), _wcsdup( p_parameter_value ) );
	}

	const wchar_t *GetExtendedInfo( wchar_t *parameter )
	{
		//for ( auto l_extended_infos_iterator = _extended_infos.begin(); l_extended_infos_iterator != _extended_infos.end(); ++l_extended_infos_iterator )
		//{
		//	wchar_t *l_key = _wcsdup( ( *l_extended_infos_iterator ).first );
		//	if ( wcscmp( l_key, parameter ) == 0 )
		//		return _wcsdup( ( *l_extended_infos_iterator ).second );
		//}
		
		// OLD
		//std::map<wchar_t *, wchar_t *>::iterator l_extended_infos_iterator = _extended_infos.find( parameter );

		//if ( l_extended_infos_iterator != _extended_infos.end() )
		//	return _wcsdup( ( *l_extended_infos_iterator ).second );

		auto it = _extended_infos.find(parameter);
		if (_extended_infos.end() != it)
		{
			return it->second;
		}

		return 0;
	}

private:
	RECVS_DISPATCH;

	std::map<wchar_t *, wchar_t *, cmpWchar_t> _extended_infos;
};

#define CBCLASS M3UInfo
START_DISPATCH;
CB( IFC_PLENTRYINFO_GETEXTENDEDINFO, GetExtendedInfo )
END_DISPATCH;
#undef CBCLASS


int M3ULoader::OnFileHelper( ifc_playlistloadercallback *playlist, const wchar_t *trackName, const wchar_t *title, int length, const wchar_t *rootPath, ifc_plentryinfo *extraInfo )
{
	if ( length == -1000 )
		length = -1;

	wcsncpy( wideFilename, trackName, FILENAME_SIZE );
	
	int ret;

	if ( wcsstr( wideFilename, L"://" ) || PathIsRootW( wideFilename ) )
	{
		ret = playlist->OnFile( wideFilename, title, length, extraInfo );
	}
	else
	{
		wchar_t fullPath[ MAX_PATH ] = { 0 };
		if ( PathCombineW( fullPath, rootPath, wideFilename ) )
		{
			wchar_t canonicalizedPath[ MAX_PATH ] = { 0 };
			PathCanonicalizeW( canonicalizedPath, fullPath );
			ret = playlist->OnFile( canonicalizedPath, title, length, extraInfo );
		}
		else
		{
			ret = ifc_playlistloadercallback::LOAD_CONTINUE;
		}
	}

	return ret;
}

static bool StringEnds( const wchar_t *a, const wchar_t *b )
{
	size_t aLen = wcslen( a );
	size_t bLen = wcslen( b );

	if ( aLen < bLen )
		return false;  // too short

	if ( !_wcsicmp( a + aLen - bLen, b ) )
		return true;

	return false;
}

int M3ULoader::Load( const wchar_t *p_filename, ifc_playlistloadercallback *playlist )
{
	// TODO: download temp file if it's a URL
	// TODO - WDP2-198

	FILE *fp = _wfopen( p_filename, L"rt,ccs=UNICODE" );
	if ( !fp )
		return IFC_PLAYLISTLOADER_FAILED;

	fseek( fp, 0, SEEK_END );
	int size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	if ( size == -1 )
	{
		fclose( fp );
		fp = 0;

		return IFC_PLAYLISTLOADER_FAILED;
	}

	if ( StringEnds( p_filename, L".m3u8" ) )
		_utf8 = true;

	int ext = 0;

	wchar_t *p;

	const int l_linebuf_size = 2048;
	wchar_t linebuf[ l_linebuf_size ] = { 0 };

	wchar_t ext_title[ MAX_PATH ]    = { 0 };

	wchar_t ext_mediahash[ 128 ]     = { 0 };
	wchar_t ext_metahash[ 128 ]      = { 0 };

	wchar_t ext_cloud_id[ 128 ]      = { 0 };
	wchar_t ext_cloud_status[ 16 ]   = { 0 };
	wchar_t ext_cloud_devices[ 128 ] = { 0 };

	int ext_len = -1;

	wchar_t rootPath[ MAX_PATH ] = { 0 };
	const wchar_t *callbackPath = playlist->GetBasePath();
	if ( callbackPath )
		StringCchCopyW( rootPath, MAX_PATH, callbackPath );
	else
	{
		StringCchCopyW( rootPath, MAX_PATH, p_filename );
		PathRemoveFileSpecW( rootPath );
	}

	unsigned char BOM[ 3 ] = { 0, 0, 0 };
	if ( fread( BOM, 3, 1, fp ) == 1 && BOM[ 0 ] == 0xEF && BOM[ 1 ] == 0xBB && BOM[ 2 ] == 0xBF )
		_utf8 = true;
	else
		fseek( fp, 0, SEEK_SET );


	std::wstring l_separator     = L"\" ";
	std::wstring l_key_separator = L"=";


	const wchar_t _ASF[]                      = L"ASF ";
	const wchar_t _DIRECTIVE_EXTINF[]         = L"#EXTINF:";
	const wchar_t _DIRECTIVE_EXTM3U[]         = L"#EXTM3U";
	const wchar_t _DIRECTIVE_EXT_X_NS_CLOUD[] = L"#EXT-X-NS-CLOUD:";
	const wchar_t _DIRECTIVE_UTF8[]           = L"#UTF8";
	const wchar_t _END_LINE[]                 = L"\r\n";

	const int l_move_size = sizeof( wchar_t );


	wa::strings::wa_string l_key_value_pair = "";
	wa::strings::wa_string l_key            = "";
	wa::strings::wa_string l_value          = "";

	std::map<std::wstring, std::wstring> l_extended_infos;

	while ( 1 )
	{
		if ( feof( fp ) )
			break;

		linebuf[ 0 ] = 0;
		fgetws( linebuf, l_linebuf_size - 1, fp );

		linebuf[ wcscspn( linebuf, _END_LINE ) ] = 0;
		if ( wcslen( linebuf ) == 0 )
			continue;

 		if ( ext == 0 && wcsstr( linebuf, _DIRECTIVE_EXTM3U ) )
		{
			ext = 1;

			continue;
		}

		if ( !wcsncmp( linebuf, _DIRECTIVE_UTF8, 5 ) )
		{
			_utf8 = true;

			continue;
		}

		p = linebuf;

		while ( p && *p == ' ' || *p == '\t' )
			p = CharNextW( p );

		if ( *p != '#' && *p != '\n' && *p != '\r' && *p )
		{
			wchar_t buf[ 4096 ] = { 0 };

			wchar_t *p2 = CharPrevW( linebuf, linebuf + wcslen( linebuf ) ); //GetLastCharacter(linebuf);
			if ( p2 && *p2 == '\n' )
				*p2 = 0;

			if ( !wcsncmp( p, _ASF, 4 ) && wcslen( p ) > 4 )
				p += 4;

			if ( wcsncmp( p, L"\\\\", 2 ) && wcsncmp( p + 1, L":\\", 2 ) && wcsncmp( p + 1, L":/", 2 ) && !wcsstr( p, L"://" ) )
			{
				if ( p[ 0 ] == '\\' )
				{
					buf[ 0 ] = rootPath[ 0 ];
					buf[ 1 ] = rootPath[ 1 ];

					StringCchCopyW( buf + 2, 4093, p );

					//buf[ wcslen( buf ) - 1 ] = 0;
					buf[ wcscspn( buf, _END_LINE ) ] = 0;
					p = buf;
				}
			}

			int ret;

			// generate extra info from the cloud specific values (if present)
			M3UInfo info( ext_mediahash, ext_metahash, ext_cloud_id, ext_cloud_status, ext_cloud_devices );


			if ( !l_extended_infos.empty() )
			{
				for ( auto l_extended_infos_iterator = l_extended_infos.begin(); l_extended_infos_iterator != l_extended_infos.end(); ++l_extended_infos_iterator )
				{
					info.SetExtendedInfo( ( *l_extended_infos_iterator ).first.c_str(), ( *l_extended_infos_iterator ).second.c_str() );
				}
			}

			l_extended_infos.clear();

			if ( ext_title[ 0 ] )
			{
				wcsncpy( wideTitle, ext_title, FILETITLE_SIZE );
				ret = OnFileHelper( playlist, p, wideTitle, ext_len * 1000, rootPath, &info );
			}
			else
			{
				ret = OnFileHelper( playlist, p, 0, -1, rootPath, &info );
			}

			if ( ret != ifc_playlistloadercallback::LOAD_CONTINUE )
				break;

			ext_len        = -1;
			ext_title[ 0 ] = 0;
		}
		else
		{
			if ( ext && !wcsncmp( p, _DIRECTIVE_EXTINF, 8 ) )
			{
				p += 8;
				ext_len = _wtoi( p );

				int l_track_length = ext_len;
				int l_digits = ( l_track_length < 0 ? 1 : 0 );
				while ( l_track_length )
				{
					l_track_length /= 10;
					++l_digits;
				}

				p += l_digits;


				if ( p && *p )
				{
					wchar_t *p2 = CharPrevW( p, p + wcslen( p ) ); // GetLastCharacter(p);
					if ( p2 && *p2 == '\n' )
						*p2 = 0;

					while ( p && *p == ' ' )
						p = CharNextW( p );

					std::wstring l_string( p );

					int l_pos = l_string.find_first_of( L"," );

					if ( l_pos > 0 )
					{
						int  l_key_separator_pos = 0;

						wa::strings::wa_string l_line_trail( l_string.substr( 0, l_pos ) );

						while ( !l_line_trail.empty() )
						{
							int l_separator_pos = l_line_trail.find( l_separator );

							if ( l_separator_pos > 0 )
								l_key_value_pair = l_line_trail.mid( 0, l_separator_pos + 1 );
							else
								l_key_value_pair = l_line_trail;


							l_key_separator_pos = l_key_value_pair.find( l_key_separator );

							l_key   = l_key_value_pair.mid( 0, l_key_separator_pos );
							l_value = l_key_value_pair.mid( l_key_separator_pos + 1, l_key_value_pair.lengthS() - l_key_separator_pos + 1 );

							l_value.replaceAll( "\"", "" );

							l_extended_infos.emplace( l_key.GetW(), l_value.GetW() );

							if ( l_separator_pos > 0 )
								l_line_trail = l_line_trail.mid( l_separator_pos + l_move_size, l_line_trail.lengthS() - l_separator_pos + 1 );
							else
								l_line_trail.clear();
						}


						l_string = l_string.substr( l_pos + 1, l_string.size() - l_pos );

						StringCchCopyW( ext_title, MAX_PATH, l_string.c_str() );
					}
					else
						StringCchCopyW( ext_title, MAX_PATH, CharNextW( p ) );
				}
				else
				{
					ext_len        = -1;
					ext_title[ 0 ] = 0;
				}
			}
			// cloud specific playlist line for holding information about the entry
			else if ( ext && !wcsncmp( p, _DIRECTIVE_EXT_X_NS_CLOUD, 16 ) )
			{
				p += 16;
				wchar_t *pt = wcstok( p, L"," );
				while ( pt != NULL )
				{
					int end = (int)wcscspn( pt, L"=" );

					if ( !wcsncmp( pt, _INFO_NAME_MEDIA_HASH, end ) )
					{
						if ( ( lstrcpynW( ext_mediahash, pt + end + 1, 128 ) ) == NULL )
							return IFC_PLAYLISTLOADER_FAILED;
					}
					else if ( !wcsncmp( pt, _INFO_NAME_META_HASH, end ) )
					{
						if ( ( lstrcpynW( ext_metahash, pt + end + 1, 128 ) ) == NULL )
							return IFC_PLAYLISTLOADER_FAILED;
					}
					else if ( !wcsncmp( pt, _INFO_NAME_CLOUD_ID, end ) )
					{
						if ( ( lstrcpynW( ext_cloud_id, pt + end + 1, 128 ) ) == NULL )
							return IFC_PLAYLISTLOADER_FAILED;
					}
					else if ( !wcsncmp( pt, _INFO_NAME_CLOUD_STATUS, end ) )
					{
						if ( ( lstrcpynW( ext_cloud_status, pt + end + 1, 16 ) ) == NULL )
							return IFC_PLAYLISTLOADER_FAILED;
					}
					else if ( !wcsncmp( pt, _INFO_NAME_CLOUD_DEVICES, end ) )
					{
						wchar_t *p2 = pt + end + 1;
						while ( p2 && *p2 != '\n' )
							p2 = CharNextW( p2 );

						if ( p2 && *p2 == '\n' )
							*p2 = 0;

						if ( ( lstrcpynW( ext_cloud_devices, pt + end + 1, 128 ) ) == NULL )
							return IFC_PLAYLISTLOADER_FAILED;
					}

					pt = wcstok( NULL, L"," );
				}
			}
			else
			{
				ext_len                = -1;
				ext_title[ 0 ]         = 0;
				ext_mediahash[ 0 ]     = 0;
				ext_metahash[ 0 ]      = 0;
				ext_cloud_id[ 0 ]      = 0;
				ext_cloud_status[ 0 ]  = 0;
				ext_cloud_devices[ 0 ] = 0;
			}
		}
	}

	if ( fp )
		fclose( fp );

	return IFC_PLAYLISTLOADER_SUCCESS;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS M3ULoader

START_DISPATCH;
CB( IFC_PLAYLISTLOADER_LOAD, Load )
#if 0
VCB( IFC_PLAYLISTLOADER_CLOSE, Close )
CB( IFC_PLAYLISTLOADER_GETITEM, GetItem )
CB( IFC_PLAYLISTLOADER_GETITEMTITLE, GetItemTitle )
CB( IFC_PLAYLISTLOADER_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds )
CB( IFC_PLAYLISTLOADER_GETITEMEXTENDEDINFO, GetItemExtendedInfo )
CB( IFC_PLAYLISTLOADER_NEXTITEM, NextItem )
#endif
END_DISPATCH;