#ifndef __SCRIPT_H
#define __SCRIPT_H

#include <bfc/ptrlist.h>
#include <api/script/vcputypes.h>

class String;

#define GURU_POPEMPTYSTACK 0
#define GURU_INVALIDHEADER 1
#define GURU_INVALIDFUNCINDLF 2
#define GURU_INVALIDFUNCBT 3
#define GURU_INVALIDVARBT 4
#define GURU_INVALIDEVENTDLF 5
#define GURU_INVALIDEVENTADDR 6
#define GURU_INVALIDEVENTVAR 7
#define GURU_INVALIDSCRIPTID 8
#define GURU_DLFSETUPFAILED 9
#define GURU_SETNONINTERNAL 10
#define GURU_INCSNONNUM 11
#define GURU_DECSNONNUM 12
#define GURU_INCPNONNUM 13
#define GURU_DECPNONNUM 14
#define GURU_OBJECTADD 15
#define GURU_SUBNONNUM 16
#define GURU_MULNONNUM 17
#define GURU_DIVNONNUM 18
#define GURU_DIVBYZERO 19
#define GURU_MODNONNUM 20
#define GURU_NEGNONNUM 21
#define GURU_BNOTNONNUM 22
#define GURU_SHLNONNUM 23
#define GURU_SHRNONNUM 24
#define GURU_XORNONNUM 25
#define GURU_ANDNONNUM 26
#define GURU_NEWFAILED 27
#define GURU_NULLCALLED 28
#define GURU_OLDFORMAT 29
#define GURU_INVALIDPEEKSTACK 30
#define GURU_INVALIDOLDID 31
#define GURU_INCOMPATIBLEOBJECT 32
#define GURU_EXCEPTION 33
#define GURU_FUTUREFORMAT 34

// This is our virtual machine basic type. Reducing of expression will use this
// type. We will not malloc or new it, but we'll pass it on the stack for speed
// and stability. 8 bytes should be ok for our stack. This way we won't have to
// maintain lists on dynamically allocated computation values, and we won't have
// to cleanup if some error (divide by zero ?) occurs in the middle of a
// statement. String values are an exception since sdata will point to dynamic
// data, but since every step of the virtual machine reducing strings will free
// that data, cleanup will be automatic

typedef struct {
	void *scriptBlock;
	int vcpuId; // id of script in virtual CPU. will be used later for unloading
} scriptEntry;

class Group;
class SystemObject;

class Script
{
public:
	static int addScript(const wchar_t *path, const wchar_t *filename, const wchar_t *id);
	static void unloadScript(int id);
	static void unloadAllScripts();
	static void guruMeditation(SystemObject *script, int code, const wchar_t *pub=NULL, int intinfo=0);
	static void setScriptParam(int id, const wchar_t *p);
	static int varIdToGlobal(int id, int script);
	static int getNumEventsLinked();
	static int getLinkedEventParams(int num, int *dlfid, int *scriptid, int *varid);
	static int getCacheCount();
	static int getUserAncestor(int varid, int scriptid);
	static int isValidScriptId(int id);
	static void setParentGroup(int id, Group *g);
	static void setSkinPartId(int id, int skinpartid);
	static int getNumScripts() { return scriptslist.getNumItems(); }

	static PtrList <scriptEntry> scriptslist;

private:
	static int codeToSeverity(int code, wchar_t *t, int len);
};

#endif
