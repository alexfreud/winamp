#include <precomp.h>
#include "dropdownlist.h"
#include <api/wnd/wndclass/listwnd.h>
#include <api/script/objects/guiobject.h>
#include <api/script/scriptguid.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/wnd/popexitcb.h>
#include <api/wnd/notifmsg.h>
#include <api/service/svc_enum.h>
#include <bfc/parse/paramparser.h>
#include <api/service/svcs/svc_textfeed.h>

#define DDL_CLOSELISTCB 0x0721

XMLParamPair DropDownList::params[] =
{
	{DROPDOWNLIST_SETFEED, L"FEED"},
	{DROPDOWNLIST_SETITEMS, L"ITEMS"},
	{DROPDOWNLIST_LISTHEIGHT, L"LISTHEIGHT"},
	{DROPDOWNLIST_MAXITEMS, L"MAXITEMS"},
	{DROPDOWNLIST_SELECT, L"SELECT"},
	{DROPDOWNLIST_SETLISTANTIALIAS, L"ANTIALIAS"},
};
// -----------------------------------------------------------------------
DropDownList::DropDownList() {
  selected = -1;
  //abstract_setAllowDeferredContent(1);
  clicks_button = NULL;
  clicks_text = NULL;
  list_key = NULL;
  height = 128;
  maxitems = 0;
  noitemtext = L"";
  list_group = NULL;
  trap_click = 0;
  disable_cfg_event = 0;

  GuiObjectWnd::getScriptObject()->vcpu_setInterface(dropDownListGuid, (void *)static_cast<DropDownList *>(this));
  GuiObjectWnd::getScriptObject()->vcpu_setClassName(L"DropDownList"); // this is the script class name
  GuiObjectWnd::getScriptObject()->vcpu_setController(dropDownListController);

  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);

	registerAcceleratorSection(L"popup", 1);
}

void DropDownList::CreateXMLParameters(int master_handle)
{
	//DROPDOWNLIST_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}
// -----------------------------------------------------------------------
DropDownList::~DropDownList() { 
  doCloseList(0);
  delete clicks_text;
  delete clicks_button;
  delete list_key;
}

// -----------------------------------------------------------------------
int DropDownList::onAcceleratorEvent(const wchar_t *name) {
  int r = DROPDOWNLIST_PARENT::onAcceleratorEvent(name);
  if (WCSCASEEQLSAFE(name, L"exit")) {
    escapeCallback();
    return 1;
  }
  return r;
}

// -----------------------------------------------------------------------
int DropDownList::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return DROPDOWNLIST_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case DROPDOWNLIST_SETITEMS:
      setItems(value);
      break;
    case DROPDOWNLIST_SETFEED:
      setFeed(value);
      break;
    case DROPDOWNLIST_SELECT:
      selectItem(findItem(value));
      break;
    case DROPDOWNLIST_LISTHEIGHT:
      setListHeight(WTOI(value));
      break;
    case DROPDOWNLIST_MAXITEMS:
      setMaxItems(WTOI(value));
      break;
  	case DROPDOWNLIST_SETLISTANTIALIAS: 
	  listAntialias = WTOI(value);
	  break;
    default:
      return 0;
  }
  return 1;
}

// -----------------------------------------------------------------------
int DropDownList::onInit() 
{
  int rt = DROPDOWNLIST_PARENT::onInit();
  abstract_setContent(dropdownlist_getMainGroupId());
  return rt;
}

// -----------------------------------------------------------------------
void DropDownList::abstract_onNewContent() {
  DROPDOWNLIST_PARENT::abstract_onNewContent();
  trapControls();
  updateTextInControl(getSelectedText());
}  

#ifdef WASABI_COMPILE_CONFIG
// -----------------------------------------------------------------------
int DropDownList::onReloadConfig() {
  int r = DROPDOWNLIST_PARENT::onReloadConfig();
  disable_cfg_event = 1;
  updateTextFromConfig(); // triggers onSelect
  disable_cfg_event = 0;
  return r;
}

// -----------------------------------------------------------------------
void DropDownList::updateTextFromConfig() { 
  const wchar_t *val = getGuiObject()->guiobject_getCfgString();
  const wchar_t *old = getSelectedText();
  if (old && val && !_wcsicmp(val, old)) return;

  if (val != NULL) {
    int id = findItem(val);
    if (id != -1)
      selectItem(id);
  }
}
#endif

// -----------------------------------------------------------------------
void DropDownList::trapControls() {
  delete clicks_button;
  delete clicks_text;

  clicks_button = NULL;
  clicks_text = NULL;

  if (wantTrapText()) {
    GuiObject *textGuiObj = getGuiObject()->guiobject_findObject(dropdownlist_getTextId());
    if (textGuiObj) clicks_text = new DDLClicksCallback(*textGuiObj, this);
  }

  if (wantTrapButton()) {
    GuiObject *butGuiObj = getGuiObject()->guiobject_findObject(dropdownlist_getButtonId());
    if (butGuiObj) clicks_button = new DDLClicksCallback(*butGuiObj, this);
  }
}


// -----------------------------------------------------------------------
void DropDownList::clickCallback() {
  if (list_group != NULL)
    closeList();
  else
    openList();
}

// -----------------------------------------------------------------------
void DropDownList::escapeCallback() {
  if (isListOpen())
    closeList();
}

// -----------------------------------------------------------------------
void DropDownList::openList() {
  onPreOpenList();
  WASABI_API_WND->appdeactivation_push_disallow(this);
#ifdef WASABI_COMPILE_WNDMGR
  list_group = WASABI_API_SKIN->group_create_layout(dropdownlist_getListGroupId());
#else
  list_group = WASABI_API_SKIN->group_create(dropdownlist_getListGroupId());
#endif
  group_dep = list_group->getDependencyPtr();
  viewer_addViewItem(group_dep);
  if (list_group == NULL) return;
  list_group->setStartHidden(1);
  list_group->setParent(WASABI_API_WND->main_getRootWnd());
  trap_click = 0;
  list_group->init(this, TRUE);
  setListParams();

  // At this point, the list should be good.  Calc for max-items size
  int calc_height = 0;
  if (maxitems) {
    ifc_window *listroot = list_group->findWindowByInterface(listGuid);
    ListWnd *listwnd = static_cast<ListWnd *>(listroot->getInterface(guilistGuid));
    GuiObject *listobj = listroot->getGuiObject();
    if (listwnd) {
      int numitems = 0;
      if (maxitems == -1) {
        numitems = listwnd->getNumItems(); 
      } else {
        numitems = MIN(maxitems, listwnd->getNumItems()); 
      }
      int offset_h = 0;
      if (listobj) {
         const wchar_t *y_param = listobj->guiobject_getXmlParam(L"y");
         const wchar_t *h_param = listobj->guiobject_getXmlParam(L"h");
         const wchar_t *ry_param = listobj->guiobject_getXmlParam(L"relaty");
         const wchar_t *rh_param = listobj->guiobject_getXmlParam(L"relath");
         int h_val = (h_param)?WTOI(h_param):0;
         int y_val = (y_param)?WTOI(y_param):0;
         if (ry_param && (wcscmp(ry_param, L"1") == 0)) {
           if (y_val < 0) y_val = -y_val;
           else y_val = 0;
         }
         if (rh_param && (wcscmp(rh_param, L"1") == 0)) {
           if (h_val < 0) h_val = -h_val;
         }
         offset_h = h_val + y_val;
      }
      calc_height = (numitems * listwnd->getItemHeight()) + offset_h;
    }
  } else {
    calc_height = height;
  }

  RECT r;
  getWindowRect(&r);
  r.top = r.bottom;
  r.bottom = r.top + calc_height;
  divRatio(&r);
  list_group->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
  list_group->setVisible(1);
  WASABI_API_WND->popupexit_register(this, list_group); // this will call us back whenever someone clicks outside us
  trap_click = 1;
  listif = list_group->findWindowByInterface(listGuid);
  if (listif != NULL)
    list_key = new DDLKeyCallback(listif->getGuiObject()->guiobject_getScriptObject(), this);
  dropdownlist_onOpenList();
}

// -----------------------------------------------------------------------
void DropDownList::dropdownlist_onOpenList()
{
#ifdef _WIN32
  SetCapture(NULL); // NONPORTABLE, the goal is to cancel any capture some of our content guiobject might have so as to let the click down + slide in list transfer mouse capture
#else
#warning port me?
#endif
  setFocus();
}

// -----------------------------------------------------------------------
void DropDownList::dropdownlist_onCloseList() {
}

// -----------------------------------------------------------------------
void DropDownList::closeList() {
  if (list_group != NULL) {
    onPreCloseList();
    postDeferredCallback(DDL_CLOSELISTCB, 0);
  }
}

// -----------------------------------------------------------------------
void DropDownList::doCloseList(int cb) {
  if (cb) dropdownlist_onCloseList();
  if (list_group) {
    trap_click = 0;
    WASABI_API_WND->popupexit_deregister(this);
    WASABI_API_SKIN->group_destroy(list_group);
    list_group = NULL;
    group_dep = NULL;
    action_list = NULL;
    delete list_key;
    list_key = NULL;
    WASABI_API_WND->appdeactivation_pop_disallow(this);
  }
}

// -----------------------------------------------------------------------
void DropDownList::setListParams() {
  ASSERT(list_group != NULL);
  GuiObject *go = static_cast<GuiObject *>(list_group->getInterface(guiObjectGuid));
  if (go != NULL) {
    dropdownlist_onConfigureList(go);
  }
}

// -----------------------------------------------------------------------
void DropDownList::dropdownlist_onConfigureList(GuiObject *go) {
  XmlObject *o = NULL;
  if (go != NULL) {
    GuiObject *list = go->guiobject_findObject(dropdownlist_getListId());
    if (list != NULL) {
      action_list = list->guiobject_getRootWnd();
      o = static_cast<XmlObject *>(list->guiobject_getScriptObject()->vcpu_getInterface(xmlObjectGuid));
    }
  }
  StringW s;
  foreach(items) 
    if (foreach_index > 0)
			s += L";";
    s += items.getfor()->getText();
  endfor;
  o->setXmlParam(L"multiselect", L"0");
  o->setXmlParam(L"hoverselect", L"1");
  o->setXmlParam(L"selectonupdown", L"0");
  o->setXmlParam(L"sort", StringPrintfW(L"%d", wantAutoSort()));
  o->setXmlParam(L"items", s);
  o->setXmlParam(L"antialias", listAntialias ? L"1" : L"0");
  if (selected != -1) 
		o->setXmlParam(L"select", getSelectedText());
}

// -----------------------------------------------------------------------
int DropDownList::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
  if (WCSCASEEQLSAFE(action, L"set_selection")) {
    int p = findItem(param);
    selectItem(p);
    return p;
  }
  if (WCSCASEEQLSAFE(action, L"get_selection")) {
    if (source)
      sendAction(source, L"set_selection", getSelectedText());
  }
  return DROPDOWNLIST_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
}

// -----------------------------------------------------------------------
int DropDownList::addItem(const wchar_t *text) {
  DDLEntry *e = new DDLEntry(text);
  items.setSorted(wantAutoSort());
  items.addItem(e);
  return e->getId();
}

// -----------------------------------------------------------------------
void DropDownList::selectDefault() {
#ifdef WASABI_COMPILE_CONFIG
  onReloadConfig();
#endif
}

// -----------------------------------------------------------------------
void DropDownList::delItem(int id) {
  foreach(items)
    if (items.getfor()->getId() == id) {
      delete items.getfor();
      items.removeByPos(foreach_index);
      break;
    }
  endfor;
  if (list_group != NULL)
    setListParams();
}

// -----------------------------------------------------------------------
void DropDownList::selectItem(int id, int hover) {
  //FG> DO NOT PUT THIS TEST BACK: if (selected == id) return;
  selected = id;
  onSelect(selected, hover);
}

// -----------------------------------------------------------------------
void DropDownList::onSelect(int id, int hover) {
  updateTextInControl(getSelectedText());
  if (!disable_cfg_event && !hover) {
#ifdef WASABI_COMPILE_CONFIG
    if (selected == -1)
      getGuiObject()->guiobject_setCfgString(L"");
    else
      getGuiObject()->guiobject_setCfgString(getSelectedText());
#endif
  }

  // Let the script have the callback, too.
  DropDownListScriptController::DropDownList_onSelect(SCRIPT_CALL, GuiObjectWnd::getScriptObject(), MAKE_SCRIPT_INT(id), MAKE_SCRIPT_INT(hover));

}

// -----------------------------------------------------------------------
const wchar_t *DropDownList::getItemText(int id) {
  foreach(items)
    if (items.getfor()->getId() == id)
      return items.getfor()->getText();
  endfor;
  return NULL;
}

// -----------------------------------------------------------------------
int DropDownList::findItem(const wchar_t *text) {
  int pos=-1;
  items.findItem(text, &pos);
  if (pos < 0) return -1;
  return items[pos]->getId();
}

// -----------------------------------------------------------------------
void DropDownList::updateTextInControl(const wchar_t *txt) 
{
  GuiObject *content = getContent();
  if (content != NULL) {
    if (wantTrapText()) {
      GuiObject *text = content->guiobject_findObject(dropdownlist_getTextId());
      if (text != NULL) {
        C_Text t(*text);
        t.setText(txt);
      }
    }
  }
}

// -----------------------------------------------------------------------
void DropDownList::setNoItemText(const wchar_t *txt) 
{
  noitemtext = txt;
  if (selected == -1)
    updateTextInControl(getSelectedText());
}

// -----------------------------------------------------------------------
int DropDownList::popupexitcb_onExitPopup() {
  closeList();
  return 1;
}

// -----------------------------------------------------------------------
int DropDownList::childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2) {
  if (msg == ChildNotify::LISTWND_ITEMSELCHANGED && param2 && trap_click) {
    sendAction(action_list, L"get_selection");
    closeList();
  }
  return DROPDOWNLIST_PARENT::childNotify(child, msg, param1, param2);
}

// -----------------------------------------------------------------------
int DropDownList::onDeferredCallback(intptr_t p1, intptr_t p2) {
  if (p1 == DDL_CLOSELISTCB)
    doCloseList();
  return DROPDOWNLIST_PARENT::onDeferredCallback(p1, p2);
}

// -----------------------------------------------------------------------
int DropDownList::viewer_onItemDeleted(ifc_dependent *item) {
  if (item == group_dep) {
    WASABI_API_WND->popupexit_deregister(this);
    trap_click = 0;
    list_group = NULL;
    group_dep = NULL;
    action_list = NULL;
  }
  return 1;
}

// -----------------------------------------------------------------------

void DropDownList::feedwatcher_onSetFeed(svc_textFeed *svc) 
{
  StringW a = getRootWndName();
  if (a.isempty())
    setName(svc->getFeedDescription(getFeedId()));
}

void DropDownList::feedwatcher_onFeedChange(const wchar_t *data) 
{
  setItems(data);
}

// -----------------------------------------------------------------------
void DropDownList::deleteAllItems() {
  items.deleteAll();
  selected = -1;
}

// -----------------------------------------------------------------------
int DropDownList::onKeyDown(int keyCode) {
#ifdef _WIN32
  if (isListOpen()) {
    switch (keyCode) {
      case VK_ESCAPE: 
        closeList();
        break;
    }
    if (listif != NULL) {
      listif->onKeyDown(keyCode);
    }
  } else {
    switch (keyCode) {
      case VK_SPACE: 
      case VK_RETURN: 
        openList();
        break;
    }
  }
#else
#warning port me
#endif
  return DROPDOWNLIST_PARENT::onKeyDown(keyCode);
}

// -----------------------------------------------------------------------
int DropDownList::onKeyUp(int keyCode) {
  if (isListOpen()) {
    if (listif != NULL) {
      listif->onKeyUp(keyCode);
      return 1;
    }
  }
  return DROPDOWNLIST_PARENT::onKeyDown(keyCode);
}

// -----------------------------------------------------------------------
void DropDownList::setItems(const wchar_t *value) {
  deleteAllItems();
  ParamParser pp(value);
  for (int i=0;i<pp.getNumItems();i++) {
    addItem(pp.enumItem(i));
  }
  selectDefault();
}

// -----------------------------------------------------------------------

int DDLEntry::id_gen=0;


// -----------------------------------------------------------------------
// Script Object

DropDownListScriptController _dropDownListController;
DropDownListScriptController *dropDownListController = &_dropDownListController;

// -- Functions table -------------------------------------
function_descriptor_struct DropDownListScriptController::exportedFunction[] = {
  {L"getItemSelected", 0, (void*)DropDownListScriptController::DropDownList_getItemSelected},
  {L"onSelect",        2, (void*)DropDownListScriptController::DropDownList_onSelect},

  {L"setListHeight",   1, (void*)DropDownListScriptController::DropDownList_setListHeight},
  {L"openList",        0, (void*)DropDownListScriptController::DropDownList_openList},
  {L"closeList",       0, (void*)DropDownListScriptController::DropDownList_closeList},
  {L"setItems",        1, (void*)DropDownListScriptController::DropDownList_setItems},
  {L"addItem",         1, (void*)DropDownListScriptController::DropDownList_addItem},
  {L"delItem",         1, (void*)DropDownListScriptController::DropDownList_delItem},
  {L"findItem",        1, (void*)DropDownListScriptController::DropDownList_findItem},
  {L"getNumItems",     0, (void*)DropDownListScriptController::DropDownList_getNumItems},
  {L"selectItem",      2, (void*)DropDownListScriptController::DropDownList_selectItem},
  {L"getItemText",     1, (void*)DropDownListScriptController::DropDownList_getItemText},
  {L"getSelected",     0, (void*)DropDownListScriptController::DropDownList_getSelected},
  {L"getSelectedText", 0, (void*)DropDownListScriptController::DropDownList_getSelectedText},
  {L"getCustomText",   0, (void*)DropDownListScriptController::DropDownList_getCustomText},
  {L"deleteAllItems",  0, (void*)DropDownListScriptController::DropDownList_deleteAllItems},
  {L"setNoItemText",   1, (void*)DropDownListScriptController::DropDownList_setNoItemText},
};
                                      
ScriptObject *DropDownListScriptController::instantiate() {
  DropDownList *ddl = new DropDownList;
  ASSERT(ddl != NULL);
  return ddl->GuiObjectWnd::getScriptObject();
}

void DropDownListScriptController::destroy(ScriptObject *o) {
  DropDownList *ddl= static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  ASSERT(ddl != NULL);
  delete ddl;
}

void *DropDownListScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for DropDownlist yet
}

void DropDownListScriptController::deencapsulate(void *o) {
}

int DropDownListScriptController::getNumFunctions() { 
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *DropDownListScriptController::getExportedFunctions() { 
  return exportedFunction; 
}


scriptVar DropDownListScriptController::DropDownList_getItemSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList*>(o->vcpu_getInterface(dropDownListGuid));
  const wchar_t *p=L"";
  if (ddl) p = ddl->getSelectedText();

  	
  return MAKE_SCRIPT_STRING(p);
}

scriptVar DropDownListScriptController::DropDownList_onSelect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar id, scriptVar hover) {
  SCRIPT_FUNCTION_INIT
  PROCESS_HOOKS2(o, dropDownListController, id, hover);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, id, hover);
}

/*void*/ scriptVar DropDownListScriptController::DropDownList_setListHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar h) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  if (ddl) {
    ddl->setListHeight(GET_SCRIPT_INT(h));
  }
  RETURN_SCRIPT_VOID;
}

/*void*/ scriptVar DropDownListScriptController::DropDownList_openList(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  if (ddl) {
    ddl->openList();
  }
  RETURN_SCRIPT_VOID;
}

/*void*/ scriptVar DropDownListScriptController::DropDownList_closeList(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  if (ddl) {
    ddl->closeList();
  }
  RETURN_SCRIPT_VOID;
}

/*void*/ scriptVar DropDownListScriptController::DropDownList_setItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar lotsofitems) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  if (ddl) {
    ddl->setItems(GET_SCRIPT_STRING(lotsofitems));
  }
  RETURN_SCRIPT_VOID;
}

/*int*/ scriptVar DropDownListScriptController::DropDownList_addItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar text) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  int retval = 0;
  if (ddl) 
	{
    retval = ddl->addItem(GET_SCRIPT_STRING(text));
  }
  return MAKE_SCRIPT_INT(retval);
}

/*void*/ scriptVar DropDownListScriptController::DropDownList_delItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar id) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  if (ddl) {
    ddl->delItem(GET_SCRIPT_INT(id));
  }
  RETURN_SCRIPT_VOID;
}

/*int*/ scriptVar DropDownListScriptController::DropDownList_findItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar text) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  int retval = 0;
  if (ddl) {
    retval = ddl->findItem(GET_SCRIPT_STRING(text));
  }
  return MAKE_SCRIPT_INT(retval);
}

/*int*/ scriptVar DropDownListScriptController::DropDownList_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  int retval = 0;
  if (ddl) {
    retval = ddl->getNumItems();
  }
  return MAKE_SCRIPT_INT(retval);
}

/*void*/ scriptVar DropDownListScriptController::DropDownList_selectItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar id, /*int*/ scriptVar hover) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  if (ddl) {
    ddl->selectItem(GET_SCRIPT_INT(id), GET_SCRIPT_INT(hover));
  }
  RETURN_SCRIPT_VOID;
}

/*String*/ scriptVar DropDownListScriptController::DropDownList_getItemText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*int*/ scriptVar id) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  const wchar_t *retval = L"";
  if (ddl) {
    retval = ddl->getItemText(GET_SCRIPT_INT(id));
  }
  return MAKE_SCRIPT_STRING(retval);
}

/*int*/ scriptVar DropDownListScriptController::DropDownList_getSelected(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  int retval = 0;
  if (ddl) {
    retval = ddl->getSelected();
  }
  return MAKE_SCRIPT_INT(retval);
}

/*String*/ scriptVar DropDownListScriptController::DropDownList_getSelectedText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
	const wchar_t *retval = L"";
  if (ddl) {
    retval = ddl->getSelectedText();
  }
  return MAKE_SCRIPT_STRING(retval);
}

/*String*/ scriptVar DropDownListScriptController::DropDownList_getCustomText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  const wchar_t *retval=L"";
  if (ddl) {
    retval = ddl->getCustomText();
  }
  return MAKE_SCRIPT_STRING(retval);
}

/*void*/ scriptVar DropDownListScriptController::DropDownList_deleteAllItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  if (ddl) {
    ddl->deleteAllItems();
  }
  RETURN_SCRIPT_VOID;
}

/*void*/ scriptVar DropDownListScriptController::DropDownList_setNoItemText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, /*String*/ scriptVar txt) {
  SCRIPT_FUNCTION_INIT
  DropDownList *ddl = static_cast<DropDownList *>(o->vcpu_getInterface(dropDownListGuid));
  if (ddl) {
    ddl->setNoItemText(GET_SCRIPT_STRING(txt));
  }
  RETURN_SCRIPT_VOID;
}

