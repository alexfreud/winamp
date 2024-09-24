#include "./usbdevice.h"
#include "./USBPlaylist.h"

#include <shlwapi.h>
#include <strsafe.h>

// dtor
// cleanup the memory allocated for the vector of songs
USBPlaylist::~USBPlaylist() 
{
}

// this is the constructor that gets called
USBPlaylist::USBPlaylist(USBDevice& d, LPCTSTR fileName, BOOL m) 
	: device(d), master(m), dirty(false)
{
	StringCbCopyW(filename, sizeof(filename), fileName);
	StringCbCopyW(playlistName, sizeof(playlistName), PathFindFileName(fileName));
	StringCbCopyW(playlistPath, sizeof(playlistName), fileName);
	PathRemoveFileSpec(playlistPath);
}


int USBPlaylist::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	if (NULL != filename)
	{
		UsbSong* song = NULL;
		song = device.findSongInMasterPlaylist(filename);
		songs.push_back(song);
	}
	return ifc_playlistloadercallback::LOAD_CONTINUE;
}