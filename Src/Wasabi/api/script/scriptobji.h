#ifndef __SCRIPTOBJI_H
#define __SCRIPTOBJI_H

#include <api/script/vcputypes.h>
#include <bfc/dispatch.h>

#include <bfc/tlist.h>
#include <bfc/ptrlist.h>
#include <bfc/pair.h>

//<?<autoheader/>
#include "scriptobj.h"
#include "scriptobjx.h"

class ScriptHook;
class ScriptObject;
class ScriptObjectController;

//?>


class ScriptObjectController;
class ScriptHook;

/*[interface.header.h]
enum {
  INTERFACE_GENERICVOIDPTR=0,
  INTERFACE_SCRIPTOBJECT,
};
*/

// ----------------------------------------------------------------------------------------------------------

/*class MemberVarCompare {
  public:
    static int compareItem(void *p1, void *p2);
    static int compareAttrib(const wchar_t *attrib, void *p);
};*/

// ----------------------------------------------------------------------------------------------------------




// ----------------------------------------------------------------------------------------------------------

class ScriptObjectI : public ScriptObjectX 
{
private:
	/* These three classes are here to ensure they don't get used anywhere else */
	struct assvar 
	{
	  int scriptid;
	  int varid;
	  TList<int> dlfs;
	};

	class InterfaceEntry 
	{
  public:
    InterfaceEntry(GUID _guid, void *_ptr, int _type = INTERFACE_GENERICVOIDPTR) : guid(_guid), ptr(_ptr), type(_type) {}
    virtual ~InterfaceEntry() {}

    virtual GUID getGuid() { return guid; }
    virtual void *getInterface() { return ptr; }
    virtual int getType() { return type; }

  private:

    GUID guid;
    void *ptr;
    int type;
};

	class MemberVar 
	{
  public:
    MemberVar(const wchar_t *name, int scriptid, int rettype);
    virtual ~MemberVar();

    const wchar_t *getName() { return name; }
    int getScriptId() { return scriptid; }
    int getReturnType() { return rettype; }
    int getGlobalId() { return globalid; }

  private:  
    StringW name;
    int scriptid;
    int rettype;
    int globalid;
};


public:
  ScriptObjectI(const wchar_t *class_name = NULL, ScriptObjectController *object_controller = NULL);
  virtual ~ScriptObjectI();

  DISPATCH(50) virtual void *vcpu_getInterface(GUID g, int *interfacetype = NULL);
  DISPATCH(60) virtual void *vcpu_getInterfaceObject(GUID g, ScriptObject **o);
  DISPATCH(100) int vcpu_getAssignedVariable(int start, int scriptid, int functionId, int *next, int *globalevententry, int *inheritedevent);
  DISPATCH(200) void vcpu_removeAssignedVariable(int var, int id);
  DISPATCH(300) void vcpu_addAssignedVariable(int var, int scriptid);
  DISPATCH(400) virtual const wchar_t *vcpu_getClassName();
  DISPATCH(500) virtual ScriptObjectController *vcpu_getController();
//  DISPATCH(600) virtual void vcpu_addClassHook(ScriptHook *h);
//  DISPATCH(700) virtual void vcpu_addObjectHook(ScriptHook *h);
  DISPATCH(800) int vcpu_getScriptId();
  DISPATCH(900) void vcpu_setScriptId(int i);
  DISPATCH(1000) int vcpu_getMember(const wchar_t *id, int scriptid, int rettype);
  DISPATCH(1100) void vcpu_delMembers(int scriptid);
  DISPATCH(1200) virtual void vcpu_setInterface(GUID g, void *v, int interfacetype = INTERFACE_SCRIPTOBJECT);
  DISPATCH(1300) virtual void vcpu_setClassName(const wchar_t *name);
  DISPATCH(1400) virtual void vcpu_setController(ScriptObjectController *c);
  DISPATCH(1500) virtual void vcpu_init();

protected:
  NODISPATCH int getEventForVar(assvar *var, int funcid, int *inheritedevent);
  NODISPATCH void computeEventList(assvar *a);
  PtrList < assvar > assignedVariables;
  PtrList < MemberVar > memberVariables;
  int cache_count;
  int id;
  StringW membercacheid;
  int membercachesid;
  int membercachegid;
  PtrList < InterfaceEntry > interfaceslist;
  const wchar_t *classname;
  ScriptObjectController * controller;
  int ingetinterface;


};

#endif
