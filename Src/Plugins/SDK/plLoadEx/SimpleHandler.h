#ifndef NULLSOFT_PLLOADEX_SIMPLEHANDLER_H
#define NULLSOFT_PLLOADEX_SIMPLEHANDLER_H

#include "../playlist/svc_playlisthandler.h"

class Cef_Handler : public svc_playlisthandler
{
public:
	const wchar_t *EnumerateExtensions(size_t n); // returns 0 when it's done
	const wchar_t *GetName();  // returns a name suitable for display to user of this playlist form (e.g. PLS Playlist)
	int SupportedFilename(const wchar_t *filename); // returns SUCCESS and FAILED, so be careful ...
	ifc_playlistloader *CreateLoader(const wchar_t *filename);
	void ReleaseLoader(ifc_playlistloader *loader);
	// there are a few more functions, but we're not going to implement them because we don't need to do, and the Dispatchable interface
	// provides smart default return values

protected:
	RECVS_DISPATCH; // all Wasabi objects implementing a Dispatchable interface require this
};

#endif