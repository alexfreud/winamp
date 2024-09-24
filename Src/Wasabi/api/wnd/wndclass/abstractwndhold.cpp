#include "precomp.h"
#include <api/syscb/api_syscb.h>

#include "abstractwndhold.h"
#include <api/service/svc_enum.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/service/svcs/svc_wndcreate.h>
#include <api/script/scriptguid.h>
#include <api/script/objects/c_script/c_guiobject.h>
#include <api/script/objects/c_script/c_group.h>
#include <api/wnd/notifmsg.h>


#define CB_ABSTRACTLOAD 0x1125

AbstractWndHolder::AbstractWndHolder(const wchar_t *grpid, int autoresize) : ServiceWndHolder(NULL, NULL) 
{
  scripts_enabled = 1;
  groupid = grpid;
  group_item = NULL;
  guid = INVALID_GUID;
  group = NULL;
  cbreg = 0;
  inselfresize = 0;
  autoresizefromcontent = autoresize;
  allow_deferred_content = 0;
  need_deferred_load = 0;
}

AbstractWndHolder::AbstractWndHolder(SkinItem *groupitem, int autoresize) : ServiceWndHolder(NULL, NULL) {
  group_item = groupitem;
  guid = INVALID_GUID;
  group = NULL;
  cbreg = 0;
  inselfresize = 0;
  autoresizefromcontent = autoresize;
  allow_deferred_content = 0;
  need_deferred_load = 0;
}

AbstractWndHolder::AbstractWndHolder(GUID g, int autoresize) : ServiceWndHolder(NULL, NULL) {
  guid = g;
  group = NULL;
  cbreg = 0;
  group_item = NULL;
  inselfresize = 0;
  autoresizefromcontent = autoresize;
  allow_deferred_content = 0;
  need_deferred_load = 0;
}

AbstractWndHolder::~AbstractWndHolder() {
  if (group) {
#ifdef WASABI_COMPILE_SKIN
    WASABI_API_SKIN->group_destroy(group);
#endif
    group = NULL;
  }
  if (cbreg) WASABI_API_SYSCB->syscb_deregisterCallback(this);
}

int AbstractWndHolder::onInit() {
  ABSTRACTWNDHOLDER_PARENT::onInit();
  createChild();
  return 1;
}

ifc_window *AbstractWndHolder::rootwndholder_getRootWnd() {
  if (group != NULL) return group;
  return ABSTRACTWNDHOLDER_PARENT::rootwndholder_getRootWnd();
}

void AbstractWndHolder::abstract_setContent(const wchar_t *group_id, int autoresize) 
{
  setBothContent(group_id, INVALID_GUID, autoresize);
}

void AbstractWndHolder::abstract_setContentBySkinItem(SkinItem *item, int autoresize) {
  setContentSkinItem(item, autoresize);
}

void AbstractWndHolder::abstract_setContent(GUID g, int autoresize) {
  setBothContent(NULL, g, autoresize);
}

void AbstractWndHolder::setBothContent(const wchar_t *group_id, GUID g, int autoresize) 
{
  group_item = NULL;
  if (autoresize != -1)
    autoresizefromcontent = autoresize;
  if (WCSCASEEQLSAFE(groupid, group_id) || (guid != INVALID_GUID && guid == g)) return;
  GUID _g = nsGUID::fromCharW(group_id);
  if (g == INVALID_GUID && _g != INVALID_GUID) {
    groupid = NULL;
    guid = _g;
  } else {
    groupid = group_id;
    guid = g;
  }
  createChild();
}

void AbstractWndHolder::setContentSkinItem(SkinItem *item, int autoresize) 
{
  if (autoresize != -1)
    autoresizefromcontent = autoresize;
  groupid.trunc(0);
  guid = INVALID_GUID;
  group_item = item;
  createChild();
}

int AbstractWndHolder::onGroupChange(const wchar_t *grpid) 
{
  if (WCSCASEEQLSAFE(groupid, grpid)) 
	{
    createChild();
    notifyParent(ChildNotify::GROUPRELOAD);
    notifyParent(ChildNotify::AUTOWHCHANGED);
  }
  return 1;
}

void AbstractWndHolder::createChild() {
  if (!isInited()) return;
  destroyContent();
  if (allow_deferred_content && !isVisible())
    need_deferred_load = 1;
  else
    doLoadContent();
}

void AbstractWndHolder::destroyContent() 
{
  if (group != NULL) {
#ifdef WASABI_COMPILE_SKIN
    WASABI_API_SKIN->group_destroy(group);
#endif
    if (cbreg) {
      WASABI_API_SYSCB->syscb_deregisterCallback(this);
      cbreg=0;
    }
    ABSTRACTWNDHOLDER_PARENT::setChild(NULL, NULL);
  }
  group = NULL;
}

int AbstractWndHolder::onDeferredCallback(intptr_t p1, intptr_t p2) 
{
  if (p1 == CB_ABSTRACTLOAD) {
    doLoadContent();
    return 1;
  }
  return ABSTRACTWNDHOLDER_PARENT::onDeferredCallback(p1, p2);
}

#include <bfc/util/profiler.h>

void AbstractWndHolder::doLoadContent() {
  int didsomething = 0;
  need_deferred_load = 0;
  if (group_item != NULL) {
#ifdef WASABI_COMPILE_SKIN
    group = WASABI_API_SKIN->group_createBySkinItem(group_item, scripts_enabled);
#endif
    if (group) {
      if (!cbreg) {
        WASABI_API_SYSCB->syscb_registerCallback(this);
        cbreg=1;
      }
      group->setParent(this);
      group->init(this);
      abstract_onNewContent();
      didsomething = 1;
    }
  } else if (!groupid.isempty()) {
#ifdef WASABI_COMPILE_SKIN
    group = WASABI_API_SKIN->group_create(groupid, scripts_enabled);
#endif
    if (group) {
      if (!cbreg) {
        WASABI_API_SYSCB->syscb_registerCallback(this);
        cbreg=1;
      }
      group->setParent(this);
      group->init(this);
      abstract_onNewContent();
      didsomething = 1;
    }
  } else if (guid != INVALID_GUID) {
    svc_windowCreate *svc = WindowCreateByGuidEnum(guid).getNext();
    ABSTRACTWNDHOLDER_PARENT::setChild(svc->createWindowByGuid(guid, this), svc);
    abstract_onNewContent();
      didsomething = 1;
  }
  if (didsomething && isPostOnInit()) 
	{
    onResize();
  }
}

void AbstractWndHolder::abstract_onNewContent() 
{
  if (isPostOnInit()) {
    ifc_window *w = getDesktopParent();
    if (w != NULL) {
      if (w->enumMinMaxEnforcer(0)) {
        w->signalMinMaxEnforcerChanged();
      }
    }
  }
}

GuiObject *AbstractWndHolder::abstract_findObject(const wchar_t *object_id) 
{
  ifc_window *w = rootwndholder_getRootWnd();
  if (w == NULL) return NULL;
  GuiObject *o = static_cast<GuiObject *>(w->getInterface(guiObjectGuid));
  return o->guiobject_findObject(object_id);
}

ScriptObject *AbstractWndHolder::abstract_findScriptObject(const wchar_t *object_id) 
{
  ifc_window *w = rootwndholder_getRootWnd();
  if (w == NULL) return NULL;
  GuiObject *o = static_cast<GuiObject *>(w->getInterface(guiObjectGuid));
  GuiObject *fo = o->guiobject_findObject(object_id);
  if (fo != NULL) return fo->guiobject_getScriptObject();
  return NULL;
}


GuiObject *AbstractWndHolder::abstract_getContent() {
  ifc_window *w = rootwndholder_getRootWnd();
  if (w == NULL) return NULL;
  return static_cast<GuiObject *>(w->getInterface(guiObjectGuid));
}

ScriptObject *AbstractWndHolder::abstract_getContentScriptObject() {
  ifc_window *w = rootwndholder_getRootWnd();
  if (w == NULL) return NULL;
  return static_cast<ScriptObject *>(w->getInterface(scriptObjectGuid));
}

int AbstractWndHolder::onResize() {
  int rt = ABSTRACTWNDHOLDER_PARENT::onResize();
  if (group != NULL && abstract_wantAutoResizeFromContent()) {
    if (inselfresize) return rt;
    inselfresize=1;
    int w = group->getPreferences(SUGGESTED_W);
    int h = group->getPreferences(SUGGESTED_H);
    resize(NOCHANGE, NOCHANGE, w == AUTOWH ? NOCHANGE : w, h == AUTOWH ? NOCHANGE : h);
    inselfresize = 0;
  }
  return rt;  
}

void AbstractWndHolder::onSetVisible( int show )
{
    ABSTRACTWNDHOLDER_PARENT::onSetVisible( show );
    if ( allow_deferred_content )
    {
        if ( show & need_deferred_load )
        {
            doLoadContent();
        }
        if ( !show && !need_deferred_load )
        {
            destroyContent();
            need_deferred_load = 1;
        }
    }
}

void AbstractWndHolder::abstract_setScriptsEnabled(int en) 
{
  if (scripts_enabled == en) return;
  scripts_enabled = en;
  if (group != NULL) {
    destroyContent();
    doLoadContent();
  }
}

int AbstractWndHolder::abstact_getScriptsEnabled() {
  return scripts_enabled;
}
