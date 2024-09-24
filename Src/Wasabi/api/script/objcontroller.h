#ifndef __SCRIPTOBJECTCONTROLLER_H
#define __SCRIPTOBJECTCONTROLLER_H

#include <api/script/scriptguid.h>

//#include <wasabicfg.h>

#include <bfc/ptrlist.h>
#include <bfc/dispatch.h>
#include <api/script/scriptobj.h>
#include <api/script/vcputypes.h>
#include <api/service/svcs/svc_scriptobj.h>

#define SCRIPT_MAXARGS    10 // 10 args max per function

#define MAKI_CMD_NONE   0
#define MAKI_CMD_SETDLF 1
#define MAKI_CMD_GETDLF 2
#define MAKI_CMD_ADDREF 3
#define MAKI_CMD_REMREF 4
#define MAKI_CMD_RESETDLF 5

#define SCRIPT_FUNCTION_INIT static int __dlfid=-1, __linkcount=0; if (__cmd != NULL) return WASABI_API_MAKI->maki_updateDlf(__cmd, &__dlfid, &__linkcount);
#define SCRIPT_FUNCTION_CHECKABORTEVENT if (__dlfid==-1) RETURN_SCRIPT_VOID;
#define SCRIPT_FUNCTION_CHECKABORTEVENT_SYS(o) { SCRIPT_FUNCTION_CHECKABORTEVENT { SystemObject *so = static_cast<SystemObject *>(o->vcpu_getInterface(systemObjectGuid)); if (!so || !so->isLoaded()) RETURN_SCRIPT_VOID; } }
#define PROCESS_HOOKS0(object, controller) { controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, NULL, 0); }
#define PROCESS_HOOKS1(object, controller, p1) { scriptVar *__table[1] = {&p1}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 1); }
#define PROCESS_HOOKS2(object, controller, p1, p2) { scriptVar *__table[2] = {&p1, &p2}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 2); }
#define PROCESS_HOOKS3(object, controller, p1, p2, p3) { scriptVar *__table[3] = {&p1, &p2, &p3}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 3); }
#define PROCESS_HOOKS4(object, controller, p1, p2, p3, p4) { scriptVar *__table[4] = {&p1, &p2, &p3, &p4}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 4); }
#define PROCESS_HOOKS5(object, controller, p1, p2, p3, p4, p5) { scriptVar *__table[5] = {&p1, &p2, &p3, &p4, &p5}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 5); }
#define PROCESS_HOOKS6(object, controller, p1, p2, p3, p4, p5, p6) { scriptVar *__table[6] = {&p1, &p2, &p3, &p4, &p5, &p6}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 6); }
#define PROCESS_HOOKS7(object, controller, p1, p2, p3, p4, p5, p6, p7) { scriptVar *__table[7] = {&p1, &p2, &p3, &p4, &p5, &p6, &p7}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 7); }
#define PROCESS_HOOKS8(object, controller, p1, p2, p3, p4, p5, p6, p7, p8) { scriptVar *__table[8] = {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 8); }
#define PROCESS_HOOKS9(object, controller, p1, p2, p3, p4, p5, p6, p7, p8, p9) { scriptVar *__table[9] = {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 9); }
#define PROCESS_HOOKS10(object, controller, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) { scriptVar *__table[10] = {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10}; controller->processHooks(static_cast<ScriptObject *>(object), DLF_ID, __table, 10); }
#define SCRIPT_EXEC_EVENT0(object) { return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 0); }
#define SCRIPT_EXEC_EVENT1(object, p1) { WASABI_API_MAKI->maki_pushAny(p1); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 1); }
#define SCRIPT_EXEC_EVENT2(object, p1, p2) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 2); }
#define SCRIPT_EXEC_EVENT3(object, p1, p2, p3) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); WASABI_API_MAKI->maki_pushAny(p3); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 3); }
#define SCRIPT_EXEC_EVENT4(object, p1, p2, p3, p4) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); WASABI_API_MAKI->maki_pushAny(p3); WASABI_API_MAKI->maki_pushAny(p4); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 4); }
#define SCRIPT_EXEC_EVENT5(object, p1, p2, p3, p4, p5) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); WASABI_API_MAKI->maki_pushAny(p3); WASABI_API_MAKI->maki_pushAny(p4); WASABI_API_MAKI->maki_pushAny(p5); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 5); }
#define SCRIPT_EXEC_EVENT6(object, p1, p2, p3, p4, p5, p6) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); WASABI_API_MAKI->maki_pushAny(p3); WASABI_API_MAKI->maki_pushAny(p4); WASABI_API_MAKI->maki_pushAny(p5); WASABI_API_MAKI->maki_pushAny(p6); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 6); }
#define SCRIPT_EXEC_EVENT7(object, p1, p2, p3, p4, p5, p6, p7) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); WASABI_API_MAKI->maki_pushAny(p3); WASABI_API_MAKI->maki_pushAny(p4); WASABI_API_MAKI->maki_pushAny(p5); WASABI_API_MAKI->maki_pushAny(p6); WASABI_API_MAKI->maki_pushAny(p7); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 7); }
#define SCRIPT_EXEC_EVENT8(object, p1, p2, p3, p4, p5, p6, p7, p8) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); WASABI_API_MAKI->maki_pushAny(p3); WASABI_API_MAKI->maki_pushAny(p4); WASABI_API_MAKI->maki_pushAny(p5); WASABI_API_MAKI->maki_pushAny(p6); WASABI_API_MAKI->maki_pushAny(p7); WASABI_API_MAKI->maki_pushAny(p8); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 8); }
#define SCRIPT_EXEC_EVENT9(object, p1, p2, p3, p4, p5, p6, p7, p8, p9) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); WASABI_API_MAKI->maki_pushAny(p3); WASABI_API_MAKI->maki_pushAny(p4); WASABI_API_MAKI->maki_pushAny(p5); WASABI_API_MAKI->maki_pushAny(p6); WASABI_API_MAKI->maki_pushAny(p7); WASABI_API_MAKI->maki_pushAny(p8); WASABI_API_MAKI->maki_pushAny(p9); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 9); }
#define SCRIPT_EXEC_EVENT10(object, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10) { WASABI_API_MAKI->maki_pushAny(p1); WASABI_API_MAKI->maki_pushAny(p2); WASABI_API_MAKI->maki_pushAny(p3); WASABI_API_MAKI->maki_pushAny(p4); WASABI_API_MAKI->maki_pushAny(p5); WASABI_API_MAKI->maki_pushAny(p6); WASABI_API_MAKI->maki_pushAny(p7); WASABI_API_MAKI->maki_pushAny(p8); WASABI_API_MAKI->maki_pushAny(p9); WASABI_API_MAKI->maki_pushAny(p10); return WASABI_API_MAKI->maki_triggerEvent(object, DLF_ID, 10); }

#define SCRIPT_FUNCTION_PARAMS maki_cmd *__cmd, int __vsd
#define SCRIPT_CALL NULL, -1
#define GET_SCRIPT_INT(v) WASABI_API_MAKI->maki_getScriptInt(v)
//CUT#define GET_SCRIPT_INT(v) ((v).getAsInt())
#define GET_SCRIPT_BOOLEAN(v) WASABI_API_MAKI->maki_getScriptBoolean(v)
#define GET_SCRIPT_FLOAT(v) WASABI_API_MAKI->maki_getScriptFloat(v)
#define GET_SCRIPT_DOUBLE(v) WASABI_API_MAKI->maki_getScriptDouble(v)
#define GET_SCRIPT_STRING(v) WASABI_API_MAKI->maki_getScriptString(v)
#define GET_SCRIPT_OBJECT(v) WASABI_API_MAKI->maki_getScriptObject(v)

scriptVar COMEXP MAKE_SCRIPT_INT(int i);
scriptVar COMEXP MAKE_SCRIPT_VOID();
scriptVar COMEXP MAKE_SCRIPT_FLOAT(float f);
scriptVar COMEXP MAKE_SCRIPT_DOUBLE(double d);
scriptVar COMEXP MAKE_SCRIPT_BOOLEAN(int b);
scriptVar COMEXP MAKE_SCRIPT_OBJECT(ScriptObject *o);
scriptVar COMEXP MAKE_SCRIPT_STRING(const wchar_t *s);
void COMEXP *GET_SCRIPT_OBJECT_AS(scriptVar v, GUID g);

#define FIXUP_FUNCTION_DLF SCRIPT_FUNCTION_INIT
/*#define FIXUP_FUNCTION_DLF \
static int fn_DLF=-1; \
if (fn_DLF == -1 && DLFid == -1) { RETURN_SCRIPT_ZERO } \
if (fn_DLF == -1 && DLFid != -1 && o == NULL) { \
  fn_DLF = DLFid; \
  RETURN_SCRIPT_VOID \
  } else if (DLFid != -1) { \
    ASSERTPR(0, "DLFId already set"); \
    RETURN_SCRIPT_VOID \
  }*/

#define DLF_ID __dlfid

#define RETURN_SCRIPT_EVENT \
{ scriptVar script_event_return={SCRIPT_EVENT,0}; \
return script_event_return; }

#define RETURN_SCRIPT_VOID \
{ scriptVar script_event_return={SCRIPT_VOID,0}; \
return script_event_return; }

#define RETURN_SCRIPT_ZERO \
{ scriptVar script_event_return={SCRIPT_INT,0}; \
return script_event_return; }

#define RETURN_SCRIPT_NULL \
{ scriptVar script_event_return={SCRIPT_OBJECT,NULL}; \
return script_event_return; }

#define SCRIPT_FUNCTION_INT(_class, func, call) \
scriptVar _class::func(int DLFid, ScriptObject *o) { \
  FIXUP_FUNCTION_DLF \
  ASSERT(o != NULL); \
  scriptVar s = SOM::makeVar(SCRIPT_INT); \
  SOM::assign(&s, ((_class *)o)->call()); \
  return s;\
}

#define DEC_SCRIPT_FUNCTION_INT(func1, func2) \
  static scriptVar func1(int DLFid, ScriptObject *o); \
  virtual int func2();


#define EVENT_ID __dlfid

typedef struct {
  const wchar_t *function_name;
  int nparams;
  void *physical_ptr;
} function_descriptor_struct;

typedef struct {
  ScriptHook *hook;
  ScriptObject *object;
} object_hook_struct;

static const GUID ROOT_GUID = 
{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };


class ScriptObjectController : public Dispatchable {
  protected:
    ScriptObjectController() {};

  public:

    void onRegisterClass(ScriptObjectController *rootController);
    GUID getClassGuid();
    const wchar_t *getClassName();
    const wchar_t *getAncestorClassName();
    ScriptObjectController *getAncestorController();
    int getNumFunctions();
    const function_descriptor_struct *getExportedFunctions();
    ScriptObject *instantiate();
    void destroy(ScriptObject *o);
    void *encapsulate(ScriptObject *o);
    void deencapsulate(void *o);
    ScriptObject *cast(ScriptObject *o, GUID g);
    void setClassId(int id);
    int getClassId();
    void setAncestorClassId(int id);
    int getAncestorClassId();
    int getInstantiable();
    int getReferenceable();
    int processHooks(ScriptObject *o, int dlfid, scriptVar **table, int nparams);
    void addClassHook(ScriptHook *h);
    void addObjectHook(ScriptHook *h, ScriptObject *o);
    void removeHooks(ScriptHook *h);

  enum {
    GETCLASSGUID          = 100,
    GETCLASSNAME	        = 200,
    GETANCESTORCLASSNAME	= 300,
    GETNUMFUNCTIONS	      = 400,
    GETEXPORTEDFUNCTIONS	= 500,
    INSTANTIATE		        = 600,
    DESTROY		            = 700,
    GETCLASSID            = 800,
    SETCLASSID            = 900,
    SETANCESTORCLASSID    = 1000,
    GETANCESTORCLASSID    = 1100,
    GETINSTANTIABLE       = 1200,
    GETREFERENCEABLE      = 1300,
    PROCESSHOOKS          = 1400,
    GETANCESTORCONTROLLER = 1500,
    ADDCLASSHOOK          = 1600,
    ADDOBJHOOK            = 1700,
    REMHOOKS              = 1800,
    ONREGISTERCLASS       = 1900,
    ENCAPSULATE           = 2000,
    DEENCAPSULATE         = 2100,
    CAST                  = 2200,
  };
};

inline GUID ScriptObjectController::getClassGuid() {
  return _call(GETCLASSGUID, ROOT_GUID);
}

inline const wchar_t *ScriptObjectController::getClassName() 
{
  return _call(GETCLASSNAME, (const wchar_t *)NULL);
}

inline const wchar_t *ScriptObjectController::getAncestorClassName() {
  return _call(GETANCESTORCLASSNAME, (const wchar_t *)NULL);
}

inline int ScriptObjectController::getNumFunctions() {
  return _call(GETNUMFUNCTIONS, 0);
}

inline const function_descriptor_struct *ScriptObjectController::getExportedFunctions() {
  return _call(GETEXPORTEDFUNCTIONS, (const function_descriptor_struct *)NULL);
}

inline ScriptObject *ScriptObjectController::instantiate() {
  return _call(INSTANTIATE, (ScriptObject *)NULL);
}

inline void ScriptObjectController::destroy(ScriptObject *o) {
  _voidcall(DESTROY, o);
}

inline int ScriptObjectController::getClassId() {
  return _call(GETCLASSID, 0);
}

inline void ScriptObjectController::setClassId(int id) {
  _voidcall(SETCLASSID, id);
}

inline int ScriptObjectController::getAncestorClassId() {
  return _call(GETANCESTORCLASSID, 0);
}

inline void ScriptObjectController::setAncestorClassId(int id) {
  _voidcall(SETANCESTORCLASSID, id);
}

inline int ScriptObjectController::getInstantiable() {
  return _call(GETINSTANTIABLE, 0);
}

inline int ScriptObjectController::getReferenceable() {
  return _call(GETREFERENCEABLE, 0);
}

inline ScriptObjectController *ScriptObjectController::getAncestorController() {
  return _call(GETANCESTORCONTROLLER, (ScriptObjectController *)NULL);
}

inline int ScriptObjectController::processHooks(ScriptObject *o, int dlfid, scriptVar **table, int nparams) {
  return _call(PROCESSHOOKS, 0, o, dlfid, table, nparams);
}

inline void ScriptObjectController::addClassHook(ScriptHook *h) {
  _voidcall(ADDCLASSHOOK, h);
}

inline void ScriptObjectController::addObjectHook(ScriptHook *h, ScriptObject *o) {
  _voidcall(ADDOBJHOOK, h, o);
}

inline void ScriptObjectController::removeHooks(ScriptHook *h) {
  _voidcall(REMHOOKS, h);
}

inline void ScriptObjectController::onRegisterClass(ScriptObjectController *rootController) {
  _voidcall(ONREGISTERCLASS, rootController);
}

inline void *ScriptObjectController::encapsulate(ScriptObject *o) {
  return _call(ENCAPSULATE, (void *) NULL, o);
}

inline void ScriptObjectController::deencapsulate(void *o) {
  _voidcall(DEENCAPSULATE, o);
}

inline ScriptObject *ScriptObjectController::cast(ScriptObject *o, GUID g) {
  return _call(CAST, (ScriptObject *)NULL, o, g);
}

class ScriptObjectControllerI : public ScriptObjectController {
  public:

    ScriptObjectControllerI(); 
    virtual ~ScriptObjectControllerI();

    virtual void onRegisterClass(ScriptObjectController *rootController);
    virtual GUID getClassGuid()=0;
    virtual const wchar_t *getClassName()=0;
    virtual const wchar_t *getAncestorClassName()=0;
    virtual ScriptObjectController *getAncestorController()=0;
    virtual int getNumFunctions()=0;
    virtual const function_descriptor_struct *getExportedFunctions()=0;
    virtual ScriptObject *instantiate()=0;
    virtual void destroy(ScriptObject *o)=0;
    virtual void *encapsulate(ScriptObject *o)=0;
    virtual void deencapsulate(void *o)=0;
    virtual ScriptObject *cast(ScriptObject *o, GUID g);

    virtual void setClassId(int id) { my_class_id = id; }
    virtual int getClassId() { return my_class_id; }
    virtual void setAncestorClassId(int id) { my_ancestor_class_id = id; }
    virtual int getAncestorClassId() { return my_ancestor_class_id; }
    virtual int getInstantiable() { return 1; };
    virtual int getReferenceable() { return 1; };
    virtual int processHooks(ScriptObject *o, int dlfid, scriptVar **table, int nparams);
    virtual void addClassHook(ScriptHook *h);
    virtual void addObjectHook(ScriptHook *h, ScriptObject *o);
    virtual void removeHooks(ScriptHook *h);

  private:
    int my_class_id;  
    int my_ancestor_class_id;

    PtrList<object_hook_struct> objhooks;
    PtrList<ScriptHook> classhooks;
    ScriptObjectController *rootController;
    int incast;

  protected:
    RECVS_DISPATCH;
};

#endif
