#ifndef _BUTTON_H
#define _BUTTON_H

#include <bfc/string/bfcstring.h>
#include <api/script/script.h>
#include <api/wnd/wndclass/buttwnd.h>
#include <api/script/objects/guiobj.h>
#include <api/syscb/callbacks/corecbi.h>
#include <api/wndmgr/layout.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/service/svcs/svc_xuiobject.h>

#define BUTTON_PARENT ButtonWnd

class ComponentBucket2;

class ScriptObject;
                       
class ButtonScriptController: public GuiObjectScriptController {
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

extern ButtonScriptController *buttonController;

namespace Wasabi // Apple defines "Button" so we're pretty much forced to namespace this... annoying
{
#ifdef WASABI_COMPILE_MEDIACORE
class Button : public BUTTON_PARENT, public CoreCallbackI {
#else
class Button : public BUTTON_PARENT {
#endif

public:
	Button();
	virtual ~Button();

  virtual int onInit();
  virtual int setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *paramname, const wchar_t *strvalue);
	virtual void setParam(const wchar_t *p);
  virtual int getPreferences(int what);
	virtual const wchar_t *getParam();
  virtual int onLeftButtonUp(int x, int y);
  virtual int onLeftButtonDown(int x, int y);
  virtual void onLeftPush(int x, int y);
  virtual void onRightPush(int x, int y);
  virtual int onActivateButton(int is);

  virtual int childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2);

  virtual int wantAutoContextMenu();
#ifdef WASABI_WIDGETS_COMPBUCK
  virtual void setCBTarget(const wchar_t *t);
#endif
  virtual void setAction(const wchar_t *action);

  virtual int onShowWindow(Container *c, GUID guid, const wchar_t *groupid);
  virtual int onHideWindow(Container *c, GUID guid, const wchar_t *groupid);

protected:
	/*static */void CreateXMLParameters(int master_handle);
#ifdef WASABI_COMPILE_MEDIACORE
  virtual int corecb_onEQStatusChange(int newval);
  virtual int corecb_onEQAutoChange(int newval);
#endif
  StringW s_normal, s_down, s_hover, s_active;
  int btn_getXuiHandle() { return xuihandle; }
  int retcode;
  virtual void setupBitmaps();

  enum {
    BUTTON_TEXT = 0,
    BUTTON_ACTION,
    BUTTON_IMAGE,
    BUTTON_DOWNIMAGE,
    BUTTON_HOVERIMAGE,
    BUTTON_ACTIVEIMAGE,
    BUTTON_PARAM,
    BUTTON_RECTRGN,
    BUTTON_CBTARGET,
    BUTTON_BORDERS,
    BUTTON_BORDERSTYLE,
    BUTTON_RETCODE,
    BUTTON_ACTIONTARGET,
    BUTTON_CENTER_IMAGE,
    BUTTON_TEXTCOLOR,
    BUTTON_TEXTHOVERCOLOR,
    BUTTON_TEXTDIMMEDCOLOR,
		BUTTON_NUMPARAMS,
  };

private:
	StringW param;
  StringW action, actionstr, actionname;
  StringW action_target;
  ComponentBucket2 *cbtarget;
  int borders;
  int disablenextcontextmenu;
	int xuihandle;
	static XMLParamPair params[];

public:
  static scriptVar script_vcpu_setActivatedNoCallback(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
  static scriptVar script_vcpu_onLeftClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onRightClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_leftClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_rightClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_setActivated(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
  static scriptVar script_vcpu_getActivated(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onActivate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
};
}

extern const wchar_t buttonXuiObjectStr[];
extern char buttonXuiSvcName[];
class ButtonXuiSvc : public XuiObjectSvc<Wasabi::Button, buttonXuiObjectStr, buttonXuiSvcName> {};


#endif
