#include "Handler.h"
#include "resource.h"
#include "M3ULoader.h"
#include "PLSLoader.h"
#include "B4SLoader.h"
#include <shlwapi.h>

void GetExtension(const wchar_t *filename, wchar_t *ext, size_t extCch)
{
	const wchar_t *s = PathFindExtensionW(filename);
	if (!PathIsURLW(filename)
		|| (!wcsstr(s, L"?") && !wcsstr(s, L"&") && !wcsstr(s, L"=") && *s))
	{
		lstrcpynW(ext, s, (int)extCch);
		return ;
	}
	// s is not a terribly good extension, let's try again
	{
		wchar_t *copy = _wcsdup(filename);
		s = L"";
	again:
		{
			wchar_t *p = StrRChrW(copy,NULL, L'?');
			if (p)
			{
				*p = 0;
				s = PathFindExtensionW(copy);
				if (!*s) goto again;
			}
			lstrcpynW(ext, s, (int)extCch);
		}
		free(copy);
	}
}

const wchar_t *M3UHandler::enumExtensions(size_t n)
{
	switch(n)
	{
	case 0:
		return L"M3U";
	case 1:
		return L"M3U8";
	default:
		return 0;
	}
}

int M3UHandler::SupportedFilename(const wchar_t *filename)
{
	wchar_t ext[16] = {0};
	GetExtension(filename, ext, 16);

	if (!lstrcmpiW(ext, L".M3U"))
		return SVC_PLAYLISTHANDLER_SUCCESS;
	if (!lstrcmpiW(ext, L".M3U8"))
		return SVC_PLAYLISTHANDLER_SUCCESS;

	return SVC_PLAYLISTHANDLER_FAILED;
}

ifc_playlistloader *M3UHandler::CreateLoader(const wchar_t *filename)
{
	return new M3ULoader;
}

void M3UHandler::ReleaseLoader(ifc_playlistloader *loader)
{
	M3ULoader *m3u;
	
	m3u = static_cast<M3ULoader *>(loader);
	delete m3u;
}

const wchar_t *M3UHandler::GetName()
{
	static wchar_t m3upl[64];
	// no point re-loading this all of the time since it won't change once we've been loaded
	return (!m3upl[0]?WASABI_API_LNGSTRINGW_BUF(IDS_M3U_PLAYLIST,m3upl,64):m3upl);
}
/* ----------------------------------------------------- */

const wchar_t *PLSHandler::enumExtensions(size_t n)
{
	switch(n)
	{
	case 0:
		return L"PLS";
	default:
		return 0;
	}
}

int PLSHandler::SupportedFilename(const wchar_t *filename)
{
	wchar_t ext[16] = {0};
	GetExtension(filename, ext, 16);

	if (!lstrcmpiW(ext, L".PLS"))
		return SVC_PLAYLISTHANDLER_SUCCESS;

	// TODO: open file and sniff it for file signature
	return SVC_PLAYLISTHANDLER_FAILED;
}

ifc_playlistloader *PLSHandler::CreateLoader(const wchar_t *filename)
{
	return new PLSLoader;
}

void PLSHandler::ReleaseLoader(ifc_playlistloader *loader)
{
	PLSLoader *pls;
	
	pls = static_cast<PLSLoader *>(loader);
	delete pls;
}


const wchar_t *PLSHandler::GetName()
{
	static wchar_t plspl[64];
	// no point re-loading this all of the time since it won't change once we've been loaded
	return (!plspl[0]?WASABI_API_LNGSTRINGW_BUF(IDS_PLS_PLAYLIST,plspl,64):plspl);
}

/* --- B4S --- */
const wchar_t *B4SHandler::enumExtensions(size_t n)
{
	switch(n)
	{
	case 0:
		return L"B4S";
		//case 1:
		//return L"BPL";
	default:
		return 0;
	}
}

int B4SHandler::SupportedFilename(const wchar_t *filename)
{
	wchar_t ext[16] = {0};
	GetExtension(filename, ext, 16);

	if (!lstrcmpiW(ext, L".B4S"))
		return SVC_PLAYLISTHANDLER_SUCCESS;
	if (!lstrcmpiW(ext, L".BPL"))
		return SVC_PLAYLISTHANDLER_SUCCESS;

	// TODO: open file and sniff it for file signature
	return SVC_PLAYLISTHANDLER_FAILED;
}

ifc_playlistloader *B4SHandler::CreateLoader(const wchar_t *filename)
{
	return new B4SLoader;
}

void B4SHandler::ReleaseLoader(ifc_playlistloader *loader)
{
	B4SLoader *pls;
	
	pls = static_cast<B4SLoader *>(loader);
	delete pls;
}

const wchar_t *B4SHandler::GetName()
{
	static wchar_t wa3pl[64];
	// no point re-loading this all of the time since it won't change once we've been loaded
	return (!wa3pl[0]?WASABI_API_LNGSTRINGW_BUF(IDS_WINAMP3_PLAYLIST,wa3pl,64):wa3pl);
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS M3UHandler
START_DISPATCH;
CB(SVC_PLAYLISTHANDLER_ENUMEXTENSIONS, enumExtensions)
CB(SVC_PLAYLISTHANDLER_SUPPORTFILENAME, SupportedFilename)
CB(SVC_PLAYLISTHANDLER_CREATELOADER, CreateLoader)
VCB(SVC_PLAYLISTHANDLER_RELEASELOADER, ReleaseLoader)
CB(SVC_PLAYLISTHANDLER_GETNAME, GetName)
CB(SVC_PLAYLISTHANDLER_HASWRITER, HasWriter)
END_DISPATCH;
#undef CBCLASS

#define CBCLASS PLSHandler
START_DISPATCH;
CB(SVC_PLAYLISTHANDLER_ENUMEXTENSIONS, enumExtensions)
CB(SVC_PLAYLISTHANDLER_SUPPORTFILENAME, SupportedFilename)
CB(SVC_PLAYLISTHANDLER_CREATELOADER, CreateLoader)
VCB(SVC_PLAYLISTHANDLER_RELEASELOADER, ReleaseLoader)
CB(SVC_PLAYLISTHANDLER_GETNAME, GetName)
CB(SVC_PLAYLISTHANDLER_HASWRITER, HasWriter)
END_DISPATCH;
#undef CBCLASS

#define CBCLASS B4SHandler
START_DISPATCH;
CB(SVC_PLAYLISTHANDLER_ENUMEXTENSIONS, enumExtensions)
CB(SVC_PLAYLISTHANDLER_SUPPORTFILENAME, SupportedFilename)
CB(SVC_PLAYLISTHANDLER_CREATELOADER, CreateLoader)
VCB(SVC_PLAYLISTHANDLER_RELEASELOADER, ReleaseLoader)
CB(SVC_PLAYLISTHANDLER_GETNAME, GetName)
CB(SVC_PLAYLISTHANDLER_HASWRITER, HasWriter)
END_DISPATCH;