#pragma once

#ifndef _WA_PLAYLISTS_H
#define _WA_PLAYLISTS_H

#include <iostream>   // for std::wstring

#include "bfc\dispatch.h"
#include "bfc\platform\types.h"

namespace wa
{
	namespace Components
	{
		namespace playlists
		{
			//
			// CONST
			//
			static const std::wstring _CONST_PLAYLIST_INFO_MEDIAHASH     = L"mediahash";
			static const std::wstring _CONST_PLAYLIST_INFO_METAHASH      = L"metahash";
			static const std::wstring _CONST_PLAYLIST_INFO_CLOUD_ID      = L"cloud_id";
			static const std::wstring _CONST_PLAYLIST_INFO_CLOUD_STATUS  = L"cloud_status";
			static const std::wstring _CONST_PLAYLIST_INFO_CLOUD_DEVICES = L"cloud_devices";
			static const std::wstring _CONST_PLAYLIST_INFO_CLOUD         = L"cloud";


			//
			// ifc_plentryinfo
			//
			class ifc_plentryinfo : public Dispatchable
			{
			protected:
				ifc_plentryinfo()                                                 {}
				~ifc_plentryinfo()                                                {}

			public:
				virtual const std::wstring GetExtendedInfo( const std::wstring &p_parameter );

				DISPATCH_CODES
				{
					IFC_PLENTRYINFO_GETEXTENDEDINFO = 10,
				};
			};

			inline const std::wstring ifc_plentryinfo::GetExtendedInfo( const std::wstring &p_parameter )
			{
				return std::wstring( _call( IFC_PLENTRYINFO_GETEXTENDEDINFO, (const wchar_t *)0, p_parameter.c_str() ) );
			}


			//
			// playlist_entry
			//
			class playlist_entry
			{
			public:
				playlist_entry()                                      {}
				playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms );
				playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms, int p_size );
				playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms, ifc_plentryinfo *p_info );
				playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms, int p_size, ifc_plentryinfo *p_info );
				playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms,
								const std::wstring &p_mediahash, const std::wstring &p_metahash, const std::wstring &p_cloud_id, const std::wstring &p_cloud_status, const std::wstring &p_cloud_devices );
				playlist_entry( const std::wstring &p_filename, const std::wstring &p_title, int p_length_ms, int p_size,
								const std::wstring &p_mediahash, const std::wstring &p_metahash, const std::wstring &p_cloud_id, const std::wstring &p_cloud_status, const std::wstring &p_cloud_devices );

				virtual ~playlist_entry()                             {}

				virtual size_t GetFilename( std::wstring &p_filename, const size_t p_filename_max_length );
				virtual size_t GetTitle( std::wstring &p_title, const size_t p_title_max_length );

				virtual int    GetLengthInMilliseconds() const        { return this->__length_ms; }
				virtual int    GetSizeInBytes() const                 { return this->__size; }

				virtual size_t GetExtendedInfo( const std::wstring &p_metadata, std::wstring &p_result, const size_t p_result_max_length );

				virtual void   SetFilename( const std::wstring &p_filename );
				virtual void   SetTitle( const std::wstring &p_title );

				virtual void   SetLengthMilliseconds( int p_length_ms );
				virtual void   SetSizeBytes( int p_size );

				virtual void   SetMediahash( const std::wstring &p_mediahash );
				virtual void   SetMetahash( const std::wstring &p_metahash );
				virtual void   SetCloudID( const std::wstring &p_cloud_id );
				virtual void   SetCloudStatus( const std::wstring &p_cloud_status );
				virtual void   SetCloudDevices( const std::wstring &p_cloud_devices );

				virtual bool   isCached() const                       { return __is_cached; }
				virtual bool   isLocalFile() const                    { return __is_local_file; }


			protected:
				std::wstring  __filename;
				std::wstring  __filetitle;

				std::wstring  __mediahash;
				std::wstring  __metahash;
				std::wstring  __cloud_id;
				std::wstring  __cloud_status;
				std::wstring  __cloud_devices;

				int           __length_ms     = -1;
				int           __size          = 0;

				bool          __is_cached     = false;
				bool          __is_local_file = false;
			};
		}
	}
}


#endif // !_WA_PLAYLISTS_H
