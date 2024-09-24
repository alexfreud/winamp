#include "main.h"
#include "PlaylistDirectoryCallback.h"

PlaylistDirectoryCallback::PlaylistDirectoryCallback(const char *_extlist, const char *winampIni)
		: extlist(_extlist), recurse(true)
{
	if (winampIni)
	{
		int rofiob = GetPrivateProfileIntA("winamp", "rofiob", 1, winampIni);
		recurse = (rofiob & 2) ? false : true;
	}
}

bool PlaylistDirectoryCallback::ShouldRecurse(const wchar_t *path)
{
	return recurse;
}

bool PlaylistDirectoryCallback::ShouldLoad(const wchar_t *filename)
{
	const wchar_t *ext = PathFindExtensionW(filename);
	if (!*ext)
		return false;

	ext++;

	const char *a = extlist;
	while (a && *a)
	{
		if (!lstrcmpiW(AutoWide(a), ext))
			return true;
		a += lstrlenA(a) + 1;
	}
	return false;
}

#define CBCLASS PlaylistDirectoryCallback
START_DISPATCH;
CB( IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDRECURSE, ShouldRecurse )
CB( IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDLOAD, ShouldLoad )
END_DISPATCH;
