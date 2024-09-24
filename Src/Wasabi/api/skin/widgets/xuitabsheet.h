#ifndef __SCRIPTTABSHEET_H
#define __SCRIPTTABSHEET_H

#include <api/wnd/wndclass/typesheet.h>
#include <api/script/objcontroller.h>

#define  SCRIPTTABSHEET_PARENT TypeSheet

// -----------------------------------------------------------------------
// Your wnd object class

class ScriptTabSheet : public  SCRIPTTABSHEET_PARENT {
  
  public:

    ScriptTabSheet();
    virtual ~ScriptTabSheet();

    // XuiObject automatically calls this back for all parameters registered using addParam
    // encountered in the xml source
    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    void setWindowType(const wchar_t *elementname);
    void setChildrenIds(const wchar_t *paramvalue);
    void setType(const wchar_t *element);
    virtual int onInit();

    void setContentMarginX(const wchar_t *value, int what);
protected:
	void CreateXMLParameters(int master_handle);
  private:

    // a list of IDs for our xml attributes, we use them in addParam() in the constructor
    enum {
       SCRIPTTABSHEET_SETWINDOWTYPE = 0,
       SCRIPTTABSHEET_SETCHILDREN,
       SCRIPTTABSHEET_SETTYPE,
       SCRIPTTABSHEET_SETCONTENTMARGINLEFT,
       SCRIPTTABSHEET_SETCONTENTMARGINTOP,
       SCRIPTTABSHEET_SETCONTENTMARGINRIGHT,
       SCRIPTTABSHEET_SETCONTENTMARGINBOTTOM,
    };
		static XMLParamPair params[];
    int myxuihandle;
    PtrList<StringW> children_id;
    void reloadChildren();
    int type;
    StringW wndtype;

    int left_margin, right_margin, top_margin, bottom_margin;
};


// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

extern const wchar_t ScriptTabSheetXuiObjectStr[];
extern char ScriptTabSheetXuiSvcName[];
class ScriptTabSheetXuiSvc : public XuiObjectSvc<ScriptTabSheet, ScriptTabSheetXuiObjectStr, ScriptTabSheetXuiSvcName> {};

// -----------------------------------------------------------------------------------------------------
class ScriptTabSheetController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"TabSheet"; }
    virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return tabsheetGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  public:
    static scriptVar tabsheet_getCurPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar tabsheet_setCurPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a);
    static scriptVar tabsheet_getNumPages(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar tabsheet_nextPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar tabsheet_previousPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  private:

    static function_descriptor_struct exportedFunction[];
};

extern ScriptTabSheetController *tabsheetController;

#endif

