#include "main.h"
#include "service.h"
#include "navigation.h"
#include "handler.h"
#include "../Agave/URIHandler/svc_urihandler.h"
#include <api/service/waservicefactory.h>
#include "api.h"
#include "../ml_online/config.h"
#include "../replicant/nu/Autowide.h"

int NowPlayingURIHandler::ProcessFilename(const wchar_t *filename)
{
	if (!_wcsnicmp(filename, L"winamp://Now Playing", 20) || !_wcsnicmp(filename, L"winamp://Now%20Playing", 22))
	{
		size_t index = 0;
		if (filename[12] == L' ')
			index = 20;
		else
			index = 22;

		wchar_t fullUrl[1024] = L"http://client.winamp.com/nowplaying";
		lstrcpynW(fullUrl, AutoWide(g_config->ReadString("nowplayingurl", "http://client.winamp.com/nowplaying")), ARRAYSIZE(fullUrl));

		if (filename[index] != 0)
		{
			StringCchCatW(fullUrl, 1024, filename + index);
		}
		Navigation_ShowService(SERVICE_ID, fullUrl, NAVFLAG_FORCEACTIVE | NAVFLAG_ENSUREMLVISIBLE | NAVFLAG_ENSUREITEMVISIBLE);
		return HANDLED_EXCLUSIVE;
	}
	return NOT_HANDLED;
}

int NowPlayingURIHandler::IsMine(const wchar_t *filename)
{
	if (!_wcsnicmp(filename, L"winamp://Now Playing", 20 )  || !_wcsnicmp(filename, L"winamp://Now%20Playing", 22))
		return HANDLED;
	else
		return NOT_HANDLED;
}

#define CBCLASS NowPlayingURIHandler
START_DISPATCH;
CB(PROCESSFILENAME, ProcessFilename);
CB(ISMINE, IsMine);
END_DISPATCH;
#undef CBCLASS