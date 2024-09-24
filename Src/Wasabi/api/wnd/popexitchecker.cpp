#include <precomp.h>
#include "popexitchecker.h"
#include <api/wnd/popexitcb.h>
#include <api/wnd/api_window.h>
#include <api/syscb/callbacks/syscbi.h>
#include <api/wnd/wndtrack.h>

//PopupExitChecker *popupExitChecker;

PopupExitChecker::PopupExitChecker() {
}

PopupExitChecker::~PopupExitChecker() {
  watchers.deleteAll(); 
}

void PopupExitChecker::registerCallback(PopupExitCallback *cb, ifc_window *watched) {
  ifc_dependent *a = cb->popupexit_getDependencyPtr();
  ifc_dependent *b = watched->getDependencyPtr();
  watchers.addItem(new PopupExitCallbackEntry(cb, watched, a, b));
  viewer_addViewItem(a);
  viewer_addViewItem(b);
}

int PopupExitChecker::check(ifc_window *w) {
  int n=0;
  foreach(watchers)
    PopupExitCallbackEntry *e = watchers.getfor();
    if (w != e->watched && !isGrandChildren(e->watched, w)) {
      e->cb->popupexitcb_onExitPopup();
      n++;
    }
  endfor;
  return n > 0;
}

void PopupExitChecker::signal() {
  foreach(watchers)
    watchers.getfor()->cb->popupexitcb_onExitPopup();
  endfor;
}

int PopupExitChecker::isGrandChildren(ifc_window *parent, ifc_window *child) {
  int i;
  for (i=0;i<parent->getNumRootWndChildren();i++) {
    if (parent->enumRootWndChildren(i) == child) return 1;
  }

  for (i=0;i<parent->getNumRootWndChildren();i++) {
    ifc_window *w = parent->enumRootWndChildren(i);
    int r = 0;
    if (w->getNumRootWndChildren() > 0) r = isGrandChildren(w, child);
    if (r) return 1;
  }
  return 0;
}

int PopupExitChecker::viewer_onItemDeleted(ifc_dependent *item) {
  for (int i=0;i<watchers.getNumItems();i++) {
    PopupExitCallbackEntry *e = watchers.enumItem(i);
    if (e->cbd == item || e->wd == item) {
      watchers.removeByPos(i); i--;
      delete e;
      // no break, wnd can be watched by many people
    }
  }
  return 0;
}

void PopupExitChecker::deregisterCallback(PopupExitCallback *cb) {
  for (int i=0;i<watchers.getNumItems();i++) {
    PopupExitCallbackEntry *e = watchers.enumItem(i);
    if (e->cb == cb) {
      watchers.removeByPos(i); i--;
      delete e;
      // no break, watcher can watch many windows, but wait, there cannot be 2 popups at once on a screen! who says so ?
    }
  }
}
