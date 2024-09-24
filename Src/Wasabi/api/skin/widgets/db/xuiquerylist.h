#ifndef __QUERYLIST_H
#define __QUERYLIST_H

#include <api/wnd/wndclass/guiobjwnd.h>
#ifdef WASABI_COMPILE_METADB
#include <api/skin/widgets/db/autoquerylist.h>
#endif
#include <api/script/objcontroller.h>

#ifdef WASABI_COMPILE_METADB
#define QUERYLIST_PARENT AutoQueryList
#else
#define QUERYLIST_PARENT GuiObjectWnd
#endif

// -----------------------------------------------------------------------
class ScriptQueryList : public QUERYLIST_PARENT {
  
  public:

    ScriptQueryList();
    virtual ~ScriptQueryList();

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    void setTitle(const char *name);

#ifdef WASABI_COMPILE_METADB
    void onResetSubqueries();
    virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, api_window *source=NULL);
#endif

  private:

    enum {
      QUERYLIST_SETTITLE = 0,
    };
		static XMLParamPair params[];
    int myxuihandle;
};

// -----------------------------------------------------------------------
class QueryListScriptController: public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"QueryList"; }
    virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return queryListGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

    static scriptVar querylist_onResetQuery(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern QueryListScriptController *queryListController;

// -----------------------------------------------------------------------
extern char QueryListXuiObjectStr[];
extern char QueryListXuiSvcName[];
class QueryListXuiSvc : public XuiObjectSvc<ScriptQueryList, QueryListXuiObjectStr, QueryListXuiSvcName> {};

#endif
