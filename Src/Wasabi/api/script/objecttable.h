#ifndef __OBJECTTABLE
#define __OBJECTTABLE

#ifdef __cplusplus

#include <wasabicfg.h>

#include <bfc/ptrlist.h>
class ScriptObjectController;

#ifdef _NOSTUDIO
//CUT #include "../../scriptcompiler/compiler.h"
//CUT #include "../../scriptcompiler/ctrlblock.h"
#endif

#endif

#include <api/script/vcputypes.h>

#define CLASS_ID_BASE 0x100


#define MAKE_NEW_OBJECT(class, objclass) \
    case class: \
      s = new objclass; \
      objclass::instantiate((objclass *)s); \
      break; 
  
// For type names table
typedef struct {
  wchar_t *name;
  int type;
  int instantiable;
  int referenceable;
} typenames;

// This is the table that link type names as they are
// recognized in scripts to actual basic types.

// Non instantiable means you can't do v = new <type>
// Non referenceable means you can't do <type> v;
// Not a ref means content is not a reference to an object but rather the actual
//  content itself
//  * special case for strings which is a ref to a char *, but not an object
#define DEFINE_TYPES \
typenames types[] = { \
  {L"", SCRIPT_VOID, 0, 0},                /* Non instantiable, non referenceable */ \
  {L"Event", SCRIPT_EVENT, 0, 0},           /* Non instantiable, non referenceable */ \
  {L"Int", SCRIPT_INT, 0, 1},              /* Not a ref */ \
  {L"Float", SCRIPT_FLOAT, 0, 1},          /* Not a ref */ \
  {L"Double", SCRIPT_DOUBLE, 0, 1},        /* Not a ref */ \
  {L"Boolean", SCRIPT_BOOLEAN, 0, 1},      /* Not a ref */ \
  {L"String", SCRIPT_STRING, 0, 1},        /* Not a ref (* special case) */ \
  {L"Any", SCRIPT_ANY, 0, 1},           /* Non instantiable, non referenceable */ \
}; \
int ntypes = sizeof(types)/sizeof(typenames);

extern typenames types[];
extern int ntypes;

#define SVC_CLASS_BASE 1024

#ifdef __cplusplus

class class_entry {
public:
  const wchar_t *classname;
  int classid;
  int ancestorclassid;
  ScriptObjectController *controller;
  GUID classGuid;
  int instantiable;
  int referenceable;
  int external;
  waServiceFactory *sf;
};

typedef struct {
  void *ptr;
  int nargs;
  void *host;
} hostrefstruct;

class SystemObject;

class ObjectTable {

  public : 

  static void start(); // initialize tables, internal objects...
  static void shutdown(); // free tables
  static void unloadExternalClasses(); // unload external classes
  static void loadExternalClasses(); // reload external classes

  static int registerClass(ScriptObjectController *c, waServiceFactory *sf = NULL); // returns classid. ancestorclass = 0 = Object

//    static void instantiate(ScriptObject *o, int classid);
  static int addrefDLF(VCPUdlfEntry *dlf, int id);
  static void delrefDLF(VCPUdlfEntry *dlf);
  static void resetDLF(VCPUdlfEntry *dlf);

  static int getClassFromName(const wchar_t *classname);
  static int getClassFromGuid(GUID g);
  static const wchar_t *getClassName(int classid);
  static int isExternal(int classid);
  static int getNumGuids();
  static GUID enumGuid(int i);
  static const wchar_t *enumClassName(int n);
  static int getClassEntryIdx(int classid);
  static int isDescendant(int class1, int classid);
  static int isClassInstantiable(int classid);
  static int isClassReferenceable(int classid);
  static ScriptObject *instantiate(int classid);
  static void *encapsulate(int classid, ScriptObject *o);
  static void destroy(ScriptObject *o);
  static void deencapsulate(int classid, void *o);
  static const wchar_t *getFunction(int dlfid, int *n, ScriptObjectController **p);
  static scriptVar callFunction(ScriptObject *obj, int dlfid, scriptVar **params);
  static int dlfAddRef(ScriptObjectController *o, const wchar_t *function_name, void *host);
  static int dlfAddRef(ScriptObjectController *o, int i, void *host);
  static void dlfAddClassRef(ScriptObjectController *o, void *host);
  static void dlfRemRef(void *host);
  static int checkScript(SystemObject *o);
  static ScriptObjectController *getController(GUID g);
  static class_entry *getClassEntry(int classid);
  static void unlinkClass(class_entry *e);

#ifdef _NOSTUDIO
  static int validateMember(int classid, wchar_t *member, ControlBlock *parms, int *ret);
#endif

  private:

  static PtrList < class_entry > classes;
  static PtrList < hostrefstruct > hostrefs;
  static int classidx;
  static int externalloaded;
};

#endif

#endif


