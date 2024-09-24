#include "SPlaylists.h"
#include "api__playlist.h"
#include "Playlists.h"
#include "SPlaylistsEnumerator.h"
#include "SPlaylist.h"
#include "PlaylistManager.h"

extern Playlists playlists;

extern ScriptObjectController *script_root;
extern PlaylistsScriptController playlistsController;

// {5829EE15-3648-4c6e-B2FE-8736CBBF39DB}
static const GUID makiPlaylistsGUID = 
{ 0x5829ee15, 0x3648, 0x4c6e, { 0xb2, 0xfe, 0x87, 0x36, 0xcb, 0xbf, 0x39, 0xdb } };

// -- Functions table -------------------------------------
function_descriptor_struct PlaylistsScriptController::exportedFunction[] = {
	{L"GetEnumerator",               0, (void*)SPlaylists::script_vcpu_GetEnumerator },
	{L"OpenPlaylist",               1, (void*)SPlaylists::script_vcpu_OpenPlaylist },
	{L"SavePlaylist",               2, (void*)SPlaylists::script_vcpu_SavePlaylist },
};
// --------------------------------------------------------

const wchar_t *PlaylistsScriptController::getClassName()
{
	return L"Playlists";
}

const wchar_t *PlaylistsScriptController::getAncestorClassName()
{
  return L"Object";
}

ScriptObjectController *PlaylistsScriptController::getAncestorController() 
{
	return script_root; 
}

int PlaylistsScriptController::getInstantiable()
{
	return 0;
}

int PlaylistsScriptController::getReferenceable() 
{
	return 0;
}

ScriptObject *PlaylistsScriptController::instantiate()
{
  SPlaylists *xd = new SPlaylists;
  ASSERT(xd != NULL);
  return xd->getScriptObject();
}

void PlaylistsScriptController::destroy(ScriptObject *o)
{
  SPlaylists *xd = static_cast<SPlaylists *>(o->vcpu_getInterface(makiPlaylistsGUID));
  ASSERT(xd != NULL);
  delete xd;
}

void *PlaylistsScriptController::encapsulate(ScriptObject *o)
{
  return NULL; 
}

void PlaylistsScriptController::deencapsulate(void *o)
{}

int PlaylistsScriptController::getNumFunctions()
{
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *PlaylistsScriptController::getExportedFunctions()
{
  return exportedFunction;                                                        
}

GUID PlaylistsScriptController::getClassGuid()
{
	return makiPlaylistsGUID;
}

/* --- */

SPlaylists::SPlaylists()
{
	getScriptObject()->vcpu_setInterface(makiPlaylistsGUID, static_cast<SPlaylists *>(this));
	getScriptObject()->vcpu_setClassName(L"Playlists");
	getScriptObject()->vcpu_setController(&playlistsController);
}

scriptVar SPlaylists::script_vcpu_GetEnumerator(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) 
{
	SCRIPT_FUNCTION_INIT;
	SPlaylistsEnumerator *enumerator = new SPlaylistsEnumerator();
	playlists.Lock();
	size_t count = playlists.GetCount();
	enumerator->Reserve(count);
	for (size_t i=0;i!=count;i++)
	{
		const PlaylistInfo &info = playlists.GetPlaylistInfo(i);
		enumerator->AppendPlaylist(info);
	}
	playlists.Unlock();
	return MAKE_SCRIPT_OBJECT(enumerator->getScriptObject());
}

scriptVar SPlaylists::script_vcpu_OpenPlaylist( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar scriptPlaylistGUID )
{
	SCRIPT_FUNCTION_INIT;
	GUID playlist_guid = nsGUID::fromCharW( GET_SCRIPT_STRING( scriptPlaylistGUID ) );
	SPlaylist *playlist = 0;

	playlists.Lock();
	size_t index;
	if ( playlists.GetPosition( playlist_guid, &index ) == API_PLAYLISTS_SUCCESS )
	{
		const wchar_t *filename = playlists.GetFilename( index );
		if ( filename )
		{
			playlist = new SPlaylist;
			if ( playlistManager.Load( filename, &playlist->playlist ) != PLAYLISTMANAGER_SUCCESS )
			{
				delete playlist;
				playlist = 0;
			}
		}
	}
	playlists.Unlock();

	return MAKE_SCRIPT_OBJECT( playlist ? playlist->getScriptObject() : 0 );
}

scriptVar SPlaylists::script_vcpu_SavePlaylist( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar scriptPlaylistGUID, scriptVar scriptPlaylist )
{
	SCRIPT_FUNCTION_INIT;
	GUID playlist_guid = nsGUID::fromCharW( GET_SCRIPT_STRING( scriptPlaylistGUID ) );
	SPlaylist *playlist = static_cast<SPlaylist *>( GET_SCRIPT_OBJECT_AS( scriptPlaylist, makiPlaylistGUID ) );
	if ( playlist )
	{
		playlists.Lock();
		size_t index;
		if ( playlists.GetPosition( playlist_guid, &index ) == API_PLAYLISTS_SUCCESS )
		{
			const wchar_t *filename = playlists.GetFilename( index );
			if ( filename )
				playlistManager.Save( filename, &playlist->playlist );
		}
		playlists.Unlock();
	}

	return MAKE_SCRIPT_OBJECT( playlist ? playlist->getScriptObject() : 0 );
}
