#ifndef __ROOTSCRIPTOBJECTCONTROLLER_IMPL_H
#define __ROOTSCRIPTOBJECTCONTROLLER_IMPL_H

#include <api/script/objcontroller.h>
/*<?<autoheader/>*/
class ScriptObject;
class ScriptObjectController;
/*?>*/

class RootScriptObjectController : public ScriptObjectControllerI 
{
  public:
    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return NULL; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void deencapsulate(void *o);
    virtual void *encapsulate(ScriptObject *o);
  private:
    static function_descriptor_struct exportedFunction[];
};

class RootObject_ScriptMethods {
  public:
    static scriptVar getClassName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o); 
    static scriptVar onNotify(SCRIPT_FUNCTION_PARAMS, ScriptObject *on, scriptVar s, scriptVar s2, scriptVar i1, scriptVar i2); 
    static scriptVar notify(SCRIPT_FUNCTION_PARAMS, ScriptObject *on, scriptVar s, scriptVar s2, scriptVar i1, scriptVar i2); 
};

extern RootScriptObjectController *rootScriptObjectController;

#endif // __ROOTSCRIPTOBJECTCONTROLLER_IMPL_H
