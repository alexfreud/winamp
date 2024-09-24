#ifndef __XUILIST_H
#define __XUILIST_H

#include <api/wnd/wndclass/listwnd.h>
#include <api/script/objcontroller.h>
#include <bfc/depend.h>

class svc_textFeed;

#define SCRIPTLIST_PARENT ListWnd

// -----------------------------------------------------------------------
class ScriptList : public SCRIPTLIST_PARENT, public DependentViewerI {
  public:
    ScriptList();
    virtual ~ScriptList();

    virtual int onInit();

  //virtual void onDoubleClick(int itemnum); // moved to the script-handling callback.
  //void onItemSelection(int itemnum, int selected);
    
    int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
#ifdef WASABI_COMPILE_CONFIG
    int onReloadConfig();
#endif

    virtual int viewer_onEvent(api_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen);
    virtual void onSetVisible(int i);

    virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);

    // Callback methods that send hooks into the Script system
    virtual void onSelectAll();
    virtual void onDelete();
    virtual void onDoubleClick(int itemnum);
    virtual void onLeftClick(int itemnum);
    virtual void onSecondLeftClick(int itemnum);
    virtual int onRightClick(int itemnum);
    virtual int onColumnDblClick(int col, int x, int y);
    virtual int onColumnLabelClick(int col, int x, int y);
    virtual void onItemSelection(int itemnum, int selected);
	virtual int onIconLeftClick(int itemnum, int x, int y);
	


    enum {
      SCRIPTLIST_SETITEMS = 0,
      SCRIPTLIST_SETMULTISELECT,
      SCRIPTLIST_SETAUTODESELECT,
      SCRIPTLIST_SELECT,
      SCRIPTLIST_FEED,
      SCRIPTLIST_HOVERSELECT,
      SCRIPTLIST_SORT,
      SCRIPTLIST_SELECTONUPDOWN,
      SCRIPTLIST_NUMCOLUMNS,
      SCRIPTLIST_COLUMNWIDTHS,
      SCRIPTLIST_COLUMNLABELS,
    };
		
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:
	static XMLParamPair params[];
#ifdef WASABI_COMPILE_CONFIG
    void saveToConfig();
    void selectFromConfig();
#endif
    void fillFromParams();
    int selectEntry(const wchar_t *e, int cb=1);
    void selectEntries(const wchar_t *multientry, int cb=1);
    void setNumColumns();
    void setColumnWidths();
    void setColumnLabels();

    void openFeed(const wchar_t *feedid);
    void closeFeed();
    
    //virtual int getColumnsHeight() { return 0; }
    virtual int wantHScroll() { return 0; }

    StringW items;
    StringW columnwidths;
    StringW columnlabels;
    int xmlnumcolumns;
    int last_numcolumns;
    int multiselect;
    int myxuihandle;
    int autosave;
#ifdef WASABI_COMPILE_CONFIG
    int config_reentry;
#endif

    svc_textFeed *feed;
    StringW last_feed;
};

// -----------------------------------------------------------------------------------------------------
class GuiListScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"GuiList"; }
    virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return guilistGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  public:
    static scriptVar guilist_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_getWantAutoDeselect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_setWantAutoDeselect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar want);
    static /*void*/ scriptVar guilist_onSetVisible(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar show);
    static /*void*/ scriptVar guilist_setAutoSort(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar dosort);
    static /*void*/ scriptVar guilist_next(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_selectCurrent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_selectFirstEntry(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_previous(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_pagedown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_pageup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_home(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_end(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_reset(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_addColumn(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar name, /*int*/ scriptVar width, /*int*/ scriptVar numeric);
    static /*int*/ scriptVar guilist_getNumColumns(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_getColumnWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar column);
    static /*void*/ scriptVar guilist_setColumnWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar column, /*int*/ scriptVar newwidth);
    static /*String*/ scriptVar guilist_getColumnLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar column);
    static /*void*/ scriptVar guilist_setColumnLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar column, /*String*/ scriptVar newlabel);
    static /*int*/ scriptVar guilist_getColumnNumeric(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar column);
    static /*void*/ scriptVar guilist_setColumnDynamic(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar column, /*int*/ scriptVar isdynamic);
    static /*int*/ scriptVar guilist_isColumnDynamic(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar column);
    static /*void*/ scriptVar guilist_setMinimumSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar size);
    static /*int*/ scriptVar guilist_addItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar label);
    static /*int*/ scriptVar guilist_insertItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos, /*String*/ scriptVar label);
    static /*int*/ scriptVar guilist_getLastAddedItemPos(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_setSubItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos, /*int*/ scriptVar subpos, /*String*/ scriptVar txt);
    static /*void*/ scriptVar guilist_deleteAllItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_deleteByPos(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*String*/ scriptVar guilist_getItemLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos, /*int*/ scriptVar subpos);
    static /*void*/ scriptVar guilist_setItemLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos, /*String*/ scriptVar text);
	
	static /*void*/ scriptVar guilist_setItemIcon(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos, /*String*/ scriptVar bitmapId);
	static /*string*/ scriptVar guilist_getItemIcon(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
	static /*void*/ scriptVar guilist_setShowIcons(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar onoff);
	static /*int*/ scriptVar guilist_getShowIcons(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static /*int*/ scriptVar guilist_getIconWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static /*void*/ scriptVar guilist_setIconWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val);
	static /*int*/ scriptVar guilist_getIconHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
	static /*void*/ scriptVar guilist_setIconHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar val);
	static /*int*/ scriptVar guilist_onIconLeftClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar pos, scriptVar x, scriptVar y);

    static /*int*/ scriptVar guilist_getItemSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*int*/ scriptVar guilist_isItemFocused(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*int*/ scriptVar guilist_getItemFocused(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_setItemFocused(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*void*/ scriptVar guilist_ensureItemVisible(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*void*/ scriptVar guilist_invalidateColumns(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_scrollAbsolute(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar x);
    static /*int*/ scriptVar guilist_scrollRelative(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar x);
    static /*void*/ scriptVar guilist_scrollLeft(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar lines);
    static /*void*/ scriptVar guilist_scrollRight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar lines);
    static /*void*/ scriptVar guilist_scrollUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar lines);
    static /*void*/ scriptVar guilist_scrollDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar lines);
    static /*String*/ scriptVar guilist_getSubitemText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos, /*int*/ scriptVar subpos);
    static /*int*/ scriptVar guilist_getFirstItemSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_getNextItemSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar lastpos);
    static /*int*/ scriptVar guilist_selectAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_deselectAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_invertSelection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_invalidateItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*int*/ scriptVar guilist_getFirstItemVisible(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_getLastItemVisible(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_setFontSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar size);
    static /*int*/ scriptVar guilist_getFontSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_jumpToNext(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*char*/ scriptVar c);
    static /*void*/ scriptVar guilist_scrollToItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*void*/ scriptVar guilist_resort(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_getSortDirection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_getSortColumn(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_setSortColumn(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar col);
    static /*void*/ scriptVar guilist_setSortDirection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar dir);
    static /*int*/ scriptVar guilist_getItemCount(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_setSelectionStart(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*void*/ scriptVar guilist_setSelectionEnd(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos);
    static /*void*/ scriptVar guilist_setSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos, /*int*/ scriptVar selected);
    static /*void*/ scriptVar guilist_toggleSelection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar pos, /*int*/ scriptVar setfocus);
    static /*int*/ scriptVar guilist_getHeaderHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_getPreventMultipleSelection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar guilist_setPreventMultipleSelection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar val);
    static /*void*/ scriptVar guilist_moveItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar from, /*int*/ scriptVar to);

    static /*void*/ scriptVar guilist_onSelectAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_onDelete(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar guilist_onDoubleClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar itemnum);
    static /*void*/ scriptVar guilist_onLeftClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar itemnum);
    static /*void*/ scriptVar guilist_onSecondLeftClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar itemnum);
    static /*int*/ scriptVar guilist_onRightClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar itemnum);
    static /*int*/ scriptVar guilist_onColumnDblClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar col, /*int*/ scriptVar x, /*int*/ scriptVar y);
    static /*int*/ scriptVar guilist_onColumnLabelClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar col, /*int*/ scriptVar x, /*int*/ scriptVar y);
    static /*void*/ scriptVar guilist_onItemSelection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar itemnum, /*int*/ scriptVar selected);

  private:
    static function_descriptor_struct exportedFunction[];

    static StringW staticStr;
};

extern GuiListScriptController *guiListController;


// -----------------------------------------------------------------------
extern const wchar_t ScriptListXuiObjectStr[];
extern char ScriptListXuiSvcName[];
class ScriptListXuiSvc : public XuiObjectSvc<ScriptList, ScriptListXuiObjectStr, ScriptListXuiSvcName> {};

#endif
