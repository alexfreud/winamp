#ifndef __WNDTRACK_H
#define __WNDTRACK_H

#include "cwndtrack.h"
#include <bfc/tlist.h>
#include <bfc/ptrlist.h>
#include <api/wndmgr/layout.h>

class ifc_window;

#define WNDTRACKCB_POPUPEXIT 0x8797
#define WNDTRACKCB_POPUPEXITALL 0x8798

const int DEFAULT_DOCK_DIST=10;
const int MIN_DOCK_DIST=1;
const int MAX_DOCK_DIST=64;

class coopEntry {
  public:
    coopEntry(ifc_window *w, int lock=0) : wnd(w), locked(lock) {}
    ~coopEntry() {}

    ifc_window *wnd;
    int locked;
};

#ifndef _REDOCK_STRUCT_DEFINED
#define _REDOCK_STRUCT_DEFINED
typedef struct {
  Layout *l;
  RECT original_rect;
} redock_struct;
#endif

class WindowTracker {
public:
  WindowTracker();
  ~WindowTracker();

  void addWindow(ifc_window *wnd);
  void removeWindow(ifc_window *wnd);
  int checkWindow(ifc_window *wnd);
  ifc_window *enumWindows(int n);
  int getNumWindows();
  ifc_window *enumAllWindows(int n);
  int getNumAllWindows();
	bool autoDock(ifc_window *thiswnd, RECT *newPosition, int mask);
	bool autoDock(ifc_window *thiswnd, RECT *newPosition, RECT *oldPosition, int mask);
  static bool touches(const RECT &z, const RECT &r);

  RECT findOpenRect(const RECT &prev, ifc_window *exclude=NULL);

  static void setDockDistance(int dd);
  static int getDockDistance();
  static void setEnableDocking(int ed);

	void startCooperativeMove(ifc_window *wnd);
	void endCooperativeMove();
	int wasCooperativeMove();
  void invalidateAllWindows();
  int getNumDocked();
  ifc_window *enumDocked(int n);

	static void addRootWnd(ifc_window *wnd);
	static void removeRootWnd(ifc_window *wnd);
	static ifc_window *rootWndFromPoint(POINT *pt);
  static ifc_window *rootWndFromHwnd(OSWINDOWHANDLE h);
  static void layoutChanged(Layout *previouswnd, Layout *newwnd); // re-dock windows when changing layout

  static void beforeRedock(Layout *l, redock_struct *rs);
  static void afterRedock(Layout *l, redock_struct *rs);
  static ifc_window *getNextDesktopWindow(ifc_window *w, int next);
  static void snapAdjustWindowRect(ifc_window *w, RECT *r, RECT *adjustvals=NULL);
  static void unsnapAdjustWindowRect(ifc_window *w, RECT *r, RECT *adjustvals=NULL);

private:
	void addCooperative(ifc_window *thiswnd);
  void addCoopWnd(ifc_window *w, int forced=0);
  int hasCoopWnd(ifc_window *w);
  void flushCoopWnds();
  static void recursAddToMoveWindows(ifc_window *wnd, redock_struct *rs, int v=-1); // used by layoutChanged()

  static PtrList<ifc_window> desktopwnds;
  static PtrList<ifc_window> nonvirtuals;
  static PtrList<coopEntry> coopList;
  static PtrList<ifc_window> recursList;
  static ifc_window *coopWnd;
  static PtrList<ifc_window> allWnd;
  static PtrList<ifc_window> tomoveWindows_top;
  static PtrList<ifc_window> tomoveWindows_left;
  static PtrList<ifc_window> tomoveWindows_bottom;
  static PtrList<ifc_window> tomoveWindows_right;

  static int dockDist;
  static int dock_enabled;
  int wascoop;
  int disabledock;
  ifc_window *coopcachewnd;
  int coopcache;
};

extern WindowTracker *windowTracker;

#endif
