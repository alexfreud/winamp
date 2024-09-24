#include <precomp.h>

#include <api/script/objects/core/coreobj.h>
#include <api/core/buttons.h>
#include <api/service/svcs/svc_coreadmin.h>

static CoreScriptObjectController _coreController;
ScriptObjectController *coreController = &_coreController;

#define TRICK_VOLUME  0

// Table of exported functions and events
// "function name",  n. params, function_pointer
function_descriptor_struct CoreScriptObjectController::exportedFunction[] = {
  {L"playFile",                      1, (void*)ScriptCoreObject::maki_playFile},
  {L"stop",                          0, (void*)ScriptCoreObject::maki_stop},
  {L"setVolume",                     1, (void*)ScriptCoreObject::maki_setVolume},
//  {L"onStop",                        0, (void*)ScriptCoreObject::maki_onStop},
  {L"getStatus",                     0, (void*)ScriptCoreObject::maki_getStatus},
//  {L"getStuff",                  0, (void*)ScriptCoreObject::maki_getStuff},

  // Here come the new ones
  {L"onStarted",                     0, (void*)ScriptCoreObject::maki_onStarted},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  {L"onStopped",                     0, (void*)ScriptCoreObject::maki_onStopped},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  {L"onPaused",                      0, (void*)ScriptCoreObject::maki_onPaused},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  {L"onUnpaused",                    0, (void*)ScriptCoreObject::maki_onUnpaused},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  {L"onSeeked",                      1, (void*)ScriptCoreObject::maki_onSeeked},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newpos);
  {L"onVolumeChange",                1, (void*)ScriptCoreObject::maki_onVolumeChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newvol);
  {L"onPanChange",                   1, (void*)ScriptCoreObject::maki_onPanChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newpan);
  {L"onEQStatusChange",              1, (void*)ScriptCoreObject::maki_onEQStatusChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newval);
  {L"onEQPreampChange",              1, (void*)ScriptCoreObject::maki_onEQPreampChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newval);
  {L"onEQBandChange",                2, (void*)ScriptCoreObject::maki_onEQBandChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ band, scriptVar /*int*/ newval);
	{L"onEQFreqChange",                1, (void*)ScriptCoreObject::maki_onEQFreqChange},  
  {L"onEQAutoChange",                1, (void*)ScriptCoreObject::maki_onEQAutoChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newval);
  {L"onCoreStatusMsg",               1, (void*)ScriptCoreObject::maki_onStatusMsg},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ text);
  {L"onWarningMsg",                  1, (void*)ScriptCoreObject::maki_onWarningMsg},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ text);
  {L"onErrorMsg",                    1, (void*)ScriptCoreObject::maki_onErrorMsg},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ text);
  {L"onTitleChange",                 1, (void*)ScriptCoreObject::maki_onTitleChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ title);
  {L"onTitle2Change",                1, (void*)ScriptCoreObject::maki_onTitle2Change},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ title2);
  {L"onInfoChange",                  1, (void*)ScriptCoreObject::maki_onInfoChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ info);
  {L"onUrlChange",                   1, (void*)ScriptCoreObject::maki_onUrlChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ url);
  {L"onLengthChange",                1, (void*)ScriptCoreObject::maki_onLengthChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newlength);
  {L"onNextFile",                    0, (void*)ScriptCoreObject::maki_onNextFile},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  {L"onNeedNextFile",                1, (void*)ScriptCoreObject::maki_onNeedNextFile},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ fileid);
  {L"onSetNextFile",                 1, (void*)ScriptCoreObject::maki_onSetNextFile},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ playstring);
  {L"onErrorOccured",                2, (void*)ScriptCoreObject::maki_onErrorOccured},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ severity, scriptVar /*String*/ text);
  {L"onAbortCurrentSong",            0, (void*)ScriptCoreObject::maki_onAbortCurrentSong},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  {L"onEndOfDecode",                 0, (void*)ScriptCoreObject::maki_onEndOfDecode},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  {L"onFileComplete",                1, (void*)ScriptCoreObject::maki_onFileComplete},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ playstring);
  {L"onConvertersChainRebuilt",      0, (void*)ScriptCoreObject::maki_onConvertersChainRebuilt},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  {L"onMediaFamilyChange",           1, (void*)ScriptCoreObject::maki_onMediaFamilyChange},  // (SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ newfamily);

  {L"setNextFile",                   1, (void*)ScriptCoreObject::maki_setNextFile},  // (scriptVar /*String*/ playstr);
//  {L"getStatus",                     0, (void*)ScriptCoreObject::maki_getStatus},  // ();
  {L"getCurrent",                    0, (void*)ScriptCoreObject::maki_getCurrent},  // ();
  {L"getCurPlaybackNumber",          0, (void*)ScriptCoreObject::maki_getCurPlaybackNumber},  // ();
  {L"getNumTracks",                  0, (void*)ScriptCoreObject::maki_getNumTracks},  // ();
  {L"getPosition",                   0, (void*)ScriptCoreObject::maki_getPosition},  // ();
  {L"getWritePosition",              0, (void*)ScriptCoreObject::maki_getWritePosition},  // ();
  {L"setPosition",                   1, (void*)ScriptCoreObject::maki_setPosition},  // (scriptVar /*int*/ ms);
  {L"getLength",                     0, (void*)ScriptCoreObject::maki_getLength},  // ();
  {L"getVolume",                     0, (void*)ScriptCoreObject::maki_getVolume},  // ();
//  {L"setVolume",                     1, (void*)ScriptCoreObject::maki_setVolume},  // (scriptVar /*int*/ vol);
  {L"getPan",                        0, (void*)ScriptCoreObject::maki_getPan},  // ();
  {L"setPan",                        1, (void*)ScriptCoreObject::maki_setPan},  // (scriptVar /*int*/ bal);
  {L"setMute",                       1, (void*)ScriptCoreObject::maki_setMute},  // (scriptVar /*int*/ mute);
  {L"getMute",                       0, (void*)ScriptCoreObject::maki_getMute},  // ();
  {L"getLeftVuMeter",                0, (void*)ScriptCoreObject::maki_getLeftVuMeter},  // ();
  {L"getRightVuMeter",               0, (void*)ScriptCoreObject::maki_getRightVuMeter},  // ();
  {L"userButton",                    1, (void*)ScriptCoreObject::maki_userButton},  // (scriptVar /*int*/ button);
  {L"getEqStatus",                   0, (void*)ScriptCoreObject::maki_getEqStatus},  // ();
  {L"setEqStatus",                   1, (void*)ScriptCoreObject::maki_setEqStatus},  // (scriptVar /*int*/ enable);
  {L"getEqPreamp",                   0, (void*)ScriptCoreObject::maki_getEqPreamp},  // ();
  {L"setEqPreamp",                   1, (void*)ScriptCoreObject::maki_setEqPreamp},  // (scriptVar /*int*/ pre);
  {L"getEqBand",                     1, (void*)ScriptCoreObject::maki_getEqBand},  // (scriptVar /*int*/ band);
  {L"setEqBand",                     2, (void*)ScriptCoreObject::maki_setEqBand},  // (scriptVar /*int*/ band, scriptVar /*int*/ val);
  {L"getEqAuto",                     0, (void*)ScriptCoreObject::maki_getEqAuto},  // ();
  {L"setEqAuto",                     1, (void*)ScriptCoreObject::maki_setEqAuto},  // (scriptVar /*int*/ enable);
  {L"setCustomMsg",                  1, (void*)ScriptCoreObject::maki_setCustomMsg},  // (scriptVar /*String*/ text);
  {L"setPriority",                   1, (void*)ScriptCoreObject::maki_setPriority},  // (scriptVar /*int*/ priority);
  {L"getPriority",                   0, (void*)ScriptCoreObject::maki_getPriority},  // ();
  {L"rebuildConvertersChain",        0, (void*)ScriptCoreObject::maki_rebuildConvertersChain},  // ();

};

int CoreScriptObjectController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

// Only called by us and our friend the admin.
ScriptObject *CoreScriptObjectController::instantiate(CoreToken token) {
  ScriptCoreObject *gvo = new ScriptCoreObject(token);
  ASSERT(gvo != NULL);
  ScriptCoreObject::objects.addItem(gvo);
  return gvo->getScriptObject();
}

ScriptObject *CoreScriptObjectController::instantiate() {
  // magic value to make a new one.
  return instantiate(0x80000000);
}

void CoreScriptObjectController::destroy(ScriptObject *o) {
  ScriptCoreObject *gvo = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  ASSERT(gvo != NULL);
  ScriptCoreObject::objects.removeItem(gvo);
  delete gvo;
}

void *CoreScriptObjectController::encapsulate(ScriptObject *o) {
  return NULL;
}

void CoreScriptObjectController::deencapsulate(void *o) {
}
                        
// -----------------------------------------------------------------------------------------------------

ScriptCoreObject::ScriptCoreObject(CoreToken token) {
  getScriptObject()->vcpu_setClassName(L"Core");
  getScriptObject()->vcpu_setController(coreController);
  getScriptObject()->vcpu_setInterface(CORE_SCRIPTOBJECT_GUID, static_cast<ScriptCoreObject *>(this));
#ifndef FAKE_SCRIPTCORE
  curvol = -1;
  if (token == 0x80000000) {
    // If we create it new for the script, bind a sequencer and set the volume
    core_handle = api->core_create();
    initAsCreated();
  } else {
    // If the ScriptCoreAdminObject caused this object to be created,
    // it will worry about calling initAsCreated() -- otherwise we don't
    // bind ourselves as a sequencer to the core.
    core_handle = token;
    registered_sequencer = 0;
  }

  waServiceFactory *s = api->service_enumService(WaSvc::COREADMIN,0);
  ASSERTPR(s,"Core Admin must be present to use ScriptCore!");
  svc = castService<svc_coreAdmin>(s);
  if (svc) {
    svc->addCallback(core_handle, this);
  }
#endif
}

ScriptCoreObject::~ScriptCoreObject() {
#ifndef FAKE_SCRIPTCORE
  if (registered_sequencer) {
    // don't remove a sequencer if we didn't add one
    api->core_deregisterSequencer(core_handle, this);
  }
  api->core_delCallback(0, this);
#endif
}

#ifndef FAKE_SCRIPTCORE
void ScriptCoreObject::initAsCreated() {
  api->core_registerSequencer(core_handle, this);
#if TRICK_VOLUME
  api->core_setVolume(core_handle, api->core_getVolume(0));
  setVolume(255);
#endif
  registered_sequencer = 1;
}
#endif

/*int ScriptCoreObject::corecb_onStopped() {
  maki_onStop(SCRIPT_CALL, getScriptObject());
  return 0;
}

int ScriptCoreObject::corecb_onEndOfDecode() {
  maki_onStop(SCRIPT_CALL, getScriptObject());
  return 0;
}*/

int ScriptCoreObject::getStatus() {
#ifndef FAKE_SCRIPTCORE
  return api->core_getStatus(core_handle);
#else 
  return 0;
#endif
}

void ScriptCoreObject::playFile(const wchar_t *file) {
#ifndef FAKE_SCRIPTCORE
  if (registered_sequencer) {
    filetoplay = file;
  } else {
    String playstring = "FILE:";
    playstring += file;
    api->core_setNextFile(core_handle, playstring);
  }
  api->core_userButton(core_handle, UserButton::PLAY);
#endif
}

#ifndef FAKE_SCRIPTCORE
const char *ScriptCoreObject::enumItem(int pos) { 
  return filetoplay; 
}
#endif

void ScriptCoreObject::stop() {
#ifndef FAKE_SCRIPTCORE
  filetoplay = "";
  api->core_userButton(core_handle, UserButton::STOP);
#endif
}

void ScriptCoreObject::setVolume(int v) {
#ifndef FAKE_SCRIPTCORE
#if TRICK_VOLUME
  // if we're not the main core....
  if (core_handle) {
    // set this core's volume as a percentage of the main core volume.
    if (v == -1) v = curvol;
    curvol = v;
    int mainvolume = api->core_getVolume(0);
    v = (int)( ((float)mainvolume * (float)v) / 256.0 );
    if ((unsigned)v == api->core_getVolume(core_handle)) return;
    api->core_setVolume(core_handle, MIN(MAX(0, v), 255));
  } else {
    // if we are the main core, don't cause recursion.
    if ((v > 0) && (v != curvol)) {
      curvol = v;
      api->core_setVolume(core_handle, v);
    }
  }
#else
  api->core_setVolume(core_handle, v);
#endif
#endif
}

/*
    Moved to be with all his friends.

int ScriptCoreObject::corecb_onVolumeChange(int newvol) {
  foreach(objects)
    objects.getfor()->setVolume(-1);
  endfor;
  return 0;
}
*/

/*scriptVar ScriptCoreObject::maki_onStop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT  
  PROCESS_HOOKS0(o, coreController); 
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}*/

scriptVar ScriptCoreObject::maki_playFile(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar f) {
  SCRIPT_FUNCTION_INIT  
  ScriptCoreObject *gvo = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (gvo) 
    gvo->playFile(GET_SCRIPT_STRING(f));
  RETURN_SCRIPT_VOID;
}

scriptVar ScriptCoreObject::maki_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT  
  ScriptCoreObject *gvo = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (gvo) 
    gvo->stop();
  RETURN_SCRIPT_VOID;
}

/*
scriptVar ScriptCoreObject::maki_setVolume(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT  
  ScriptCoreObject *gvo = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (gvo) gvo->setVolume(GET_SCRIPT_INT(v));
  RETURN_SCRIPT_VOID;
}

scriptVar ScriptCoreObject::maki_getStatus(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT  
  ScriptCoreObject *gvo = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (gvo) return MAKE_SCRIPT_INT(gvo->getStatus());
  RETURN_SCRIPT_ZERO;
}                   
*/

PtrList<ScriptCoreObject> ScriptCoreObject::objects;


int ScriptCoreObject::corecb_onStarted() {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onStarted(SCRIPT_CALL, getScriptObject());
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onStopped() {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onStopped(SCRIPT_CALL, getScriptObject());
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onPaused() {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onPaused(SCRIPT_CALL, getScriptObject());
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onUnpaused() {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onUnpaused(SCRIPT_CALL, getScriptObject());
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onSeeked(int newpos) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onSeeked(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(newpos));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onVolumeChange(int newvol) {
#ifndef FAKE_SCRIPTCORE
#if TRICK_VOLUME
  foreach(objects)
    objects.getfor()->setVolume(-1);
  endfor;
#endif
  scriptVar svInt = maki_onVolumeChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(newvol));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onPanChange(int newpan) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onPanChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(newpan));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onEQStatusChange(int newval) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onEQStatusChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(newval));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onEQPreampChange(int newval) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onEQPreampChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(newval));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onEQBandChange(int band, int newval) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onEQBandChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(band), MAKE_SCRIPT_INT(newval));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onEQFreqChange(int newval) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onEQFreqChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(newval));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onEQAutoChange(int newval) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onEQAutoChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(newval));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onStatusMsg(const wchar_t *text) 
{
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onStatusMsg(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(text));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onWarningMsg(const wchar_t *text) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onWarningMsg(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(text));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onErrorMsg(const wchar_t *text) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onErrorMsg(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(text));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onTitleChange(const wchar_t *title) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onTitleChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(title));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onTitle2Change(const wchar_t *title2) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onTitle2Change(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(title2));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onInfoChange(const wchar_t *info) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onInfoChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(info));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onUrlChange(const wchar_t *url) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onUrlChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(url));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onLengthChange(int newlength) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onLengthChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(newlength));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onNextFile() {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onNextFile(SCRIPT_CALL, getScriptObject());
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onNeedNextFile(int fileid) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onNeedNextFile(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(fileid));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onSetNextFile(const wchar_t *playstring) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onSetNextFile(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(playstring));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onErrorOccured(int severity, const wchar_t *text) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onErrorOccured(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(severity), MAKE_SCRIPT_STRING(text));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onAbortCurrentSong() {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onAbortCurrentSong(SCRIPT_CALL, getScriptObject());
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onEndOfDecode() {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onEndOfDecode(SCRIPT_CALL, getScriptObject());
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onFileComplete(const wchar_t *playstring) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onFileComplete(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(playstring));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onConvertersChainRebuilt() {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onConvertersChainRebuilt(SCRIPT_CALL, getScriptObject());
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}

int ScriptCoreObject::corecb_onMediaFamilyChange(const wchar_t *newfamily) {
#ifndef FAKE_SCRIPTCORE
  scriptVar svInt = maki_onMediaFamilyChange(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_STRING(newfamily));
  if ((svInt.type != SCRIPT_VOID) && (svInt.type != SCRIPT_OBJECT) && (svInt.type != SCRIPT_STRING)) { 
    return GET_SCRIPT_INT(svInt);
  }
#endif
  return 0;
}


scriptVar /*int*/ ScriptCoreObject::maki_onStarted(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS0(o, coreController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar /*int*/ ScriptCoreObject::maki_onStopped(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
   PROCESS_HOOKS0(o, coreController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar /*int*/ ScriptCoreObject::maki_onPaused(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
   PROCESS_HOOKS0(o, coreController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar /*int*/ ScriptCoreObject::maki_onUnpaused(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
   PROCESS_HOOKS0(o, coreController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar /*int*/ ScriptCoreObject::maki_onSeeked(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newpos) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newpos);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newpos);
}

scriptVar /*int*/ ScriptCoreObject::maki_onVolumeChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newvol) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newvol);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newvol);
}

scriptVar /*int*/ ScriptCoreObject::maki_onPanChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newpan) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newpan);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newpan);
}

scriptVar /*int*/ ScriptCoreObject::maki_onEQStatusChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newval) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newval);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newval);
}

scriptVar /*int*/ ScriptCoreObject::maki_onEQPreampChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newval) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newval);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newval);
}

scriptVar /*int*/ ScriptCoreObject::maki_onEQBandChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ band, scriptVar /*int*/ newval) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS2(o, coreController, band, newval);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, band, newval);
}


scriptVar /*int*/ ScriptCoreObject::maki_onEQFreqChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newval) 
{
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newval);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o,  newval);
}

scriptVar /*int*/ ScriptCoreObject::maki_onEQAutoChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newval) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newval);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newval);
}

scriptVar /*int*/ ScriptCoreObject::maki_onStatusMsg(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ text) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, text);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, text);
}

scriptVar /*int*/ ScriptCoreObject::maki_onWarningMsg(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ text) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, text);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, text);
}

scriptVar /*int*/ ScriptCoreObject::maki_onErrorMsg(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ text) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, text);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, text);
}

scriptVar /*int*/ ScriptCoreObject::maki_onTitleChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ title) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, title);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, title);
}

scriptVar /*int*/ ScriptCoreObject::maki_onTitle2Change(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ title2) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, title2);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, title2);
}

scriptVar /*int*/ ScriptCoreObject::maki_onInfoChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ info) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, info);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, info);
}

scriptVar /*int*/ ScriptCoreObject::maki_onUrlChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ url) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, url);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, url);
}

scriptVar /*int*/ ScriptCoreObject::maki_onLengthChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ newlength) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newlength);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newlength);
}

scriptVar /*int*/ ScriptCoreObject::maki_onNextFile(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS0(o, coreController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar /*int*/ ScriptCoreObject::maki_onNeedNextFile(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ fileid) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, fileid);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, fileid);
}

scriptVar /*int*/ ScriptCoreObject::maki_onSetNextFile(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ playstring) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, playstring);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, playstring);
}

scriptVar /*int*/ ScriptCoreObject::maki_onErrorOccured(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ severity, scriptVar /*String*/ text) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS2(o, coreController, severity, text);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, severity, text);
}

scriptVar /*int*/ ScriptCoreObject::maki_onAbortCurrentSong(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS0(o, coreController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar /*int*/ ScriptCoreObject::maki_onEndOfDecode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS0(o, coreController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar /*int*/ ScriptCoreObject::maki_onFileComplete(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ playstring) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, playstring);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, playstring);
}

scriptVar /*int*/ ScriptCoreObject::maki_onConvertersChainRebuilt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS0(o, coreController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar /*int*/ ScriptCoreObject::maki_onMediaFamilyChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ newfamily) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS1(o, coreController, newfamily);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, newfamily);
}

                  
scriptVar /*int*/ ScriptCoreObject::maki_setNextFile(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ playstr) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->setNextFile(sco->core_handle, GET_SCRIPT_STRING(playstr)));
  }
#endif
  RETURN_SCRIPT_ZERO;
}

scriptVar /*int*/ ScriptCoreObject::maki_getStatus(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getStatus(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*String*/ ScriptCoreObject::maki_getCurrent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_STRING(sco->svc->getCurrent(sco->core_handle));
  }
#endif
  return MAKE_SCRIPT_STRING(L"");
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getCurPlaybackNumber(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getCurPlaybackNumber(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getNumTracks(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getNumTracks(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getPosition(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getWritePosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getWritePosition(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_setPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ ms) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->setPosition(sco->core_handle, GET_SCRIPT_INT(ms)));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getLength(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getLength(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getVolume(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getVolume(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setVolume(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ vol) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setVolume(sco->core_handle, GET_SCRIPT_INT(vol));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getPan(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getPan(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setPan(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ bal) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setPan(sco->core_handle, GET_SCRIPT_INT(bal));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setMute(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ mute) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setMute(sco->core_handle, GET_SCRIPT_INT(mute));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getMute(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getMute(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getLeftVuMeter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getLeftVuMeter(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getRightVuMeter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getRightVuMeter(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_userButton(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ button) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->userButton(sco->core_handle, GET_SCRIPT_INT(button));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getEqStatus(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getEqStatus(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setEqStatus(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ enable) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setEqStatus(sco->core_handle, GET_SCRIPT_INT(enable));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getEqPreamp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getEqPreamp(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setEqPreamp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ pre) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setEqPreamp(sco->core_handle, GET_SCRIPT_INT(pre));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getEqBand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ band) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getEqBand(sco->core_handle, GET_SCRIPT_INT(band)));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setEqBand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ band, scriptVar /*int*/ val) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setEqBand(sco->core_handle, GET_SCRIPT_INT(band), GET_SCRIPT_INT(val));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getEqAuto(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getEqAuto(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setEqAuto(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ enable) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setEqAuto(sco->core_handle, GET_SCRIPT_INT(enable));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setCustomMsg(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*String*/ text) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setCustomMsg(sco->core_handle, GET_SCRIPT_STRING(text));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_setPriority(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ priority) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->setPriority(sco->core_handle, GET_SCRIPT_INT(priority));
  }
#endif
  RETURN_SCRIPT_VOID;
}
  
scriptVar /*int*/ ScriptCoreObject::maki_getPriority(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    return MAKE_SCRIPT_INT(sco->svc->getPriority(sco->core_handle));
  }
#endif
  RETURN_SCRIPT_ZERO;
}
  
scriptVar /*void*/ ScriptCoreObject::maki_rebuildConvertersChain(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
#ifndef FAKE_SCRIPTCORE
  ScriptCoreObject *sco = static_cast<ScriptCoreObject *>(o->vcpu_getInterface(CORE_SCRIPTOBJECT_GUID));
  if (sco) {
    ASSERT(sco->svc != NULL);
    sco->svc->rebuildConvertersChain(sco->core_handle);
  }
#endif
  RETURN_SCRIPT_VOID;
}

