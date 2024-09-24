#ifndef NULLSOFT_SVC_PLAYLISTHANDLER_H
#define NULLSOFT_SVC_PLAYLISTHANDLER_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include "ifc_playlistloader.h"
#include <stdint.h>

enum
{
	SVC_PLAYLISTHANDLER_SUCCESS = 0,
	SVC_PLAYLISTHANDLER_FAILED = 1,
};
class svc_playlisthandler : public Dispatchable
{
protected:
	svc_playlisthandler() {}
	~svc_playlisthandler() {}

public:
	static FOURCC getServiceType() { return svc_playlisthandler::SERVICETYPE; }
	const wchar_t *EnumerateExtensions(size_t n); // returns 0 when it's done
	const char *EnumerateMIMETypes(size_t n); // returns 0 when it's done, returns char * to match HTTP specs
	const wchar_t *GetName();  // returns a name suitable for display to user of this playlist form (e.g. PLS Playlist)
	int SupportedFilename(const wchar_t *filename); // returns SUCCESS and FAILED, so be careful ...
	int SupportedMIMEType(const char *filename); // returns SUCCESS and FAILED, so be careful ...
	ifc_playlistloader *CreateLoader(const wchar_t *filename);
	void ReleaseLoader(ifc_playlistloader *loader);
	int HasWriter(); // returns 1 if writing is supported
	//ifc_playlistwriter CreateWriter(const wchar_t *writer);
	//void ReleaseWriter(ifc_playlistwriter *writer);

	size_t SniffSizeRequired(); // return number of bytes required for detection on an unknown file
	bool IsOurs(const int8_t *data, size_t sizeBytes);

public:
	DISPATCH_CODES
	{
		SVC_PLAYLISTHANDLER_ENUMEXTENSIONS = 10,
		SVC_PLAYLISTHANDLER_ENUMMIMETYPES = 20,
		SVC_PLAYLISTHANDLER_SUPPORTFILENAME= 30,
		SVC_PLAYLISTHANDLER_SUPPORTMIME= 40,
		SVC_PLAYLISTHANDLER_CREATELOADER = 50,
		SVC_PLAYLISTHANDLER_RELEASELOADER = 60,
		SVC_PLAYLISTHANDLER_CREATEWRITER= 70,
		SVC_PLAYLISTHANDLER_RELEASEWRITER= 80,
		SVC_PLAYLISTHANDLER_SNIFFSIZE=90,
		SVC_PLAYLISTHANDLER_SNIFF=100,
		SVC_PLAYLISTHANDLER_GETNAME=110,
		SVC_PLAYLISTHANDLER_HASWRITER=120,
	};

	enum
	{
		SERVICETYPE = MK3CC('p','l','h')
	};

};

inline const wchar_t *svc_playlisthandler::GetName()
{
	return _call(SVC_PLAYLISTHANDLER_GETNAME, (const wchar_t *)0);
}

inline const wchar_t *svc_playlisthandler::EnumerateExtensions(size_t n)
{
	return _call(SVC_PLAYLISTHANDLER_ENUMEXTENSIONS, (const wchar_t *)0, n);
};

inline const char *svc_playlisthandler::EnumerateMIMETypes(size_t n)
{
	return _call(SVC_PLAYLISTHANDLER_ENUMMIMETYPES, (const char *)0, n);
}

inline int svc_playlisthandler::SupportedFilename(const wchar_t *filename)
{
	return _call(SVC_PLAYLISTHANDLER_SUPPORTFILENAME, (int)SVC_PLAYLISTHANDLER_FAILED, filename);
}

inline int svc_playlisthandler::SupportedMIMEType(const char *filename)
{
	return _call(SVC_PLAYLISTHANDLER_SUPPORTMIME, (int)SVC_PLAYLISTHANDLER_FAILED, filename);
}

inline ifc_playlistloader *svc_playlisthandler::CreateLoader(const wchar_t *filename)
{
	return _call(SVC_PLAYLISTHANDLER_CREATELOADER, (ifc_playlistloader *)0, filename);
}

inline void svc_playlisthandler::ReleaseLoader(ifc_playlistloader *loader)
{
	_voidcall(SVC_PLAYLISTHANDLER_RELEASELOADER, loader);
}

inline size_t svc_playlisthandler::SniffSizeRequired()
{
	return _call(SVC_PLAYLISTHANDLER_SNIFFSIZE, (size_t)0);
}

inline bool svc_playlisthandler::IsOurs(const int8_t *data, size_t sizeBytes)
{
	return _call(SVC_PLAYLISTHANDLER_SNIFF, (bool)false, data, sizeBytes);
}

inline int svc_playlisthandler::HasWriter()
{
	return _call(SVC_PLAYLISTHANDLER_HASWRITER, (int)0);
}

#endif