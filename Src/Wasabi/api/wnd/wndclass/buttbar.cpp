#include "precomp.h"
//PORTABLE
#include <bfc/wasabi_std.h>
#include <api/wnd/wndclass/buttbar.h>
#include <api/wnd/notifmsg.h>
#include <api/wnd/wndclass/buttwnd.h>
#include <api/wnd/popup.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/script/objects/c_script/h_button.h>


class ButtHooker : public H_Button {
public:
  ButtHooker(ButtBar *hangout, ScriptObject *butt) : bb(hangout), H_Button(butt) { }

  void hook_onLeftClick() {
    bb->onLeftPush(0, 0);
  }

private:
  ButtBar *bb;
};

ButtBar::ButtBar(int resizemode) {
  spacer = 0;
  resize_mode = resizemode;
  hooker = NULL;
  if (resize_mode == STACK) {
    setContent(L"wasabi.buttonbar.stack");
  }
}

ButtBar::~ButtBar() {
  buttons.deleteAll();
  delete hooker;
}

void ButtBar::setResizeMode(int resizemode) {
  if (resize_mode == resizemode) return;
  resize_mode = resizemode;
  if (isPostOnInit()) onResize();
}

int ButtBar::onInit() {
  int i;

  BUTTBAR_PARENT::onInit();

  // create the buttons
  for (i = 0; i < buttons.getNumItems(); i++) {
    buttons[i]->init(this);
    if (resize_mode == STACK) {
      if (i != 0) buttons[i]->setVisible(FALSE);
      if (i == 0) setGroupLabel(buttons[i]->getButtonText());
    }
  }

  return 1;
}

int ButtBar::addChild(ButtonWnd *child) {
  buttons.addItem(child);
  if (isInited()) {
    child->init(this);
    child->setParent(this);
    onResize();
    if (buttons.getNumItems() == 1)
			setGroupLabel(child->getButtonText());
  }
  return 1;
}

int ButtBar::removeChild(ButtonWnd *child) {
  if (!buttons.haveItem(child)) return 0;
  if (isInited()) onResize();
  return 1;
}

int ButtBar::getNumChildren() {
  return buttons.getNumItems();
}

ButtonWnd *ButtBar::enumChild(int n) {
  return buttons[n];
}

int ButtBar::getWidth() {
  int w = 0;
  for (int i = 0; i < buttons.getNumItems(); i++) {
    w += buttons[i]->getWidth()+spacer;
  }
  return w;
}

int ButtBar::getHeight() {
  if (resize_mode == STACK) {
    ifc_window *rw = getContentRootWnd();
    return rw->getPreferences(SUGGESTED_H);
  } else {
    int h = 0;
    for (int i = 0; i < buttons.getNumItems(); i++) {
      h = MAX(h, buttons[i]->getHeight()+1);
    }
    return h;
  }
}

void ButtBar::onLeftPush(int x, int y) 
{
  if (resize_mode == STACK)
	{
    PopupMenu pop(this);
    foreach(buttons)
      pop.addCommand(buttons.getfor()->getButtonText(), foreach_index);
    endfor
    int r = pop.popAnchored();
    if (r >= 0) {
      buttons[r]->onLeftPush(0, 0);
      setGroupLabel(buttons[r]->getButtonText());
    }
  }
}

int ButtBar::childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2) {
  switch (msg) {
    case ChildNotify::BUTTON_LEFTPUSH: {
      int ret;
      if (ret = onLeftPush(child->getNotifyId())) {
         return ret;
      } else {
// This won't fit the current notification schema.
// We _must_ change it -- too many interfaces assume that the
// button notification is called back through the parent.
//      return notifyParent(msg, p1, p2);

// So, I made a new basewnd method passNotifyUp() to defer a notification
// to the current object's notification target.
        return passNotifyUp(child, msg, p1, p2);
      }
    }
    break;
  }
  return BUTTBAR_PARENT::childNotify(child, msg, p1, p2);
}

int ButtBar::onResize() {
  BUTTBAR_PARENT::onResize(); // calling your parent is good(tm) =)
  if (!isPostOnInit()) return 0; // that's just an optim, in case someone's dumb and calling us directly when it shouldnt
  switch (resize_mode) {
    case NORMAL: {
      RECT r = clientRect();
      int height = r.bottom - r.top;
      int x = r.left;
      for (int i = 0; i < buttons.getNumItems(); i++) {
        int w = buttons[i]->getWidth()+spacer;
        buttons[i]->resize(x, r.top, w, height);
        x += w;
        if (x > r.right) break;
      }
    }
    break;
    case STRETCH: {
      if (buttons.getNumItems() > 0) {
  	    RECT r = clientRect();
        int height = r.bottom - r.top;
        int w = (r.right - r.left) / buttons.getNumItems();
        int x = r.left;
        for (int i = 0; i < buttons.getNumItems(); i++) {
          if (i == buttons.getNumItems()-1) w = (r.right - r.left) - x;
          buttons[i]->resize(x, r.top, w, height);
          x += w;
        }
      }
    }
    break;
    case STACK:	// no point
    break;
  }

  return 1;
}

int ButtBar::onPaint(Canvas *canvas) {
  ASSERT(canvas != NULL);
  if (resize_mode != STACK) {
    BUTTBAR_PARENT::onPaint(canvas);
    renderBaseTexture(canvas, clientRect()); 
  }
  return 1;
}

void ButtBar::setGroupLabel(const wchar_t *l) {
  setName(l);
  onNewContent();
}

void ButtBar::onNewContent() 
{
  if (resize_mode != STACK) return;
  ScriptObject *text = findScriptObject(L"buttonbar.text");
  if (text == NULL) return;
  C_Text(text).setText(getNameSafe(L"no tabs"));

  // hook the clicks
  delete hooker;
  ScriptObject *mousetrap = findScriptObject(L"mousetrap");
  hooker = new ButtHooker(this, mousetrap);
}
