#ifndef _LISTWND_H
#define _LISTWND_H

#include <api/wnd/wndclass/scbkgwnd.h>
#include <bfc/common.h>

#include <bfc/freelist.h>
#include "SelItemList.h"
#define POS_LAST -1

#define LISTWND_PARENT ScrlBkgWnd

#define LW_HT_DONTKNOW	(-1)
#define LW_HT_ABOVE	(-10)
#define LW_HT_BELOW	(-20)

#define COL_LEFTALIGN 0
#define COL_CENTERALIGN 1
#define COL_RIGHTALIGN 2

class listItem;
class ListWnd;
class CompareListItem;

class ListColumn : public NamedW
{
friend class ListWnd;
public:
  ListColumn(const wchar_t *name=NULL, int isdynamic=FALSE);
  virtual ~ListColumn() { }

  int getWidth();
  void setWidth(int newwidth);
  const wchar_t *getLabel();
  void setLabel(const wchar_t *newlabel);
	virtual int customDrawHeader(Canvas *c, RECT *cr, const Wasabi::FontInfo *fontInfo);
  virtual int onHeaderClick() { return 0; }//return 1 if you override
  virtual int onColumnLeftClick(int pos) { return 0; }//return 1 if you override
  int getNumeric() { return numeric; }
  void setDynamic(int isdynamic);
  int isDynamic() { return dynamic; }
  void setAlignment(int _align) { align = _align; }
  int getAlignment() { return align; }

protected:  	
  void setIndex(int i);
  int getIndex();
  void setList(ListWnd *list);
  ListWnd *getList();
 
  void setNumeric(int n) { numeric=n; }

private:
  int width;
  int index;
  int numeric;
  int dynamic;
  ListWnd *list;
  int align;
};

//class SelItemList;



class ListWnd : public ScrlBkgWnd 
{
friend class ListColumn;
friend class SelItemList;
public:
	ListWnd();

	virtual ~ListWnd();
	virtual int onInit();
	virtual int onPostOnInit();
	virtual int onPaint(Canvas *canvas);
	virtual int onResize();
	virtual int onLeftButtonDown(int x, int y);
	virtual int onLeftButtonUp(int x, int y);
	virtual int onRightButtonDown(int x, int y);
	virtual int onRightButtonUp(int x, int y);
	virtual int onMouseMove(int x, int y);
	virtual int onLeftButtonDblClk(int x, int y);
	virtual int onChar(unsigned int c);
	virtual int onKeyDown(int keyCode);
	virtual int onContextMenu (int x, int y);
	virtual int wantAutoContextMenu();
	virtual int onMouseWheelUp(int click, int lines);
	virtual int onMouseWheelDown(int click, int lines);
	virtual int wantAutoDeselect() { return wantautodeselect; }
	virtual void setWantAutoDeselect(int want) { wantautodeselect = want; }

	void onSetVisible(int show);

	void setAutoSort(bool dosort);
	void setOwnerDraw(bool doownerdraw);

	virtual void timerCallback(int id);

	void next(int wantcb=1);
	void selectCurrent();
	void selectFirstEntry(int wantcb=1);
	void previous(int wantcb=1);
	void pagedown(int wantcb=1);
	void pageup(int wantcb=1);
	void home(int wantcb=1);
	void end(int wantcb=1);
	void setItemCount(int c);
	void reset();
	void setShowColumnsHeaders(int show);
	int addColumn(const wchar_t *name, int width, int numeric=0, int align=COL_LEFTALIGN);	// adds to end
	ListColumn *getColumn(int n);
	int getNumColumns();
	int getColumnWidth(int col);
	bool setRedraw(bool redraw);	// returns prev state
	bool getRedraw();
	void setMinimumSize(int size);
	virtual int addItem(const wchar_t *label, LPARAM lParam);
	virtual int insertItem(int pos, const wchar_t *label, LPARAM lParam);
	virtual int getLastAddedItemPos();
	virtual void setSubItem(int pos, int subpos, const wchar_t *txt);
	virtual void deleteAllItems();
	virtual int deleteByPos(int pos);
	int getNumItems(void);

	virtual int getItemLabel(int pos, int subpos, wchar_t *text, int textmax);
	virtual void setItemLabel(int pos, const wchar_t *text);
	virtual LPARAM getItemData(int pos);
	virtual int getItemRect(int pos, RECT *r);  
	virtual int getItemSelected(int pos);	// returns 1 if selected  
	virtual int getItemFocused(int pos);	// returns 1 if focused  
	virtual int getItemFocused();         // returns focused item  
	void setItemFocused(int pos, int ensure_visible=TRUE);
	void ensureItemVisible(int pos);
	void invalidateColumns();
	virtual int scrollAbsolute(int x);
	virtual int scrollRelative(int x);
	virtual void scrollLeft(int lines=1);
	virtual void scrollRight(int lines=1);
	virtual void scrollUp(int lines=1);
	virtual void scrollDown(int lines=1);
	virtual const wchar_t *getSubitemText(int pos, int subpos);


	int getFirstItemSelected();


	int getNextItemSelected(int lastpos);	// next item AFTER given pos


	virtual int selectAll(int cb=1);	// force all items selected

	virtual int deselectAll(int cb=1); // force all items to be deselected

	virtual int invertSelection(int cb=1);	// invert all selections

	virtual int hitTest(POINT pos, int drag=0);

	/**
	Method

	@see 
	@ret 
		*/
	virtual int hitTest(int x, int y, int drag=0);

	/**
	Method

	@see 
	@ret 
		*/
	virtual int invalidateItem(int pos);
	virtual int locateData(LPARAM data);

	// -1 if we've never been drawn yet

	/**
	Method

	@see 
	@ret 
		*/
	int getFirstItemVisible() const { return firstItemVisible; }

	/**
	Method

	@see 
	@ret 
		*/
	int getLastItemVisible() const { return lastItemVisible; }

	virtual int setFontSize(int size);

	virtual int getFontSize();
	virtual void jumpToNext(wchar_t c);
	int wantFocus() { return 1; }
	void scrollToItem(int pos);
	virtual void resort();
	int getSortDirection();

	/**
	Method

	@see 
	@ret 
		*/
	int getSortColumn();

	void setSortColumn(int col);

	void setSortDirection(int dir);

	int findItemByParam(LPARAM param);

	void setItemParam(int pos, LPARAM param);

	int getItemCount() { return getNumItems(); }

	void setSelectionStart(int pos, int wantcb=1);

	/**
	Method

	@see 
	@ret 
		*/
	virtual void setSelectionEnd(int pos);

	void setSelected(int pos, int selected, int cb=1);
	void toggleSelection(int pos, int setfocus=TRUE, int cb=1);
	virtual int getHeaderHeight();

	// this sort function just provides string/numeric comparison
	// if you need more types, just override and provide your own
  
  virtual int sortCompareItem(listItem *p1, listItem *p2);

	int getPreventMultipleSelection() {	return preventMultipleSelection; }
	int setPreventMultipleSelection(int val) {	return preventMultipleSelection = val; }
	void moveItem(int from, int to);
	virtual int onAcceleratorEvent(const wchar_t *name);

	// override this to turn the LPARAM into a text
	virtual const wchar_t *convertlParam(LPARAM lParam) { return NULL; }
	virtual void convertlParamColumn(int col, int pos, LPARAM param, wchar_t *str, int maxlen) { };

protected:
	/*static */void CreateXMLParameters(int master_handle);

	// return 1 if you override this
  
	virtual int ownerDraw(Canvas *canvas, int pos, RECT *r, LPARAM lParam, int selected, int focused) { return 0; };
	virtual void onPreItemDraw(Canvas *canvas, int pos, RECT *r, LPARAM lParam, int selected, int focused) { }
	virtual void onPostItemDraw(Canvas *canvas, int pos, RECT *r, LPARAM lParam, int selected, int focused) { };
	virtual ARGB32 getTextColor(LPARAM lParam);
	int getTextAntialias(LPARAM lParam) { return antialias; }
	virtual int getTextBold(LPARAM lParam) { return 0; }
	virtual int getTextItalic(LPARAM lParam) { return 0; }
	virtual ARGB32 getSelBgColor(LPARAM lParam);
	virtual ARGB32 getSelFgColor(LPARAM lParam);
	virtual ARGB32 getBgColor();
	virtual ARGB32 getFocusColor(LPARAM lParam);
	virtual ARGB32 getFocusRectColor(LPARAM lParam);
	virtual int needFocusRect(LPARAM lParam) { return 0; }
	virtual ARGB32 getColumnSepColor();
	virtual int wantColSepOnItems();
	virtual int getXShift();

public:
	int insertColumn(ListColumn *col, int pos=-1, int alignment=COL_LEFTALIGN);// -1 is add to end
//  void deleteColumn(int pos);
	void deleteAllColumns();

	void setHoverSelect(int a) { hoverselect = a; }
	int getHoverSelect() { return hoverselect; }

	void setSelectOnUpDown(int i) { selectonupdown = i; }
	int getSelectOnUpDown() { return selectonupdown; }
	virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);

	/**
	Method
		Will only work with simple text lists, be forwarned!!!

	@see 
	@ret 
		*/
	int getItemHeight();
	void setItemHeight(int height, bool forceInvalidate = true);

	int		getIconWidth();
	void	setIconWidth(int width);
	int		getIconHeight();
	void	setIconHeight(int height);

protected:

	virtual int getColumnsHeight();
	virtual int getColumnsWidth();
	virtual int getContentsWidth();
	virtual int getContentsHeight();

	virtual void drawBackground(Canvas *canvas);

	void drawColumnHeaders(Canvas *c);

	void drawItems(Canvas *canvas);

	void updateScrollX();

	void updateScrollY();
	int doJumpToNext(wchar_t c, bool fromTop);
    int fullyVisible(int pos);

  virtual int onBeginDrag(int iItem);
  

  virtual int dragOver(int x, int y, ifc_window *sourceWnd);
  virtual void onSelectAll();	// hit Control-A

  virtual void onDelete();		// hit 'delete'

  virtual void onItemDelete(LPARAM lparam) {}
  
  virtual void onDoubleClick(int itemnum);	// double-click on an item
  // this is called with the selected item#
  
  virtual void onLeftClick(int itemnum);	// left-click
  // the second time you click on an already-focused item
  
  virtual void onSecondLeftClick(int itemnum);
  // this is called once for the item under cursor on click
  
  virtual int onRightClick(int itemnum);	// right-click on item

  virtual int onIconLeftClick(int itemnum, int x, int y); // Returns 1 if we should not invoke onLeftClick()

  // override this to be notified of item selections & deselections
  
  virtual void onItemSelection(int itemnum, int selected);
  
  virtual int onColumnDblClick(int col, int x, int y) { return 0; }
 
  virtual int onColumnLabelClick(int col, int x, int y);

  void selectRect(int x1, int y1, int x2, int y2);
  
  void drawRect(int x1, int y1, int x2, int y2);

  // interface to Freelist

	listItem *createListItem();
	void deleteListItem(listItem *item);
	ListColumn *enumListColumn(int pos);

	int getColumnPosByName(const wchar_t *name);

	int delColumnByPos(int pos);
public: // Martin> dunno why these were protected...
	void setShowIcons(int icons);
	int getShowIcons(); // Maybe useful or not
	SkinBitmap *getItemIcon(int item);
	void setItemIcon(int pos, const wchar_t *bitmapid);

protected:
	int item_invalidate_border;
	bool showColumnsHeaders;
	void recalcHeaders();
	void itemSelection(int itemnum, int selected);

private:
	int doAddItem(const wchar_t *label, LPARAM lParam, int pos);


	int hitTestColumns(POINT p, int *origin=NULL);
	int hitTestColumnClient(int x);
	int hitTestColumnsLabel(POINT p);
	void drawXorLine(int x);
	void calcNewColWidth(int col, int x);
	void calcBounds();
	void onDragTimer();
	void notifySelChanged(int item=-1, int sel=-1);
	virtual int wantResizeCols() { return 1; }

	int autosort, ownerdraw;
	int textsize;
	int itemHeight;
	int iconWidth; // If it's still negative use itemHeight instead -- better user getIconWidth()
	int iconHeight;
	bool metrics_ok;
	bool redraw;
	int columnsHeight;
	int dragtimeron;

	int antialias;

	PtrList<ListColumn> columnsList;
	PtrListQuickSorted<listItem,CompareListItem> itemList;

	int firstItemVisible;
	int lastItemVisible;

	listItem *lastItemFocused;
	int lastItemFocusedPos;

	listItem *lastAddedItem;
	SelItemList selItemList;

	int dragskip;
	int dragskipcount;
	int selectionStart;
	int colresize;
	POINT colresizept;
	bool resizing_col;
	int colresizeo;

	bool processbup;
	bool bdown;
	bool nodrag;
	int bdownx, bdowny;
	bool firstComplete, lastComplete;

	int rectselecting;
	POINT selectStart;
	POINT selectLast;

	int sortdir, sortcol, lastcolsort;

	int preventMultipleSelection;
	
	Freelist<listItem> listItem_freelist;
	int wantautodeselect;

	int hoverselect;
	int selectonupdown;
	PtrList<ifc_window> tempselectnotifies;
	StringW accessibleItemName;
	int showicons;

private:
	/* XML Parameters */
	static XMLParamPair params[];
	int xuihandle;
	bool hasUserBg;
		
    enum 
		{
      LIST_ANTIALIAS = 0,
	  LIST_BACKGROUND,
	  LIST_TILE,
	  LIST_NOCOLHEADER,
    };
		protected:
			int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
};

#endif
