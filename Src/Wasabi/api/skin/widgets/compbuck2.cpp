#include "precomp.h"
#include <bfc/std_math.h>
#include "compbuck2.h"
#include <api/wac/wac.h>
#include <tataki/bitmap/bitmap.h>
#include <api/wndmgr/layout.h>
#include <api/wnd/notifmsg.h>
#include <api/service/service.h>
#include <api/service/services.h>
#include <api/wnd/wndclass/svcwndhold.h>

#define TIMER_SCROLL        (0x400+8879)
#define TIMER_SCROLLPAGE    (0x400+8880)

#define TIMER_SPEED 10
#define SCROLL_STEPS 10
#define SCROLL_SPEED 20

#define SCROLL_LEFT  -2
#define SCROLL_RIGHT +2

#ifndef PI
#define PI 3.1415926536
#endif

const wchar_t componentBucketXuiObjectStr[] = L"ComponentBucket"; // This is the xml tag
char componentBucketXuiSvcName[] = "ComponentBucket xui object"; // this is the name of the xuiservice

XMLParamPair ComponentBucket2::params[] =
{
  {COMPBUCK_LEFTMARGIN, L"LEFTMARGIN"},
  {COMPBUCK_RIGHTMARGIN, L"RIGHTMARGIN"},
  {COMPBUCK_SPACING, L"SPACING"},
  {COMPBUCK_VERTICAL, L"VERTICAL"},
  {COMPBUCK_WNDTYPE, L"WNDTYPE"},
};

ComponentBucket2::ComponentBucket2() {
  getScriptObject()->vcpu_setInterface(cbucketGuid, (void *)static_cast<ComponentBucket2 *>(this));
  getScriptObject()->vcpu_setClassName(L"ComponentBucket");
  getScriptObject()->vcpu_setController(cbucketController);
  lastticcount=0;
  wndtype = BUCKETITEM;
  timeron = 0;
  lmargin = 2;
  rmargin = 2;
  spacing = 2;
  direction = 0;
  xscroll = 0;
  timerset = 0;
  cblist.addItem(this);
  vertical = 0;
  scrollpage_timerset = 0;
  scrollpage_starttime = 0;
  scrollpage_start = 0;
  scrollpage_target = 0;
  scrollpage_speed = 250;
  xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);		
}

void ComponentBucket2::CreateXMLParameters(int master_handle)
{
	//COMPONENTBUCKET2_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

ComponentBucket2::~ComponentBucket2() {
  if (scrollpage_timerset) killTimer(TIMER_SCROLLPAGE);
  myclients.deleteAll();
  cblist.removeItem(this);
  if (cblist.getNumItems() == 0) cblist.removeAll();
}

int ComponentBucket2::onInit() {
  COMPONENTBUCKET2_PARENT::onInit();
  load();
  return 1;
}

int ComponentBucket2::setXuiParam(int _xuihandle, int id, const wchar_t *name, const wchar_t *strval) {
  if (xuihandle == _xuihandle) {
    switch (id) {
      case COMPBUCK_LEFTMARGIN: setLMargin(WTOI(strval)); return 1;
      case COMPBUCK_RIGHTMARGIN: setRMargin(WTOI(strval)); return 1;
      case COMPBUCK_SPACING: setSpacing(WTOI(strval)); return 1;
      case COMPBUCK_VERTICAL: setVertical(WTOI(strval)); return 1;
      case COMPBUCK_WNDTYPE: wndtype = strval; return 1;
    }
  }
  return COMPONENTBUCKET2_PARENT::setXuiParam(_xuihandle, id, name, strval);
}

void ComponentBucket2::setScroll(int v) {
  xscroll = v;
  if (isInited())
    onResize();
  invalidate();
}

int ComponentBucket2::getScroll() {
  return xscroll;
}

void ComponentBucket2::timerCallback(int id) {
  switch (id) {
    case TIMER_SCROLL: {
      RECT r;
      getClientRect(&r);
      do {
        xscroll += direction;
        lastticcount += TIMER_SPEED;
      } while (lastticcount < Wasabi::Std::getTickCount() - TIMER_SPEED);
      if (!vertical)
        xscroll = MIN((int)(getMaxWidth()-(r.right-r.left)), xscroll);
      else
        xscroll = MIN((int)(getMaxHeight()-(r.bottom-r.top)), xscroll);
      xscroll = MAX(0, xscroll);
      if (isInited())
        onResize();
      invalidate();
      return;
    }
    case TIMER_SCROLLPAGE: {
      int n = MulDiv(Wasabi::Std::getTickCount()-scrollpage_starttime,256,scrollpage_speed);
      if (n > 255) n = 255; if (n < 0) n = 0;
  	  float sintrans = (float)(sin(((float)n/255)*PI-PI/2)/2+0.5);
      xscroll = (int)(((float)(scrollpage_target - scrollpage_start) * sintrans) + scrollpage_start);
      if (n == 255) {
        killTimer(TIMER_SCROLLPAGE);
        scrollpage_timerset = 0;
      }
      if (isInited())
        onResize();
      invalidate();
      return;
    }
    default:
      COMPONENTBUCKET2_PARENT::timerCallback(id);
      return;
  }
}

int ComponentBucket2::childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2) {
  switch (msg) {
    case ChildNotify::COMPONENTBUCKET_SETTEXT: 
      setText((const wchar_t *)p1);
      return 1;
  }
  return COMPONENTBUCKET2_PARENT::childNotify(child, msg, p1, p2);
}

void ComponentBucket2::prev_down(Group *l) {
  if (!l) return;
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist[i]->getGuiObject()->guiobject_getParentGroup()->getParentContainer() == l->getParentContainer()) 
      cblist[i]->prev_down();
  }
}

void ComponentBucket2::next_down(Group *l) {
  if (!l) return;
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist[i]->getGuiObject()->guiobject_getParentGroup()->getParentContainer() == l->getParentContainer()) 
      cblist[i]->next_down();
  }
}

void ComponentBucket2::prev_up(Group *l) {
  if (!l) return;
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist[i]->getGuiObject()->guiobject_getParentGroup()->getParentContainer() == l->getParentContainer()) 
      cblist[i]->prev_up();
  }
}

void ComponentBucket2::next_up(Group *l) {
  if (!l) return;
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist[i]->getGuiObject()->guiobject_getParentGroup()->getParentContainer() == l->getParentContainer()) 
      cblist[i]->next_up();
  }
}

void ComponentBucket2::prev_page(Group *l) {
  if (!l) return;
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist[i]->getGuiObject()->guiobject_getParentGroup()->getParentContainer() == l->getParentContainer()) 
      cblist[i]->prev_page();
  }
}

void ComponentBucket2::next_page(Group *l) {
  if (!l) return;
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist[i]->getGuiObject()->guiobject_getParentGroup()->getParentContainer() == l->getParentContainer()) 
      cblist[i]->next_page();
  }
}

void ComponentBucket2::prev_down() {
  direction = SCROLL_LEFT;
  startScrollTimer();
}

void ComponentBucket2::prev_up() {
  stopScrollTimer();
}

void ComponentBucket2::next_down() {
  direction = SCROLL_RIGHT;
  startScrollTimer();
}

void ComponentBucket2::next_up() {
  stopScrollTimer();
}

void ComponentBucket2::prev_page() {
  scrollpage_starttime = Wasabi::Std::getTickCount();
  scrollpage_start = xscroll;
  RECT r;
  getClientRect(&r);
  int nx = 0;
  if (vertical)
    nx = r.bottom - r.top;
  else
    nx = r.right - r.left;
  nx = xscroll - nx;
  if (nx < 0) nx = 0;
  scrollpage_target = nx;
  if (!scrollpage_timerset) {
    setTimer(TIMER_SCROLLPAGE, 20);
    scrollpage_timerset = 1;
  }
}

void ComponentBucket2::next_page() {
  scrollpage_starttime = Wasabi::Std::getTickCount();
  scrollpage_start = xscroll;
  RECT r;
  getClientRect(&r);
  int nx = 0;
  if (vertical)
    nx = r.bottom - r.top;
  else
    nx = r.right - r.left;
  nx += xscroll;
  if (!vertical)
    nx = MIN((int)(getMaxWidth()-(r.right-r.left)), nx);
  else
    nx = MIN((int)(getMaxHeight()-(r.bottom-r.top)), nx);
  scrollpage_target = nx;
  if (!scrollpage_timerset) {
    setTimer(TIMER_SCROLLPAGE, 20);
    scrollpage_timerset = 1;
  }
}

void ComponentBucket2::startScrollTimer() {
  if (timerset) return;
  timerset=1;
  lastticcount = Wasabi::Std::getTickCount();
  setTimer(TIMER_SCROLL, TIMER_SPEED);
}

void ComponentBucket2::stopScrollTimer() {
  if (!timerset) return;
  killTimer(TIMER_SCROLL);
  timerset=0;
}

void ComponentBucket2::setText(const wchar_t *t) 
{
  for (int i=0;i<txtlist.getNumItems();i++) {
    if (txtlist.enumItem(i)->getRootParent() == getRootParent()) {
      txtlist.enumItem(i)->setName(t);
      txtlist.enumItem(i)->invalidate();
    }
  }
}

void ComponentBucket2::setText(ifc_window *cb , const wchar_t *txt) {
  for (int i=0;i<cblist.getNumItems();i++) {
    if (static_cast<ifc_window *>(cblist.enumItem(i)) == cb)
      cblist.enumItem(i)->setText(txt);
  }
}

void ComponentBucket2::registerText(Text *t, const wchar_t *id) 
{
  ComponentBucket2 *cb = getComponentBucket(id);
  if (cb) {
    cb->doRegisterText(t);
    return;
  }
  ifc_window *p = t->getDesktopParent();
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist.enumItem(i)->getDesktopParent() == p)
      cblist.enumItem(i)->doRegisterText(t);
  }
}

void ComponentBucket2::unRegisterText(Text *t, const wchar_t *id) {
  ComponentBucket2 *cb = getComponentBucket(id);
  if (cb) {
    cb->doUnregisterText(t);
    return;
  }
  ifc_window *p = t->getDesktopParent();
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist.enumItem(i)->getDesktopParent() == p)
      cblist.enumItem(i)->doUnregisterText(t);
  }
}

ComponentBucket2 *ComponentBucket2::getComponentBucket(const wchar_t *cb) {
  if (!cb) return NULL;
  for (int i=0;i<cblist.getNumItems();i++) {
    if (cblist.enumItem(i)->getGuiObject()->guiobject_getId() 
			&& !WCSICMP(cblist.enumItem(i)->getGuiObject()->guiobject_getId(), cb))
      return cblist.enumItem(i);
  }
  return NULL;
}

void ComponentBucket2::setLMargin(int i) {
  lmargin = i;
  invalidate();
}

void ComponentBucket2::setRMargin(int i) {
  rmargin = i;
  invalidate();
}

void ComponentBucket2::setSpacing(int i) {
  spacing = i;
  invalidate();
}

int ComponentBucket2::getLMargin(void) {
  return lmargin;
}

int ComponentBucket2::getRMargin(void) {
  return rmargin;
}

int ComponentBucket2::getSpacing(void) {
  return spacing;
}

void ComponentBucket2::load() {
  WindowCreateByTypeEnum wce(wndtype);
  svc_windowCreate *obj;
  while ((obj = wce.getNext()) != NULL) {
    addItems(obj);
  }
  if (isPostOnInit())
    onResize();
}

void ComponentBucket2::doRegisterText(Text *t) {
  txtlist.addItem(t);
}

void ComponentBucket2::doUnregisterText(Text *t) {
  txtlist.delItem(t);
}


void ComponentBucket2::addItems(svc_windowCreate *svc) {

  for (int i = 0; ; i++) {
    ASSERTPR(i < 1336, "someone is lame-o");
    ServiceWndHolder *svcwnd = new ServiceWndHolder;
    ifc_window *wnd = svc->createWindowOfType(wndtype, svcwnd, i);
    if (wnd == NULL) {
      delete svcwnd;
      if (i==0) SvcEnum::release(svc);
      break;
    }
    svcwnd->setChild(wnd, svc);
    myclients.addItem(svcwnd);
    svcwnd->setStartHidden(1);
    svcwnd->init(this);
  }
}

int ComponentBucket2::getMaxWidth() { 
  if (!vertical) {
    int p = getLMargin();

    for (int j=0;j<myclients.getNumItems();j++) {
  //CUT    for (int i=0;i<myclients.enumItem(j)->wnds.getNumItems();i++) {
        RECT r;
        myclients[j]->getClientRect(&r);
        p+=(r.right-r.left)+getSpacing();
  //CUT    }
    }

    return p+getRMargin();
  }
  RECT r;
  getClientRect(&r);
  return r.right-r.left;
}

int ComponentBucket2::getMaxHeight() { 
  if (vertical) {
    int p = getLMargin();

    for (int j=0;j<myclients.getNumItems();j++) {
  //CUT    for (int i=0;i<myclients.enumItem(j)->wnds.getNumItems();i++) {
        RECT r;
        myclients[j]->getClientRect(&r);
        p+=(r.bottom-r.top)+getSpacing();
  //CUT    }
    }

    return p+getLMargin();
  }
  RECT r;
  getClientRect(&r);
  return r.right-r.left;
}

int ComponentBucket2::onResize() {
  COMPONENTBUCKET2_PARENT::onResize();

  RECT r;
  getClientRect(&r);

  int sh;
  int myh;
  if (!vertical) {
    sh = r.bottom - r.top;
  } else {
    sh = r.right - r.left;
  }
  myh = sh;


  int p = getLMargin() - xscroll;

  for (int j=0;j<myclients.getNumItems();j++) {
//CUT    for (int i=0;i<myclients.enumItem(j)->wnds.getNumItems();i++) {
//CUT      api_window *c = myclients.enumItem(j)->wnds.enumItem(i);
ifc_window *c = myclients[j];
      int w = c->getPreferences(SUGGESTED_W);
      int h = c->getPreferences(SUGGESTED_H);

      if (!vertical) {
        float rt = (float)myh / (float)h;
        if (ABS(rt-1.0f) > 0.1) {
          w = (int)((float)w*rt);
          h = (int)((float)h*rt);
        }
      } else {
        float rt = (float)myh / (float)w;
        if (ABS(rt-1.0f) > 0.1) {
          w = (int)((float)w*rt);
          h = (int)((float)h*rt);
        }
      }

      if (!vertical) 
        c->resize(p+r.left, r.top+(sh - h) / 2, w, h);
      else
        c->resize(r.left+(sh - w) / 2, p+r.top, w, h);

      if (!vertical)
        p+=w+getSpacing();
      else
        p+=h+getSpacing();

      if (!c->isVisible()) c->setVisible(1);
//CUT    }
  }

  return 1;
}

void ComponentBucket2::setVertical(int v) {
  if (v == vertical) return;
  vertical = v;
  if (isInited()) invalidate();
}

int ComponentBucket2::getNumChildren() {
  return myclients.getNumItems();
}

GuiObject *ComponentBucket2::enumChildren(int i) {
  ServiceWndHolder *w = myclients.enumItem(i);
  if (!w) return NULL;
  ifc_window *_w = w->rootwndholder_getRootWnd();
  if (!_w) return NULL;
  return _w->getGuiObject();
}

PtrList<ComponentBucket2> ComponentBucket2::cblist;

CompBucketScriptController _cbucketController;
CompBucketScriptController  *cbucketController = &_cbucketController;


// -- Functions table -------------------------------------
function_descriptor_struct CompBucketScriptController ::exportedFunction[] = {
  {L"getMaxWidth", 0, (void*)ComponentBucket2::script_vcpu_getMaxWidth},
  {L"getMaxHeight", 0, (void*)ComponentBucket2::script_vcpu_getMaxHeight},
  {L"getScroll", 0, (void*)ComponentBucket2::script_vcpu_getScroll},
  {L"setScroll", 1, (void*)ComponentBucket2::script_vcpu_setScroll},
  {L"getNumChildren", 0, (void*)ComponentBucket2::script_vcpu_getNumChildren},
  {L"enumChildren", 1, (void*)ComponentBucket2::script_vcpu_enumChildren},
  {L"fake", 0, (void*)ComponentBucket2::script_vcpu_fake },
};

const wchar_t *CompBucketScriptController ::getClassName() {
  return L"ComponentBucket";
}

const wchar_t *CompBucketScriptController ::getAncestorClassName() {
  return L"GuiObject";
}

ScriptObject *CompBucketScriptController::instantiate() {
  ComponentBucket2 *cb = new ComponentBucket2;
  ASSERT(cb != NULL);
  return cb->getScriptObject();
}

void CompBucketScriptController::destroy(ScriptObject *o) {
  ComponentBucket2 *cb = static_cast<ComponentBucket2 *>(o->vcpu_getInterface(cbucketGuid));
  ASSERT(cb != NULL);
  delete cb;
}

void *CompBucketScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for componentbucket yet
}

void CompBucketScriptController::deencapsulate(void *o) {
}

int CompBucketScriptController ::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *CompBucketScriptController ::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID CompBucketScriptController ::getClassGuid() {
  return cbucketGuid;
}

scriptVar ComponentBucket2::script_vcpu_fake(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  RETURN_SCRIPT_VOID;
}

scriptVar ComponentBucket2::script_vcpu_getMaxWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ComponentBucket2 *cb = static_cast<ComponentBucket2*>(o->vcpu_getInterface(cbucketGuid));
  if (cb) return MAKE_SCRIPT_INT(cb->getMaxWidth());
  RETURN_SCRIPT_ZERO;
}

scriptVar ComponentBucket2::script_vcpu_getMaxHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ComponentBucket2 *cb = static_cast<ComponentBucket2*>(o->vcpu_getInterface(cbucketGuid));
  if (cb) return MAKE_SCRIPT_INT(cb->getMaxHeight());
  RETURN_SCRIPT_ZERO;
}

scriptVar ComponentBucket2::script_vcpu_getScroll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ComponentBucket2 *cb = static_cast<ComponentBucket2*>(o->vcpu_getInterface(cbucketGuid));
  if (cb) return MAKE_SCRIPT_INT(cb->getScroll());
  RETURN_SCRIPT_ZERO;
}

scriptVar ComponentBucket2::script_vcpu_setScroll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT
  ComponentBucket2 *cb = static_cast<ComponentBucket2*>(o->vcpu_getInterface(cbucketGuid));
  if (cb) cb->setScroll(GET_SCRIPT_INT(v));
  RETURN_SCRIPT_VOID;
}

scriptVar ComponentBucket2::script_vcpu_getNumChildren(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ComponentBucket2 *cb = static_cast<ComponentBucket2*>(o->vcpu_getInterface(cbucketGuid));
  if (cb) return MAKE_SCRIPT_INT(cb->getNumChildren());
  RETURN_SCRIPT_VOID;
}

scriptVar ComponentBucket2::script_vcpu_enumChildren(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT
  ComponentBucket2 *cb = static_cast<ComponentBucket2*>(o->vcpu_getInterface(cbucketGuid));
  if (cb) {
    GuiObject *o = cb->enumChildren(GET_SCRIPT_INT(v));
    if (o) return MAKE_SCRIPT_OBJECT(o->guiobject_getScriptObject());
  }
  RETURN_SCRIPT_ZERO;
}

