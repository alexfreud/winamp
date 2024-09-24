#include <precomp.h>
#include "groupclickwnd.h"
#include <api/script/objects//guiobject.h>
#include <api/wnd/notifmsg.h>

GroupClickWnd::GroupClickWnd() {
  trap = NULL;
  inarea = 0;
}

GroupClickWnd::~GroupClickWnd() {
  delete trap;
}

void GroupClickWnd::abstract_onNewContent() {
  delete trap;
  trap = NULL;

  if (!abstract_getContent()) return;

  GuiObject *mousetrap = abstract_getContent()->guiobject_findObject(L"mousetrap");

  if (mousetrap != NULL)
    trap = new MouseTrap(this, mousetrap->guiobject_getScriptObject());
}

void GroupClickWnd::groupclick_onLeftPush() {
  notifyParent(ChildNotify::BUTTON_LEFTPUSH);
}

void GroupClickWnd::groupclick_onRightPush() {
  notifyParent(ChildNotify::BUTTON_RIGHTPUSH);
}

void GroupClickWnd::content_onLeftButtonDown() {
  notifyParent(ChildNotify::CLICKWND_LEFTDOWN);
}

void GroupClickWnd::content_onLeftButtonUp() {
  notifyParent(ChildNotify::CLICKWND_LEFTUP);
  if (inarea) groupclick_onLeftPush();
}

void GroupClickWnd::content_onRightButtonDown() {
  notifyParent(ChildNotify::CLICKWND_RIGHTDOWN);
}

void GroupClickWnd::content_onRightButtonUp() {
  notifyParent(ChildNotify::CLICKWND_RIGHTUP);
  if (inarea) groupclick_onRightPush();
}

void GroupClickWnd::content_onEnterArea() {
  inarea = 1;
}

void GroupClickWnd::content_onLeaveArea() {
  inarea = 0;
}


void MouseTrap::hook_onLeftButtonDown(int x, int y) { 
  window->content_onLeftButtonDown(); 
}

void MouseTrap::hook_onLeftButtonUp(int x, int y) { 
  window->content_onLeftButtonUp(); 
}

void MouseTrap::hook_onRightButtonDown(int x, int y) { 
  window->content_onRightButtonDown(); 
}

void MouseTrap::hook_onRightButtonUp(int x, int y) { 
  window->content_onRightButtonUp(); 
}

void MouseTrap::hook_onEnterArea() { 
  window->content_onEnterArea(); 
}

void MouseTrap::hook_onLeaveArea() { 
  window->content_onLeaveArea(); 
}
