#include "main.h"
#include "config.h"
#include "PlaylistHandler.h"
#include "WPLLoader.h"
#include "ASXLoader.h"
#include <shlwapi.h>
#include "resource.h"

void GetExtension(const wchar_t *filename, wchar_t *ext, size_t extCch)
{
	const wchar_t *s = PathFindExtensionW(filename);
	if (!PathIsURLW(filename)
		|| (!wcsstr(s, L"?") && !wcsstr(s, L"&") && !wcsstr(s, L"=") && *s))
	{
		lstrcpynW(ext, s, extCch);
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
			lstrcpynW(ext, s, extCch);
		}
		free(copy);
	}
}

/* ---------------------------------- WPL --------------------------------------- */
const wchar_t *WPLHandler::enumExtensions(size_t n)
{
	switch(n)
	{
	case 0:
		return L"WPL";
	/*case 1:
		return L"ZPL";*/
	default:
		return 0;
	}
}

int WPLHandler::SupportedFilename(const wchar_t *filename)
{
	wchar_t ext[16] = {0};
	GetExtension(filename, ext, 16);

	if (!lstrcmpiW(ext, L".WPL") || !lstrcmpiW(ext, L".ZPL"))
		return SVC_PLAYLISTHANDLER_SUCCESS;

	// TODO: open file and sniff it for file signature
	return SVC_PLAYLISTHANDLER_FAILED;
}

ifc_playlistloader *WPLHandler::CreateLoader(const wchar_t *filename)
{
	return new WPLLoader;
}

void WPLHandler::ReleaseLoader(ifc_playlistloader *loader)
{
	WPLLoader *pls;
	
	pls = static_cast<WPLLoader *>(loader);
	delete pls;
}

const wchar_t *WPLHandler::GetName()
{
	static wchar_t wplpl[64];
	// no point re-loading this all of the time since it won't change once we've been loaded
	return (!wplpl[0]?WASABI_API_LNGSTRINGW_BUF(IDS_WINDOWS_MEDIA_PLAYLIST,wplpl,64):wplpl);
}

/* --- ASX --- */
const wchar_t *ASXHandler::enumExtensions(size_t n)
{
	
	switch(n)
	{
	case 0:
		return L"ASX";
	case 1:
		if (config_extra_asx_extensions)
			return L"WAX";
		else
			return 0;
	case 2:
		if (config_extra_asx_extensions)
		return L"WMX";
		else
			return 0;
	case 3:
		if (config_extra_asx_extensions)
		return L"WVX";
		else
			return 0;
	default:
		return 0;
	}
}

int ASXHandler::SupportedFilename(const wchar_t *filename)
{
	wchar_t ext[16] = {0};
	GetExtension(filename, ext, 16);

	if (!lstrcmpiW(ext, L".ASX"))
		return SVC_PLAYLISTHANDLER_SUCCESS;
	if (!lstrcmpiW(ext, L".WAX"))
		return SVC_PLAYLISTHANDLER_SUCCESS;
	if (!lstrcmpiW(ext, L".WMX"))
		return SVC_PLAYLISTHANDLER_SUCCESS;
	if (!lstrcmpiW(ext, L".WVX"))
		return SVC_PLAYLISTHANDLER_SUCCESS;

	// TODO: open file and sniff it for file signature
	return SVC_PLAYLISTHANDLER_FAILED;
}

ifc_playlistloader *ASXHandler::CreateLoader(const wchar_t *filename)
{
	return new ASXLoader;
}

void ASXHandler::ReleaseLoader(ifc_playlistloader *loader)
{
	ASXLoader *pls;
	
	pls = static_cast<ASXLoader *>(loader);
	delete pls;
}

const wchar_t *ASXHandler::GetName()
{
	static wchar_t asxpl[64];
	// no point re-loading this all of the time since it won't change once we've been loaded
	return (!asxpl[0]?WASABI_API_LNGSTRINGW_BUF(IDS_ASX_PLAYLIST,asxpl,64):asxpl);
}

#define CBCLASS WPLHandler
START_DISPATCH;
CB(SVC_PLAYLISTHANDLER_ENUMEXTENSIONS, enumExtensions)
CB(SVC_PLAYLISTHANDLER_SUPPORTFILENAME, SupportedFilename)
CB(SVC_PLAYLISTHANDLER_CREATELOADER, CreateLoader)
VCB(SVC_PLAYLISTHANDLER_RELEASELOADER, ReleaseLoader)
CB(SVC_PLAYLISTHANDLER_GETNAME, GetName)
END_DISPATCH;
#undef CBCLASS

#define CBCLASS ASXHandler
START_DISPATCH;
CB(SVC_PLAYLISTHANDLER_ENUMEXTENSIONS, enumExtensions)
CB(SVC_PLAYLISTHANDLER_SUPPORTFILENAME, SupportedFilename)
CB(SVC_PLAYLISTHANDLER_CREATELOADER, CreateLoader)
VCB(SVC_PLAYLISTHANDLER_RELEASELOADER, ReleaseLoader)
CB(SVC_PLAYLISTHANDLER_GETNAME, GetName)
END_DISPATCH;
#undef CBCLASS