#ifndef NULLSOFT_GEN_FF_WINAMPCONFIGSCRIPTOBJECT_H
#define NULLSOFT_GEN_FF_WINAMPCONFIGSCRIPTOBJECT_H

#include <api/script/objects/rootobj.h>
#include <api/script/script.h>
#include <api/script/objects/rootobject.h>
#include <api/service/svcs/svc_scriptobji.h>
namespace Agave
{
	#include "../Agave/Config/api_config.h"
}

// -----------------------------------------------------------------------------------------------------
// ScriptObject Service
class WinampConfigScriptObjectSvc : public svc_scriptObjectI 
{
public:
  WinampConfigScriptObjectSvc() {};
  virtual ~WinampConfigScriptObjectSvc() {};

  static const char *getServiceName() { return "Winamp Config script object"; }
  virtual ScriptObjectController *getController(int n);
};       

class WinampConfigScriptController : public ScriptObjectControllerI 
{
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

extern ScriptObjectController *winampConfigController;
extern ScriptObjectController *winampConfigGroupController;

#define WINAMPCONFIG_PARENT RootObjectInstance

class WinampConfig : public WINAMPCONFIG_PARENT 
{
public:
  WinampConfig();
	~WinampConfig();
public:
	virtual const wchar_t *vcpu_getClassName() { return L"WinampConfig"; }
	virtual ScriptObjectController *vcpu_getController() { return winampConfigController; }

	static scriptVar script_vcpu_getGroup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n); 

	Agave::api_config *config;
};

class WinampConfigGroupScriptController : public ScriptObjectControllerI {
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

#define WINAMPCONFIGGROUP_PARENT RootObjectInstance
class WinampConfigGroup : public WINAMPCONFIGGROUP_PARENT 
{
public:
	WinampConfigGroup();
	WinampConfigGroup(Agave::ifc_configgroup *_configGroup);

public:
	virtual const wchar_t *vcpu_getClassName() { return L"WinampConfigGroup"; }
	virtual ScriptObjectController *vcpu_getController() { return winampConfigGroupController; }

	static Agave::ifc_configitem *GetItem(ScriptObject *o, scriptVar n); // helper function
	static scriptVar script_vcpu_getBool(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
	static scriptVar script_vcpu_getString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
	static scriptVar script_vcpu_getInt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
	static scriptVar script_vcpu_setBool(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar itemname, scriptVar value);

	Agave::ifc_configgroup *configGroup;
};

#endif
