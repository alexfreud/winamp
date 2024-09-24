#ifndef __SLIST_H
#define __SLIST_H

#include <api/script/objects/rootobject.h>
#include <api/script/objects/rootobj.h>
#include <bfc/ptrlist.h>
#include <bfc/tlist.h>
class ScriptObject;

// {B2023AB5-434D-4ba1-BEAE-59637503F3C6}
static const GUID slistGuid = 
{ 0xb2023ab5, 0x434d, 0x4ba1, { 0xbe, 0xae, 0x59, 0x63, 0x75, 0x3, 0xf3, 0xc6 } };

#define SLIST_SCRIPTPARENT RootObjectInstance

class ListScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return rootScriptObjectController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern ListScriptController *listController;


class SList : public SLIST_SCRIPTPARENT {

public:
  SList();
  virtual ~SList();

  TList<scriptVar> *getTList();
  
private:
	TList<scriptVar> list;

// -- SCRIPT -----------------------------------------------------
public:
//  INSERT_SCRIPT_OBJECT_CONTROL
  static scriptVar script_vcpu_enumItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i);
  static scriptVar script_vcpu_addItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj);
  static scriptVar script_vcpu_removeItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj);
  static scriptVar script_vcpu_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_findItem2(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj, scriptVar start);
  static scriptVar script_vcpu_findItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj);
  static scriptVar script_vcpu_removeAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};

#endif
