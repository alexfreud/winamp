#ifndef NULLSOFT_GEN_FF_PLAYLISTSCRIPTOBJECT_H
#define NULLSOFT_GEN_FF_PLAYLISTSCRIPTOBJECT_H

#include "wa2frontend.h"

class SPlaylist;

#include <api/script/script.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>
#include <api/script/objcontroller.h>

#include <api/service/svcs/svc_scriptobji.h>

// -----------------------------------------------------------------------------------------------------
// ScriptObject Provider Service

class PlaylistScriptObjectSvc : public svc_scriptObjectI {

public:
  PlaylistScriptObjectSvc() {};
  virtual ~PlaylistScriptObjectSvc() {};

  static const char *getServiceName() { return "PlEdit maki object"; }
  virtual ScriptObjectController *getController(int n);
};  

// -----------------------------------------------------------------------------------------------------
// PlaylistScriptObject GUID
// {345BEEBC-0229-4921-90BE-6CB6A49A79D9}
static const GUID playlistScriptObjectGUID = 
{ 0x345beebc, 0x229, 0x4921, { 0x90, 0xbe, 0x6c, 0xb6, 0xa4, 0x9a, 0x79, 0xd9 } };

#define SPLAYLIST_SCRIPTPARENT RootObjectInstance

// -----------------------------------------------------------------------------------------------------
// ScriptObject Service

class PlaylistScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"PlEdit"; }
    virtual const wchar_t *getAncestorClassName() { return L"Object"; }
	virtual ScriptObjectController *getAncestorController() { return rootScriptObjectController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions() { return exportedFunction; }
    virtual GUID getClassGuid() { return playlistScriptObjectGUID; }
    //virtual int getInstantiable() { return 0; }
    //virtual int getReferenceable() { return 0; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern ScriptObjectController *playlistController;

class SPlaylist : public SPLAYLIST_SCRIPTPARENT {
public:
	SPlaylist();
	virtual ~SPlaylist();

	static PtrList < SPlaylist > SOList;

	static	void		onPleditModified();
	static	void		showEntry (int i);
	static	void		swap (int a, int b);
	static	fileinfoW *	getFileInfoStructW1 (int index);

	// Maki functions table
    static	scriptVar	script_vcpu_showCurrentlyPlayingEntry(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static	scriptVar	script_vcpu_showEntry(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i);
	static	scriptVar	script_vcpu_getCurrentIndex(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static	scriptVar	script_vcpu_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static	scriptVar	script_vcpu_getTrackRating(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i);
	static	scriptVar	script_vcpu_setTrackRating(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i, scriptVar rating);
	static	scriptVar	script_vcpu_enqueueFile (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar file);
	static	scriptVar	script_vcpu_clear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static	scriptVar	script_vcpu_removeTrack(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i);
	static	scriptVar	script_vcpu_swapTrack(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar track, scriptVar to);
	static	scriptVar	script_vcpu_moveUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar track);
	static	scriptVar	script_vcpu_moveDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar track);
	static	scriptVar	script_vcpu_moveTo(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar track, scriptVar to);
	static	scriptVar	script_vcpu_getTitle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track);
	static	scriptVar	script_vcpu_getLength(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track);
	static	scriptVar	script_vcpu_getExtendedInfo(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _track, scriptVar _name);
	static	scriptVar	script_vcpu_getNumSelectedItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static	scriptVar	script_vcpu_getNextSelectedItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _num);
	static	scriptVar	script_vcpu_getFileName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _num);
	static	scriptVar	script_vcpu_playTrack(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar _num);
	static	scriptVar	script_vcpu_onPleditModified(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

private:
	static	wchar_t		staticStr[4096];
};

#endif