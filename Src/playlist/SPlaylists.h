#pragma once

#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>

class PlaylistsScriptController : public ScriptObjectControllerI
{
public:
    const wchar_t                    *getClassName();
    const wchar_t                    *getAncestorClassName();
    ScriptObjectController           *getAncestorController();
    int                               getNumFunctions();
    const function_descriptor_struct *getExportedFunctions();
    GUID                              getClassGuid();

    ScriptObject                     *instantiate();
    void                              destroy( ScriptObject *o );

    void                             *encapsulate( ScriptObject *o );
    void                              deencapsulate( void *o );

    int                               getInstantiable();
    int                               getReferenceable();

private:
    static function_descriptor_struct exportedFunction[];
};

class SPlaylists : public RootObjectInstance
{
public:
    SPlaylists();

    static scriptVar script_vcpu_GetEnumerator( SCRIPT_FUNCTION_PARAMS, ScriptObject *o );
    static scriptVar script_vcpu_OpenPlaylist( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar scriptPlaylistGUID );
    static scriptVar script_vcpu_SavePlaylist( SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar scriptPlaylistGUID, scriptVar scriptPlaylist );

private:

};