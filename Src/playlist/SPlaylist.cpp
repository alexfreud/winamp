#include "SPlaylist.h"
#include "api__playlist.h"
#include "PlaylistManager.h"

extern ScriptObjectController *script_root;
extern PlaylistScriptController playlistController;


// -- Functions table -------------------------------------
function_descriptor_struct PlaylistScriptController::exportedFunction[] = {
	{L"Clear",               0, (void*)SPlaylist::script_vcpu_Clear },
	{L"GetNumItems",         0, (void*)SPlaylist::script_vcpu_GetNumItems },
	{L"GetItem",             1, (void*)SPlaylist::script_vcpu_GetItem },
	{L"GetItemTitle",        1, (void*)SPlaylist::script_vcpu_GetItemTitle },
	{L"GetItemLength",       1, (void*)SPlaylist::script_vcpu_GetItemLength },
	{L"GetItemExtendedInfo", 2, (void*)SPlaylist::script_vcpu_GetItemExtendedInfo },
	{L"Reverse",             0, (void*)SPlaylist::script_vcpu_Reverse },
	{L"Swap",                2, (void*)SPlaylist::script_vcpu_Swap },
	{L"Randomize",           0, (void*)SPlaylist::script_vcpu_Randomize },
	{L"Remove",              1, (void*)SPlaylist::script_vcpu_Remove },
	{L"SortByTitle",         0, (void*)SPlaylist::script_vcpu_SortByTitle },
	{L"SortByFilename",      0, (void*)SPlaylist::script_vcpu_SortByFilename },
	/* TODO:
	{L"Append",      3, (void*)SPlaylist::script_vcpu_Append },
	{L"Insert",      4, (void*)SPlaylist::script_vcpu_Insert },
	{L"SetItemFilename",      2, (void*)SPlaylist::script_vcpu_SetItemFilename },
	{L"SetItemTitle",      2, (void*)SPlaylist::script_vcpu_SetItemTitle },
	{L"SetItemLengthMilliseconds",      2, (void*)SPlaylist::script_vcpu_SetItemLengthMilliseconds },
	{L"SortByDirectory",      0, (void*)SPlaylist::script_vcpu_SortByDirectory },
	*/
	
};
// --------------------------------------------------------

const wchar_t *PlaylistScriptController::getClassName()
{
	return L"Playlist";
}

const wchar_t *PlaylistScriptController::getAncestorClassName()
{
	return L"Object";
}

ScriptObjectController *PlaylistScriptController::getAncestorController()
{
	return script_root;
}

ScriptObject *PlaylistScriptController::instantiate()
{
	SPlaylist *xd = new SPlaylist;

	ASSERT( xd != NULL );

	return xd->getScriptObject();
}

void PlaylistScriptController::destroy( ScriptObject *o )
{
	SPlaylist *xd = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );

	ASSERT( xd != NULL );

	delete xd;
}

void *PlaylistScriptController::encapsulate( ScriptObject *o )
{
	return NULL;
}

void PlaylistScriptController::deencapsulate( void *o )
{}

int PlaylistScriptController::getNumFunctions()
{
	return sizeof( exportedFunction ) / sizeof( function_descriptor_struct );
}

const function_descriptor_struct *PlaylistScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID PlaylistScriptController::getClassGuid()
{
	return makiPlaylistGUID;
}


/* ---- */
SPlaylist::SPlaylist()
{
	getScriptObject()->vcpu_setInterface( makiPlaylistGUID, static_cast<SPlaylist *>( this ) );
	getScriptObject()->vcpu_setClassName( L"Playlist" );
	getScriptObject()->vcpu_setController( &playlistController );
}

scriptVar SPlaylist::script_vcpu_Clear( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		playlist->playlist.Clear();
	}
	RETURN_SCRIPT_VOID
}

scriptVar SPlaylist::script_vcpu_GetNumItems( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		size_t count = playlist->playlist.GetNumItems();

		return MAKE_SCRIPT_INT( (int)count );
	}
	RETURN_SCRIPT_ZERO
}

static wchar_t splaylist_string_return[ 1024 ];
static wchar_t *splaylist_string_empty = L"";

scriptVar SPlaylist::script_vcpu_GetItem( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber )
{
	SCRIPT_FUNCTION_INIT;

	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		int    item = GET_SCRIPT_INT( itemNumber );
		size_t cch  = playlist->playlist.GetItem( item, splaylist_string_return, sizeof( splaylist_string_return ) / sizeof( *splaylist_string_return ) );

		if ( cch == 0 )
			return MAKE_SCRIPT_STRING( splaylist_string_empty );
		else
			return MAKE_SCRIPT_STRING( splaylist_string_return );
	}

	return MAKE_SCRIPT_STRING( splaylist_string_empty );
}

scriptVar SPlaylist::script_vcpu_GetItemTitle( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber )
{
	SCRIPT_FUNCTION_INIT;

	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		int    item = GET_SCRIPT_INT( itemNumber );
		size_t cch  = playlist->playlist.GetItemTitle( item, splaylist_string_return, sizeof( splaylist_string_return ) / sizeof( *splaylist_string_return ) );

		if ( cch == 0 )
			return MAKE_SCRIPT_STRING( splaylist_string_empty );
		else
			return MAKE_SCRIPT_STRING( splaylist_string_return );
	}

	return MAKE_SCRIPT_STRING( splaylist_string_empty );
}

scriptVar SPlaylist::script_vcpu_GetItemLength( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber )
{
	SCRIPT_FUNCTION_INIT;

	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		int item   = GET_SCRIPT_INT( itemNumber );
		int length = playlist->playlist.GetItemLengthMilliseconds( item );

		return MAKE_SCRIPT_INT( length );
	}

	return MAKE_SCRIPT_INT( -1 );
}

scriptVar SPlaylist::script_vcpu_GetItemExtendedInfo( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber, scriptVar metadata )
{
	SCRIPT_FUNCTION_INIT;

	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		int            item = GET_SCRIPT_INT( itemNumber );
		const wchar_t *tag  = GET_SCRIPT_STRING( metadata );
		size_t cch = playlist->playlist.GetItemExtendedInfo( item, tag, splaylist_string_return, sizeof( splaylist_string_return ) / sizeof( *splaylist_string_return ) );

		if ( cch == 0 )
			return MAKE_SCRIPT_STRING( splaylist_string_empty );
		else
			return MAKE_SCRIPT_STRING( splaylist_string_return );
	}

	return MAKE_SCRIPT_STRING( splaylist_string_empty );
}

scriptVar SPlaylist::script_vcpu_Reverse( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		playlist->playlist.Reverse();
	}
	RETURN_SCRIPT_VOID
}

scriptVar SPlaylist::script_vcpu_Swap( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar position1, scriptVar position2 )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		int item1 = GET_SCRIPT_INT( position1 );
		int item2 = GET_SCRIPT_INT( position2 );

		playlist->playlist.Swap( item1, item2 );
	}
	RETURN_SCRIPT_VOID
}

scriptVar SPlaylist::script_vcpu_Randomize( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		playlistManager.Randomize( &playlist->playlist );
	}
	RETURN_SCRIPT_VOID
}


scriptVar SPlaylist::script_vcpu_Remove( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemIndex )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		int item = GET_SCRIPT_INT( itemIndex );
		playlist->playlist.Remove( item );
	}
	RETURN_SCRIPT_VOID
}

scriptVar SPlaylist::script_vcpu_SortByTitle( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		playlist->playlist.SortByTitle();
	}
	RETURN_SCRIPT_VOID
}

scriptVar SPlaylist::script_vcpu_SortByFilename( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylist *playlist = static_cast<SPlaylist *>( o->vcpu_getInterface( makiPlaylistGUID ) );
	if ( playlist )
	{
		playlist->playlist.SortByFilename();
	}
	RETURN_SCRIPT_VOID
}
