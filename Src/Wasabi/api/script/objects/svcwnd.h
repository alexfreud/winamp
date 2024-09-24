#ifndef _SVCWND_H
#define _SVCWND_H

#include "script.h"
#include "../../common/script/scriptobj.h"
#include "guiobj.h"
#include "../../bfc/svc_enum.h"

class SMap;
class SRegion;
class Container;
class Layout;

// {8776F715-503A-41f9-BD63-FB148AD05765}
static const GUID svcWndGuid = 
{ 0x8776f715, 0x503a, 0x41f9, { 0xbd, 0x63, 0xfb, 0x14, 0x8a, 0xd0, 0x57, 0x65 } };

#define SVCWND_PARENT GuiObjectWnd

class SvcWndScriptController : public GuiObjectScriptController {
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

extern SvcWndScriptController *svcWndController;

class SvcWnd : public SVCWND_PARENT {
public:
  SvcWnd();
  virtual ~SvcWnd();

  virtual int onLeftButtonDblClk(int x, int y);
  virtual int onResize();
  virtual void onSetVisible(int v);
  virtual int onInit();

  virtual int getPreferences(int what);
  virtual int handleRatio();

	void setGUID(GUID g);
	GUID getGUID(void);

  virtual int childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2);
  virtual int setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
  virtual int onUnknownXuiParam(const wchar_t *param, const wchar_t *value);

  // VCPU
  static scriptVar script_vcpu_getGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  PtrList<StringW> params;
  // End VCPU

protected:

  enum {
    SVCWND_GUID=0,
    SVCWND_DBLCLKACTION,
  };

private:
  GUID myGUID;
  ifc_window *svcwnd;
  svc_windowCreate *svc;
  int forwarded;
  StringW dblClickAction;
  int xuihandle;
};

extern char svcWndXuiObjectStr[];
extern char svcWndXuiSvcName[];
class SvcWndXuiSvc : public XuiObjectSvc<SvcWnd, svcWndXuiObjectStr, svcWndXuiSvcName> {};


#endif
