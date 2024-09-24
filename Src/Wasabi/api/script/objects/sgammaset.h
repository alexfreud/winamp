#ifndef __SGammaset_H
#define __SGammaset_H

class SGamma;

#include <api/script/script.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>
#include <api/skin/skinelem.h>
#include <api/skin/gammamgr.h>
#include "sgammagroup.h"
#define SGAMMASET_SCRIPTPARENT RootObjectInstance

class GammasetScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController();
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

extern GammasetScriptController *gammasetController;

class SGammaset : public SGAMMASET_SCRIPTPARENT {
public:
	SGammaset();
	SGammaset(const wchar_t *);
	SGammaset(int i);
	virtual ~SGammaset();

protected:
	const wchar_t *gammasetID;

private:
	void __construct();

public:
	static scriptVar script_vcpu_apply(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_rename(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name);
	static scriptVar script_vcpu_update(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_delete(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getDefaultGammaGroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getGammaGroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name);
	static scriptVar script_vcpu_getNumGammaGroups(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_enumGammaGroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
};

#endif
