#ifndef _WACOBJ_H
#define _WACOBJ_H

#include <api/script/script.h>
#include <api/script/scriptobj.h>
#include <api/script/objects/guiobj.h>
#include <bfc/ptrlist.h>
#include <api/wac/wac.h>

// {00C074A0-FEA2-49a0-BE8D-FABBDB161640}
static const GUID wacGuid = 
{ 0xc074a0, 0xfea2, 0x49a0, { 0xbe, 0x8d, 0xfa, 0xbb, 0xdb, 0x16, 0x16, 0x40 } };

#define WACOBJECT_SCRIPTPARENT RootObjectInstance

class WacScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController();
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual int getInstantiable();
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern WacScriptController *wacController;

#include <api/wnd/virtualwnd.h>

class WACObject : public WACOBJECT_SCRIPTPARENT {

public:
	WACObject();
	virtual ~WACObject();

	void setGUID(GUID g);
	GUID getGUID(void);

  int onScriptNotify(const wchar_t *s, int i1, int i2);

  // VCPU
  static scriptVar script_getGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_getName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o); 
  static scriptVar script_vcpu_onNotify(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str, scriptVar i1, scriptVar i2);

  static scriptVar script_vcpu_dummy4(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a, scriptVar b, scriptVar c, scriptVar d);
  static scriptVar script_vcpu_dummy1(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
  static scriptVar script_vcpu_dummy0(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  // End VCPU

  static int notifyScripts(WaComponent *comp, const wchar_t *s, int i1, int i2);

  GUID myGUID;

private:

  static PtrList<WACObject> wacobjs;

public:
};


#endif
