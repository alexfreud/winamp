#pragma once

#include "api/script/objcontroller.h"
#include "api/script/objects/rootobj.h"
#include "Playlist.h"

class PlaylistScriptController : public ScriptObjectControllerI
{
public:
	virtual const wchar_t *getClassName();
	virtual const wchar_t *getAncestorClassName();
	virtual ScriptObjectController *getAncestorController();
	virtual int getNumFunctions();
	virtual const function_descriptor_struct *getExportedFunctions();
	virtual GUID getClassGuid();
	virtual ScriptObject *instantiate();
	virtual void destroy( ScriptObject *o );
	virtual void *encapsulate( ScriptObject *o );
	virtual void deencapsulate( void *o );

private:
	static function_descriptor_struct exportedFunction[];
};

class SPlaylist : public RootObjectInstance
{
public:
	SPlaylist();

	/* ifc_playlist wrapper */
	static scriptVar script_vcpu_Clear( SCRIPT_FUNCTION_PARAMS, ScriptObject *o );
	static scriptVar script_vcpu_GetNumItems( SCRIPT_FUNCTION_PARAMS, ScriptObject *o );
	static scriptVar script_vcpu_GetItem( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber );
	static scriptVar script_vcpu_GetItemTitle( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber );
	static scriptVar script_vcpu_GetItemLength( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber );
	static scriptVar script_vcpu_GetItemExtendedInfo( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber, scriptVar metadata );
	static scriptVar script_vcpu_Reverse( SCRIPT_FUNCTION_PARAMS, ScriptObject *o );
	static scriptVar script_vcpu_Swap( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar position1, scriptVar position2 );
	static scriptVar script_vcpu_Randomize( SCRIPT_FUNCTION_PARAMS, ScriptObject *o );
	static scriptVar script_vcpu_Remove( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemIndex );
	static scriptVar script_vcpu_SortByTitle( SCRIPT_FUNCTION_PARAMS, ScriptObject *o );
	static scriptVar script_vcpu_SortByFilename( SCRIPT_FUNCTION_PARAMS, ScriptObject *o );

	/* extra functions */
//private:
	Playlist playlist;
};


// {632883FC-159F-4330-B193-CFD62CA47EC1}
static const GUID makiPlaylistGUID =
{ 0x632883fc, 0x159f, 0x4330, { 0xb1, 0x93, 0xcf, 0xd6, 0x2c, 0xa4, 0x7e, 0xc1 } };
