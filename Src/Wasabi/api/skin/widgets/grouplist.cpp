#include <precomp.h>
#include <api/skin/widgets/group.h>
#include <api/skin/groupmgr.h>
#include "grouplist.h"
#include <api/config/items/cfgitem.h>
#include <api/wnd/notifmsg.h>
const wchar_t groupListXuiObjectStr[] = L"GroupList"; // This is the xml tag
char groupListXuiSvcName[] = "GroupList xui object"; // this is the name of the xuiservice

GroupList::GroupList() {
  getScriptObject()->vcpu_setInterface(grouplistGuid, (void *)static_cast<GroupList *>(this));
  getScriptObject()->vcpu_setClassName(L"GroupList");
  getScriptObject()->vcpu_setController(grouplistController);
  scrollY = 0;
  maxheight = 0;
  maxwidth = 0;
  redraw=1;
}

GroupList::~GroupList() {
  // todo: unload group scripts
  for (int i=0;i<groups.getNumItems();i++)
    GroupMgr::destroy(groups.enumItem(i));
  groups.removeAll();
}

Group *GroupList::instantiate(const wchar_t *id, int n) {
  Group *last=NULL;
  RECT r;
  getClientRect(&r);

  for (int i=0;i<n;i++) {

    last = GroupMgr::instantiate(id);
    last->setParent(this);
    last->init(this);

    groups.addItem(last);
  }

  reposChildren();
  notifyParent(ChildNotify::AUTOWHCHANGED);
  invalidate();

  return last;
}

int GroupList::onResize() {
  int r = GROUPLIST_PARENT::onResize();
  reposChildren();
  return r;
}

Group *GroupList::enumItem(int n) {
  return groups.enumItem(n);
}

void GroupList::removeAll() {
  for (int i=0;i<groups.getNumItems();i++)
    GroupMgr::destroy(groups.enumItem(i));
  groups.removeAll();
  notifyParent(ChildNotify::AUTOWHCHANGED);
  if (!redraw) return;
  invalidate();
}

int GroupList::getNumItems() {
  return groups.getNumItems();
}

void GroupList::scrollToPercent(int p) {
  RECT r;
  getClientRect(&r);
  int height = r.bottom - r.top;
  if (height > maxheight) return;
  scrollTo((int)((float)(maxheight - height) * (float)p / 100.0f));
}

void GroupList::scrollTo(int y) {
  scrollY = y;
  if (!redraw) return;
  invalidate();
}

void GroupList::reposChildren() {
  if (!redraw) return;

  RECT r;
  getClientRect(&r);

  int ch = -scrollY+r.top;
  maxheight = 0;

  for (int i=0;i<getNumItems();i++) {

    Group *g = enumItem(i);
    
    int h = g->getPreferences(SUGGESTED_H);
    int w = g->getPreferences(SUGGESTED_W);

    RECT cr;
    getClientRect(&cr);
    cr.top = ch;
    cr.bottom = cr.top + h;

    g->resize(&cr);
    maxheight += h;
    if (maxwidth < w) maxwidth = w;
    ch += h;
  }

}

void GroupList::setRedraw(int i) {
  if (redraw == i) return;
  redraw=i;
  if (redraw) {
    notifyParent(ChildNotify::AUTOWHCHANGED);
    reposChildren();
    invalidate();
  }
}

int GroupList::getPreferences(int what) {
  if (what == SUGGESTED_H) { reposChildren(); return maxheight; }
  if (what == SUGGESTED_W) { reposChildren(); return maxwidth; }
  return GROUPLIST_PARENT::getPreferences(what);
}


GroupListScriptController _grouplistController;
GroupListScriptController  *grouplistController = &_grouplistController;


// -- Functions table -------------------------------------
function_descriptor_struct GroupListScriptController ::exportedFunction[] = {
  {L"instantiate", 2, (void*)GroupList::script_vcpu_instantiate },
  {L"getNumItems", 0, (void*)GroupList::script_vcpu_getNumItems },
  {L"enumItem", 1, (void*)GroupList::script_vcpu_enumItem },
  {L"removeAll", 0, (void*)GroupList::script_vcpu_removeAll },
  {L"scrollToPercent", 1, (void*)GroupList::script_vcpu_scrollToPercent },
  {L"setRedraw", 1, (void*)GroupList::script_vcpu_setRedraw},
};

const wchar_t *GroupListScriptController ::getClassName() {
  return L"GroupList";
}

const wchar_t *GroupListScriptController ::getAncestorClassName() {
  return L"GuiObject";
}

ScriptObject *GroupListScriptController::instantiate() {
  GroupList *gl = new GroupList;
  ASSERT(gl != NULL);
  return gl->getScriptObject();
}

void GroupListScriptController::destroy(ScriptObject *o) {
  GroupList *gl = static_cast<GroupList *>(o->vcpu_getInterface(grouplistGuid));
  ASSERT(gl != NULL);
  delete gl;
}

void *GroupListScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for grouplist yet
}

void GroupListScriptController::deencapsulate(void *o) {
}


int GroupListScriptController ::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *GroupListScriptController ::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID GroupListScriptController ::getClassGuid() {
  return grouplistGuid;
}

scriptVar GroupList::script_vcpu_instantiate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar id, scriptVar n) {
  SCRIPT_FUNCTION_INIT
  GroupList *gl = static_cast<GroupList *>(o->vcpu_getInterface(grouplistGuid));
  Group *g = NULL;
  if (gl) g = gl->instantiate(GET_SCRIPT_STRING(id), GET_SCRIPT_INT(n));
  return MAKE_SCRIPT_OBJECT(g ? g->getScriptObject() : NULL);
}

scriptVar GroupList::script_vcpu_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  GroupList *gl = static_cast<GroupList *>(o->vcpu_getInterface(grouplistGuid));
  if (gl) return MAKE_SCRIPT_INT(gl->getNumItems());
  RETURN_SCRIPT_ZERO;
}

scriptVar GroupList::script_vcpu_enumItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n) {
  SCRIPT_FUNCTION_INIT
  GroupList *gl = static_cast<GroupList *>(o->vcpu_getInterface(grouplistGuid));
  Group *g = NULL;
  if (gl) g = gl->enumItem(GET_SCRIPT_INT(n));
  return MAKE_SCRIPT_OBJECT(g ? g->getScriptObject() : NULL);
}

scriptVar GroupList::script_vcpu_removeAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  GroupList *gl = static_cast<GroupList *>(o->vcpu_getInterface(grouplistGuid));
  if (gl) gl->removeAll();
  RETURN_SCRIPT_VOID;
}

scriptVar GroupList::script_vcpu_scrollToPercent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n) {
  SCRIPT_FUNCTION_INIT
  GroupList *gl = static_cast<GroupList *>(o->vcpu_getInterface(grouplistGuid));
  if (gl) gl->scrollToPercent(GET_SCRIPT_INT(n));
  RETURN_SCRIPT_VOID;
}

scriptVar GroupList::script_vcpu_setRedraw(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n) {
  SCRIPT_FUNCTION_INIT
  GroupList *gl = static_cast<GroupList *>(o->vcpu_getInterface(grouplistGuid));
  if (gl) gl->setRedraw(GET_SCRIPT_INT(n));
  RETURN_SCRIPT_VOID;
}

