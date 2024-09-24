#ifndef NULLSOFT_PLAYLIST_PLAYLISTS_H
#define NULLSOFT_PLAYLIST_PLAYLISTS_H

#include "api_playlists.h"
#include <vector>
#include "../nu/AutoLock.h"

class PlaylistInfo
{
public:
	PlaylistInfo();
	PlaylistInfo(const wchar_t *_filename, const wchar_t *_title, GUID playlist_guid = INVALID_GUID);
	PlaylistInfo(const PlaylistInfo &copy);
	wchar_t filename[MAX_PATH];
	wchar_t title[1024];
	int length; // in seconds
	int numItems;
	GUID guid;
	uint64_t iTunesID; // this is used by ml_impex
	int cloud;	// this is used by ml_playlists
};

class Playlists : public api_playlists
{
public:
	Playlists();
	bool DelayLoad();

	// api_playlists implementations
	void Lock();
	void Unlock();

	size_t GetIterator();
	int Sort(size_t sort_type);
	void Flush();

	// get information about playlists
	size_t GetCount(); // returns number of playlists
	const wchar_t *GetFilename(size_t index); // returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE.  only valid until you Unlock()
	const wchar_t *GetName(size_t index); // returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE.  only valid until you Unlock()
	GUID GetGUID(size_t index); // retrieves a unique ID which identifies this playlist
	int GetPosition(GUID playlist_guid, size_t *index); // retrieves the index where a particular playlist ID lives. returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE
	int GetInfo(size_t index, GUID info, void *data, size_t dataLen); // This is for getting "extra" data, see list of GUIDs below.  returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE

	// manipulating playlists
	int MoveBefore(size_t index1, size_t index2); // moves playlist at position index1 to before index2.  setting index2 to anything larger than GetCount() moves to end
	size_t AddPlaylist(const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid = INVALID_GUID); // adds a new playlist, returns new index.  Generates a GUID if you don't pass third parameter.
	size_t AddPlaylist_NoCallback(const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid = INVALID_GUID);
	size_t AddCloudPlaylist(const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid = INVALID_GUID); // adds a new playlist, returns new index.  Generates a GUID if you don't pass third parameter.
	int SetGUID(size_t index, GUID playlist_guid); // sets (overrides) a playlist ID.  Don't use unless you have some very specific need
	int RenamePlaylist(size_t index, const wchar_t *name);
	int MovePlaylist(size_t index, const wchar_t *filename); // sets a new filename.  NOTE: IT'S UP TO YOU TO PHYSICALLY MOVE/RENAME/CREATE THE NEW FILENAME.
	int SetInfo(size_t index, GUID info, void *data, size_t dataLen); // returns API_PLAYLISTS_SUCCESS or API_PLAYLISTS_FAILURE
	int RemovePlaylist(size_t index); // removes a particular playlist
	int ClearPlaylists(); // [*] clears the entire playlist.  Use at your own risk :)

	size_t AddPlaylist_internal(const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid, size_t numItems, size_t length, uint64_t iTunesID, size_t cloud);
	
	// for SPlaylists to use
	const PlaylistInfo &GetPlaylistInfo(size_t i);
private:
	bool loaded; // whether or not playlists.xml has been loaded
	bool triedLoaded; // whether or not we tried to load
	bool dirty;	// whether a flush should save on Flush()
	size_t iterator; // a counter to help clients determine if the list data has changed
	typedef std::vector<PlaylistInfo> PlaylistsList;
	PlaylistsList playlists;

	Nullsoft::Utility::LockGuard playlistsGuard;

	RECVS_DISPATCH;
};

#endif