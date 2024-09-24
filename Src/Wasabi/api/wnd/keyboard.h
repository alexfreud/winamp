#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <bfc/string/StringW.h>
#include <bfc/ptrlist.h>
#include <bfc/depview.h>

#ifdef WIN32
#define MAX_KEY 256
#else
#define MAX_KEY 65536
#endif

class ifc_window;

class AccSec {
public:
  AccSec(const wchar_t *pname, ifc_window *pwnd, int pglobal=0) : name(pname), wnd(pwnd), global(pglobal) { }
  StringW name;
  ifc_window *wnd;
  int global;
};

#include <api/wnd/api_window.h>
class AccSecViewer : public DependentViewerTPtr<ifc_window> {
public:
  void viewItem(ifc_window *i) { viewer_addViewItem(i); }
  virtual int viewer_onItemDeleted(ifc_window *item);
};

class Keyboard {

public:

  static int onForwardOnChar(ifc_window *from, unsigned int c, int kd);
  static int onForwardOnKeyDown(ifc_window *from, int k, int kd, int nomsg=0);
  static int onForwardOnKeyUp(ifc_window *from, int k, int kd);
  static int onForwardOnSysKeyDown(ifc_window *from, int k, int kd);
  static int onForwardOnSysKeyUp(ifc_window *from, int k, int kd);
  static int onForwardOnKillFocus();

  static int interceptOnChar(unsigned int c);
  static int interceptOnKeyDown(int k);
  static int interceptOnKeyUp(int k);
  static int interceptOnSysKeyDown(int k, int kd);
  static int interceptOnSysKeyUp(int k, int kd);

  static void hookKeyboard(ifc_window *hooker);
  static void unhookKeyboard(ifc_window *hooker);

  static void reset();
  
  static void registerAcceleratorSection(const wchar_t *name, ifc_window *wnd, int pglobal);

  static PtrList<AccSec> accSecEntries;

private:
  static int forwardKbdMessage(ifc_window *from, int msg, int wp, int lp);
  static wchar_t *getVkName(int vkey);
  static void syncKeyTable();

  // special keys
  typedef struct {
    int vk;
    wchar_t *trans;
  } vkEntry;
  static vkEntry vkEntries[];

  static wchar_t pressedKeys[MAX_KEY];

  static AccSecViewer viewer;
  static PtrList<ifc_window> hookers;
  static int infw;
  static int lastwasreset;
};

#endif
