#ifndef NULLSOFT_PLLOADEX_SIMPLELOADER_H
#define NULLSOFT_PLLOADEX_SIMPLELOADER_H

#include "../playlist/ifc_playlistloader.h"

class SimpleLoader : public ifc_playlistloader
{
public:
	int Load(const wchar_t *filename, ifc_playlistloadercallback *playlist);
protected:
	RECVS_DISPATCH; // all Wasabi objects implementing a Dispatchable interface require this
};

#endif