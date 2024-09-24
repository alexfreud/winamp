#include "SPlaylistManager.h"
#include "SPlaylist.h"
#include "api__playlist.h"
#include "PlaylistManager.h"

extern ScriptObjectController *script_root;
extern PlaylistManagerScriptController playlistManagerController;

// {C6207729-2600-4bb8-B562-2E0BC04E4416}
static const GUID makePlaylistManagerGUID = 
{ 0xc6207729, 0x2600, 0x4bb8, { 0xb5, 0x62, 0x2e, 0xb, 0xc0, 0x4e, 0x44, 0x16 } };


// -- Functions table -------------------------------------
function_descriptor_struct PlaylistManagerScriptController::exportedFunction[] = {
	{L"OpenPlaylist", 1, (void*)SPlaylistManager::script_vcpu_OpenPlaylist },
	{L"SavePlaylist", 2, (void*)SPlaylistManager::script_vcpu_SavePlaylist },
};
// --------------------------------------------------------

const wchar_t *PlaylistManagerScriptController::getClassName() {
	return L"PlaylistManager";
}

const wchar_t *PlaylistManagerScriptController::getAncestorClassName() {
	return L"Object";
}

ScriptObjectController *PlaylistManagerScriptController::getAncestorController() 
{
	return script_root; 
}

int PlaylistManagerScriptController::getInstantiable()
{
	return 0;
}

int PlaylistManagerScriptController::getReferenceable() 
{
	return 0;
}

ScriptObject *PlaylistManagerScriptController::instantiate() {
  SPlaylistManager *xd = new SPlaylistManager;
  ASSERT(xd != NULL);
  return xd->getScriptObject();
}

void PlaylistManagerScriptController::destroy(ScriptObject *o) {
  SPlaylistManager *xd = static_cast<SPlaylistManager *>(o->vcpu_getInterface(makePlaylistManagerGUID));
  ASSERT(xd != NULL);
  delete xd;
}

void *PlaylistManagerScriptController::encapsulate(ScriptObject *o) {
  return NULL; 
}

void PlaylistManagerScriptController::deencapsulate(void *o) {
}

int PlaylistManagerScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *PlaylistManagerScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID PlaylistManagerScriptController::getClassGuid() {
	return makePlaylistManagerGUID;
}

/* --- */

SPlaylistManager::SPlaylistManager()
{
	getScriptObject()->vcpu_setInterface(makePlaylistManagerGUID, static_cast<SPlaylistManager *>(this));
	getScriptObject()->vcpu_setClassName(L"PlaylistManager");
	getScriptObject()->vcpu_setController(&playlistManagerController);
}

scriptVar SPlaylistManager::script_vcpu_OpenPlaylist(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar scriptFilename)
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *filename = GET_SCRIPT_STRING(scriptFilename);
	if (filename && *filename)
	{
		SPlaylist *playlist = new SPlaylist;
		if (playlistManager.Load(filename, &playlist->playlist) == PLAYLISTMANAGER_SUCCESS)
		{
			return MAKE_SCRIPT_OBJECT(playlist->getScriptObject());
		}
		else
		{
			delete playlist;
			return MAKE_SCRIPT_OBJECT(0);
		}
	}

	return MAKE_SCRIPT_OBJECT(0);
}

scriptVar SPlaylistManager::script_vcpu_SavePlaylist(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar scriptFilename, scriptVar scriptPlaylist)
{
	SCRIPT_FUNCTION_INIT;
	const wchar_t *filename = GET_SCRIPT_STRING(scriptFilename);
	SPlaylist *playlist =  static_cast<SPlaylist *>(GET_SCRIPT_OBJECT_AS(scriptPlaylist, makiPlaylistGUID));
	if (filename && *filename && playlist)
	{
		playlistManager.Save(filename, &playlist->playlist);
	}

	RETURN_SCRIPT_VOID;
}

