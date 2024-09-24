#ifndef __XUISTATUS_H
#define __XUISTATUS_H

#include <api/wnd/wndclass/status.h>
#include <api/script/objcontroller.h>

#define XUISTATUS_PARENT StatusBar
class XuiStatus : public XUISTATUS_PARENT {
public:
  XuiStatus();
  virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

  virtual void callme(const wchar_t *txt);
protected:
	/*static */void CreateXMLParameters(int master_handle);
private:
	
enum {
  EXCLUDE, INCLUDE
};
	static XMLParamPair params[];
  int myxuihandle;
};

extern const wchar_t StatusXuiObjectStr[];
extern char StatusXuiSvcName[];
class LayoutStatusXuiSvc : public XuiObjectSvc<XuiStatus, StatusXuiObjectStr, StatusXuiSvcName> {};

// -----------------------------------------------------------------------------------------------------
class LayoutStatusController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"LayoutStatus"; }
    virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return layoutStatusGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  public:
    static scriptVar layoutstatus_callme(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str);

  private:

    static function_descriptor_struct exportedFunction[];
};

extern LayoutStatusController *layoutStatusController;

#endif
