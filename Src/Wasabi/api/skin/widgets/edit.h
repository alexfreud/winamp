#ifndef __EDIT_H
#define __EDIT_H

#include <api/script/script.h>
#include <api/script/objects/guiobj.h>

#define EDIT_PARENT EditWnd

class EditScriptController : public GuiObjectScriptController {
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

extern EditScriptController *editController;

#ifndef _NOSTUDIO

#include <api/wnd/wndclass/editwnd.h>

class Edit : public EDIT_PARENT {
public:
  Edit();
  ~Edit();

  virtual void onEditUpdate();
  virtual void onIdleEditUpdate();
  virtual int onEnter();	// user hit enter.. return 1 to close window
  virtual int onAbort();	// user hit escape.. return 1 to close window
  virtual int onInit();
#ifdef WASABI_COMPILE_CONFIG
  virtual int onReloadConfig();
#endif

  virtual int setXuiParam(int _xuihandle, int xmlattrid, const wchar_t *name, const wchar_t *value);

  void setText(const wchar_t *t);
  void setAutoUrl(int a);

protected:
		/*static */void CreateXMLParameters(int master_handle);
private:
  wchar_t *my_buffer;
  int autourl;
  int xuihandle;

#else
class Edit : public EDIT_SCRIPTPARENT {
#endif

public:

  static scriptVar script_vcpu_setText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t);
  static scriptVar script_vcpu_setAutoEnter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t);
  static scriptVar script_vcpu_getAutoEnter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_setIdleEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t);
  static scriptVar script_vcpu_getIdleEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onEnter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onAbort(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onEditUpdate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onIdleEditUpdate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_selectAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_enter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

protected:
	enum {
  EDIT_TEXT=0,
  EDIT_ACTION,
  EDIT_MULTILINE,
  EDIT_VSCROLL,
  EDIT_AUTOHSCROLL,
  EDIT_AUTOENTER,
  EDIT_PASSWORD,
  EDIT_AUTOSELECT,
};
private:
	static XMLParamPair params[];
};

extern const wchar_t editXuiObjectStr[];
extern char editXuiSvcName[];
class EditXuiSvc : public XuiObjectSvc<Edit, editXuiObjectStr, editXuiSvcName> {};


#endif
