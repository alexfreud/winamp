#include "precomp.h"
#ifndef _NOSTUDIO

#include "../../common/std.h"
#include "script.h"
#include "scriptmgr.h"
#include "../../common/script/scriptobj.h"
#include "compoobj.h"
#include "../api.h"
#include "vcpu.h"
#include "../smap.h"
#include "sregion.h"
#include "../skinparse.h"
#include "../compon.h"
#include "../compwnd.h"
#include "../cbmgr.h"
#include "../smap.h"
#include "../../common/wndcb.h"

#else
#include "compoobj.h"
#endif

CompoObjScriptController _compoController;
CompoObjScriptController *compoController=&_compoController;

// -- Functions table -------------------------------------
function_descriptor_struct CompoObjScriptController::exportedFunction[] = {
  {"getGUID", 1, (void*)ComponentObject::script_vcpu_getGUID },
  {"getWac", 0, (void*)ComponentObject::script_vcpu_getWac },
  {"setRegionFromMap", 3, (void*)ComponentObject::script_vcpu_setRegionFromMap },
  {"setRegion", 1, (void*)ComponentObject::script_vcpu_setRegion },
  {"onGetWac", 1, (void*)ComponentObject::script_vcpu_onGetWac },
  {"onGiveUpWac", 1, (void*)ComponentObject::script_vcpu_onGiveUpWac },
  {"setAcceptWac", 1, (void*)ComponentObject::script_vcpu_setAcceptWac },
};
// --------------------------------------------------------

const char *CompoObjScriptController::getClassName() {
  return "Component";
}

const wchar_t *CompoObjScriptController::getAncestorClassName() {
  return "GuiObject";
}

int CompoObjScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *CompoObjScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID CompoObjScriptController::getClassGuid() {
  return componentObjectGuid;
}

ScriptObject *CompoObjScriptController::instantiate() {
  ComponentObject *obj = new ComponentObject();
  return obj->getScriptObject();
}

void CompoObjScriptController::destroy(ScriptObject *o) {
  ComponentObject *obj = static_cast<ComponentObject *>(o->vcpu_getInterface(componentObjectGuid));
  ASSERT(obj != NULL);
  delete obj;
}

void *CompoObjScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for componentobject yet
}

void CompoObjScriptController::deencapsulate(void *) {
}


ComponentObject::ComponentObject() {
  getScriptObject()->vcpu_setInterface(componentObjectGuid, (void *)static_cast<ComponentObject *>(this));
  getScriptObject()->vcpu_setClassName("Component");
  getScriptObject()->vcpu_setController(compoController);
  deleting = 0;
  compwnd = NULL;
  noshowcmdbar = 0;
  noshowbtnbar = 0;
  MEMCPY(&myGUID, &INVALID_GUID, sizeof(GUID));
  MEMCPY(&myCompGuid, &INVALID_GUID, sizeof(GUID));
  my_region_clone = NULL;
  ComponentManager::registerComponentObject(this);
  autoopen = 0;
  autoclose = 0;
  accept = 1;
  denyDesktopAlpha = 0;
  denyTransparency = 0;
  noanimrects = 0;
}

int ComponentObject::setXmlParam(const char *name, const char *value) {
  if (COMPONENTOBJECT_PARENT::setXmlParam(name, value)) return 1;
  else if (STRCASEEQL(name, "param")) {
    GUID *g;
    g = SkinParser::getComponentGuid(value);
    if (g)
      setGUID(*g);
  } else if (STRCASEEQL(name, "noshowcmdbar")) {
    noshowcmdbar = WTOI(value);
  } else if (STRCASEEQL(name, "autoopen")) {
    autoopen = WTOI(value);
  } else if (STRCASEEQL(name, "autoclose")) {
    autoclose = WTOI(value);
  } else if (STRCASEEQL(name, "disableanimatedrects")) {
    noanimrects = WTOI(value);
  } else return 0;
  return 1;
}

ComponentObject::~ComponentObject() {
  deleting = 1;
  ComponentManager::unregisterComponentObject(this, 1);
  delete my_region_clone;
}

int ComponentObject::onResize() {
  COMPONENTOBJECT_PARENT::onResize();
  RECT r = clientRect();
  if (compwnd) {
    compwnd->resize(&r);
  }
  return 1;
}

int ComponentObject::handleRatio() {
  return compwnd ? compwnd->handleRatio() : 0;
}

void ComponentObject::setGUID(GUID g) {
  myGUID = g;
}

void ComponentObject::setCompGUID(GUID g) {
  myCompGuid = g;
}

GUID ComponentObject::getGUID(void) {
  if (!MEMCMP(&myCompGuid, &INVALID_GUID, sizeof(GUID)))
    return myGUID;
  else
    return myCompGuid;
}

void ComponentObject::script_resetRegion() {
  HWND h = ComponentManager::getComponentHWnd(myGUID);
  if (h) SetWindowRgn(h, NULL, TRUE); 
  delete my_region_clone;
  my_region_clone = NULL;
}

void ComponentObject::script_setRegionFromMap(SMap *map, int byte, int inv) {
  RECT r={map->getBitmap()->getX(), map->getBitmap()->getY(), map->getBitmap()->getWidth(), map->getBitmap()->getHeight()};
  api_region *reg = new api_region(map->getBitmap(), &r, 0, 0,  FALSE, 1, (unsigned char)byte, inv);
  if (!reg) { script_resetRegion(); return; }
  delete my_region_clone;
  my_region_clone = new api_region();
  my_region_clone->add(reg);
  delete reg;
  HWND h = ComponentManager::getComponentHWnd(myGUID);
  if (h) {
    api_region *clone = my_region_clone->clone();
    clone->scale(getRenderRatio());
    SetWindowRgn(h, clone->makeWindowRegion(), TRUE); 
    my_region_clone->disposeClone(clone);
  }
}

void ComponentObject::script_setRegion(SRegion *r) {
  api_region *reg = r->getRegion();
  if (!reg) { script_resetRegion(); return; }
  delete my_region_clone;
  my_region_clone = new api_region();
  my_region_clone->add(reg);
  HWND h = ComponentManager::getComponentHWnd(myGUID);
  if (h) {
    api_region *clone = my_region_clone->clone();
    clone->scale(getRenderRatio());
    SetWindowRgn(h, clone->makeWindowRegion(), TRUE); 
    my_region_clone->disposeClone(clone);
  }
}

void ComponentObject::onSetVisible(int s) {
  COMPONENTOBJECT_PARENT::onSetVisible(s);
  if (compwnd) {
    compwnd->setVisible(s);
    onResize();
  } else {
    if (s && autoopen && myGUID != INVALID_GUID && myGUID != GENERIC_GUID) 
      api->setComponentVisible(myGUID, 1);
  }
}

void ComponentObject::deniedComponentCompWnd(CompWnd *c, GUID g) {
  if (!deleting) {
    Container *_c = getGuiObject()->guiobject_getParentGroup()->getParentContainer();
    if (_c) _c->resetGUID();
  }
  compwnd = c;
  compwnd->suppressStatusBar(noshowcmdbar);

  CallbackManager::issueCallback(SysCallback::WINDOW, WndCallback::HIDEWINDOW, reinterpret_cast<int>((int *)&g));

  onReleaseComponent();
  compwnd = NULL;
  denyDesktopAlpha = 0;
  denyTransparency = 0;
  if (!deleting) getGuiObject()->guiobject_getParentLayout()->onGuiObjectSetVisible(getGuiObject(), 0);
}

void ComponentObject::grantedComponentCompWnd(CompWnd *c, GUID g) {
  Container *_c = getGuiObject()->guiobject_getParentGroup()->getParentContainer();
  if (_c) _c->setGUID(g); // tells the container to change its script_id to id:{guid}
  compwnd = c;
  compwnd->suppressStatusBar(noshowcmdbar);

  onResize();

  if (isVisible())
    c->setVisible(1);

  CallbackManager::issueCallback(SysCallback::WINDOW, WndCallback::SHOWWINDOW, reinterpret_cast<int>((int *)&g));

  onGetComponent(g);
}

void ComponentObject::onReleaseComponent() {
  HWND h = ComponentManager::getComponentHWnd(myGUID);
  SetWindowRgn(h, NULL, FALSE); 
}

void ComponentObject::onGetComponent(GUID g) {
  HWND h = ComponentManager::getComponentHWnd(myGUID);
  if (h) {
    if (my_region_clone) {
      api_region *clone = my_region_clone->clone();
      clone->scale(getRenderRatio());
      SetWindowRgn(h, clone->makeWindowRegion(), TRUE); 
      my_region_clone->disposeClone(clone);
    } else
      SetWindowRgn(h, NULL, FALSE); 
  } 
}

int ComponentObject::wantGUID(GUID *g) {
  if (!accept || !getGuiObject()->guiobject_getParentGroup()) return 0;
  Container *_c = getGuiObject()->guiobject_getParentGroup()->getParentContainer();
  if (_c && _c->isDynamic() && compwnd) return 0;
  if (!MEMCMP(&myGUID, &GENERIC_GUID, sizeof(GUID))) return 1;
  return !MEMCMP(&myGUID, g, sizeof(GUID));
}

void ComponentObject::onBeforeGetWac(GUID g, CompWnd *c) {
//  if (!Std::isXPOrGreater())
  if (!c->handleDesktopAlpha()) 
    denyDesktopAlpha = 1;
  if (!c->handleTransparency()) 
    denyTransparency = 1;
  getGuiObject()->guiobject_getParentLayout()->onGuiObjectSetVisible(getGuiObject(), 1);
  WACObject *wo = SOM::getWACObject(g);
  script_vcpu_onGetWac(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_OBJECT(wo ? wo->getScriptObject() : NULL));
}

void ComponentObject::onBeforeGiveUpWac(GUID g, CompWnd *c) {
  WACObject *wo = SOM::getWACObject(g);
  script_vcpu_onGiveUpWac(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_OBJECT(wo ? wo->getScriptObject() : NULL));
}

int ComponentObject::getAutoClose() {
  return autoclose;
}

void ComponentObject::setAcceptWac(int a) {
  accept = a;
}

int ComponentObject::handleDesktopAlpha() {
  if (denyDesktopAlpha) return 0;
  if (!compwnd) return 1;
  return 0;
}

int ComponentObject::handleTransparency() {
  if (denyTransparency) return 0;
  if (!compwnd) return 1;
  return compwnd->handleTransparency();
}

int ComponentObject::getPreferences(int what) {
  if (compwnd) return compwnd->getPreferences(what);
  return COMPONENTOBJECT_PARENT::getPreferences(what);
}

scriptVar ComponentObject::script_vcpu_getGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 

  OLECHAR oguid[256] = {0}; // NONPORTABLE
  ComponentObject *co = static_cast<ComponentObject *>(o->vcpu_getInterface(componentObjectGuid));
  if (co) {
    static char guid[256];
    StringFromGUID2(((ComponentObject *)o)->myGUID, oguid, 256);
    wsprintf(guid, "%$", oguid);
    return MAKE_SCRIPT_STRING(guid);
  }
  return MAKE_SCRIPT_STRING("");
}

scriptVar ComponentObject::script_vcpu_getWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; ;
  ComponentObject *co = static_cast<ComponentObject *>(o->vcpu_getInterface(componentObjectGuid));
  return MAKE_SCRIPT_OBJECT(co ? SOM::getWACObject(co->getGUID())->getScriptObject() : NULL);
}

scriptVar ComponentObject::script_vcpu_setRegionFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv) {
  SCRIPT_FUNCTION_INIT; ;
  ASSERT(SOM::isNumeric(&byte));
  ASSERT(SOM::isNumeric(&inv));
  ComponentObject *co = static_cast<ComponentObject *>(o->vcpu_getInterface(componentObjectGuid));
  SMap *sm = static_cast<SMap *>(GET_SCRIPT_OBJECT_AS(map, mapGuid));
  co->script_setRegionFromMap(sm, SOM::makeInt(&byte), SOM::makeBoolean(&inv));
  RETURN_SCRIPT_VOID;  
}

scriptVar ComponentObject::script_vcpu_setRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r) {
  SCRIPT_FUNCTION_INIT; ;
  ComponentObject *co = static_cast<ComponentObject *>(o->vcpu_getInterface(componentObjectGuid));
  SRegion *reg = static_cast<SRegion *>(GET_SCRIPT_OBJECT_AS(r, regionGuid));
  if (co) co->script_setRegion(reg);
  RETURN_SCRIPT_VOID;  
}

scriptVar ComponentObject::script_vcpu_onGetWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar wac) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, compoController, wac); 
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, wac);
}

scriptVar ComponentObject::script_vcpu_onGiveUpWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar wac) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, compoController, wac);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, wac);
}

scriptVar ComponentObject::script_vcpu_setAcceptWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar on) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&on));
  ComponentObject *co = static_cast<ComponentObject *>(o->vcpu_getInterface(componentObjectGuid));
  if (co) co->setAcceptWac(GET_SCRIPT_BOOLEAN(on));
  RETURN_SCRIPT_VOID;
}


