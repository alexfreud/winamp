//PORTABLE
#ifndef _ITEMLIST_H
#define _ITEMLIST_H

#include "listwnd.h"
#include "../canvas.h"
#include "../named.h"
#include "../ptrlist.h"
#include "../string.h"
#include "../../studio/metacb.h"

class FilenameNC;
class DragItemI;
class ContextMenu;

// this class just handles rendering the various properties of playitems
// in a listwnd... the rest is up to you, just override the convert fn

// abstract base class to render something in a column for a playstring

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class NOVTABLE ItemListColumn : public ListColumn {
protected:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  ItemListColumn(const wchar_t *name=NULL) : ListColumn(name) {}
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual ~ItemListColumn() {}

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void render(int pos, const wchar_t *playstring, Canvas &c, RECT &r)=0;
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */

  virtual void columnToText(int pos, const wchar_t *playstring, wchar_t *str, int maxlen)=0;
};

#define ITEMLISTWND_PARENT ListWnd

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class ItemListWnd : public ListWnd, private MetaCallbackI {
friend class ItemListColumn_Callback;
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  ItemListWnd();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual ~ItemListWnd();

  
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
  int insertColumn(ItemListColumn *column, int width, int pos=-1);

protected:
  // override and return 0 to suppress auto-dragging from window
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int wantAutoDrag() { return 1; }
  // handles auto-adding all selected rows and calls addDragTypes
  // so you can add more via addDragItem()
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onBeginDrag(int);
  // if you return 0, the Filename version will be auto-added, otherwise not
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int addMoreDragTypes(int pos) { return 0; }

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int dragComplete(int success);

  // tell ListWnd we do our own drawing
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int ownerDraw(Canvas *canvas, int pos, RECT *r, LPARAM lParam, int isselected, int isfocused);

  // ItemListColumn_Callback calls this to do its rendering, lParam is what you
  // gave it to pass back to you
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void userRender(int pos, const wchar_t *playstring, Canvas &c, RECT &r, LPARAM lParam) {}

  // ItemListColumn_Callback calls this to get the column text, lParam is what you
  // gave it to pass back to you
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void userColumnToText(int pos, const wchar_t *playstring, LPARAM lParam, wchar_t *str, int maxlen) {}

  // override this to turn the ownerdraw into a playstring
  virtual const wchar_t *convertlParam(LPARAM lParam)=0;
  virtual void convertlParamColumn(int col, int pos, LPARAM lParam, wchar_t *str, int maxlen);

  // override this and return 1 if you want a "current" box around item
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int getSelected(LPARAM lParam) { return 0; }

//  virtual int onRightClick(int itemnum, int x, int y);
  // automatically generated context menu (uses Filename)
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onContextMenu(int x, int y);

  // return optional DragItemI for context menu (will be deleted for you)
  virtual DragItemI *itemlistwnd_getDragItem(int x, int y) { return NULL; }
  virtual DragItemI *itemlistwnd_getSecondDragItem(int n) { return NULL; }
  virtual void itemlistwnd_addCustomContextMenuCommands(ContextMenu *cm) { }
  virtual void itemlistwnd_contextMenuResult(int res) { }

  // return TRUE if it's ok to edit in place
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int allowEdition(const wchar_t *playstring, wchar_t *field) { return 0; }

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void resort() { 
    //TODO> implement me!
  }

protected:
  // implement this if you want to know when an item's metadata changed
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void onItemChange(int pos, const wchar_t *playstring) { }
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void onItemDel(int pos, const wchar_t *playstring) { }

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void metacb_onItemChange(const wchar_t *playstring, const wchar_t *tag);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void metacb_onItemDel(const wchar_t *);

private:
  PtrList<FilenameNC> *keep;
};

// column class to ask ItemListWnd to do the rendering

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class ItemListColumn_Callback : public ItemListColumn {
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  ItemListColumn_Callback(ItemListWnd *_parent, LPARAM _lparam, const wchar_t *name=NULL);

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void render(int pos, const wchar_t *playstring, Canvas &c, RECT &r);
  virtual void columnToText(int pos, const wchar_t *playstring, wchar_t *str, int maxlen);

private:
  ItemListWnd *parent;
  LPARAM lparam;
};

// column class to render a metatag

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class ItemListColumn_MetaTag : public ItemListColumn {
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  ItemListColumn_MetaTag(const wchar_t *tag, int center=0, const wchar_t *label=NULL);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual ~ItemListColumn_MetaTag() {}

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void render(int pos, const wchar_t *playstring, Canvas &c, RECT &r);
  virtual void columnToText(int pos, const wchar_t *playstring, wchar_t *str, int maxlen);

  const wchar_t *getTag();

private:
  StringW tag;
  int center;
  int datatype;
};

// this just renders the position of the item, starting from 1

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
 */
class ItemListColumn_Numbered : public ItemListColumn {
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  ItemListColumn_Numbered(int _offset=0) : offset(_offset) {}
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void render(int pos, const wchar_t *playstring, Canvas &c, RECT &r);
  virtual void columnToText(int pos, const wchar_t *playstring, wchar_t *str, int maxlen);

private:
  int offset;
};

#endif
