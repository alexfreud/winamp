#pragma once
#include "../playlist/ifc_playlist.h"
#include <vector>
#include <windows.h> // for MAX_PATH
#include <bfc/multipatch.h>
#include "../playlist/ifc_playlistloadercallback.h"

class merge_pl_entry
{
public:
	merge_pl_entry()                                                  {}
	merge_pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms );
	~merge_pl_entry();

	size_t GetFilename( wchar_t *p_filename, size_t filenameCch );
	size_t GetTitle( wchar_t *p_title, size_t titleCch );
	int    GetLengthInMilliseconds();
	size_t GetExtendedInfo( const wchar_t *metadata, wchar_t *info, size_t infoCch );

	void   SetFilename( const wchar_t *p_filename );
	void   SetTitle( const wchar_t *p_title );
	void   SetLengthMilliseconds( int p_length_ms );


	wchar_t *filename  = 0;
	wchar_t *filetitle = 0;
	int      length    = -1;
	bool     cached    = false;
};

enum { patch_playlist, patch_playlistloadercallback };

class MergePlaylist : public MultiPatch<patch_playlist, ifc_playlist>, public MultiPatch<patch_playlistloadercallback, ifc_playlistloadercallback>
{
public:
	MergePlaylist();
	~MergePlaylist();

	void           Clear();
	void           OnFile( const wchar_t *p_filename, const wchar_t *p_title, int lengthInMS, ifc_plentryinfo *info );
	void           AppendWithInfo( const wchar_t *p_filename, const wchar_t *p_title, int lengthInMS );
	
	size_t         GetNumItems();

	size_t         GetItem( size_t item, wchar_t *p_filename, size_t filenameCch );
	size_t         GetItemTitle( size_t item, wchar_t *p_title, size_t titleCch );
	const wchar_t *ItemTitle( size_t item );
	const wchar_t *ItemName( size_t item );
	int            GetItemLengthMilliseconds( size_t item );
	size_t         GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch );

	void           SetItemFilename( size_t item, const wchar_t *p_filename );
	void           SetItemTitle( size_t item, const wchar_t *p_title );
	void           SetItemLengthMilliseconds( size_t item, int p_length_ms );

	void           AppendPlaylist( MergePlaylist &copy );

	bool           HasFilename( const wchar_t *p_filename );

	uint64_t total_time;


protected:
	RECVS_MULTIPATCH;

public:
	//private:
	typedef std::vector<merge_pl_entry *> PlaylistEntries;
	PlaylistEntries entries;
};
