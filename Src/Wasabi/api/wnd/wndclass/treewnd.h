#ifndef _TREEWND_H
#define _TREEWND_H

// BU: lots of changes
// - all items must be deletable, and will be deleted on destructor
// - root items list not allocated w/ new
// - items set sorting within their PtrListSorted instead of manually calling it
// - setting an item to auto-sort does *not* make subitems autosort too

#include <api/wnd/wndclass/scbkgwnd.h>
#include <bfc/ptrlist.h>
#include <api/wnd/wndclass/editwnd.h>
#include <bfc/common.h>
#include <tataki/color/skinclr.h>

#define TREEWND_PARENT ScrlBkgWnd

#define STATUS_EXPANDED  0
#define STATUS_COLLAPSED 1

#define HITTEST_BEFORE   0
#define HITTEST_IN       1
#define HITTEST_AFTER    2

#define LINK_RIGHT       1
#define LINK_TOP         2
#define LINK_BOTTOM      4

#define TAB_NO           FALSE
#define TAB_YES          TRUE
#define TAB_AUTO         2

#define WM_SETITEMDEFERRED WM_USER+6546

#define DC_SETITEM	10
#define DC_DELITEM	20
#define DC_EXPAND	30
#define DC_COLLAPSE	40

// Forward references

class TreeItemList;
class TreeItem;
class TreeWnd;

class FontSize;

// classes & structs
class CompareTreeItem {
public:
  static int compareItem(TreeItem *p1, TreeItem *p2);
};

class TreeItemList : public PtrListQuickSorted<TreeItem, CompareTreeItem> { };

class TreeItem 
{
friend class TreeWnd;
public:
  TreeItem(const wchar_t *label=NULL);
  virtual ~TreeItem();

  virtual SkinBitmap *getIcon();
  virtual void setIcon(SkinBitmap *newicon);

  virtual void onTreeAdd() {}
  virtual void onTreeRemove() {}
  virtual void onChildItemRemove(TreeItem *item) {}
  // override this to keep from being selected
  virtual int isHitTestable() { return 1; }
  virtual void onSelect() {}
  virtual void onDeselect() {}
  virtual int onLeftDoubleClick() { return 0; }
  virtual int onRightDoubleClick() { return 0; }
  virtual int onContextMenu(int x, int y);
  virtual int onChar(UINT key) { return 0; }	// return 1 if you eat the key

  // these are called after the expand/collapse happens
  virtual void onExpand() {}
  virtual void onCollapse() {}

  virtual int onBeginLabelEdit();
  virtual int onEndLabelEdit(const wchar_t *newlabel);

  virtual void setLabel(const wchar_t *label);
  virtual const wchar_t *getLabel();

  void setTip(const wchar_t *tip);
  const wchar_t *getTip();

  // override to draw by yourself. Return the width of what you've drawn
  virtual int customDraw(Canvas *canvas, const POINT &pt, int defaultTxtHeight, int indentation, const RECT &clientRect, const Wasabi::FontInfo *fontInfo); 

  // return 0 to refuse being dragged
  // else return 1 and install the droptype and dropitem
  // also, write suggested title into suggestedTitle if any
  virtual int onBeginDrag(wchar_t *suggestedTitle) { return 0; }

  virtual int dragOver(ifc_window *sourceWnd) { return 0; }
  virtual int dragLeave(ifc_window *sourceWnd) { return 0; }
  virtual int dragDrop(ifc_window *sourceWnd) { return 0; }

  virtual int dragComplete(int success) { return 0; }

  void ensureVisible();

  TreeItem *getNthChild(int nth); // enumerates children (zero based)
  TreeItem *getChild();
  TreeItem *getChildSibling(TreeItem *item);
  TreeItem *getSibling();
  TreeItem *getParent();

  void editLabel();
	
  int getNumChildren();
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

  int getCurRect(RECT *r);

  void setCurrent(bool tf);

  TreeWnd *getTree() const;

protected:

  bool isHilitedDrop();
  void setHilitedDrop(bool ishilitedDrop);

  void linkTo(TreeItem *linkto);
//  void childDeleted(TreeItem *child);
  void setTree(TreeWnd *newtree);
  void addSubItem(TreeItem *item);
  void setCurRect(int x1, int y1, int x2, int y2, int z);
  int getIndent();

  bool needTab();
  void sortItems(); // sorts the children of this item
  void setEdition(bool isedited);
  bool getEdition();

private:
  void setSelected(bool isselected, bool expandCollapse=false, bool editifselected=false);
  // this really calls delete on the subitems
  void deleteSubitems();

  int removeSubitem(TreeItem *item);

  int getItemWidth(int txtHeight, int indentation);

  StringW label;
  class TreeItem *parent;
  TreeItemList subitems;	// children
  RECT curRect;
  int childTab;
  TreeWnd *tree;
  int expandStatus;
  SkinBitmap *icon;
  int _z;
  StringW tooltip;	// if empty, falls back to livetip

  bool selected:1;
  bool hilitedDrop:1;
  bool hilited:1;
  bool being_edited:1;
};


/**
  
  
  @short Tree-like view with leaf items.
  @ver 1.0
  @author Nullsoft
  @see TreeItem
*/
class TreeWnd : public TREEWND_PARENT {

friend class TreeItem;

public:
	
	/**
	  Sets up the default values for the TreeWnd. These defaults are 
	  auto collapse enabled and sets the TreeWnd bitmaps to the default Wasabi
	  values.
	*/
  TreeWnd();
  
  /**
    Deletes all the root items (including subitems).
  */
  virtual ~TreeWnd();

  /**  
    Event is triggered when the button is about to be initialized.
    Override this event to implement your own behavior.
  
    @ret 1
  */
  virtual int onInit();
  
  /**
    Paints the bitmap on canvas according 
	  to current options (centering, tiling, stretching, title).

    @ret 0 for failure, 1 for success
    @param canvas The canvas on which to paint.
  */
  virtual int onPaint(Canvas *canvas);
  
  /**
    Notify a child window via a generic message system.

    @see addChild()
    @ret 
    @param child A pointer to the child window which will receive the notify.
    @param msg The message you want to send to the child.
    @param p1 A user parameter.
    @param p2 A user parameter.
  */
  virtual int childNotify(ifc_window *child, int msg, intptr_t param1=0, intptr_t param2=0);

  /**
    Event triggered when the left mouse
    button is pressed over the TreeWnd.
    
    Override this to implement your own behavior.
    
    Default behavior is to stop editing a TreeItem label
    (if editing was occuring). Also will cause a collapse
    or expansion of the subitems if an item was previously
    selected. 
    
    @ret 1, If you handle the event.
    @param x The X coordinate of the mouse.
    @param y The Y coordinate of the mouse.
  */
  virtual int onLeftButtonDown(int x, int y);
  
  /**
    Event is triggered when the left mouse button 
    is released from a previously pressed state.
    
    Override this to implement your own behavior.
    
    @ret 1, If you handle the event.
    @param x The X coordinate of the mouse.
    @param y The Y coordinate of the mouse.
  */
  virtual int onLeftButtonUp(int x, int y);
  
  /**
    Event is triggered when the right mouse button 
    is released from a previously pressed state.
    
    Override this to implement your own behavior.
    
    @ret 1, If you handle the event.
    @param x The X coordinate of the mouse.
    @param y The Y coordinate of the mouse.
  */
  virtual int onRightButtonUp(int x, int y);
  
  /**
    Event is triggered when the mouse is moved 
    over the TreeWnd.
    
    Override this to implement your own behavior.
    
    Default is to handle drops (drag and drop).
    
    @ret 1, If you handle the event.
    @param x The X coordinate of the mouse.
    @param y The Y coordinate of the mouse.
  */
  virtual int onMouseMove(int x, int y);
  
  /**
    Do we want the context command menu to pop-up
    on right clicks?
    
    Default is no.
    
    @see ContextCmdI
    @ret 0, AutoContextMenu off; 1, AutoContextMenu on;
  */
  virtual int wantAutoContextMenu() { return 0; }

  /**
    Event is triggered when the left mouse button
    is double clicked and the cursor is over the
    TreeWnd.
    
    Default is to check if the doubleclick
    happened over an item, if it did, it calls
    the item's handler of this event.
    
    @ret 1, if you handle the event.
    @param x The X coordinate of the mouse.
    @param y The Y coordinate of the mouse.
  */
  virtual int onLeftButtonDblClk(int x, int y);
  
  /**
    Event is triggered when the right mouse button
    is double clicked and the cursor is over the
    TreeWnd.
    
    Default is to check if the doubleclick
    happened over an item, if it did, it calls
    the item's handler of this event.
    
    @ret 1, If you handle the event.
    @param x The X coordinate of the mouse.
    @param y The y coordinate of the mouse.
  */
  virtual int onRightButtonDblClk(int x, int y);

  /**
    Event is triggered when the mouse wheel 
    is rolled up.
    
    Override this to implement your own behavior.
    
    Default is to scroll vertically as required.
    When the wheel is clicked and rolled, the
    TreeWnd is scrolled horizontally.
    
    @ret 1, If you handle the event.
    @param clicked The pushed state of the mouse wheel.
    @param lines The number of lines to scroll (or columns if horizontally scrolling).
  */
  virtual int onMouseWheelUp(int clicked, int lines);
  
  /**
    Event is triggered when the mouse wheel 
    is rolled down.
    
    Override this to implement your own behavior.
    
    Default is to scroll vertically as required.
    When the wheel is clicked and rolled, the
    TreeWnd is scrolled horizontally.
    
    @ret 1, If you handle the event.
    @param clicked The pushed state of the mouse wheel.
    @param lines The number of lines to scroll (or columns if horizontally scrolling).
  */
  virtual int onMouseWheelDown(int clicked, int lines);
  
  /**
  */
  virtual void timerCallback(int c);

  /**
    Event is triggered when the right click occurs over 
    the TreeWnd, but not on a TreeItem.
    
    Override this to implement your own behavior.
        
    @ret 1, If you handle the event.
    @param x The X coordinate of the mouse.
    @param y The Y coordinate of the mouse.
  */
  virtual int onContextMenu(int x, int y);

  // override and return 1 to abort calling context menu on item
  virtual int onPreItemContextMenu(TreeItem *item, int x, int y) { return 0; }
  // override to catch when item context menu complete
  virtual void onPostItemContextMenu(TreeItem *item, int x, int y, int retval) { }
  
  /**
    Event is triggered when a scheduled deferred callback
    occurs.
    
    Override this to implement your own behavior.
    
    @ret 1, If you handle this event; 0, If you do not handle this event;
    @param param1 Generic user paramater 1.
    @param param2 Generic user paramater 2.
  */
  virtual int onDeferredCallback(intptr_t param1, intptr_t param2);

  /**
    Event is triggered when a key is pressed
    and the TreeWnd has focus.
    
    Override this to implement your own behavior.
    
    @ret 1, If you handle the event.
    @param c The key that was pressed.
  */
  virtual int onChar(unsigned int c);
  
  /**
    Event is triggered when a key is pressed
    and the TreeWnd has focus.
    
    This method handles extended keys.
    
    @ret 1, If you handle the event.
  */
  virtual int onKeyDown(int keycode);
  
  /**
    
  */
  virtual void jumpToNext(wchar_t c);
  
  /**
    Verifies if the item received is in the 
    viewable area of the TreeWnd. If not, it
    will make it visible by scrolling to the
    appropriate position.
    
    @param item A pointer to the item to verify.
  */
  void ensureItemVisible(TreeItem *item);

  // don't need to override this: just calls thru to the treeitem
  virtual int onBeginDrag(TreeItem *treeitem);

  virtual int dragEnter(ifc_window *sourceWnd);
  virtual int dragOver(int x, int y, ifc_window *sourceWnd);
  virtual int dragLeave(ifc_window *sourceWnd);
  virtual int dragDrop(ifc_window *sourceWnd, int x, int y);

  virtual int dragComplete(int success);

  int wantFocus() { return 1; }

  // override this if you want to control the item sort order
  virtual int compareItem(TreeItem *p1, TreeItem *p2);

protected:
  // these will be called if the pointer is not over a treeitem
  virtual int defaultDragOver(int x, int y, ifc_window *sourceWnd) { return 0; }
  virtual int defaultDragDrop(ifc_window *sourceWnd, int x, int y) { return 0; }

  // called with item that received a drop
  virtual void onItemRecvDrop(TreeItem *item) {}

  virtual void onLabelChange(TreeItem *item) {}

  virtual void onItemSelected(TreeItem *item) {}
  virtual void onItemDeselected(TreeItem *item) {}

  virtual int onGetFocus();
  virtual int onKillFocus();

public:

  virtual int getContentsWidth();
  virtual int getContentsHeight();

  void setRedraw(bool r);

  TreeItem *addTreeItem(TreeItem *item, TreeItem *par=NULL, int sorted=TRUE, int haschildtab=FALSE);

  // just removes a TreeItem from the tree, doesn't delete it... this is for
  // ~TreeItem to call only
  int removeTreeItem(TreeItem *item);

  void moveTreeItem(TreeItem *item, TreeItem *newparent);

  void deleteAllItems();

  int expandItem(TreeItem *item);
  void expandItemDeferred(TreeItem *item);
  int collapseItem(TreeItem *item);
  void collapseItemDeferred(TreeItem *item);

  void selectItem(TreeItem *item);	// selects.
  void selectItemDeferred(TreeItem *item);// selects. posted.
  void delItemDeferred(TreeItem *item);
  void hiliteItem(TreeItem *item);
  void unhiliteItem(TreeItem *item);
  void setHilitedColor(const wchar_t *colorname);
  ARGB32 getHilitedColor();

  TreeItem *getCurItem();

  TreeItem *hitTest(POINT pos);
  TreeItem *hitTest(int x, int y);

  void editItemLabel(TreeItem *item);
  void cancelEditLabel(int destroyit=0);
  void setAutoEdit(int ae);
  int getAutoEdit();
  // use a NULL item to search all items. returns first item found
  TreeItem *getByLabel(TreeItem *item, const wchar_t *name);

  int getItemRect(TreeItem *item, RECT *r);

  int ownerDraw();

  int getNumRootItems();
  TreeItem *enumRootItem(int which);

  void setSorted(bool dosort);
  bool getSorted();

  void sortTreeItems();

  TreeItem *getSibling(TreeItem *item);

  TreeItem *getItemFromPoint(POINT *pt);

  void setAutoCollapse(bool doautocollapse);

  virtual int setFontSize(int newsize);
  int getFontSize();

  int getNumVisibleChildItems(TreeItem *c);
  int getNumVisibleItems();
  TreeItem *enumVisibleItems(int n);
  TreeItem *enumVisibleChildItems(TreeItem *c, int n);
  int findItem(TreeItem *i); // reverse
  int findChildItem(TreeItem *c, TreeItem *i, int *n);

  TreeItem *enumAllItems(int n);	// unsorted

  void onSelectItem(TreeItem *i);
  void onDeselectItem(TreeItem *i);

protected:
  void hiliteDropItem(TreeItem *item);  
  void unhiliteDropItem(TreeItem *item);
  void invalidateMetrics();

private:
  TreeItemList items;	// root-level stuff

  PtrList<TreeItem> all_items;	// unsorted

  TreeItem *curSelected;

	BltCanvas *dCanvas;

	void drawItems(Canvas *c, const Wasabi::FontInfo *fontInfo);
  void setCurItem(TreeItem *item, bool expandCollapse=true, bool editifselected=false);
  void countSubItems(PtrList<TreeItem> &drawlist, TreeItemList *list, int indent, int *c, int *m, int z);
  void getMetrics(int *numItemsShow, int *maxWidth);
  void ensureMetricsValid();
  int getLinkLine(TreeItem *item, int level);
  void endEditLabel(const wchar_t *newlabel);
  void editUpdate();
  int jumpToNextSubItems(TreeItemList *list, wchar_t c);

  int itemHeight;

  AutoSkinBitmap tabClosed, tabOpen;
  AutoSkinBitmap linkTopRight, linkTopBottom, linkTopRightBottom;
  AutoSkinBitmap linkTabTopRight, linkTabTopBottom, linkTabTopRightBottom;
  
  TreeItem *firstItemVisible;
  TreeItem *lastItemVisible;

  TreeItem *mousedown_item, *prevbdownitem;
  POINT mousedown_anchor;
  bool mousedown_dragdone;
  TreeItem *hitItem,		// the dest item
           *draggedItem;	// the source item

  int inHitTest;

  bool metrics_ok;
  int maxWidth;
  int maxHeight;

  StringW defaultTip;

  const wchar_t *getLiveTip();
  void setLiveTip(const wchar_t *tip);
  TreeItem *tipitem;

  bool redraw;

  PtrList<TreeItem> drawList;
  TreeItem *edited;

  EditWnd *editwnd;
  wchar_t editbuffer[256];

  int deleteItems;
  bool firstFound;

  TreeItem *currentItem;
  StringW hilitedColorName;
  SkinColor hilitedColor;
  int autoedit;
  int autocollapse;
  int textsize;
  StringW oldtip;
  StringW accValue;
};

template<class T> class TreeItemParam : public TreeItem {
public:
  TreeItemParam(T _param, const wchar_t *label=NULL) : TreeItem(label) { param = _param; }

  T getParam() { return param; }
  operator T() { return getParam(); }

private:
  T param;
};

#endif
