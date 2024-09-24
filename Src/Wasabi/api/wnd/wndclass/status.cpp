#include <precomp.h>
#include "status.h"

#include <tataki/color/skinclr.h>
#include <tataki/canvas/canvas.h>

#include <api/wnd/wndclass/buttbar.h>
#include <api/wnd/wndclass/buttwnd.h>
#include <api/wndmgr/appcmds.h>
#include <bfc/parse/paramparser.h>

#include <api/script/objects/c_script/c_text.h>

#define STATUS_TIMER_DECAY 1

#define COMPLETED_WIDTH 96

static SkinColor textcolor(L"wasabi.statusbar.text");

class CmdButton : public ButtonWnd 
{
public:
  CmdButton(const wchar_t *name, AppCmds *_cmd, int _id) : ButtonWnd(name), cmd(_cmd), id(_id) {}

  virtual void onLeftPush(int x, int y) {
    cmd->appcmds_onCommand(id, &windowRect(), AppCmds::LEFT_CLICK);
  }

  virtual void onRightPush(int x, int y) {
    cmd->appcmds_onCommand(id, &windowRect(), AppCmds::RIGHT_CLICK);
  }

  virtual int wantAutoContextMenu() { return 0; }

  AppCmds *cmd;
  int id;
};

StatusBar::StatusBar() {
  overtimer = 0;
  max = 0;
  completed = 0;
  progress_width = 0;
  bg.setContent(L"wasabi.statusbar");
  bbleft = bbright = NULL;
}

StatusBar::~StatusBar() 
{
  killTimer(STATUS_TIMER_DECAY);
  delete bbleft;
  delete bbright;
}

int StatusBar::onInit() {
  STATUSBAR_PARENT::onInit();

  bg.init(this);

  #ifdef WASABI_COMPILE_WNDMGR
  getGuiObject()->guiobject_registerStatusCB(this); // watched
  #endif

  regenerate();

  return 1;
}

void StatusBar::timerCallback(int id) {
  switch (id) {
    case STATUS_TIMER_DECAY: {
      killTimer(STATUS_TIMER_DECAY);
      onSetStatusText(status_text, FALSE);	// revert to main text
    }
    break;
    default:
      STATUSBAR_PARENT::timerCallback(id);
      break;
  }
}

void StatusBar::pushCompleted(int _max) {
  max = MAX(_max, 0);
  completed = 0;

  GuiObject *outer = bg.findObject(L"wasabi.statusbar.progress.outline");
  outer->guiobject_setXmlParam(L"visible", L"0");
  ASSERT(outer != NULL);
  ifc_window *outerw = outer->guiobject_getRootWnd();
  RECT cr;
  outerw->getClientRect(&cr);
  progress_width = cr.right - cr.left;

  outerw->setVisible(TRUE);//CUT
  outer->guiobject_setTargetA(255);
  outer->guiobject_setTargetSpeed(0.1f);
  outer->guiobject_gotoTarget();

  GuiObject *inner = bg.findObject(L"wasabi.statusbar.progress.inside");
  inner->guiobject_setTargetA(255);
  inner->guiobject_setTargetSpeed(1.0f);
  inner->guiobject_gotoTarget();
  inner->guiobject_setXmlParam(L"visible", L"0");

  incCompleted(0);
}

void StatusBar::incCompleted(int add) {
  setCompleted(completed + add);
}

void StatusBar::setCompleted(int _completed) {
  completed = _completed;
  GuiObject *inner = bg.findObject(L"wasabi.statusbar.progress.inside");
  ASSERT(inner != NULL);
  if (!inner->guiobject_getRootWnd()->isVisible(1)) {
    inner->guiobject_setXmlParam(L"visible", L"1");
    inner->guiobject_setTargetA(255);
    inner->guiobject_setTargetSpeed(0.75);
    inner->guiobject_gotoTarget();
  }
  int pos = (int)(((float)completed / (float)max)*(float)progress_width);
  inner->guiobject_setXmlParam(L"w", StringPrintfW(L"%d", pos));
}

void StatusBar::popCompleted() {
  completed = 0;
  max = 0;
  GuiObject *inner = bg.findObject(L"wasabi.statusbar.progress.inside");
  inner->guiobject_setXmlParam(L"w", L"0");
  inner->guiobject_setTargetA(0);
  inner->guiobject_setTargetSpeed(0.75);
  inner->guiobject_gotoTarget();

//CUT later
  inner->guiobject_setXmlParam(L"visible", L"0");
  GuiObject *outer = bg.findObject(L"wasabi.statusbar.progress.outline");
  outer->guiobject_setXmlParam(L"visible", L"0");
}

int StatusBar::onResize() {
  STATUSBAR_PARENT::onResize();

  RECT cr = clientRect();

  bbleft->resize(cr.left, cr.top, bbleft->getWidth(), cr.bottom - cr.top);

  bbright->resize(cr.right-bbright->getWidth(), cr.top, bbright->getWidth(), cr.bottom - cr.top);

  cr.left += bbleft->getWidth();
  cr.right -= bbright->getWidth();

  bg.resizeToRect(&cr);	// put bg group in place

  invalidate();
  return 1;
}

void StatusBar::onSetStatusText(const wchar_t *text, int overlay) 
{
  killTimer(STATUS_TIMER_DECAY);
  if (!overlay) 
		status_text = text;
  else setTimer(STATUS_TIMER_DECAY, 4000);
  ScriptObject *tx = bg.findScriptObject(L"wasabi.statusbar.text");
  if (tx == NULL) return;
  C_Text(tx).setText(text ? text : L"");
}

void StatusBar::onAddAppCmds(AppCmds *commands) {
  if (appcmds.haveItem(commands)) appcmds.removeItem(commands);
  appcmds.addItem(commands);
  regenerate();
}

void StatusBar::onRemoveAppCmds(AppCmds *commands) {
  if (appcmds.haveItem(commands)) {
    appcmds.removeItem(commands);
    regenerate();
  }
}

void StatusBar::regenerate() {
  if (!isInited()) return;

  delete bbleft; bbleft = new ButtBar;
  delete bbright; bbright = new ButtBar;
  bbleft->init(this);
  bbright->init(this);

  ParamParser exclude(exclude_list, L";");
  ParamParser showonly(include_only, L";");

  foreach(appcmds)
    int n = appcmds.getfor()->appcmds_getNumCmds();
    for (int i = 0; i < n; i++) {
      int side, id;
      const wchar_t *name = appcmds.getfor()->appcmds_enumCmd(i, &side, &id);
      if (name == NULL) break;
      if (exclude.hasString(name)) continue;	// exclusion list
      if (showonly.getNumItems()) {
        if (!showonly.hasString(name)) continue;	// include-only list
      }
      CmdButton *cb = new CmdButton(name, appcmds.getfor(), id);
//      cb->setXmlParam("wantfocus", "1");
      if (side == AppCmds::SIDE_LEFT) bbleft->addChild(cb);
      else bbright->addChild(cb);
    }
  endfor
  if (isPostOnInit())
    onResize();
}

void StatusBar::fakeButtonPush(const wchar_t *name) {
  if (!fakeButtonPush(bbleft, name))
    fakeButtonPush(bbright, name);
}

int StatusBar::fakeButtonPush(ButtBar *bb, const wchar_t *name) 
{
  for (int i = 0; i < bb->getNumChildren(); i++) {
    ButtonWnd *cmdb = bb->enumChild(i);
    if (!WCSICMP(cmdb->getName(), name)) {
      int x, y;
      Wasabi::Std::getMousePos(&x, &y);
      cmdb->screenToClient(&x, &y);
      cmdb->onLeftPush(x, y);
      return 1;
    }
  }
  return 0;
}

void StatusBar::setExclude(const wchar_t *val) {
  exclude_list = val;
  regenerate();
}

void StatusBar::setIncludeOnly(const wchar_t *val) {
  include_only = val;
  regenerate();
}
