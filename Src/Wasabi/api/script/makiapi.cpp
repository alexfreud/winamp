#include "precomp.h"
#include "makiapi.h"
#include "../../studio/api.h"
#include "../../bfc/api/api_maki.h"
#include "../../studio/script/vcpu.h"
#include "../../studio/script/objecttable.h"
#include "../../studio/script/scriptmgr.h"
#include "../../studio/script/script.h"
#include "../../common/script/scriptobj.h"

api_maki *makiApi = NULL;

ScriptApi::ScriptApi() {
  ObjectTable::loadExternalClasses();
  VCPU::scriptManager = new ScriptObjectManager();
}

ScriptApi::~ScriptApi() {
  delete VCPU::scriptManager;
  VCPU::scriptManager = NULL;
  VCPU::shutdown();
  ObjectTable::unloadExternalClasses();
}

void ScriptApi::maki_pushObject(void *o) {
  VCPU::pushObject(o);
}

void ScriptApi::maki_pushInt(int i) {
  VCPU::pushInt(i);
}

void ScriptApi::maki_pushBoolean(int b) {
  VCPU::pushBoolean(b);
}

void ScriptApi::maki_pushFloat(float f) {
  VCPU::pushFloat(f);
}

void ScriptApi::maki_pushDouble(double d) {
  VCPU::pushDouble(d);
}

void ScriptApi::maki_pushString(const char *s) {
  VCPU::pushString(s);
}

void ScriptApi::maki_pushAny(scriptVar v) {
  VCPU::push(v);
}

void ScriptApi::maki_pushVoid() {
  VCPU::pushVoid();
}

void *ScriptApi::maki_popObject() {
  return VCPU::popObject();
}

int ScriptApi::maki_popInt() {
  return VCPU::popInt();
}

bool ScriptApi::maki_popBoolean() {
  return VCPU::popBoolean();
}

float ScriptApi::maki_popFloat() {
  return VCPU::popFloat();
}

double ScriptApi::maki_popDouble() {
  return VCPU::popDouble();
}

const char *ScriptApi::maki_popString() {
  return VCPU::popString();
}

scriptVar ScriptApi::maki_popAny() {
  return VCPU::pop().v;
}

void ScriptApi::maki_popDiscard() {
  VCPU::popDiscard();
}

const char *ScriptApi::maki_getFunction(int dlfid, int *nparams, ScriptObjectController **p) {
  return ObjectTable::getFunction(dlfid, nparams, p);
}

int ScriptApi::maki_addDlfRef(ScriptObjectController *o, const char *function_name, void *host) {
  return ObjectTable::dlfAddRef(o, function_name, host);
}

void ScriptApi::maki_remDlfRef(void *host) {
  ObjectTable::dlfRemRef(host);
}

void ScriptApi::maki_addDlfClassRef(ScriptObjectController *o, void *host) {
  ObjectTable::dlfAddClassRef(o, host);
}

scriptVar ScriptApi::maki_callFunction(ScriptObject *o, int dlfid, scriptVar **params) {
  return ObjectTable::callFunction(o, dlfid, params);
}

scriptVar ScriptApi::maki_triggerEvent(ScriptObject *o, int dlfid, int np, int scriptid) {
  scriptVar v = MAKE_SCRIPT_OBJECT(o);
  return VCPU::executeEvent(v, dlfid, np, scriptid);
}

ScriptObject *ScriptApi::maki_instantiate(GUID classguid) {
  int classid = ObjectTable::getClassFromGuid(classguid);
  if (classid == -1) return NULL;
  return ObjectTable::instantiate(classid);
}

void ScriptApi::maki_destroy(ScriptObject *o) {
  ObjectTable::destroy(o);
}

void *ScriptApi::maki_encapsulate(GUID classguid, ScriptObject *o) {
  int classid = ObjectTable::getClassFromGuid(classguid);
  if (classid == -1) return NULL;
  return ObjectTable::encapsulate(classid, o);
}

void ScriptApi::maki_deencapsulate(GUID classguid, void *o) {
  int classid = ObjectTable::getClassFromGuid(classguid);
  if (classid == -1) return;
  ObjectTable::deencapsulate(classid, o);
}

ScriptObjectController *ScriptApi::maki_getController(GUID scriptclass) {
  return ObjectTable::getController(scriptclass);
}
int ScriptApi::maki_createOrphan(int type) {
  return VCPU::createOrphan(type);
}

void ScriptApi::maki_killOrphan(int id) {
  VCPU::killOrphan(id);
}

int ScriptApi::maki_getScriptInt(scriptVar v) {
  return SOM::makeInt(&v);
}

bool ScriptApi::maki_getScriptBoolean(scriptVar v) {
  return SOM::makeBoolean(&v);
}

float ScriptApi::maki_getScriptFloat(scriptVar v) {
  return SOM::makeFloat(&v);
}

double ScriptApi::maki_getScriptDouble(scriptVar v) {
  return SOM::makeDouble(&v);
}

const char *ScriptApi::maki_getScriptString(scriptVar v) {
  ASSERT(v.type == SCRIPT_STRING);
  return v.data.sdata;
}

ScriptObject *ScriptApi::maki_getScriptObject(scriptVar v) {
  ASSERT(!SOM::isNumeric(&v) && v.type != SCRIPT_STRING);
  return v.data.odata;
}

scriptVar ScriptApi::maki_updateDlf(maki_cmd *cmd, int *dlfid, int *linkcount) {
  switch (cmd->cmd) {
    case MAKI_CMD_ADDREF:
      (*linkcount)++;
      break;
    case MAKI_CMD_REMREF:
      (*linkcount)--;
      ASSERT(*linkcount >= 0);
      if (*linkcount == 0)
        *dlfid = -1;
      break;
    case MAKI_CMD_SETDLF:
      ASSERT(*dlfid == -1);
      *dlfid = cmd->id;
      break;
    case MAKI_CMD_GETDLF:
      cmd->id = *dlfid;
      break;
  }
  RETURN_SCRIPT_VOID
}

void ScriptApi::maki_setObjectAtom(const char *atomname, ScriptObject *object) {
  VCPU::setAtom(atomname, object);
}

ScriptObject *ScriptApi::maki_getObjectAtom(const char *atomname) {
  return VCPU::getAtom(atomname);
}

#ifdef WASABI_COMPILE_WND
ScriptObject *ScriptApi::maki_findObject(const char *name) {
  return ScriptObjectManager::findObject(name);
}
#endif

void ScriptApi::vcpu_addScriptObject(ScriptObject *o) {
  SystemObject::addScriptObject(o);
}

void ScriptApi::vcpu_removeScriptObject(ScriptObject *o) {
  SystemObject::removeScriptObject(o);
}

int ScriptApi::vcpu_getCacheCount() {
  return Script::getCacheCount();
}

int ScriptApi::vcpu_isValidScriptId(int id) {
  return Script::isValidScriptId(id);
}

int ScriptApi::vcpu_mapVarId(int varid, int scriptid) {
  return Script::varIdToGlobal(varid, scriptid);
}

int ScriptApi::vcpu_getUserAncestorId(int varid, int scriptid) {
  return Script::getUserAncestor(varid, scriptid);
}

int ScriptApi::vcpu_getNumEvents() {
  return Script::getNumEventsLinked();
}

int ScriptApi::vcpu_getEvent(int event, int *dlf, int *script, int *var) {
  return Script::getLinkedEventParams(event, dlf, script, var);
}

int ScriptApi::vcpu_getComplete() {
  return VCPU::getComplete();
}

const char * ScriptApi::vcpu_getClassName(int vcpuid, int localclassid) {
  return VCPU::getClassName(vcpuid, localclassid);
}

