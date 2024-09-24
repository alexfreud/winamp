#pragma once

#include <vector>
#include "../playlist/ifc_playlistloadercallback.h"
#include "../nu/AutoLock.h"

class AndroidDevice;
class AndroidSong;

class AndroidPlaylist: public ifc_playlistloadercallback
{
public:
	AndroidPlaylist(AndroidDevice& d, LPCTSTR pszPlaylist, BOOL master);
	~AndroidPlaylist();
	
public:
	/*** ifc_playlistloadercallback ***/
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);

public:
	// utility
	BOOL isMaster() { return master; }
	wchar_t* getFilename() { return filename; }
	size_t size();
	AndroidSong *&at(size_t index);
	void push_back(AndroidSong *callback);
	void RemoveSong(AndroidSong *song);
	void swap(size_t index1, size_t index2);
	void eraseAt(size_t index);
	AndroidSong *FindSong(const wchar_t *filename);

protected:
	RECVS_DISPATCH;
private:
	
	AndroidDevice &device;
	typedef std::vector<AndroidSong*> SongList;
	SongList songs;
public:
	//Nullsoft::Utility::LockGuard songs_guard;
	wchar_t playlistName[MAX_PATH];
	wchar_t playlistPath[MAX_PATH];

	wchar_t filename[MAX_PATH];
	BOOL master;
	BOOL dirty;
};


