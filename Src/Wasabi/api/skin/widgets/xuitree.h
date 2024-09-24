#ifndef __XUITREE_H
#define __XUITREE_H

#include <map>
#include <api/wnd/wndclass/treewnd.h>
#include <api/script/objcontroller.h>
#include <bfc/depend.h>

class HPNode;
class ScriptTreeItem;
class svc_textFeed;

#define SCRIPTTREE_PARENT TreeWnd

typedef std::map<TreeItem *,ScriptTreeItem *>  ScriptTreeMap;

// -----------------------------------------------------------------------
class ScriptTree : public SCRIPTTREE_PARENT, public DependentViewerI {
  
  public:

    ScriptTree();
    virtual ~ScriptTree();

    virtual int onInit();

    int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
    int onReloadConfig();

    virtual int viewer_onEvent(api_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen);
    virtual void onSetVisible(int i);

    virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);

    // Find a ScriptTreeItem to wrap a TreeItem
    ScriptTreeItem *bindScriptTreeItem(TreeItem *item);

    // Someone is deleting a ScriptTreeItem so we should stop tracking it.
    int destroyScriptTreeItem(ScriptTreeItem *item);

    // Transfer a TreeItem from our tree to a different tree (or global space)
    int transferScriptTreeItem(TreeItem *item, ScriptTree *tree);

    // Callback methods that send hooks into the Script system
    virtual int onLeftButtonDown(int x, int y);
    virtual int onLeftButtonUp(int x, int y);
    virtual int onRightButtonUp(int x, int y);
    virtual int onMouseMove(int x, int y);
    virtual int wantAutoContextMenu();
    virtual int onLeftButtonDblClk(int x, int y);
    virtual int onRightButtonDblClk(int x, int y);
    virtual int onMouseWheelUp(int clicked, int lines);
    virtual int onMouseWheelDown(int clicked, int lines);
    virtual int onContextMenu(int x, int y);
    virtual int onChar(wchar_t c);
    virtual int onKeyDown(int keycode);
    virtual void onItemRecvDrop(TreeItem *item);
    virtual void onLabelChange(TreeItem *item);
    virtual void onItemSelected(TreeItem *item);
    virtual void onItemDeselected(TreeItem *item);
    virtual int onKillFocus();



    // Valid XML Params for Tree
    enum {
      SCRIPTTREE_SETITEMS = 0,
      SCRIPTTREE_FEED,
      SCRIPTTREE_SORTED,
      SCRIPTTREE_CHILDTABS,
      SCRIPTTREE_EXPANDROOT,
    };

protected:
	/*static */void CreateXMLParameters(int master_handle);

  private:

#ifdef WASABI_COMPILE_CONFIG
    void saveToConfig();
    void selectFromConfig();
#endif
    void expandRoot(int val);

    void fillFromParams();
    void fillFromHPNode(HPNode *node, TreeItem *parent = NULL);

    int selectEntry(const wchar_t *e, int cb=1);
    void selectEntries(const wchar_t *multientry, int cb=1);

    void openFeed(const wchar_t *feedid);
    void closeFeed();
    
    StringW items;
    int myxuihandle;
    int childtabs;
    int expandroot;
    svc_textFeed *feed;
    StringW last_feed;
		static XMLParamPair params[];

//    ScriptTreeMap scriptitems;  
};

// -----------------------------------------------------------------------------------------------------
class GuiTreeScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"GuiTree"; }
    virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return guitreeGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  public:
    static scriptVar /*int*/ guitree_getNumRootItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar /*TreeItem*/ guitree_enumRootItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar /*int*/ which);

    static /*Int*/ scriptVar guitree_onLeftButtonDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);
    static /*Int*/ scriptVar guitree_onLeftButtonUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);
    static /*Int*/ scriptVar guitree_onRightButtonUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);
    static /*Int*/ scriptVar guitree_onMouseMove(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);
    static /*Int*/ scriptVar guitree_wantAutoContextMenu(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*Int*/ scriptVar guitree_onLeftButtonDblClk(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);
    static /*Int*/ scriptVar guitree_onRightButtonDblClk(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);
    static /*Int*/ scriptVar guitree_onMouseWheelUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar clicked, /*Int*/ scriptVar lines);
    static /*Int*/ scriptVar guitree_onMouseWheelDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar clicked, /*Int*/ scriptVar lines);
    static /*Int*/ scriptVar guitree_onContextMenu(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);
    static /*Int*/ scriptVar guitree_onChar(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar c);
    static /*Int*/ scriptVar guitree_onKeyDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar keycode);
    static /*Void*/ scriptVar guitree_onItemRecvDrop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_onLabelChange(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_onItemSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_onItemDeselected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Int*/ scriptVar guitree_onKillFocus(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

    static /*Void*/ scriptVar guitree_jumpToNext(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar c);
    static /*Void*/ scriptVar guitree_ensureItemVisible(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Int*/ scriptVar guitree_getContentsWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*Int*/ scriptVar guitree_getContentsHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*TreeItem*/ scriptVar guitree_addTreeItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item, /*TreeItem*/ scriptVar par, /*Int*/ scriptVar sorted, /*Int*/ scriptVar haschildtab);
    static /*Int*/ scriptVar guitree_removeTreeItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_moveTreeItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item, /*TreeItem*/ scriptVar newparent);
    static /*Void*/ scriptVar guitree_deleteAllItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*Int*/ scriptVar guitree_expandItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_expandItemDeferred(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Int*/ scriptVar guitree_collapseItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_collapseItemDeferred(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_selectItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_selectItemDeferred(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_delItemDeferred(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_hiliteItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_unhiliteItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*TreeItem*/ scriptVar guitree_getCurItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*TreeItem*/ scriptVar guitree_hitTest(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);
    static /*Void*/ scriptVar guitree_editItemLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_cancelEditLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar destroyit);
    static /*Void*/ scriptVar guitree_setAutoEdit(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar ae);
    static /*Int*/ scriptVar guitree_getAutoEdit(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*TreeItem*/ scriptVar guitree_getByLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item, /*String*/ scriptVar  name);
    static /*Void*/ scriptVar guitree_setSorted(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar dosort);
    static /*Int*/ scriptVar  guitree_getSorted(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*Void*/ scriptVar guitree_sortTreeItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*TreeItem*/ scriptVar guitree_getSibling(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Void*/ scriptVar guitree_setAutoCollapse(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar doautocollapse);
    static /*Int*/ scriptVar guitree_setFontSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar newsize);
    static /*Int*/ scriptVar guitree_getFontSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*Int*/ scriptVar guitree_getNumVisibleChildItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar c);
    static /*Int*/ scriptVar guitree_getNumVisibleItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*TreeItem*/ scriptVar guitree_enumVisibleItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar n);
    static /*TreeItem*/ scriptVar guitree_enumVisibleChildItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar c, /*Int*/ scriptVar n);
    static /*TreeItem*/ scriptVar guitree_enumAllItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar n);
    static /*Int*/ scriptVar guitree_getItemRectX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Int*/ scriptVar guitree_getItemRectY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Int*/ scriptVar guitree_getItemRectW(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
    static /*Int*/ scriptVar guitree_getItemRectH(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar item);
//    static /*TreeItem*/ scriptVar guitree_getItemFromPoint(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*Int*/ scriptVar x, /*Int*/ scriptVar y);

  private:
    static function_descriptor_struct exportedFunction[];

    static StringW staticStr;
};

extern GuiTreeScriptController *guiTreeController;


// -----------------------------------------------------------------------
extern const wchar_t ScriptTreeXuiObjectStr[];
extern char ScriptTreeXuiSvcName[];
class ScriptTreeXuiSvc : public XuiObjectSvc<ScriptTree, ScriptTreeXuiObjectStr, ScriptTreeXuiSvcName> {};



// -----------------------------------------------------------------------
#define SCRIPTTREEITEM_SCRIPTPARENT RootObjectInstance

class ScriptTreeItem : public SCRIPTTREEITEM_SCRIPTPARENT {
public:
  ScriptTreeItem(TreeItem *_item = NULL, ScriptTree *_tree = NULL);
  virtual ~ScriptTreeItem();

  TreeItem *getItem() {return item;}
  void setItem(TreeItem *_item) {item = _item;}
  ScriptTree *getScriptTree() {return tree;}
  void setScriptTree(ScriptTree *_tree) {tree = _tree;}

  int destroyScriptTreeItem() {
    if (tree) return tree->destroyScriptTreeItem(this); // CAREFUL, WE GET OURSELVES DELETED HERE!!!!!!!!
    return 0;
  }

// These methods all thunk directly to the TreeItem
public:
  int getNumChildren();
  void setLabel(const wchar_t *label);
  const wchar_t *getLabel();
  void ensureVisible();
  TreeItem *getNthChild(int nth);
  TreeItem *getChild();
  TreeItem *getChildSibling(TreeItem *_item);
  TreeItem *getSibling();
  TreeItem *getParent();
  void editLabel();
  bool hasSubItems();
  void setSorted(int issorted);
  void setChildTab(int haschildtab);
  bool isSorted();
  bool isCollapsed();
  bool isExpanded();
  void invalidate();
  bool isSelected();
  bool isHilited();
  void setHilited(bool ishilited);
  int collapse();
  int expand();
//  void setCurrent(bool tf);
  TreeWnd *getTree();
  
private:
  ScriptTreeItem *bindScriptTreeItem(TreeItem *item) {
    if (tree) return tree->bindScriptTreeItem(item);
    return NULL;
  }
  
  TreeItem *item;
  ScriptTree *tree;
};


// -----------------------------------------------------------------------------------------------------
class TreeItemScriptController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"TreeItem"; }
    virtual const wchar_t *getAncestorClassName() { return L"Object"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(rootObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return treeitemGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  public:
    static /*int*/ scriptVar treeitem_getNumChildren(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_setLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar label);
    static /*String*/ scriptVar treeitem_getLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_ensureVisible(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*TreeItem*/ scriptVar treeitem_getNthChild(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar nth);
    static /*TreeItem*/ scriptVar treeitem_getChild(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*TreeItem*/ scriptVar treeitem_getChildSibling(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*TreeItem*/ scriptVar _item);
    static /*TreeItem*/ scriptVar treeitem_getSibling(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*TreeItem*/ scriptVar treeitem_getParent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_editLabel(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar treeitem_hasSubItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_setSorted(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar issorted);
    static /*void*/ scriptVar treeitem_setChildTab(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar haschildtab);
    static /*int*/ scriptVar treeitem_isSorted(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar treeitem_isCollapsed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar treeitem_isExpanded(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_invalidate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar treeitem_isSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar treeitem_isHilited(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_setHilited(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar ishilited);
    static /*int*/ scriptVar treeitem_collapse(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar treeitem_expand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
//    static /*void*/ scriptVar treeitem_setCurrent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar tf);
    static /*GuiTree*/ scriptVar treeitem_getTree(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

    static /*void*/ scriptVar treeitem_onTreeAdd(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_onTreeRemove(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_onSelect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*void*/ scriptVar treeitem_onDeselect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar treeitem_onLeftDoubleClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) ;
    static /*int*/ scriptVar treeitem_onRightDoubleClick(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) ;
    static /*int*/ scriptVar treeitem_onChar(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar _key) ;
    static /*void*/ scriptVar treeitem_onExpand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) ;
    static /*void*/ scriptVar treeitem_onCollapse(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) ;
    static /*int*/ scriptVar treeitem_onBeginLabelEdit(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static /*int*/ scriptVar treeitem_onEndLabelEdit(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar _newlabel);
    static /*int*/ scriptVar treeitem_onContextMenu(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar _x, /*int*/ scriptVar _y);


  private:
    static function_descriptor_struct exportedFunction[];

    static StringW staticStr;

    friend ScriptTree;
    static ScriptTreeMap g_scriptitems;  // items not living in trees are tracked here.

};

extern TreeItemScriptController *treeItemController;

#define TISC TreeItemScriptController 


#endif
