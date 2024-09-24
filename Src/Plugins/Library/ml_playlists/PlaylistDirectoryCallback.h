#include "../playlist/ifc_playlistdirectorycallback.h"

class PlaylistDirectoryCallback : public ifc_playlistdirectorycallback
{
public:
	PlaylistDirectoryCallback(const char *_extlist, const char *winampIni=0);
bool ShouldRecurse(const wchar_t *path);
	bool ShouldLoad(const wchar_t *filename);

	const char *extlist;
	bool recurse;

protected:
	RECVS_DISPATCH;
};
