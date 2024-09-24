#include "wac_playlists_entry.h"


wa::Components::playlists::playlist_entry::playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms )
{
	this->__filename  = p_filename;
	this->__filetitle = p_title;

	SetLengthMilliseconds( p_length_ms );
}

wa::Components::playlists::playlist_entry::playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms, int p_size )
{
	this->__filename  = p_filename;
	this->__filetitle = p_title;

	SetLengthMilliseconds( p_length_ms );
	SetSizeBytes( p_size );
}

wa::Components::playlists::playlist_entry::playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms, ifc_plentryinfo *p_info )
{
	this->__filename  = p_filename;
	this->__filetitle = p_title;

	SetLengthMilliseconds( p_length_ms );

	if ( p_info )
	{
		SetMediahash( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_MEDIAHASH ) );
		SetMetahash( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_METAHASH ) );
		SetCloudID( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_CLOUD_ID ) );
		SetCloudStatus( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_CLOUD_STATUS ) );
		SetCloudDevices( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_CLOUD_DEVICES ) );
	}
}

wa::Components::playlists::playlist_entry::playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms, int p_size, ifc_plentryinfo *p_info )
{
	this->__filename  = p_filename;
	this->__filetitle = p_title;

	SetLengthMilliseconds( p_length_ms );
	SetSizeBytes( p_size );

	if ( p_info )
	{
		SetMediahash( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_MEDIAHASH ) );
		SetMetahash( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_METAHASH ) );
		SetCloudID( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_CLOUD_ID ) );
		SetCloudStatus( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_CLOUD_STATUS ) );
		SetCloudDevices( p_info->GetExtendedInfo( _CONST_PLAYLIST_INFO_CLOUD_DEVICES ) );
	}
}

wa::Components::playlists::playlist_entry::playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms,
														   const std::wstring &p_mediahash, const std::wstring &p_metahash, const std::wstring &p_cloud_id, const std::wstring &p_cloud_status, const std::wstring &p_cloud_devices )
{
	this->__filename      = p_filename;
	this->__filetitle     = p_title;

	SetLengthMilliseconds( p_length_ms );

	this->__mediahash     = p_mediahash;
	this->__metahash      = p_metahash;
	
	SetCloudID( p_cloud_id );
	SetCloudStatus( p_cloud_status );

	this->__cloud_devices = p_cloud_devices;
}

wa::Components::playlists::playlist_entry::playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms, int p_size,
														   const std::wstring &p_mediahash, const std::wstring &p_metahash, const std::wstring &p_cloud_id, const std::wstring &p_cloud_status, const std::wstring &p_cloud_devices )
{
	this->__filename = p_filename;
	this->__filetitle = p_title;

	SetLengthMilliseconds( p_length_ms );
	SetSizeBytes( p_size );

	this->__mediahash = p_mediahash;
	this->__metahash = p_metahash;

	SetCloudID( p_cloud_id );
	SetCloudStatus( p_cloud_status );

	this->__cloud_devices = p_cloud_devices;
}

size_t wa::Components::playlists::playlist_entry::GetFilename( std::wstring &p_filename, size_t p_filename_max_length )
{
	if ( p_filename.empty() )
		return this->__filename.size();

	if ( this->__filename.empty() )
		return 0;

	p_filename = this->__filename.substr( 0, p_filename_max_length );

	return 1;
}

size_t wa::Components::playlists::playlist_entry::GetTitle( std::wstring &p_title, size_t p_title_max_length )
{
	if ( p_title.empty() )
		return this->__filetitle.size();

	if ( this->__filetitle.empty() )
		return 0;

	p_title = this->__filetitle.substr( 0, p_title_max_length );

	return 1;
}


size_t wa::Components::playlists::playlist_entry::GetExtendedInfo( const std::wstring &p_metadata, std::wstring &p_result, const size_t p_result_max_length )
{
	if ( !__cloud_id.empty() )
	{
		if ( p_metadata.compare( _CONST_PLAYLIST_INFO_MEDIAHASH ) == 0 )
		{
			if ( !__mediahash.empty() )
			{
				p_result = __mediahash.substr( 0, p_result_max_length );

				return 1;
			}
			else
				return 0;
		}
		else if ( p_metadata.compare( _CONST_PLAYLIST_INFO_METAHASH ) == 0 )
		{
			if ( !__metahash.empty() )
			{
				p_result = __metahash.substr( 0, p_result_max_length );

				return 1;
			}
			else
				return 0;
		}
		else if ( p_metadata.compare( _CONST_PLAYLIST_INFO_CLOUD_ID ) == 0 )
		{
			if ( !__cloud_id.empty() )
			{
				p_result = __cloud_id.substr( 0, p_result_max_length );

				return 1;
			}
			else
				return 0;
		}
		else if ( p_metadata.compare( _CONST_PLAYLIST_INFO_CLOUD_STATUS ) == 0 )
		{
			if ( !__cloud_status.empty() )
			{
				p_result = __cloud_status.substr( 0, p_result_max_length );

				return 1;
			}
			else
				return 0;
		}
		else if ( p_metadata.compare( _CONST_PLAYLIST_INFO_CLOUD_DEVICES ) == 0 )
		{
			if ( !__cloud_devices.empty() )
			{
				p_result = __cloud_devices.substr( 0, p_result_max_length );

				return 1;
			}
			else
				return 0;
		}
		else if ( p_metadata.compare( _CONST_PLAYLIST_INFO_CLOUD ) == 0 )
		{
			if ( _wtoi( __cloud_id.c_str() ) > 0 )
			{
				wchar_t l_result_buffer[ 500 ] = {0};
				
				swprintf_s( l_result_buffer, p_result_max_length, L"#EXT-X-NS-CLOUD:mediahash=%s,metahash=%s,cloud_id=%s,cloud_status=%s,cloud_devices=%s", __mediahash.c_str(), __metahash.c_str(), __cloud_id.c_str(), __cloud_status.c_str(), __cloud_devices.c_str() );

				p_result = std::wstring( l_result_buffer );

							  
				return 1;
			}
		}
	}

	return 0;
}


void wa::Components::playlists::playlist_entry::SetFilename( const std::wstring &p_filename )
{
	this->__filename = p_filename;

	if ( p_filename.size() > 4 )
		__is_local_file = this->__filename.substr( 0, 4 ).compare( L"http" ) != 0;
}

void wa::Components::playlists::playlist_entry::SetTitle( const std::wstring &p_title )
{
	this->__filetitle = p_title;
	this->__is_cached = !p_title.empty();
}


void wa::Components::playlists::playlist_entry::SetLengthMilliseconds( int p_length_ms )
{
	if ( p_length_ms <= 0 )
		this->__length_ms = -1000;
	else
		this->__length_ms = p_length_ms;
}


void wa::Components::playlists::playlist_entry::SetMediahash( const std::wstring &p_mediahash )
{
	this->__mediahash = p_mediahash;
}

void wa::Components::playlists::playlist_entry::SetSizeBytes( int p_size )
{
	if ( p_size <= 0 )
		this->__size = 0;
	else
		this->__size = p_size;
}

void wa::Components::playlists::playlist_entry::SetMetahash( const std::wstring &p_metahash )
{
	this->__metahash = p_metahash;
}

void wa::Components::playlists::playlist_entry::SetCloudID( const std::wstring &p_cloud_id )
{
	if ( _wtoi( p_cloud_id.c_str() ) > 0 )
		this->__cloud_id = p_cloud_id;
	else
		this->__cloud_id.clear();
}

void wa::Components::playlists::playlist_entry::SetCloudStatus( const std::wstring &p_cloud_status )
{
	if ( _wtoi( p_cloud_status.c_str() ) > 0 )
		this->__cloud_status = p_cloud_status;
	else
		this->__cloud_status.clear();
}

void wa::Components::playlists::playlist_entry::SetCloudDevices( const std::wstring &p_cloud_devices )
{
	this->__cloud_devices = p_cloud_devices;
}



