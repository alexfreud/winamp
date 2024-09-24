#include "precomp.h"

#include "itemlistwnd.h"
#include "../../common/metatags.h"

#include "../../studio/api.h"

#include "../skinclr.h"
#include "../../common/filename.h"

#include "../../common/dragitemi.h"
#include "../../common/contextmenu.h"

#define NIFTY_OUTLINE

static SkinColor current("wasabi.itemlist.outline.current");
static SkinColor selborder("wasabi.itemlist.selborder");
static SkinColor focus("wasabi.itemlist.outline.focus");

ItemListColumn_MetaTag::ItemListColumn_MetaTag(const char *newtag, int centered, const char *label)
 : ItemListColumn() {
  if (label == NULL) label = newtag;
  setLabel(label);
  tag = newtag;
  center = centered;
  datatype = api->metadb_getMetaDataType(tag);
}

ItemListColumn_Callback::ItemListColumn_Callback(ItemListWnd *_parent, LPARAM _lparam, const char *name)
: ItemListColumn(name)
{
  parent = _parent;
  lparam = _lparam;
}

void ItemListColumn_Callback::render(int pos, const char *playstring, Canvas &c, RECT &r) {
  parent->userRender(pos, playstring, c, r, lparam);
}

void ItemListColumn_Callback::columnToText(int pos, const char *playstring, char *str, int maxlen) {
  parent->userColumnToText(pos, playstring, lparam, str, maxlen);
}

void ItemListColumn_MetaTag::render(int pos, const char *playstring, Canvas &canvas, RECT &r) {
  char buf[4096]="";
  if (api->metadb_getMetaData(playstring, tag, buf, 4096) <= 1) return;
  api->metadb_renderData(&canvas, r, buf, datatype, center);
}

void ItemListColumn_MetaTag::columnToText(int pos, const char *playstring, char *str, int maxlen) {
  api->metadb_getMetaData(playstring, tag, str, maxlen);
}

const char *ItemListColumn_MetaTag::getTag() {
  return tag;
}

void ItemListColumn_Numbered::render(int pos, const char *playstring, Canvas &c, RECT &r) {
  StringPrintf buf("%d.", pos+1);
  c.textOutEllipsed(r.left, r.top, r.right - r.left, r.bottom-r.top, buf);
}

void ItemListColumn_Numbered::columnToText(int pos, const char *playstring, char *str, int maxlen) {
  StringPrintf buf("%d", pos+1);
  STRNCPY(str, buf, maxlen);
}

ItemListWnd::ItemListWnd() {
  keep = NULL;
  item_invalidate_border++;
}

ItemListWnd::~ItemListWnd() {
  api->syscb_deregisterCallback(static_cast<MetaCallbackI *>(this));
}

int ItemListWnd::onInit() {

  ITEMLISTWND_PARENT::onInit();

  api->syscb_registerCallback(static_cast<MetaCallbackI *>(this));

  return 1;
}

int ItemListWnd::insertColumn(ItemListColumn *column, int width, int pos) {
  column->setWidth(width);
  return ITEMLISTWND_PARENT::insertColumn(column, pos);
}

int ItemListWnd::onBeginDrag(int iItem) {

  if (!wantAutoDrag()) return ITEMLISTWND_PARENT::onBeginDrag(iItem);

  // add all the entries
  int n, i, c = 0;
  n = getNumItems();
  if (n == 0) return 0;			// empty list

  ASSERT(keep == NULL);
  keep = new PtrList<FilenameNC>();

  // count up the items
  for (i = 0; i < n; i++) {
    if (getItemSelected(i)) {	// expose all the pointers we can
      LPARAM lparam = getItemData(i);
      if (!addMoreDragTypes(i)) {
        FilenameNC *fn = keep->addItem(new FilenameNC(convertlParam(lparam)));
        addDragItem(DD_FILENAME, static_cast<Filename*>(fn));
      }
      c++;
    }
  }

  if (keep->getNumItems() == 0 || c <= 0) {
    delete keep; keep = NULL;
    return 0;
  }

  handleDrag();

  return 1;
}

int ItemListWnd::dragComplete(int success) {
  ASSERT(keep != NULL);

  keep->deleteAll();
  delete keep; keep = NULL;

  return 1;
}

int ItemListWnd::ownerDraw(Canvas *canvas, int pos, RECT *r, LPARAM lParam, int isselected, int isfocused) {
  const char *playstring = convertlParam(lParam);
  if (playstring == NULL) return 0;
  COLORREF bgcolor = isfocused ? getFocusColor(lParam) : getSelBgColor(lParam);
  COLORREF fgcolor = getTextColor(lParam);
  int iscur = getSelected(lParam);

  RECT box;
  canvas->getClipBox(&box);

  if (!getBgBitmap()) {
    RECT r2 = *r;
    r2.left = box.left;
    RegionI *reg = new RegionI(&r2);
    canvas->selectClipRgn(reg);
    delete reg;
    canvas->fillRect(r, getBgColor());
  }

  canvas->setTextColor(fgcolor);

//  ASSERT(cols.getNumItems() == getNumColumns());

  if (isselected) {
    RECT mr = *r;
    canvas->fillRect(&mr, bgcolor);
/*au gratin potatoes/broccoli
chicken^2
salad, croutons, cheese, ranch
fries */
#ifdef NIFTY_OUTLINE
    int prevsel = getItemSelected(pos-1);
    int nextsel = getItemSelected(pos+1);
    mr.bottom--;
    canvas->pushPen(selborder);
    canvas->lineDraw(mr.left, mr.top, mr.left, mr.bottom);
    canvas->lineDraw(mr.right-1, mr.top, mr.right-1, mr.bottom);
    if (!prevsel) canvas->lineDraw(mr.left, mr.top, mr.right, mr.top);
    if (!nextsel) canvas->lineDraw(mr.left, mr.bottom, mr.right, mr.bottom);
    canvas->popPen();
#endif
  }

  if (isfocused || iscur) {
    int pentype = isfocused ? FALSE : TRUE;
    int boxcolor = iscur ? current : focus;
    canvas->drawRect(r, pentype, boxcolor);
  } 

  canvas->pushTextSize(getFontSize());

  int x = 1+r->left;
  for (int i = 0; i < getNumColumns(); i++) {
    ItemListColumn *col = (ItemListColumn *)enumListColumn(i);
    RECT ir;
    ir.left = x;
    ir.right = x + getColumnWidth(i);
    ir.top = r->top;
    ir.bottom = r->bottom;
    if (ir.right >= box.left && ir.bottom >= box.top && ir.left <= box.right && ir.top <= box.bottom) {
      col->render(pos, playstring, *canvas, ir);
    }
    x = ir.right;
  }
  canvas->popTextSize();
  return 1;
}

void ItemListWnd::convertlParamColumn(int col, int pos, LPARAM param, char *str, int maxlen) {
  ASSERT(str != NULL);
  ItemListColumn *cl = (ItemListColumn *)enumListColumn(col);
  const char *playstring = convertlParam(param);
  cl->columnToText(pos, playstring, str, maxlen);
}


int ItemListWnd::onContextMenu(int x, int y) {
  PtrList<FilenameNC> filenames;
  DragItemI *dragitem = NULL;

  if ((dragitem = itemlistwnd_getDragItem(x, y)) == NULL) {
    DragItemT<Filename> *di = new DragItemT<Filename>;
    int n = getNumItems();
    for (int i = 0; i < n; i++) {
      if (getItemSelected(i)) {
        LPARAM lparam = getItemData(i);
        const char *playstring = convertlParam(lparam);
        di->addDatum(filenames.addItem(new FilenameNC(playstring)));
      }
    }
    dragitem = di;
  }

  ContextMenu cm(this, filenames.getNumItems() ? dragitem : NULL, false);

  PtrList<DragItemI> drags;
  for (int i = 0; ; i++) {
    DragItemI *aitem = itemlistwnd_getSecondDragItem(i);
    if (aitem == NULL) break;
    drags.addItem(aitem);
    if (cm.getNumCommands()) cm.addSeparator();
    cm.addDragItem(aitem);
  }
  itemlistwnd_addCustomContextMenuCommands(&cm);
  int r = cm.popAtMouse();
  if (r < -10) itemlistwnd_contextMenuResult(r);
  drags.deleteAll();

  filenames.deleteAll();

  delete dragitem;

  return 1;
}

#if 0
int ItemListWnd::onRightClick(int itemnum, int _x, int _y) {
  // figure out which column they clicked on
  int x, l = 0, r = 0;
  for (int i = 0; i < cols.getNumItems(); i++) {
    l = r;
    r += getColumnWidth(i);
    if (_x >= l && y <= r) break;
  }
  if (i >= cols.getNumItems()) return 0;
  PlayItem *item = convertlParam(itemnum);
  if (!allowEdition(item, cols[i]->getTag())) return 0;

  return 0;
}
#endif

void ItemListWnd::metacb_onItemChange(const char *playstring, const char *tag) {
  int n = getNumItems();
  for (int i = 0; i < n; i++) {
    const char *ps = convertlParam(getItemData(i));
    if (ps != NULL && STREQL(ps, playstring)) {
      onItemChange(i, ps);
      invalidateItem(i);
    }
  }
}

void ItemListWnd::metacb_onItemDel(const char *playstring) {
  int n = getNumItems();
  for (int i = 0; i < n; i++) {
    const char *ps = convertlParam(getItemData(i));
    if (ps != NULL && STREQL(ps, playstring)) {
      onItemDel(i, ps);
      invalidateItem(i);
    }
  }
}
