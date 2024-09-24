#include "WifiPlaylist.h"
#include "api.h"
#include "nu/AutoWide.h"

/* ---- WifiTrack ---- */
WifiTrack::WifiTrack()
{
	last_updated=0;
	id=0;
	artist=0;
	album=0;
	composer=0;
	duration=0;
	track=0;
	year=0;
	size=0;
	title=0;
	mime_type=0;
}

WifiTrack::WifiTrack(const char *id, const itemRecordW *record, const wchar_t *filename)
{
	this->id=AutoWideDup(id);
	artist=_wcsdup(record->artist);
	album=_wcsdup(record->album);
	composer=_wcsdup(record->composer);
	duration=record->length;
	track=record->track;
	year=record->year;
	size=record->filesize;
	title=_wcsdup(record->title);
	wchar_t mime[128] = {0};
	if (AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"mime", mime, 128) && mime[0])
	{
		mime_type=_wcsdup(mime);
	}
	else
	{
		mime_type=0;
	}

	last_updated=record->lastupd;
}

WifiTrack::WifiTrack(const WifiTrack &copy)
{
	id=_wcsdup(copy.id);
	artist=_wcsdup(copy.artist);
	album=_wcsdup(copy.album);
	composer=_wcsdup(copy.composer);
	duration=copy.duration;
	track=copy.track;
	year=copy.year;
	size=copy.size;
	title=_wcsdup(copy.title);
	mime_type=_wcsdup(copy.mime_type);
	last_updated=copy.last_updated;
}

WifiTrack::~WifiTrack()
{
	free(id);
	free(artist);
	free(album);
	free(composer);
	free(title);
	free(mime_type);
}


/* ---- WifiPlaylist ---- */

WifiPlaylist::WifiPlaylist()
{
	id=0;
	name=0;
}

WifiPlaylist::WifiPlaylist(const char *id, const wchar_t *name)
{
	this->id = AutoWideDup(id);
	this->name = _wcsdup(name);
}

WifiPlaylist::~WifiPlaylist()
{
	free(id);
	free(name);
	//tracks.deleteAll();
	for (auto obj : tracks)
	{
		delete obj;
	}
	tracks.clear();
}

void WifiPlaylist::SetName(const wchar_t *new_name)
{
	if (name != new_name)
	{
		free(name);
		name = _wcsdup(new_name);
	}
}