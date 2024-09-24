#ifndef __SFile_H
#define __SFile_H

class SFile;

#include <api/script/script.h>
#include <api/script/objcontroller.h>
#include <api/script/objects/rootobj.h>
#include <api/script/objects/rootobject.h>

#define SFile_SCRIPTPARENT RootObjectInstance

class fileScriptController : public ScriptObjectControllerI {
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

extern fileScriptController *fileController;

class SFile : public SFile_SCRIPTPARENT {
public:
  SFile();
  virtual ~SFile();


  void loadfile(const wchar_t *b);


protected:
	const wchar_t *filename;

public:
  static scriptVar script_vcpu_load(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar fn);
  static scriptVar script_vcpu_getSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_exists(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};

#endif
