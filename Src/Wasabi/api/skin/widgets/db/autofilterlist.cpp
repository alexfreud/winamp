#include <precomp.h>
#include "autofilterlist.h"
#include <api/db/subqueryserver.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/script/objects/c_script/h_button.h>
#include <api/wnd/popup.h>
#include <bfc/file/filename.h>

#define TIMER_SCANNERDEL 0x6891

class ButtHooker : public H_Button {
public:
  ButtHooker(AutoFilterList *hangout, ScriptObject *butt) : afl(hangout), H_Button(butt) { }
  void hook_onLeftClick() {
    afl->doFieldPopup();
  }

private:
  AutoFilterList *afl;
};

FilterListItem::FilterListItem(void *_data, int _datalen, int _datatype) {
  data_len = _datalen;
  data_type = _datatype;
  data.setSize(data_len+1);
  MEMCPY(data.getMemory(), _data, data_len);
}

AutoFilterList::AutoFilterList() {
  showColumnsHeaders = FALSE;
  local_scanner = NULL;
  order = -1;
  viscount = 0;
  grab_playstrings = 0;
  linked = 1;
  hooker = NULL;
  needrestart = 0;
  querydirection = SetQuery::FORWARD;
  last_populate_flag = -1;
  setContent("wasabi.buttonbar.stack");
  fn = NULL;
  data_type = NULL;
}

AutoFilterList::~AutoFilterList() {
  delete local_scanner;
  delete hooker;
}

int AutoFilterList::onInit() {
  AUTOFILTERLIST_PARENT::onInit();
  //scannerserver_newScanner();

  return 1;
}

void AutoFilterList::scannerserver_onNewScanner(SharedDbScannerI *scanner) {
  AUTOFILTERLIST_DBPARENTSRV::scannerserver_onNewScanner(scanner);
  delete local_scanner;
  local_scanner = new dbScanner(scannerserver_getTable());
  local_scanner->setQuery(sqs_getQuery());
}

void AutoFilterList::mqc_onNewMultiQuery(SubQueryServer *modifier, int flag) {
  AUTOFILTERLIST_DBPARENTCLIENT::mqc_onNewMultiQuery(modifier);
  if (!isInited()) return;
  if (modifier == this && !needrestart) return;
  if (local_scanner == NULL) return;
  int id = sqs_getCooperativeId();
  int need_reset = (modifier == NULL || needrestart);
  if (!need_reset) switch (flag) {//glag!
    default:
    case SetQuery::FORWARD:
      need_reset = (id >= modifier->sqs_getCooperativeId());
    break;
    case SetQuery::REVERSE:
      need_reset = (id <= modifier->sqs_getCooperativeId());
    break;
    case SetQuery::GLOBAL:
      need_reset = (id != modifier->sqs_getCooperativeId());
    break;
  }

  if (need_reset) {
    needrestart = 0;

    last_populate_flag = flag;

    int optimized = sqs_getMultiQueryServer() && sqs_getMultiQueryServer()->mqs_isQueryEmpty();

    uniques.deleteAll();
    deleteAllItems();

    const char *allstr = getAllString();
    FilterListItem *fli = new FilterListItem((void *)allstr, STRLEN(allstr)+1, MDT_STRINGZ);
    insertItem(0, NULL, reinterpret_cast<LPARAM>(fli));
    setSelected(0, 1);

    SharedDbScannerI *shs = scannerserver_getScanner();
    if (shs != NULL) {
      dbSvcScanner *sc = shs->getScanner();
      if (sc != NULL) sc->cancelQuery();
    }
    dbSvcScanner *lc= local_scanner->getScanner();
    if (lc != NULL) lc->cancelQuery();
    
    if (optimized) {
      grab_playstrings = 0;
      setRedraw(0);
      dbScanner *unique_scanner = new dbScanner(sqs_getTable(), field);
      while (!unique_scanner->eof()) {
        char data[4096]="";
        unique_scanner->getData("Value", data, 4095, MDT_STRINGZ);
        data[4095]=0;
        insertData(data, STRLEN(data)+1, MDT_STRINGZ);
        unique_scanner->next();
      }
      setRedraw(1);
      delete unique_scanner;
    } else {
      grab_playstrings = 1;
      lc->cancelQuery();
    }

    return;
  }
}

void AutoFilterList::mqc_onCompleteMultiQuery() {
  grab_playstrings = 0;
}

void AutoFilterList::mqc_onAddPlaystring(const char *playstring, int nitems, int thispos) {
  AUTOFILTERLIST_DBPARENTCLIENT::mqc_onAddPlaystring(playstring, nitems, thispos);
  if (grab_playstrings)
    filterEntry(playstring, local_scanner, field);
}

void AutoFilterList::setMetadataField(const char *_field) {
  if (!field.isempty() && STRCASEEQL(field, _field)) return;
  field = _field;
  delColumnByPos(0);
  addColumn(field, 100);
  data_type = api->metadb_getMetaDataType(field);
  sqs_setTable(api->metadb_getMetaDataTable(field));
  if (isPostOnInit())
    setLabelName();
  sqs_setQuery("");
  needrestart = 1;
  scannerserver_newScanner();
}

int AutoFilterList::onResize() {
  AUTOFILTERLIST_PARENT::onResize();
  RECT r = clientRect();
  ListColumn *c = enumListColumn(0);
  if (!c) return 1;
  c->setWidth(r.right-r.left-4);
  recalcHeaders();

  return 1;
}

void AutoFilterList::getClientRect(RECT *r) {
  AUTOFILTERLIST_PARENT::getClientRect(r);
  api_window *rw = getContentRootWnd();
  if (rw) r->top += rw->getPreferences(SUGGESTED_H);
//  else r->top += 16;
}

void AutoFilterList::rootwndholder_getRect(RECT *r) {
  getClientRect(r);
  r->bottom = r->top;
  api_window *rw = getContentRootWnd();
  if (rw) r->top -= rw->getPreferences(SUGGESTED_H);
//  else r->top += 16;
}

void AutoFilterList::onNewContent() {
  setLabelName();
  // hook the clicks
  delete hooker;
  ScriptObject *mousetrap = findScriptObject("mousetrap");
  hooker = new ButtHooker(this, mousetrap);
}

int AutoFilterList::ownerDraw(Canvas *canvas, int pos, RECT *r, LPARAM lParam, int isselected, int isfocused) {
  COLORREF bgcolor = isfocused ? getFocusColor(lParam) : getSelBgColor(lParam);
  COLORREF fgcolor = getTextColor(lParam);

  RECT box;
  canvas->getClipBox(&box);

  if (!getBgBitmap()) {
    RECT r2 = *r;
    r2.left = box.left;
    new RegionI reg(&r2);
    canvas->selectClipRgn(&reg);
    canvas->fillRect(r, getBgColor());
  }

  canvas->setTextColor(fgcolor);

  if (isselected) {
    RECT mr = *r;
    canvas->fillRect(&mr, bgcolor);
  }

  if (isfocused) 
    canvas->drawRect(r, 0, getFocusColor(lParam));

  canvas->pushTextSize(getFontSize());

  int x = 1+r->left;
  for (int i = 0; i < getNumColumns(); i++) {
    RECT ir;
    ir.left = x;
    ir.right = x + getColumnWidth(i);
    ir.top = r->top;
    ir.bottom = r->bottom;
    if (ir.right >= box.left && ir.bottom >= box.top && ir.left <= box.right && ir.top <= box.bottom) {
      FilterListItem *fli = reinterpret_cast<FilterListItem *>(lParam);
      api->metadb_renderData(canvas, ir, (void *)fli->getData(), fli->getDatatype(), 0);
    }
    x = ir.right;
  }
  canvas->popTextSize();
  return 1;
}

void AutoFilterList::filterEntry(const char *playstring, dbScanner *scanner, const char *field) {
  dbSvcScanner *sp = scanner->getScanner();
  if (sp != NULL) sp->push();
  scanner->setIndexName(MT_PLAYSTRING);
  scanner->setIndexValue(playstring);
  scanner->first();

  if (scanner->eof()) return;

  char data[4096]="";
  scanner->getData((char *)field, data, 4095, data_type);
  data[4095]=0;

  sp->pop();

  switch (data_type) {
    case MDT_INT: 
    case MDT_TIME:
    case MDT_BOOLEAN:
    case MDT_TIMESTAMP: 
      filterInt(*(int *)data);
      break;
    case MDT_STRINGZ:
      filterString((const char *)data);
      break;
  }
}

void AutoFilterList::filterInt(int data) {
  int pos=0;
  if (uniques.findItem((const char *)&data, &pos)) return;
  insertData(&data, 4, data_type);
}

void AutoFilterList::insertData(void *data, int len, int type) {
  int pos=0;
  FilterListItem *fli = new FilterListItem(data, len, type);
  uniques.addItem(fli);
  pos = uniques.searchItem(fli);
  insertItem(pos+1/*+1 for item ALL at the top*/, NULL, reinterpret_cast<LPARAM>(fli));
}

void AutoFilterList::filterString(const char *data) {
  if (!data || !*data) return;
  int pos=0;
  if (uniques.findItem(data, &pos)) return;
  insertData((void *)data, STRLEN(data)+1, data_type);
}

void AutoFilterList::onLeftClick(int itemnum) {

  if (itemnum == 0 && last_populate_flag != lastflag) {
    needrestart = 1;
  }
  String query;
  if (itemnum > 0) {
    for (int i=0;i<getNumItems();i++) {
      if (getItemSelected(i)) {
        if (!query.isempty())
          query += " || ";
        query += field;
        query += " == ";
        FilterListItem *fli = reinterpret_cast<FilterListItem *>(getItemData(i));
        switch (fli->getDatatype()) {
          case MDT_INT: 
          case MDT_TIME:
          case MDT_BOOLEAN:
          case MDT_TIMESTAMP: 
            query += StringPrintf("%d", *(int *)fli->getData());
            break;
          case MDT_STRINGZ:
            query += "\"";
            query += fli->getData();
            query += "\"";
            break;
        }
      }
    }
  }
  sqs_setQuery(query, querydirection);
  if (local_scanner != NULL) local_scanner->setQuery(query);
}

void AutoFilterList::onDoubleClick(int itemnum) {
  int sav = querydirection;
  querydirection = SetQuery::GLOBAL;
  onLeftClick(itemnum);
  querydirection = sav;
}

void AutoFilterList::sqs_onAttachServer(MultiQueryServer *s) {
  s->mqs_registerClient(this);
}

void AutoFilterList::sqs_onDetachServer(MultiQueryServer *s) {
  s->mqs_unregisterClient(this);
}

void AutoFilterList::sqs_reset() {
  postDeferredCallback(7873, 3245);
}

int AutoFilterList::onDeferredCallback(intptr_t p1, intptr_t p2) {
  if (p1 == 7873 && p2 == 3245) {
    deselectAll();
    setItemFocused(0);
    setSelected(0, TRUE);
    needrestart=1; last_populate_flag = -1;
    onLeftClick(0);
    ensureItemVisible(0);
    return 1;
  }
  return AUTOFILTERLIST_PARENT::onDeferredCallback(p1, p2);
}

void AutoFilterList::setLabelName() {
  ScriptObject *tx = findScriptObject("buttonbar.text");
  if (tx == NULL) return;
  C_Text(tx).setText(field);
}

void AutoFilterList::setQueryDirection(int glag) {
  querydirection = glag;
}

void AutoFilterList::doFieldPopup() {
  PopupMenu popup;
  dbSvcScanner *scanner = api->metadb_newScanner(sqs_getTable());
  if (scanner == NULL) return;
  int n = scanner->getNumCols();
  PtrListQuickSorted<String, StringComparator> fields;
  for (int i = 0; i < n; i++) {
    dbSvcColInfo *info = scanner->enumCol(i);
    if (!info->uniques_indexed) continue;
    fields.addItem(new String(info->name));
  }
  fields.sort();
  foreach(fields)
    const char *name = fields.getfor()->getValue();
    int checked = STRCASEEQLSAFE(name, field);
    popup.addCommand(name, foreach_index, checked);
  endfor
  RECT cr = clientRect();
  clientToScreen((int *)&cr.left, (int *)&cr.top);
  int r = popup.popAtXY(cr.left, cr.top);
  if (r >= 0) {
    const char *col = fields.enumItem(r)->getValue();
    dbSvcColInfo *info = scanner->getColByName(col);
    setMetadataField(info->name);
  }
  fields.deleteAll();
  api->metadb_deleteScanner(scanner);
}

void AutoFilterList::onVScrollToggle(BOOL set) {
  AUTOFILTERLIST_PARENT::onVScrollToggle(set);
  if (getContentRootWnd() && isPostOnInit())
    onResize();
}

int AutoFilterList::onBeginDrag(int iItem) {
  String query;
  FilterListItem *fli = reinterpret_cast<FilterListItem *>(getItemData(iItem));
  if (fli == NULL) return 0;	// BU added in response to talkback data we'll see if it helps
  String val = (fli->getDatatype() == MDT_STRINGZ) ? fli->getData() : StringPrintf(*(int *)fli->getData()).getValue();
  fn = new FilenameI(StringPrintf("query://%s;\"%s\" == \"%s\"", StringPrintf(scannerserver_getTable()).getValue(), field.getValue(), val.getValue()));
  addDragItem(Filename::dragitem_getDatatype(), static_cast<Filename*>(fn));
  handleDrag();
  return 1;
}

int AutoFilterList::dragComplete(int success) {
  delete fn; fn = NULL;
  return 1;
}

