#ifndef NULLSOFT_PLAYLISTS_HANDLER_H
#define NULLSOFT_PLAYLISTS_HANDLER_H

#include "svc_playlisthandler.h"
#include <bfc/platform/types.h>

#define DECLARE_HANDLER(className, IS_WRITER) class className ## Handler : public svc_playlisthandler {\
public:\
	const wchar_t *enumExtensions(size_t n); \
	int SupportedFilename(const wchar_t *filename); \
	ifc_playlistloader *CreateLoader(const wchar_t *filename);\
	void ReleaseLoader(ifc_playlistloader *loader);\
	const wchar_t *GetName(); \
	int HasWriter() { return IS_WRITER; }\
protected:	RECVS_DISPATCH;}

DECLARE_HANDLER(M3U, 1);
DECLARE_HANDLER(PLS, 1);
DECLARE_HANDLER(B4S, 0);

// {8D031378-4209-4bfe-AC94-03C57C896214}
static const GUID m3uHandlerGUID = 
{ 0x8d031378, 0x4209, 0x4bfe, { 0xac, 0x94, 0x3, 0xc5, 0x7c, 0x89, 0x62, 0x14 } };

// {4FF33CC0-F82C-4550-8E4A-DDF90036BE85}
static const GUID plsHandlerGUID = 
{ 0x4ff33cc0, 0xf82c, 0x4550, { 0x8e, 0x4a, 0xdd, 0xf9, 0x0, 0x36, 0xbe, 0x85 } };

static const GUID b4sHandlerGUID = 
{ 0x6f62cbb8, 0x7e1f, 0x43eb, { 0xb3, 0xf6, 0x1, 0xc2, 0x60, 0x10, 0x29, 0xa3 } };

#endif