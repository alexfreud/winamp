#ifndef __VCPU_H
#define __VCPU_H

#include "script.h"
#include "opcodes.h"

#include <bfc/tlist.h>
#include <bfc/ptrlist.h>
#include <bfc/stack.h>
#include <bfc/critsec.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/script/objects/systemobj.h>

class ConsoleEnum;

#include <api/script/vcputypes.h>

class OrphanEntry {

  public:

    OrphanEntry(int _id, int type);
    virtual ~OrphanEntry() {}

    int id;
    VCPUscriptVar v;
};

class OrphanQuickSort {
  public:
    static int compareItem(void *p1, void* p2);
    static int compareAttrib(const wchar_t *attrib, void *item);
  };

class ScriptAtom {
  public:
    ScriptAtom(const wchar_t *atomname, ScriptObject *object) : name(atomname), obj(object) {}
    ~ScriptAtom() {}

    const wchar_t *getAtomName() { return name; }
    ScriptObject *getAtomObject() { return obj; }

  private:
    StringW name;
    ScriptObject *obj;
};

class ScriptAtomSort {
public:
  static int compareItem(ScriptAtom *p1, ScriptAtom*p2) {
    return WCSICMP(p1->getAtomName(), p2->getAtomName());
  }
  static int compareAttrib(const wchar_t *attrib, ScriptAtom *item) {
    return WCSICMP(attrib, item->getAtomName());
  }
};


class VCPU {

public:
  static void shutdown();
  static int assignNewScriptId();
  static int addScript(void *mem, int memsize, int cpuid);
  static void removeScript(int id);

  static void push(VCPUscriptVar v);
  static void push(scriptVar v);
  static VCPUscriptVar pop();
  static VCPUscriptVar peekAt(int n);

  static scriptVar executeEvent(scriptVar v, int functionId, int np, int vcpuid=-1);
  static int runEvent(VCPUeventEntry *e, int np, int pbase);
  static void runCode(int scriptId, int pointer, int np);
  static scriptVar callDLF(VCPUdlfEntry *e, int np);
  static scriptVar VCPUassign(int id, scriptVar sv, int scriptId);

  static int findObject(ScriptObject *o, int start, int dlfid, int vcpuid=-1);
  static int numScripts;
  static void setupDLF(VCPUdlfEntry *e, int base);
  static void delrefDLF(VCPUdlfEntry *e);
  static void setupDLFFunction(void *ptr, int nargs, int DLFid, VCPUdlfEntry *e);
  static int getDlfGlobalIndex(int dlfid, int scriptid);

  static scriptVar safeDiv(VCPUscriptVar*, VCPUscriptVar*);
  static void setComplete() { complete=1; }
  static void resetComplete() { complete=0; }
  static int getComplete();
  static int newDlf();
  static void resetDlf();

  static PtrList<VCPUscriptVar> variablesTable;
  static PtrList<VCPUeventEntry> eventsTable;
  static PtrList<VCPUdlfEntry> DLFentryTable;
  static PtrList<VCPUcodeBlock> codeTable;

  static int highestDLFId;
  static Stack<VCPUscriptVar> CpuStack;
  static Stack<char *> CallStack;

  static int varBase(int scriptId);
  static int dlfBase(int scriptId);
  static int nVars(int scriptId);
  static char *getCodeBlock(int scriptId, int *size=NULL);
  static VCPUcodeBlock *getCodeBlockEntry(int vcpuid);

  static void addStatementString(wchar_t *s);

  static int getCacheCount();
  static int getUserAncestor(int varid, int scriptid);
  static int isValidScriptId(int id);
  static int oldClassToClassId(int id);
  static const wchar_t *getClassName(int vcpuid, int localclassid);

  static int VIP;
  static int VSP;
  static int VSD;
  static int VCC;
  static int complete;
  static int cacheCount;

  static Stack<int> VIPstack;
  static Stack<int> VSPstack;
  static Stack<int> VSDstack;
  static Stack<int> VCCstack;

	static void RemoveOldScripts();
	static TList<int> scriptsToRemove;

  static scriptVar paramList[SCRIPT_MAXARGS];
  static TList<VCPUscriptVar> plist;
  static PtrList<wchar_t> statementStringList;

  static int isInstantiable(int id);

  static void pushObject(void *o);
  static void pushInt(int i);
  static void pushBoolean(int b);
  static void pushFloat(float f);
  static void pushDouble(double d);
  static void pushString(const wchar_t *s);
  static void pushVoid();
  static void *popObject();
  static int popInt();
  static bool popBoolean();
  static float popFloat();
  static double popDouble();
  static const wchar_t *popString();
  static void popDiscard();
  static void callDlfCommand(void *ptr, int nargs, maki_cmd *cmd);
  static int getDLFFromPointer(void *ptr, int nargs);
  static void DLF_addref(void *ptr, int nargs);
  static void DLF_reset(void *ptr, int nargs);
  static void DLF_remref(void *ptr, int nargs);
  static VCPUdlfEntry *getGlobalDlfEntry(int dlfid);
  static void registerGlobalDlf(VCPUdlfEntry *e, int dlf);
  static void traceState(VCPUscriptVar object, VCPUdlfEntry *e);
  static PtrList<VCPUdlfEntry> globalDlfList;
  static PtrListInsertSorted< OrphanEntry, OrphanQuickSort >orphans;
  static int orphanid;
  static VCPUscriptVar *getOrphan(int id);
  static int createOrphan(int type);
  static void killOrphan(int id);
  static void setAtom(const wchar_t *atomname, ScriptObject *o);
  static ScriptObject *getAtom(const wchar_t *atomname);
  static PtrListQuickSorted<ScriptAtom, ScriptAtomSort> atoms;
  static ScriptObjectManager *scriptManager;
};

#endif
