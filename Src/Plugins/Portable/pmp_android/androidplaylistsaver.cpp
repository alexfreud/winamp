#include "./androidplaylistsaver.h"
#include "./androidplaylist.h"
#include "./androiddevice.h"
#include "./api.h"

#include <strsafe.h>


AndroidPlaylistSaver::AndroidPlaylistSaver(LPCTSTR iFilename, LPCTSTR iPlaylistName, AndroidPlaylist *iPlaylist)
	: filename((LPTSTR)iFilename), title((LPTSTR)iPlaylistName), playlist(iPlaylist)
{
}

AndroidPlaylistSaver::~AndroidPlaylistSaver()
{
}

HRESULT AndroidPlaylistSaver::Save()
{
	INT result = WASABI_API_PLAYLISTMNGR->Save(filename, this);
	
	return (PLAYLISTMANAGER_SUCCESS == result) ? S_OK : E_FAIL;
}

size_t AndroidPlaylistSaver::GetNumItems()
{
	return playlist->size();
}

size_t AndroidPlaylistSaver::GetItem(size_t item, wchar_t *filename, size_t filenameCch)
{
	AndroidSong* song = (AndroidSong *) playlist->at(item);
	if (!song) return 0;

	HRESULT hr = StringCchCopyEx(filename, filenameCch, song->filename, NULL, NULL, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr))
		*filename = L'\0';

	return SUCCEEDED(hr);
}

size_t AndroidPlaylistSaver::GetItemTitle(size_t item, wchar_t *title, size_t titleCch)
{
	AndroidSong* song = (AndroidSong *) playlist->at(item);
	if (!song) return 0;

	HRESULT hr = StringCchCopyEx(title, titleCch, song->title, NULL, NULL, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr))
		*title = L'\0';
	
	return SUCCEEDED(hr);
}

int AndroidPlaylistSaver::GetItemLengthMs(size_t item)
{
	AndroidSong* song = (AndroidSong *) playlist->at(item);
	if (!song) return 0;

	return song->length ? song->length: -1;
}

size_t AndroidPlaylistSaver::GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch)
{
	return 0;
}

#define CBCLASS AndroidPlaylistSaver
START_DISPATCH;
CB(IFC_PLAYLIST_GETNUMITEMS, GetNumItems)
CB(IFC_PLAYLIST_GETITEM, GetItem)
CB(IFC_PLAYLIST_GETITEMTITLE, GetItemTitle)
CB(IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMs)
CB(IFC_PLAYLIST_GETITEMEXTENDEDINFO, GetItemExtendedInfo)
END_DISPATCH;