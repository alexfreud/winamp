#ifndef NULLSOFT_IFC_PLAYLISTLOADERCALLBACK_H
#define NULLSOFT_IFC_PLAYLISTLOADERCALLBACK_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include "ifc_plentryinfo.h"

#ifndef FILENAME_SIZE
#define FILENAME_SIZE (MAX_PATH * 4)
#endif

#ifndef FILETITLE_SIZE
#define FILETITLE_SIZE 400
#endif

class ifc_playlistinfo; // TODO
class ifc_playlistloadercallback : public Dispatchable
{
protected:
	ifc_playlistloadercallback()                                      {}
	~ifc_playlistloadercallback()                                     {}
	
public:
	// return 0 to continue enumeration, or 1 to quit

	// title will be NULL if no title found, length will be -1
	int            OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info );
	void           OnFileOld( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info );

	// numEntries is just a hint, there is no gaurantee.  0 means "don't know"
	int            OnPlaylistInfo( const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info );
	void           OnPlaylistInfoOld( const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info );

	const wchar_t *GetBasePath(); // return 0 to use playlist file path as base (or just don't implement)

	DISPATCH_CODES
	{
		IFC_PLAYLISTLOADERCALLBACK_ONFILE             = 10,
		IFC_PLAYLISTLOADERCALLBACK_ONFILE_RET         = 11,
		IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO     = 20,
		IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO_RET = 21,
		IFC_PLAYLISTLOADERCALLBACK_GETBASEPATH        = 30,
	};
	enum
	{
		LOAD_CONTINUE = 0,
		LOAD_ABORT    = 1,
	};
};

inline void ifc_playlistloadercallback::OnFileOld( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info )
{
	_voidcall( IFC_PLAYLISTLOADERCALLBACK_ONFILE, filename, title, lengthInMS, info );
}

inline int ifc_playlistloadercallback::OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info )
{
	void *params[ 4 ] = { &filename, &title, &lengthInMS, &info };
	int   retval;

	if ( _dispatch( IFC_PLAYLISTLOADERCALLBACK_ONFILE_RET, &retval, params, 4 ) == 0 )
	{
		_dispatch( IFC_PLAYLISTLOADERCALLBACK_ONFILE, 0, params, 4 );

		return LOAD_CONTINUE;
	}

	return retval;
}

inline void ifc_playlistloadercallback::OnPlaylistInfoOld( const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info )
{
	_voidcall( IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO, playlistName, numEntries, info );
}

inline int ifc_playlistloadercallback::OnPlaylistInfo( const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info )
{
	void *params[ 3 ] = { &playlistName, &numEntries, &info };
	int   retval;

	if ( _dispatch( IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO_RET, &retval, params, 3 ) == 0 )
	{
		_dispatch( IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO, 0, params, 3 );

		return LOAD_CONTINUE;
	}

	return retval;
}

inline const wchar_t *ifc_playlistloadercallback::GetBasePath()
{
	return _call( IFC_PLAYLISTLOADERCALLBACK_GETBASEPATH, (const wchar_t *)0 );
}

#endif