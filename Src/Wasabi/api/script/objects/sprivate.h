#pragma once
#include <api/script/api_maki.h>
#include <api/script/script.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>

class PDownloadCallback;

#define SPRIVATE_SCRIPTPARENT RootObjectInstance

class SPrivateScriptObjectController : public ScriptObjectControllerI 
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

extern ScriptObjectController *SPrivateController;

class SPrivate : public SPRIVATE_SCRIPTPARENT 
{
public:
  SPrivate();
  virtual ~SPrivate();

  PDownloadCallback * dlcb;

public:
	static scriptVar vcpu_updateLinks(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar version, scriptVar bversion);
	static scriptVar vcpu_onLinksUpdated(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};
