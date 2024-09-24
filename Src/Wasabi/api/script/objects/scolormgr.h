#ifndef __SColorMgr_H
#define __SColorMgr_H

class SColorMgr;

#include <api/script/script.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>
#include <api/skin/skinelem.h>
#include <api/skin/gammamgr.h>
#include "scolor.h"
#include "sgammaset.h"
#define SCOLORMGR_SCRIPTPARENT RootObjectInstance

class ColorMgrScriptController : public ScriptObjectControllerI {
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

extern ColorMgrScriptController *colorMgrController;

class SColorMgr : public SCOLORMGR_SCRIPTPARENT,  public SkinCallbackI {
public:
	SColorMgr();
	virtual ~SColorMgr();

	virtual int skincb_onBeforeLoadingElements();
	virtual int skincb_onGuiLoaded();
	virtual int skincb_onLoaded();
	virtual int skincb_onColorThemeChanged(const wchar_t *newcolortheme);
	virtual int skincb_onColorThemesListChanged();

public:
	static scriptVar script_vcpu_getColor(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar colorID);
	static scriptVar script_vcpu_enumColor(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
	static scriptVar script_vcpu_getNumColors(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);	
	static scriptVar script_vcpu_getCurrentGammaSet(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);	
	static scriptVar script_vcpu_getGammaSet(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar colorID);
	static scriptVar script_vcpu_enumGammaSet(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
	static scriptVar script_vcpu_getNumGammaSets(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);	
	static scriptVar script_vcpu_newGammaSet(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar colorID);	

	static scriptVar script_vcpu_onBeforeLoadingElements(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_onGuiLoaded(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_onColorThemeChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newcolortheme);
	static scriptVar script_vcpu_onColorThemesListChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_onLoaded(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};

#endif
