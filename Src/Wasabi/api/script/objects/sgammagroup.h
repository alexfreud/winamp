#ifndef __SGammagroup_H
#define __SGammagroup_H

class SGamma;

#include <api/script/script.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>
#include <api/skin/skinelem.h>
#include <api/skin/gammamgr.h>
#define SGAMMAGROUP_SCRIPTPARENT RootObjectInstance

class GammagroupScriptController : public ScriptObjectControllerI {
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

extern GammagroupScriptController *gammagroupController;

class SGammagroup : public SGAMMAGROUP_SCRIPTPARENT {
public:
	SGammagroup();
	SGammagroup(const wchar_t * parentSet, const wchar_t * grpName);
	SGammagroup(const wchar_t * parentSet, int i);
	virtual ~SGammagroup();

//protected:
	ColorThemeGroup * grp;
	const wchar_t * parentSet;

private:
	void __construct();

public:
	static scriptVar script_vcpu_setRed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color);
	static scriptVar script_vcpu_setGreen(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color);
	static scriptVar script_vcpu_setBlue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color);
	static scriptVar script_vcpu_setBoost(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color);
	static scriptVar script_vcpu_setGray(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color);
	static scriptVar script_vcpu_setID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar color);

	static scriptVar script_vcpu_getRed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getGreen(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getBlue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getBoost(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getGray(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};

#endif
