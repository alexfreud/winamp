#include "main.h"
#include "MergePlaylist.h"
#include <strsafe.h>

merge_pl_entry::merge_pl_entry( const wchar_t *p_filename, const wchar_t *p_title, int p_length_ms )
{
	SetFilename( p_filename );
	SetTitle( p_title );
	SetLengthMilliseconds( p_length_ms );
}

merge_pl_entry::~merge_pl_entry()
{
	plstring_release( this->filename );
	plstring_release( this->filetitle );
}


size_t merge_pl_entry::GetFilename( wchar_t *p_filename, size_t filenameCch )
{
	if ( !this->filename )
		return 0;

	if ( !p_filename )
		return wcslen( this->filename );

	if ( !this->filename[ 0 ] )
		return 0;

	StringCchCopyW( p_filename, filenameCch, this->filename );

	return 1;
}

size_t merge_pl_entry::GetTitle( wchar_t *p_title, size_t titleCch )
{
	if ( !this->filetitle )
		return 0;

	if ( !p_title )
		return wcslen( this->filetitle );

	if ( !this->filetitle[ 0 ] )
		return 0;

	StringCchCopyW( p_title, titleCch, this->filetitle );

	return 1;
}

int merge_pl_entry::GetLengthInMilliseconds()
{
	return length;
}

size_t merge_pl_entry::GetExtendedInfo( const wchar_t *metadata, wchar_t *info, size_t infoCch )
{
	return 0;
}


void merge_pl_entry::SetFilename( const wchar_t *p_filename )
{
	plstring_release( this->filename );

	if ( p_filename && p_filename[ 0 ] )
		this->filename = plstring_wcsdup( p_filename );
	else
		this->filename = 0;
}

void merge_pl_entry::SetTitle( const wchar_t *p_title )
{
	plstring_release( filetitle );

	if ( p_title && p_title[ 0 ] )
	{
		this->filetitle = plstring_wcsdup( p_title );
		this->cached    = true;
	}
	else
		filetitle = 0;
}

void merge_pl_entry::SetLengthMilliseconds( int p_length_ms )
{
	if ( p_length_ms <= 0 )
		this->length = -1000;
	else
		this->length = p_length_ms;
}



MergePlaylist::MergePlaylist()
{
	total_time = 0;
}

void MergePlaylist::Clear()
{
	for ( merge_pl_entry *l_merged_entry : entries )
		delete l_merged_entry;

	entries.clear();
}

void MergePlaylist::OnFile( const wchar_t *p_filename, const wchar_t *p_title, int lengthInMS, ifc_plentryinfo *info )
{
	if ( lengthInMS > 0 )
		total_time += lengthInMS;

	entries.push_back( new merge_pl_entry( p_filename, p_title, lengthInMS ) );
}

void MergePlaylist::AppendWithInfo( const wchar_t *p_filename, const wchar_t *p_title, int lengthInMS )
{
	if ( lengthInMS > 0 )
		total_time += lengthInMS;

	entries.push_back( new merge_pl_entry( p_filename, p_title, lengthInMS ) );
}

MergePlaylist::~MergePlaylist()
{
	Clear();
}

size_t MergePlaylist::GetNumItems()
{
	return entries.size();
}

size_t MergePlaylist::GetItem( size_t item, wchar_t *p_filename, size_t filenameCch )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->GetFilename( p_filename, filenameCch );
}

size_t MergePlaylist::GetItemTitle( size_t item, wchar_t *p_title, size_t titleCch )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->GetTitle( p_title, titleCch );
}

const wchar_t *MergePlaylist::ItemTitle( size_t item )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->filetitle;
}

const wchar_t *MergePlaylist::ItemName( size_t item )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->filename;
}

int MergePlaylist::GetItemLengthMilliseconds( size_t item )
{
	if ( item >= entries.size() )
		return -1;

	return entries[ item ]->GetLengthInMilliseconds();
}

size_t MergePlaylist::GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->GetExtendedInfo( metadata, info, infoCch );
}


void MergePlaylist::SetItemFilename( size_t item, const wchar_t *p_filename )
{
	if ( item < entries.size() )
		entries[ item ]->SetFilename( p_filename );
}

void MergePlaylist::SetItemTitle( size_t item, const wchar_t *p_title )
{
	if ( item < entries.size() )
		entries[ item ]->SetTitle( p_title );
}

void MergePlaylist::SetItemLengthMilliseconds( size_t item, int p_length_ms )
{
	if ( item < entries.size() )
		entries[ item ]->SetLengthMilliseconds( p_length_ms );
}

void MergePlaylist::AppendPlaylist( MergePlaylist &copy )
{
	for ( merge_pl_entry *l_merged_entry : copy.entries )
	{
		if ( l_merged_entry->filename && !HasFilename( l_merged_entry->filename ) )
		{
			if ( l_merged_entry->length > 0 )
				total_time += l_merged_entry->length;

			entries.push_back( l_merged_entry );
		}
	}

	copy.entries.clear();
}

bool MergePlaylist::HasFilename( const wchar_t *p_filename )
{
	for ( merge_pl_entry *l_merged_entry : entries )
	{
		if ( l_merged_entry->filename && !_wcsicmp( p_filename, l_merged_entry->filename ) )
			return true;
	}

	return false;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MergePlaylist

START_MULTIPATCH;
START_PATCH(patch_playlist)
M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_CLEAR,                     Clear)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_APPENDWITHINFO, AppendWithInfo)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_APPEND, Append)
M_CB(patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETNUMITEMS,               GetNumItems)
M_CB(patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETITEM,                   GetItem)
M_CB(patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETITEMTITLE,              GetItemTitle)
M_CB(patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds)
M_CB(patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETITEMEXTENDEDINFO,       GetItemExtendedInfo)
NEXT_PATCH(patch_playlistloadercallback)
M_VCB(patch_playlistloadercallback, ifc_playlistloadercallback, IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile);
END_PATCH
END_MULTIPATCH;
