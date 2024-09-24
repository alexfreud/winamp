#ifndef __DROPDOWNLIST_H
#define __DROPDOWNLIST_H

#include <api/wnd/popexitcb.h>
#include <api/wnd/wndclass/embeddedxui.h>
#include <api/script/objects/c_script/h_guiobject.h>
#include <api/script/objects/c_script/h_button.h>
#include <api/skin/feeds/feedwatch.h>
#include <api/script/objcontroller.h>

#define  DROPDOWNLIST_PARENT EmbeddedXuiObject

class DDLClicksCallback;
class DDLKeyCallback;
class svc_textFeed;

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class DDLEntry {
  public:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    DDLEntry(const wchar_t *txt) : text(txt), id(id_gen++) { } 
    const wchar_t *getText() { return text; }
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    int getId() { return id; }

  private:
    StringW text;
    int id;
    static int id_gen;
};


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class SortDDLEntries{
public:
  
  static int compareItem(DDLEntry *p1, DDLEntry *p2) {
    
    return WCSICMP(p1->getText(), p2->getText());
  }
  
  static int compareAttrib(const wchar_t *attrib, DDLEntry *item) 
	{
    return WCSICMP(attrib, item->getText());
  }
};




/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class DropDownList : public DROPDOWNLIST_PARENT, public PopupExitCallbackI, public FeedWatcher, public DependentViewerI {
  
  public:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    DropDownList();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual ~DropDownList();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual int onInit();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void clickCallback();
    void escapeCallback();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
#ifdef WASABI_COMPILE_CONFIG
    virtual int onReloadConfig();
#endif
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void abstract_onNewContent();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void setListHeight(int h) { height = h; }

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual int popupexitcb_onExitPopup();
    virtual api_dependent *popupexit_getDependencyPtr() { return rootwnd_getDependencyPtr(); }
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void openList();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void closeList();

   
    void setItems(const wchar_t *lotsofitems);
 
    int addItem(const wchar_t *text); 
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void delItem(int id);
    

    int findItem(const wchar_t *text);

    int getNumItems() { return items.getNumItems(); }
    DDLEntry *enumItem(int i) { return items.enumItem(i); }
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void selectItem(int id, int hover=0);
    const wchar_t *getItemText(int id);
    
    int getSelected() { return selected; }
    const wchar_t *getSelectedText() { int a = getSelected(); if (a == -1) return getCustomText(); return getItemText(a); }
    virtual const wchar_t *getCustomText() { return noitemtext; }
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void deleteAllItems();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void onSelect(int id, int hover=0);

    virtual void setNoItemText(const wchar_t *txt);

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual int childNotify(ifc_window *child, int msg, intptr_t param1=0, intptr_t param2=0);
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual int viewer_onItemDeleted(api_dependent *item);

    virtual void feedwatcher_onSetFeed(svc_textFeed *svc);
    virtual void feedwatcher_onFeedChange(const wchar_t *data);
    
    virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);
    
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void selectDefault();
    
    virtual void setMaxItems(int _maxitems) { maxitems = _maxitems; }
    virtual int getMaxItems() { return maxitems; }

    virtual int wantTrapButton() { return 1; }
    virtual int wantTrapText() { return 1; }
    virtual int wantFocus() { return 1; }

    virtual const wchar_t *dropdownlist_getMainGroupId() { return L"wasabi.dropdownlist.main.group"; }
    virtual const wchar_t *dropdownlist_getListGroupId() { return L"wasabi.dropdownlist.list.group"; }
    virtual const wchar_t *dropdownlist_getTextId() { return L"dropdownlist.text"; }
    virtual const wchar_t *dropdownlist_getButtonId() { return L"dropdownlist.button"; }
    virtual const wchar_t *dropdownlist_getListId() { return L"dropdownlist.list"; }

    virtual void updateTextInControl(const wchar_t *txt);

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    virtual const wchar_t *embeddedxui_getContentId() { return dropdownlist_getMainGroupId(); }
    virtual const wchar_t *embeddedxui_getEmbeddedObjectId() { return dropdownlist_getTextId(); }

    int isListOpen() { return list_group != NULL; };
    virtual int wantAutoSort() { return 1; }

    virtual void dropdownlist_onCloseList();
    virtual void dropdownlist_onOpenList();

    virtual void dropdownlist_onConfigureList(GuiObject *o);
    virtual int onKeyDown(int keyCode);
    virtual int onKeyUp(int keyCode);
    virtual int onAcceleratorEvent(const wchar_t *name);

    virtual void onPreCloseList() {}
    virtual void onPreOpenList() {}
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    enum {
      DROPDOWNLIST_SETITEMS = 0,
      DROPDOWNLIST_SETFEED,
      DROPDOWNLIST_SELECT,
      DROPDOWNLIST_LISTHEIGHT,
      DROPDOWNLIST_MAXITEMS,
  	  DROPDOWNLIST_SETLISTANTIALIAS,
    };
    int myxuihandle;
		static XMLParamPair params[];

  private:

#ifdef WASABI_COMPILE_CONFIG
    void updateTextFromConfig();
#endif
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void trapControls();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void setListParams();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void doCloseList(int cb=1);
    
    DDLClicksCallback *clicks_button;
    DDLClicksCallback *clicks_text;
    DDLKeyCallback *list_key;
    ifc_window *list_group;
    PtrListInsertSorted<DDLEntry, SortDDLEntries> items;
    int selected;

    int height;
    int maxitems;
    StringW noitemtext;
    int trap_click;
    api_dependent *group_dep;
    ifc_window *action_list;
    int disable_cfg_event;
    ifc_window *listif;
	int listAntialias;
};


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class DDLClicksCallback : public H_GuiObject {
  public:
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    DDLClicksCallback(ScriptObject *trap, DropDownList *_callback) :
        
        /**
          Method
        
          @see 
          @ret 
          @param 
        */
        callback(_callback), H_GuiObject(trap) {
    }

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onLeftButtonDown(int x, int y) {
      callback->clickCallback();
    }
    virtual void hook_onChar(wchar_t c) 
  {
#ifdef _WIN32
      if (c == VK_SPACE || c == VK_RETURN)
        callback->clickCallback();
#else
#warning port me
#endif
    }
  private:
    DropDownList *callback;
};

class DDLKeyCallback : public H_GuiObject {
  public:
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    DDLKeyCallback(ScriptObject *trap, DropDownList *_callback) :
        
        /**
          Method
        
          @see 
          @ret 
          @param 
        */
        callback(_callback), H_GuiObject(trap) {
    }

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */

    virtual void hook_onChar(wchar_t c) 
  {
#ifdef _WIN32
      if (c == VK_ESCAPE)
        callback->escapeCallback();
#else
#warning port me
#endif
    }
  private:
    DropDownList *callback;
};

// -----------------------------------------------------------------------
class DropDownListScriptController: public ScriptObjectControllerI {
public:
  virtual const wchar_t *getClassName() { return L"DropDownList"; }
  virtual const wchar_t *getAncestorClassName() { return L"ObjectEmbedder"; }
  virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(embeddedXuiGuid); }
  virtual int getNumFunctions();
  virtual const function_descriptor_struct *getExportedFunctions();
  virtual GUID getClassGuid() { return dropDownListGuid; }
  virtual ScriptObject *instantiate();
  virtual void destroy(ScriptObject *o);
  virtual void *encapsulate(ScriptObject *o);
  virtual void deencapsulate(void *o);

  // public cause it's called by the xui object.
  static scriptVar DropDownList_onSelect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar id, scriptVar hover);

private:

  static function_descriptor_struct exportedFunction[];
  static scriptVar DropDownList_getItemSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  static /*void*/ scriptVar DropDownList_setListHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar h);
  static /*void*/ scriptVar DropDownList_openList(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static /*void*/ scriptVar DropDownList_closeList(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static /*void*/ scriptVar DropDownList_setItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar lotsofitems);
  static /*int*/ scriptVar DropDownList_addItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar text); 
  static /*void*/ scriptVar DropDownList_delItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar id);
  static /*int*/ scriptVar DropDownList_findItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar text);
  static /*int*/ scriptVar DropDownList_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static /*void*/ scriptVar DropDownList_selectItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar id, /*int*/ scriptVar hover);
  static /*String*/ scriptVar DropDownList_getItemText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar id);
  static /*int*/ scriptVar DropDownList_getSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static /*String*/ scriptVar DropDownList_getSelectedText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static /*String*/ scriptVar DropDownList_getCustomText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static /*void*/ scriptVar DropDownList_deleteAllItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static /*void*/ scriptVar DropDownList_setNoItemText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar txt);


};

extern COMEXP DropDownListScriptController *dropDownListController;

#endif
