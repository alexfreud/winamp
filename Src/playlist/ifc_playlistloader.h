#ifndef NULLSOFT_IFC_PLAYLISTLOADER_H
#define NULLSOFT_IFC_PLAYLISTLOADER_H

#include <bfc/dispatch.h>
#include <wchar.h>
#include "ifc_playlistloadercallback.h"

enum
{
	IFC_PLAYLISTLOADER_SUCCESS      = 0,
	IFC_PLAYLISTLOADER_FAILED       = 1,

	IFC_PLAYLISTLOADER_NEXTITEM_EOF = 1,
};

class ifc_playlistloader : public Dispatchable
{
protected:
	ifc_playlistloader()                                              {}
	~ifc_playlistloader()                                             {}

public:
	int Load( const wchar_t *filename, ifc_playlistloadercallback *playlist );

	DISPATCH_CODES
	{
		IFC_PLAYLISTLOADER_LOAD = 10,
	};

};

inline int ifc_playlistloader::Load( const wchar_t *filename, ifc_playlistloadercallback *playlist )
{
	return _call( IFC_PLAYLISTLOADER_LOAD, (int)IFC_PLAYLISTLOADER_FAILED, filename, playlist );
}

#endif
