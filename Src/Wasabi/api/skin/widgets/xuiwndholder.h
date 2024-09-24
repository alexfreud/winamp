#ifndef __XUIWNDHOLDER_H
#define __XUIWNDHOLDER_H

#include <api/wnd/wndclass/wndholder.h>
#include <api/script/scriptguid.h>
#include <api/script/objcontroller.h>

#define XUIWNDHOLDER_PARENT WindowHolderWnd 

// -----------------------------------------------------------------------
class XuiWindowHolder : public XUIWNDHOLDER_PARENT 
{
  
  public:

    XuiWindowHolder();
    virtual ~XuiWindowHolder();

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    void setRegionFromMap(ScriptObject *map, int byte, int inverse);
    void setRegion(ScriptObject *region);
    static GUID *parseGUID(const wchar_t *id);

protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    enum {
      XUIWNDHOLDER_ADDHOLD = 10,
      XUIWNDHOLDER_SETNOSHOWCMDBAR = 20,
      XUIWNDHOLDER_SETNOANIMRECTS = 30,
      XUIWNDHOLDER_SETAUTOOPEN = 40,
      XUIWNDHOLDER_SETAUTOCLOSE = 50,
      XUIWNDHOLDER_SETAUTOFOCUS = 60,
      XUIWNDHOLDER_SETAUTOAVAILABLE = 70,
    };
		static XMLParamPair params[];
    int myxuihandle;
};

// -----------------------------------------------------------------------------------------------------
class WindowHolderScriptController: public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"WindowHolder"; }
    virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return windowHolderGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  public:
    static scriptVar script_getGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar script_setRegionFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv);
    static scriptVar script_setRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar reg);
    static scriptVar script_getContent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar script_getComponentName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  private:

    static function_descriptor_struct exportedFunction[];
};

extern WindowHolderScriptController *windowHolderController;


// -----------------------------------------------------------------------
extern const wchar_t WindowHolderXuiObjectStr[];
extern const wchar_t WindowHolderXuiObjectStrCompat[];
extern char WindowHolderXuiSvcName[];
extern char WindowHolderXuiSvcNameCompat[];
class WindowHolderXuiSvc : public XuiObjectSvc<XuiWindowHolder, WindowHolderXuiObjectStr, WindowHolderXuiSvcName> {};
class WindowHolderXuiSvc2 : public XuiObjectSvc<XuiWindowHolder, WindowHolderXuiObjectStrCompat, WindowHolderXuiSvcNameCompat> {};

#endif
