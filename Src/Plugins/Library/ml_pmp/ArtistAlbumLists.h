#ifndef _ARTISTALBUMLISTS_H_
#define _ARTISTALBUMLISTS_H_

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <shlobj.h>
#include <time.h>
#include "..\..\General\gen_ml/ml.h"
#include "../nu/listview.h"
#include "..\..\General\gen_ml/itemlist.h"
#include "..\..\General\gen_ml/childwnd.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"
#include "pmp.h"
#include "SkinnedListView.h" 
#include "config.h"

class Filter;
class FilterList;
class TracksList;

#define MAX_FILTERS 3
extern int thread_killed;

class ArtistAlbumLists {
protected:
	Device * dev;
	int playlistId;
	C_ItemList * searchedTracks;
	C_ItemList * unrefinedTracks;
	C_ItemList * trackList;
	C_ItemList * CompilePrimaryList(const C_ItemList * songs);
	C_ItemList * CompileSecondaryList(const C_ItemList * selectedArtists, int level, bool updateTopArtist);
	C_ItemList * FilterSongs(const wchar_t * filter, const C_ItemList * songs);
	int numFilters;
	FilterList *filters[MAX_FILTERS];
	Filter * firstFilter;
	TracksList *tracksLC;
	int type;
	wchar_t * lastSearch;
	wchar_t * lastRefine;
public:
	ArtistAlbumLists(Device * dev, int playlistId, C_Config * config, wchar_t ** filterNames, int numFilters, int type=-1, bool async=false);
	~ArtistAlbumLists();
	void SetRefine(const wchar_t * str, bool async=false);
	void SetSearch(const wchar_t * str, bool async=false);
	void SelectionChanged(int filterNum, SkinnedListView **listview);
	ListContents * GetFilterList(int i);
	PrimaryListContents * GetTracksList();
	void RemoveTrack(songid_t song); // removes song from all relevant lists

	// used for threaded background scans (mainly aimed for cloud support but could be used for other devices if needed)
	DWORD WINAPI bgLoadThreadProc(void *tmp);
	DWORD WINAPI bgSearchThreadProc(void *tmp);
	DWORD WINAPI bgRefineThreadProc(void *tmp);
	void bgQuery_Stop();
	int bgThread_Kill;
	HANDLE bgThread_Handle;
	bool async;
	void bgQuery(int mode=0);
};

#endif //_ARTISTALBUMLISTS_H_