#include "precomp.h"
#include "appbarwnd.h"
#include <tataki/region/region.h>
#include <api/wnd/resizable.h>
#include <api/wndmgr/layout.h>
#include <api/config/items/cfgitem.h>
#include <api/config/items/attrint.h>
#include "../../../../Plugins/General/gen_ff/wa2cfgitems.h"

#define CB_CHECK 0x101
#define DOCK_DISTANCE_X 5
#define DOCK_DISTANCE_Y 5

#ifndef WIN32
#error port me or remove me from the inheritance on this platform !
#endif

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include "../../../../Plugins/General/gen_ff/main.h"
#include "appbarwnd.h"

extern _int cfg_options_appbardockingdistance;

// -----------------------------------------------------------------------
AppBarWnd::AppBarWnd() {
	m_registered = 0;
	m_side = APPBAR_NOTDOCKED;
	m_enabled = 0;
	m_cur_side = APPBAR_NOTDOCKED;
	m_cur_autohide = 0;
	m_cur_hiding = 0;
	m_oldZOrder = NULL;
	m_destroying = FALSE;
	m_norestore = 0;
	m_sliding = 0;
	m_autounhide_timer_set = 0;
	m_autohide_timer_set = 0;
	m_suspended = 0;
	m_fs = 0;
	m_wahidden = 0;
}

// -----------------------------------------------------------------------
AppBarWnd::~AppBarWnd() {
	m_destroying = TRUE;
	if (m_cur_side != APPBAR_NOTDOCKED) unDock();
	unregisterWinAppBar();
}

// -----------------------------------------------------------------------
int AppBarWnd::registerWinAppBar() 
{
	if (m_registered) 
		unregisterWinAppBar();

	APPBARDATA abd;

	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = getOsWindowHandle();
	abd.uCallbackMessage = APPBAR_CALLBACK;

	m_registered = (int)SHAppBarMessage(ABM_NEW, &abd);
	return m_registered;
}

// -----------------------------------------------------------------------
void AppBarWnd::unregisterWinAppBar() {
	if (m_registered) {
		APPBARDATA abd;

		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = getOsWindowHandle();

		SHAppBarMessage(ABM_REMOVE, &abd);
		m_registered = 0;
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::appbar_dock(int side) {
	m_side = side;
	updateDocking();
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_isDocked() {
	return m_side != APPBAR_NOTDOCKED;
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_getSide() {
	return m_side;
}

// -----------------------------------------------------------------------
void AppBarWnd::appbar_setEnabledSides(int mask) {
	m_enabled = mask;
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_getEnabledSides() {
	return m_enabled;
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_isSideEnabled(int side) {
	if (side == APPBAR_LEFT && !(m_enabled & APPBAR_LEFT_ENABLED)) return 0;
	if (side == APPBAR_TOP && !(m_enabled & APPBAR_TOP_ENABLED)) return 0;
	if (side == APPBAR_RIGHT && !(m_enabled & APPBAR_RIGHT_ENABLED)) return 0;
	if (side == APPBAR_BOTTOM && !(m_enabled & APPBAR_BOTTOM_ENABLED)) return 0;
	return 1;
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_isSideAutoHideSafe(int side) {
	OSWINDOWHANDLE cur = getCurAutoHide(side);

	if (cur == NULL || cur == getOsWindowHandle()) {
		RECT primary = {0};
		Wasabi::Std::getViewport(&primary, hwnd, 1);

		DebugStringW( L"primary screen coords = %d,%d -> %d,%d (%dx%d)\n", primary.left, primary.top, primary.right, primary.bottom, primary.right - primary.left, primary.bottom - primary.top );
		int monitor = 0;
		//int g = 0;
		while (1) {
			RECT r;
			int ret = Wasabi::Std::enumViewports(monitor++, &r, 1);
			if (ret == 0) break;

			if (Wasabi::Std::rectEqual(&primary, &r)) continue;

			DebugStringW(L"secondary screen = %d,%d -> %d,%d (%dx%d)\n", r.left, r.top, r.right, r.bottom, r.right-r.left, r.bottom-r.top);
			if (r.right <= primary.left && side == APPBAR_LEFT) return 0;
			if (r.bottom <= primary.top && side == APPBAR_TOP) return 0;
			if (r.left >= primary.right && side == APPBAR_RIGHT) return 0;
			if (r.top >= primary.bottom && side == APPBAR_BOTTOM) return 0;
		}
	}
	else 
		return 0;
	  
	return 1;
}

// -----------------------------------------------------------------------
OSWINDOWHANDLE AppBarWnd::getCurAutoHide(int side) {
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = getOsWindowHandle();
	abd.uEdge = side;
	return (OSWINDOWHANDLE)SHAppBarMessage(ABM_GETAUTOHIDEBAR, &abd);
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_testDock(int x, int y, RECT *dockrect) {
	POINT ptCursor = {x, y};
	LONG cxScreen, cyScreen;
	int dx=999999, dy=999999;
	int horiz=-1, vert=-1;
	RECT viewRect = {0};

	Wasabi::Std::getViewport(&viewRect, hwnd, 1);

	// Find out which edge of the screen we're closest to
	cxScreen = viewRect.right;
	cyScreen = viewRect.bottom;

	if (x < viewRect.left || x > cxScreen || y < viewRect.top || y > cyScreen) return APPBAR_NOTDOCKED;

	if (ptCursor.x < (cxScreen / 2)) {
		if (m_enabled & APPBAR_LEFT_ENABLED) {
			dx = ptCursor.x;
			horiz = APPBAR_LEFT;
		}
	}
	else {
		if (m_enabled & APPBAR_RIGHT_ENABLED) {
			dx = cxScreen - ptCursor.x;
			horiz = APPBAR_RIGHT;
		}
	}

	if (ptCursor.y < (cyScreen / 2)) {
		if (m_enabled & APPBAR_TOP_ENABLED) {
			dy = ptCursor.y;
			vert = APPBAR_TOP;
		}
	}
	else {
		if (m_enabled & APPBAR_BOTTOM_ENABLED) {
  			dy = cyScreen - ptCursor.y;
			vert = APPBAR_BOTTOM;
		}
	}

	int ret = -1;
	#ifdef GEN_FF
	int dockdist = cfg_options_appbardockingdistance;
	#else
	// TODO: do a config lookup, but make it not so slow
	/*
		const GUID options_guid = 
	{ 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
	int dockdist = _intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid), L"Appbars Docking Distance", 5);*/
	int dockdist = 5;
	#endif
	if ((cxScreen * dy) > (cyScreen * dx))
		if (dx <= dockdist)
			ret = horiz;
	if (dy <= dockdist)
		ret = vert;

	if (dockrect && ret != -1) {
		getDockRect(ret, dockrect);
	}

	return ret;
}

// -----------------------------------------------------------------------
void AppBarWnd::getDockRect(int side, RECT *rect) {
	LONG cxScreen, cyScreen;
	RECT viewRect = {0};
	Wasabi::Std::getViewport(&viewRect, hwnd, 1);

	cxScreen = viewRect.right;
	cyScreen = viewRect.bottom;

	if (isMaximized()) {
		getRestoredRect(rect);
		if (renderRatioActive()) multRatio(rect);
	}
	else getWindowRect(rect);

	Layout *l = (Layout *)getInterface(layoutGuid);
	if (l) {
		RECT adj;
		l->getSnapAdjust(&adj);
		if (renderRatioActive()) {
			multRatio((int *)&adj.left, (int *)&adj.top);
			multRatio((int *)&adj.right, (int *)&adj.bottom);
		}
		int h = rect->bottom - rect->top;
		int w = rect->right - rect->left;
		h -= adj.top + adj.bottom;
		w -= adj.left + adj.right;
		rect->left += adj.left;
		rect->top += adj.top;
		rect->bottom = rect->top + h;
		rect->right = rect->left + w;
	}

	switch (side) {
		case APPBAR_TOP:
		case APPBAR_LEFT:
			OffsetRect(rect, -rect->left, -rect->top);
		break;
		case APPBAR_BOTTOM:
		case APPBAR_RIGHT:
			OffsetRect(rect, cxScreen-rect->right, cyScreen-rect->bottom);
		break;
	}

	switch (side) {
		case APPBAR_TOP:
		case APPBAR_BOTTOM:
			rect->left = viewRect.left;
			rect->right = cxScreen;
		break;
		case APPBAR_LEFT:
		case APPBAR_RIGHT:
			rect->top = viewRect.top;
			rect->bottom = cyScreen;
		break;
	}
	
	OSWINDOWHANDLE cur = getCurAutoHide(side);
	int safeah = appbar_isSideAutoHideSafe(side);

	if (!safeah || !(appbar_wantAutoHide() && (!cur || cur == getOsWindowHandle()))) {
		straightenRect(side, rect);
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::updateDocking() {
	if (!isVisible()) {
		m_suspended = 1;
		return;
	}
	updateSide();
	appbar_updateAutoHide();
	appbar_updateAlwaysOnTop();
	updateTimers();
}

// -----------------------------------------------------------------------
void AppBarWnd::updateTimers() {
	if (m_cur_autohide) {
		if (m_cur_hiding) {
			resetAutoHideTimer();
			setAutoUnHideTimer();
		}
		else {
			resetAutoUnHideTimer();
			setAutoHideTimer();
		}
	}
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_updateAlwaysOnTop() {
	if (m_side == APPBAR_NOTDOCKED) return 0;
	SetWindowPos(getOsWindowHandle(), appbar_wantAlwaysOnTop() ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	return 1;
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_updateAutoHide() {
	int autohide = appbar_wantAutoHide();
	if (m_cur_autohide == autohide) return 0;
	  
	if (autohide && !appbar_isSideAutoHideSafe(m_cur_side)) autohide = 0;

	if (m_cur_autohide == autohide) return 0;

	if (autohide) {
		// cur_autohide is off, turn it on
		m_cur_hiding = 0;
		setAutoHideTimer();
	}
	else {
		// cur_autohide is on, turn it off
		if (m_cur_hiding) resetAutoUnHideTimer();
		else resetAutoHideTimer();
	}
	  
	m_cur_autohide = autohide;
	dock(m_cur_side);

	return 1;
}

// -----------------------------------------------------------------------
void AppBarWnd::onAfterReinit() {
	APPBARWND_PARENT::onAfterReinit();
	m_autohide_timer_set = 0;
	m_autounhide_timer_set = 0;
	updateTimers();
}

// -----------------------------------------------------------------------
void AppBarWnd::setAutoHideTimer(){
	if (!m_autohide_timer_set) {
		SetTimer(getOsWindowHandle(), IDT_AUTOHIDE, cfg_uioptions_appbarshidetime, NULL);
		m_autohide_timer_set = 1;
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::setAutoUnHideTimer(){
	if (!m_autounhide_timer_set) {
		SetTimer(getOsWindowHandle(), IDT_AUTOUNHIDE, cfg_uioptions_appbarsshowtime, NULL);
		m_autounhide_timer_set = 1;
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::resetAutoHideTimer(){
	if (m_autohide_timer_set) {
		KillTimer(getOsWindowHandle(), IDT_AUTOHIDE);
		m_autohide_timer_set = 0;
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::resetAutoUnHideTimer() {
	if (m_autounhide_timer_set) {
		KillTimer(getOsWindowHandle(), IDT_AUTOUNHIDE);
		m_autounhide_timer_set = 0;
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::updateSide() {
	if (m_cur_side == m_side) return;
	if (m_side != m_cur_side && m_cur_side != APPBAR_NOTDOCKED && m_side != APPBAR_NOTDOCKED && m_cur_autohide) {
		resetAutoHideSide(m_cur_side);
	}
	if (m_side == APPBAR_NOTDOCKED) unDock();
	else dock(m_side);
}

// -----------------------------------------------------------------------
void AppBarWnd::resetAutoHideSide(int side) {
	HWND cur = getCurAutoHide(side);
	if (cur == getOsWindowHandle()) {
		APPBARDATA abd;
		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = cur;
		abd.uEdge = side;
		abd.lParam = FALSE;			
		SHAppBarMessage(ABM_SETAUTOHIDEBAR, &abd);
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::setAutoHideSide(int side) {
	APPBARDATA abd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = getOsWindowHandle();
	abd.uEdge = side;
	abd.lParam = TRUE;
	SHAppBarMessage(ABM_SETAUTOHIDEBAR, &abd);
}

// -----------------------------------------------------------------------
void AppBarWnd::dock(int side) {
	unOwn();

	if (!registerWinAppBar()) { 
		reOwn(); 
		m_side = APPBAR_NOTDOCKED;
		m_cur_side = APPBAR_NOTDOCKED;
		m_cur_autohide = 0;
	}

	maximize(0);

	RECT rect;
	getDockRect(side, &rect);

	{
		RECT adj = rect;
		if (ABS(getRenderRatio() - 1.0) > 0.01f) {
			int _w = adj.right-adj.left;
			int _h = adj.bottom-adj.top;
			double rr = getRenderRatio();
			_w = (int)((double)(_w) / rr + 0.5);
			_h = (int)((double)(_h) / rr + 0.5);
			adj.right = adj.left + _w;
			adj.bottom = adj.top + _h;
		}
		snapAdjust(&adj, 1);
		resizeToRect(&adj);
	}

	if (!appbar_wantAutoHide() || !appbar_isSideAutoHideSafe(side)) {
		notifyWinAppBarPosition(side, rect);
	}
	else {
		getEdge(side, &rect);
		notifyWinAppBarPosition(side, rect);
		setAutoHideSide(side);
		m_cur_hiding = 0;
	}

	if (!m_suspended) appbar_onDock(side);

	#ifdef WASABI_APPBAR_ONDOCKCHANGED
	WASABI_APPBAR_ONDOCKCHANGED(this)
	#endif
}

// -----------------------------------------------------------------------
void AppBarWnd::unDock() {
	if (m_cur_side != APPBAR_NOTDOCKED) {

		resetAutoHideSide(m_cur_side);
		unregisterWinAppBar();

		if (!m_destroying) {
			reOwn();
			if (!m_norestore) restore();
			#ifdef WASABI_APPBAR_ONDOCKCHANGED
			WASABI_APPBAR_ONDOCKCHANGED(this)
			#endif
		}

		m_cur_side = APPBAR_NOTDOCKED;

		if (!m_suspended) appbar_onUnDock();
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::notifyWinAppBarPosition(int side, RECT rect) {
  APPBARDATA abd;

	abd.cbSize = sizeof(APPBARDATA);
	abd.hWnd = getOsWindowHandle();
	abd.rc = rect;
	abd.uEdge = side;

	SHAppBarMessage(ABM_SETPOS, &abd);
	m_cur_side = side;
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_isHiding() {
	return m_cur_hiding;
}

// -----------------------------------------------------------------------
int AppBarWnd::appbar_isAutoHiding() {
	return m_cur_autohide;
}

// -----------------------------------------------------------------------
void AppBarWnd::appbar_posChanged() {
	if (m_side == APPBAR_NOTDOCKED) return;
	RECT wr;
	getWindowRect(&wr);
	int w = wr.right-wr.left;
	int h = wr.bottom-wr.top;

	if (m_cur_autohide && m_cur_side != APPBAR_NOTDOCKED && !appbar_isSideAutoHideSafe(m_cur_side))
		m_cur_autohide = 0;

	RECT rc;
	getDockRect(m_cur_side, &rc);

	if (!m_cur_autohide) {
		{
			RECT adj = rc;
			if (ABS(getRenderRatio() - 1.0) > 0.01f) {
				int _w = adj.right-adj.left;
				int _h = adj.bottom-adj.top;
				double rr = getRenderRatio();
				_w = (int)((double)(_w) / rr + 0.5);
				_h = (int)((double)(_h) / rr + 0.5);
				adj.right = adj.left + _w;
				adj.bottom = adj.top + _h;
			}
			snapAdjust(&adj, 1);
			resizeToRect(&adj);
		}
		notifyWinAppBarPosition(m_cur_side, rc);
	}
	else {
		int aaw = appbar_getAutoHideWidthHeight();
		RECT er;
		getEdge(m_cur_side, &er);
		notifyWinAppBarPosition(m_cur_side, er);
		RECT adj = {0,0,0,0};
		Layout *l = (Layout *)getInterface(layoutGuid);
		if (l) l->getSnapAdjust(&adj);
		if (renderRatioActive()) multRatio(&adj);
		if (m_cur_hiding) {
			switch (m_cur_side) {
				case APPBAR_TOP:
					rc.bottom = er.top + aaw + adj.bottom;
					rc.top = rc.bottom - h;
				break;
				case APPBAR_BOTTOM:
					rc.top = er.bottom - aaw - adj.top;
					rc.bottom = rc.top + h;
				break;
				case APPBAR_LEFT:
					rc.right = er.left + aaw + adj.right;
					rc.left = rc.right - w;
				break;
				case APPBAR_RIGHT:
					rc.left = er.right - aaw - adj.left;
					rc.right = rc.left + w;
				break;
			}
		}
		if (ABS(getRenderRatio() - 1.0) > 0.01f) {
			int _w = rc.right-rc.left;
			int _h = rc.bottom-rc.top;
			double rr = getRenderRatio();
			_w = (int)((double)(_w) / rr + 0.5);
			_h = (int)((double)(_h) / rr + 0.5);
			rc.right = rc.left + _w;
			rc.bottom = rc.top + _h;
		}
		resizeToRect(&rc);
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::getEdge(int side, RECT *rc) {
	ASSERT(rc != NULL);
	Wasabi::Std::getViewport(rc, hwnd, 1);
	switch (side) {
		case APPBAR_TOP:
			rc->bottom = rc->top; break;
		case APPBAR_BOTTOM:
			rc->top = rc->bottom; break;
		case APPBAR_LEFT:
			rc->right = rc->left; break;
		case APPBAR_RIGHT:
			rc->left = rc->right; break;
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::appBarCallback(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	APPBARDATA abd = {0};

	if (m_registered) {
		abd.cbSize = sizeof(abd);
		abd.hWnd = getOsWindowHandle();

		switch (wParam) 
		{
			// the taskbar's autohide or always-on-top state has changed.
			case ABN_STATECHANGE:
  				DebugString("AppBarCallback: ABN_STATECHANGE\n");
			break;

			// a full screen application is opening or closing.  we must drop
			// to the bottom of the Z-Order and restore it later.
			case ABN_FULLSCREENAPP:
				DebugString("AppBarCallback: ABN_FULLSCREENAPP\n");
				if (lParam && !m_fs) {
					m_fs=1;
  					m_oldZOrder = GetWindow(getOsWindowHandle(), GW_HWNDPREV);
					SetWindowPos(getOsWindowHandle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
				else if (!lParam && m_fs) {
					m_fs = 0;
					SetWindowPos(getOsWindowHandle(), appbar_wantAlwaysOnTop() ? HWND_TOPMOST : m_oldZOrder, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					m_oldZOrder = NULL;
				}
				break;
        
			// something changed that may have modified the possible appbar's positions
    		case ABN_POSCHANGED:
				DebugString("AppBarCallback: ABN_POSCHANGED\n");
				appbar_posChanged();
       		break;

			case ABN_WINDOWARRANGE:
				if (lParam && !m_wahidden) {
					m_wahidden = 1;
					ShowWindow(getOsWindowHandle(), SW_HIDE);
				}
				else if (!lParam && m_wahidden) {
					m_wahidden = 0;
					ShowWindow(getOsWindowHandle(), SW_NORMAL);
				}
			break;
		}
	}
}

// -----------------------------------------------------------------------
LRESULT AppBarWnd::wndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	if ( m_registered )
	{
		switch ( msg )
		{
			case WM_MOVE:
			{
				//DebugString("WM_MOVE\n");
				return 0;
			}
			case WM_WINDOWPOSCHANGED:
			{
				//DebugString("WM_WINDOWPOSCHANGED\n");
				//LPWINDOWPOS lpwpos = (LPWINDOWPOS)lparam;
				APPBARDATA abd = { 0 };
				abd.cbSize = sizeof( APPBARDATA );
				abd.hWnd   = getOsWindowHandle();

				SHAppBarMessage( ABM_WINDOWPOSCHANGED, &abd );
			}
			case APPBAR_CALLBACK:
			{
				if ( !m_destroying ) appBarCallback( msg, wparam, lparam );
				return 0;
			}
			case WM_DISPLAYCHANGE:
			{
				DebugString( "WM_DISPLAYCHANGE\n" );
				appbar_posChanged();
			}
			case WM_TIMER:
			{ // // not using multiplexed timer for independent speed
				switch ( wparam )
				{
					case IDT_AUTOHIDE:
						onAutoHideTimer();
						break;
					case IDT_AUTOUNHIDE:
						onAutoUnHideTimer();
						break;
				}
			}
			case WM_COMMAND:
			{
				// forward onto the main Winamp window and let it do it
				if ( HIWORD( wparam ) == THBN_CLICKED )
				{
					SendMessageW( plugin.hwndParent, msg, wparam, lparam );
				}
			}
		}
	}

	return APPBARWND_PARENT::wndProc( hwnd, msg, wparam, lparam );
}

// -----------------------------------------------------------------------
void AppBarWnd::onAutoHideTimer() {
	HWND me = getOsWindowHandle();
	POINT pt;
	RECT rc;
	HWND hact;
	if (m_cur_autohide) {
		if (!m_cur_hiding) {
			GetCursorPos(&pt);
			GetWindowRect(hwnd, &rc);
			snapAdjust(&rc, -1);
			hact = GetForegroundWindow();

			if ((!PtInRect(&rc, pt) || screenCorner(&pt)) && (hact != me) && /*(hact!= NULL) && */(GetWindowOwner(hact) != me)) {
  				resetAutoHideTimer();
				autoHide();
				setAutoUnHideTimer();
			}
		}
		else {
			resetAutoHideTimer();
			setAutoUnHideTimer();
		}
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::onAutoUnHideTimer() {
	RECT rc;
	POINT pt;
	HWND me = getOsWindowHandle();

	GetWindowRect(me, &rc);
	snapAdjust(&rc, -1);
	if (m_cur_autohide) {
		if (m_cur_hiding) {
			GetCursorPos(&pt);
			if (PtInRect(&rc, pt) && !screenCorner(&pt)) {
				resetAutoUnHideTimer();
				autoUnHide();
				setAutoHideTimer();
			}
  		}
		else {
  			resetAutoUnHideTimer();
			setAutoHideTimer();
		}
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::autoHide() {
	if (m_cur_autohide) {
		if (!m_cur_hiding)	{
			RECT rc;

			getWindowRect(&rc); 

			int h = rc.bottom-rc.top;
			int w = rc.right-rc.left;

			int aaw = appbar_getAutoHideWidthHeight();

			RECT adj={0,0,0,0};
			Layout *l = static_cast<Layout*>(getInterface(layoutGuid));
			l->getSnapAdjust(&adj);
			if (renderRatioActive()) multRatio(&adj);

			RECT viewRect = {0};
			Wasabi::Std::getViewport(&viewRect, hwnd, 1);

			switch (m_side) {
				case APPBAR_TOP:
					rc.top = -(h - aaw + adj.top - adj.bottom);
			    break;
				case APPBAR_BOTTOM:
					rc.top = (viewRect.bottom - viewRect.top) - aaw - adj.top;
			    break;
				case APPBAR_LEFT:
					rc.left = -(w - aaw + adj.left - adj.right);
			    break;
				case APPBAR_RIGHT:
					rc.left = viewRect.right - aaw - adj.left;
			    break;
			}

			switch (m_side) {
				case APPBAR_TOP:
				case APPBAR_BOTTOM:
					rc.bottom = rc.top + h;
			    break;
				case APPBAR_LEFT:
				case APPBAR_RIGHT:
					rc.right = rc.left + w;
			    break;
			}

			slideWindow(&rc);
			m_cur_hiding = 1;	
		}
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::autoUnHide() {
	if (m_cur_autohide) {
		if (m_cur_hiding) {
			m_cur_hiding = 0;

			RECT rc;
			getWindowRect(&rc); 

			int h = rc.bottom-rc.top;
			int w = rc.right-rc.left;

			int aaw = appbar_getAutoHideWidthHeight();

			RECT adj={0,0,0,0};
			Layout *l = static_cast<Layout*>(getInterface(layoutGuid));
			l->getSnapAdjust(&adj);
			if (renderRatioActive()) multRatio(&adj);

			switch (m_side) {
				case APPBAR_TOP:
					rc.top += (h - aaw) - (adj.top + adj.bottom); 
					rc.bottom += (h - aaw) - (adj.top + adj.bottom);
				break;
				case APPBAR_BOTTOM:
					rc.top -= (h - aaw) - (adj.top + adj.bottom);
					rc.bottom -= (h - aaw) - (adj.top + adj.bottom);
			    break;
				case APPBAR_LEFT:
					rc.right += (w - aaw) - (adj.left + adj.right);
					rc.left += (w - aaw) - (adj.left + adj.right);
			    break;
				case APPBAR_RIGHT:
				    rc.left -= (w - aaw) - (adj.left + adj.right);
				    rc.right -= (w - aaw) - (adj.left + adj.right);
			    break;
			}

			slideWindow(&rc);
		}
	}
}

// -----------------------------------------------------------------------

const int g_dtSlideHide = 400;
const int g_dtSlideShow = 200;

// -----------------------------------------------------------------------
void AppBarWnd::slideWindow(RECT *prc) {
	if (m_cur_autohide)	{
		m_sliding = 1;
		RECT rcOld;
		RECT rcNew;
		int x, y, dx, dy, dt, t, t0;
		BOOL fShow;
		HANDLE hThreadMe;
		int priority;

		HWND hwnd = getOsWindowHandle();

		rcNew = *prc;

		/*DebugString("rcNew : left=%d, top=%d, "
					  "right=%d, bottom=%d\n", rcNew.left,
                      rcNew.top, rcNew.right, rcNew.bottom);*/

		if ((g_dtSlideShow > 0) && (g_dtSlideHide > 0)) {
			GetWindowRect(hwnd, &rcOld);

			fShow = TRUE;/*(rcNew.bottom - rcNew.top) > (rcOld.bottom - rcOld.top) || 
							(rcNew.right - rcNew.left) > (rcOld.right - rcOld.left);*/
                              
			dx = (rcNew.left - rcOld.left);
			dy = (rcNew.top - rcOld.top);

			if (fShow) {
				rcOld = rcNew;
				OffsetRect(&rcOld, -dx, -dy);
				//DebugString("appbar_slideWindow %d %d\n", rcOld.left, rcOld.top);
				move(rcOld.left, rcOld.top);
				dt = g_dtSlideShow;
			}
			else {
				dt = g_dtSlideHide;
			}

			hThreadMe = GetCurrentThread();
			priority = GetThreadPriority(hThreadMe);
			SetThreadPriority(hThreadMe, THREAD_PRIORITY_HIGHEST);

			t0 = GetTickCount();
			while ((t = GetTickCount()) < t0 + dt) {
				x = rcOld.left + dx * (t - t0) / dt;
				y = rcOld.top + dy * (t - t0) / dt;

				//DebugString("appbar_slideWindow(2) %d %d\n", x, y);
				move(x, y);
				//SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				if (fShow) {
					UpdateWindow(hwnd);
					//invalidateWindowRegion();
					//updateWindowRegion();
				}
				else UpdateWindow(GetDesktopWindow());
			}

			SetThreadPriority(hThreadMe, priority);
		}

		//DebugString("appbar_slideWindow(3) %d %d\n", rcNew.left, rcNew.top);
		move(rcNew.left, rcNew.top);
		appbar_onSlide();
	}
	WASABI_API_MAKI->vcpu_setComplete();
	m_sliding = 0;
}

// -----------------------------------------------------------------------
void AppBarWnd::unOwn() 
{
	// registration was successful, we should reparent the window to NULL so that minimizing the app or changing the main AOT flag does
	// nothing to this window
	Layout *l = static_cast<Layout*>(getInterface(layoutGuid));
	if (l) {
		if (!l->getNoParent()) {
			l->setNoParent(2);
			// whoaaah!
			reinit();
		}
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::reOwn() {
	OSWINDOWHANDLE mw = WASABI_API_WND->main_getRootWnd()->getOsWindowHandle();
	if (IsIconic(mw)) ShowWindow(mw, SW_RESTORE);
	// undock was successful, we should re-own the window to what it was previously. if the old owner is minimized, we should restore it first
	OSWINDOWHANDLE oldparent = WASABI_API_WND->main_getRootWnd()->getOsWindowHandle();
	if (IsIconic(oldparent)) ShowWindow(oldparent, SW_RESTORE);
	Layout *l = static_cast<Layout *>(getInterface(layoutGuid));
	if (l) {
		int oldnp = l->getNoParent();
		const wchar_t *np = l->getGuiObject()->guiobject_getXmlParam(L"noparent");
		int newnp = WTOI(np);
		if (oldnp != newnp) 
		{
			l->setNoParent(newnp);
			// whoaaah!
			reinit();
		}
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::straightenRect(int side, RECT *r) {
	int w=0, h=0;

	int wasregistered = m_registered;
	if (!m_registered) registerWinAppBar();

	APPBARDATA abd;
	abd.hWnd = hwnd;
	abd.cbSize = sizeof(APPBARDATA);
	abd.rc = *r;
	abd.uEdge = side;

	RECT viewRect = {0};
	Wasabi::Std::getViewport(&viewRect, hwnd, 1);

	switch (side) {
		case APPBAR_LEFT:
		case APPBAR_RIGHT:
			w = abd.rc.right - abd.rc.left;
			abd.rc.top = viewRect.top;
			abd.rc.bottom = viewRect.bottom;
		break;
		case APPBAR_TOP:
		case APPBAR_BOTTOM:
			h = abd.rc.bottom - abd.rc.top;
			abd.rc.left = viewRect.left;
			abd.rc.right = viewRect.right;
		break;
	}

	SHAppBarMessage(ABM_QUERYPOS, &abd);

	switch (abd.uEdge) {
		case APPBAR_LEFT:
			abd.rc.right = abd.rc.left + w;
		break;
		case APPBAR_RIGHT:
			abd.rc.left = abd.rc.right - w;
		break;
		case APPBAR_TOP:
			abd.rc.bottom = abd.rc.top + h;
		break;
		case APPBAR_BOTTOM:
			abd.rc.top = abd.rc.bottom - h;
		break;
	}

	if (!wasregistered) unregisterWinAppBar();

	*r = abd.rc;	
}

// -----------------------------------------------------------------------
void AppBarWnd::appbar_setNoRestore(int no) {
	m_norestore = no;
}

// -----------------------------------------------------------------------
void AppBarWnd::onSetVisible( int show )
{
	if ( !show && m_side != APPBAR_NOTDOCKED && !m_suspended )
	{
		if ( m_cur_autohide )
		{
			resetAutoHideSide( m_cur_side );
			if ( m_cur_hiding )
				resetAutoUnHideTimer();
			else
				resetAutoHideTimer();
		}

		m_suspended = 1;
		unDock();
		APPBARWND_PARENT::onSetVisible( show );
		return;
	}
	else if ( show && m_suspended )
	{
		APPBARWND_PARENT::onSetVisible( show );
		m_suspended = 0;
		updateDocking();
		return;
	}

	APPBARWND_PARENT::onSetVisible( show );
}

// -----------------------------------------------------------------------
int AppBarWnd::screenCorner(POINT *pt) {
	RECT primary = {0};
	Wasabi::Std::getViewport(&primary, hwnd, 1);
	if (pt->x > primary.right-2 && pt->x <= primary.right) {
	    if (pt->y > primary.bottom-2 && pt->y <= primary.bottom) {
			// bottom right corner
			return 1;
		}
		else if (pt->y < primary.top+2 && pt->y >= primary.top) {
			// top right corner
			return 1;
		}
		}
		else if (pt->x < primary.left+2 && pt->x >= primary.left) {
			if (pt->y > primary.bottom-2 && pt->y <= primary.bottom) {
			// bottom left corner
			return 1;
		}
		else if (pt->y < primary.top+2 && pt->y >= primary.top) {
			// top left  corner
			return 1;
		}
	}
	return 0;
}

// -----------------------------------------------------------------------
void AppBarWnd::snapAdjust(RECT *r, int way) 
{
	RECT s;
	Layout *l = static_cast<Layout*>(getInterface(layoutGuid));
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
	}
	else if (way == -1) {
		h -= s.top + s.bottom;
		w -= s.left + s.right;
		r->left += s.left;
		r->top += s.top;
		r->bottom = r->top + h;
		r->right = r->left + w;
	}
}

// -----------------------------------------------------------------------
void AppBarWnd::onRatioChanged() 
{
	APPBARWND_PARENT::onRatioChanged();
	if (m_side != APPBAR_NOTDOCKED) appbar_posChanged();
}