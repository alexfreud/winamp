#pragma once
#include "../../General/gen_ml/ml.h" // for itemRecordW
#include <vector>

struct WifiTrack
{
	WifiTrack();
	WifiTrack(const WifiTrack &copy);
	~WifiTrack();
	WifiTrack(const char *id, const itemRecordW *record, const wchar_t *filename);
	wchar_t *id;
	wchar_t *artist;
	wchar_t *album;
	wchar_t *composer;
	int duration;
	int track;
	int year;
	int size;
	wchar_t *title;
	wchar_t *mime_type;
	__time64_t last_updated;
	wchar_t *ext;
};

class WifiPlaylist
{
public:
	WifiPlaylist();
	WifiPlaylist(const char *id, const wchar_t *name);
	~WifiPlaylist();
	void SetName(const wchar_t *new_name);
	typedef std::vector<WifiTrack*> TrackList;
	TrackList tracks;
	wchar_t *id;
	wchar_t *name;
};