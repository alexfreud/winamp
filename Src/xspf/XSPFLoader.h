#ifndef NULLSOFT_XSPF_XSPFLOADER_H
#define NULLSOFT_XSPF_XSPFLOADER_H

#include "../playlist/ifc_playlistloader.h"

// this is the class that actually loads the playlist.  
// everything up to this point (component, handler, handler factory) was just administrative
class XSPFLoader : public ifc_playlistloader
{
public:
	int Load(const wchar_t *filename, ifc_playlistloadercallback *playlist);

protected:
	RECVS_DISPATCH; // boiler-plate code for our dispatch table
};
#endif