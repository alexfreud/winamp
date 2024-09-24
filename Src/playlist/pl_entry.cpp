#include "main.h"
#include "pl_entry.h"
#include "plstring.h"
#include <wchar.h>
#include "../nu/strsafe.h"

#include <iostream>

static const wchar_t *_INFO_NAME_MEDIA_HASH    = L"mediahash";
static const wchar_t *_INFO_NAME_META_HASH     = L"metahash";
static const wchar_t *_INFO_NAME_CLOUD_ID      = L"cloud_id";
static const wchar_t *_INFO_NAME_CLOUD_STATUS  = L"cloud_status";
static const wchar_t *_INFO_NAME_CLOUD_DEVICES = L"cloud_devices";

pl_entry::pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms )
{
	SetFilename( p_filename );
	SetTitle( p_title );
	SetLengthMilliseconds( p_length_ms );
}

pl_entry::pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms, int p_size )
{
	SetFilename( p_filename );
	SetTitle( p_title );
	SetLengthMilliseconds( p_length_ms );
	SetSizeBytes( p_size );
}

pl_entry::pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms, ifc_plentryinfo *p_info )
{
	SetFilename( p_filename );
	SetTitle( p_title );
	SetLengthMilliseconds( p_length_ms );

	if ( p_info )
	{
		SetMediahash( p_info->GetExtendedInfo( _INFO_NAME_MEDIA_HASH ) );
		SetMetahash( p_info->GetExtendedInfo( _INFO_NAME_META_HASH ) );
		SetCloudID( p_info->GetExtendedInfo( _INFO_NAME_CLOUD_ID ) );
		SetCloudStatus( p_info->GetExtendedInfo( _INFO_NAME_CLOUD_STATUS ) );
		SetCloudDevices( p_info->GetExtendedInfo( _INFO_NAME_CLOUD_DEVICES ) );

		const wchar_t *l_tvg_name = p_info->GetExtendedInfo( L"tvg-name" );

		if ( l_tvg_name && wcslen( l_tvg_name ) > 0 )
		{
			_extended_infos.emplace( L"tvg-name", l_tvg_name );

			const wchar_t *l_tvg_id = p_info->GetExtendedInfo( L"tvg-id" );
			if ( l_tvg_id && *l_tvg_id )
				_extended_infos.emplace( L"tvg-id", l_tvg_id );

			const wchar_t *l_tvg_logo = p_info->GetExtendedInfo( L"tvg-logo" );
			if ( l_tvg_logo && *l_tvg_logo )
				_extended_infos.emplace( L"tvg-logo", l_tvg_logo );

			const wchar_t *l_tvg_title = p_info->GetExtendedInfo( L"tvg-title" );
			if ( l_tvg_title && *l_tvg_title )
				_extended_infos.emplace( L"group-title", l_tvg_title );
		}

		const wchar_t *l_ext = p_info->GetExtendedInfo( L"ext" );

		if ( l_ext && wcslen( l_ext ) )
			_extended_infos.emplace( L"ext", l_ext );
	}
}

pl_entry::pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms, int p_size, ifc_plentryinfo *p_info )
{
	SetFilename( p_filename );
	SetTitle( p_title );
	SetLengthMilliseconds( p_length_ms );
	SetSizeBytes( p_size );

	if ( p_info )
	{
		SetMediahash( p_info->GetExtendedInfo( _INFO_NAME_MEDIA_HASH ) );
		SetMetahash( p_info->GetExtendedInfo( _INFO_NAME_META_HASH ) );
		SetCloudID( p_info->GetExtendedInfo( _INFO_NAME_CLOUD_ID ) );
		SetCloudStatus( p_info->GetExtendedInfo( _INFO_NAME_CLOUD_STATUS ) );
		SetCloudDevices( p_info->GetExtendedInfo( _INFO_NAME_CLOUD_DEVICES ) );

		const wchar_t *l_tvg_name = p_info->GetExtendedInfo( L"tvg-name" );

		if ( l_tvg_name && wcslen( l_tvg_name ) > 0 )
		{
			_extended_infos.emplace( L"tvg-id", p_info->GetExtendedInfo( L"tvg-id" ) );
			_extended_infos.emplace( L"tvg-name", l_tvg_name );
			_extended_infos.emplace( L"tvg-logo", p_info->GetExtendedInfo( L"tvg-logo" ) );
			_extended_infos.emplace( L"group-title", p_info->GetExtendedInfo( L"group-title" ) );
		}
	}
}

pl_entry::pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms,
					const wchar_t *mediahash, const wchar_t *metahash,
					const wchar_t *cloud_id, const wchar_t *cloud_status,
					const wchar_t *cloud_devices )
{
	SetFilename( p_filename );
	SetTitle( p_title );
	SetLengthMilliseconds( p_length_ms );

	SetMediahash( mediahash );
	SetMetahash( metahash );
	SetCloudID( cloud_id );
	SetCloudStatus( cloud_status );
	SetCloudDevices( cloud_devices );
}

pl_entry::~pl_entry()
{
	plstring_release( filename );
	plstring_release( filetitle );
	plstring_release( mediahash );
	plstring_release( metahash );
	plstring_release( cloud_id );
	plstring_release( cloud_status );
	plstring_release( cloud_devices );
}


size_t pl_entry::GetFilename( wchar_t *p_filename, size_t filenameCch )
{
	if ( !this->filename )
		return 0;

	if ( !p_filename )
		return wcslen( this->filename );

	if ( !this->filename[ 0 ] )
		return 0;

	StringCchCopyW( p_filename, filenameCch, this->filename );

	return 1;
}

size_t pl_entry::GetTitle( wchar_t *title, size_t titleCch )
{
	if ( !this->filetitle )
		return 0;

	if ( !title )
		return wcslen( this->filetitle );

	if ( !this->filetitle[ 0 ] )
		return 0;

	StringCchCopyW( title, titleCch, this->filetitle );

	return 1;
}

int pl_entry::GetLengthInMilliseconds()
{
	return this->length;
}

int pl_entry::GetSizeInBytes()
{
	return this->size;
}

size_t pl_entry::GetExtendedInfo( const wchar_t *metadata, wchar_t *info, size_t infoCch )
{
	if ( cloud_id )
	{
		if ( !_wcsnicmp( _INFO_NAME_MEDIA_HASH, metadata, 9 ) && mediahash )
		{
			lstrcpynW( info, mediahash, (int)infoCch );
			return 1;
		}
		else if ( !_wcsnicmp( _INFO_NAME_META_HASH, metadata, 8 ) && metahash )
		{
			lstrcpynW( info, metahash, (int)infoCch );
			return 1;
		}
		else if ( !_wcsnicmp( _INFO_NAME_CLOUD_ID, metadata, 8 ) && cloud_id )
		{
			lstrcpynW( info, cloud_id, (int)infoCch );
			return 1;
		}
		else if ( !_wcsnicmp( _INFO_NAME_CLOUD_STATUS, metadata, 12 ) && cloud_status )
		{
			lstrcpynW( info, cloud_status, (int)infoCch );
			return 1;
		}
		else if ( !_wcsnicmp( _INFO_NAME_CLOUD_DEVICES, metadata, 13 ) && cloud_devices )
		{
			lstrcpynW( info, cloud_devices, (int)infoCch );
			return 1;
		}
		else if ( !_wcsnicmp( metadata, L"cloud", 5 ) )
		{
			if ( _wtoi( cloud_id ) > 0 )
			{
				StringCchPrintfW( info, infoCch, L"#EXT-X-NS-CLOUD:mediahash=%s,metahash=%s,cloud_id=%s,cloud_status=%s,cloud_devices=%s",
								  ( mediahash && *mediahash ? mediahash : L"" ),
								  ( metahash && *metahash ? metahash : L"" ), cloud_id,
								  ( cloud_status && *cloud_status ? cloud_status : L"" ),
								  ( cloud_devices && *cloud_devices ? cloud_devices : L"" ) );
				return 1;
			}
		}
		else
		{
			auto l_extended_infos_iterator = _extended_infos.find( metadata );

			if ( l_extended_infos_iterator != _extended_infos.end() )
			{
				lstrcpynW( info, ( *l_extended_infos_iterator ).second.c_str(), (int)infoCch);
				return 1;
			}
		}
	}

	if ( !this->_extended_infos.empty() )
	{
		for ( std::map<std::wstring, std::wstring>::iterator l_extented_infos_iterator = _extended_infos.begin(); l_extented_infos_iterator != _extended_infos.end(); l_extented_infos_iterator++ )
		{
			const std::wstring &l_parameter_name = ( *l_extented_infos_iterator ).first;

			if ( l_parameter_name.compare( metadata ) == 0 )
			{
				lstrcpynW( info, ( *l_extented_infos_iterator ).second.c_str(), (int)infoCch);
				return 1;
			}
		}
	}


	return 0;
}


void pl_entry::SetFilename( const wchar_t *p_filename )
{
	plstring_release( this->filename );

	if ( p_filename && p_filename[ 0 ] )
	{
		this->filename = plstring_wcsdup( p_filename );

		if ( wcslen( p_filename ) > 4 )
			_is_local_file = wcsncmp( this->filename, L"http", 4 ) != 0;
	}
	else
		this->filename = 0;
}

void pl_entry::SetTitle( const wchar_t *title )
{
	plstring_release( this->filetitle );

	if ( title && title[ 0 ] )
	{
		const wchar_t *t = L" \t\n\r\f\v";
		std::wstring l_title( title  );

		l_title.erase( 0, l_title.find_first_not_of( t ) );

		this->filetitle = plstring_wcsdup( l_title.c_str() );
		this->cached    = true;
	}
	else
		this->filetitle = 0;
}

void pl_entry::SetLengthMilliseconds( int length )
{
	if ( length <= 0 )
		this->length = -1000;
	else
		this->length = length;
}

void pl_entry::SetMediahash( const wchar_t *mediahash )
{
	plstring_release( this->mediahash );

	if ( mediahash && mediahash[ 0 ] )
		this->mediahash = plstring_wcsdup( mediahash );
	else
		this->mediahash = 0;
}

void pl_entry::SetSizeBytes( int size )
{
	if ( size <= 0 )
		this->size = 0;
	else
		this->size = size;
}

void pl_entry::SetMetahash( const wchar_t *metahash )
{
	plstring_release( this->metahash );

	if ( metahash && metahash[ 0 ] )
		this->metahash = plstring_wcsdup( metahash );
	else
		this->metahash = 0;
}

void pl_entry::SetCloudID( const wchar_t *cloud_id )
{
	plstring_release( this->cloud_id );

	if ( cloud_id && cloud_id[ 0 ] && _wtoi( cloud_id ) > 0 )
		this->cloud_id = plstring_wcsdup( cloud_id );
	else
		this->cloud_id = 0;
}

void pl_entry::SetCloudStatus( const wchar_t *cloud_status )
{
	plstring_release( this->cloud_status );

	if ( cloud_status && cloud_status[ 0 ] && _wtoi( cloud_status ) >= 0 )
		this->cloud_status = plstring_wcsdup( cloud_status );
	else
		this->cloud_status = 0;
}

void pl_entry::SetCloudDevices( const wchar_t *cloud_devices )
{
	plstring_release( this->cloud_devices );

	if ( cloud_devices && cloud_devices[ 0 ] )
		this->cloud_devices = plstring_wcsdup( cloud_devices );
	else
		this->cloud_devices = 0;
}
