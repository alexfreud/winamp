#pragma once
#include <api/script/objects/rootobj.h>
#include <api/script/objcontroller.h>
#include <api/script/scriptobj.h>
#include <bfc/depend.h>
#include <api/service/svcs/svc_scriptobji.h>
#include <api/wnd/wndclass/listwnd.h>
#include "wa2playlist.h"
#include <api/syscb/callbacks/playlistcb.h>
#include <api/service/svcs/svc_scriptobji.h>

class PlDirObject;

extern ScriptObjectController *pldirController;

// -----------------------------------------------------------------------------------------------------
// ScriptObject Service
class PlDirScriptObjectSvc : public svc_scriptObjectI {

public:
  PlDirScriptObjectSvc() {};
  virtual ~PlDirScriptObjectSvc() {};

  static const char *getServiceName() { return "PlDir script object"; }
  virtual ScriptObjectController *getController(int n);
};       

// -----------------------------------------------------------------------------------------------------
// Script classe GUIDS

// {61A7ABAD-7D79-41f6-B1D0-E1808603A4F4}
static const GUID PLDIR_SCRIPTOBJECT_GUID = 
{ 0x61a7abad, 0x7d79, 0x41f6, { 0xb1, 0xd0, 0xe1, 0x80, 0x86, 0x3, 0xa4, 0xf4 } };

// -----------------------------------------------------------------------------------------------------
// ScriptObject Interface

//   PlDir 
class PlDirObject : public ListWnd, public PlaylistCallbackI
{  
  public:

    PlDirObject();
    virtual ~PlDirObject();

    virtual int onInit();
    //virtual int onResize();
		virtual int wantResizeCols() { return 0; }
    virtual int wantHScroll() { return 0; }

    virtual void onDoubleClick(int itemnum);
    virtual Wa2Playlist *getPlaylist(int itemnum);

		/* PlaylistCallbackI method overrides */
		int playlistcb_added(size_t index);
		int playlistcb_saved(size_t index);


		int onDeferredCallback(intptr_t p1, intptr_t p2);

		void Populate();
  private:
    PtrList<Wa2Playlist> playlists;

};

// -----------------------------------------------------------------------------------------------------
// ScriptObjectControllers for our script classes

//   PlDir
class PlDirScriptObjectController : public ScriptObjectControllerI {
  public:
    virtual const wchar_t *getClassName() { return L"PlDir"; }
    virtual const wchar_t *getAncestorClassName() { return L"Object"; }
    virtual ScriptObjectController *getAncestorController() { return NULL; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions() { return exportedFunction; }
    virtual GUID getClassGuid() { return PLDIR_SCRIPTOBJECT_GUID; }
    virtual int getInstantiable() { return 0; }
    virtual int getReferenceable() { return 0; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

	virtual ~PlDirScriptObjectController();

	// Maki functions table
    static scriptVar pldir_showCurrentlyPlayingEntry(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar pldir_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar pldir_renameItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar item, scriptVar name);
	static scriptVar pldir_getItemName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar item);
	static scriptVar pldir_playItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar item);
	static scriptVar pldir_enqueueItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar item);
	static scriptVar pldir_refresh(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	PtrList<PlDirObject> mylist;

  private:
    static function_descriptor_struct exportedFunction[];
};

extern const wchar_t plDirXuiObjectStr[];
extern char plDirXuiSvcName[];
class PlDirXuiSvc : public XuiObjectSvc<PlDirObject, plDirXuiObjectStr, plDirXuiSvcName> {};

