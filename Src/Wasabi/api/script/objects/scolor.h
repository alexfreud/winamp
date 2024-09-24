#ifndef __SColor_H
#define __SColor_H

class SColor;

#include <api/script/script.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>
#include <api/skin/skinelem.h>
#include <api/skin/gammamgr.h>
#define SCOLOR_SCRIPTPARENT RootObjectInstance

class colorScriptController : public ScriptObjectControllerI {
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

extern colorScriptController *colorController;

class SColor : public SCOLOR_SCRIPTPARENT {
public:
	SColor();
	SColor(const wchar_t*);
	SColor(int i);
	virtual ~SColor();
	void enumColor(int n);
	int getARGBValue(int whichCol);
	int getARGBValueWithGamma(int whichCol);

protected:
	const wchar_t *gammagroup;
	const wchar_t *colorID;
	ARGB32 color;

private:
	void __construct();

public:
	static scriptVar script_vcpu_getRed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getGreen(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getBlue(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getRedWithGamma(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getGreenWithGamma(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getBlueWithGamma(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getGammagroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar script_vcpu_getColorID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};

#endif
