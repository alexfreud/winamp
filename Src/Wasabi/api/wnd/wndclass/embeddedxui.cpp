#include "precomp.h"
#include "embeddedxui.h"

EmbeddedXuiObject::EmbeddedXuiObject() {
  embedded = NULL;
  myxuihandle = newXuiHandle();
  getScriptObject()->vcpu_setInterface(embeddedXuiGuid, (void *)static_cast<EmbeddedXuiObject *>(this));
  getScriptObject()->vcpu_setClassName(L"ObjectEmbedded"); // this is the script class name
  getScriptObject()->vcpu_setController(embeddedXuiController);
}

EmbeddedXuiObject::~EmbeddedXuiObject() {
  paramlist.deleteAll();
}

void EmbeddedXuiObject::onNewContent() {
  embeddedxui_onNewEmbeddedContent();
}

void EmbeddedXuiObject::embeddedxui_onNewEmbeddedContent() {
  embedded = NULL;
  const wchar_t *id = embeddedxui_getEmbeddedObjectId();
  if (id != NULL && *id) {
    GuiObject *myself = getGuiObject();
    embedded = myself->guiobject_findObject(id);  
    if (embedded != NULL && embedded != myself) {
      foreach(paramlist)
        EmbeddedXuiObjectParam *p = paramlist.getfor();
        embedded->guiobject_setXmlParam(p->param, p->value);
      endfor;
#ifdef WASABI_COMPILE_CONFIG
      syncCfgAttrib();
#endif
    }
  }
}

int EmbeddedXuiObject::onUnknownXuiParam(const wchar_t *xmlattributename, const wchar_t *value) {
  int r = EMBEDDEDXUIOBJECT_PARENT::onUnknownXuiParam(xmlattributename, value);
  paramlist.addItem(new EmbeddedXuiObjectParam(xmlattributename, value));
  if (embedded)
    r = embedded->guiobject_setXmlParam(xmlattributename, value);
  return r;
}

int EmbeddedXuiObject::onInit()
{
  int r = EMBEDDEDXUIOBJECT_PARENT::onInit();
  const wchar_t *id = embeddedxui_getContentId();
  if (id != NULL && *id)
    setContent(id);
  return r;
}

#ifdef WASABI_COMPILE_CONFIG
int EmbeddedXuiObject::onReloadConfig() {
  int r = EMBEDDEDXUIOBJECT_PARENT::onReloadConfig();
  syncCfgAttrib();
  return r;
}
#endif

#ifdef WASABI_COMPILE_CONFIG
void EmbeddedXuiObject::syncCfgAttrib() 
{
  if (embedded == NULL) return;
  CfgItem *item = getGuiObject()->guiobject_getCfgItem();
  const wchar_t *attrib = getGuiObject()->guiobject_getCfgAttrib();
  if (item != embedded->guiobject_getCfgItem() ||
      attrib != embedded->guiobject_getCfgAttrib()) {
    embedded->guiobject_setCfgAttrib(item, attrib);
  }
}
#endif

// -----------------------------------------------------------------------
// Script Object

EmbeddedXuiScriptController _embeddedXuiController;
EmbeddedXuiScriptController *embeddedXuiController = &_embeddedXuiController;

// -- Functions table -------------------------------------
function_descriptor_struct EmbeddedXuiScriptController::exportedFunction[] = {
  {L"getEmbeddedObject",       0, (void*)EmbeddedXuiScriptController::EmbeddedXui_getEmbeddedObject},
};
                                      
ScriptObject *EmbeddedXuiScriptController::instantiate() {
  EmbeddedXuiObject *ex = new EmbeddedXuiObject;
  ASSERT(ex != NULL);
  return ex->getScriptObject();
}

void EmbeddedXuiScriptController::destroy(ScriptObject *o) {
  EmbeddedXuiObject *ex= static_cast<EmbeddedXuiObject *>(o->vcpu_getInterface(embeddedXuiGuid));
  ASSERT(ex != NULL);
  delete ex;
}

void *EmbeddedXuiScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for DropDownlist yet
}

void EmbeddedXuiScriptController::deencapsulate(void *o) {
}

int EmbeddedXuiScriptController::getNumFunctions() { 
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *EmbeddedXuiScriptController::getExportedFunctions() { 
  return exportedFunction; 
}


scriptVar EmbeddedXuiScriptController::EmbeddedXui_getEmbeddedObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  EmbeddedXuiObject *ex = static_cast<EmbeddedXuiObject*>(o->vcpu_getInterface(embeddedXuiGuid));
  ScriptObject *_o = NULL;
  if (ex) {
    GuiObject *go = ex->embeddedxui_getEmbeddedObject();
    if (go != NULL)
      _o = go->guiobject_getScriptObject();
  }
  return MAKE_SCRIPT_OBJECT(_o);
}

ScriptObject *EmbeddedXuiScriptController::cast(ScriptObject *o, GUID g) {
  EmbeddedXuiObject *exo = static_cast<EmbeddedXuiObject *>(o->vcpu_getInterface(embeddedXuiGuid));
  if (!exo) return NULL;
  GuiObject *go = exo->embeddedxui_getEmbeddedObject();
  if (go != NULL) {
    ScriptObject *eo = go->guiobject_getScriptObject();
    if (eo != NULL) {
      void *i = eo->vcpu_getInterface(g);
      if (i != NULL)
        return eo;
    }
  }
  return NULL;
}


ScriptObjectController *EmbeddedXuiScriptController::getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }