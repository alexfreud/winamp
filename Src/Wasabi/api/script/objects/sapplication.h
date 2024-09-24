#pragma once
#include <api/script/api_maki.h>
#include <api/script/script.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>

#define SApplication_SCRIPTPARENT RootObjectInstance

class ApplicationScriptObjectController : public ScriptObjectControllerI 
{
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController();
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
		virtual int getInstantiable();
		virtual int getReferenceable() {return 0;}
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:
    static function_descriptor_struct exportedFunction[];
    
};

extern ScriptObjectController *applicationController;

class SApplication : public SApplication_SCRIPTPARENT 
{
public:
  SApplication();
  virtual ~SApplication();

public:
	static scriptVar GetApplicationName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetVersionString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetVersionNumberString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetBuildNumber(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetCommandLine(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar Shutdown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar CancelShutdown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar IsShuttingDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetApplicationPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetSettingsPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetWorkingPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar SetWorkingPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar string_path);
	static scriptVar GetMachineGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetUserGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static scriptVar GetSessionGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};
