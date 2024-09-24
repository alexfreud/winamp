#include "SPlaylistsEnumerator.h"
#include "api__playlist.h"
#include <bfc/nsguid.h>
#include "Playlists.h"

extern ScriptObjectController *script_root;
extern PlaylistsEnumeratorScriptController playlistsEnumeratorController;

// {C18F8E50-2C81-4001-9F46-FD942B07ECCD}
static const GUID makiPlaylistsEnumeratorGUID = 
{ 0xc18f8e50, 0x2c81, 0x4001, { 0x9f, 0x46, 0xfd, 0x94, 0x2b, 0x7, 0xec, 0xcd } };

// -- Functions table -------------------------------------
function_descriptor_struct PlaylistsEnumeratorScriptController::exportedFunction[] = {
	{L"GetCount",    0, (void *)SPlaylistsEnumerator::script_vcpu_GetCount },
	{L"GetFilename", 1, (void *)SPlaylistsEnumerator::script_vcpu_GetFilename },
	{L"GetTitle",    1, (void *)SPlaylistsEnumerator::script_vcpu_GetTitle },
	{L"GetLength",   1, (void *)SPlaylistsEnumerator::script_vcpu_GetLength },
	{L"GetNumItems", 1, (void *)SPlaylistsEnumerator::script_vcpu_GetNumItems },
	{L"GetGUID",     1, (void *)SPlaylistsEnumerator::script_vcpu_GetGUID },

};
// --------------------------------------------------------

const wchar_t *PlaylistsEnumeratorScriptController::getClassName()
{
	return L"PlaylistsEnumerator";
}

const wchar_t *PlaylistsEnumeratorScriptController::getAncestorClassName()
{
	return L"Object";
}

ScriptObjectController *PlaylistsEnumeratorScriptController::getAncestorController() 
{
	return script_root; 
}

ScriptObject *PlaylistsEnumeratorScriptController::instantiate()
{
	SPlaylistsEnumerator *xd = new SPlaylistsEnumerator;
	ASSERT(xd != NULL);
	return xd->getScriptObject();
}

void PlaylistsEnumeratorScriptController::destroy( ScriptObject *o )
{
	SPlaylistsEnumerator *xd = static_cast<SPlaylistsEnumerator *>( o->vcpu_getInterface( makiPlaylistsEnumeratorGUID ) );
	ASSERT( xd != NULL );
	delete xd;
}

void *PlaylistsEnumeratorScriptController::encapsulate( ScriptObject *o )
{
	return NULL;
}

void PlaylistsEnumeratorScriptController::deencapsulate( void *o )
{}

int PlaylistsEnumeratorScriptController::getNumFunctions()
{
	return sizeof( exportedFunction ) / sizeof( function_descriptor_struct );
}

const function_descriptor_struct *PlaylistsEnumeratorScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID PlaylistsEnumeratorScriptController::getClassGuid()
{
	return makiPlaylistsEnumeratorGUID;
}

/* --- */

SPlaylistsEnumerator::SPlaylistsEnumerator()
{
	getScriptObject()->vcpu_setInterface( makiPlaylistsEnumeratorGUID, static_cast<SPlaylistsEnumerator *>( this ) );
	getScriptObject()->vcpu_setClassName( L"PlaylistsEnumerator" );
	getScriptObject()->vcpu_setController( &playlistsEnumeratorController );
}

SPlaylistsEnumerator::~SPlaylistsEnumerator()
{
	//info.deleteAll();
	for (auto obj : info)
	{
		delete obj;
	}
	info.clear();
}

scriptVar SPlaylistsEnumerator::script_vcpu_GetCount( SCRIPT_FUNCTION_PARAMS, ScriptObject *o )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylistsEnumerator *enumerator = static_cast<SPlaylistsEnumerator *>( o->vcpu_getInterface( makiPlaylistsEnumeratorGUID ) );
	if ( enumerator )
	{
		return MAKE_SCRIPT_INT( (int)enumerator->info.size() );
	}

	return MAKE_SCRIPT_INT( 0 );
}

static const wchar_t *static_splaylistsenumerator_empty_string = L"";
scriptVar SPlaylistsEnumerator::script_vcpu_GetFilename( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar playlistNumber )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylistsEnumerator *enumerator = static_cast<SPlaylistsEnumerator *>( o->vcpu_getInterface( makiPlaylistsEnumeratorGUID ) );
	size_t i = GET_SCRIPT_INT( playlistNumber );
	if ( enumerator && i < enumerator->info.size() )
	{
		return MAKE_SCRIPT_STRING( enumerator->info[ i ]->filename );
	}

	return MAKE_SCRIPT_STRING( static_splaylistsenumerator_empty_string );
}

scriptVar SPlaylistsEnumerator::script_vcpu_GetTitle( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar playlistNumber )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylistsEnumerator *enumerator = static_cast<SPlaylistsEnumerator *>( o->vcpu_getInterface( makiPlaylistsEnumeratorGUID ) );
	size_t i = GET_SCRIPT_INT( playlistNumber );
	if ( enumerator && i < enumerator->info.size() )
	{
		return MAKE_SCRIPT_STRING( enumerator->info[ i ]->title );
	}

	return MAKE_SCRIPT_STRING( static_splaylistsenumerator_empty_string );
}

scriptVar SPlaylistsEnumerator::script_vcpu_GetLength( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar playlistNumber )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylistsEnumerator *enumerator = static_cast<SPlaylistsEnumerator *>( o->vcpu_getInterface( makiPlaylistsEnumeratorGUID ) );
	size_t i = GET_SCRIPT_INT( playlistNumber );
	if ( enumerator && i < enumerator->info.size() )
	{
		return MAKE_SCRIPT_INT( enumerator->info[ i ]->length );
	}

	return MAKE_SCRIPT_INT( -1 );
}

scriptVar SPlaylistsEnumerator::script_vcpu_GetNumItems( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar playlistNumber )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylistsEnumerator *enumerator = static_cast<SPlaylistsEnumerator *>( o->vcpu_getInterface( makiPlaylistsEnumeratorGUID ) );
	size_t i = GET_SCRIPT_INT( playlistNumber );
	if ( enumerator && i < enumerator->info.size() )
	{
		return MAKE_SCRIPT_INT( enumerator->info[ i ]->numItems );
	}

	return MAKE_SCRIPT_INT( 0 );
}

static wchar_t static_splaylistsenumerator_guid_string[39];
scriptVar SPlaylistsEnumerator::script_vcpu_GetGUID( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar playlistNumber )
{
	SCRIPT_FUNCTION_INIT;
	SPlaylistsEnumerator *enumerator = static_cast<SPlaylistsEnumerator *>( o->vcpu_getInterface( makiPlaylistsEnumeratorGUID ) );
	size_t i = GET_SCRIPT_INT( playlistNumber );
	if ( enumerator && i < enumerator->info.size() )
	{
		nsGUID::toCharW( enumerator->info[ i ]->guid, static_splaylistsenumerator_guid_string );
		return MAKE_SCRIPT_STRING( static_splaylistsenumerator_guid_string );
	}

	return MAKE_SCRIPT_STRING( static_splaylistsenumerator_empty_string );
}

void SPlaylistsEnumerator::Reserve( size_t count )
{
	info.reserve( count );
}

void SPlaylistsEnumerator::AppendPlaylist( const PlaylistInfo &newPlaylist )
{
	info.push_back( new PlaylistInfo( newPlaylist ) );
}