#include "./usbplaylistsaver.h"
#include "./usbplaylist.h"
#include "./usbdevice.h"
#include "./api.h"

#include <strsafe.h>


USBPlaylistSaver::USBPlaylistSaver(LPCTSTR iFilename, LPCTSTR iPlaylistName, USBPlaylist *iPlaylist)
	: title((LPTSTR)iPlaylistName), filename((LPTSTR)iFilename), playlist(iPlaylist)
{
}

USBPlaylistSaver::~USBPlaylistSaver()
{
}

HRESULT USBPlaylistSaver::Save()
{
	INT result = WASABI_API_PLAYLISTMNGR->Save(filename, this);
	
	return (PLAYLISTMANAGER_SUCCESS == result) ? S_OK : E_FAIL;
}

size_t USBPlaylistSaver::GetNumItems()
{
	return (size_t) playlist->songs.size();
}

size_t USBPlaylistSaver::GetItem(size_t item, wchar_t *filename, size_t filenameCch)
{
	UsbSong* song = (UsbSong *) playlist->songs.at(item);
	if (!song) return 0;

	HRESULT hr = StringCchCopyEx(filename, filenameCch, song->filename, NULL, NULL, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr))
		*filename = L'\0';

	return SUCCEEDED(hr);
}

size_t USBPlaylistSaver::GetItemTitle(size_t item, wchar_t *title, size_t titleCch)
{
	UsbSong* song = (UsbSong *) playlist->songs.at(item);
	if (!song) return 0;

	HRESULT hr = StringCchCopyEx(title, titleCch, song->title, NULL, NULL, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr))
		*title = L'\0';
	
	return SUCCEEDED(hr);
}

int USBPlaylistSaver::GetItemLengthMs(size_t item)
{
	UsbSong* song = (UsbSong *) playlist->songs.at(item);
	if (!song) return 0;

	return song->length ? song->length: -1;
}

size_t USBPlaylistSaver::GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch)
{
	return 0;
}

#define CBCLASS USBPlaylistSaver
START_DISPATCH;
CB(IFC_PLAYLIST_GETNUMITEMS, GetNumItems)
CB(IFC_PLAYLIST_GETITEM, GetItem)
CB(IFC_PLAYLIST_GETITEMTITLE, GetItemTitle)
CB(IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMs)
CB(IFC_PLAYLIST_GETITEMEXTENDEDINFO, GetItemExtendedInfo)
END_DISPATCH;