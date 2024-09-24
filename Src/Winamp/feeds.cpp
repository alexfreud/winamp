#include "main.h"
#include "feeds.h"

const static wchar_t VID_Info[] = L"VID_Info";
int VideoTextFeed::hasFeed(const wchar_t *name)
{
	if (!_wcsicmp(name, VID_Info))
		return 1;
	else
		return 0;
}

//extern "C" extern	char vidoutbuf_save[1024];
static wchar_t wideVideo[1024]=L"";
	const wchar_t *VideoTextFeed::getFeedText(const wchar_t *name)
	{
		return wideVideo;
	}

	const wchar_t *VideoTextFeed::getFeedDescription(const wchar_t *name)
	{
		return L"Video Info Text";
	}
	
	void VideoTextFeed::UpdateText(const wchar_t *text, int length)
	{
		if (!text)
			text=L"";
		wideVideo[1023]=0;
		StringCchCopyW(wideVideo, 1024, text); 
		CallViewers(VID_Info, text, length);
	}


// --------

const static wchar_t PE_Info[] = L"PE_Info";
int PlaylistTextFeed::hasFeed(const wchar_t *name)
{
	if (!_wcsicmp(name, PE_Info))
		return 1;
	else
		return 0;
}

	const wchar_t *PlaylistTextFeed::getFeedText(const wchar_t *name)
	{
		return playlistStr;
	}

	const wchar_t *PlaylistTextFeed::getFeedDescription(const wchar_t *name)
	{
		return L"Playlist Info Text";
	}

	void PlaylistTextFeed::UpdateText(const wchar_t *text, int length)
	{
		CallViewers(PE_Info, text, length);
	}

