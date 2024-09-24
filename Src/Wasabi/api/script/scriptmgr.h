#ifndef _SCRIPT_H
#define _SCRIPT_H

#include <api/script/objects/systemobj.h>
#ifdef WASABI_COMPILE_CONFIG
#include <api/script/objects/wacobj.h>
#endif

#define SOM ScriptObjectManager

// This class should ONLY contain generic functions which don't have to be duplicated for each script layer !
// This is why this is (mostly) a static class. Please DO NOT add anything which relates to function pointers,
// variable tables, etc.


class ScriptObjectManager {
public:
  ScriptObjectManager();
  ~ScriptObjectManager();

  static scriptVar makeVar(int type);
  static scriptVar makeVar(int type, ScriptObject *o);
  static void assign(scriptVar *v, const wchar_t *str);
  static void assign(scriptVar *v, int i);
  static void assign(scriptVar *v, float f);
  static void assign(scriptVar *v, double d);
  static void assign(scriptVar *v, ScriptObject *o);
  static void assign(scriptVar *v1, scriptVar *v2);
	static void assignPersistent(scriptVar *v1, scriptVar *v2);
  static void strflatassign(scriptVar *v, const wchar_t *str);
  static void persistentstrassign(scriptVar *v, const wchar_t *str);

  static int compEq(scriptVar *v1, scriptVar *v2);
  static int compNeq(scriptVar *v1, scriptVar *v2);
  static int compA(scriptVar *v1, scriptVar *v2);
  static int compAe(scriptVar *v1, scriptVar *v2);
  static int compB(scriptVar *v1, scriptVar *v2);
  static int compBe(scriptVar *v1, scriptVar *v2);

  static void mid(wchar_t *dest, const wchar_t *str, int s, int l);

  static int makeInt(scriptVar *v);
  static float makeFloat(scriptVar *v);
  static double makeDouble(scriptVar *v);
  static bool makeBoolean(scriptVar *v);
  static int isNumeric(scriptVar *s);
  static int isString(scriptVar *s);
  static int isVoid(scriptVar *s);
  static int isObject(scriptVar *s);
  static int isNumericType(int t);

#ifdef WASABI_COMPILE_COMPONENTS
  static WACObject *getWACObject(const wchar_t *guid);
  static WACObject *getWACObject(GUID cg);
#endif
  static SystemObject *getSystemObject(int n);
  static SystemObject *getSystemObjectByScriptId(int id);

  static void registerSystemObject(SystemObject *o);
  static void unregisterSystemObject(SystemObject *o);
  static int getNumSystemObjects();
  static SystemObject *enumSystemObject(int n);
  static int typeCheck(VCPUscriptVar *v, int fail = 1);
  static WindowHolder *getSuitableWindowHolderFromScript(GUID g);
  static int checkAbortShowHideWindow(GUID g, int visible);
#ifdef WASABI_COMPILE_WND
  static ScriptObject *findObject(const wchar_t *name);
#endif

private:

  static SystemObject * system;
  static PtrList < SystemObject > syslist;
  static int inited;
};

#ifdef WASABI_COMPILE_COMPONENTS
extern PtrList<WACObject> comps;
#endif

#endif