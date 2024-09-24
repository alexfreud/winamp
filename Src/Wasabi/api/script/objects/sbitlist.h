#ifndef __SBITLIST_H
#define __SBITLIST_H

#include <api/script/objects/rootobj.h>

#include <api/script/script.h>
#include <api/script/objects/rootobject.h>

class BitList;

#define SBITLIST_SCRIPTPARENT RootObjectInstance

class BitlistScriptController : public ScriptObjectControllerI {
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

extern BitlistScriptController *bitlistController;


class SBitlist : public SBITLIST_SCRIPTPARENT {

public:
  SBitlist();
  virtual ~SBitlist();

  BitList *getBitList();

private:

  BitList *list;

public:
  static scriptVar script_vcpu_getitem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n); 
  static scriptVar script_vcpu_setitem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n, scriptVar v);
  static scriptVar script_vcpu_setsize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newsize);
  static scriptVar script_vcpu_getsize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

};

#endif
