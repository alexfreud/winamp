#pragma once

#include <vector>
#include "../playlist/ifc_playlistloadercallbackT.h"

class USBDevice;
class UsbSong;

class USBPlaylist: public ifc_playlistloadercallbackT<USBPlaylist>
{
public:
	USBPlaylist(USBDevice& d, LPCTSTR pszPlaylist, BOOL master);
	~USBPlaylist();
	
public:
	/*** ifc_playlistloadercallback ***/
	int OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);

public:
	// utility
	BOOL isMaster() { return master; }
	wchar_t* getFilename() { return filename; }

public:
	USBDevice &device;
	wchar_t playlistName[MAX_PATH];
	wchar_t playlistPath[MAX_PATH];
	typedef std::vector<UsbSong*> SongList;
	SongList songs;
	wchar_t filename[MAX_PATH];
	BOOL master;
	BOOL dirty;
};


