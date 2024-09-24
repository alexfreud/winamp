#include <precomp.h>
#include <bfc/wasabi_std.h>
#include <api/wnd/notifmsg.h>
#include <api/script/scriptmgr.h>
#include <api/script/script.h>
#include <api/script/objects/guiobj.h>
#include <api/script/vcpu.h>
#include <api/skin/widgets/group.h>
//#include <api/wac/main.h>
#include <math.h>
#include <api/skin/skinparse.h>
#include <api/wndmgr/layout.h>
#include <api/script/objecttable.h>
#include <api/wnd/cwndtrack.h>
#include <api/wnd/popup.h>
#include <bfc/parse/paramparser.h>
#include <api/wnd/cursor.h>
#include <api/wnd/wndtrack.h>
#include <bfc/string/stringdict.h>
#include <api/config/items/cfgitem.h>
#include <api/config/items/attrbool.h>
#ifndef PI
#define PI 3.1415926536F
#endif

#include <api/script/objects/guiobject.h>

const wchar_t guiobjectXuiStr[] = L"GuiObject"; // This is the xml tag
char guiobjectXuiSvcName[] = "GuiObject xui object"; // this is the name of the xuiservice

#ifdef USEAPPBAR
extern _bool cfg_options_appbarondrag;
#endif

static wchar_t txt[4096];

GuiObjectI::GuiObjectI(ScriptObject *o) {
	translate = 1;
  my_script_object = o;
  my_root_wnd = NULL;

  redock.l = NULL;
  MEMSET(&redock.original_rect, 0, sizeof(RECT));
  targetstatus = TARGET_FROZEN;
  targetspeed = 4;
  start_time = 0;
  reversetarget = 0;
  dodragcheck = 0;
  gui_rx = 0;
  gui_ry = 0;
  gui_rw = 0;
  gui_rh = 0;
  gui_x = 0;
  gui_y = 0;
  gui_w = AUTOWH;
  gui_h = AUTOWH;
  p_group = NULL;
  targetx = AUTOWH;
  targety = AUTOWH;
  targetw = AUTOWH;
  targeth = AUTOWH;
  targeta = AUTOWH;
  in_area = 0;
  clickthrough = 0;
  autosysmetricsx = 0;
  autosysmetricsy = 0;
  autosysmetricsw = 0;
  autosysmetricsh = 0;
  xuisvc = NULL;
  xuifac = NULL;
  mover = 0;
  moving = 0;
  droptarget = 0;
#ifdef WASABI_COMPILE_CONFIG
  cfgitem = NULL;
#endif
  timer.setGuiObjectI(this);
  wantfocus = 0;
  anchorage = ANCHOR_NONE;
  anchor_x1 = anchor_x2 = anchor_y1 = anchor_y2 = 0;
  anchorage_invalidated = 0;
  anchorage = ANCHOR_LEFT|ANCHOR_TOP;
  cursor = NULL;
#ifdef USEAPPBAR
  m_dock_side = APPBAR_NOTDOCKED;
#endif
  m_lastnondocked_x = -0xFFFF;
  m_lastnondocked_y = -0xFFFF;
}

GuiObjectI::~GuiObjectI() {
  notifylist.deleteAll();
  delete cursor;
  if (guiobject_getParentGroup())
    guiobject_getParentGroup()->removeObject(this);
  if (targetstatus == TARGET_RUNNING) 
    stopTargetTimer();
#ifdef WASABI_COMPILE_CONFIG
  if (cfgitem) viewer_delViewItem(cfgitem);
#endif
}

ScriptObject *GuiObjectI::guiobject_getScriptObject() 
{
  return my_script_object;
}

// Used by us when parsing xml to assign the object's ID, shouldn't be used elsewhere
void GuiObjectI::guiobject_setId(const wchar_t *id) 
{
  guiobject_id = id;
}

const wchar_t *GuiObjectI::guiobject_getId() 
{
  if (guiobject_id.isempty()) return L"";
  return guiobject_id; //FG> avoid returning NULL
} 

int GuiObjectI::guiobject_setXmlParam(const wchar_t *paramname, const wchar_t *strvalue) 
{
  int r = 0;
  //ifc_window *w = guiobject_getRootWnd();
  XmlObject *xo = static_cast<XmlObject *>(guiobject_getScriptObject()->vcpu_getInterface(xmlObjectGuid));
  if (xo != NULL) {
    r = xo->setXmlParam(paramname, strvalue);
  }
  return r;
}

const wchar_t *GuiObjectI::guiobject_getXmlParam(const wchar_t *paramname) {
  const wchar_t *rt = NULL;
  //ifc_window *w = guiobject_getRootWnd();
  XmlObject *xo = static_cast<XmlObject *>(guiobject_getScriptObject()->vcpu_getInterface(xmlObjectGuid));
  if (xo != NULL) {
    int r = xo->getXmlParam(paramname);
    rt = xo->getXmlParamValue(r);
  }
  return rt;
}

int GuiObjectI::guiobject_setXmlParamById(int id, const wchar_t *strvalue) 
{
  int a;
  switch (id) {
    case GuiObjectWnd::GUIOBJECT_ID:
      guiobject_setId(strvalue);
      break;
    case GuiObjectWnd::GUIOBJECT_ALPHA:
      if (wcschr(strvalue, ','))  // erroneous value, this is probably a color, or something
        guiobject_setAlpha(255);
      else
        guiobject_setAlpha(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_ACTIVEALPHA:
      if (wcschr(strvalue, ','))  // erroneous value, this is probably a color, or something
        guiobject_setActiveAlpha(255);
      else
        guiobject_setActiveAlpha(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_INACTIVEALPHA:
      if (wcschr(strvalue, ','))  // erroneous value, this is probably a color, or something
        guiobject_setInactiveAlpha(255);
      else
        guiobject_setInactiveAlpha(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SYSREGION:
      guiobject_setRegionOp(WASABI_API_SKIN->parse(strvalue, L"regionop"));
      break;
    case GuiObjectWnd::GUIOBJECT_RECTRGN:
      guiobject_setRectRgn(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_TOOLTIP:
      guiobject_getRootWnd()->setTip(strvalue);
      break;
    case GuiObjectWnd::GUIOBJECT_SYSMETRICSX:
      guiobject_setAutoSysMetricsX(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SYSMETRICSY:
      guiobject_setAutoSysMetricsY(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SYSMETRICSW:
      guiobject_setAutoSysMetricsW(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SYSMETRICSH:
      guiobject_setAutoSysMetricsH(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_MOVE:
      guiobject_setMover(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_RENDERBASETEXTURE:
      guiobject_getRootWnd()->setRenderBaseTexture(WTOI(strvalue));
      break;
#ifdef WASABI_COMPILE_CONFIG
    case GuiObjectWnd::GUIOBJECT_CFGATTR:
      setCfgAttr(strvalue);
      break;
#endif
    case GuiObjectWnd::GUIOBJECT_TABORDER:
      guiobject_setTabOrder(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_X: {
      a = WTOI(strvalue);
      guiobject_setGuiPosition(&a, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; }
    case GuiObjectWnd::GUIOBJECT_Y: {
      a = WTOI(strvalue);
      guiobject_setGuiPosition(NULL, &a, NULL, NULL, NULL, NULL, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; }
    case GuiObjectWnd::GUIOBJECT_W: {
      a = WTOI(strvalue);
      guiobject_setGuiPosition(NULL, NULL, &a, NULL, NULL, NULL, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; }
    case GuiObjectWnd::GUIOBJECT_H: {
      a = WTOI(strvalue);
      guiobject_setGuiPosition(NULL, NULL, NULL, &a, NULL, NULL, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; }
    case GuiObjectWnd::GUIOBJECT_FITTOPARENT: {
      int v = WTOI(strvalue);
      if (v) {
        int one = 1;
        int x = 0, y = 0, w = 0, h = 0;
        if (v < 0) {
          v = -v;
          x += v;
          y += v;
          w -= v * 2;
          h -= v * 2;
        }
        guiobject_setGuiPosition(&x, &y, &w, &h, NULL, NULL, &one, &one);
        Group *g = guiobject_getParentGroup();
        if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
          g->updatePos(this);
      }
      break; }
    case GuiObjectWnd::GUIOBJECT_WANTFOCUS:
      wantfocus = WTOI(strvalue);
      if (guiobject_getRootWnd()->isPostOnInit()) {
        if (wantfocus) {
          if (guiobject_getRootWnd()->getTabOrder() == -1)
            guiobject_getRootWnd()->setAutoTabOrder();
        } else {
          if (guiobject_getRootWnd()->getTabOrder() != -1)
          guiobject_getRootWnd()->setTabOrder(-1);
        }
      }
      break;
    case GuiObjectWnd::GUIOBJECT_VISIBLE: {
      a = WTOI(strvalue);
      ifc_window *w = guiobject_getRootWnd();
      if (w->isPostOnInit())
        w->setVisible(a);
      else
        w->setStartHidden(!a);
      break; }
    case GuiObjectWnd::GUIOBJECT_RELATX: {
      if (strvalue && *strvalue == '%')
        a = 2;
      else
        a = WTOI(strvalue);
      guiobject_setGuiPosition(NULL, NULL, NULL, NULL, &a, NULL, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; }
    case GuiObjectWnd::GUIOBJECT_RELATY: {
      if (strvalue && *strvalue == '%')
        a = 2;
      else
        a = WTOI(strvalue);
      guiobject_setGuiPosition(NULL, NULL, NULL, NULL, NULL, &a, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; }
    case GuiObjectWnd::GUIOBJECT_RELATW: {
      if (strvalue && *strvalue == '%')
        a = 2;
      else
        a = WTOI(strvalue);
      guiobject_setGuiPosition(NULL, NULL, NULL, NULL, NULL, NULL, &a, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; }
    case GuiObjectWnd::GUIOBJECT_RELATH: {
      if (strvalue && *strvalue == '%')
        a = 2;
      else
        a = WTOI(strvalue);
      guiobject_setGuiPosition(NULL, NULL, NULL, NULL, NULL, NULL, NULL, &a);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; }
    case GuiObjectWnd::GUIOBJECT_DROPTARGET:
      guiobject_setDropTarget(strvalue);
      break;
    case GuiObjectWnd::GUIOBJECT_GHOST:
      guiobject_setClickThrough(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SETNODBLCLICK:
      guiobject_setNoDoubleClick(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SETNOLEFTCLICK:
      guiobject_setNoLeftClick(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SETNORIGHTCLICK:
      guiobject_setNoRightClick(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SETNOMOUSEMOVE:
      guiobject_setNoMouseMove(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SETNOCONTEXTMENU:
      guiobject_setNoContextMenu(WTOI(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_SETX1: {
      a = WTOI(strvalue);
      guiobject_setAnchoragePosition(&a, NULL, NULL, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; 
      }
    case GuiObjectWnd::GUIOBJECT_SETY1:{
      a = WTOI(strvalue);
      guiobject_setAnchoragePosition(NULL, &a, NULL, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; 
      }
    case GuiObjectWnd::GUIOBJECT_SETX2:{
      a = WTOI(strvalue);
      guiobject_setAnchoragePosition(NULL, NULL, &a, NULL, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; 
      }
    case GuiObjectWnd::GUIOBJECT_SETY2:{
      a = WTOI(strvalue);
      guiobject_setAnchoragePosition(NULL, NULL, NULL, &a, NULL);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
      break; 
      }
    case GuiObjectWnd::GUIOBJECT_SETANCHOR: {
      int anchorage=ANCHOR_NONE;
      ParamParser pp(strvalue, L"|");
      for (int m=0;m<pp.getNumItems();m++) 
			{
        const wchar_t *s = pp.enumItem(m);
        if (!WCSICMP(s, L"left")) anchorage |= ANCHOR_LEFT;
        else if (!WCSICMP(s, L"top")) anchorage |= ANCHOR_TOP;
        else if (!WCSICMP(s, L"right")) anchorage |= ANCHOR_RIGHT;
        else if (!WCSICMP(s, L"bottom")) anchorage |= ANCHOR_BOTTOM;
      }
      guiobject_setAnchoragePosition(NULL, NULL, NULL, NULL, &anchorage);
      Group *g = guiobject_getParentGroup();
      if (g != NULL && guiobject_getRootWnd()->isPostOnInit())
        g->updatePos(this);
    }
    break;
    case GuiObjectWnd::GUIOBJECT_SETCURSOR:
      guiobject_setCursor(strvalue);
      break;
    case GuiObjectWnd::GUIOBJECT_NOTIFY:
      notifylist.addItem(new StringW(strvalue));
      break;
    case GuiObjectWnd::GUIOBJECT_USERDATA:
      // nothing to do, param goes in xmlobject
      break;
#ifdef USEAPPBAR
    case GuiObjectWnd::GUIOBJECT_APPBAR:
      setAppBar(strvalue);
      break;
#endif
		case GuiObjectWnd::GUIOBJECT_TRANSLATE:
			translate = WTOI(strvalue);
			break;
    default:
      return 0;
  }
  return 1;
}

void GuiObjectI::guiobject_getGuiPosition(int *x, int *y, int *w, int *h, int *rx, int *ry, int *rw, int *rh) {
  if (x) *x = gui_x;
  if (y) *y = gui_y;
  if (w) *w = gui_w;
  if (h) *h = gui_h;
  if (rx) *rx = gui_rx;
  if (ry) *ry = gui_ry;
  if (rw) *rw = gui_rw;
  if (rh) *rh = gui_rh;
}

void GuiObjectI::guiobject_setGuiPosition(int *x, int *y, int *w, int *h, int *rx, int *ry, int *rw, int *rh) {
  if (x && *x != NOCHANGE && *x != AUTOWH) gui_x = *x;
  if (y && *y != NOCHANGE && *y != AUTOWH) gui_y = *y;
  if (w && *w != NOCHANGE && *w != AUTOWH) gui_w = *w;
  if (h && *h != NOCHANGE && *h != AUTOWH) gui_h = *h;
  if (rx && *rx != NOCHANGE && *rx != AUTOWH) gui_rx = *rx;           
  if (ry && *ry != NOCHANGE && *ry != AUTOWH) gui_ry = *ry;
  if (rw && *rw != NOCHANGE && *rw != AUTOWH) gui_rw = *rw;
  if (rh && *rh != NOCHANGE && *rh != AUTOWH) gui_rh = *rh;
  if (guiobject_getRootWnd()->isInited() && guiobject_getParentGroup())
    guiobject_getParentGroup()->updatePos(this);
  else {
    if (!guiobject_getRootWnd()->isVirtual())
      guiobject_getRootWnd()->resize(gui_x, gui_y, gui_w, gui_h);
  }
}

int GuiObjectI::guiobject_getAnchoragePosition(int *x1, int *y1, int *x2, int *y2, int *anchor) {
  if (x1) *x1 = anchor_x1;
  if (y1) *y1 = anchor_y1;
  if (x2) *x2 = anchor_x2;
  if (y2) *y2 = anchor_y2;
  if (anchor) *anchor = anchorage;
  return anchorage_invalidated;
}

void GuiObjectI::guiobject_setAnchoragePosition(int *x1, int *y1, int *x2, int *y2, int *anchor) {
  anchorage_invalidated = 1;
  if (x1) anchor_x1 = *x1;
  if (x2) anchor_x2 = *x2;
  if (y1) anchor_y1 = *y1;
  if (y2) anchor_y2 = *y2;
  if (anchor) anchorage = *anchor;
  if (guiobject_getRootWnd()->isInited() && guiobject_getParentGroup()) {
    guiobject_getParentGroup()->updatePos(this);
  }
}

void GuiObjectI::guiobject_validateAnchorage() {
  anchorage_invalidated = 0;
}

void GuiObjectI::guiobject_setClickThrough(int c) {
  guiobject_getRootWnd()->setClickThrough(c);
}

void GuiObjectI::guiobject_setRegionOp(int op) {
  guiobject_getRootWnd()->setRegionOp(op);
}

int GuiObjectI::guiobject_getRegionOp() {
  return guiobject_getRootWnd()->getRegionOp();
}

void GuiObjectI::guiobject_setRectRgn(int rrgn) {
  guiobject_getRootWnd()->setRectRgn(rrgn);
}

void GuiObjectI::guiobject_setMover(int n) {
  mover = n;
}

int GuiObjectI::guiobject_getMover() {
  return mover;
}

FOURCC GuiObjectI::guiobject_getDropTarget() {
  return droptarget;
}

void GuiObjectI::guiobject_setDropTarget(const wchar_t *strval) 
{
  if (strval == NULL)
    droptarget = 0;
  else
	{
		uint8_t *temp = (uint8_t *)&droptarget;
		temp[3]=(uint8_t)strval[0];
		temp[2]=(uint8_t)strval[1];
		temp[1]=(uint8_t)strval[2];
		temp[0]=(uint8_t)strval[3];
	}
}

int GuiObjectI::guiobject_isRectRgn() {
  return guiobject_getRootWnd()->isRectRgn();
}

int GuiObjectI::guiobject_isClickThrough() {
  return guiobject_getRootWnd()->isClickThrough();
}

void GuiObjectI::guiobject_setParentGroup(Group *l) {
  if (!l) { p_group = NULL; return; }
  p_group = l;
}

Group *GuiObjectI::guiobject_getParentGroup() {
  if (!p_group) return NULL;
  return p_group;
}

GuiObject *GuiObjectI::guiobject_getParent() {
  ifc_window *grw = guiobject_getRootWnd();
  if (!grw) return NULL;
  ifc_window *w = grw->getParent();
  if (!w) return NULL;
  return static_cast<GuiObject *>(w->getInterface(guiObjectGuid));
}

#ifdef WASABI_COMPILE_WNDMGR
Layout *GuiObjectI::guiobject_getParentLayout() {
  Group *m = p_group;
  Layout *l = NULL;
  while (m) {
    if (m->isLayout()) {
      l = static_cast<Layout *>(m);
      break;
    }
    m = m->getGuiObject()->guiobject_getParentGroup();
  }
  if (!l) {
    ifc_window *w = guiobject_getRootWnd()->getDesktopParent();
    if (w)
      l = static_cast<Layout *>(w->getInterface(layoutGuid));
  }
  if (l && l->isDeleting()) return NULL;
  return l;
}
#endif

GuiObject *GuiObjectI::guiobject_getTopParent() {
  ifc_window *m = guiobject_getRootWnd();
  GuiObject *top = this;

  while (m != NULL) {
    m = m->getParent();
    if (m != NULL) {
      GuiObject *g = m->getGuiObject();
      if (g != NULL)                   
        top = g;
    }
  }

  return top;
}

/*void GuiObjectI::parseNotify(const char *s) {
 scriptNotify(s, "", 0, 0);
}*/

void GuiObjectI::guiobject_bringToFront() {
  ifc_window *b = guiobject_getRootWnd();
  if (b) {
    if (b->getParent())
      b->getParent()->bringVirtualToFront(b);
  }
}

void GuiObjectI::guiobject_bringToBack() {
  ifc_window *b = guiobject_getRootWnd();
  if (b) {
    if (b->getParent())
      b->getParent()->bringVirtualToBack(b);
  }
}

void GuiObjectI::guiobject_bringAbove(GuiObject *o) {
  ASSERT(o != NULL);
  ifc_window *b = guiobject_getRootWnd();
  ifc_window *c = o->guiobject_getRootWnd();
  if (b && c) {
    if (b->getParent())
      b->getParent()->bringVirtualAbove(b, c);
  }
}

void GuiObjectI::guiobject_bringBelow(GuiObject *o) {
  ASSERT(o != NULL);
  ifc_window *b = guiobject_getRootWnd();
  ifc_window *c = o->guiobject_getRootWnd();
  if (b && c) {
    if (b->getParent())
      b->getParent()->bringVirtualBelow(b, c);
  }
}

void GuiObjectI::guiobject_setTargetSpeed(float s) { // s == n of seconds
  if (targetspeed == (int)(s * 4.0)) return;
  targetspeed = (int)(s * 4.0); // units of 250ms
  if (targetstatus == TARGET_RUNNING) {
    stopTargetTimer();
    startTargetTimer();
  }
}

void GuiObjectTimer::timerclient_timerCallback(int id) {
  if (id == TARGETTIMER_ID && obj) 
    obj->onTargetTimer();
}

void GuiObjectI::guiobject_setTargetX(int x) {
  targetx = x;
}

void GuiObjectI::guiobject_setTargetY(int y) {
  targety = y;
}

void GuiObjectI::guiobject_setTargetW(int w) {
  targetw = w;
}

void GuiObjectI::guiobject_setTargetH(int h) {
  targeth = h;
}

void GuiObjectI::guiobject_setTargetA(int a) {
  targeta = a;
}

void GuiObjectI::guiobject_gotoTarget() {
  if (!guiobject_getRootWnd()) return;
  start_time=0;
  guiobject_getGuiPosition(&startx, &starty, &startw, &starth, NULL, NULL, NULL, NULL);
  starta = guiobject_getAlpha();
  if (targetx == AUTOWH || targetx == NOCHANGE)
    targetx = startx;
  if (targety == AUTOWH || targety == NOCHANGE)
    targety = starty;
  if (targetw == AUTOWH || targetw == NOCHANGE)
    targetw = startw;
  if (targeth == AUTOWH || targeth == NOCHANGE)
    targeth = starth;
  if (targeta == AUTOWH || targeta == NOCHANGE)
    targeta = starta;
  startTargetTimer();
  Layout *l = static_cast<Layout *>(guiobject_getScriptObject()->vcpu_getInterface(layoutGuid));
  if (targetx != startx ||
      targety != starty ||
      targetw != startw ||
      targeth != starth) {
    if (l) windowTracker->beforeRedock(l, &redock);
  }
}

void GuiObjectI::startTargetTimer() {
  timer.timerclient_setTimer(TARGETTIMER_ID, 20);
  targetstatus = TARGET_RUNNING;
}

void GuiObjectI::stopTargetTimer() {
  timer.timerclient_killTimer(TARGETTIMER_ID);
  targetstatus = TARGET_FROZEN;
}

void GuiObjectI::onTargetTimer() {

  if (targetstatus != TARGET_RUNNING) return;
  if (!guiobject_getRootWnd()) return;

  RECT r;
  guiobject_getRootWnd()->getClientRect(&r);
  RECT wr;
  guiobject_getRootWnd()->getWindowRect(&wr);

  int ttime=250*targetspeed;
  if (ttime < 0) ttime = 0;

  int n;
  if (!start_time)
	{
		n=0; 
		start_time = Wasabi::Std::getTickCount();
	}
  else
	{
		n=MulDiv(Wasabi::Std::getTickCount()-start_time,256,ttime);
	}

  if (ttime == 0) n = 255;

  if (n >= 255) n=255;

	float sintrans = (float)(sin(((float)n/255)*PI-PI/2)/2+0.5); // used for smoothing transitions

  float nw = ((float)(targetw - startw) * sintrans) + startw;
  float nh = ((float)(targeth - starth) * sintrans) + starth;
  float na = ((float)(targeta - starta) * sintrans) + starta;

  Layout *l = static_cast<Layout *>(guiobject_getScriptObject()->vcpu_getInterface(layoutGuid));
  int islayout = l != NULL && static_cast<ifc_window*>(l) == guiobject_getRootWnd();

  float rat = 1.0f;
  if (islayout) rat = (float)l->getRenderRatio();

  float nx;
  float ny;
  if (!reversetarget) {
    nx = ((float)(targetx - startx) * sintrans) + startx;
    ny = ((float)(targety - starty) * sintrans) + starty;
  } else {
    nx = startx - ((float)(targetw - startw) * sintrans) * rat;
    ny = starty - ((float)(targeth - starth) * sintrans) * rat;
  }

  int zx=(int)nx;
  int zy=(int)ny;
  int zw=(int)nw;
  int zh=(int)nh;

  if (reversetarget) {
    while (zy + zh * rat < wr.bottom) zy++;
    while (zx + zw * rat < wr.right) zx++;
  }

  int oldredraw = -1;
  if (reversetarget && islayout) {
    oldredraw = l->wantRedrawOnResize();
    l->setWantRedrawOnResize(0);

    int paddtop = wr.top - (int)ny;
    int paddleft = wr.left - (int)nx;

#ifdef _WIN32
    if (paddtop > 0 || paddleft > 0) {
      RegionI r;
      GetWindowRgn(l->gethWnd(), r.getOSHandle());
      r.offset(MAX(0, paddleft), MAX(0, paddtop));
      SetWindowRgn(l->gethWnd(), r.makeWindowRegion(), FALSE);
    }
#else
#warning port me
#endif
  }
  guiobject_setGuiPosition(&zx, &zy, &zw, &zh, NULL, NULL, NULL, NULL);
  guiobject_getRootWnd()->cascadeRepaint(0);
  guiobject_setAlpha((int)na);
  if (n==255) {
    stopTargetTimer();
    guiobject_onTargetReached();
  }
  if (l != NULL) l->savePosition();
  if (oldredraw != -1 && l) { 
    l->setWantRedrawOnResize(oldredraw);
  }
}

void GuiObjectI::guiobject_cancelTarget() {
  stopTargetTimer();
  Layout *l = static_cast<Layout *>(guiobject_getScriptObject()->vcpu_getInterface(layoutGuid));
  if (l && redock.l) windowTracker->afterRedock(l, &redock);
}

void GuiObjectI::guiobject_reverseTarget(int reverse) {
  reversetarget = reverse;
}

#ifdef WASABI_COMPILE_WNDMGR
void GuiObjectI::guiobject_popParentLayout() {
  Layout *l = guiobject_getParentLayout();
  if (l && l->getParentContainer()) {
    SkinParser::showContainer(l->getParentContainer()->getId(), TRUE);
#ifdef WIN32
    SetForegroundWindow(l->gethWnd());
#else
    l->bringToFront();
#endif
  }
}
#endif

int GuiObjectI::guiobject_movingToTarget() {
  return targetstatus == TARGET_RUNNING;
}

void GuiObjectI::guiobject_onLeftButtonDown(int x, int y) {
  if (!VCPU::getComplete()) {
    scriptVar _x = SOM::makeVar(SCRIPT_INT);
    SOM::assign(&_x, x);
    scriptVar _y = SOM::makeVar(SCRIPT_INT);
    SOM::assign(&_y, y);
    GuiObject_ScriptMethods::onLeftButtonDown(SCRIPT_CALL, guiobject_getScriptObject(), _x, _y);
    if (mover && !VCPU::getComplete()) {
  #ifdef WASABI_COMPILE_WNDMGR
      Layout *l = guiobject_getParentLayout();
      if (l) {
		  // Martin> (9/9/8) added l->getGuiObject()->guiobject_getMover() so setting move="0" to layouts will disable moving it around
		  if (!l->isLocked() && l->getGuiObject()->guiobject_getMover()) {
#ifdef USEAPPBAR
          if (cfg_options_appbarondrag) {
            m_initial_dock_side = m_dock_side = l->appbar_getSide();
            if (m_dock_side != APPBAR_NOTDOCKED) {
              dodragcheck = 1;
              goto skipit;
            }
          }
#endif
          l->maximize(0);
          skipit:
          l->beginMove();
          if (l->getParentContainer() && l->getParentContainer()->isMainContainer() || Std::keyModifier(STDKEY_ALT))
            WASABI_API_WNDMGR->wndTrackStartCooperative(l);
          moving = 1;
          anchor.x = (int)((float)x * guiobject_getRootWnd()->getRenderRatio());
          anchor.y = (int)((float)y * guiobject_getRootWnd()->getRenderRatio());
        }
      }
  #endif //WASABI_COMPILE_WNDMGR
    }
  }
}

void GuiObjectI::guiobject_onLeftButtonUp(int x, int y) {
  scriptVar _x = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_x, x);
  scriptVar _y = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_y, y);
  GuiObject_ScriptMethods::onLeftButtonUp(SCRIPT_CALL, guiobject_getScriptObject(), _x, _y);
  if (moving) {
    int sendendmove = 0;
#ifdef WASABI_COMPILE_WNDMGR
    Group *l = guiobject_getParentLayout();
    if (l) {
      if (WASABI_API_WNDMGR->wndTrackWasCooperative())
        WASABI_API_WNDMGR->wndTrackEndCooperative();
      moving = 0;
      sendendmove = 1; 
    }
#ifdef USEAPPBAR
    if (cfg_options_appbarondrag) {
      if (m_dock_side != m_initial_dock_side) {
        ifc_window *dw = guiobject_getRootWnd()->getDesktopParent();
        if (dw) {
          AppBar *ab = reinterpret_cast<AppBar *>(dw->getInterface(appBarGuid));
          if (ab) {
            if (m_dock_side == APPBAR_NOTDOCKED) ab->appbar_setNoRestore(1);
            ab->appbar_dock(m_dock_side);
            if (m_dock_side == APPBAR_NOTDOCKED) ab->appbar_setNoRestore(0);
          }
        }
      } 
      if (m_dock_side == APPBAR_NOTDOCKED) {
        ifc_window *dp = guiobject_getRootWnd()->getDesktopParent();
        if (dp) dp->restore(0);
      }
    } else {
      ifc_window *dp = guiobject_getRootWnd()->getDesktopParent();
      if (dp) dp->restore(0);
    }
#else
    ifc_window *dp = guiobject_getRootWnd()->getDesktopParent();
    if (dp) dp->restore(0);
#endif // USEAPPBAR
    if (sendendmove && l) l->endMove();
#endif //WASABI_COMPILE_WNDMGR
  }
}

void GuiObjectI::guiobject_onRightButtonDown(int x, int y) {
  scriptVar _x = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_x, x);
  scriptVar _y = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_y, y);
  GuiObject_ScriptMethods::onRightButtonDown(SCRIPT_CALL, guiobject_getScriptObject(), _x, _y);
}

#ifdef WASABI_CUSTOM_CONTEXTMENUS
extern void appContextMenu(ifc_window *wnd);
extern void appControlMenu(ifc_window *wnd);
#endif

void GuiObjectI::guiobject_onRightButtonUp(int x, int y) {
  scriptVar _x = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_x, x);
  scriptVar _y = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_y, y);
  GuiObject_ScriptMethods::onRightButtonUp(SCRIPT_CALL, guiobject_getScriptObject(), _x, _y);
  ifc_window *w = guiobject_getRootWnd();
  if (Std::keyModifier(STDKEY_CONTROL) && 
      Std::keyModifier(STDKEY_SHIFT)) {
    GuiObjectI::infoMenu(this, x, y);
  } else {
    if (w && w->wantAutoContextMenu() && !VCPU::getComplete() && WASABI_API_WNDMGR->getModalWnd() == NULL) {
      ifc_window *par = w->getParent();
      if (par && guiobject_getParentLayout()) {
        if (!Std::keyModifier(STDKEY_CONTROL)) {
#if defined(WA3COMPATIBILITY)
          Main::appContextMenu(par, TRUE, guiobject_getParentLayout()->isTransparencySafe());
#elif defined(WASABI_CUSTOM_CONTEXTMENUS)
          appContextMenu(par);
#endif
        } else {
          Layout *l = guiobject_getParentLayout();
          if (l->getParent() == NULL) {
#if defined(WA3COMPATIBILITY)
            l->controlMenu();
#elif defined(WASABI_CUSTOM_CONTEXTMENUS)
            appControlMenu(l);
#endif
          }
        }
      } else {
        Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
        if (l != NULL) {
          if (!Std::keyModifier(STDKEY_CONTROL)) {
#if defined(WA3COMPATIBILITY)
            Main::appContextMenu(w, TRUE, l->isTransparencySafe());
#elif defined(WASABI_CUSTOM_CONTEXTMENUS)
            appContextMenu(w);
#endif
         } else {
#if defined(WA3COMPATIBILITY)
            l->controlMenu();
#elif defined(WASABI_CUSTOM_CONTEXTMENUS)
            appControlMenu(l);
#endif
          }
        }
      }
    }
  }
}

void GuiObjectI::guiobject_onRightButtonDblClk(int x, int y) {
  scriptVar _x = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_x, x);
  scriptVar _y = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_y, y);
  GuiObject_ScriptMethods::onRightButtonDblClk(SCRIPT_CALL, guiobject_getScriptObject(), _x, _y);
}

void GuiObjectI::guiobject_onLeftButtonDblClk(int x, int y) {
  scriptVar _x = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_x, x/* - wr.left*/);
  scriptVar _y = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_y, y/* - wr.top*/);
  GuiObject_ScriptMethods::onLeftButtonDblClk(SCRIPT_CALL, guiobject_getScriptObject(), _x, _y);
}

int GuiObjectI::guiobject_onMouseWheelUp(int clicked, int lines)
{
	scriptVar retval;
	retval = GuiObject_ScriptMethods::onMouseWheelUp(SCRIPT_CALL, guiobject_getScriptObject(), MAKE_SCRIPT_INT(clicked), MAKE_SCRIPT_INT(lines) );
	int retv;

	if ((retval.type == SCRIPT_VOID) || (retval.type == SCRIPT_OBJECT) || (retval.type == SCRIPT_STRING))
		retv = 0;
	else
		retv = GET_SCRIPT_INT(retval);

	return retv;
}

int GuiObjectI::guiobject_onMouseWheelDown(int clicked, int lines)
{
	scriptVar retval;
	retval = GuiObject_ScriptMethods::onMouseWheelDown(SCRIPT_CALL, guiobject_getScriptObject(), MAKE_SCRIPT_INT(clicked), MAKE_SCRIPT_INT(lines) );
	int retv;

	if ((retval.type == SCRIPT_VOID) || (retval.type == SCRIPT_OBJECT) || (retval.type == SCRIPT_STRING))
		retv = 0;
	else
		retv = GET_SCRIPT_INT(retval);

	return retv;
}

void GuiObjectI::guiobject_onMouseMove(int x, int y) {
  scriptVar _x = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_x, x);
  scriptVar _y = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_y, y);
  GuiObject_ScriptMethods::onMouseMove(SCRIPT_CALL, guiobject_getScriptObject(), _x, _y);
  POINT pos={x,y};
  guiobject_getRootWnd()->clientToScreen((int *)&pos.x, (int *)&pos.y);
#ifdef WASABI_COMPILE_WNDMGR
  if (moving) {
#ifdef _WIN32
    if (!Std::keyDown(MK_LBUTTON)) {
#else
#warning port me
    if (0) {
#endif
      moving = 0;
    } else {
#ifdef WIN32
      int drag_x = GetSystemMetrics(SM_CXDRAG);
      int drag_y = GetSystemMetrics(SM_CYDRAG);
#else
#warning port me 
      int drag_x = 5;
      int drag_y = 5;
#endif
      POINT relatpos;
      relatpos.x = (int)((float)x*guiobject_getRootWnd()->getRenderRatio()) - anchor.x;
      relatpos.y = (int)((float)y*guiobject_getRootWnd()->getRenderRatio()) - anchor.y;
      if (!dodragcheck || (ABS(relatpos.x) >= drag_x || ABS(relatpos.y) >= drag_y)) {
        dodragcheck = 0;
        ifc_window *p = guiobject_getRootWnd()->getDesktopParent();
        if (p) {
          RECT r, cr;
          p->getWindowRect(&r);
          p->getClientRect(&cr);
          RECT nr=r;
          int w,h;

  #ifdef USEAPPBAR
          int side = APPBAR_NOTDOCKED;
          if (cfg_options_appbarondrag) {
            AppBar *ab = reinterpret_cast<AppBar *>(p->getInterface(appBarGuid));
            if (ab) {
              int _x=x, _y=y;
              p->clientToScreen(&_x, &_y);
              side = ab->appbar_testDock(_x, _y, &nr);
              if (ABS(p->getRenderRatio() - 1.0) > 0.01f) {
                int _w = nr.right-nr.left;
                int _h = nr.bottom-nr.top;
                double rr = p->getRenderRatio();
                _w = (int)((double)(_w) / rr + 0.5);
                _h = (int)((double)(_h) / rr + 0.5);
                nr.right = nr.left + _w;
                nr.bottom = nr.top + _h;
              }
            }
          }
  #endif
          w = cr.right-cr.left;
          h = cr.bottom-cr.top;
          int resize=0;
#ifdef USEAPPBAR
          if (cfg_options_appbarondrag) {
            if (side != m_dock_side) {
              if (side != APPBAR_NOTDOCKED) {
                m_lastnondocked_x = r.left;
                m_lastnondocked_y = r.top;
                resize=1;
              } else {
                RECT rr;
                p->getRestoredRect(&rr);
                w = (rr.right-rr.left);
                h = (rr.bottom-rr.top);
                if (m_lastnondocked_x != -0xFFFF) {
                  r.left = m_lastnondocked_x;
                  r.top = m_lastnondocked_y;
                }
                r.right = r.left + w;
                r.bottom = r.top + h;
                snapAdjust(p, &r, -1);
                nr=r;
                resize=1;
              }
            }
          }
#endif

		  // use the scaling to adjust the overall size so docking will be correct
		  double rr = p->getRenderRatio();
          r.left += relatpos.x;
          r.top += relatpos.y;
          r.right = r.left + (int)((double)w * rr);
          r.bottom = r.top + (int)((double)h * rr);

#ifdef USEAPPBAR

          if (side == APPBAR_NOTDOCKED && m_dock_side == APPBAR_NOTDOCKED) 
            WASABI_API_WNDMGR->wndTrackDock(p, &r, &nr, LEFT|TOP|RIGHT|BOTTOM|KEEPSIZE);

          if (side != APPBAR_NOTDOCKED || resize) {
            Layout *l = (Layout *)p->getInterface(layoutGuid);
            if (l) l->pushForceUnlink();
            {
              RECT adj = nr;
              snapAdjust(p, &adj, 1);
              int _w = adj.right-adj.left;
              int _h = adj.bottom-adj.top;
              if (ABS(p->getRenderRatio() - 1.0) > 0.01f) {
                double rr = p->getRenderRatio();
                if ((int)((double)(_w) * rr) == (int)((double)(w) * rr)) _w = w;
                if ((int)((double)(_h) * rr) == (int)((double)(h) * rr)) _h = h;
              }
              p->resize(adj.left, adj.top, _w, _h);
            }
            if (l) l->popForceUnlink();
          } else {
            p->move(r.left, r.top);
          }

          m_dock_side = side;
          guiobject_getParentLayout()->onMove();
          if (GetCapture() != guiobject_getRootWnd()->getRootParent()->gethWnd()) {
            DebugStringW(L"not mine anymore :(\n");
          }
#endif
        }
      }
    }
  }
#endif
}

// -----------------------------------------------------------------------
void GuiObjectI::snapAdjust(ifc_window *rw, RECT *r, int way) {
  RECT s;
  Layout *l = static_cast<Layout*>(rw->getInterface(layoutGuid));
  if (!l) return;
  l->getSnapAdjust(&s);
  int h = r->bottom - r->top;
  int w = r->right - r->left;
  if (way == 1) {
    h += s.top + s.bottom;
    w += s.left + s.right;
    r->left -= s.left;
    r->top -= s.top;
    r->bottom = r->top + h;
    r->right = r->left + w;
  } else if (way == -1) {
    h -= s.top + s.bottom;
    w -= s.left + s.right;
    r->left += s.left;
    r->top += s.top;
    r->bottom = r->top + h;
    r->right = r->left + w;
  }
}

#ifdef USEAPPBAR
int GuiObjectI::guiobject_getAppBar() {
  AppBar *ab = reinterpret_cast<AppBar *>(guiobject_getRootWnd()->getInterface(appBarGuid));
  if (ab) return ab->appbar_getEnabledSides();
  return 0;
}

BEGIN_STRINGDICTIONARY(_appbarvalues)
SDI(L"top", APPBAR_TOP_ENABLED);
SDI(L"left", APPBAR_LEFT_ENABLED);
SDI(L"right", APPBAR_RIGHT_ENABLED);
SDI(L"bottom", APPBAR_BOTTOM_ENABLED);
END_STRINGDICTIONARY(_appbarvalues, appbarvalues)

void GuiObjectI::guiobject_setAppBar(int en) {
  AppBar *ab = reinterpret_cast<AppBar *>(guiobject_getRootWnd()->getInterface(appBarGuid));
  if (ab) ab->appbar_setEnabledSides(en);
}

void GuiObjectI::setAppBar(const wchar_t *en) 
{
  AppBar *ab = reinterpret_cast<AppBar *>(guiobject_getRootWnd()->getInterface(appBarGuid));
  if (ab) {
    int e = 0;
    ParamParser pp(en, L"|;");
    for (int i=0;i<pp.getNumItems();i++) 
		{
			        const wchar_t *s = pp.enumItem(i);
        if (!_wcsicmp(s, L"left")) e |= APPBAR_LEFT_ENABLED;
        else if (!_wcsicmp(s, L"top")) e |= APPBAR_TOP_ENABLED;
        else if (!_wcsicmp(s, L"right")) e |= APPBAR_RIGHT_ENABLED;
        else if (!_wcsicmp(s, L"bottom")) e |= APPBAR_BOTTOM_ENABLED;
    }
    ab->appbar_setEnabledSides(e);
  }
}
#endif

const wchar_t *GuiObjectI::guiobject_getName() 
{
  const wchar_t *ret = NULL;
  ifc_window *w = guiobject_getRootWnd();
  if (w != NULL) 
		ret = w->getRootWndName();
  return ret;
}

void GuiObjectI::guiobject_onEnable(int en) {
  scriptVar _is = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_is, en);
  GuiObject_ScriptMethods::onEnable(SCRIPT_CALL, guiobject_getScriptObject(), _is);
}

void GuiObjectI::guiobject_onStartup() {
  GuiObject_ScriptMethods::onStartup(SCRIPT_CALL, guiobject_getScriptObject());
  if (guiobject_getRootObject()) {
    foreach (notifylist)
      guiobject_getRootObject()->rootobject_notify(notifylist.getfor()->getValue(), L"", 0, 0);
    endfor;
  }
}

void GuiObjectI::guiobject_onEnterArea() {
  if (in_area) return;
  GuiObject_ScriptMethods::onEnterArea(SCRIPT_CALL, guiobject_getScriptObject());
  in_area = 1;
}

void GuiObjectI::guiobject_onLeaveArea() {
  if (!in_area) return;
  GuiObject_ScriptMethods::onLeaveArea(SCRIPT_CALL, guiobject_getScriptObject());
  in_area = 0;
}

void GuiObjectI::guiobject_onCancelCapture() {
  moving = 0;
}

ifc_window *GuiObjectI::guiobject_getRootWnd(void) {
  return my_root_wnd;
}

void GuiObjectI::guiobject_setRootWnd(ifc_window *r) {
  my_root_wnd = r;
}

RootObject *GuiObjectI::guiobject_getRootObject() {
  ScriptObject *o = guiobject_getScriptObject();
  if (!o) return NULL;
  return static_cast<RootObject *>(o->vcpu_getInterface(rootObjectGuid));
}

void GuiObjectI::guiobject_onResize(int x, int y, int w, int h) {
  scriptVar _x = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_x, x);
  scriptVar _y = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_y, y);
  scriptVar _w = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_w, w);
  scriptVar _h = SOM::makeVar(SCRIPT_INT);
  SOM::assign(&_h, h);
  GuiObject_ScriptMethods::onResize(SCRIPT_CALL, guiobject_getScriptObject(), _x, _y, _w, _h);
}

void GuiObjectI::guiobject_onSetVisible(int v) {
#ifdef WASABI_COMPILE_WNDMGR
  if (guiobject_getParentLayout())
    guiobject_getParentLayout()->onGuiObjectSetVisible(this, v);
#endif
  scriptVar _v = SOM::makeVar(SCRIPT_BOOLEAN);
  SOM::assign(&_v, v);
  GuiObject_ScriptMethods::onSetVisible(SCRIPT_CALL, guiobject_getScriptObject(), _v);
#ifdef WASABI_COMPILE_WNDMGR
  if (moving) {
    if (WASABI_API_WNDMGR->wndTrackWasCooperative())
      WASABI_API_WNDMGR->wndTrackEndCooperative();
    moving = 0;
    ifc_window *dp = guiobject_getRootWnd()->getDesktopParent();
    if (dp) dp->restore(0);
    guiobject_getParentGroup()->endMove();
  }
#endif //WASABI_COMPILE_WNDMGR
}

void GuiObjectI::guiobject_setAlpha(int a) {
  if (!my_root_wnd) return;
  Layout *l = static_cast<Layout*>(my_root_wnd->getInterface(layoutGuid));
  if (l) l->setAlpha(a);
  else my_root_wnd->setAlpha(a);
}

int GuiObjectI::guiobject_getAlpha() {
  if (!my_root_wnd) return 255;
  Layout *l = static_cast<Layout*>(my_root_wnd->getInterface(layoutGuid));
  if (l) return l->getAlpha();
  return my_root_wnd->getPaintingAlpha();
}

void GuiObjectI::guiobject_setActiveAlpha(int a) {
  if (my_root_wnd) {
    int i;
    my_root_wnd->getAlpha(NULL, &i);
    my_root_wnd->setAlpha(a, i);
  }
}

int GuiObjectI::guiobject_getActiveAlpha() {
  int a = 255;
  if (my_root_wnd) my_root_wnd->getAlpha(&a);
  return a;
}

void GuiObjectI::guiobject_setInactiveAlpha(int a) {
  if (my_root_wnd) {
    int _a;
    my_root_wnd->getAlpha(&_a);
    my_root_wnd->setAlpha(_a, a);
  }
}

int GuiObjectI::guiobject_getInactiveAlpha() {
  if (my_root_wnd) {
    int i;
    my_root_wnd->getAlpha(NULL, &i);
    return i;
  }
  return 255;
}

void GuiObjectI::guiobject_onTargetReached() {
  GuiObject_ScriptMethods::onTargetReached(SCRIPT_CALL, guiobject_getScriptObject());
  Layout *l = static_cast<Layout *>(guiobject_getScriptObject()->vcpu_getInterface(layoutGuid));
  if (l && redock.l) windowTracker->afterRedock(l, &redock);
}

void GuiObjectI::guiobject_setAutoSysMetricsX(int a) {
  if (a == autosysmetricsx) return;
  autosysmetricsx = a;
  if (guiobject_getRootWnd() && guiobject_getRootWnd()->isInited())
    if (guiobject_getParentGroup()) guiobject_getParentGroup()->updatePos(this);
}

void GuiObjectI::guiobject_setAutoSysMetricsY(int a) {
  if (a == autosysmetricsy) return;
  autosysmetricsy = a;
  if (guiobject_getRootWnd() && guiobject_getRootWnd()->isInited())
    if (guiobject_getParentGroup()) guiobject_getParentGroup()->updatePos(this);
}

void GuiObjectI::guiobject_setAutoSysMetricsW(int a) {
  if (a == autosysmetricsw) return;
  autosysmetricsw = a;
  if (guiobject_getRootWnd() && guiobject_getRootWnd()->isInited())
    if (guiobject_getParentGroup()) guiobject_getParentGroup()->updatePos(this);
}

void GuiObjectI::guiobject_setAutoSysMetricsH(int a) {
  if (a == autosysmetricsh) return;
  autosysmetricsh = a;
  if (guiobject_getRootWnd() && guiobject_getRootWnd()->isInited())
    if (guiobject_getParentGroup()) guiobject_getParentGroup()->updatePos(this);
}

int GuiObjectI::guiobject_getAutoSysMetricsX() {
  return autosysmetricsx;
}

int GuiObjectI::guiobject_getAutoSysMetricsY() {
  return autosysmetricsy;
}

int GuiObjectI::guiobject_getAutoSysMetricsW() {
  return autosysmetricsw;
}

int GuiObjectI::guiobject_getAutoSysMetricsH() {
  return autosysmetricsh;
}

int GuiObjectI::guiobject_getAutoWidth() {
  if (!guiobject_getRootWnd()) return AUTOWH;
  return guiobject_getRootWnd()->getPreferences(SUGGESTED_W);
}

int GuiObjectI::guiobject_getAutoHeight() {
  if (!guiobject_getRootWnd()) return AUTOWH;
  return guiobject_getRootWnd()->getPreferences(SUGGESTED_H);
}

#ifdef WASABI_COMPILE_WNDMGR
int GuiObjectI::guiobject_runModal() {
  if (!guiobject_getRootWnd()) return 0;
  ifc_window *w = guiobject_getRootWnd()->getDesktopParent();
  #ifdef WASABI_MODAL_PUSH
  WASABI_MODAL_PUSH
  #endif
  int r = w->runModal();
  #ifdef WASABI_MODAL_POP
  WASABI_MODAL_POP
  #endif
  return r;
}

void GuiObjectI::guiobject_endModal(int retcode) {
  if (!guiobject_getRootWnd()) return;
  ifc_window *w = guiobject_getRootWnd()->getDesktopParent();
  w->endModal(retcode);
}
#endif

int GuiObjectI::guiobject_isActive() {
  if (!guiobject_getRootWnd()) return 0;
  return guiobject_getRootWnd()->isActive();
}

svc_xuiObject *GuiObjectI::guiobject_getXuiService() {
  return xuisvc;
}

void GuiObjectI::guiobject_setXuiService(svc_xuiObject *svc) {
  xuisvc = svc;
}

waServiceFactory *GuiObjectI::guiobject_getXuiServiceFactory() {
  return xuifac;
}
void GuiObjectI::guiobject_setXuiServiceFactory(waServiceFactory *fac) {
  xuifac = fac;
}

#ifdef WASABI_COMPILE_WNDMGR
void GuiObjectI::guiobject_setStatusText(const wchar_t *text, int overlay) 
{
  Layout *l = guiobject_getParentLayout();
  if (l)
    l->setStatusText(text, overlay);
}

void GuiObjectI::guiobject_registerStatusCB(GuiStatusCallback *cb) {
  Layout *l = guiobject_getParentLayout();
  if (l) l->registerStatusCallback(cb);
}

void GuiObjectI::guiobject_addAppCmds(AppCmds *commands){
  Layout *l = guiobject_getParentLayout();
  if (l) l->addAppCmds(commands);
}

void GuiObjectI::guiobject_removeAppCmds(AppCmds *commands){
  Layout *l = guiobject_getParentLayout();
  if (l) l->removeAppCmds(commands);
}

void GuiObjectI::guiobject_pushCompleted(int max) {
  Layout *l = guiobject_getParentLayout();
  if (l) l->pushCompleted(max);
}
void GuiObjectI::guiobject_incCompleted(int add) {
  Layout *l = guiobject_getParentLayout();
  if (l) l->incCompleted(add);
}
void GuiObjectI::guiobject_setCompleted(int pos) {
  Layout *l = guiobject_getParentLayout();
  if (l) l->setCompleted(pos);
}
void GuiObjectI::guiobject_popCompleted() {
  Layout *l = guiobject_getParentLayout();
  if (l) l->popCompleted();
}
#endif //WASABI_COMPILE_WNDMGR

void GuiObjectI::infoMenu(GuiObject *o, int x, int y) 
{
	PopupMenu pop;
	pop.addCommand(StringPrintfW(L"Class : %s", o->guiobject_getScriptObject()->vcpu_getClassName()), 0, 0, 1);
	pop.addCommand(StringPrintfW(L"Id : %s", o->guiobject_getId()), 0, 0, 1);
	RECT r;
	guiobject_getRootWnd()->getNonClientRect(&r);
	pop.addCommand(StringPrintfW(L"Coordinates : %d,%d (%d x %d)", r.left, r.top, r.right-r.left, r.bottom-r.top), 0, 0, 1);
	int _x, _y, _w, _h, _rx, _ry, _rw, _rh;
	guiobject_getGuiPosition(&_x, &_y, &_w, &_h, &_rx, &_ry, &_rw, &_rh);
	pop.addCommand(StringPrintfW(L"GuiPos : x=%d relatx=%d, y=%d relaty=%d, w=%d relatw=%d, h=%d relath=%d", _x, _rx, _y, _ry, _w, _rw, _h, _rh), 0, 0, 1);

	if (guiobject_wantTranslation() == 2)
	{
		if (!_wcsicmp(o->guiobject_getScriptObject()->vcpu_getClassName(), L"Text"))
		{
			pop.addCommand(StringPrintfW(L"StringEntry : %s", guiobject_getXmlParam(L"text")), 0, 0, 1);
		}
		else if (!_wcsicmp(o->guiobject_getScriptObject()->vcpu_getClassName(), L"TitleBar"))
		{
			pop.addCommand(StringPrintfW(L"StringEntry : %s", guiobject_getXmlParam(L"title")), 0, 0, 1);
		}
	}
	guiobject_getRootWnd()->clientToScreen(&x, &y);
	pop.popAtXY(x, y, 1);
}

#ifdef WASABI_COMPILE_CONFIG

void GuiObjectI::guiobject_setCfgAttrib(CfgItem *item, const wchar_t *name) 
{
  if (cfgitem) viewer_delViewItem(cfgitem);
  cfgitem = item;
  cfgattrname = name;
  if (cfgitem) {
    viewer_addViewItem(cfgitem);
  }
  ifc_window *mw = guiobject_getRootWnd();
  if (mw != NULL) {
    if (mw->isPostOnInit())
      dataChanged();
  }
}

int GuiObjectI::viewer_onEvent(CfgItem *item, int event, intptr_t param, void *ptr, size_t ptrlen) {
  if (item == cfgitem)
    dataChanged();  
  return 1;
}

void GuiObjectI::dataChanged() {
  ifc_window *w = guiobject_getRootWnd();
  if (w != NULL) {
    w->onAction(L"reload_config", NULL, -1, -1, 0, 0, NULL, 0, w);
  }
  GuiObject_ScriptMethods::onCfgChanged(SCRIPT_CALL, guiobject_getScriptObject());
}


CfgItem *GuiObjectI::guiobject_getCfgItem() {
  return cfgitem;
}

const wchar_t *GuiObjectI::guiobject_getCfgAttrib() 
{
  return cfgattrname;
}
#endif //WASABI_COMPILE_CONFIG

void GuiObjectI::guiobject_onChar(wchar_t c) 
{
  wchar_t _c[2]=L"X";
  _c[0]=c;
  GuiObject_ScriptMethods::onChar(SCRIPT_CALL, guiobject_getScriptObject(), MAKE_SCRIPT_STRING(_c));
}

void GuiObjectI::guiobject_onKeyDown(int vkcode) {
  GuiObject_ScriptMethods::onKeyDown(SCRIPT_CALL, guiobject_getScriptObject(), MAKE_SCRIPT_INT(vkcode));
}

void GuiObjectI::guiobject_onKeyUp(int vkcode) {
  GuiObject_ScriptMethods::onKeyUp(SCRIPT_CALL, guiobject_getScriptObject(), MAKE_SCRIPT_INT(vkcode));
}

void GuiObjectI::guiobject_setCursor(const wchar_t *c) 
{
  ifc_window *w = guiobject_getRootWnd();
#ifdef _WIN32
  if (w != NULL) {
    delete cursor;
    cursor = new SkinCursor(c);
    w->setDefaultCursor(cursor);
  }
#else
#warning port me
#endif
}

void GuiObjectI::guiobject_setEnabled(int en) {
  ifc_window *w = guiobject_getRootWnd();
  if (w != NULL) w->setEnabled(en);
}

int GuiObjectI::guiobject_wantTranslation()
{
	return translate;
}
int GuiObjectI::guiobject_dragEnter(ifc_window *sourceWnd)
{
	GuiObject_ScriptMethods::onDragEnter(SCRIPT_CALL, guiobject_getScriptObject());
	return 1;
}

int GuiObjectI::guiobject_dragOver(int x, int y, ifc_window *sourceWnd)
{
	GuiObject_ScriptMethods::onDragOver(SCRIPT_CALL, guiobject_getScriptObject(), MAKE_SCRIPT_INT(x), MAKE_SCRIPT_INT(y) );
	return 1;
}

int GuiObjectI::guiobject_dragLeave(ifc_window *sourceWnd)
{
	GuiObject_ScriptMethods::onDragLeave(SCRIPT_CALL, guiobject_getScriptObject());
	return 1;
}

GuiObjectScriptController _guiController;
GuiObjectScriptController *guiController = &_guiController;

// -- Functions table -------------------------------------
function_descriptor_struct GuiObjectScriptController::exportedFunction[] = {
  {L"getId", 0, (void*)GuiObject_ScriptMethods::getId },
  {L"show", 0, (void*)GuiObject_ScriptMethods::show },
  {L"hide", 0, (void*)GuiObject_ScriptMethods::hide },
  {L"onSetVisible", 1, (void*)GuiObject_ScriptMethods::onSetVisible},
  {L"isVisible", 0, (void*)GuiObject_ScriptMethods::isvisible },
  {L"setAlpha", 1, (void*)GuiObject_ScriptMethods::setAlpha },
  {L"getAlpha", 0, (void*)GuiObject_ScriptMethods::getAlpha },
  {L"setActiveAlpha", 1, (void*)GuiObject_ScriptMethods::setActiveAlpha },
  {L"getActiveAlpha", 0, (void*)GuiObject_ScriptMethods::getActiveAlpha },
  {L"setInactiveAlpha", 1, (void*)GuiObject_ScriptMethods::setInactiveAlpha },
  {L"getInactiveAlpha", 0, (void*)GuiObject_ScriptMethods::getInactiveAlpha },
  {L"onLeftButtonDown", 2, (void*)GuiObject_ScriptMethods::onLeftButtonDown },
  {L"onLeftButtonUp", 2, (void*)GuiObject_ScriptMethods::onLeftButtonUp },
  {L"onRightButtonDown", 2, (void*)GuiObject_ScriptMethods::onRightButtonDown },
  {L"onRightButtonUp", 2, (void*)GuiObject_ScriptMethods::onRightButtonUp },
  {L"onRightButtonDblClk", 2, (void*)GuiObject_ScriptMethods::onRightButtonDblClk },
  {L"onLeftButtonDblClk", 2, (void*)GuiObject_ScriptMethods::onLeftButtonDblClk },
  {L"onMouseWheelUp", 2, (void*)GuiObject_ScriptMethods::onMouseWheelUp },
  {L"onMouseWheelDown", 2, (void*)GuiObject_ScriptMethods::onMouseWheelDown },
  {L"onMouseMove", 2, (void*)GuiObject_ScriptMethods::onMouseMove },
  {L"onEnterArea", 0, (void*)GuiObject_ScriptMethods::onEnterArea },
  {L"onLeaveArea", 0, (void*)GuiObject_ScriptMethods::onLeaveArea },
  {L"isMouseOverRect", 0, (void*)GuiObject_ScriptMethods::isMouseOverRect},
  {L"onStartup", 0, (void*)GuiObject_ScriptMethods::onStartup },
  {L"onChar", 1, (void*)GuiObject_ScriptMethods::onChar },
  {L"onKeyDown", 1, (void*)GuiObject_ScriptMethods::onKeyDown},
  {L"onKeyUp", 1, (void*)GuiObject_ScriptMethods::onKeyUp},
  {L"setEnabled", 1, (void*)GuiObject_ScriptMethods::setEnabled },
  {L"getEnabled", 0, (void*)GuiObject_ScriptMethods::getEnabled },
  {L"onEnable", 1, (void*)GuiObject_ScriptMethods::onEnable },
  {L"resize", 4, (void*)GuiObject_ScriptMethods::resize },
  {L"onResize", 4, (void*)GuiObject_ScriptMethods::onResize },
  {L"isMouseOver", 2, (void*)GuiObject_ScriptMethods::isMouseOver },
  {L"getLeft", 0, (void*)GuiObject_ScriptMethods::getLeft },
  {L"getTop", 0, (void*)GuiObject_ScriptMethods::getTop },
  {L"getWidth", 0, (void*)GuiObject_ScriptMethods::getWidth },
  {L"getHeight", 0, (void*)GuiObject_ScriptMethods::getHeight },
  {L"getGuiX", 0, (void*)GuiObject_ScriptMethods::getGuiX },
  {L"getGuiY", 0, (void*)GuiObject_ScriptMethods::getGuiY },
  {L"getGuiW", 0, (void*)GuiObject_ScriptMethods::getGuiW },
  {L"getGuiH", 0, (void*)GuiObject_ScriptMethods::getGuiH },
  {L"getGuiRelatX", 0, (void*)GuiObject_ScriptMethods::getGuiRelatX },
  {L"getGuiRelatY", 0, (void*)GuiObject_ScriptMethods::getGuiRelatX },
  {L"getGuiRelatW", 0, (void*)GuiObject_ScriptMethods::getGuiRelatX },
  {L"getGuiRelatH", 0, (void*)GuiObject_ScriptMethods::getGuiRelatX },
  {L"clientToScreenX", 1, (void*)GuiObject_ScriptMethods::clientToScreenX },
  {L"clientToScreenY", 1, (void*)GuiObject_ScriptMethods::clientToScreenY },
  {L"clientToScreenW", 1, (void*)GuiObject_ScriptMethods::clientToScreenW },
  {L"clientToScreenH", 1, (void*)GuiObject_ScriptMethods::clientToScreenH },
  {L"screenToClientX", 1, (void*)GuiObject_ScriptMethods::screenToClientX },
  {L"screenToClientY", 1, (void*)GuiObject_ScriptMethods::screenToClientY },
  {L"screenToClientW", 1, (void*)GuiObject_ScriptMethods::screenToClientW },
  {L"screenToClientH", 1, (void*)GuiObject_ScriptMethods::screenToClientH },
  {L"setTargetX", 1, (void*)GuiObject_ScriptMethods::setTargetX },
  {L"setTargetY", 1, (void*)GuiObject_ScriptMethods::setTargetY },
  {L"setTargetW", 1, (void*)GuiObject_ScriptMethods::setTargetW },
  {L"setTargetH", 1, (void*)GuiObject_ScriptMethods::setTargetH },
  {L"setTargetA", 1, (void*)GuiObject_ScriptMethods::setTargetA },
  {L"setTargetSpeed", 1, (void*)GuiObject_ScriptMethods::setTargetSpeed },
  {L"gotoTarget", 0, (void*)GuiObject_ScriptMethods::gotoTarget },
  {L"onTargetReached", 0, (void*)GuiObject_ScriptMethods::onTargetReached },
  {L"cancelTarget", 0, (void*)GuiObject_ScriptMethods::cancelTarget },
  {L"reverseTarget", 1, (void*)GuiObject_ScriptMethods::reverseTarget },
  {L"isGoingToTarget", 0, (void*)GuiObject_ScriptMethods::movingToTarget },
  {L"setXmlParam", 2, (void*)GuiObject_ScriptMethods::setXmlParam },
  {L"getXmlParam", 1, (void*)GuiObject_ScriptMethods::getXmlParam },
  {L"init", 1, (void*)GuiObject_ScriptMethods::init },
  {L"bringToFront", 0, (void*)GuiObject_ScriptMethods::bringToFront },
  {L"bringToBack", 0, (void*)GuiObject_ScriptMethods::bringToBack },
  {L"bringAbove", 1, (void*)GuiObject_ScriptMethods::bringAbove },
  {L"bringBelow", 1, (void*)GuiObject_ScriptMethods::bringBelow },
  {L"isActive", 0, (void*)GuiObject_ScriptMethods::isActive},
  {L"getParent", 0, (void*)GuiObject_ScriptMethods::getParent},
  {L"getTopParent", 0, (void*)GuiObject_ScriptMethods::getTopParent},
  {L"getInterface", 1, (void*)GuiObject_ScriptMethods::getInterface},
  {L"onAction", 7, (void*)GuiObject_ScriptMethods::onAction},
#ifdef WASABI_COMPILE_WNDMGR
  {L"getParentLayout", 0, (void*)GuiObject_ScriptMethods::getParentLayout},
  {L"runModal", 0, (void*)GuiObject_ScriptMethods::runModal},
  {L"endModal", 1, (void*)GuiObject_ScriptMethods::endModal},
  {L"popParentLayout", 0, (void*)GuiObject_ScriptMethods::popParentLayout},
  {L"setStatusText", 2, (void*)GuiObject_ScriptMethods::setStatusText},
#endif
  {L"findObject", 1, (void*)GuiObject_ScriptMethods::findObject},
  {L"findObjectXY", 2, (void*)GuiObject_ScriptMethods::findObjectXY},
  {L"getName", 0, (void*)GuiObject_ScriptMethods::getName},
  {L"getAutoWidth", 0, (void*)GuiObject_ScriptMethods::getAutoWidth },
  {L"getAutoHeight", 0, (void*)GuiObject_ScriptMethods::getAutoHeight },
  {L"setFocus", 0, (void*)GuiObject_ScriptMethods::setFocus},
  {L"onGetFocus", 0, (void*)GuiObject_ScriptMethods::onGetFocus},
  {L"onKillFocus", 0, (void*)GuiObject_ScriptMethods::onKillFocus},
  {L"sendAction", 6, (void*)GuiObject_ScriptMethods::sendAction},
  {L"onAccelerator", 1, (void*)GuiObject_ScriptMethods::onAccelerator},
#ifdef WASABI_COMPILE_CONFIG
  {L"cfg_getInt",           0, (void*)GuiObject_ScriptMethods::cfgGetInt },
  {L"cfg_setInt",           1, (void*)GuiObject_ScriptMethods::cfgSetInt },
  {L"cfg_getFloat",           0, (void*)GuiObject_ScriptMethods::cfgGetFloat },
  {L"cfg_setFloat",           1, (void*)GuiObject_ScriptMethods::cfgSetFloat },
  {L"cfg_getString",           0, (void*)GuiObject_ScriptMethods::cfgGetString },
  {L"cfg_setString",           1, (void*)GuiObject_ScriptMethods::cfgSetString },
  {L"cfg_onDataChanged",        0, (void*)GuiObject_ScriptMethods::onCfgChanged },
  {L"cfg_getItemGuid",        0, (void*)GuiObject_ScriptMethods::cfgGetGuid},
  {L"cfg_getAttributeName",        0, (void*)GuiObject_ScriptMethods::cfgGetAttributeName},
#endif
	{L"onDragEnter",        0, (void*)GuiObject_ScriptMethods::onDragEnter },
  {L"onDragOver",        2, (void*)GuiObject_ScriptMethods::onDragOver},
  {L"onDragLeave",        0, (void*)GuiObject_ScriptMethods::onDragLeave},
};

const wchar_t *GuiObjectScriptController::getClassName() {
  return L"GuiObject";
}

const wchar_t *GuiObjectScriptController::getAncestorClassName() {
  return L"Object";
}

ScriptObjectController *GuiObjectScriptController::getAncestorController() {
  return rootScriptObjectController;}

int GuiObjectScriptController::getNumFunctions() {
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *GuiObjectScriptController::getExportedFunctions() {
  return exportedFunction;                                                        
}

GUID GuiObjectScriptController::getClassGuid() {
  return guiObjectGuid;
}

int GuiObjectScriptController::getInstantiable() {
  return 1;
}

ScriptObject *GuiObjectScriptController::instantiate() {
  GuiObjectWnd *w = new GuiObjectWnd;
  ASSERT(w != NULL);
  return w->getScriptObject();
}

void GuiObjectScriptController::destroy(ScriptObject *o) {
  GuiObjectWnd *w = static_cast<GuiObjectWnd *>(o->vcpu_getInterface(guiObjectWndGuid));
  ASSERT(w != NULL);
  delete w;
}

void *GuiObjectScriptController::encapsulate(ScriptObject *o) {
  return static_cast<void *>(new GuiObjectI(o));
}

void GuiObjectScriptController::deencapsulate(void *o) {
  delete static_cast<GuiObjectI *>(o);
}

// ----------------------------------------------------------------------------------------------------------------------------------

// returns a new ScriptString object containing the xml id of this object
scriptVar GuiObject_ScriptMethods::getId(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
	
  if (g)
		return MAKE_SCRIPT_STRING(g->guiobject_getId());

  return MAKE_SCRIPT_STRING(L"");
} 

scriptVar GuiObject_ScriptMethods::hide(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w)
      w->setVisible(0);
  }
  RETURN_SCRIPT_VOID;
} 

scriptVar GuiObject_ScriptMethods::show(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w)
      w->setVisible(1);
  }
  RETURN_SCRIPT_VOID;
} 

scriptVar GuiObject_ScriptMethods::isvisible(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w) return MAKE_SCRIPT_BOOLEAN(w->isVisible());
  }
  RETURN_SCRIPT_ZERO;
} 

scriptVar GuiObject_ScriptMethods::getAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) return MAKE_SCRIPT_INT(g->guiobject_getAlpha());
  return MAKE_SCRIPT_INT(255);
}

scriptVar GuiObject_ScriptMethods::setAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setAlpha(GET_SCRIPT_INT(a));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::getActiveAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) return MAKE_SCRIPT_INT(g->guiobject_getActiveAlpha());
  return MAKE_SCRIPT_INT(255);
}

scriptVar GuiObject_ScriptMethods::setActiveAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setActiveAlpha(GET_SCRIPT_INT(a));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::getInactiveAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) return MAKE_SCRIPT_INT(g->guiobject_getInactiveAlpha());
  return MAKE_SCRIPT_INT(255);
}

scriptVar GuiObject_ScriptMethods::setInactiveAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setInactiveAlpha(GET_SCRIPT_INT(a));
  RETURN_SCRIPT_VOID;
}


scriptVar GuiObject_ScriptMethods::onMouseMove(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, x, y);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, x, y);
}

scriptVar GuiObject_ScriptMethods::onLeftButtonDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, x, y);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, x, y);
}

scriptVar GuiObject_ScriptMethods::onLeftButtonUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, x, y);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, x, y);
}

scriptVar GuiObject_ScriptMethods::onRightButtonDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, x, y);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, x, y);
}

scriptVar GuiObject_ScriptMethods::onRightButtonUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, x, y);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, x, y);
}

scriptVar GuiObject_ScriptMethods::onLeftButtonDblClk(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, x, y);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, x, y);
}

scriptVar GuiObject_ScriptMethods::onRightButtonDblClk(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, x, y);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, x, y);
}

scriptVar GuiObject_ScriptMethods::onMouseWheelUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar clicked, scriptVar lines) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, clicked, lines);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, clicked, lines);
}

scriptVar GuiObject_ScriptMethods::onMouseWheelDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar clicked, scriptVar lines) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS2(o, guiController, clicked, lines);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT2(o, clicked, lines);
}

scriptVar GuiObject_ScriptMethods::onEnterArea(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, guiController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar GuiObject_ScriptMethods::onLeaveArea(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, guiController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar GuiObject_ScriptMethods::setEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&v));
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *b = g->guiobject_getRootWnd();
    if (b) b->setEnabled(GET_SCRIPT_BOOLEAN(v));
  }
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::getEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *b = g->guiobject_getRootWnd();
    if (b) return MAKE_SCRIPT_BOOLEAN(b->isEnabled());
  }
  RETURN_SCRIPT_ZERO;
}

scriptVar GuiObject_ScriptMethods::onEnable(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, guiController, v);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, v);
}

scriptVar GuiObject_ScriptMethods::onSetVisible(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, guiController, v);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, v);
}

scriptVar GuiObject_ScriptMethods::onResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y, scriptVar w, scriptVar h) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS4(o, guiController, x, y, w, h);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT4(o, x, y, w, h);
}

scriptVar GuiObject_ScriptMethods::resize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y, scriptVar w, scriptVar h) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&x));
  ASSERT(SOM::isNumeric(&y));
  ASSERT(SOM::isNumeric(&w));
  ASSERT(SOM::isNumeric(&h));
  GuiObject *go = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (go) {
    int _x = SOM::makeInt(&x);
    int _y = SOM::makeInt(&y);
    int _w = SOM::makeInt(&w);
    int _h = SOM::makeInt(&h);
    ifc_window *b = go->guiobject_getRootWnd();
    if (b) b->resize(_x, _y, _w, _h);
    go->guiobject_setGuiPosition(_x == NOCHANGE ? &_x : NULL, _y == NOCHANGE ? &_y : NULL, _w == NOCHANGE ? &_w : NULL, _h == NOCHANGE ? &_h : NULL, NULL, NULL, NULL, NULL);
    if (b && b->getInterface(layoutGuid)) {
      b->cascadeRepaint();
      ((Layout *)b->getInterface(layoutGuid))->savePosition();
    }
  }
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::isMouseOver(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&x));
  ASSERT(SOM::isNumeric(&y));
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *b = g->guiobject_getRootWnd();
    POINT pt={GET_SCRIPT_INT(x), GET_SCRIPT_INT(y)};
    b->clientToScreen((int *)&pt.x, (int *)&pt.y);
    return MAKE_SCRIPT_BOOLEAN(WASABI_API_WND->rootWndFromPoint(&pt) == b);
  }
  RETURN_SCRIPT_ZERO;
}

scriptVar GuiObject_ScriptMethods::getLeft(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  ifc_window *b = go->guiobject_getRootWnd();
  int r=0;
  if (b && b->isInited()) {
    POINT pt;
    b->getPosition(&pt);
    r = pt.x;
  } else if (b && !b->isInited()) {
    go->guiobject_getGuiPosition(&r, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  }
  return MAKE_SCRIPT_INT(r);
}

scriptVar GuiObject_ScriptMethods::movingToTarget(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) return MAKE_SCRIPT_BOOLEAN(g->guiobject_movingToTarget());
  RETURN_SCRIPT_ZERO;
}

scriptVar GuiObject_ScriptMethods::cancelTarget(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g && g->guiobject_movingToTarget()) g->guiobject_cancelTarget();
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::reverseTarget(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_reverseTarget(GET_SCRIPT_INT(r));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::getTop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  ifc_window *b = go->guiobject_getRootWnd();
  int r=0;
  if (b && b->isInited()) {
    POINT pt;
    b->getPosition(&pt);
    r = pt.y;
  } else if (b && !b->isInited()) {
    go->guiobject_getGuiPosition(NULL, &r, NULL, NULL, NULL, NULL, NULL, NULL);
  }
  return MAKE_SCRIPT_INT(r);
}

scriptVar GuiObject_ScriptMethods::getWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  ifc_window *b = go->guiobject_getRootWnd();
  int r=0;
  if (b && b->isInited()) {
    RECT rc;
    b->getClientRect(&rc);
    r = rc.right-rc.left;
  } else if (b && !b->isInited()) {
    go->guiobject_getGuiPosition(NULL, NULL, &r, NULL, NULL, NULL, NULL, NULL);
  }
  return MAKE_SCRIPT_INT(r);
}

scriptVar GuiObject_ScriptMethods::getHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  ifc_window *b = go->guiobject_getRootWnd();
  int r=0;
  if (b && b->isInited()) {
    RECT rc;
    b->getClientRect(&rc);
    r = rc.bottom-rc.top;
  } else if (b && !b->isInited()) {
    go->guiobject_getGuiPosition(NULL, NULL, NULL, &r, NULL, NULL, NULL, NULL);
  }
  return MAKE_SCRIPT_INT(r);
}

scriptVar GuiObject_ScriptMethods::setTargetX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&x));
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setTargetX(GET_SCRIPT_INT(x));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::setTargetY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&y));
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setTargetY(GET_SCRIPT_INT(y));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::setTargetW(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar w) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&w));
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setTargetW(GET_SCRIPT_INT(w));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::setTargetH(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar h) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&h));
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setTargetH(GET_SCRIPT_INT(h));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::setTargetA(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&a));
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setTargetA(GET_SCRIPT_INT(a));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::setTargetSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(SOM::isNumeric(&s));
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setTargetSpeed(GET_SCRIPT_FLOAT(s));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::gotoTarget(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_gotoTarget();
  RETURN_SCRIPT_VOID;
} 

scriptVar GuiObject_ScriptMethods::bringToFront(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_bringToFront();
  RETURN_SCRIPT_VOID;
} 

scriptVar GuiObject_ScriptMethods::bringToBack(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_bringToBack();
  RETURN_SCRIPT_VOID;
} 

scriptVar GuiObject_ScriptMethods::bringAbove(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(obj.data.odata != NULL);
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_bringAbove((static_cast<GuiObject *>(GET_SCRIPT_OBJECT_AS(obj, guiObjectGuid))));
  RETURN_SCRIPT_VOID;
} 

scriptVar GuiObject_ScriptMethods::bringBelow(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(obj.data.odata != NULL);
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_bringBelow((static_cast<GuiObject *>(GET_SCRIPT_OBJECT_AS(obj, guiObjectGuid))));
  RETURN_SCRIPT_VOID;
} 


scriptVar GuiObject_ScriptMethods::onTargetReached(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, guiController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar GuiObject_ScriptMethods::setXmlParam(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar param, scriptVar value) 
{
  SCRIPT_FUNCTION_INIT; 
  XmlObject *x = static_cast<XmlObject*>(o->vcpu_getInterface(xmlObjectGuid));
  if (x) 
		x->setXmlParam(GET_SCRIPT_STRING(param), GET_SCRIPT_STRING(value));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::getXmlParam(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar param) {
  SCRIPT_FUNCTION_INIT; 
  const wchar_t *rt = NULL;
  XmlObject *x = static_cast<XmlObject*>(o->vcpu_getInterface(xmlObjectGuid));
  if (x) {
    int r = x->getXmlParam(GET_SCRIPT_STRING(param));
    if (r != -1)
      rt = x->getXmlParamValue(r);
  }
  if (rt == NULL)
		rt = L"";  // returning null in a string is kinda bad, y'know?
	
  return MAKE_SCRIPT_STRING(rt);
}

scriptVar GuiObject_ScriptMethods::onStartup(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, guiController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar GuiObject_ScriptMethods::getGuiX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_getGuiPosition(&v, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::getGuiY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_getGuiPosition(NULL, &v, NULL, NULL, NULL, NULL, NULL, NULL);
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::getGuiW(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_getGuiPosition(NULL, NULL, &v, NULL, NULL, NULL, NULL, NULL);
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::getGuiH(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_getGuiPosition(NULL, NULL, NULL, &v, NULL, NULL, NULL, NULL);
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::getGuiRelatX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_getGuiPosition(NULL, NULL, NULL, NULL, &v, NULL, NULL, NULL);
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::getGuiRelatY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_getGuiPosition(NULL, NULL, NULL, NULL, NULL, &v, NULL, NULL);
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::getGuiRelatW(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_getGuiPosition(NULL, NULL, NULL, NULL, NULL, NULL, &v, NULL);
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::getGuiRelatH(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_getGuiPosition(NULL, NULL, NULL, NULL, NULL, NULL, NULL, &v);
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::clientToScreenX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w != NULL) {
      v = GET_SCRIPT_INT(x);
      w->clientToScreen(&v, NULL);
    }
  }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::clientToScreenY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w != NULL) {
      v = GET_SCRIPT_INT(y);
      w->clientToScreen(NULL, &v);
    }
  }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::clientToScreenW(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar w) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *wn = g->guiobject_getRootWnd();
    if (wn != NULL) {
      v = GET_SCRIPT_INT(w);
      double rr = wn->getRenderRatio();
      v = (int)((double)(v) * rr);
    }
  }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::clientToScreenH(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar h) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w != NULL) {
      v = GET_SCRIPT_INT(h);
      double rr = w->getRenderRatio();
      v = (int)((double)(v) * rr);
    }
  }
  return MAKE_SCRIPT_INT(v);
}


scriptVar GuiObject_ScriptMethods::screenToClientX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w != NULL) {
      v = GET_SCRIPT_INT(x);
      w->screenToClient(&v, NULL);
    }
  }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::screenToClientY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w != NULL) {
      v = GET_SCRIPT_INT(y);
      w->screenToClient(NULL, &v);
    }
  }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::screenToClientW(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar w) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *wn = g->guiobject_getRootWnd();
    if (wn != NULL) {
      v = GET_SCRIPT_INT(w);
      double rr = wn->getRenderRatio();
      v = (int)((double)(v) / rr);
    }
  }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::screenToClientH(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar h) {
  SCRIPT_FUNCTION_INIT; 
  int v=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w != NULL) {
      v = GET_SCRIPT_INT(h);
      double rr = w->getRenderRatio();
      v = (int)((double)(v) / rr);
    }
  }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::isActive(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g)  {
    ifc_window *w = (static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid)))->guiobject_getRootWnd();
    if (w) return MAKE_SCRIPT_INT(w->isActive());
  }
  RETURN_SCRIPT_ZERO;
}

scriptVar GuiObject_ScriptMethods::getParent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = (static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid)));
  if (go) {
    GuiObject *g = go->guiobject_getParent();
    ScriptObject *so = NULL;
    if (g != NULL)
      so = g->guiobject_getScriptObject();
    return MAKE_SCRIPT_OBJECT(so);
  }
  RETURN_SCRIPT_VOID;
}

#ifdef WASABI_COMPILE_WNDMGR
scriptVar GuiObject_ScriptMethods::getParentLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = (static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid)));
  if (go) {
    Layout *l = go->guiobject_getParentLayout();
    return MAKE_SCRIPT_OBJECT(l ? l->getGuiObject()->guiobject_getScriptObject() : NULL);
  }
  RETURN_SCRIPT_VOID;
}
#endif

scriptVar GuiObject_ScriptMethods::getTopParent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = (static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid)));
  if (go) {
    GuiObject *l = go->guiobject_getTopParent();
    return MAKE_SCRIPT_OBJECT(l ? l->guiobject_getScriptObject() : NULL);
  }
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::getAutoWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = (static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid)));
  int v = 0;
  if (go) { v = go->guiobject_getRootWnd()->getPreferences(SUGGESTED_W); if (v == AUTOWH) v = go->guiobject_getAutoWidth(); }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::getAutoHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *go = (static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid)));
  int v = 0;
  if (go) { v = go->guiobject_getRootWnd()->getPreferences(SUGGESTED_H); if (v == AUTOWH) v = go->guiobject_getAutoHeight(); }
  return MAKE_SCRIPT_INT(v);
}

scriptVar GuiObject_ScriptMethods::init(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar parentGroup) {
  SCRIPT_FUNCTION_INIT; 
  ASSERT(parentGroup.data.odata != NULL);
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    Group *pg = static_cast<Group *>(GET_SCRIPT_OBJECT_AS(parentGroup, groupGuid));
    pg->addChild(g);
    if (!g->guiobject_getRootWnd()->isInited())
      g->guiobject_getRootWnd()->init(pg);
  }
  RETURN_SCRIPT_VOID;
}

#ifdef WASABI_COMPILE_WNDMGR
scriptVar GuiObject_ScriptMethods::runModal(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int r=0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) r = g->guiobject_runModal();
  return MAKE_SCRIPT_INT(r);
}

scriptVar GuiObject_ScriptMethods::endModal(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar retcode) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_endModal(GET_SCRIPT_INT(retcode));
  RETURN_SCRIPT_VOID;
}
#endif

scriptVar GuiObject_ScriptMethods::setFocus(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) {
    ifc_window *w = g->guiobject_getRootWnd();
    if (w != NULL) {
      w->setFocus();
    }
  }
  RETURN_SCRIPT_VOID;
}

#ifdef WASABI_COMPILE_WNDMGR
scriptVar GuiObject_ScriptMethods::popParentLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_popParentLayout();
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::setStatusText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar text, scriptVar overlay) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g)
		g->guiobject_setStatusText(GET_SCRIPT_STRING(text), GET_SCRIPT_INT(overlay));
  RETURN_SCRIPT_VOID;
}
#endif

scriptVar GuiObject_ScriptMethods::findObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar id) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  ScriptObject *s = NULL;
  if (g) {
    GuiObject *o = g->guiobject_findObject(GET_SCRIPT_STRING(id));
    if (o != NULL) s = o->guiobject_getScriptObject();
  }
  return MAKE_SCRIPT_OBJECT(s);
}

scriptVar GuiObject_ScriptMethods::findObjectXY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  ScriptObject *s = NULL;
  if (g) {
    GuiObject *o = g->guiobject_findObjectXY(GET_SCRIPT_INT(x), GET_SCRIPT_INT(y));
    if (o != NULL) s = o->guiobject_getScriptObject();
  }
  return MAKE_SCRIPT_OBJECT(s);
}

scriptVar GuiObject_ScriptMethods::getName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) 
{
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) 
		return MAKE_SCRIPT_STRING(g->guiobject_getName());

  return MAKE_SCRIPT_STRING(L"");
}

scriptVar GuiObject_ScriptMethods::getMover(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  int r = 0;
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) r = g->guiobject_getMover();
  return MAKE_SCRIPT_INT(r);
}

scriptVar GuiObject_ScriptMethods::setMover(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i) {
  SCRIPT_FUNCTION_INIT; 
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  if (g) g->guiobject_setMover(GET_SCRIPT_INT(i));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::setDropTarget(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar targ) {
  GuiObject *g = static_cast<GuiObject*>(o->vcpu_getInterface(guiObjectGuid));
  const wchar_t *s = GET_SCRIPT_STRING(targ);
  if (g)
		g->guiobject_setDropTarget(s);
  RETURN_SCRIPT_VOID;
}

#ifdef WASABI_COMPILE_CONFIG
scriptVar GuiObject_ScriptMethods::cfgGetInt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  if (go) return MAKE_SCRIPT_INT(go->guiobject_getCfgInt());
  return MAKE_SCRIPT_INT(0);
}

scriptVar GuiObject_ScriptMethods::cfgSetInt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT
  ASSERT(SOM::isNumeric(&v));
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  if (go) go->guiobject_setCfgInt(GET_SCRIPT_INT(v));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::cfgGetString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));

  if (go)
		return MAKE_SCRIPT_STRING(go->guiobject_getCfgString());

  return MAKE_SCRIPT_STRING(L"");
}

scriptVar GuiObject_ScriptMethods::cfgSetString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  if (go) go->guiobject_setCfgString(GET_SCRIPT_STRING(v));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::cfgGetFloat(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  if (go) return MAKE_SCRIPT_FLOAT(go->guiobject_getCfgFloat());
  return MAKE_SCRIPT_FLOAT(0);
}

scriptVar GuiObject_ScriptMethods::cfgSetFloat(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  if (go) go->guiobject_setCfgFloat(GET_SCRIPT_FLOAT(v));
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::cfgGetGuid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  if (go) {
    CfgItem *i = go->guiobject_getCfgItem();
    if (i != NULL) 
		{
      GUID guid = i->getGuid();
      nsGUID::toCharW(guid, txt);
      return MAKE_SCRIPT_STRING(txt);
    }
  }
  return MAKE_SCRIPT_STRING(L"");
}

scriptVar GuiObject_ScriptMethods::cfgGetAttributeName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));

  if (go) 
    return MAKE_SCRIPT_STRING(go->guiobject_getCfgAttrib());

  return MAKE_SCRIPT_STRING(L"");
}
#endif

scriptVar GuiObject_ScriptMethods::isMouseOverRect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  int r = 0;
  if (go) {
    ifc_window *w = go->guiobject_getRootWnd();
    if (w != NULL) {
      RECT rc;
      int x, y;
      w->getWindowRect(&rc);
      Wasabi::Std::getMousePos(&x, &y);
      if (x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom) r = 1;
    }
  }
  return MAKE_SCRIPT_BOOLEAN(r);
}

#ifdef WASABI_COMPILE_CONFIG
scriptVar GuiObject_ScriptMethods::onCfgChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, guiController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}
#endif

scriptVar GuiObject_ScriptMethods::onChar(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar c) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, guiController, c);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, c);
}

scriptVar GuiObject_ScriptMethods::onKeyDown(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar c) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, guiController, c);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, c);
}

scriptVar GuiObject_ScriptMethods::onKeyUp(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar c) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, guiController, c);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, c);
}

scriptVar GuiObject_ScriptMethods::onGetFocus(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, guiController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar GuiObject_ScriptMethods::onKillFocus(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS0(o, guiController);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT0(o);
}

scriptVar GuiObject_ScriptMethods::sendAction(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar action, scriptVar param, scriptVar x, scriptVar y, scriptVar p1, scriptVar p2) {
  SCRIPT_FUNCTION_INIT
  GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
  int a = 0;
  if (go) {
    ifc_window *w = go->guiobject_getRootWnd();
    if (w!=NULL) {
      a = w->onAction(GET_SCRIPT_STRING(action), GET_SCRIPT_STRING(param), GET_SCRIPT_INT(x), GET_SCRIPT_INT(y), GET_SCRIPT_INT(p1), GET_SCRIPT_INT(p2));
    }
  }
  return MAKE_SCRIPT_INT(a);
}

scriptVar GuiObject_ScriptMethods::onAccelerator(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar accel) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS1(o, guiController, accel);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT1(o, accel);
}

scriptVar GuiObject_ScriptMethods::onAction(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar action, scriptVar param, scriptVar x, scriptVar y, scriptVar p1, scriptVar p2, scriptVar source) {
  SCRIPT_FUNCTION_INIT; 
  PROCESS_HOOKS7(o, guiController, action, param, x, y, p1, p2, source);
  SCRIPT_FUNCTION_CHECKABORTEVENT;
  SCRIPT_EXEC_EVENT7(o, action, param, x, y, p1, p2, source);
}

#ifdef WASABI_COMPILE_CONFIG
int GuiObjectI::guiobject_getCfgInt() 
{
  if (!cfgitem) 
		return 0;
  return 
		cfgitem->getDataAsInt(cfgattrname);
}

void GuiObjectI::guiobject_setCfgInt(int i) {
  if (!cfgitem) return;
  cfgitem->setDataAsInt(cfgattrname, i);
}

float GuiObjectI::guiobject_getCfgFloat() {
  if (!cfgitem) return 0;
  return (float)cfgitem->getDataAsFloat(cfgattrname);
}

void GuiObjectI::guiobject_setCfgFloat(float f) {
  if (!cfgitem) return;
  cfgitem->setDataAsFloat(cfgattrname, f);
}

const wchar_t *GuiObjectI::guiobject_getCfgString()
{
	static StringW blah;
  if (!cfgitem) return 0;
  cfgitem->getData(cfgattrname, txt, 4096); 
  return txt;
}

void GuiObjectI::guiobject_setCfgString(const wchar_t *s) 
{
  if (!cfgitem) 
		return;
  cfgitem->setData(cfgattrname, s);
}

void GuiObjectI::setCfgAttr(const wchar_t *strvalue)
{
  ParamParser pp(strvalue);
  if (pp.getNumItems() < 2) return;

  GUID g = nsGUID::fromCharW(pp.enumItem(0));
  if (g == INVALID_GUID) return;
  CfgItem *i = WASABI_API_CONFIG->config_getCfgItemByGuid(g);
  if (i == NULL) return;
  cfgattrname = pp.enumItem(1);
  guiobject_setCfgAttrib(i, cfgattrname);
}

int GuiObjectI::guiobject_hasCfgAttrib() {
  return (guiobject_getCfgItem() && guiobject_getCfgAttrib());
}
#endif

GuiObject *GuiObjectI::guiobject_findObject(const wchar_t *id) 
{
  ifc_window *me = guiobject_getRootWnd();
  ifc_window *w = me->findWindow(id);
  if (w != NULL) return w->getGuiObject();
  return NULL;
}

GuiObject *GuiObjectI::guiobject_findObjectXY(int x, int y) {
  ifc_window *me = guiobject_getRootWnd();
  POINT pt={x,y};
  me->clientToScreen((int *)&pt.x, (int *)&pt.y);
  ifc_window *w = WASABI_API_WND->rootWndFromPoint(&pt);
  if (w != NULL) return w->getGuiObject();
  return NULL;
}

GuiObject *GuiObjectI::guiobject_findObjectByInterface(GUID interface_guid) {
  ifc_window *me = guiobject_getRootWnd();
  ifc_window *w = me->findWindowByInterface(interface_guid);
  if (w != NULL) return w->getGuiObject();
  return NULL;
}

GuiObject *GuiObjectI::guiobject_findObjectByCallback(FindObjectCallback *cb) {
  ifc_window *me = guiobject_getRootWnd();
  ifc_window *w = me->findWindowByCallback(cb);
  if (w != NULL) return w->getGuiObject();
  return NULL;
}

void GuiObjectI::guiobject_onAccelerator(const wchar_t *accel) 
{
  GuiObject_ScriptMethods::onAccelerator(SCRIPT_CALL, guiobject_getScriptObject(), MAKE_SCRIPT_STRING(accel));
}

int GuiObjectI::guiobject_onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) 
{
  GuiObject *gsourceobj = (source == NULL) ? NULL : source->getGuiObject();
  ScriptObject *sourceobj = (gsourceobj == NULL) ? NULL : gsourceobj->guiobject_getScriptObject();
  GuiObject_ScriptMethods::onAction(SCRIPT_CALL, guiobject_getScriptObject(), MAKE_SCRIPT_STRING(action), MAKE_SCRIPT_STRING(param), MAKE_SCRIPT_INT(x), MAKE_SCRIPT_INT(y), MAKE_SCRIPT_INT(p1), MAKE_SCRIPT_INT(p2), MAKE_SCRIPT_OBJECT(sourceobj));
  return 1;
}

void GuiObjectI::guiobject_setTabOrder(int a) {
  ifc_window *me = guiobject_getRootWnd();
  me->setTabOrder(a);
}

void GuiObjectI::guiobject_onInit() {
  // api_window *me = guiobject_getRootWnd();
  // nothing to do here anymore, for now
  return;
}

int GuiObjectI::guiobject_wantFocus() {
  return wantfocus;
}

void GuiObjectI::guiobject_setNoDoubleClick(int no) {
  ifc_window *w = guiobject_getRootWnd();
  if (w != NULL) 
    w->setNoDoubleClicks(no);
}

void GuiObjectI::guiobject_setNoLeftClick(int no) {
  ifc_window *w = guiobject_getRootWnd();
  if (w != NULL) 
    w->setNoLeftClicks(no);
}

void GuiObjectI::guiobject_setNoRightClick(int no) {
  ifc_window *w = guiobject_getRootWnd();
  if (w != NULL) 
    w->setNoRightClicks(no);
}

void GuiObjectI::guiobject_setNoMouseMove(int no) {
  ifc_window *w = guiobject_getRootWnd();
  if (w != NULL) 
    w->setNoMouseMoves(no);
}

void GuiObjectI::guiobject_setNoContextMenu(int no) {
  ifc_window *w = guiobject_getRootWnd();
  if (w != NULL) 
    w->setNoContextMenus(no);
}

scriptVar GuiObject_ScriptMethods::getInterface(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar guid) {
  SCRIPT_FUNCTION_INIT; 
  int type=-1;
  GUID _g = nsGUID::fromCharW(GET_SCRIPT_STRING(guid));
  void *i = o->vcpu_getInterface(_g, &type);
  if (i != NULL && type == INTERFACE_SCRIPTOBJECT) {
    return MAKE_SCRIPT_OBJECT(reinterpret_cast<ScriptObject*>(i));
  }
  RETURN_SCRIPT_VOID;
}

scriptVar GuiObject_ScriptMethods::onDragEnter(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT; 
	PROCESS_HOOKS0(o, guiController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar GuiObject_ScriptMethods::onDragOver(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y)
{
	SCRIPT_FUNCTION_INIT; 
	PROCESS_HOOKS2(o, guiController, x, y);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT2(o, x, y);
}

scriptVar GuiObject_ScriptMethods::onDragLeave(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT; 
	PROCESS_HOOKS0(o, guiController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}
