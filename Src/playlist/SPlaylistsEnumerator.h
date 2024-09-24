
#pragma once

#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include "Playlists.h"
#include <vector>

class PlaylistsEnumeratorScriptController : public ScriptObjectControllerI 
{
  public:
    const wchar_t *getClassName();
    const wchar_t *getAncestorClassName();
    ScriptObjectController *getAncestorController();
    int getNumFunctions();
    const function_descriptor_struct *getExportedFunctions();
    GUID getClassGuid();
    ScriptObject *instantiate();
    void destroy(ScriptObject *o);
    void *encapsulate(ScriptObject *o);
    void deencapsulate(void *o);

  private:
    static function_descriptor_struct exportedFunction[];
};

class SPlaylistsEnumerator : public RootObjectInstance
{
public:
	SPlaylistsEnumerator();
	~SPlaylistsEnumerator();

	void Reserve(size_t count);
	void AppendPlaylist(const PlaylistInfo &newPlaylist);

	static scriptVar script_vcpu_GetCount(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_GetFilename(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber);
	static scriptVar script_vcpu_GetTitle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber);
	static scriptVar script_vcpu_GetLength(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber);
	static scriptVar script_vcpu_GetNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber);
	static scriptVar script_vcpu_GetGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemNumber);

private:
	std::vector<PlaylistInfo*> info;
};