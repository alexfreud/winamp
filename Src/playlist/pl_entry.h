#ifndef NULLSOFT_ML_PLAYLISTS_PL_ENTRY_H
#define NULLSOFT_ML_PLAYLISTS_PL_ENTRY_H

#include "ifc_plentryinfo.h"
#include <windows.h>
#include <map>
#include <iostream>


class pl_entry
{
public:
	pl_entry()                                                        {}
	pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms );
	pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms, int p_size );
	pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms, ifc_plentryinfo *p_info );
	pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms, int p_size, ifc_plentryinfo *p_info );
	pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms,
			  const wchar_t *mediahash, const wchar_t *metahash,
			  const wchar_t *cloud_id, const wchar_t *cloud_status,
			  const wchar_t *cloud_devices );
	~pl_entry();

	size_t GetFilename( wchar_t *p_filename, size_t filenameCch );
	size_t GetTitle( wchar_t *title, size_t titleCch );
	int    GetLengthInMilliseconds();
	int    GetSizeInBytes();
	size_t GetExtendedInfo( const wchar_t *metadata, wchar_t *info, size_t infoCch );

	void   SetFilename( const wchar_t *p_filename );
	void   SetTitle( const wchar_t *title );
	void   SetLengthMilliseconds( int length );
	void   SetSizeBytes( int size );

	void   SetMediahash( const wchar_t *mediahash );
	void   SetMetahash( const wchar_t *metahash );
	void   SetCloudID( const wchar_t *cloud_id );
	void   SetCloudStatus( const wchar_t *cloud_status );
	void   SetCloudDevices( const wchar_t *cloud_devices );

	bool   isLocal() const                                            { return _is_local_file; }

	wchar_t *filename       = 0;
	wchar_t *filetitle      = 0;

	wchar_t *mediahash      = 0;
	wchar_t *metahash       = 0;
	wchar_t *cloud_id       = 0;
	wchar_t *cloud_status   = 0;
	wchar_t *cloud_devices  = 0;

	std::map<std::wstring, std::wstring> _extended_infos;

	int      length         = -1;
	int      size           = 0;

	bool     cached         = false;
	bool     _is_local_file = false;
};

#endif