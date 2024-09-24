#include <precomp.h>
#include <bfc/ptrlist.h>
#include <api/wnd/basewnd.h>
#include <bfc/util/findopenrect.h>
#include <bfc/bfc_assert.h>
#include <api/wndmgr/resize.h>
#include <api/wnd/wndtrack.h>
#include <api/config/items/attrint.h>
#include <api/config/items/attrbool.h>
#include <bfc/wasabi_std_wnd.h>
#ifdef WASABI_COMPILE_WNDMGR
#include <api/wndmgr/snappnt.h>
#endif

#ifdef WASABI_COMPILE_SCRIPT
#include <api/script/scriptobj.h>
#include <api/script/scriptguid.h>
#include <api/wnd/wndclass/guiobjwnd.h> // for appbar define
#endif

#ifdef WASABI_COMPILE_WNDMGR
#include <api/wndmgr/layout.h>
#include <api/wnd/popexitcb.h>
#endif

#ifdef WASABI_COMPILE_SYSCB 
//#include <api/syscb/cbmgr.h>
#endif

WindowTracker *windowTracker;

WindowTracker::WindowTracker()
		: coopcache(0),
		  coopcachewnd(NULL)
{
	wascoop = 0;
	disabledock = 0;
	dock_enabled = 1;
}

WindowTracker::~WindowTracker()
{
	coopList.deleteAll();
}

void WindowTracker::addWindow(ifc_window *wnd)
{
	ASSERT(wnd);
	desktopwnds.addItem(wnd);
}

void WindowTracker::removeWindow(ifc_window *wnd)
{
	ASSERT(wnd);
	ASSERTPR(desktopwnds.haveItem(wnd), "removewindow on invalid wnd");
	desktopwnds.removeItem(wnd);
}

int WindowTracker::checkWindow(ifc_window *wnd)
{
	return allWnd.haveItem(wnd);
}

ifc_window *WindowTracker::enumWindows(int n)
{
	return desktopwnds.enumItem(n);
}

ifc_window *WindowTracker::getNextDesktopWindow(ifc_window *w, int next)
{
	ifc_window *nw = NULL;
	if (w == NULL) nw = desktopwnds.getFirst();
	else
	{
		w = w->getDesktopParent();
		int pos = desktopwnds.searchItem(w);
		if (pos == -1) nw = desktopwnds.getFirst();
		else
		{
			pos += next;
			if (pos > desktopwnds.getNumItems() - 1) pos = 0;
			if (pos == -1) pos = desktopwnds.getNumItems() - 1;
			nw = desktopwnds.enumItem(pos);
		}
	}
	if (nw == w) return w;
	if (!nw->isVisible()) return getNextDesktopWindow(nw, next);
	return nw;
}

ifc_window *WindowTracker::enumAllWindows(int n)
{
	return allWnd.enumItem(n);
}

int WindowTracker::getNumWindows()
{
	return desktopwnds.getNumItems();
}

int WindowTracker::getNumAllWindows()
{
	return allWnd.getNumItems();
}

void WindowTracker::invalidateAllWindows()
{
	for (int i = allWnd.getNumItems() - 1;i >= 0;i--)
	{
		ifc_window *w = allWnd[i];
		w->triggerEvent(TRIGGER_INVALIDATE);
		w->invalidate();
		if (!w->isVirtual()) continue;
		w->triggerEvent(TRIGGER_ONRESIZE);
	}
}

RECT WindowTracker::findOpenRect(const RECT &prev, ifc_window *exclude)
{
	POINT pp = { 0, 0 };
	//CUT  if (prev != NULL) {
	pp.x = prev.left;
	pp.y = prev.top;
	//CUT  }
	RECT vr;	// viewport rect
	Wasabi::Std::getViewport(&vr, &pp);

	// make a rect list
	PtrList<RECT> list;
	for (int i = 0; ; i++)
	{
		ifc_window *wnd = enumWindows(i);
		if (wnd == NULL) break;
		if (wnd == exclude) continue;
		if (!wnd->isPostOnInit() && !wnd->isVisible()) continue;
		RECT *r = new RECT;
		wnd->getWindowRect(r);
		snapAdjustWindowRect(wnd, r);
		list.addItem(r);
	}

	FindOpenRect fr;
	RECT ret = fr.find(vr, list, prev);
	list.deleteAll();
	return ret;
}

void WindowTracker::setDockDistance(int dd)
{
	dockDist = MINMAX(dd, MIN_DOCK_DIST, MAX_DOCK_DIST);
}

int WindowTracker::getDockDistance()
{
	if (dock_enabled) return dockDist;
	return 0;
}

void WindowTracker::setEnableDocking(int ed)
{
	dock_enabled = ed;
}

bool WindowTracker::touches(const RECT &r2, const RECT &r1)
{
	if (r2.left == r1.right || r2.right == r1.left || r2.right == r1.right || r2.left == r1.left)
	{
		if (r2.bottom >= r1.top && r2.top <= r1.bottom)
			return true;
	}
	if (r2.top == r1.bottom || r2.bottom == r1.top || r2.bottom == r1.bottom || r2.top == r1.top)
	{
		if (r2.right >= r1.left && r2.left <= r1.right)
			return true;
	}
	return false;
}

void WindowTracker::endCooperativeMove()
{
	wascoop = 1;
	flushCoopWnds();
	coopWnd = NULL;
	recursList.removeAll();
}

void WindowTracker::startCooperativeMove(ifc_window *thiswnd)
{
	coopWnd = thiswnd;
	wascoop = 1;
	flushCoopWnds();
	if (recursList.getNumItems() > 0) recursList.removeAll();
	addCooperative(thiswnd);
	foreach_reverse(recursList)
	// FG> we need to prevent windows from excessively activating our windows or focus is gonna blow up
	// thiswnd->bringToFront();
#ifdef WIN32
	SetWindowPos(recursList.getfor()->gethWnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
#else
	recursList.getfor()->bringToFront();
#endif
	endfor;
}

int WindowTracker::getNumDocked()
{
	return recursList.getNumItems();
}

ifc_window *WindowTracker::enumDocked(int n)
{
	return recursList.enumItem(n);
}

void WindowTracker::addCooperative(ifc_window *thiswnd)
{
	int i;
	RECT r;
	RECT thisr;
	bool forceall = false;

	if (Std::keyModifier(STDKEY_ALT))
	{
		forceall = TRUE;
	}

#ifdef WASABI_COMPILE_WNDMGR
	Layout *l = static_cast<Layout *>(thiswnd->getInterface(layoutGuid));
	if (l)
	{
		for (int i = 0; i < l->getNumLockedLayouts(); i++)
		{
			ifc_window *wnd = l->enumLockedLayout(i);
			addCoopWnd(wnd, 1);
			addCooperative(wnd);
		}
	}
#endif

	if (recursList.searchItem(thiswnd) != -1) return ;

	recursList.addItem(thiswnd);

	if (Std::keyModifier(STDKEY_SHIFT)) return ;

	thiswnd->getWindowRect(&thisr);
	snapAdjustWindowRect(thiswnd, &thisr);

	for (i = 0; i < desktopwnds.getNumItems(); i++)
	{
		ifc_window *wnd = desktopwnds.enumItem(i);
		if (!wnd->isVisible()) continue;
		if (hasCoopWnd(wnd)) continue;
		if (wnd == thiswnd) continue;
		Layout *l = (Layout*)wnd->getInterface(layoutGuid);
		if (l && (l->getNoDock()
#ifdef USEAPPBAR
		          || l->appbar_isDocked()
#endif
		         )) continue;
		wnd->getWindowRect(&r);
		snapAdjustWindowRect(wnd, &r);

#ifdef WASABI_COMPILE_WNDMGR
		int snap = SnapPoint::match(thiswnd, NULL, wnd, KEEPSIZE, NULL, NULL, 0, 0);
		if (forceall || snap || (touches(r, thisr) && !Wasabi::Std::rectIntersect(r, thisr)))
		{
#else
      if (forceall || (touches(r, thisr) && !Std::rectIntersect(r, thisr)))
		{
#endif
			addCoopWnd(wnd);
			addCooperative(wnd);
		}
	}
}

bool WindowTracker::autoDock(ifc_window *thishWnd, RECT *newPosition, int mask)
{
	return autoDock(thishWnd, newPosition, NULL, mask);
}

bool WindowTracker::autoDock(ifc_window *thiswnd, RECT *z, RECT *_oldPosition, int mask)
{
	int i = 0;
	RECT r = {0};
#ifdef WASABI_COMPILE_CONFIG
	extern _bool cfg_options_docking;
	extern _int cfg_options_dockingdistance;

	dockDist = cfg_options_dockingdistance;
	dock_enabled = cfg_options_docking;
#else
#warning check these values
	dockDist = 4;
	dock_enabled = 4;
#endif

#ifdef USEAPPBAR
	//  Layout *_l = static_cast<Layout *>(thiswnd->getInterface(layoutGuid));
	//  if (_l->appbar_isDocked()) return 0;
#endif

	RECT z_snapAdjust = {0};
	snapAdjustWindowRect(thiswnd, z, &z_snapAdjust);
	RECT *oldPosition = _oldPosition;
	if (oldPosition) 
	{
		oldPosition->left += z_snapAdjust.left;
		oldPosition->top += z_snapAdjust.top;
	}

	if (!coopWnd)
		wascoop = 0;

	disabledock = 0;

	if (Std::keyModifier(STDKEY_SHIFT))
	{
		for (int i = 0;i < coopList.getNumItems();i++)
		{
			coopEntry *e = coopList.enumItem(i);
			if (!e->locked)
			{
				delete e;
				coopList.removeByPos(i);
				coopcachewnd = NULL;
				i--;
			}
		}
		disabledock = 1;
	}

	int f = 0, s = 0;
	int w = z->right - z->left;
	int h = z->bottom - z->top;

	POINT done = {0};

	if (!disabledock)
	{
		ifc_window *wnd = NULL;
		for (i = desktopwnds.getNumItems(); i > -1; i--)
		{
			if (i == desktopwnds.getNumItems())
			{
#ifdef USEAPPBAR
				Layout *l = static_cast<Layout *>(thiswnd->getInterface(layoutGuid));
				if (l->appbar_isDocked()) continue;
#endif
				Wasabi::Std::getViewport(&r, thiswnd->gethWnd());
				wnd = NULL;
			}
			else
			{
				wnd = desktopwnds.enumItem(i);
				if (coopWnd != NULL && hasCoopWnd(wnd)) continue;
				Layout *l = (Layout*)wnd->getInterface(layoutGuid);
				if (l && (l->getNoDock()
#ifdef USEAPPBAR
			          || l->appbar_isDocked()
#endif
			         )) continue;

				if (wnd->isVisible())
				{
					wnd->getWindowRect(&r);
					snapAdjustWindowRect(wnd, &r);
				}
				else continue;
			}

			if (coopWnd != NULL && coopWnd == wnd || (i >= 0 && hasCoopWnd(desktopwnds.enumItem(i)))) continue;

			if (thiswnd == wnd) continue;

			RECT oz = *z;
			POINT thisdone = {0};

#ifdef WASABI_COMPILE_WNDMGR
			if (SnapPoint::match(thiswnd, z, wnd, mask, (int *)&thisdone.x, (int *)&thisdone.y, w, h)) s++;
#endif
			if (z->left > r.left - getDockDistance() && z->left < r.left + getDockDistance() && (mask & LEFT) && !thisdone.x)
			{
				z->left = r.left;
				thisdone.x = 1;
				if (mask & KEEPSIZE) z->right = r.left + w;
				f++;
			}
			if (i != desktopwnds.getNumItems() && z->right > r.left - getDockDistance() && z->right < r.left + getDockDistance() && (mask & RIGHT) && !thisdone.x)
			{
				z->right = r.left;
				thisdone.x = 1;
				if (mask & KEEPSIZE) z->left = r.left - w;
				f++;
			}
			if (z->top > r.top - getDockDistance() && z->top < r.top + getDockDistance() && (mask & TOP) && !thisdone.y)
			{
				z->top = r.top;
				thisdone.y = 1;
				if (mask & KEEPSIZE) z->bottom = r.top + h;
				f++;
			}
			if (i != desktopwnds.getNumItems() && z->bottom > r.top - getDockDistance() && z->bottom < r.top + getDockDistance() && (mask & BOTTOM) && !thisdone.y)
			{
				z->bottom = r.top;
				thisdone.y = 1;
				if (mask & KEEPSIZE) z->top = r.top - h;
				f++;
			}
			if (z->right > r.right - getDockDistance() && z->right < r.right + getDockDistance() && (mask & RIGHT) && !thisdone.x)
			{
				z->right = r.right;
				thisdone.x = 1;
				if (mask & KEEPSIZE) z->left = r.right - w;
				f++;
			}
			if (i != desktopwnds.getNumItems() && z->left > r.right - getDockDistance() && z->left < r.right + getDockDistance() && (mask & LEFT) && !thisdone.x)
			{
				z->left = r.right;
				thisdone.x = 1;
				if (mask & KEEPSIZE) z->right = r.right + w;
				f++;
			}

			if (z->bottom > r.bottom - getDockDistance() && z->bottom < r.bottom + getDockDistance() && (mask & BOTTOM) && !thisdone.y)
			{
				z->bottom = r.bottom;
				thisdone.y = 1;
				if (mask & KEEPSIZE) z->top = r.bottom - h;
				f++;
			}

			if (i != desktopwnds.getNumItems() && z->top > r.bottom - getDockDistance() && z->top < r.bottom + getDockDistance() && (mask & TOP) && !thisdone.y)
			{
				z->top = r.bottom;
				thisdone.y = 1;
				if (mask & KEEPSIZE) z->bottom = r.bottom + h;
				f++;
			}

			if (((wnd != NULL && (mask & NOINTERSECT) && Wasabi::Std::rectIntersect(*z, r)) || !touches(*z, r)) && !s)
			{
				*z = oz;
				thisdone.x = 0;
				thisdone.y = 0;
			}

			done.x |= thisdone.x;
			done.y |= thisdone.y;
		}
	}

	if (coopWnd == thiswnd && oldPosition)
	{
		POINT s = {0}, redock = {0};
		TList<RECT> rlist;
		s.x = z->left - oldPosition->left;
		s.y = z->top - oldPosition->top;
		for (i = 0;i < coopList.getNumItems();i++)
		{
			RECT r = {0};
			ifc_window *W = coopList.enumItem(i)->wnd;
			if (!checkWindow(W)) { coopEntry *e = coopList.enumItem(i); delete e; coopList.removeByPos(i); i--; continue; }
			if (W != (BaseWnd*) - 1)
			{
				W->getWindowRect(&r);
				//snapAdjustWindowRect(W, &r);
			}
#ifdef WIN32
			else
				GetWindowRect(WASABI_API_WND->main_getRootWnd()->gethWnd(), &r);
#endif
			int w = r.right - r.left, h = r.bottom - r.top;
			r.left += s.x;
			r.top += s.y;
			r.right = r.left + w;
			r.bottom = r.top + h;
			RECT cr = r;
			if (autoDock(W, &cr, LEFT | RIGHT | TOP | BOTTOM | NOINTERSECT | KEEPSIZE))
			{
				if (redock.x == 0) redock.x = cr.left - r.left;
				if (redock.y == 0) redock.y = cr.top - r.top;
			}
			rlist.addItem(r);
		}

		if (redock.x || redock.y)
		{
			Wasabi::Std::offsetRect(z, redock.x, redock.y);
			f++;
		}
#ifdef WIN32
		HDWP hd = NULL;
		if (coopList.getNumItems() > 0) hd = BeginDeferWindowPos(coopList.getNumItems());
#endif
		for (i = 0;i < coopList.getNumItems();i++)
		{
			RECT r = rlist.enumItem(i);
			ifc_window *W = coopList.enumItem(i)->wnd;
			r.left += redock.x;
			r.top += redock.y;
			//unsnapAdjustWindowRect(W, &r);
#ifdef WIN32
			W->notifyDeferredMove(r.left, r.top);
			//if (GetWindow(W->gethWnd(), GW_OWNER))
//				SetWindowPos(W->gethWnd(), NULL, r.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
//			else
				hd = DeferWindowPos(hd, W->gethWnd(), NULL, r.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
#else
			W->move( r.left, r.top );
#endif
		}
		foreach(coopList)
		ifc_window *w = coopList.getfor()->wnd;
		if (w != coopWnd)
		{
			Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
			if (l)
			{
				l->beginMove();
			}
		}
		endfor;
#ifdef WIN32
		if (coopList.getNumItems() > 0) EndDeferWindowPos(hd);
#endif
		foreach(coopList)
		ifc_window *w = coopList.getfor()->wnd;
		if (w != coopWnd)
		{
			Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
			if (l)
			{
				l->onMove();
				l->endMove();
			}
		}
		endfor;
		rlist.removeAll();
	}

	z->left -= z_snapAdjust.left;
	z->top -= z_snapAdjust.top;
	z->right += z_snapAdjust.right;
	z->bottom += z_snapAdjust.bottom;

	return ((f + s) != 0);
}

int WindowTracker::hasCoopWnd(ifc_window *w)
{
	if (coopcachewnd == w) return coopcache;
	coopcachewnd = w;
	coopcache = 0;
	for (int i = 0;i < coopList.getNumItems();i++)
		if (coopList.enumItem(i)->wnd == w)
		{
			coopcache = 1;
			break;
		}
	return coopcache;
}

void WindowTracker::addCoopWnd(ifc_window *w, int forced)
{
	coopList.addItem(new coopEntry(w, forced));
	coopcachewnd = NULL;
}

void WindowTracker::flushCoopWnds()
{
	coopList.deleteAll();
	coopcachewnd = NULL;
}

void WindowTracker::addRootWnd(ifc_window *wnd)
{
	ASSERT(!allWnd.haveItem(wnd));
	allWnd.addItem(wnd);
	if (!wnd->isVirtual())
	{
		ASSERT(!nonvirtuals.haveItem(wnd));
		nonvirtuals.addItem(wnd);
	}
}

void WindowTracker::removeRootWnd(ifc_window *wnd)
{
	allWnd.delItem(wnd);
	if (allWnd.getNumItems() == 0) allWnd.deleteAll(); // avoid fortify fals alarm on static
	int n = nonvirtuals.searchItem(wnd);
	if (n > -1) nonvirtuals.removeByPos(n);
}

ifc_window *WindowTracker::rootWndFromPoint(POINT *pt)
{
	/*  api_window *last = NULL;
	  api_window *last_parent = NULL;
	  for (int i=0;i<allWnd.getNumItems();i++) {
	    api_window *w = allWnd[i];
	    if (last && w->getRootWndParent() != last_parent)
	      return checkGhost(last, (signed short)pt->x, (signed short)pt->y);
	    if (w->pointInWnd(pt)) {
	      if (!w->getRootWndParent() || w->gethWnd() != w->getRootWndParent()->gethWnd()) return checkGhost(w, (signed short)pt->x, (signed short)pt->y);
	      last = w;
	      last_parent = w->getRootWndParent();
	    }
	  }
	  return NULL;*/

	// Get window's top level window for pt
#ifdef _WIN32
	OSWINDOWHANDLE t = WindowFromPoint(*pt);
	if (!t) return NULL;

	//CHECK IF SAFE ! if (!rootWndFromHwnd(t)) return NULL;

	// Find its rootWnd
	for (int i = nonvirtuals.getNumItems() - 1;i >= 0;i--)
	{
		ifc_window *r = nonvirtuals[i];
		if (r->gethWnd() == t)
		{
			POINT p = *pt;
			r->screenToClient((int*)&p.x, (int *)&p.y);
			return r->findRootWndChild(p.x, p.y);
		}
	}
#else
#warning port me!
#endif
	return NULL;
}

ifc_window *WindowTracker::rootWndFromHwnd(OSWINDOWHANDLE h)
{
	if (!h) return NULL;
	// Find its rootWnd
	for (int i = 0;i < allWnd.getNumItems();i++)
	{
		ifc_window *r = allWnd[i];
		if (r->gethWnd() == h) return r;
	}
	return NULL;
}

int WindowTracker::wasCooperativeMove()
{
	return wascoop;
}

// TODO: can be moved to a static function - doesn't seem to use any class data
void WindowTracker::snapAdjustWindowRect(ifc_window *w, RECT *r, RECT *adjustvals)
{
#ifdef WASABI_COMPILE_WNDMGR
	if (w->getInterface(layoutGuid))
	{
		RECT snapAdjust = {0};
		static_cast<Layout *>(w)->getSnapAdjust(&snapAdjust);
		double rr = w->getRenderRatio();
		if (rr != 1.0)
		{
			snapAdjust.left = (int)((double)(snapAdjust.left) * rr);
			snapAdjust.top = (int)((double)(snapAdjust.top) * rr);
			snapAdjust.right = (int)((double)(snapAdjust.right) * rr);
			snapAdjust.bottom = (int)((double)(snapAdjust.bottom) * rr);
		}
		r->left += snapAdjust.left;
		r->top += snapAdjust.top;
		r->right -= snapAdjust.right;
		r->bottom -= snapAdjust.bottom;
		if (adjustvals) *adjustvals = snapAdjust;
	}
	else { adjustvals = NULL; }
#else
	if (adjustvals) MEMSET(adjustvals, 0, sizeof(RECT));
#endif
}

void WindowTracker::unsnapAdjustWindowRect(ifc_window *w, RECT *r, RECT *adjustvals)
{
#ifdef WASABI_COMPILE_WNDMGR
	if (w->getInterface(layoutGuid))
	{
		RECT snapAdjust = {0};
		static_cast<Layout *>(w)->getSnapAdjust(&snapAdjust);
		if (w->getRenderRatio() != 1.0)
		{
			double rr = w->getRenderRatio();
			snapAdjust.left = (int)((double)(snapAdjust.left) * rr);
			snapAdjust.top = (int)((double)(snapAdjust.top) * rr);
			snapAdjust.right = (int)((double)(snapAdjust.right) * rr);
			snapAdjust.bottom = (int)((double)(snapAdjust.bottom) * rr);
		}
		r->left -= snapAdjust.left;
		r->top -= snapAdjust.top;
		r->right += snapAdjust.right;
		r->bottom += snapAdjust.bottom;
		if (adjustvals) *adjustvals = snapAdjust;
	}
	else { adjustvals = NULL; }
#else
	if (adjustvals) MEMSET(adjustvals, 0, sizeof(RECT));
#endif
}

void WindowTracker::recursAddToMoveWindows(ifc_window *wnd, redock_struct *rs, int v)
{
	if (!rs) return ;
	RECT r1;
	if (wnd != NULL)
	{
		wnd->getWindowRect(&r1);
		snapAdjustWindowRect(wnd, &r1);
	}
	else
	{
		wnd = rs->l;
		r1 = rs->original_rect;
		if (!WASABI_API_WND->rootwndIsValid(wnd)) return ;
	}

	{
		Layout *l = (Layout*)wnd->getInterface(layoutGuid);
		if (l && (l->getNoDock()
#ifdef USEAPPBAR
		          || l->appbar_isDocked()
#endif
		         )) return ;
	}

	// add all touching windows
	for (int i = 0; i < desktopwnds.getNumItems(); i++)
	{
		ifc_window *w = desktopwnds[i];
		if (!w->isVisible()) continue;
		if (w == wnd) continue;
		Layout *l = (Layout*)w->getInterface(layoutGuid);
		if (l && (l->getNoDock()
#ifdef USEAPPBAR
		          || l->appbar_isDocked()
#endif
		         )) continue;
		RECT r2;
		w->getWindowRect(&r2);
		snapAdjustWindowRect(w, &r2);
		// check for bottom touch
		if ((v == 1 || v == -1) && r2.top == r1.bottom && !tomoveWindows_bottom.haveItem(w))
		{
			if (r2.right >= r1.left && r2.left <= r1.right)
			{
				tomoveWindows_bottom.addItem(w);
				recursAddToMoveWindows(w, rs, 1);
			}
		}
		// check for right touch
		if ((v == 0 || v == -1) && r2.left == r1.right && !tomoveWindows_right.haveItem(w))
		{
			if (r2.bottom >= r1.top && r2.top <= r1.bottom)
			{
				tomoveWindows_right.addItem(w);
				recursAddToMoveWindows(w, rs, 0);
			}
		}
		// check for left touch
		if ((v == 0 || v == -1) && r2.right == r1.left && !tomoveWindows_left.haveItem(w))
		{
			if (r2.bottom >= r1.top && r2.top <= r1.bottom)
			{
				tomoveWindows_left.addItem(w);
				recursAddToMoveWindows(w, rs, 0);
			}
		}
		// check for top touch
		if ((v == 1 || v == -1) && r2.bottom == r1.top && !tomoveWindows_top.haveItem(w))
		{
			if (r2.right >= r1.left && r2.left <= r1.right)
			{
				tomoveWindows_top.addItem(w);
				recursAddToMoveWindows(w, rs, 1);
			}
		}
	}
}

void WindowTracker::beforeRedock(Layout *l, redock_struct *rs)
{
	if (!l) return ;
	rs->l = l;
	l->getWindowRect(&rs->original_rect);
	snapAdjustWindowRect(rs->l, &rs->original_rect);
}

void WindowTracker::afterRedock(Layout *l, redock_struct *rs)
{
	RECT nr;
	if (!rs) return ;

	if (!WASABI_API_WND->rootwndIsValid(l)) return ;
	if (!WASABI_API_WND->rootwndIsValid(rs->l)) return ;
	recursAddToMoveWindows(NULL, rs);

	l->getWindowRect(&nr);
	snapAdjustWindowRect(l, &nr);

	if (l->isUnlinked() || rs->l->isUnlinked()) return ;

#ifdef WIN32
	HDWP hdwp = BeginDeferWindowPos(desktopwnds.getNumItems());
#endif

	PtrList<Layout> toendmove;

	int diff = rs->original_rect.bottom - nr.bottom;
	if (diff)
	{ // check for bottom side dock changes
		for (int i = 0;i < tomoveWindows_bottom.getNumItems();i++)
		{
			ifc_window *w = tomoveWindows_bottom[i];
			if (w == l) continue;
			if (w == rs->l) continue;
			if (!allWnd.haveItem(w)) continue;
			RECT r;
			w->getWindowRect(&r);
			r.top -= diff;
			r.bottom -= diff;
#ifdef WIN32
			w->notifyDeferredMove(r.left, r.top);
			DeferWindowPos(hdwp, w->gethWnd(), NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
#else
			w->move( r.left, r.top );
#endif
			Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
			if (l) toendmove.addItem(l);
		}
	}

	diff = rs->original_rect.top - nr.top;
	if (diff)
	{ // check for top side dock changes
		for (int i = 0;i < tomoveWindows_top.getNumItems();i++)
		{
			ifc_window *w = tomoveWindows_top[i];
			if (w == l) continue;
			if (w == rs->l) continue;
			if (!allWnd.haveItem(w)) continue;
			RECT r;
			w->getWindowRect(&r);
			r.top -= diff;
			r.bottom -= diff;
#ifdef WIN32
			w->notifyDeferredMove(r.left, r.top);
			DeferWindowPos(hdwp, w->gethWnd(), NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
#else
			w->move( r.left, r.top );
#endif
			Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
			if (l) toendmove.addItem(l);
		}
	}

	diff = rs->original_rect.right - nr.right;
	if (diff)
	{ // check for right side dock changes
		for (int i = 0;i < tomoveWindows_right.getNumItems();i++)
		{
			ifc_window *w = tomoveWindows_right[i];
			if (w == l) continue;
			if (w == rs->l) continue;
			if (!allWnd.haveItem(w)) continue;
			RECT r;
			w->getWindowRect(&r);
			r.left -= diff;
			r.right -= diff;
			Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
			if (l) l->beginMove();
#ifdef WIN32
			w->notifyDeferredMove(r.left, r.top);
			DeferWindowPos(hdwp, w->gethWnd(), NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
#else
			w->move( r.left, r.top );
#endif
			if (l) toendmove.addItem(l);
		}
	}

	diff = rs->original_rect.left - nr.left;
	if (diff)
	{ // check for left side dock changes
		for (int i = 0;i < tomoveWindows_left.getNumItems();i++)
		{
			ifc_window *w = tomoveWindows_left[i];
			if (w == l) continue;
			if (w == rs->l) continue;
			if (!allWnd.haveItem(w)) continue;
			RECT r;
			w->getWindowRect(&r);
			r.left -= diff;
			r.right -= diff;
			Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
			if (l) l->beginMove();
#ifdef WIN32
			w->notifyDeferredMove(r.left, r.top);
			DeferWindowPos(hdwp, w->gethWnd(), NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
#else
			w->move( r.left, r.top );
#endif
			if (l) toendmove.addItem(l);
		}
	}

#ifdef WIN32
	EndDeferWindowPos(hdwp);
#endif
	tomoveWindows_left.removeAll();
	tomoveWindows_top.removeAll();
	tomoveWindows_right.removeAll();
	tomoveWindows_bottom.removeAll();
	rs->l = NULL;
	foreach(toendmove)
	toendmove.getfor()->onMove();
	toendmove.getfor()->endMove();
	endfor;
}

void WindowTracker::layoutChanged(Layout *previouswnd, Layout *newwnd)
{
	redock_struct rs;
	beforeRedock(previouswnd, &rs);
	afterRedock(newwnd, &rs);
}

ifc_window *WindowTracker::coopWnd = NULL;
PtrList<ifc_window> WindowTracker::desktopwnds;
PtrList<ifc_window> WindowTracker::nonvirtuals;
PtrList<coopEntry> WindowTracker::coopList;
PtrList<ifc_window> WindowTracker::recursList;
PtrList<ifc_window> WindowTracker::tomoveWindows_left;
PtrList<ifc_window> WindowTracker::tomoveWindows_top;
PtrList<ifc_window> WindowTracker::tomoveWindows_right;
PtrList<ifc_window> WindowTracker::tomoveWindows_bottom;
PtrList<ifc_window> WindowTracker::allWnd;
int WindowTracker::dockDist = DEFAULT_DOCK_DIST;
int WindowTracker::dock_enabled = 1;
