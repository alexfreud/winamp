#ifndef _SCRIPTBROWSER_H
#define _SCRIPTBROWSER_H

#include <api/skin/widgets/mb/minibrowserwnd.h>
#include <api/script/script.h>
#include <api/script/objects/guiobj.h>

class BrowserScriptController : public GuiObjectScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return guiController; }
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

extern BrowserScriptController *browserController;


#define SCRIPTBROWSERWND_PARENT MiniBrowserWnd
class ScriptBrowserWnd : public SCRIPTBROWSERWND_PARENT {
public:

  ScriptBrowserWnd();
  virtual ~ScriptBrowserWnd();

  void setMainMB(int m);
  virtual int setXmlParam(const wchar_t *name, const wchar_t *value);
  virtual int onInit();

  virtual int handleDesktopAlpha() { return 0; } // showing the browser will turn off desktop alpha on the parent layout

  virtual void onDocumentComplete(const char *url);
  virtual int onBeforeNavigate(const char *url, int flags, const char *frame);

public:
  static scriptVar script_vcpu_gotoUrl(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url);
  static scriptVar script_vcpu_back(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_forward(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_home(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_refresh(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_setTargetName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar name);
  static scriptVar script_vcpu_onDocumentComplete(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url);
  static scriptVar script_vcpu_onBeforeNavigate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar url, scriptVar flags, scriptVar framename);

private:

  String defurl;
};

#endif
