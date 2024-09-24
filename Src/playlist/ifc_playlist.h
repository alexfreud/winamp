#ifndef NULLSOFT_IFC_PLAYLIST_H
#define NULLSOFT_IFC_PLAYLIST_H

#include "bfc/dispatch.h"
#include "bfc/platform/types.h"

enum
{
	PLAYLIST_SUCCESS       = 0,
	PLAYLIST_UNIMPLEMENTED = 1,
};

class ifc_playlist : public Dispatchable
{
protected:
	ifc_playlist()                                                    {}
	~ifc_playlist()                                                   {}

public:
	DISPATCH_CODES
	{
		IFC_PLAYLIST_CLEAR                     = 10,
		//IFC_PLAYLIST_APPENDWITHINFO            = 20,
		//IFC_PLAYLIST_APPEND                    = 30,		
		IFC_PLAYLIST_GETNUMITEMS               = 40,
		IFC_PLAYLIST_GETITEM                   = 50,
		IFC_PLAYLIST_GETITEMTITLE              = 60,
		IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS = 70,
		IFC_PLAYLIST_GETITEMEXTENDEDINFO       = 80,
		IFC_PLAYLIST_REVERSE                   = 90,
		IFC_PLAYLIST_SWAP                      = 100,
		IFC_PLAYLIST_RANDOMIZE                 = 110,
		IFC_PLAYLIST_REMOVE                    = 120,
		IFC_PLAYLIST_SORTBYTITLE               = 130,
		IFC_PLAYLIST_SORTBYFILENAME            = 140,
		IFC_PLAYLIST_SORTBYDIRECTORY           = 150,
	};

	void   Clear();
	//void AppendWithInfo(const wchar_t *filename, const char *title, int lengthInMS);
	//void Append(const wchar_t *filename);

	size_t GetNumItems();

	size_t GetItem( size_t item, wchar_t *filename, size_t filenameCch );

	size_t GetItemTitle( size_t item, wchar_t *title, size_t titleCch );

	int    GetItemLengthMilliseconds( size_t item ); // TODO: maybe microsecond for better resolution?

	size_t GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch );

	int    Reverse();                                // optional, return 1 to indicate that you did the reversal (otherwise, caller must perform manually)
	int    Swap( size_t item1, size_t item2 );
	int    Randomize( int ( *generator )( ) );       // optional, return 1 to indicate that you did the randomization (otherwise, caller must perform manually)
	void   Remove( size_t item );

	int    SortByTitle();                            // optional, return 1 to indicate that you did the sort (otherwise, caller must perform manually)
	int    SortByFilename();                         // optional, return 1 to indicate that you did the sort (otherwise, caller must perform manually)
	int    SortByDirectory();
};

inline void ifc_playlist::Clear()
{
	_voidcall( IFC_PLAYLIST_CLEAR );
}
/*
inline void ifc_playlist::AppendWithInfo(const wchar_t *filename, const char *title, int lengthInMS)
{
_voidcall(IFC_PLAYLIST_APPENDWITHINFO, filename, title, lengthInMS);
}
*/
/*
inline void ifc_playlist::Append(const wchar_t *filename)
{
_voidcall(IFC_PLAYLIST_APPEND, filename);
}*/

inline size_t ifc_playlist::GetNumItems()
{
	return _call( IFC_PLAYLIST_GETNUMITEMS, (size_t)0 );
}

inline size_t ifc_playlist::GetItem( size_t item, wchar_t *filename, size_t filenameCch )
{
	return _call( IFC_PLAYLIST_GETITEM, (size_t)0, item, filename, filenameCch );
}

inline size_t ifc_playlist::GetItemTitle( size_t item, wchar_t *title, size_t titleCch )
{
	return _call( IFC_PLAYLIST_GETITEMTITLE, (size_t)0, item, title, titleCch );
}

inline int ifc_playlist::GetItemLengthMilliseconds( size_t item )
{
	return _call( IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, (int)-1, item );
}

inline size_t ifc_playlist::GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch )
{
	return _call( IFC_PLAYLIST_GETITEMEXTENDEDINFO, (size_t)0, item, metadata, info, infoCch );
}

inline int ifc_playlist::Reverse()
{
	return _call( IFC_PLAYLIST_REVERSE, (int)PLAYLIST_UNIMPLEMENTED );
}

inline int ifc_playlist::Swap( size_t item1, size_t item2 )
{
	return _call( IFC_PLAYLIST_SWAP, (int)PLAYLIST_UNIMPLEMENTED, item1, item2 );
}

inline int ifc_playlist::Randomize( int ( *generator )( ) )
{
	return _call( IFC_PLAYLIST_RANDOMIZE, (int)PLAYLIST_UNIMPLEMENTED, generator );
}

inline void ifc_playlist::Remove( size_t item )
{
	_voidcall( IFC_PLAYLIST_REMOVE, item );
}

inline int ifc_playlist::SortByTitle()
{
	return _call( IFC_PLAYLIST_SORTBYTITLE, (int)0 );
}

inline int ifc_playlist::SortByFilename()
{
	return _call( IFC_PLAYLIST_SORTBYFILENAME, (int)0 );
}

inline int ifc_playlist::SortByDirectory()
{
	return _call( IFC_PLAYLIST_SORTBYDIRECTORY, (int)0 );
}

#endif