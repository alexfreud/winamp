#ifndef __COREADMINOBJ_H
#define __COREADMINOBJ_H

#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/core/sequence.h>
#include <api/syscb/callbacks/corecbi.h>

#ifdef GEN_FF
#ifndef FAKE_SCRIPTCORE
#define FAKE_SCRIPTCORE
#endif
#endif

class svc_coreAdmin;

// {F857BECA-8E19-41f1-973E-097E39649F03}
static const GUID COREADMIN_SCRIPTOBJECT_GUID =
    { 0xf857beca, 0x8e19, 0x41f1, { 0x97, 0x3e, 0x9, 0x7e, 0x39, 0x64, 0x9f, 0x3 } };

extern ScriptObjectController *coreAdminController;

// -----------------------------------------------------------------------------------------------------
class ScriptCoreAdminObject : public RootObjectInstance
{
public:
	ScriptCoreAdminObject();
	virtual ~ScriptCoreAdminObject();

	CoreToken getNamedCore(const wchar_t *name);
	CoreToken newNamedCore(const wchar_t *name);
	int freeCore(CoreToken core);
	int freeCoreByName(const wchar_t *name);

private:
#ifndef FAKE_SCRIPTCORE
	svc_coreAdmin *admin;
#endif
};

// -----------------------------------------------------------------------------------------------------
class CoreAdminScriptObjectController : public ScriptObjectControllerI
{
public:
	virtual const wchar_t *getClassName() { return L"CoreAdmin"; }
	virtual const wchar_t *getAncestorClassName() { return L"Object"; }
	virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(rootObjectGuid); }
	virtual int getNumFunctions();
	virtual const function_descriptor_struct *getExportedFunctions() { return exportedFunction; }
	virtual GUID getClassGuid() { return COREADMIN_SCRIPTOBJECT_GUID; }
	virtual int getInstantiable() { return 0; }
	virtual int getReferenceable() { return 0; }
	virtual ScriptObject *instantiate();
	virtual void *encapsulate(ScriptObject *o);
	virtual void destroy(ScriptObject *o);
	virtual void deencapsulate(void *o);

	static scriptVar coreadmin_getNamedCore(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name);
	static scriptVar coreadmin_newNamedCore(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name);
	static scriptVar coreadmin_freeCore(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar core);
	static scriptVar coreadmin_freeCoreByName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name);

private:
	static function_descriptor_struct exportedFunction[];
};

#endif
