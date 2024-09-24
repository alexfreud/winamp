#ifndef NULLSOFT_IFC_PLAYLIST_ENTRY_H
#define NULLSOFT_IFC_PLAYLIST_ENTRY_H

#include <bfc/dispatch.h>

class ifc_playlistentry : public Dispatchable
{
protected:
	ifc_playlistentry()                                               {}
	~ifc_playlistentry()                                              {}

	
public:
	DISPATCH_CODES
	{
		IFC_PLAYLISTENTRY_GETFILENAME     = 10,
		IFC_PLAYLISTENTRY_GETTITLE        = 20,
		IFC_PLAYLISTENTRY_GETLENGTHMS     = 30,
		IFC_PLAYLISTENTRY_GETEXTENDEDINFO = 40,
	};


	size_t GetFilename( wchar_t *filename, size_t filenameCch );
	size_t GetTitle( wchar_t *title, size_t titleCch );
	int    GetLengthInMilliseconds();
	size_t GetExtendedInfo( const wchar_t *metadata, wchar_t *info, size_t infoCch );
};

inline size_t ifc_playlistentry::GetFilename( wchar_t *filename, size_t filenameCch )
{
	return _call( IFC_PLAYLISTENTRY_GETFILENAME, (size_t)0, filename, filenameCch );
}

inline size_t ifc_playlistentry::GetTitle( wchar_t *title, size_t titleCch )
{
	return _call( IFC_PLAYLISTENTRY_GETTITLE, (size_t)0, title, titleCch );
}

inline int ifc_playlistentry::GetLengthInMilliseconds()
{
	return _call( IFC_PLAYLISTENTRY_GETLENGTHMS, (int)-1 );
}

inline size_t ifc_playlistentry::GetExtendedInfo( const wchar_t *metadata, wchar_t *info, size_t infoCch )
{
	return _call( IFC_PLAYLISTENTRY_GETEXTENDEDINFO, (size_t)0, metadata, info, infoCch );
}

#endif