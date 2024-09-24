#ifndef NULLSOFT_PLAYLIST_API_PLAYLISTS_H
#define NULLSOFT_PLAYLIST_API_PLAYLISTS_H

#include <bfc/dispatch.h>
#include <bfc/platform/guid.h>
#include <bfc/std_mkncc.h>
// manages Winamp's master list of playlists

/* Important note to users of this API:
 This API does not actually parse or in any way read the contents of the playlist files themselves.
 It only manages the master "list" of playlists, used in e.g. ml_playlists.

 --- important ---
 This also means that some information retrieved through this API can be inaccurate,
 such as playlist item length or total time.  These values are provided as a cache,
 to speed up display of UI.  They are not to be relied on for determining how many items
 are actually in the playlist.  Don't use this value to allocate memory for data structures,
 unless it's just an initial guess and you allow for realloc'ing if the count is higher.
 -----------------

 It is recommended (but not required) that you call SetInfo to update calculated values,
 such as playlist item length, whenever you parse the playlist and have accurate information.

 If you need playlist parsing, use api_playlistmanager.

 This API is thread-safe, as long as you properly call Lock/Unlock
 Methods which don't require external locking are marked with [*]
 Note that these methods still lock internally
 */

enum
{
	API_PLAYLISTS_SUCCESS                  = 0,
	API_PLAYLISTS_FAILURE                  = 1, // general purpose failure
	API_PLAYLISTS_UNKNOWN_INFO_GUID        = 2, // bad GUID passed to Set/GetInfo
	API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS = 3, // take that variable name, FORTRAN77!
	API_PLAYLISTS_INVALID_INDEX            = 4, // index you passed was out of range
	API_PLAYLISTS_BAD_SIZE                 = 5, // bad dataLen passed to Set/GetInfo
};

class api_playlists : public Dispatchable
{
protected:
	api_playlists()                                                   {}
	virtual ~api_playlists()                                          {}

public:
	// call these to lock the list of playlists so no one changes in the middle of an operation.  be careful with this!
	// you can use AutoLockT<api_playlists> to help you out
	// indices are only valid between these two calls.  call GetGUID() if you need session-persistent identifiers
	void           Lock();
	void           Unlock();

	size_t         GetIterator();
	/* this value changes each time a modification is made that would invalidate indices previously retrieved.
	It does not change when information is changed
	Use it to test if your index is out of date.
	example:
	size_t playlistIterator = playlists->GetIterator();
	playlists->GetPosition(myGUID, &index);
	// ... do a bunch of stuff
	if (playlistIterator != playlists->GetIterator())
	  playlists->GetPosition(myGUID, &index);

	This is meant as a tool to aid implementations that want to cache indices to prevent too many GetPosition() lookups
	you don't need this function for casual usage of the API
	*/

	int            Sort( size_t sort_type );

	void           Flush();                     // [*] flushes playlists to disk.  avoid usage - mainly useful when some program is parsing the playlists.xml file externally

	// get information about playlists
	size_t         GetCount();                  // returns number of playlists

	const wchar_t *GetFilename( size_t index ); // returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE.  only valid until you Unlock()
	const wchar_t *GetName( size_t index );     // returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE.  only valid until you Unlock()

	GUID           GetGUID( size_t index );     // retrieves a unique ID which identifies this playlist

	int            GetPosition( GUID playlist_guid, size_t *index );               // retrieves the index where a particular playlist ID lives. returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE
	int            GetInfo( size_t index, GUID info, void *data, size_t dataLen ); // This is for getting "extra" data, see list of GUIDs below.  returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE

	// manipulating playlists
	// at this time, it is not recommended that you use this API.  It is reserved for ml_playlists.
	int            MoveBefore( size_t index1, size_t index2 ); // moves playlist at position index1 to before index2.  setting index2 to anything larger than GetCount() moves to end

	size_t         AddPlaylist( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid = INVALID_GUID );      // [*] adds a new playlist, returns new index.  Generates a GUID if you don't pass third parameter. returns (size_t)-1 on error and (size_t)-2 if already exists
	// note: AddPlaylist locks internally, but you need to lock externally if you want to trust the return value																																																					

	size_t         AddCloudPlaylist( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid = INVALID_GUID ); // [*] adds a new playlist, returns new index.  Generates a GUID if you don't pass third parameter. returns (size_t)-1 on error and (size_t)-2 if already exists
	// note: AddCloudPlaylist locks internally, but you need to lock externally if you want to trust the return value																																																					

	size_t         AddPlaylist_NoCallback( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid = INVALID_GUID );
	// same as AddPlaylist, but doesn't do a syscallback, use when you want to make a few SetInfo calls, 
	// when you are done, call WASABI_API_SYSCB->syscb_issueCallback(api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_REMOVED_POST, index, 0); yourself

	int            SetGUID( size_t index, GUID playlist_guid );                    // sets (overrides) a playlist ID.  Don't use unless you have some very specific need
	int            RenamePlaylist( size_t index, const wchar_t *name );
	int            MovePlaylist( size_t index, const wchar_t *filename );          // sets a new filename.  NOTE: IT'S UP TO YOU TO PHYSICALLY MOVE/RENAME/CREATE THE NEW FILENAME.
	int            SetInfo( size_t index, GUID info, void *data, size_t dataLen ); // returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE
	int            RemovePlaylist( size_t index );                                 // removes a particular playlist
	int            ClearPlaylists();                                               // [*] clears the entire list of playlists.  Use at your own risk :)

	/*
		callbacks.  these are sent through api_syscb.
		callbacks are typically done within the api_playlists Lock,
		so be careful not to deadlock by waiting on another thread that is also using api_playlists
		TODO
		probably want move, remove, add
		need to think through adding, though.  Someone might add a playlist and then SetInfo, so don't want to call syscb too early
	*/

	enum
	{
		SYSCALLBACK = MK4CC( 'p', 'l', 'a', 'y' ),
		PLAYLIST_ADDED         = 10, // param1 = index
		PLAYLIST_REMOVED_PRE   = 20, // param1 = index, called BEFORE it's removed internally, so you can still query for data (Get* function calls)
		PLAYLIST_REMOVED_POST  = 30, // no parameters, called after it's removed internally
		PLAYLIST_RENAMED       = 40, // param1 = index
		/* These two callbacks are made by you (not api_playlists)
		 * pass some unique ID as param2  (e.g. some function pointer or pointer to a global variable)
		 * so that you can identify your own callbacks if you also listen for these events
		 */
		 PLAYLIST_SAVED         = 50, // param1 = index.  You should send this when you save a playlist.  Surround with Lock()/Unlock() so that the index is valid
		 PLAYLIST_FLUSH_REQUEST = 60, // param1 = index.  Call before you load a playlist to request anyone who might be currently modifying the same playlist to save
	};

	enum
	{
		SORT_TITLE_ASCENDING,
		SORT_TITLE_DESCENDING,
		SORT_NUMBER_ASCENDING,
		SORT_NUMBER_DESCENDING,
	};

	DISPATCH_CODES
	{
		API_PLAYLISTS_LOCK             =  10,
		API_PLAYLISTS_UNLOCK           =  20,
		API_PLAYLISTS_GETITERATOR      =  30,
		API_PLAYLISTS_FLUSH            =  40,
		API_PLAYLISTS_GETCOUNT         =  50,
		API_PLAYLISTS_GETFILENAME      =  60,
		API_PLAYLISTS_GETNAME          =  70,
		API_PLAYLISTS_GETGUID          =  80,
		API_PLAYLISTS_GETPOSITION      =  90,
		API_PLAYLISTS_GETINFO          = 100,
		API_PLAYLISTS_MOVEBEFORE       = 110,
		API_PLAYLISTS_ADDPLAYLIST      = 120,
		API_PLAYLISTS_ADDPLAYLISTNOCB  = 121,
		API_PLAYLISTS_ADDCLOUDPLAYLIST = 122,
		API_PLAYLISTS_SETGUID          = 130,
		API_PLAYLISTS_RENAMEPLAYLIST   = 140,
		API_PLAYLISTS_MOVEPLAYLIST     = 150,
		API_PLAYLISTS_SETINFO          = 160,
		API_PLAYLISTS_REMOVEPLAYLIST   = 170,
		API_PLAYLISTS_CLEARPLAYLISTS   = 180,
		API_PLAYLISTS_SORT             = 190,
	};
};

// Info GUIDS

// {C4FAD6CE-DA38-47b0-AAA9-E966D8E8E7C5}
static const GUID api_playlists_itemCount =
{ 0xc4fad6ce, 0xda38, 0x47b0, { 0xaa, 0xa9, 0xe9, 0x66, 0xd8, 0xe8, 0xe7, 0xc5 } };

// {D4E0E000-A3F5-4f18-ADA5-F2BA40689593}
static const GUID api_playlists_totalTime =
{ 0xd4e0e000, 0xa3f5, 0x4f18, { 0xad, 0xa5, 0xf2, 0xba, 0x40, 0x68, 0x95, 0x93 } };

// {F6E1AB19-6931-4cc9-BCBA-4B40DE2A959F}
static const GUID api_playlists_iTunesID =
{ 0xf6e1ab19, 0x6931, 0x4cc9, { 0xbc, 0xba, 0x4b, 0x40, 0xde, 0x2a, 0x95, 0x9f } };

// {B83AD244-7CD3-4a24-B2C5-41F42CA37F14}
static const GUID api_playlists_cloud =
{ 0xb83ad244, 0x7cd3, 0x4a24, { 0xb2, 0xc5, 0x41, 0xf4, 0x2c, 0xa3, 0x7f, 0x14 } };


inline void api_playlists::Lock()
{
	_voidcall( API_PLAYLISTS_LOCK );
}
inline void api_playlists::Unlock()
{
	_voidcall( API_PLAYLISTS_UNLOCK );
}

inline size_t api_playlists::GetIterator()
{
	return _call( API_PLAYLISTS_GETITERATOR, 0 );
}

inline void api_playlists::Flush()
{
	_voidcall( API_PLAYLISTS_FLUSH );
}

inline int api_playlists::Sort( size_t sort_type )
{
	return _call( API_PLAYLISTS_SORT, 0, sort_type );
}

inline size_t api_playlists::GetCount()
{
	return _call( API_PLAYLISTS_GETCOUNT, 0 );
}

inline const wchar_t *api_playlists::GetFilename( size_t index )
{
	return _call( API_PLAYLISTS_GETFILENAME, (const wchar_t *)0, index );
}

inline const wchar_t *api_playlists::GetName( size_t index )
{
	return _call( API_PLAYLISTS_GETNAME, (const wchar_t *)0, index );
}

inline GUID api_playlists::GetGUID( size_t index )
{
	return _call( API_PLAYLISTS_GETGUID, INVALID_GUID, index );
}

inline int api_playlists::GetPosition( GUID playlist_guid, size_t *index )
{
	return _call( API_PLAYLISTS_GETPOSITION, API_PLAYLISTS_FAILURE, playlist_guid, index );
}

inline int api_playlists::GetInfo( size_t index, GUID info, void *data, size_t dataLen )
{
	return _call( API_PLAYLISTS_GETINFO, API_PLAYLISTS_FAILURE, index, info, data, dataLen );
}

inline int api_playlists::MoveBefore( size_t index1, size_t index2 )
{
	return _call( API_PLAYLISTS_MOVEBEFORE, API_PLAYLISTS_FAILURE, index1, index2 );
}

inline size_t api_playlists::AddPlaylist( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid )
{
	return _call( API_PLAYLISTS_ADDPLAYLIST, (size_t)-1, filename, playlistName, playlist_guid );
}

inline size_t api_playlists::AddPlaylist_NoCallback( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid )
{
	return _call( API_PLAYLISTS_ADDPLAYLISTNOCB, (size_t)-1, filename, playlistName, playlist_guid );
}

inline size_t api_playlists::AddCloudPlaylist( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid )
{
	return _call( API_PLAYLISTS_ADDCLOUDPLAYLIST, (size_t)-1, filename, playlistName, playlist_guid );
}

inline int api_playlists::SetGUID( size_t index, GUID playlist_guid )
{
	return _call( API_PLAYLISTS_SETGUID, API_PLAYLISTS_FAILURE, index, playlist_guid );
}

inline int api_playlists::RenamePlaylist( size_t index, const wchar_t *name )
{
	return _call( API_PLAYLISTS_RENAMEPLAYLIST, API_PLAYLISTS_FAILURE, index, name );
}

inline int api_playlists::MovePlaylist( size_t index, const wchar_t *filename )
{
	return _call( API_PLAYLISTS_MOVEPLAYLIST, API_PLAYLISTS_FAILURE, index, filename );
}

inline int api_playlists::SetInfo( size_t index, GUID info, void *data, size_t dataLen )
{
	return _call( API_PLAYLISTS_SETINFO, API_PLAYLISTS_FAILURE, index, info, data, dataLen );
}

inline int api_playlists::RemovePlaylist( size_t index )
{
	return _call( API_PLAYLISTS_REMOVEPLAYLIST, API_PLAYLISTS_FAILURE, index );
}

inline int api_playlists::ClearPlaylists()
{
	return _call( API_PLAYLISTS_CLEARPLAYLISTS, API_PLAYLISTS_FAILURE );
}

// {2DC3C390-D9B8-4a49-B230-EF240ADDDCDB}
static const GUID api_playlistsGUID =
{ 0x2dc3c390, 0xd9b8, 0x4a49, { 0xb2, 0x30, 0xef, 0x24, 0xa, 0xdd, 0xdc, 0xdb } };

#endif