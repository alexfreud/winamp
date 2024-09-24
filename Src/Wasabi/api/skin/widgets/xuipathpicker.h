#ifndef __XUIPATHPICKER_H
#define __XUIPATHPICKER_H

#include <api/skin/widgets/pathpicker.h>
#include <api/script/objcontroller.h>

#define SCRIPTPATHPICKER_PARENT PathPicker

// -----------------------------------------------------------------------
// Your wnd object class

class ScriptPathPicker: public SCRIPTPATHPICKER_PARENT {
  
  public:

    ScriptPathPicker();
    virtual ~ScriptPathPicker();

  private:
};

// -----------------------------------------------------------------------
class PathPickerScriptController: public ScriptObjectControllerI {
public:
  virtual const wchar_t *getClassName() { return L"PathPicker"; }
  virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
  virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
  virtual int getNumFunctions();
  virtual const function_descriptor_struct *getExportedFunctions();
  virtual GUID getClassGuid() { return pathPickerGuid; }
  virtual ScriptObject *instantiate();
  virtual void destroy(ScriptObject *o);
  virtual void *encapsulate(ScriptObject *o);
  virtual void deencapsulate(void *o);

private:

  static function_descriptor_struct exportedFunction[];
  static scriptVar PathPicker_getPath(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar PathPicker_onPathChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newpath);
};

extern PathPickerScriptController *pathPickerController;


// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

extern const wchar_t PathPickerXuiObjectStr[];
extern char PathPickerXuiSvcName[];
class PathPickerXuiSvc : public XuiObjectSvc<ScriptPathPicker, PathPickerXuiObjectStr, PathPickerXuiSvcName> {};

#endif
