
#include "PlaylistGeneratorAPI.h"
#include <api/service/waservicefactory.h>

#include "main.h"

int PlaylistGeneratorAPI::GeneratePlaylist(HWND	parent, const itemRecordListW *selectedSeedRecordList)
{
	if (hwndDlgCurrent)					// Warn if trying to open two seperate playlist generators
	{
		MultipleInstancesWarning();
		return DISPATCH_SUCCESS;
	}
	
	AddSeedTracks(selectedSeedRecordList);
	
	if (SongsSelected())
		return DISPATCH_SUCCESS;
	
	return DISPATCH_FAILURE;
}

int PlaylistGeneratorAPI::AddSeedTracks(const itemRecordListW *recordList)
{
	wchar_t winamp_title[MAX_TITLE_SIZE] = {0};

	for (int i = 0; i < recordList->Size; i++)
	{
		itemRecordW *item = &recordList->Items[i];
		GetTitleFormattingML(item->filename, item, winamp_title, MAX_TITLE_SIZE);

		seedPlaylist.AppendWithInfo(item->filename, winamp_title, item->length * 1000, item->filesize * 1024);
	}

	return true;
}



#define CBCLASS PlaylistGeneratorAPI
START_DISPATCH;
CB(API_PLAYLIST_GENERATOR_GENERATEPLAYLIST, GeneratePlaylist)
END_DISPATCH;
#undef CBCLASS
