#ifndef NULLSOFT_XSPF_XSPFHANDLER_H
#define NULLSOFT_XSPF_XSPFHANDLER_H

#include "../playlist/svc_playlisthandler.h"
// the "Playlist Handler" is responsible for describing all the capabilities of the playlist format to Winamp
// It is a singleton class (for each playlist type) 
// besides informational functions, it contains factory methods for creating playlist loaders and writers

class XSPFHandler : public svc_playlisthandler
{
public:
	const wchar_t *EnumerateExtensions(size_t n); // returns 0 when it's done
	const char *EnumerateMIMETypes(size_t n); // returns 0 when it's done, returns char * to match HTTP specs
	const wchar_t *GetName();  // returns a name suitable for display to user of this playlist form (e.g. PLS Playlist)
	int SupportedFilename(const wchar_t *filename); // returns SUCCESS and FAILED, so be careful ...
	int SupportedMIMEType(const char *filename); // returns SUCCESS and FAILED, so be careful ...
	ifc_playlistloader *CreateLoader(const wchar_t *filename);
	void ReleaseLoader(ifc_playlistloader *loader);

protected:
	RECVS_DISPATCH; // boiler-plate code for the dispatch-table
};
#endif