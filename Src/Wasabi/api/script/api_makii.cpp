#include <precomp.h>
#include "api_makii.h"

#include <api/script/vcpu.h>
#include <api/script/objecttable.h>
#include <api/script/scriptmgr.h>
#include <api/script/script.h>
#include <api/script/scriptobj.h>

api_maki *makiApi = NULL;

api_makiI::api_makiI() {
  VCPU::scriptManager = new ScriptObjectManager();
}

api_makiI::~api_makiI() {
  delete VCPU::scriptManager;
  VCPU::scriptManager = NULL;
  VCPU::shutdown();
  ObjectTable::unloadExternalClasses();
}

void api_makiI::init() {
  ObjectTable::loadExternalClasses();
}

void api_makiI::maki_pushObject(void *o) {
  VCPU::pushObject(o);
}

void api_makiI::maki_pushInt(int i) {
  VCPU::pushInt(i);
}

void api_makiI::maki_pushBoolean(int b) {
  VCPU::pushBoolean(b);
}

void api_makiI::maki_pushFloat(float f) {
  VCPU::pushFloat(f);
}

void api_makiI::maki_pushDouble(double d) {
  VCPU::pushDouble(d);
}

void api_makiI::maki_pushString(const wchar_t *s) {
  VCPU::pushString(s);
}

void api_makiI::maki_pushVoid() {
  VCPU::pushVoid();
}

void api_makiI::maki_pushAny(scriptVar v) {
  VCPU::push(v);
}

void *api_makiI::maki_popObject() {
  return VCPU::popObject();
}

int api_makiI::maki_popInt() {
  return VCPU::popInt();
}

bool api_makiI::maki_popBoolean() {
  return VCPU::popBoolean();
}

float api_makiI::maki_popFloat() {
  return VCPU::popFloat();
}

double api_makiI::maki_popDouble() {
  return VCPU::popDouble();
}

const wchar_t *api_makiI::maki_popString() {
  return VCPU::popString();
}

scriptVar api_makiI::maki_popAny() {
  return VCPU::pop().v;
}

void api_makiI::maki_popDiscard() {
  VCPU::popDiscard();
}

const wchar_t *api_makiI::maki_getFunction(int dlfid, int *nparams, ScriptObjectController **p) {
  return ObjectTable::getFunction(dlfid, nparams, p);
}

int api_makiI::maki_addDlfRef(ScriptObjectController *o, const wchar_t *function_name, void *host) {
  return ObjectTable::dlfAddRef(o, function_name, host);
}

void api_makiI::maki_addDlfClassRef(ScriptObjectController *o, void *host) {
  ObjectTable::dlfAddClassRef(o, host);
}

void api_makiI::maki_remDlfRef(void *host) {
  ObjectTable::dlfRemRef(host);
}

scriptVar api_makiI::maki_callFunction(ScriptObject *o, int dlfid, scriptVar **params) {
  return ObjectTable::callFunction(o, dlfid, params);
}

scriptVar api_makiI::maki_triggerEvent(ScriptObject *o, int dlfid, int np, int scriptid) {
  scriptVar v = MAKE_SCRIPT_OBJECT(o);
  return VCPU::executeEvent(v, dlfid, np, scriptid);
}

int api_makiI::maki_getScriptInt(scriptVar v) {
  return SOM::makeInt(&v);
}

bool api_makiI::maki_getScriptBoolean(scriptVar v) {
  return SOM::makeBoolean(&v);
}

float api_makiI::maki_getScriptFloat(scriptVar v) {
  return SOM::makeFloat(&v);
}

double api_makiI::maki_getScriptDouble(scriptVar v) {
  return SOM::makeDouble(&v);
}

const wchar_t *api_makiI::maki_getScriptString(scriptVar v) 
{
  ASSERT(v.type == SCRIPT_STRING);
  return v.data.sdata;
}

ScriptObject *api_makiI::maki_getScriptObject(scriptVar v) {
  ASSERT(!SOM::isNumeric(&v) && v.type != SCRIPT_STRING);
  return v.data.odata;
}

scriptVar api_makiI::maki_updateDlf(maki_cmd *cmd, int *dlfid, int *linkcount) {
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
    case MAKI_CMD_RESETDLF:
      *dlfid = -1;
      break;
  }
  RETURN_SCRIPT_VOID
}

ScriptObject *api_makiI::maki_instantiate(GUID classguid) {
  int classid = ObjectTable::getClassFromGuid(classguid);
  if (classid == -1) return NULL;
  return ObjectTable::instantiate(classid);
}

void api_makiI::maki_destroy(ScriptObject *o) {
  ObjectTable::destroy(o);
}

void *api_makiI::maki_encapsulate(GUID classguid, ScriptObject *o) {
  int classid = ObjectTable::getClassFromGuid(classguid);
  if (classid == -1) return NULL;
  return ObjectTable::encapsulate(classid, o);
}

void api_makiI::maki_deencapsulate(GUID classguid, void *o) {
  int classid = ObjectTable::getClassFromGuid(classguid);
  if (classid == -1) return;
  ObjectTable::deencapsulate(classid, o);
}

ScriptObjectController *api_makiI::maki_getController(GUID scriptclass) {
  return ObjectTable::getController(scriptclass);
}

int api_makiI::maki_createOrphan(int type) {
  return VCPU::createOrphan(type);
}

void api_makiI::maki_killOrphan(int id) {
  VCPU::killOrphan(id);
}

void api_makiI::maki_setObjectAtom(const wchar_t *atomname, ScriptObject *object) {
  VCPU::setAtom(atomname, object);
}

ScriptObject *api_makiI::maki_getObjectAtom(const wchar_t *atomname) {
  return VCPU::getAtom(atomname);
}

#ifdef WASABI_COMPILE_WND
ScriptObject *api_makiI::maki_findObject(const wchar_t *name) 
{
  return ScriptObjectManager::findObject(name);
}
#endif

void api_makiI::vcpu_addScriptObject(ScriptObject *o) {
  SystemObject::addScriptObject(o);
}

void api_makiI::vcpu_removeScriptObject(ScriptObject *o) {
  SystemObject::removeScriptObject(o);
}

int api_makiI::vcpu_getCacheCount() {
  return Script::getCacheCount();
}

int api_makiI::vcpu_isValidScriptId(int id) {
  return Script::isValidScriptId(id);
}

int api_makiI::vcpu_mapVarId(int varid, int scriptid) {
  return Script::varIdToGlobal(varid, scriptid);
}

int api_makiI::vcpu_getUserAncestorId(int varid, int scriptid) {
  return Script::getUserAncestor(varid, scriptid);
}

int api_makiI::vcpu_getNumEvents() {
  return Script::getNumEventsLinked();
}

int api_makiI::vcpu_getEvent(int event, int *dlf, int *script, int *var) {
  return Script::getLinkedEventParams(event, dlf, script, var);
}

int api_makiI::vcpu_getComplete() {
  return VCPU::getComplete();
}

void api_makiI::vcpu_setComplete() {
  VCPU::setComplete();
}

void api_makiI::vcpu_resetComplete() {
  VCPU::resetComplete();
}

const wchar_t *api_makiI::vcpu_getClassName(int vcpuid, int localclassid) {
  return VCPU::getClassName(vcpuid, localclassid);
}

