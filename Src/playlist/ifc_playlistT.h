#pragma once
#include "ifc_playlist.h"
template <class T>
class ifc_playlistT : public ifc_playlist
{
protected:
	ifc_playlistT()                                                      {}
	~ifc_playlistT()                                                     {}

	void Clear();
	//void AppendWithInfo(const wchar_t *filename, const char *title, int lengthInMS);
	//void Append(const wchar_t *filename);

	size_t GetNumItems()                                                 { return 0; }
	size_t GetItem( size_t item, wchar_t *filename, size_t filenameCch ) { return 0; }
	size_t GetItemTitle( size_t item, wchar_t *title, size_t titleCch )  { return 0; }
	int    GetItemLengthMilliseconds( size_t item )                      { return -1; }
	size_t GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch ) { return 0; }
	int    Reverse()                                                     { return PLAYLIST_UNIMPLEMENTED; }
	int Swap( size_t item1, size_t item2 )                               { return PLAYLIST_UNIMPLEMENTED; }
	int Randomize( int ( *generator )( ) )                               { return PLAYLIST_UNIMPLEMENTED; }
	void Remove( size_t item )                                           {}
	int SortByTitle()                                                    { return 0; }
	int SortByFilename()                                                 { return 0; }
	int SortByDirectory()                                                { return 0; }

#define CBCLASS T
#define CBCLASST ifc_playlistT<T>
	START_DISPATCH_INLINE;
	VCBT( IFC_PLAYLIST_CLEAR, Clear )
		//M_VCB( IFC_PLAYLIST_APPENDWITHINFO, AppendWithInfo)
		//M_VCB( IFC_PLAYLIST_APPEND, Append)
		CBT(  IFC_PLAYLIST_GETNUMITEMS,               GetNumItems )
		CBT(  IFC_PLAYLIST_GETITEM,                   GetItem )
		CBT(  IFC_PLAYLIST_GETITEMTITLE,              GetItemTitle )
		CBT(  IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds )
		CBT(  IFC_PLAYLIST_GETITEMEXTENDEDINFO,       GetItemExtendedInfo )
		CBT(  IFC_PLAYLIST_REVERSE,                   Reverse )
		CBT(  IFC_PLAYLIST_SWAP,                      Swap )
		CBT(  IFC_PLAYLIST_RANDOMIZE,                 Randomize )
		VCBT( IFC_PLAYLIST_REMOVE,                    Remove )
		CBT(  IFC_PLAYLIST_SORTBYTITLE,               SortByTitle )
		CBT(  IFC_PLAYLIST_SORTBYFILENAME,            SortByFilename )
		CBT(  IFC_PLAYLIST_SORTBYDIRECTORY,           SortByDirectory )
		END_DISPATCH;
#undef CBCLASS
#undef CBCLASST
};
