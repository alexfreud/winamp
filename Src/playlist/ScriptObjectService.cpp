#include "ScriptObjectService.h"
#include <api/script/objects/rootobjcontroller.h>
#include "SPlaylist.h"
#include "SPlaylists.h"
#include "SPlaylistsEnumerator.h"
#include "SPlaylistManager.h"

ScriptObjectController *script_root=0;
PlaylistScriptController playlistController;
PlaylistsScriptController playlistsController;
PlaylistsEnumeratorScriptController playlistsEnumeratorController;
PlaylistManagerScriptController playlistManagerController;

ScriptObjectController *ScriptObjectService::getController(int n)
{
	switch (n)
	{
	case 0:
		return &playlistController;
	case 1:
		return &playlistsController;
	case 2:
		return &playlistsEnumeratorController;
	case 3:
		return &playlistManagerController;
	}

	return 0;
}


void ScriptObjectService::onRegisterClasses(ScriptObjectController *rootController)
{
	script_root = rootController;
}

#define CBCLASS ScriptObjectService
START_DISPATCH;
  CB(GETCONTROLLER, getController);
  VCB(ONREGISTER,   onRegisterClasses);
END_DISPATCH;
#undef CBCLASS
