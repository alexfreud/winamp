#include <precomp.h>

#include <bfc/wasabi_std.h>
#include <bfc/wasabi_std_wnd.h>
#include <api/wnd/wndevent.h>

#include <bfc/bfc_assert.h>
#include <api/wnd/wndclass/tooltip.h>
#include <api/wnd/cursor.h>
#include <api/wnd/accessible.h>
#include <api/service/svcs/svc_accessibility.h>
#include <api/wnd/paintsets.h>
#include <api/wnd/PaintCanvas.h>

#ifdef _WIN32
#include <shellapi.h>	// for HDROP
#endif
#include <tataki/canvas/bltcanvas.h>

#define DESKTOPALPHA
#define REFRESH_RATE 25
#define DRAWTIMERID 125

#include <api/wnd/basewnd.h>
#include <api/wnd/usermsg.h>

#include <api/wnd/paintcb.h>
#include <tataki/canvas/canvas.h>
#include <bfc/file/filename.h>
#include <tataki/region/region.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/scriptguid.h>
#include <api/wnd/notifmsg.h>
#include <api/metrics/metricscb.h>

#include <api/wndmgr/gc.h>
#include <api/wndmgr/layout.h>

namespace Agave
{
	#include "../Agave/Config/api_config.h"
}



//#define TIP_TIMER_ID        1601
#define TIP_DESTROYTIMER_ID 1602
#define TIP_AWAY_ID         1603
#define TIP_AWAY_DELAY       100

#define TIP_TIMER_THRESHOLD  350
#define TIP_LENGTH          3000

#define VCHILD_TIMER_ID_MIN	2000
#define VCHILD_TIMER_ID_MAX	2100

#define BUFFEREDMSG_TIMER_ID 1604

#define DEFERREDCB_INVALIDATE   0x201 // move to .h
#define DEFERREDCB_FOCUSFIRST   0x202 // move to .h
#define DC_KILLGHOST            0x204

#ifdef _WIN32
#define WM_DEFER_CALLBACK (WM_USER+0x333)
#endif
class DragSet : public PtrList<void>, public NamedW {};

//CUT? static void register_wndClass(HINSTANCE);

//CUT? #define ROOTSTRING "RootWnd"

//CUT? #define BASEWNDCLASSNAME "BaseWindow_" ROOTSTRING

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x20A
#endif

static ifc_window *stickyWnd;
static RECT sticky;

static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;

/*api_window *api_window::rootwndFromPoint(POINT &point, int level) {
  api_window *wnd;
  wnd = WASABI_API_WND->rootWndFromPoint(&point);
  return api_window::rootwndFromRootWnd(wnd, level, &point);
}

api_window *api_window::rootwndFromRootWnd(api_window *wnd, int level, POINT *point) {

  for (;;) {
    if (wnd == NULL || level < 0) return NULL;
    if (point) {
      RECT r;
      wnd->getWindowRect(&r);
      if (!PtInRect(&r, *point)) return NULL; // PORT ME
    }
    if (level == 0) return wnd;
    wnd = wnd->getRootWndParent();
    level--;
  }
  // should never get here
}*/

static BOOL DisabledWindow_OnMouseClick(HWND hwnd)
{
	DWORD windowStyle = (DWORD)GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (WS_DISABLED != ((WS_CHILD | WS_DISABLED) & windowStyle))
		return FALSE;

	HWND hActive = GetActiveWindow();
	HWND hPopup = GetWindow(hwnd, GW_ENABLEDPOPUP);

	BOOL beepOk = (hPopup == hActive || hwnd == GetWindow(hActive, GW_OWNER));
	if (!beepOk && NULL == hPopup)
	{
		for (HWND hWalker = hwnd; ;)
		{											
			hWalker = GetWindow(hWalker, GW_OWNER);
			if (NULL == hWalker || (0 != (WS_CHILD & GetWindowLongPtrW(hWalker, GWL_STYLE))))
				break;
			if (hActive == GetWindow(hWalker, GW_ENABLEDPOPUP))
			{
				beepOk = TRUE;
				break;
			}
		}
	}
	
	if (beepOk)
	{	
		static const GUID accessibilityConfigGroupGUID = 
		{ 0xe2e7f4a, 0x7c51, 0x478f, { 0x87, 0x74, 0xab, 0xbc, 0xf6, 0xd5, 0xa8, 0x57 } };

		#define GetBoolConfig(__group, __itemName, __default)\
			((NULL != (__group)) && NULL != (item = group->GetItem(__itemName)) ? item->GetBool() : (__default))

		waServiceFactory *serviceFactory = WASABI_API_SVC->service_getServiceByGuid(Agave::AgaveConfigGUID);
		Agave::api_config *config = (NULL != serviceFactory) ? (Agave::api_config *)serviceFactory->getInterface() : NULL;
		Agave::ifc_configgroup *group = (NULL != config) ? config->GetGroup(accessibilityConfigGroupGUID) : NULL;
		Agave::ifc_configitem *item;
				
		if (GetBoolConfig(group, L"modalflash", true))
		{
			FLASHWINFO flashInfo;
			flashInfo.cbSize = sizeof(FLASHWINFO);
			flashInfo.hwnd = hActive;
			flashInfo.dwFlags = FLASHW_CAPTION;
			flashInfo.uCount = 2;
			flashInfo.dwTimeout = 100;
			FlashWindowEx(&flashInfo);
		}

		if (GetBoolConfig(group, L"modalbeep", false))
			MessageBeep(MB_OK);

		if (NULL != config)
			serviceFactory->releaseInterface(config);
	}
	else
	{		
		for (HWND hWalker = hwnd; NULL == hPopup;)
		{											
			hWalker = GetWindow(hWalker, GW_OWNER);
			if (NULL == hWalker || (0 != (WS_CHILD & GetWindowLongPtrW(hWalker, GWL_STYLE))))
				break;
			hPopup = GetWindow(hWalker, GW_ENABLEDPOPUP);
		}

		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		if (NULL != hPopup && hPopup != hwnd)
		{
			BringWindowToTop(hPopup);
			SetActiveWindow(hPopup);
		}
	}

	return TRUE;
}
int WndWatcher::viewer_onItemDeleted(ifc_dependent *item)
{
	if (item == dep)
	{
		dep = NULL;
		watcher->wndwatcher_onDeleteWindow(watched);
		watched = NULL;
	}
	return 1;
}

BaseWnd::BaseWnd()
{
	uiwaslocked = 0;
	m_takenOver = 0;
	rootfocus = NULL;
	rootfocuswatcher.setWatcher(this);
	alwaysontop = 0;
	customdefaultcursor = NULL;
	preventcancelcapture = 0;
	ratiolinked = 1;
	deleting = 0;
	hinstance = NULL;
	hwnd = NULL;
	parentWnd = NULL;
	dragging = 0;
	prevtarg = NULL;
	inputCaptured = 0;
	btexture = NULL;
	postoninit = 0;
	inited = 0;
	skipnextfocus = 0;
	ncb = FALSE;
	accessible = NULL;

	tooltip = NULL;
	tip_done = FALSE;
	tipshowtimer = FALSE;
	tipawaytimer = FALSE;
	tipdestroytimer = FALSE;
	start_hidden = 0;
	notifyWindow = NULL;
	lastClick[0] = 0;
	lastClick[1] = 0;
	lastClickP[0].x = 0;
	lastClickP[0].y = 0;
	lastClickP[1].x = 0;
	lastClickP[1].y = 0;
	destroying = FALSE;

	curVirtualChildCaptured = NULL;
	curVirtualChildFocus = NULL;

	virtualCanvas = NULL; virtualCanvasH = virtualCanvasW = 0;
	deferedInvalidRgn = NULL;

	hasfocus = 0;
	focus_on_click = 1;
	lastnullregion = 0;
	ratio = 1;
	lastratio = 1;
	rwidth = rheight = 0;
	skin_id = -1;
	wndalpha = 255;
	activealpha = 255;
	inactivealpha = 255;
	w2k_alpha = 0; //FUCKO
	scalecanvas = NULL;
	clickthrough = 0;

	mustquit = 0;
	returnvalue = 0;
	notifyid = 0;
	cloaked = 0;
	disable_tooltip_til_recapture = 0;

	subtractorrgn = NULL;
	composedrgn = NULL;
	wndregioninvalid = 1;
	regionop = REGIONOP_NONE;
	rectrgn = 1;
	need_flush_cascaderepaint = 0;
	deferedCascadeRepaintRgn = NULL;
	this_visible = 0;
	this_enabled = 1;
	renderbasetexture = 0;
	oldCapture = NULL;
	my_guiobject = NULL;
	want_autoresize_after_init = 0;
	resizecount = 0;
	suggested_w = 320;
	suggested_h = 200;
	maximum_w = maximum_h = AUTOWH;
	minimum_w = minimum_h = AUTOWH;
	rx = 0;
	ry = 0;
	rwidth = 0;
	rheight = 0;
	allow_deactivate = 1;
	minimized = 0;
	inonresize = 0;
#ifndef WA3COMPATIBILITY
	m_target = NULL;
#endif

	nodoubleclick = noleftclick = norightclick = nomousemove = nocontextmnu = 0;
	focusEventsEnabled = 1;
	maximized = 0;
	MEMSET(&restore_rect, 0, sizeof(RECT));
	ghostbust = 0;

	lastActiveWnd = NULL;
}

BaseWnd::~BaseWnd()
{
	//ASSERT(virtualChildren.getNumItems() == 0);
	childtabs.deleteAll();
	if (WASABI_API_WND && WASABI_API_WND->getModalWnd() == this) WASABI_API_WND->popModalWnd(this);
	destroying = TRUE;
	curVirtualChildFocus = NULL;
#ifdef _WIN32
	if (inputCaptured && GetCapture() == getOsWindowHandle()) ReleaseCapture();
#else
#warning port me
#endif

	for (int i = 0;i < ghosthwnd.getNumItems();i++)
		Wasabi::Std::Wnd::destroyWnd(ghosthwnd.enumItem(i));

	if (hwnd != NULL && !m_takenOver)
	{
#ifdef URLDROPS
		if (acceptExternalDrops()) Wasabi::Std::Wnd::revokeDragNDrop(hwnd /*, &m_target*/);
#else
#ifndef WA3COMPATIBILITY
		if (m_target != NULL)
		{
			Wasabi::Std::Wnd::revokeDragNDrop(hwnd);
		}
#endif
#endif
		int popact = !wantActivation();
		if (popact) WASABI_API_WND->appdeactivation_push_disallow(this);

		Wasabi::Std::Wnd::destroyWnd(hwnd);

		if (popact) WASABI_API_WND->appdeactivation_pop_disallow(this);
	}

	deleteFrameBuffer(virtualCanvas);
	virtualCanvas = NULL;
	delete scalecanvas;
	scalecanvas = NULL;

	resetDragSet();

	notifyParent(ChildNotify::DELETED);
	if (tipdestroytimer)
		killTimer(TIP_DESTROYTIMER_ID);
	if (tipshowtimer)
	{
		// TODO: on the mac, use CreateMouseTrackingRegion
		TRACKMOUSEEVENT tracker;
		tracker.cbSize=sizeof(tracker);
		tracker.dwFlags = TME_HOVER|TME_CANCEL;
		tracker.hwndTrack = this->getOsWindowHandle();
		tracker.dwHoverTime = TIP_TIMER_THRESHOLD;

		TrackMouseEvent(&tracker);

	}
	if (tipawaytimer)
		killTimer(TIP_AWAY_ID);

	destroyTip();

	delete tooltip;

	if (uiwaslocked)
		killTimer(BUFFEREDMSG_TIMER_ID);

	if (deferedInvalidRgn)
		delete deferedInvalidRgn;

	delete composedrgn;
	delete subtractorrgn;
	delete deferedCascadeRepaintRgn;

	if (parentWnd != NULL)
		parentWnd->unregisterRootWndChild(this);

	if (!m_takenOver && WASABI_API_WND) WASABI_API_WND->unregisterRootWnd(this);
	hwnd = NULL;
}

int BaseWnd::init(ifc_window *parWnd, int nochild)
{
	if (parWnd == NULL)
		return 0;

	OSWINDOWHANDLE phwnd = parWnd->getOsWindowHandle();
	ASSERT(phwnd != NULL);

	parentWnd = parWnd;	// set default parent wnd
	int ret = init(parWnd->getOsModuleHandle(), phwnd, nochild);

	if (!ret)
		parentWnd = NULL;	// abort

	return ret;
}

int BaseWnd::init(OSMODULEHANDLE moduleHandle, OSWINDOWHANDLE parent, int nochild)
{
	RECT r;
	int w, h;

	ASSERTPR(getOsWindowHandle() == NULL, "don't you double init you gaybag");

	hinstance = moduleHandle;

#ifdef _WIN32
	ASSERT(hinstance != NULL);
#endif

	//CUT  register_wndClass(hinstance);

	if (parent != NULL)
	{
		Wasabi::Std::Wnd::getClientRect(parent, &r);
	}
	else
	{
		Wasabi::Std::setRect(&r, 0, 0, getPreferences(SUGGESTED_W), getPreferences(SUGGESTED_H));
	}

	w = (r.right - r.left);
	h = (r.bottom - r.top);

	rwidth  = w;
	rheight = h;
	rx      = r.left;
	ry      = r.top;

	int popact = !wantActivation();
	if (popact) WASABI_API_WND->appdeactivation_push_disallow(this);

	//CUThwnd = createWindow(r.left, r.top, w, h, nochild, parent, hinstance);
	hwnd = Wasabi::Std::Wnd::createWnd(&r, nochild, acceptExternalDrops(), parent, hinstance, static_cast<ifc_window*>(this));
#ifdef __APPLE__
#warning remove me
	Wasabi::Std::Wnd::showWnd(hwnd);
#endif

	if (popact) WASABI_API_WND->appdeactivation_pop_disallow(this);

	//ASSERT(hwnd != NULL); // lets fail nicely, this could happen for some win32 reason, we don't want to fail the whole app for it, so lets just fail the wnd
	if (hwnd == NULL) return 0;

	if (wantActivation()) bringToFront();

	//CUT  nreal++;

	//FUCKO
#ifdef _WIN32 // PORT ME
#ifdef URLDROPS
	if (acceptExternalDrops()) RegisterDragDrop(hwnd, &m_target);
#else
#ifndef WA3COMPATIBILITY
	if (!m_target && WASABI_API_WND != NULL)
		m_target = WASABI_API_WND->getDefaultDropTarget();
	if (m_target != NULL)
	{
		RegisterDragDrop(hwnd, (IDropTarget *)m_target);
	}
#endif
#endif
#endif

	this_visible = 0;

	onInit();

	this_visible = !start_hidden;

	onPostOnInit();

	return 1;
}

#ifndef WA3COMPATIBILITY
void BaseWnd::setDropTarget(void *dt)
{
#ifdef _WIN32
	if (isVirtual()) return ;
	if (isInited() && m_target != NULL)
	{
		Wasabi::Std::Wnd::revokeDragNDrop(getOsWindowHandle());
		m_target = NULL;
	}
	m_target = dt;
	if (m_target != NULL && isInited())
	{
		RegisterDragDrop(gethWnd(), (IDropTarget *)m_target);
	}
#else
#warning port me
#endif
}

void *BaseWnd::getDropTarget()
{
	return m_target;
}
#endif

int BaseWnd::onInit()
{

	const wchar_t *s = getName();
	if (s != NULL)
		setOSWndName(s);

	inited = 1;

	if (getParent())
		getParent()->registerRootWndChild(this);

	if (WASABI_API_WND != NULL)
		WASABI_API_WND->registerRootWnd(this);

#ifdef _WIN32
	if (!Wasabi::Std::Wnd::isDesktopAlphaAvailable())
		w2k_alpha = 0; //FUCKO

	if (w2k_alpha)
	{
		setLayeredWindow(1);
	}

	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
		WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");

#endif

	return 0;
}

int BaseWnd::onPostOnInit()
{
	postoninit = 1; // from now on, isInited() returns 1;
	if (want_autoresize_after_init) onResize();
	else invalidateWindowRegion();
	if (isVisible()) onSetVisible(1);
	if (getTabOrder() == -1) setAutoTabOrder();
	ifc_window *dp = getDesktopParent();
	if ((dp == NULL || dp == this) && WASABI_API_TIMER != NULL)
		postDeferredCallback(DEFERREDCB_FOCUSFIRST, 0, 500);
	return 0;
}

void BaseWnd::setLayeredWindow(int i)
{
	if (!Wasabi::Std::Wnd::isValidWnd(getOsWindowHandle())) return ;
	if (!isInited()) return ;
	Wasabi::Std::Wnd::setLayeredWnd(getOsWindowHandle(), i);
#if 0//CUT
	if (i)
	{
		SetWindowLong(getOsWindowHandle(), GWL_EXSTYLE, GetWindowLong(getOsWindowHandle(), GWL_EXSTYLE) & ~WS_EX_LAYERED);
		SetWindowLong(getOsWindowHandle(), GWL_EXSTYLE, GetWindowLong(getOsWindowHandle(), GWL_EXSTYLE) | WS_EX_LAYERED);
	}
	else
	{
		SetWindowLong(getOsWindowHandle(), GWL_EXSTYLE, GetWindowLong(getOsWindowHandle(), GWL_EXSTYLE) & ~WS_EX_LAYERED);
	}
#endif
	setTransparency(-1);
}

int BaseWnd::getCursorType(int x, int y)
{
	if (!customdefaultcursor)
		return BASEWND_CURSOR_POINTER;
	return BASEWND_CURSOR_USERSET;
}

void BaseWnd::onSetName()
{
	if (isInited() && !isVirtual())
		Wasabi::Std::Wnd::setWndName(getOsWindowHandle(), getNameSafe());
	notifyParent(ChildNotify::NAMECHANGED);
	if (accessible)
		accessible->onSetName(getName());
}

OSWINDOWHANDLE BaseWnd::getOsWindowHandle()
{
	OSWINDOWHANDLE handle;

	if ( isVirtual() )
		handle = getParent()->getOsWindowHandle();
	else
		handle = hwnd;
	

	return handle;
}

OSMODULEHANDLE BaseWnd::getOsModuleHandle()
{
	return hinstance;
}

void BaseWnd::onTip()
{
	tipshowtimer = FALSE;
	tip_done = TRUE;

	POINT p;
	Wasabi::Std::getMousePos(&p);
	if (WASABI_API_WND->rootWndFromPoint(&p) == (ifc_window *)this)
	{
		createTip();
		setTimer(TIP_DESTROYTIMER_ID, TIP_LENGTH);
		tipdestroytimer = TRUE;
	}
	setTimer(TIP_AWAY_ID, TIP_AWAY_DELAY);
	tipawaytimer = TRUE;
}

void BaseWnd::timerCallback(int id)
{
	switch (id)
	{
	case BUFFEREDMSG_TIMER_ID:
		checkLockedUI();
		break;
//	case TIP_TIMER_ID:
		//onTip();
		//break;
	case TIP_DESTROYTIMER_ID:
		killTimer(TIP_DESTROYTIMER_ID);
		killTimer(TIP_AWAY_ID);
		tipawaytimer = FALSE;
		tipdestroytimer = FALSE;
		destroyTip();
		break;
	case TIP_AWAY_ID:
		onTipMouseMove();
		break;
	}
}

int BaseWnd::isInited()
{
	return inited;
}

int BaseWnd::isDestroying()
{
	return destroying;
}

int BaseWnd::wantSiblingInvalidations()
{
	return FALSE;
}

void BaseWnd::setRSize(int x, int y, int w, int h)
{
	rwidth = w;
	rheight = h;
	rx = x;
	ry = y;
}

void BaseWnd::resize(int x, int y, int w, int h, int wantcb)
{
	inonresize = 1;

	if (x == AUTOWH) x = NOCHANGE;
	if (y == AUTOWH) y = NOCHANGE;
	if (w == AUTOWH) w = NOCHANGE;
	if (h == AUTOWH) h = NOCHANGE;

	if (getNumMinMaxEnforcers() > 0)
	{
		int min_w = getPreferences(MINIMUM_W);
		int min_h = getPreferences(MINIMUM_H);
		int max_w = getPreferences(MAXIMUM_W);
		int max_h = getPreferences(MAXIMUM_H);
		if (min_w != AUTOWH && w != NOCHANGE && w < min_w) w = min_w;
		if (max_w != AUTOWH && w != NOCHANGE && w > max_w) w = max_w;
		if (min_h != AUTOWH && h != NOCHANGE && h < min_h) h = min_h;
		if (max_h != AUTOWH && h != NOCHANGE && h > max_h) h = max_h;
	}

	int noresize = (w == NOCHANGE && h == NOCHANGE);
	int nomove = (x == NOCHANGE && y == NOCHANGE)/* || (x == rx && y == ry)*/;
	if (x == NOCHANGE) x = rx;
	if (y == NOCHANGE) y = ry;
	if (w == NOCHANGE) w = rwidth;
	if (h == NOCHANGE) h = rheight;

#ifdef _DEBUG
	ASSERT(x < 0xFFF0);
	ASSERT(y < 0xFFF0);
	ASSERT(w < 0xFFF0);
	ASSERT(h < 0xFFF0);
#endif

	double thisratio = getRenderRatio();
	int different_ratio = (lastratio != thisratio);
	lastratio = thisratio;

	int noevent = (resizecount > 1 && w == rwidth && h == rheight);
	//ifc_window *dp = getDesktopParent();
	if (different_ratio == 1 && noevent == 1)
	{
		if (Wasabi::Std::Wnd::getTopmostChild(getOsWindowHandle()) != INVALIDOSWINDOWHANDLE)
			noevent = 0;
		invalidateWindowRegion();
	}

	RECT oldsize, newsize = Wasabi::Std::makeRect(x, y, w, h);
	if (hwnd != NULL)
		BaseWnd::getNonClientRect(&oldsize);
	else
		oldsize = newsize;

	setRSize(x, y, w, h);

	if (handleRatio() && renderRatioActive())
	{
		multRatio(&w, &h);
		if (getParent() != NULL)
		{
			multRatio(&x, &y);
		}
	}

	if (!noevent)
	{
		if (wantcb && isPostOnInit())
		{
			resizecount = MIN(5, ++resizecount);
			if (!isVirtual())
				invalidateWindowRegion();
			onResize();
			if (ensureWindowRegionValid())
				updateWindowRegion();
		}
	}

	if (getOsWindowHandle() != NULL)
	{

		RECT oldsizescaled;
		getWindowRect(&oldsizescaled);
		RECT newsizescaled = {x, y, x + w, y + h};
		if (MEMCMP(&newsizescaled, &oldsizescaled, sizeof(RECT)))
		{
			//CUT      SetWindowPos(getOsWindowHandle(), NULL, x, y, w, h,
			//CUT      SWP_NOZORDER |
			//CUT      SWP_NOACTIVATE |
			//CUT      (!wantRedrawOnResize() ? SWP_NOCOPYBITS: 0) |
			//CUT      (ncb ? SWP_NOCOPYBITS : 0) |
			//CUT      ( nomove ? SWP_NOMOVE : 0) |
			//CUT      ( noresize ? SWP_NOSIZE : 0) |
			//CUT      0);
			Wasabi::Std::Wnd::setWndPos( getOsWindowHandle(), NULL, x, y, w, h, TRUE, TRUE, !wantRedrawOnResize() || ncb, nomove, noresize );
		}
		//else
		//{
		//	DebugStringW(L"BaseWnd::resize optimized\n");
		//}

		onAfterResize();

		if (ncb)
			invalidate();
		else
		{
			RECT r;
			if (hwnd != NULL)
			{
				if (newsize.left == oldsize.left && newsize.top == oldsize.top)
				{
					if (newsize.right > oldsize.right)
					{
						// growing in width
						r.left = oldsize.right;
						r.right = newsize.right;
						r.top = newsize.top;
						r.bottom = newsize.bottom;
						invalidateRect(&r);
						if (newsize.bottom > oldsize.bottom)
						{
							// growing in width & height
							r.left = oldsize.left;
							r.right = newsize.right;
							r.top = oldsize.bottom;
							r.bottom = newsize.bottom;
							invalidateRect(&r);
						}
					}
					else if (newsize.bottom > oldsize.bottom)
					{
						if (newsize.bottom > oldsize.bottom)
						{
							// growing in height
							r.left = oldsize.left;
							r.right = newsize.right;
							r.top = oldsize.bottom;
							r.bottom = newsize.bottom;
							invalidateRect(&r);
						}
					}
				}
			}
		}
	}
	inonresize = 0;
}

void BaseWnd::forcedOnResizeChain(ifc_window *w)
{
	w->triggerEvent(TRIGGER_ONRESIZE);
	int n = w->getNumRootWndChildren();
	for (int i = 0;i < n;i++)
	{
		forcedOnResizeChain(w->enumRootWndChildren(i));
	}
}

int BaseWnd::forcedOnResize()
{
	forcedOnResizeChain(this);
	return 1;
}

int BaseWnd::onResize()
{
	if (!isVirtual() || (getRegionOp() != REGIONOP_NONE))
		invalidateWindowRegion();
	// you are not supposed to call onResize until after onInit has returned. If what you wanted was to generate
	// an onResize event to do some custom client coordinates recalculations (ie: to apply on your children)
	// then you don't need to do anything since onResize is going to be called after onInit() is done. If you still want to
	// trigger it because your code might be called by onInit and after onInit, use isPostOnInit() as a test.
	// if what you wanted was to signal a object that you just resized it, then you don't need to do anything beside
	// resize(...), it will generate the event on its own if the window is inited, and will defer to until after onInit
	// if it is not.
	// shortly put: do not call onResize before or inside onInit()
	// if you have any valid reason for doing that, i'd like to know about it so i can make it possible. -FG
#ifdef _DEBUG
	if (!isPostOnInit())
	{
		//__asm int 3;
		ASSERTPR(isPostOnInit(), "do not call onResize before or inside onInit()");
	}
#endif
	return FALSE;
}

void BaseWnd::resizeToClient(BaseWnd *wnd)
{
	if (wnd != NULL)
		wnd->resize(&clientRect());
}

int BaseWnd::onPostedMove()
{
	/*
	if (w2k_alpha && Wasabi::Std::Wnd::isDesktopAlphaAvailable() && !cloaked)
	{
		RECT r;
		getWindowRect(&r);
	   Wasabi::Std::Wnd::moveLayeredWnd(hwnd, r.left, r.top);
	}*/
	return FALSE;
}

void BaseWnd::resize(RECT *r, int wantcb)
{
	resize(r->left, r->top, r->right - r->left, r->bottom - r->top, wantcb);
}

void BaseWnd::move(int x, int y)
{
	//DebugStringW( L"BaseWnd::move( x = %d, y = %d )\n", x, y );

	setRSize(x, y, rwidth, rheight);
	Wasabi::Std::Wnd::setWndPos( getOsWindowHandle(), NULL, x, y, 0, 0, TRUE, TRUE, ncb, FALSE, TRUE );
	//CUT  if (!ncb)
	//CUT    SetWindowPos(getOsWindowHandle(), NULL, x, y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_DEFERERASE);
	//CUT  else
	//CUT    SetWindowPos(getOsWindowHandle(), NULL, x, y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOCOPYBITS|SWP_NOACTIVATE|SWP_DEFERERASE);
}

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
BOOL CALLBACK EnumOwnedTopMostWindows(HWND hwnd, LPARAM lParam)
{
	enumownedstruct *st = (enumownedstruct *)lParam;
	if (hwnd != st->hthis && GetWindow(hwnd, GW_OWNER) == st->owner)
	{
		ifc_window *w = (ifc_window*)GetWindowLong(hwnd, GWL_USERDATA);
		if (w != NULL && w->getAlwaysOnTop())
			st->hlist->addItem(w);
	}
	return TRUE;
}

void BaseWnd::saveTopMosts()
{
	HWND owner = GetWindow(getOsWindowHandle(), GW_OWNER);
	enumownedstruct st;
	ontoplist.removeAll();
	if (owner != NULL)
	{
		st.owner = owner;
		st.hlist = &ontoplist;
		st.hthis = getOsWindowHandle();
		EnumWindows(EnumOwnedTopMostWindows, (long)&st);
	}
}

void BaseWnd::restoreTopMosts()
{
	HWND owner = GetWindow(getOsWindowHandle(), GW_OWNER);
	if (owner != NULL)
	{
		for (int i = 0;i < ontoplist.getNumItems();i++)
		{
			ontoplist.enumItem(i)->setAlwaysOnTop(1);
		}
	}
}
#endif

void BaseWnd::bringToFront()
{
	// when we set a window to the top of the zorder (not topmost), win32 finds the owner and removes any topmost flag its children may
	// have because it assumes we want this window over these, which we definitly don't. so we need to first go thru all the owner's children,
	// make a list of the ones with a topmost flag, set this window on top, and set the topmost flags back. yay
	ASSERT(!isVirtual());
#ifdef EXPERIMENTAL_INDEPENDENT_AOT
	saveTopMosts();
#endif
	//CUT  SetWindowPos(getOsWindowHandle(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE|SWP_DEFERERASE|SWP_NOOWNERZORDER);
	Wasabi::Std::Wnd::bringToFront(getOsWindowHandle());
#ifdef EXPERIMENTAL_INDEPENDENT_AOT
	restoreTopMosts();
#endif
}

void BaseWnd::bringToBack()
{
	ASSERT(!isVirtual());
#ifdef EXPERIMENTAL_INDEPENDENT_AOT
	saveTopMosts();
#endif
	//CUT  SetWindowPos(getOsWindowHandle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE|SWP_DEFERERASE|SWP_NOOWNERZORDER);
	Wasabi::Std::Wnd::sendToBack(getOsWindowHandle());
#ifdef EXPERIMENTAL_INDEPENDENT_AOT
	restoreTopMosts();
#endif
}

void BaseWnd::setVisible(int show)
{
	int visible = isVisible(1);
	if (!!visible == !!show) return ;
	invalidate();
	this_visible = !!show;
	/*if (!getParent() || getParent() == WASABI_API_WND->main_getRootWnd() && IsWindow(getOsWindowHandle())) {
	  if (!show) {
	    setLayeredWindow(0);
	    if (setLayeredWindowAttributes)
	      setLayeredWindowAttributes(hwnd, RGB(0,0,0), 255, LWA_ALPHA);
	  } else {
	    setLayeredWindow(w2k_alpha);
	  }
	}*/
	if (!getParent() || getParent() == WASABI_API_WND->main_getRootWnd() || getParent()->isVisible())
	{
		onSetVisible(show);
	}
}

void BaseWnd::setCloaked(int cloak)
{
	if (cloaked == cloak) return ;
	cloaked = cloak;
	if (isVirtual()) return ;
	if (cloaked)
	{
		//CUTif (IsWindowVisible(getOsWindowHandle()))
		//CUT  ShowWindow(getOsWindowHandle(), SW_HIDE);
		if (Wasabi::Std::Wnd::isWndVisible(getOsWindowHandle()))
			Wasabi::Std::Wnd::hideWnd(getOsWindowHandle());
	}
	else
	{
		if (isVisible(1))
			//CUTShowWindow(getOsWindowHandle(), SW_NORMAL);
			Wasabi::Std::Wnd::showWnd(getOsWindowHandle());
	}
}


void BaseWnd::onSetVisible(int show)
{
	/* for debug purposes - don't delete please
	  #include "../../../studio/container.h"
	  #include "../../../studio/layout.h"
	  if (!show && getGuiObject() && STRCASEEQLSAFE(getGuiObject()->guiobject_getId(), "normal")) {
	    Layout *l = (Layout *)getInterface(layoutGuid);
	    if (l) {
	      if (l->getParentContainer() && STRCASEEQLSAFE(l->getParentContainer()->getId(), "main")) {
	        DebugString("Hiding main player\n");
	      }
	    }
	  }*/
	if (!isVirtual())
		if (hwnd != NULL)
			if (!cloaked)
			{
				//CUT //      SetWindowPos(getOsWindowHandle(),NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_SHOWWINDOW);
				//CUT        ShowWindow(getOsWindowHandle(), show ? SW_SHOWNA : SW_HIDE);
				if (show)
					Wasabi::Std::Wnd::showWnd(getOsWindowHandle(), TRUE);
				else
					Wasabi::Std::Wnd::hideWnd(getOsWindowHandle());
			}
	/*  if (!show)
	    postDeferredCallback(0x7849);
	  else {*/
	foreach(rootwndchildren)
	ifc_window *w = rootwndchildren.getfor();
	if (w && w->isVisible(1)) // check internal flag only
		w->onSetVisible(show);
	endfor;
	dependent_sendEvent(BaseWnd::depend_getClassGuid(), Event_SETVISIBLE, show);
	//}
	/*  if (getDesktopParent() == this) {
	    cascadeRepaint(0);
	  }*/

	/*#ifdef WIN32 // os-specific non virtual child wnd support
	  if (!isVirtual()) {
	    HWND w = GetWindow(getOsWindowHandle(), GW_CHILD);
	    while (w != NULL) {
	      api_window *rootwnd = (api_window*)GetWindowLong(w, GWL_USERDATA);
	      if (rootwnd && rootwnd != this)
	        if (rootwnd->isInited())
	          rootwnd->onSetVisible(show);
	      w = GetWindow(w, GW_HWNDNEXT);
	    }
	  }
	#endif*/
	if (!isVirtual())
	{
		if (!show)
		{
			deferedInvalidate();
			delete virtualCanvas;
			virtualCanvas = NULL;
		}
	}
	invalidateWindowRegion();
}

void BaseWnd::setEnabled(int en)
{
	int enabled = isEnabled(1);
	if (!!enabled == !!en) return ;
	invalidate();
	this_enabled = !!en;
	if (!getParent() || getParent() == WASABI_API_WND->main_getRootWnd() || getParent()->isEnabled())
	{
		onEnable(en);
	}
}

int BaseWnd::isEnabled(int within)
{
	if (!isVirtual() && !getOsWindowHandle()) return 0;
	if (!this_enabled) return 0;

	if (within) return this_enabled; // whatever, local

	if (isVirtual()) // virtual, global
		if (getParent())
			return getParent()->isEnabled();
		else
			return 0;

	// non virtual, global
	//CUT  if (GetWindowLong(getOsWindowHandle(), GWL_STYLE) & WS_POPUP) return this_enabled;
	if (Wasabi::Std::Wnd::isPopup(getOsWindowHandle())) return this_enabled;
	//CUT  if (!Wasabi::Std::Wnd::isValidWnd(GetParent(gethWnd()))) return this_enabled;
	if (!Wasabi::Std::Wnd::isValidWnd(Wasabi::Std::Wnd::getParent(getOsWindowHandle()))) return this_enabled;
	if (getParent()) return getParent()->isEnabled(); // not a popup, check its parent or fail
	return this_enabled;
}

int BaseWnd::onEnable(int en)
{
	if (!isVirtual())
	{
		if (hwnd != NULL)
			//CUT      EnableWindow(getOsWindowHandle(), en);
			Wasabi::Std::Wnd::setEnabled(getOsWindowHandle(), en);
		foreach(rootwndchildren)
		ifc_window *w = rootwndchildren.getfor();
		if (w->isEnabled(1)) // check internal flag only
			w->onEnable(en);
		endfor;
	}
	return 1;
}

void BaseWnd::setFocus()
{
	if (curVirtualChildFocus != NULL)
	{
		curVirtualChildFocus->onKillFocus();
		curVirtualChildFocus = NULL;
	}
	onSetRootFocus(this);
	//CUT  SetFocus(getOsWindowHandle());
	Wasabi::Std::Wnd::setFocus(getOsWindowHandle());
}

void BaseWnd::setFocusOnClick(int f)
{
	focus_on_click = f;
}

api_region *BaseWnd::getDeferedInvalidRgn()
{
	return deferedInvalidRgn;
}

void BaseWnd::deferedInvalidate()
{
	if (!hasVirtualChildren() || !isVisible(1)) return ;
	RECT r = Wasabi::Std::makeRect(0, 0, 0, 0);
	getNonClientRect(&r);
	deferedInvalidateRect(&r);
}

void BaseWnd::deferedInvalidateRect(RECT *r)
{
	if (!hasVirtualChildren()) return ;
	RegionI h(r);
	deferedInvalidateRgn(&h);
}

void BaseWnd::deferedInvalidateRgn(api_region *h)
{
	if (!hasVirtualChildren()) return ;
	if (!deferedInvalidRgn)
	{
		deferedInvalidRgn = new RegionI();
	}

	deferedInvalidRgn->addRegion(h);
}

void BaseWnd::deferedValidate()
{
	if (!hasVirtualChildren() || !isVisible(1)) return ;
	RECT r = Wasabi::Std::makeRect(0,0,0,0);
	getNonClientRect(&r);
	deferedValidateRect(&r);
}

void BaseWnd::deferedValidateRect(RECT *r)
{
	if (!hasVirtualChildren()) return ;
	RegionI h(r);
	deferedValidateRgn(&h);
}

void BaseWnd::deferedValidateRgn(api_region *h)
{
	if (!hasVirtualChildren()) return ;
	if (!deferedInvalidRgn) return ;

	deferedInvalidRgn->subtractRgn(h);
}

int BaseWnd::hasVirtualChildren()
{
	return 1; //virtualChildren.getNumItems() > 0;
}

void BaseWnd::invalidate()
{
	invalidateFrom(this);
}

void BaseWnd::invalidateFrom(ifc_window *who)
{
	if (hasVirtualChildren()) deferedInvalidate();
	//CUT  if (hwnd != NULL && isVisible(1)) InvalidateRect(getOsWindowHandle(), NULL, FALSE);
	if (hwnd != NULL && isVisible(1))
		Wasabi::Std::Wnd::invalidateRect(getOsWindowHandle());
}

void BaseWnd::invalidateRectFrom(RECT *r, ifc_window *who)
{
	if (hasVirtualChildren()) deferedInvalidateRect(r);
	RegionI rg(r);
	invalidateRgnFrom(&rg, who);
}

void BaseWnd::invalidateRgn(api_region *r)
{
	invalidateRgnFrom(r, this);
}

void BaseWnd::invalidateRect(RECT *r)
{
	invalidateRectFrom(r, this);
}

void BaseWnd::invalidateRgnFrom(api_region *r, ifc_window *who)
{
	if (parentWnd) parentWnd->onChildInvalidate(r, who);
	PaintCallbackInfoI pc(NULL, r);
	dependent_sendEvent(BaseWnd::depend_getClassGuid(), Event_ONINVALIDATE, 0, &pc);
	if (hwnd != NULL && isVisible(1))
	{
		if (hasVirtualChildren())
		{
			api_region *_r = r->clone();
			int j = virtualChildren.searchItem(who);
			for (int i = 0;i < virtualChildren.getNumItems();i++)
			{
				ifc_window *w = virtualChildren[i];
				if (w != who && w->wantSiblingInvalidations())
					w->onSiblingInvalidateRgn(_r, who, j, i);
			}

			deferedInvalidateRgn(_r);
			physicalInvalidateRgn(_r);
			r->disposeClone(_r);
		}
		else
		{
			deferedInvalidateRgn(r);
			physicalInvalidateRgn(r);
		}
	}
}

void BaseWnd::physicalInvalidateRgn(api_region *r)
{
	if (hwnd != NULL && isVisible(1))
	{
		if (renderRatioActive())
		{
			api_region *clone = r->clone();
			clone->scale(getRenderRatio(), getRenderRatio(), TRUE);
			//CUT      InvalidateRgn(getOsWindowHandle(), clone->getOSHandle(), FALSE);
			Wasabi::Std::Wnd::invalidateRegion(getOsWindowHandle(), clone->getOSHandle());
			r->disposeClone(clone);
		}
		else
			//CUT      InvalidateRgn(getOsWindowHandle(), r->getOSHandle(), FALSE);
			Wasabi::Std::Wnd::invalidateRegion(getOsWindowHandle(), r->getOSHandle());
	}
}

void BaseWnd::validate()
{
	//CUT  if (hwnd != NULL) ValidateRect(getOsWindowHandle(), NULL);
	if (hwnd != NULL)
		Wasabi::Std::Wnd::validateRect(getOsWindowHandle());
}

void BaseWnd::validateRect(RECT *r)
{
	if (hwnd != NULL)
	{
		if (renderRatioActive())
		{
			RECT r2 = *r;
			Wasabi::Std::scaleRect(&r2, getRenderRatio());
			//CUT      ValidateRect(getOsWindowHandle(), &r2);
			Wasabi::Std::Wnd::validateRect(getOsWindowHandle(), &r2);
		}
		else
			//CUT    ValidateRect(getOsWindowHandle(), r);
			Wasabi::Std::Wnd::validateRect(getOsWindowHandle(), r);
	}
}

void BaseWnd::validateRgn(api_region *reg)
{
	if (hwnd != NULL)
	{
		if (renderRatioActive())
		{
			api_region *clone = reg->clone();
			clone->scale(getRenderRatio(), getRenderRatio(), TRUE);
			//CUT      ValidateRgn(getOsWindowHandle(), clone->getOSHandle());
			Wasabi::Std::Wnd::validateRegion(getOsWindowHandle(), clone->getOSHandle());
			reg->disposeClone(clone);
		}
		else
			//CUT      ValidateRgn(getOsWindowHandle(), reg->getOSHandle());
			Wasabi::Std::Wnd::validateRegion(getOsWindowHandle(), reg->getOSHandle());
	}
}

void BaseWnd::repaint()
{
	/*	if (hasVirtualChildren())	{
		  api_region *h = new api_region();
		  int s = GetUpdateRgn(getOsWindowHandle(), h->getHRGN(), FALSE);
		  if (s != NULLREGION && s != ERROR) {
	  		virtualDrawRgn(h);
		  }
		  delete h;
		}*/
	//CUTif (hwnd != NULL) UpdateWindow(getOsWindowHandle());
	if (hwnd != NULL)
		Wasabi::Std::Wnd::update(getOsWindowHandle());
}

void BaseWnd::getClientRect(RECT *rect)
{
	/*  rect->left = rx;
	  rect->right = rx + rwidth;
	  rect->top = ry;
	  rect->bottom = ry + rheight;*/
	//ASSERT(hwnd != NULL);
	if (!Wasabi::Std::Wnd::isValidWnd(hwnd))
	{
		MEMSET(rect, 0, sizeof(RECT));
		return ;
	}

	GetClientRect(getOsWindowHandle(), rect);
	////Wasabi::Std::Wnd::getClientRect(getOsWindowHandle(), rect);
	rect->right = rect->left + rwidth;
	rect->bottom = rect->top + rheight;
}

RECT BaseWnd::clientRect()
{
	RECT ret;
	getClientRect(&ret);
	return ret;
}

void BaseWnd::getNonClientRect(RECT *rect)
{
	//  ASSERT(hwnd != NULL);
	if (!hwnd)
		getClientRect(rect);
	else
	{
		Wasabi::Std::Wnd::getClientRect(getOsWindowHandle(), rect);
		if (getRenderRatio() != 1.0)
		{
			rect->right = rect->left + rwidth;
			rect->bottom = rect->left + rheight;
		}
	}
	/*  rect->left = rx;
	  rect->right = rx + rwidth;
	  rect->top = ry;
	  rect->bottom = ry + rheight;*/
}

RECT BaseWnd::nonClientRect()
{
	RECT ret;
	getNonClientRect(&ret);
	return ret;
}

void BaseWnd::getWindowRect(RECT *rect)
{
	//CUT#ifdef WIN32
	//CUT  ASSERT(hwnd != NULL);
	//CUT  GetWindowRect(getOsWindowHandle(), rect);
	//CUT#else
	//CUT#error port me
	//CUT#endif
	Wasabi::Std::Wnd::getWindowRect(getOsWindowHandle(), rect);
}

// get position relative to parent (same coordinate system for basewnd & virtualwnd)
void BaseWnd::getPosition(POINT *pt)
{
	pt->x = rx;
	pt->y = ry;
}

void *BaseWnd::dependent_getInterface(const GUID *classguid)
{
	HANDLEGETINTERFACE(ifc_window);
	//CUT  HANDLEGETINTERFACE(api_window);
	return NULL;
}

RECT BaseWnd::windowRect()
{
	RECT ret;
	getWindowRect(&ret);
	return ret;
}


void BaseWnd::clientToScreen(int *x, int *y)
{
	int _x = x ? *x : 0;
	int _y = y ? *y : 0;
	if (renderRatioActive())
	{
		_x = (int)((double)_x * getRenderRatio());
		_y = (int)((double)_y * getRenderRatio());
	}
	Wasabi::Std::Wnd::clientToScreen(getOsWindowHandle(), &_x, &_y);
	if (x) *x = _x;
	if (y) *y = _y;
}

void BaseWnd::clientToScreen(RECT *r)
{
	clientToScreen((int*)&r->left, (int*)&r->top);
	clientToScreen((int*)&r->right, (int*)&r->bottom);
}

void BaseWnd::clientToScreen(POINT *p)
{
	clientToScreen((int *)&p->x, (int *)&p->y);
}

void BaseWnd::screenToClient(int *x, int *y)
{
	//CUT  POINT p;
	int _x = x ? *x : 0;
	int _y = y ? *y : 0;
	//CUT  ScreenToClient(getOsWindowHandle(), &p);
	Wasabi::Std::Wnd::screenToClient(getOsWindowHandle(), &_x, &_y);
	if (renderRatioActive())
	{
		_x = (int)((double)_x / getRenderRatio());
		_y = (int)((double)_y / getRenderRatio());
	}
	if (x) *x = _x;
	if (y) *y = _y;
}

void BaseWnd::screenToClient(RECT *r)
{
	screenToClient((int*)&r->left, (int*)&r->top);
	screenToClient((int*)&r->right, (int*)&r->bottom);
}

void BaseWnd::screenToClient(POINT *p)
{
	screenToClient((int *)&p->x, (int *)&p->y);
}

void BaseWnd::setParent(ifc_window *newparent)
{
	ASSERTPR(newparent != NULL, "quit being a weeny");
	ASSERTPR(parentWnd == NULL || newparent == parentWnd, "can't reset parent");
	parentWnd = newparent;
	if (isInited())
	{
		OSWINDOWHANDLE w1 = getOsWindowHandle();
		OSWINDOWHANDLE w2 = newparent->getOsWindowHandle();
		if (w1 != w2)
			//CUT      SetParent(w1, w2);
			Wasabi::Std::Wnd::setParent(w1, w2);
	}
}

//FUCKO
int BaseWnd::reparent(ifc_window *newparent)
{
#ifdef _WIN32
	if (!isVirtual())
	{
		if (isInited())
		{
			ifc_window *old = getParent();
			if (!old && newparent)
			{
				::SetParent(getOsWindowHandle(), newparent->getOsWindowHandle());
				SetWindowLong(getOsWindowHandle() , GWL_STYLE, GetWindowLong(getOsWindowHandle(), GWL_STYLE) & ~WS_POPUP);
				SetWindowLong(getOsWindowHandle() , GWL_STYLE, GetWindowLong(getOsWindowHandle(), GWL_STYLE) | WS_CHILD);
			}
			else if (old && !newparent)
			{
				SetWindowLong(getOsWindowHandle() , GWL_STYLE, GetWindowLong(getOsWindowHandle(), GWL_STYLE) & ~WS_CHILD);
				SetWindowLong(getOsWindowHandle() , GWL_STYLE, GetWindowLong(getOsWindowHandle(), GWL_STYLE) | WS_POPUP);
				::SetParent(getOsWindowHandle(), NULL);
			}
			else
			{
				::SetParent(getOsWindowHandle(), newparent ? newparent->getOsWindowHandle() : NULL);
			}
		}
	}

	parentWnd = newparent;
	onSetParent(newparent);

#ifdef WASABI_ON_REPARENT
	WASABI_ON_REPARENT(getOsWindowHandle());
#endif
#else
#warning port me
#endif
	return 1;
}

ifc_window *BaseWnd::getParent()
{
	return parentWnd;
}

ifc_window *BaseWnd::getRootParent()
{
	return this;
}

//PORTME
ifc_window *BaseWnd::getDesktopParent()
{
#ifdef _WIN32
	// NONPORTABLE
	HWND w = getOsWindowHandle();
	HWND last = w;
	if (!w) return NULL;
	HWND p = w;
	wchar_t cn[256] = {0};
	while (p && !(GetWindowLong(p, GWL_STYLE) & WS_POPUP))
	{
		GetClassNameW(p, cn, 255); cn[255] = 0;
		if (!wcscmp(cn, BASEWNDCLASSNAME))
			last = p;
		p = GetParent(p);
	}
	if (p)
	{
		GetClassNameW(p, cn, 255); cn[255] = 0;
		if (!wcscmp(cn, BASEWNDCLASSNAME))
			return (ifc_window*)GetWindowLongPtrW(p, GWLP_USERDATA);
		else if (last != NULL)
			return (ifc_window*)GetWindowLongPtrW(last, GWLP_USERDATA);
	}
#else
#warning port me
#endif
	return NULL;
}

int BaseWnd::notifyParent(int msg, int param1, int param2)
{
	ifc_window *notifywnd = getNotifyWindow();
	if (getParent() == NULL && notifywnd == NULL) return 0;
	if (notifywnd == NULL) notifywnd = getParent();
	ASSERT(notifywnd != NULL);
	return notifywnd->childNotify(this, msg, param1, param2);
}

int BaseWnd::passNotifyUp(ifc_window *child, int msg, int param1, int param2)
{
	// Same code as above to decide for whom we should notify.
	ifc_window *notifywnd = getNotifyWindow();
	if (getParent() == NULL && notifywnd == NULL) return 0;
	if (notifywnd == NULL) notifywnd = getParent();
	ASSERT(notifywnd != NULL);
	// And here we just change the api_window pointer.
	return notifywnd->childNotify(child, msg, param1, param2);
}

void BaseWnd::setNotifyId(int id)
{
	notifyid = id;
}

int BaseWnd::getNotifyId()
{
	return notifyid;
}

DragInterface *BaseWnd::getDragInterface()
{
	return this;
}

ifc_window *BaseWnd::rootWndFromPoint(POINT *pt)
{
	// pt is in client coordinates
	int x = (int)((double)pt->x / getRenderRatio());
	int y = (int)((double)pt->y / getRenderRatio());

	ifc_window *ret = findRootWndChild(x, y);
	if (ret == NULL) ret = this;
	return ret;
}

int BaseWnd::rootwnd_paintTree(ifc_canvas *canvas, api_region *r)
{
	BaseCloneCanvas c(canvas);
	return paintTree(&c, r);
}

const wchar_t *BaseWnd::getRootWndName()
{
	return getName();
}

const wchar_t *BaseWnd::getId()
{
	return NULL;
}

void BaseWnd::setSkinId(int id)
{
	skin_id = id;
}

void BaseWnd::setPreferences(int what, int v)
{
	switch (what)
	{
	case MAXIMUM_W: maximum_w = v; break;
	case MAXIMUM_H: maximum_h = v; break;
	case MINIMUM_W: minimum_w = v; break;
	case MINIMUM_H: minimum_h = v; break;
	case SUGGESTED_W: suggested_w = v; break;
	case SUGGESTED_H: suggested_h = v; break;
	}
}

int BaseWnd::getPreferences(int what)
{
	if (getNumMinMaxEnforcers() > 0)
	{

		int min_x = minimum_w, min_y = minimum_h, max_x = maximum_w, max_y = maximum_h, sug_x = suggested_w, sug_y = suggested_h;

		for (int i = 0;i < getNumMinMaxEnforcers();i++)
		{

			int tmin_x = MINIMUM_W, tmin_y = MINIMUM_H, tmax_x = MAXIMUM_W, tmax_y = MAXIMUM_H, tsug_x = SUGGESTED_W, tsug_y = SUGGESTED_H;

			ifc_window *w = enumMinMaxEnforcer(i);

			if (w)
			{

				tmin_x = w->getPreferences(MINIMUM_W);
				tmin_y = w->getPreferences(MINIMUM_H);
				tmax_x = w->getPreferences(MAXIMUM_W);
				tmax_y = w->getPreferences(MAXIMUM_H);
				tsug_x = w->getPreferences(SUGGESTED_W);
				tsug_y = w->getPreferences(SUGGESTED_H);

				if (tmin_x == -1) tmin_x = AUTOWH;
				if (tmin_y == -1) tmin_y = AUTOWH;
				if (tmax_x == -1) tmax_x = AUTOWH;
				if (tmax_y == -1) tmax_y = AUTOWH;
				if (tsug_x == -1) tsug_x = AUTOWH;
				if (tsug_y == -1) tsug_y = AUTOWH;

#ifndef DISABLE_SYSFONTSCALE
				TextInfoCanvas textInfoCanvas(this);
				double fontScale = textInfoCanvas.getSystemFontScale();
				GuiObject *o = static_cast<GuiObject *>(getInterface(guiObjectGuid));
				if (o != NULL)
				{
					if (o->guiobject_getAutoSysMetricsW())
					{
						if (tmin_x != AUTOWH) tmin_x = (int)((float)tmin_x * fontScale);
						if (tmax_x != AUTOWH) tmax_x = (int)((float)tmax_x * fontScale);
						if (tsug_x != AUTOWH) tsug_x = (int)((float)tsug_x * fontScale);
					}
					if (o->guiobject_getAutoSysMetricsH())
					{
						if (tmin_y != AUTOWH) tmin_y = (int)((float)tmin_y * fontScale);
						if (tmax_y != AUTOWH) tmax_y = (int)((float)tmax_y * fontScale);
						if (tsug_y != AUTOWH) tsug_y = (int)((float)tsug_y * fontScale);
					}
				}
#endif

				RECT cor;
				w->getNonClientRect(&cor);
				RECT wr;
				getNonClientRect(&wr);

				int xdif = (wr.right - wr.left) - (cor.right - cor.left);
				int ydif = (wr.bottom - wr.top) - (cor.bottom - cor.top);
				if (tmin_x != AUTOWH) tmin_x += xdif;
				if (tmin_y != AUTOWH) tmin_y += ydif;
				if (tmax_x != AUTOWH) tmax_x += xdif;
				if (tmax_y != AUTOWH) tmax_y += ydif;
				if (tsug_x != AUTOWH) tsug_x += xdif;
				if (tsug_y != AUTOWH) tsug_y += ydif;
			}

			if (min_x != AUTOWH) min_x = (tmin_x != AUTOWH) ? MAX(min_x, tmin_x) : min_x; else min_x = tmin_x;
			if (max_x != AUTOWH) max_x = (tmax_x != AUTOWH) ? MAX(max_x, tmax_x) : max_x; else max_x = tmax_x;
			if (min_y != AUTOWH) min_y = (tmin_y != AUTOWH) ? MAX(min_y, tmin_y) : min_y; else min_y = tmin_y;
			if (max_y != AUTOWH) max_y = (tmax_y != AUTOWH) ? MAX(max_y, tmax_y) : max_y; else max_y = tmax_y;
			if (sug_x != AUTOWH) sug_x = (tsug_x != AUTOWH) ? MAX(sug_x, tsug_x) : sug_x; else sug_x = tsug_x;
			if (sug_y != AUTOWH) sug_y = (tsug_y != AUTOWH) ? MAX(sug_y, tsug_y) : sug_y; else sug_y = tsug_y;
		}

		if (min_x != AUTOWH && min_x == max_x) sug_x = min_x;
		if (min_y != AUTOWH && min_y == max_y) sug_y = min_y;

		switch (what)
		{
		case MINIMUM_W: return min_x;
		case MINIMUM_H: return min_y;
		case MAXIMUM_W: return max_x;
		case MAXIMUM_H: return max_y;
		case SUGGESTED_W: return sug_x;
		case SUGGESTED_H: return sug_y;
		}
	}

	switch (what)
	{
	case SUGGESTED_W: return suggested_w;
	case SUGGESTED_H: return suggested_h;
	case MAXIMUM_W: return maximum_w;
	case MAXIMUM_H: return maximum_h;
	case MINIMUM_W: return minimum_w;
	case MINIMUM_H: return minimum_h;
	}

	return AUTOWH;
}

void BaseWnd::setStartHidden(int wtf)
{
	start_hidden = wtf;
}

//PORTME
#ifdef _WIN32



#define EQUAL_CLSNAME(__name1, __name2)\
(CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),\
NORM_IGNORECASE, (__name1), -1, (__name2), -1))

static BOOL BaseWnd_IsFrameWindow(HWND hwnd)
{
	WCHAR szClass[64] = {0};
	if (NULL == hwnd || !GetClassNameW(hwnd, szClass, ARRAYSIZE(szClass)))
		return FALSE;

	return EQUAL_CLSNAME(szClass, L"Winamp v1.x") || 
			EQUAL_CLSNAME(szClass, L"BaseWindow_RootWnd");
}


LRESULT BaseWnd::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!isDestroying()) switch (uMsg)
		{
		case WM_DEFER_CALLBACK:
			timerclient_onDeferredCallback(wParam, lParam);
			break;
		case WM_SYSCOMMAND:
		{
			if ((wParam & 0xfff0) == SC_SCREENSAVE || (wParam & 0xfff0) == SC_MONITORPOWER)
			{
				ifc_window *main = WASABI_API_WND->main_getRootWnd();
				if (main && main != this)
					return SendMessageW(main->gethWnd(), uMsg, wParam, lParam);
			}
			break;
		}
		//CUT    case WM_CREATE:
		//CUT      hwnd = hWnd;
		//CUT    break;

		//CUT    case WM_CLOSE:
		//CUT      return 0;

		case WM_PAINT:
		{
			if (inonresize && !wantRedrawOnResize()) return 1;
			ASSERT(hwnd != NULL);
			if (!isVisible(1) || IsIconic(hWnd)) break;
			RECT r;
			if (GetUpdateRect(hWnd, &r, FALSE))
			{
				if (virtualOnPaint())
				{
					return 0;
				}
			}
		}
		break;
		
		case WM_PRINTCLIENT:
			{
				bool old_cloaked = (!!cloaked);
				cloaked = true;
				DCCanvas dc((HDC)wParam, this);
				paint(&dc, 0);
				cloaked = old_cloaked;

				if (lParam & PRF_CHILDREN)
				{
					RECT wnd_size;
					GetWindowRect(hwnd, &wnd_size);

					HWND child = GetWindow(hwnd, GW_CHILD);
					while (child != NULL) 
					{
						if (GetWindowLongPtrW(child, GWL_STYLE) & WS_VISIBLE) 
						{
							RECT child_size;
							GetWindowRect(child, &child_size);
							if (child_size.right && child_size.bottom)
							{
								BltCanvas bitmap(child_size.right, child_size.bottom, child);;
								SendMessageW(child, WM_PRINT, (WPARAM)bitmap.getHDC(),  PRF_CHILDREN | PRF_CLIENT | PRF_NONCLIENT/*| PRF_OWNED*/);
								//bitmap->makeAlpha(255);

								//set alpha to 255
								int w, h;
								bitmap.getDim(&w, &h, NULL);
								ARGB32 *m_pBits = (ARGB32 *)bitmap.getBits();
								int nwords = w*h;
								for (; nwords > 0; nwords--, m_pBits++)
								{
									unsigned char *pixel = (unsigned char *)m_pBits;
									pixel[3] = 255;	// alpha
								}

								POINT offset;
								offset.x = child_size.left - wnd_size.left;
								offset.y = child_size.top - wnd_size.top;

								//BLENDFUNCTION blendFn;
								//blendFn.BlendOp = AC_SRC_OVER;
								//blendFn.BlendFlags  = 0;
								//blendFn.SourceConstantAlpha  = 255;
								//blendFn.AlphaFormat = 0;
								//AlphaBlend((HDC)wParam, offset.x, offset.y, child_size.right-child_size.left, child_size.bottom-child_size.top, 
								//	bitmap->getHDC(), 0, 0, child_size.right-child_size.left, child_size.bottom-child_size.top, blendFn);
								StretchBlt((HDC)wParam, offset.x, offset.y, child_size.right-child_size.left, child_size.bottom-child_size.top, 
									bitmap.getHDC(), 0, 0, child_size.right-child_size.left, child_size.bottom-child_size.top, SRCCOPY);
							}
						}
						child = GetWindow(child, GW_HWNDNEXT);
					}

				}
			}
			return 0;
		//CUT    case WM_NCPAINT: return 0;
		//CUT    case WM_SYNCPAINT: return 0;

		case WM_SETCURSOR:
			if (checkModal()) return TRUE;
			if (hWnd == (HWND)wParam)
			{
				DWORD windowStyle = (DWORD)GetWindowLongPtrW(hWnd, GWL_STYLE);
				switch(HIWORD(lParam))
				{
					case WM_LBUTTONDOWN:
					case WM_RBUTTONDOWN:
					case WM_MBUTTONDOWN:
					case 0x020B/*WM_XBUTTONDOWN*/:
						DisabledWindow_OnMouseClick(hWnd);
						break;
				}
				int ct = BASEWND_CURSOR_POINTER;
				int _x, _y;
				Wasabi::Std::getMousePos(&_x, &_y);
				screenToClient(&_x, &_y);
				OSCURSORHANDLE c = NULL;

				if (0 == (WS_DISABLED & windowStyle))
				{					
					if (!handleVirtualChildMsg(WM_SETCURSOR, _x, _y, &ct, &c))
					{
						ct = getCursorType(_x, _y);
					}
				}
				wchar_t *wincursor = NULL;
				switch (ct)
				{
					case BASEWND_CURSOR_USERSET:

						if (c == NULL)
							c = getCustomCursor(_x, _y);
						if (c != NULL)
						{
							SetCursor(c);
							return TRUE;
						}
						else wincursor = IDC_ARROW; // Ensure to have at least a cursor
						break;
					case BASEWND_CURSOR_POINTER:
						wincursor = IDC_ARROW;
						break;
					case BASEWND_CURSOR_NORTHSOUTH:
						wincursor = IDC_SIZENS;
						break;
					case BASEWND_CURSOR_EASTWEST:
						wincursor = IDC_SIZEWE;
						break;
					case BASEWND_CURSOR_NORTHWEST_SOUTHEAST:
						wincursor = IDC_SIZENWSE;
						break;
					case BASEWND_CURSOR_NORTHEAST_SOUTHWEST:
						wincursor = IDC_SIZENESW;
						break;
					case BASEWND_CURSOR_4WAY:
						wincursor = IDC_SIZEALL;
						break;
					case BASEWND_CURSOR_EDIT:
						wincursor = IDC_IBEAM;
						break;
					default:
						wincursor = IDC_ARROW;
						break;
				}
				if (wincursor != NULL) 
				{
					SetCursor(LoadCursor(NULL, wincursor));
					return TRUE;
				}
			}
			return FALSE;

		case WM_TIMER:
			timerCallback((int)wParam);
			return 0;

		case WM_GETOBJECT:
			if (lParam == OBJID_CLIENT)
			{
				Accessible *acc = getAccessibleObject();
				if (acc != NULL)
				{
					LRESULT lAcc = acc->getOSHandle((int)wParam);
					return lAcc;
				}
			}
			break;  // Fall through to DefWindowProc


		case WM_SETFOCUS:
			if (!focusEventsEnabled) break;
			if (isInited())
			{
				if (rootfocus != NULL && rootfocus != this)
				{
					if (rootfocus != curVirtualChildFocus)
						rootfocus->setFocus();
					break;
				}
				else
				{
					if (wantFocus())
					{
						onGetFocus();
						break;
					}
					else
					{
						ifc_window *w = getTab(TAB_GETFIRST);
						if (w != NULL)
						{
							w->setFocus();
						}
					}
				}
			}
			break;

		case WM_KILLFOCUS:
		{
			ifc_window *rp = getRootParent();
			if (!WASABI_API_WND->rootwndIsValid(rp) || !Wasabi::Std::Wnd::isValidWnd(rp->getOsWindowHandle())) break;
			if (!focusEventsEnabled) break;
#ifdef WASABI_COMPILE_WND
			if (WASABI_API_WND) WASABI_API_WND->forwardOnKillFocus(); // resets the keyboard active keys buffer
#endif
			if (!WASABI_API_WND->rootwndIsValid(curVirtualChildFocus)) curVirtualChildFocus = NULL;
			if (curVirtualChildFocus)
			{
				curVirtualChildFocus->onKillFocus();
				curVirtualChildFocus = NULL;
			}
			else
				if (hasfocus) onKillFocus();
			break;
		}

		// dragging and dropping

		case WM_LBUTTONDOWN:
		{
			if (lParam == 0xdeadc0de)
				return 1;

			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam))
				return 0;

			WASABI_API_WND->popupexit_check(this);

			if (checkModal())
				return 0;

			abortTip();

			int xPos = (signed short)LOWORD(lParam);
			int yPos = (signed short)HIWORD(lParam);

			xPos = (int)((float)xPos / getRenderRatio());
			yPos = (int)((float)yPos / getRenderRatio());
			
			if (!getCapture() && hasVirtualChildren() && handleVirtualChildMsg(WM_LBUTTONDOWN, xPos, yPos))
				return 0;

			if (isEnabled() && !dragging)
			{
				autoFocus(this);

				int r = 0;

				if (wantLeftClicks())
					r = onLeftButtonDown(xPos, yPos);

				if (checkDoubleClick(uMsg, xPos, yPos) && wantDoubleClicks() && onLeftButtonDblClk(xPos, yPos))
					return 0;
				
				return r;
			}
		}
		break;

		case WM_RBUTTONDOWN:
		{
			if (lParam == 0xdeadc0de) return 1;
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			WASABI_API_WND->popupexit_check(this);
			if (checkModal()) return 0;
			abortTip();
			int xPos = (signed short)LOWORD(lParam);
			int yPos = (signed short)HIWORD(lParam);
			xPos = (int)((float)xPos / getRenderRatio());
			yPos = (int)((float)yPos / getRenderRatio());
			if (!getCapture() && hasVirtualChildren())
				if (handleVirtualChildMsg(WM_RBUTTONDOWN, xPos, yPos))
					return 0;
			if (isEnabled() && !dragging)
			{
				autoFocus(this);
				int r = 0;
				if (wantRightClicks())
					r = onRightButtonDown(xPos, yPos);
				if (checkDoubleClick(uMsg, xPos, yPos) && wantDoubleClicks()) if (onRightButtonDblClk(xPos, yPos)) return 0;
				return r;
			}
		}
		break;
		case WM_MOUSEHOVER:
			if (checkModal()) return 0;
			if (!getCapture() && hasVirtualChildren())
				if (handleVirtualChildMsg(WM_MOUSEHOVER, 0, 0))
					return 0;
			break;
		case WM_MOUSEMOVE:
		{
			/*      static int mm=0;
			      DebugString("mousemove %d\n", mm++);*/
			if (checkModal()) return 0;
			int xPos = (signed short)LOWORD(lParam);
			int yPos = (signed short)HIWORD(lParam);
			xPos = (int)((float)xPos / getRenderRatio());
			yPos = (int)((float)yPos / getRenderRatio());
			if (dragging)
			{
				POINT pt = {xPos, yPos};
				clientToScreen(&pt);
				ifc_window *targ;
				int candrop = 0;
				// find the window the mouse is over

				targ = NULL;
				if (stickyWnd)
				{
					RECT wr;
					GetWindowRect(stickyWnd->getOsWindowHandle(), &wr);
					if (pt.x >= wr.left - sticky.left &&
					    pt.x <= wr.right + sticky.right &&
					    pt.y >= wr.top - sticky.top &&
					    pt.y <= wr.bottom + sticky.bottom) targ = stickyWnd;
					else stickyWnd = NULL;
				}

				if (targ == NULL && WASABI_API_WND) targ = WASABI_API_WND->rootWndFromPoint(&pt); // FG> not to self, check

				DI prevtargdi(prevtarg);
				DI targdi(targ);

				if (prevtarg != targ)
				{
					// window switch
					if (prevtarg != NULL) prevtargdi.dragLeave(this);
					if (targ != NULL) targdi.dragEnter(this);
				}
				if (targ != NULL)
					candrop = targdi.dragOver(pt.x, pt.y, this);
				if (targ == NULL || !candrop)
					SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_NO)));
				else
					SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_APPSTARTING)));
				prevtarg = targ;
			}
			else if (isEnabled())
			{
				tipbeenchecked = FALSE;
				if (!getCapture() && hasVirtualChildren())
				{
					if (handleVirtualChildMsg(WM_MOUSEMOVE, xPos, yPos))
						return 0;
				}
				if (getCapture())
				{
					if (wantMouseMoves())
						if (onMouseMove(xPos, yPos))
							return 0;
				}
				if (!tipbeenchecked) onTipMouseMove();
				return 0;
			}
		}
		break;

		case WM_LBUTTONUP:
		{
			if (lParam == 0xdeadc0de) return 1;
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			if (checkModal()) return 0;
			int xPos = (signed short)LOWORD(lParam);
			int yPos = (signed short)HIWORD(lParam);
			xPos = (int)((float)xPos / getRenderRatio());
			yPos = (int)((float)yPos / getRenderRatio());
			abortTip();
			if (!dragging && !getCapture() && hasVirtualChildren())
			{
				if (handleVirtualChildMsg(WM_LBUTTONUP, xPos, yPos))
					return 0;
			}
			if (dragging)
			{
				clientToScreen(&xPos, &yPos);
				int res = 0;
				if (prevtarg != NULL)
				{
					res = DI(prevtarg).dragDrop(this, xPos, yPos);
				}

				// inform source what happened
				dragComplete(res);

				resetDragSet();
				prevtarg = NULL;
				stickyWnd = NULL;
				suggestedTitle = NULL;
				SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
				Wasabi::Std::Wnd::releaseCapture();
				dragging = 0;
			}
			else if (isEnabled())
			{
				if (wantLeftClicks())
					if (onLeftButtonUp(xPos, yPos)) return 0;
			}
		}
		break;

		case WM_RBUTTONUP:
		{
			if (lParam == 0xdeadc0de) return 1;
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			if (checkModal()) return 0;
			abortTip();
			int xPos = (signed short)LOWORD(lParam);
			int yPos = (signed short)HIWORD(lParam);
			xPos = (int)((float)xPos / getRenderRatio());
			yPos = (int)((float)yPos / getRenderRatio());
			if (!getCapture() && hasVirtualChildren())
			{
				if (handleVirtualChildMsg(WM_RBUTTONUP, xPos, yPos))
					return 0;
			}
			if (isEnabled() && !dragging)
			{
				if (wantRightClicks())
					if (onRightButtonUp(xPos, yPos)) return 0;
			}
		}
		break;

		case WM_CONTEXTMENU:
		{
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			if (checkModal()) return 0;
			ASSERT(hWnd != NULL);
			int xPos = (signed short)LOWORD(lParam);
			int yPos = (signed short)HIWORD(lParam);
			if (hWnd == getOsWindowHandle())
			{
				if (wantContextMenus())
					if (onContextMenu(xPos, yPos)) return 0;
			}
			else if (GetParent(hWnd) == getOsWindowHandle())
			{
				if (wantContextMenus())
					if (onChildContextMenu(xPos, yPos)) return 0;
			}
		}
		break;

		case WM_ERASEBKGND:
			return (onEraseBkgnd((HDC)wParam));

		case WM_MOUSEWHEEL:
		{
			abortTip();

			int l, a;
			l = (short)HIWORD(wParam) / 120;
			a = (short)HIWORD(wParam);
			if (!l)
				if (a > 0) l = 1;
				else if (a < 0)l = 0;
			a = l >= 0 ? l : -l;
			if (GetAsyncKeyState(VK_MBUTTON)&0x8000)
			{
				if (l >= 0) l = 0; // Fast Forward 5s
				else l = 1; // Rewind 5s
			}
			else
			{
				if (l >= 0) l = 2; // Volume up
				else l = 3; // Volume down
			}

			int r = 0;

			if (l & 1)
				r = onMouseWheelDown(!(BOOL)(l & 2), a);
			else
				r = onMouseWheelUp(!(BOOL)(l & 2), a);
			if (r == 0)
			{
				r = WASABI_API_WND->forwardOnMouseWheel(l, a);
			}
			// if it wasn't handled by this wnd, nor by the api, send it to the main wnd, unless we're it
			if (r == 0)
			{
				if (WASABI_API_WND->main_getRootWnd() != this)
					r = (int)SendMessageW(WASABI_API_WND->main_getRootWnd()->gethWnd(), uMsg, wParam, lParam);
			}

			return r;
		}

		case WM_WA_RELOAD:
		{
			if (wParam == 0)
				freeResources();
			else
				reloadResources();
			return 0;
		}

		case WM_WA_GETFBSIZE:
		{
			SIZE *s = (SIZE *)wParam;
			s->cx = rwidth;
			s->cy = rheight;
			return 0;
		}

		case WM_USER + 8976:  // wheel in tip, delete tip
			abortTip();
			return 0;

		case WM_CHAR:
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			if (WASABI_API_WND->interceptOnChar((TCHAR) wParam)) return 0;
			if (curVirtualChildFocus == NULL)
			{
				if (onChar(((TCHAR) wParam))) return 0;
			}
			else
			{
				if (curVirtualChildFocus->onChar(((TCHAR) wParam))) return 0;
			}
			if (WASABI_API_WND && WASABI_API_WND->forwardOnChar(this, (TCHAR) wParam, (int)lParam)) return 0;
			break;

		case WM_KEYDOWN:
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			if (WASABI_API_WND->interceptOnKeyDown((int) wParam)) return 0;
			if (curVirtualChildFocus == NULL)
			{
				if (onKeyDown((int) wParam)) return 0;
			}
			else
			{
				if (curVirtualChildFocus->onKeyDown((int)wParam)) return 0;
			}
			if (WASABI_API_WND && WASABI_API_WND->forwardOnKeyDown(this, (int) wParam, (int)lParam)) return 0;
			break;

		case WM_KEYUP:
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			if (WASABI_API_WND->interceptOnKeyUp((int) wParam)) return 0;
			if (curVirtualChildFocus == NULL)
			{
				if (onKeyUp((int) wParam)) return 0;
			}
			else
			{
				if (curVirtualChildFocus->onKeyUp((int)wParam)) return 0;
			}
			if (WASABI_API_WND && WASABI_API_WND->forwardOnKeyUp(this, (int) wParam, (int)lParam)) return 0;
			break;

		case WM_SYSKEYDOWN:
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			if (WASABI_API_WND->interceptOnSysKeyDown((int) wParam, (int)lParam)) return 0;
			if (curVirtualChildFocus == NULL)
			{
				if (onSysKeyDown((int) wParam, (int)lParam)) return 0;
			}
			else
			{
				if (curVirtualChildFocus->onSysKeyDown((int)wParam, (int)lParam)) return 0;
			}
			if (WASABI_API_WND && WASABI_API_WND->forwardOnSysKeyDown(this, (int) wParam, (int)lParam)) return 0;
			break;

		case WM_SYSKEYUP:
			if (bufferizeLockedUIMsg(uMsg, (int)wParam, (int)lParam)) return 0;
			if (WASABI_API_WND->interceptOnSysKeyUp((int) wParam, (int)lParam)) return 0;
			if (curVirtualChildFocus == NULL)
			{
				if (onSysKeyUp((int) wParam, (int)lParam)) return 0;
			}
			else
			{
				if (curVirtualChildFocus->onSysKeyUp((int)wParam, (int)lParam)) return 0;
			}
			if (WASABI_API_WND && WASABI_API_WND->forwardOnSysKeyUp(this, (int) wParam, (int)lParam)) return 0;
			break;

		case WM_MOUSEACTIVATE:
		{
			if (checkModal() || !wantActivation())
				return MA_NOACTIVATE;
			//SetFocus(getOsWindowHandle());
			return MA_ACTIVATE;
		}

		case WM_ACTIVATEAPP:

			if (wParam == FALSE)
			{
				
				if (WASABI_API_WND != NULL)
				{
					WASABI_API_WND->popupexit_signal();
					WASABI_API_SYSCB->syscb_issueCallback(SysCallback::GC, GarbageCollectCallback::GARBAGECOLLECT);
					WASABI_API_WND->kbdReset();
					if (ghosthwnd.getNumItems() > 0 && ghostbust)
					{
						ghostbust = 0; postDeferredCallback(DC_KILLGHOST);
					}
					return 0;
				}
			}
			
			break;

		case WM_ACTIVATE:
			switch(LOWORD(wParam))
			{
				case WA_ACTIVE:
				case WA_CLICKACTIVE: 
					if (WASABI_API_WND != NULL)
					WASABI_API_WND->popupexit_check(this);
					
					onActivate();

					if (WA_CLICKACTIVE == LOWORD(wParam))
					{
						POINT pt;
						DWORD pts = GetMessagePos();
						POINTSTOPOINT(pt, pts);
						MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
						HWND hTarget = ChildWindowFromPointEx(hwnd, pt, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);
						if (hTarget && hTarget != hwnd) lastActiveWnd = hTarget;
					}
					
					if (lastActiveWnd != hwnd && NULL != lastActiveWnd && IsWindow(lastActiveWnd))
					{
						SendMessageW(lastActiveWnd, uMsg, wParam, lParam);
						return 0;
					}
					break;
				default:

					onDeactivate();
				
					lastActiveWnd = GetFocus();
					
					if (NULL != lastActiveWnd && !IsChild(hwnd, lastActiveWnd))
						lastActiveWnd = NULL;
					
					{
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(*x))
#endif
						if (NULL != lastActiveWnd && !BaseWnd_IsFrameWindow(lastActiveWnd))
						{
							while (lastActiveWnd)
							{
								if (BaseWnd_IsFrameWindow(GetWindow(lastActiveWnd, GW_OWNER)))
									break;
								lastActiveWnd = GetAncestor(lastActiveWnd, GA_PARENT);
							}
						}
					}
				
					if (lastActiveWnd != hwnd && NULL != lastActiveWnd)
					{
						SendMessageW(lastActiveWnd, uMsg, wParam, lParam);
						return 0;
					}
				
					break;
			}
			break;
			
		case WM_NCACTIVATE:
			if (allowDeactivation())
				return TRUE;
			return FALSE;

		case WM_WINDOWPOSCHANGING:
		{
			if (!isVirtual() && Wasabi::Std::Wnd::isPopup(hwnd))
			{
				WINDOWPOS *wp = (WINDOWPOS *)lParam;
				if (wp->x != rx || wp->y != ry) wp->flags |= SWP_NOMOVE;
			}
		}
		break;

		case WM_WINDOWPOSCHANGED:
		{

			WINDOWPOS *lpwp = (WINDOWPOS *)lParam; // points to size and position data
			if (lpwp->flags & SWP_HIDEWINDOW)
			{
				minimized = 1;
				onMinimize();
			}
			else if (lpwp->flags & SWP_SHOWWINDOW)
			{
				minimized = 0;
				onRestore();
			}

			if (!inonresize)
			{
				int w = rwidth;
				int h = rheight;
				multRatio(&w, &h);
				if (lpwp->cx != w || lpwp->cy != h)
				{
					DebugStringW(L"external onResize\n");
					w = lpwp->cx;
					h = lpwp->cy;
					divRatio(&w, &h);
					setRSize(rx, ry, w, h);
					if (isPostOnInit())
						onResize();
				}
			}

			onPostedMove();
			return 0;
		}

		case WM_DROPFILES:
		{
			if (checkModal()) break;
			WASABI_API_WND->pushModalWnd();
			onExternalDropBegin();
			HDROP h = (HDROP)wParam;
			POINT dp = {0};
			DragQueryPoint(h, &dp);
			clientToScreen(&dp);
			// build a file list
			wchar_t buf[WA_MAX_PATH] = {0};
			PtrList<FilenamePS> keep;

			SetCursor(LoadCursor(NULL, IDC_WAIT));

			//CUT #if UTF8
			//CUT       // doesn't really need UTF8, the "buf" is never written to.
			//CUT       // made to be NULL to enforce this concept.
			int nfiles = DragQueryFile(h, 0xffffffff, NULL, 0);
			//CUT #else
			//CUT       int nfiles = DragQueryFile(h, 0xffffffff, buf, sizeof(buf));
			//CUT #endif

			// convert them all to PlayItem *'s
			for (int i = 0; i < nfiles; i++)
			{
				DragQueryFileW(h, i, buf, WA_MAX_PATH);
				addDroppedFile(buf, &keep);	// recursive

			}
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			dragging = 1;
			if (dragEnter(this))
			{
				if (dragOver(dp.x, dp.y, this)) dragDrop(this, dp.x, dp.y);
			}
			else
			{
				dragLeave(this);
#ifdef FORWARD_DRAGNDROP
				HWND w = WASABI_API_WND->main_getRootWnd()->gethWnd();
				SendMessageW(w, WM_DROPFILES, wParam, lParam);
#endif

			}
			dragging = 0;

			// remove data
			keep.deleteAll();
			resetDragSet();

			onExternalDropEnd();
			WASABI_API_WND->popModalWnd();
		}
		return 0;	// dropfiles

		case WM_CAPTURECHANGED:
			/*    static int cc=0;
			    DebugString("capture changed! %d\n", cc++);*/
			if (preventcancelcapture) return 0;
			inputCaptured = 0;
			if (curVirtualChildCaptured != NULL)
			{
				ifc_window *w = curVirtualChildCaptured;
				curVirtualChildCaptured = NULL;
				w->onCancelCapture();
			}
			else
			{
				onCancelCapture();
			}
			return 0;

		} //switch

	if (WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg && 
		WM_NULL != WINAMP_WM_DIRECT_MOUSE_WHEEL)
	{
		wndProc(hWnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}

	if (uMsg >= WM_USER)
	{
		int ret;
		if (onUserMessage(uMsg, (int)wParam, (int)lParam, &ret))
			return ret;
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
#endif
int BaseWnd::onUserMessage(int msg, int w, int l, int *r)
{
	return 0;
}

int BaseWnd::checkDoubleClick(int b, int x, int y)
{
#ifdef _WIN32
	uint32_t now = Wasabi::Std::getTickCount();

	switch (b)
	{
	case WM_LBUTTONDOWN:
		if (lastClick[0] > now - Wasabi::Std::getDoubleClickDelay())
		{
			lastClick[0] = 0;
			if (ABS(lastClickP[0].x - x) > Wasabi::Std::getDoubleClickX() || ABS(lastClickP[0].y - y) > Wasabi::Std::getDoubleClickY()) return 0;
			return 1;
		}
		lastClick[0] = now;
		lastClickP[0].x = x;
		lastClickP[0].y = y;
		break;

	case WM_RBUTTONDOWN:
		if (lastClick[1] > now - Wasabi::Std::getDoubleClickDelay())
		{
			lastClick[1] = 0;
			if (ABS(lastClickP[1].x - x) > Wasabi::Std::getDoubleClickX() || ABS(lastClickP[1].y - y) > Wasabi::Std::getDoubleClickY()) return 0;
			return 1;
		}
		lastClick[1] = now;
		lastClickP[1].x = x;
		lastClickP[1].y = y;
		break;
	}
#else
#warning port me
#endif
	return 0;
}

int BaseWnd::onMouseWheelUp(int click, int lines)
{
	return 0;
}

int BaseWnd::onMouseWheelDown(int click, int lines)
{
	return 0;
}

int BaseWnd::onContextMenu(int x, int y)
{
	return 0;
}

int BaseWnd::onChildContextMenu(int x, int y)
{
	return 0;
}

int BaseWnd::onDeferredCallback(intptr_t param1, intptr_t param2)
{
	switch (param1)
	{
	case DEFERREDCB_FLUSHPAINT:
		do_flushPaint();
		break;
	case DEFERREDCB_INVALIDATE:
		if (isPostOnInit())
			invalidate();
		break;
	case DC_KILLGHOST:
		if (ghosthwnd.getNumItems() > 0)
		{
			preventcancelcapture = 1;
			for (int i = 0;i < ghosthwnd.getNumItems();i++)
				Wasabi::Std::Wnd::destroyWnd(ghosthwnd.enumItem(i));
			ghosthwnd.freeAll();
			preventcancelcapture = 0;
		}
		break;
	case DEFERREDCB_FOCUSFIRST:
		assignRootFocus(NULL);
		if (Wasabi::Std::Wnd::getFocus() == getOsWindowHandle())
		{
			focusNext();
		}
		break;
	case 0x7849 /*DEFERREDCB_ONHIDE*/:
	{
		foreach(rootwndchildren)
		ifc_window *w = rootwndchildren.getfor();
		if (w->isVisible(1)) // check internal flag only
			w->onSetVisible(0);
		endfor;
		dependent_sendEvent(BaseWnd::depend_getClassGuid(), Event_SETVISIBLE, 0);
		break;
	}
	}
	return 0;
}

int BaseWnd::onPaint(Canvas *canvas)
{
#if 0
	// example:
	PaintCanvas c;
	if (!c.beginPaint(this)) return 0;
	(do some painting)
	c will self -destruct on return
#endif
	  if (renderbasetexture)
	{
		PaintCanvas paintcanvas;
		if (canvas == NULL)
		{
			if (!paintcanvas.beginPaint(this)) return 0;
			canvas = &paintcanvas;
		}
		RECT r;
		getNonClientRect(&r);
		RenderBaseTexture(canvas, r);
	}
	return 0;
}

int BaseWnd::onPaint(Canvas *canvas, api_region *h)
{
	if (!canvas) return onPaint(canvas);

#ifdef _WIN32
	int sdc = SaveDC(canvas->getHDC());
#elif defined(__APPLE__)
	CGContextSaveGState(canvas->getHDC());
#endif

	canvas->selectClipRgn(h);

	int rs = onPaint(canvas);

#ifdef _WIN32
	RestoreDC(canvas->getHDC(), sdc);
#elif defined(__APPLE__)
	CGContextRestoreGState(canvas->getHDC());
#endif

	return rs;
}

int BaseWnd::getTransparency()
{
	return wndalpha;
}

void BaseWnd::setTransparency(int amount)
{
	//if (wndalpha == amount) return;
	if (amount == 254) amount = 255;
	if (amount == 1) amount = 0;

	if (amount != -1) wndalpha = amount; else amount = wndalpha;

	if (!Wasabi::Std::Wnd::isDesktopAlphaAvailable())
	{
		wndalpha = 255;
		return ;
	}

	if (w2k_alpha)
	{
		invalidate();
		return ;
	}

#ifdef WIN32

	if (!isInited() || isVirtual()) return ;
	if (!Wasabi::Std::Wnd::isValidWnd(getOsWindowHandle())) return ;

	if (amount < -1) amount = 0;
	else if (amount > 255) amount = 255;

	//CUT  DWORD dwLong = GetWindowLong(hwnd, GWL_EXSTYLE);
	if (amount == 255 && !forceTransparencyFlag())
	{
		Wasabi::Std::Wnd::setLayeredWnd(hwnd, FALSE);
		//CUT    if (dwLong & WS_EX_LAYERED)
		//CUT      SetWindowLong(hwnd, GWL_EXSTYLE, dwLong & ~WS_EX_LAYERED);
		has_alpha_flag = 0;
	}
	else
	{
		if (!Wasabi::Std::Wnd::isLayeredWnd(hwnd))
			Wasabi::Std::Wnd::setLayeredWnd(hwnd, TRUE);
		//CUT    if (!(dwLong & WS_EX_LAYERED))
		//CUT      SetWindowLong(hwnd, GWL_EXSTYLE, dwLong | WS_EX_LAYERED);
		Wasabi::Std::Wnd::setLayeredAlpha(hwnd, amount);
		//CUT    setLayeredWindowAttributes(hwnd, RGB(0,0,0), amount, LWA_ALPHA);
		has_alpha_flag = 1;
	}
#endif
}

int BaseWnd::forceTransparencyFlag()
{
	return 0;
}

int BaseWnd::beginCapture()
{
	if (!getCapture())
	{
		disable_tooltip_til_recapture = 0;
		curVirtualChildCaptured = NULL;
		/*    oldCapture = */Wasabi::Std::Wnd::setCapture(getOsWindowHandle());
		/*    if (oldCapture) {
		      DebugString("Stolen capture detected, this may be ok, but try to avoid it if possible. Saving old capture\n");
		    }*/
		inputCaptured = 1;
	}
	return 1;
}

int BaseWnd::endCapture()
{
	preventcancelcapture = 1;
	if (Wasabi::Std::Wnd::getCapture() == getOsWindowHandle()) Wasabi::Std::Wnd::releaseCapture();
	/*  if (oldCapture) {
	    DebugString("Restoring old capture\n");
	    SetCapture(oldCapture);
	    oldCapture = NULL;
	  }*/
	inputCaptured = 0;
	preventcancelcapture = 0;
	return 1;
}

int BaseWnd::getCapture()
{
	if (inputCaptured && Wasabi::Std::Wnd::getCapture() == getOsWindowHandle() && curVirtualChildCaptured == NULL) return 1;
	return 0;
}

void BaseWnd::cancelCapture()
{
	if (curVirtualChildCaptured != NULL)
	{
		curVirtualChildCaptured->cancelCapture();
		return ;
	}
	if (getCapture()) endCapture();
	onCancelCapture();
}

int BaseWnd::onMouseMove(int x, int y)
{
	onTipMouseMove();
	return 0;
}

void BaseWnd::onTipMouseMove()
{
	POINT p;

	if (dragging) return ;
	if (disable_tooltip_til_recapture) return ;

	tipbeenchecked = TRUE;

	Wasabi::Std::getMousePos(&p);

	if (WASABI_API_WND->rootWndFromPoint(&p) != (ifc_window *)this)
	{
		// leaving area
		tip_done = FALSE;
		if (tipawaytimer)
			killTimer(TIP_AWAY_ID);
		tipawaytimer = FALSE;
		if (tipshowtimer)
		{
			// TODO: on the mac, use CreateMouseTrackingRegion
			TRACKMOUSEEVENT tracker;
			tracker.cbSize=sizeof(tracker);
			tracker.dwFlags = TME_HOVER|TME_CANCEL;
			tracker.hwndTrack = this->getOsWindowHandle();
			tracker.dwHoverTime = TIP_TIMER_THRESHOLD;

			TrackMouseEvent(&tracker);
		}
		tipshowtimer = FALSE;
		destroyTip();
	}
	else
	{
		// moving in area
		const wchar_t *t = getTip();
		if (!disable_tooltip_til_recapture && !tipshowtimer && !tip_done && t != NULL && *t != 0)
		{
			//entering area & need tip

			// TODO: on the mac, use CreateMouseTrackingRegion
			TRACKMOUSEEVENT tracker;
			tracker.cbSize=sizeof(tracker);
			tracker.dwFlags = TME_HOVER;
			tracker.hwndTrack = this->getOsWindowHandle();
			tracker.dwHoverTime = TIP_TIMER_THRESHOLD;

			TrackMouseEvent(&tracker);

			tipshowtimer = TRUE;
		}
		/*else if (tipshowtimer)
		{
		TRACKMOUSEEVENT tracker;
		tracker.cbSize=sizeof(tracker);
		tracker.dwFlags = TME_HOVER;
		tracker.hwndTrack = this->getOsWindowHandle();
		tracker.dwHoverTime = TIP_TIMER_THRESHOLD;

		TrackMouseEvent(&tracker);
		}*/
	}
}

int BaseWnd::onLeftButtonDblClk(int x, int y)
{
	return 0;
}

int BaseWnd::onRightButtonDblClk(int x, int y)
{
	return 0;
}

int BaseWnd::onGetFocus()
{
	// return TRUE if you override this
	hasfocus = 1;
	notifyParent(ChildNotify::GOTFOCUS);
	getRootParent()->onSetRootFocus(this);
	invalidate();
	Accessible *a = getAccessibleObject();
	if (a != NULL)
		a->onGetFocus();
	return 1;
}

int BaseWnd::onKillFocus()
{
	// return TRUE if you override this
	hasfocus = 0;
	notifyParent(ChildNotify::KILLFOCUS);
	invalidate();
	return 1;
}

#if defined(_WIN64)
int BaseWnd::childNotify(ifc_window* child, int msg, int p1, int p2)
{
	return 0;
}
#else
int BaseWnd::childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2)
{
	return 0;
}
#endif

int BaseWnd::addDragItem(const wchar_t *droptype, void *item)
{
	ASSERT(droptype != NULL);
	if (item == NULL) return -1;
	DragSet *set;
	int pos = dragCheckData(droptype);
	if (pos == -1)
	{
		set = new DragSet();
		set->setName(droptype);
		dragsets.addItem(set);
		pos = dragsets.getNumItems() - 1;
	}
	else set = dragsets[pos];
	set->addItem(item);
	return pos;
}

#ifdef _WIN32
int BaseWnd::handleDrag()
{
	abortTip();
	if (dragsets.getNumItems() == 0) return 0;

	Wasabi::Std::Wnd::setCapture(hwnd);
	SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_APPSTARTING)));

	dragging = 1;
	stickyWnd = NULL;

	return 1;
}
#endif

int BaseWnd::resetDragSet()
{
	dragsets.deleteAll();
	return 1;
}

int BaseWnd::dragEnter(ifc_window *sourceWnd)
{
	ifc_window *rw = getParent(); //api_window::rootwndFromRootWnd(getParent()); //FG> note to self, check!
	if (rw) return DI(rw).dragEnter(sourceWnd);
	return 0;
}

int BaseWnd::dragSetSticky(ifc_window *wnd, int left, int right, int up, int down)
{
	ASSERT(dragging);
	stickyWnd = wnd;

	if (left < 0) left = 0;
	if (right < 0) right = 0;
	if (up < 0) up = 0;
	if (down < 0) down = 0;

	Wasabi::Std::setRect(&sticky, left, up, right, down);

	return 1;
}

void BaseWnd::setSuggestedDropTitle(const wchar_t *title)
{
	ASSERT(title != NULL);
	suggestedTitle = title;
}

const wchar_t *BaseWnd::dragGetSuggestedDropTitle()
{
	return suggestedTitle;	// can be NULL
}

int BaseWnd::dragCheckData(const wchar_t *type, int *nitems)
{
	for (int i = 0; i < dragsets.getNumItems(); i++)
	{
		if (!WCSICMP(type, dragsets[i]->getName()))
		{
			if (nitems != NULL) *nitems = dragsets[i]->getNumItems();
			return i;
		}
	}
	return -1;
}

void *BaseWnd::dragGetData(int slot, int itemnum)
{
	if (slot < 0 || slot >= dragsets.getNumItems()) return 0;
	if (itemnum < 0 || itemnum >= dragsets[slot]->getNumItems()) return 0;
	return dragsets[slot]->enumItem(itemnum);
}

void BaseWnd::addDroppedFile(const wchar_t *filename, PtrList<FilenamePS> *plist)
{
#ifdef RECURSE_SUBDIRS_ON_DROP
	const char *slash = filename + STRLEN(filename) - 1;
	for (; slash > filename; slash--) if (*slash == '/' || *slash == '\\') break;
	if (STREQL(slash + 1, ".") || STREQL(slash + 1, "..")) return ;

	char buf[WA_MAX_PATH] = {0};
	STRCPY(buf, filename);
	// try to resolve shortcuts
	char *ext = buf + STRLEN(buf) - 1;
	for (; ext > buf; ext--) if (*ext == '.' || *ext == '\\' || *ext == '/') break;
#ifdef WIN32
	if (!STRICMP(ext, ".lnk"))
	{
		char buf2[MAX_PATH] = {0};
		if (StdFile::resolveShortcut(buf, buf2)) STRCPY(buf, buf2);
	}
#endif

	int isdir = 0;

	// handle root dir specially?
	WIN32_FIND_DATA data = {0};
	HANDLE r = FindFirstFile(buf, &data);
	if (!r) return ;
	FindClose(r);
	if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) isdir = 1;

	if (isdir)
	{
		onExternalDropDirScan(buf);

		// enumerate that dir
		char search[WA_MAX_PATH] = {0};
		wsprintf(search, "%s\\*.*", buf);
		HANDLE files = FindFirstFile(search, &data);
		if (files == INVALID_HANDLE_VALUE) return ;	// nothin' in it
		for (;;)
		{
			wchar_t obuf[WA_MAX_PATH] = {0};
			swprintf(obuf, L"%s\\%s", buf, data.cFileName);
			addDroppedFile(obuf, plist);
			if (!FindNextFile(files, &data))
			{
				FindClose(files);
				return ;
			}
		}

		// should never get here
	}
	else
	{
		addDragItem(DD_FILENAME, plist->addItem(new FilenamePS(StringPrintfW(L"file:%s", buf))));
	}
#else
	addDragItem(DD_FILENAME, plist->addItem(new FilenamePS(StringPrintfW(L"file:%s", filename))));
#endif
}

bool BaseWnd::getNoCopyBits(void)
{
	return ncb;
}

void BaseWnd::setNoCopyBits(bool newncb)
{
	ncb = newncb;
}

int BaseWnd::onEraseBkgnd(HDC dc)
{
	return 1;
}

void BaseWnd::setIcon(OSICONHANDLE icon, int _small)
{
	Wasabi::Std::Wnd::setIcon(getOsWindowHandle(), icon, !_small);
	//CUT  SendMessageW(getOsWindowHandle(), WM_SETICON, _small ? ICON_SMALL : ICON_BIG, (int)icon);
}

const wchar_t *BaseWnd::getTip()
{
	return tip;
}

void BaseWnd::setTip(const wchar_t *_tooltip)
{
	tip = _tooltip;
	abortTip();
}

int BaseWnd::getStartHidden()
{
	return start_hidden;
}

void BaseWnd::abortTip()
{
	if (tipshowtimer)
	{
		// TODO: on the mac, use CreateMouseTrackingRegion
		TRACKMOUSEEVENT tracker;
		tracker.cbSize=sizeof(tracker);
		tracker.dwFlags = TME_HOVER|TME_CANCEL;
		tracker.hwndTrack = this->getOsWindowHandle();
		tracker.dwHoverTime = TIP_TIMER_THRESHOLD;

		TrackMouseEvent(&tracker);
	}
	tipshowtimer = FALSE;
	if (tipawaytimer)
		killTimer(TIP_AWAY_ID);
	tipawaytimer = FALSE;
	if (tipdestroytimer)
		killTimer(TIP_DESTROYTIMER_ID);
	tipdestroytimer = FALSE;
	destroyTip();
	tip_done = FALSE;
	RECT r;
	if (getOsWindowHandle() && Wasabi::Std::Wnd::getUpdateRect(getOsWindowHandle(), &r) != 0) // FG> avoids xoring over disapearing tip
		repaint();
}

int BaseWnd::isVisible(int within)
{
	if (!isVirtual() && !getOsWindowHandle()) return 0;
	if (!this_visible) return 0;

	if (within) return this_visible; // whatever, local

	if (isVirtual()) // virtual, global
		if (getParent())
			return getParent()->isVisible();
		else
			return 0;

	// non virtual, global
	if (Wasabi::Std::Wnd::isPopup(getOsWindowHandle())) return this_visible;
	if (!Wasabi::Std::Wnd::isValidWnd(Wasabi::Std::Wnd::getParent(getOsWindowHandle()))) return 0;
	if (getParent()) return getParent()->isVisible(); // not a popup, check its parent or fail
	return this_visible;
}

void BaseWnd::registerRootWndChild(ifc_window *child)
{
	rootwndchildren.addItem(child);
	if (child->isVirtual())
		virtualChildren.addItem(child);
}

void BaseWnd::unregisterRootWndChild(ifc_window *child)
{
	delTabOrderEntry(child);
	rootwndchildren.removeItem(child);
	if (curVirtualChildCaptured == child)
	{
		setVirtualChildCapture(NULL);
	}
	if (curVirtualChildFocus == child)
		curVirtualChildFocus = NULL;
	virtualChildren.removeItem(child);
	//WASABI_API_WND->timer_remove(this, -1);
	if (isPostOnInit() && isVisible())
		postDeferredCallback(DEFERREDCB_INVALIDATE, 0);
}

int BaseWnd::isVirtual()
{
	return 0;
}

//CUT?inline int isInRect(RECT *r,int x,int y) {
//CUT?  if (x>=r->left&&x<=r->right&&y>=r->top&&y<=r->bottom) return 1;
//CUT?  return 0;
//CUT?}

int BaseWnd::ensureVirtualCanvasOk()
{
	RECT ncr;

	if (isVirtual() && getParent()) return 1;

	int size_w = rwidth;
	int size_h = rheight;

	if (!size_w || !size_h) return 0;

	if (!virtualCanvas || virtualCanvasH != size_h || virtualCanvasW != size_w)
	{
		if (virtualCanvas)
		{
			deleteFrameBuffer(virtualCanvas);
			virtualCanvas = NULL;
		}
		delete scalecanvas;
		scalecanvas = NULL;
		virtualCanvas = createFrameBuffer(size_w, size_h);
		prepareFrameBuffer(virtualCanvas, size_w, size_h);
		virtualCanvas->setBaseWnd(this);
		virtualCanvasH = size_h; virtualCanvasW = size_w;
		BaseWnd::getNonClientRect(&ncr);
		deferedInvalidateRect(&ncr);
	}
	return 1;
}

Canvas *BaseWnd::createFrameBuffer(int w, int h)
{
	return new BltCanvas(w, h, getOsWindowHandle());
}

void BaseWnd::prepareFrameBuffer(Canvas *canvas, int w, int h)
{
	RECT r = {0, 0, w, h};
	RegionI reg(&r);
	virtualBeforePaint(&reg);
#ifdef _WIN32
	canvas->selectClipRgn(NULL);
	canvas->fillRect(&r, 0);
#elif defined(__APPLE__)
	CGContextClearRect(canvas->getHDC(), CGRectInfinite); // TODO: make "clear" function in canvas
#endif
	virtualAfterPaint(&reg);
}

void BaseWnd::deleteFrameBuffer(Canvas *canvas)
{
	delete canvas;
}

// paints the client content, followed by the virtual child tree. recursive
int BaseWnd::paintTree(Canvas *canvas, api_region *r)
{
	onPaint(canvas, r);

#ifdef WASABI_DRAW_FOCUS_RECT
	if (gotFocus())
	{
		RECT ncr;
		getNonClientRect(&ncr);
		// try to use skinned focus rect
		if (WASABI_API_WND->paintset_present(Paintset::FOCUSRECT))
			WASABI_API_WND->paintset_render(Paintset::FOCUSRECT, canvas, &ncr, 128);
		else // otherwise this looks kinda nice :P
			canvas->drawRect(&ncr, 0, 0xFFFFFF, 128);
	}
#endif

	if (isVirtual() && !hasVirtualChildren()) return 0;

	api_region *hostrgn = NULL;
	api_region *update = r;

	if (!(hwnd != NULL && getParent() == NULL))
	{
		hostrgn = getRegion();
		update = r->clone();
		if (hostrgn && !isRectRgn())
		{
			RECT ncr = clientRect();
			api_region *hostclone = hostrgn->clone();
			hostclone->addRegion(getComposedRegion());
			hostclone->offset(ncr.left, ncr.top);
			update->andRegion(hostclone);
			hostrgn->disposeClone(hostclone);
		}
	}

	RegionI client_update;
	for (int i = 0;i < virtualChildren.getNumItems();i++)
	{
		if (!virtualChildren[i]->isVisible(1)) continue;
		RECT rChild;
		ifc_window *w = virtualChildren[i];
		w->getNonClientRect(&rChild);
		if ((rChild.right != rChild.left) && (rChild.bottom != rChild.top))
			if (update->intersectRect(&rChild, &client_update))
			{
				w->paintTree(canvas, &client_update);
			}
	}

	if (update != r) r->disposeClone(update);

	return 1;
}

void BaseWnd::setVirtualCanvas(Canvas *c)
{
	virtualCanvas = c;
}

int BaseWnd::pointInWnd(POINT *p)
{
	RECT r;
	if (!isVisible(1)) return 0;
	getWindowRect(&r);
	if (!Wasabi::Std::pointInRect(r, *p))
		return 0;
	for (int i = 0; i < getNumRootWndChildren(); i++)
	{
		ifc_window *c = enumRootWndChildren(i);
		if (!c->isVisible(1)) continue;
		RECT rChild;
		c->getWindowRect(&rChild);
		if (Wasabi::Std::pointInRect(rChild, *p))
			return 0;
	}
	//NONPORTABLE
	/*  HWND child = GetWindow(getOsWindowHandle(), GW_CHILD);
	  while (child != NULL) {
	    if (IsWindowVisible(child)) {
	      RECT r2;
	      GetWindowRect(child, &r2);
	      if (Std::pointInRect(r2, *p))
	        return 0;
	    }
	    child = GetWindow(child, GW_HWNDNEXT);
	  }*/
	return 1;
}

int BaseWnd::paint(Canvas *c, api_region *r)
{
	if (isVirtual())
	{
		RegionI d;
		RECT cr;
		getClientRect(&cr);
		if (r == NULL)
		{
			d.addRect(&cr);
		}
		else
		{
			d.addRegion(r);
			d.offset(cr.left, cr.top);
		}
		ifc_window *rp = getRootParent();
		deferedInvalidate();
		rp->paint(NULL, &d);
		BltCanvas *cc = static_cast<BltCanvas*>(rp->getFrameBuffer());
		if (r != NULL) c->selectClipRgn(r);
		cc->blit(cr.left, cr.top, c, 0, 0, cr.right - cr.left, cr.bottom - cr.top);
		return 1;
	}

	if (!ensureVirtualCanvasOk()) return 0;
	RegionI *deleteme = NULL;
	if (r == NULL)
	{
		RECT cr;
		getNonClientRect(&cr);
		deleteme = new RegionI(&cr);
		r = deleteme;
	}

	virtualBeforePaint(r);

	RECT rcPaint;
	r->getBox(&rcPaint);

	double ra = getRenderRatio();

	if (deferedInvalidRgn)
	{
		api_region *nr = NULL;
		if (renderRatioActive())
		{
			nr = r->clone();
			double d = 1.0 / ra;
			nr->scale(d, d, TRUE);
		}

		if (deferedInvalidRgn->isEmpty() == 0)
		{
			// some deferednvalidated regions needs to be repainted
			// TODO: need a "clear region" function in canvas
			api_region *i = deferedInvalidRgn->clone();
#ifdef _WIN32
			FillRgn(virtualCanvas->getHDC(), i->getOSHandle(), (HBRUSH)GetStockObject(BLACK_BRUSH));
#elif defined(__APPLE__)
			CGContextSaveGState(virtualCanvas->getHDC());
			virtualCanvas->selectClipRgn(i);
			CGContextClearRect(virtualCanvas->getHDC(), CGRectInfinite);
//      virtualCanvas->selectClipRgn(0);
			CGContextRestoreGState(virtualCanvas->getHDC());
#endif
			paintTree(virtualCanvas, i);

			deferedValidateRgn(i);
			deferedInvalidRgn->disposeClone(i);

		}
		if (nr) r->disposeClone(nr);
	}

	virtualAfterPaint(r);

	if (c != NULL)
	{
		commitFrameBuffer(c, &rcPaint, ra);  //TH WDP2-212
	}

	delete deleteme;
	return 1;
}

int BaseWnd::virtualOnPaint()
{
#ifdef _WIN32
	RECT cr;
	getNonClientRect(&cr);
	if (cr.left >= cr.right || cr.top >= cr.bottom) return 0;

	if (!ensureVirtualCanvasOk()) return 0;

	RegionI reg;

	//CUT  GetUpdateRgn(getOsWindowHandle(), r->getOSHandle(), FALSE);
	Wasabi::Std::Wnd::getUpdateRegion(getOsWindowHandle(), reg.getOSHandle());

	PaintCanvas paintcanvas;
	if (!paintcanvas.beginPaint(this))
	{
		virtualAfterPaint(&reg); return 0;
	}

	// DO NOT DELETE - This looks like it does nothing, but it actually makes the GDI call us again with WM_PAINT if some window
	// moves over this one between BeginPaint and EndPaint. We still use GetUpdateRgn so we don't have to check for
	// the version of Windows. See doc. If this function is not available (should be here in 95/98/NT/2K, but we never know)
	// then we use the rcPaint rect... less precise, but still works.
	//CUT  if (getRandomRgn) {
	if (Wasabi::Std::Wnd::haveGetRandomRegion())
	{
		RegionI zap;
		//CUT    getRandomRgn(paintcanvas.getHDC(), zap.getOSHandle(), SYSRGN);
		Wasabi::Std::Wnd::getRandomRegion(paintcanvas.getHDC(), zap.getOSHandle());
	}
	else
	{
		RECT z;
		paintcanvas.getRcPaint(&z);
		reg.setRect(&z);
	}

	// -------------

	/*// for debug
	  HDC dc = GetDC(getOsWindowHandle());
	  InvertRgn(dc, r->getHRGN());
	  InvertRgn(dc, r->getHRGN());
	  ReleaseDC(getOsWindowHandle(), dc);*/

	paint(&paintcanvas, &reg);
#else
#warning port me or remove me
#endif

	return 1;
}

ifc_window *BaseWnd::enumVirtualChild(int _enum)
{
	return virtualChildren[_enum];
}

int BaseWnd::getNumVirtuals()
{
	return virtualChildren.getNumItems();
}

ifc_window *BaseWnd::enumRootWndChildren(int n)
{
	return rootwndchildren.enumItem(n);
}

int BaseWnd::getNumRootWndChildren()
{
	return rootwndchildren.getNumItems();
}

ifc_window *BaseWnd::findRootWndChild(int x, int y, int only_virtuals)
{
	for (int i = getNumRootWndChildren() - 1; i > -1; i--)
	{
		RECT r;
		ifc_window *child = enumRootWndChildren(i);
		//DebugStringW(L"findRootWndChild = entering = %s\n", child->getId());
		if (only_virtuals && !child->isVirtual()) continue;
		child->getNonClientRect(&r);
		int _x = x;
		int _y = y;
		if (!child->isVirtual())
		{
			POINT pt;
			child->getPosition(&pt);
			_x -= pt.x;
			_y -= pt.y;
		}
		int iv = child->isVisible(1);
		//int gpa = child->getPaintingAlpha();
		POINT _p = Wasabi::Std::makePoint(_x, _y);
		if (iv /*&& gpa > 0*/ && Wasabi::Std::pointInRect(r, _p))
		{
			// GROUP
			ifc_window *z = child->findRootWndChild(_x, _y);
			if (z) return z;
		}
		/*gpa > 0 &&*/
		/*if (iv && _x>=r.left&&_x<=r.right&&_y>=r.top&&_y<=r.bottom && !child->isClickThrough() && child->ptInRegion(_x, _y)) {
		  return child;
		}*/
	}
	return (!isClickThrough() && ptInRegion(x, y)) ? this : NULL;
}

//PORTME
int BaseWnd::handleVirtualChildMsg(UINT uMsg, int x, int y, void *p, void *d)
{
#ifdef _WIN32
	ifc_window *child = NULL;

	if (curVirtualChildCaptured)
		child = curVirtualChildCaptured;
	else
		child = findRootWndChild(x, y, 1); // warning, can return this which is not virtual

	//  ASSERT(child != NULL);	// BU this came up and I don't know why, looks like it should never happen
	// FG> actually it can happen when coming back from a popup menu when cpu usage is high, the window might be
	// hidden (destroying) and ptInRegion returns false.
	if (!child) return 0;

	//int isvirtual = child->isVirtual();

	if (child) child = child->getForwardWnd();

	if (child && child->isEnabled())
	{
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
			/*        if (isvirtual && child != curVirtualChildFocus)
			          focusVirtualChild(child);*/
			autoFocus(child);
			if (child->wantLeftClicks())
				child->onLeftButtonDown(x, y);
			if (child->checkDoubleClick(uMsg, x, y) && child->wantDoubleClicks())
				child->onLeftButtonDblClk(x, y);
			return 1;
		case WM_RBUTTONDOWN:
			/*        if (isvirtual && child != curVirtualChildFocus)
			          focusVirtualChild(child);*/
			autoFocus(child);
			if (child->wantRightClicks())
				child->onRightButtonDown(x, y);
			if (child->checkDoubleClick(uMsg, x, y) && child->wantDoubleClicks())
				child->onRightButtonDblClk(x, y);
			return 1;
		case WM_LBUTTONUP:
			if (child->wantLeftClicks())
				child->onLeftButtonUp(x, y);
			return 1;
		case WM_RBUTTONUP:
			if (child->wantRightClicks())
				child->onRightButtonUp(x, y);
			return 1;
		case WM_MOUSEMOVE:
		{
			if (curVirtualChildCaptured == child || (curVirtualChildCaptured == NULL))
			{
				if (child->wantMouseMoves())
					child->onMouseMove(x, y);
				return 1;
			}
			return 0;
		}
		case WM_MOUSEHOVER:
			((BaseWnd *)child)->onTip();
			break;
		case WM_SETCURSOR:
			int a = child->getCursorType(x, y);
			if (!p) return 0;
			*(int *)p = a;
			if (a == BASEWND_CURSOR_USERSET)
			{
				OSCURSORHANDLE c = child->getCustomCursor(x, y);
				if (!d) return 0;
				*(OSCURSORHANDLE *)d = c;
			}
			return 1;
		}
	}
#else
#warning port me or remove me
#endif
	return 0;
}

int BaseWnd::onLeftButtonDown(int x, int y)
{
	disable_tooltip_til_recapture = 1;
	abortTip();
	return 0;
}

int BaseWnd::onLeftButtonUp(int x, int y)
{
	disable_tooltip_til_recapture = 1;
	abortTip();
	return 0;
}

void BaseWnd::setVirtualChildCapture(ifc_window *child)
{
	if (child)
	{
		if (!inputCaptured)
		{
			beginCapture();
		}
	}
	else
	{
		endCapture();
	}
	curVirtualChildCaptured = child;
}

ifc_window *BaseWnd::getVirtualChildCapture()
{
	if (inputCaptured && Wasabi::Std::Wnd::getCapture() == getOsWindowHandle())
		return curVirtualChildCaptured;
	else
		if (inputCaptured) inputCaptured = 0;
	return NULL;
}

ifc_window *BaseWnd::getBaseTextureWindow()
{
	// return our base texture window if we have it
	if (btexture)
		return btexture;
	// return our parent's if they have it
	if (getParent())
		return getParent()->getBaseTextureWindow();
	else
		return NULL;
}

void BaseWnd::renderBaseTexture(ifc_canvas *c, const RECT &r, int alpha)
{
	WASABI_API_WND->skin_renderBaseTexture(getBaseTextureWindow(), c, r, this, alpha);
}

void BaseWnd::setBaseTextureWindow(ifc_window *w)
{
	btexture = w;
}

void BaseWnd::setNotifyWindow(ifc_window *newnotify)
{
	notifyWindow = newnotify;
}

ifc_window *BaseWnd::getNotifyWindow()
{
	return destroying ? NULL : notifyWindow;
}

int BaseWnd::gotFocus()
{
	return hasfocus && curVirtualChildFocus == NULL;
}

int BaseWnd::isActive()
{
	OSWINDOWHANDLE h = hwnd;
	if (h == NULL)
	{
		ifc_window *par = getParent();
		if (par == NULL) return 0;
		h = par->getOsWindowHandle();
	}
	if (h == NULL) return 0;
	return (Wasabi::Std::Wnd::getActiveWindow() == h);
}

int BaseWnd::onChar(unsigned int c)
{
	switch (c)
	{
	case 9:  // TAB
		if (Std::keyModifier(STDKEY_SHIFT))
			focusPrevious();
		else
			focusNext();
		return 1;
	}
	return 0;
}

/*int BaseWnd::focusVirtualChild(api_window *child) {
  if (!gotFocus()) setFocus();
  if (!child->wantFocus()) return 0;
  setVirtualChildFocus(child);
  return 1;
}*/

int BaseWnd::wantFocus()
{
	return 0;
}

// Return 1 if there is a modal window that is not this
int BaseWnd::checkModal()
{
	if (bypassModal()) return 0;
	ifc_window *w = WASABI_API_WND->getModalWnd();
	if (w && w != static_cast<ifc_window*>(this) && w != getDesktopParent())
	{
		return 1;
	}
	return 0;
}

int BaseWnd::cascadeRepaintFrom(ifc_window *who, int pack)
{
	RECT r;
	BaseWnd::getNonClientRect(&r);
	return BaseWnd::cascadeRepaintRect(&r, pack);
}

int BaseWnd::cascadeRepaint(int pack)
{
	return cascadeRepaintFrom(this, pack);
}

int BaseWnd::cascadeRepaintRgn(api_region *r, int pack)
{
	return cascadeRepaintRgnFrom(r, this, pack);
}

int BaseWnd::cascadeRepaintRect(RECT *r, int pack)
{
	return cascadeRepaintRectFrom(r, this, pack);
}

int BaseWnd::cascadeRepaintRectFrom(RECT *r, ifc_window *who, int pack)
{
	RegionI reg(r);
	int rt = cascadeRepaintRgnFrom(&reg, who, pack);
	return rt;
}

void BaseWnd::_cascadeRepaintRgn(api_region *rg)
{
	if (!ensureVirtualCanvasOk()) return ;

	WndCanvas paintcanvas;
	if (paintcanvas.attachToClient(this) == 0)
		return;

	virtualBeforePaint(rg);

	deferedInvalidateRgn(rg);
	paintTree(virtualCanvas, rg);

	virtualAfterPaint(rg);

	double ra = getRenderRatio();

	RECT rcPaint;
	rg->getBox(&rcPaint);

	RECT rc;
	getClientRect(&rc); //JF> this gets it in virtual (non-scaled) coordinates,
	// so we need to do these comparisons before scaling.
	rcPaint.bottom = MIN((int)rc.bottom, (int)rcPaint.bottom);
	rcPaint.right = MIN((int)rc.right, (int)rcPaint.right);

	if (renderRatioActive()) // probably faster than scaling the clone
	{
		rcPaint.left = (int)((rcPaint.left - 1) * ra);
		rcPaint.top = (int)((rcPaint.top - 1) * ra);
		rcPaint.right = (int)(rcPaint.right * ra + 0.999999);
		rcPaint.bottom = (int)(rcPaint.bottom * ra + 0.999999);
	}
	rcPaint.left = MAX(0, (int)rcPaint.left);
	rcPaint.top = MAX(0, (int)rcPaint.top);
	rcPaint.right = MIN((int)rcPaint.right, (int)(rwidth * ra));
	rcPaint.bottom = MIN((int)rcPaint.bottom, (int)(rheight * ra));

	commitFrameBuffer(&paintcanvas, &rcPaint, ra);
}


void BaseWnd::packCascadeRepaintRgn(api_region *rg)
{
	if (!deferedCascadeRepaintRgn) deferedCascadeRepaintRgn = new RegionI;
	deferedCascadeRepaintRgn->addRegion(rg);
	need_flush_cascaderepaint = 1;
}

int BaseWnd::cascadeRepaintRgnFrom(api_region *_rg, ifc_window *who, int pack)
{

	api_region *rg = _rg->clone();

	int j = virtualChildren.searchItem(who);
	for (int i = 0; i < virtualChildren.getNumItems(); i++)
	{
		ifc_window *w = virtualChildren[i];
		if (w != who && w->wantSiblingInvalidations())
			w->onSiblingInvalidateRgn(rg, who, j, i);
	}

	if (!pack)
	{
		_cascadeRepaintRgn(rg);
	}
	else
	{
		packCascadeRepaintRgn(rg);
	}

	_rg->disposeClone(rg);

	return 1;
}

void BaseWnd::setDesktopAlpha(int a)
{
	if (a && !Wasabi::Std::Wnd::isDesktopAlphaAvailable()) return ;
	if (a == w2k_alpha) return ;
	w2k_alpha = a;
	if (!a && scalecanvas)
	{
		delete scalecanvas;
		scalecanvas = NULL;
	}
	setLayeredWindow(w2k_alpha);
	onSetDesktopAlpha(a);
}

void BaseWnd::onSetDesktopAlpha(int a) { }

void BaseWnd::commitFrameBuffer(Canvas *paintcanvas, RECT *r, double ra)
{
	if (w2k_alpha && Wasabi::Std::Wnd::isDesktopAlphaAvailable() && !cloaked)
	{
		//CUT    BLENDFUNCTION blend= {AC_SRC_OVER, 0, wndalpha, AC_SRC_ALPHA };
		//CUT    POINT pt={0,0};
		RECT spr;
		getWindowRect(&spr);
		//CUT    POINT sp={spr.left,spr.top};
		//CUT    SIZE ss={spr.right-spr.left, spr.bottom-spr.top};
		int sw = spr.right - spr.left, sh = spr.bottom - spr.top;
		//CUT    SysCanvas c;

		if (handleRatio() && renderRatioActive())
		{
			// eek slow!
			RECT r;
			getWindowRect(&r);
			int w = r.right - r.left;
			int h = r.bottom - r.top;
			if (!scalecanvas) scalecanvas = new BltCanvas(w, h, getOsWindowHandle());
			virtualCanvas->stretchblit(0, 0, (int)((double)virtualCanvasW * 65536.0), (int)((double)virtualCanvasH * 65536.0), scalecanvas, 0, 0, w, h);
		}

		//CUT    updateLayeredWindow(hwnd, c.getHDC(), &sp, &ss, (scalecanvas ? scalecanvas : virtualCanvas)->getHDC(), &pt, 0, &blend, ULW_ALPHA);
		Wasabi::Std::Wnd::updateLayeredWnd(hwnd, spr.left, spr.top, sw, sh, (scalecanvas ? scalecanvas : virtualCanvas)->getHDC(), wndalpha);
	}
	else
	{
		if (ABS(ra - 1.0) <= 0.01)
		{
			virtualCanvas->blit(r->left, r->top, paintcanvas, r->left, r->top, r->right - r->left, r->bottom - r->top);
		}
		else
		{
			RECT tr = *r;

			double invra = 65536.0 / ra;
			int lp = tr.left;
			int tp = tr.top;
			int w = tr.right - tr.left;
			int h = tr.bottom - tr.top;

			int sx = (int)((double)lp * invra);
			int sy = (int)((double)tp * invra);
			int sw = (int)((double)w * invra);
			int sh = (int)((double)h * invra);

			virtualCanvas->stretchblit(sx, sy, sw, sh, paintcanvas, lp, tp, w, h);
		}
	}
}

void BaseWnd::flushPaint()
{
	postDeferredCallback(DEFERREDCB_FLUSHPAINT, 0);
}

void BaseWnd::do_flushPaint()
{
	if (!deferedInvalidRgn || deferedInvalidRgn->isEmpty()) return ;
	api_region *r = deferedInvalidRgn->clone();
	cascadeRepaintRgn(r);
	deferedInvalidRgn->disposeClone(r);
	deferedInvalidRgn->empty();
}

int BaseWnd::isMouseOver(int x, int y)
{
	POINT pt = {x, y};
	clientToScreen(&pt);

	return (WASABI_API_WND->rootWndFromPoint(&pt) == this);
}

void BaseWnd::freeResources()
{}

void BaseWnd::reloadResources()
{
	invalidate();
}

double BaseWnd::getRenderRatio()
{
	if (!handleRatio()) return 1.0;
	if (!ratiolinked) return ratio;
	return getParent() ? getParent()->getRenderRatio() : ratio;
}

void BaseWnd::setRenderRatio(double r)
{
	// "snap" to 1.0
	if (ABS(r - 1.0) <= 0.02f) r = 1.0;
	if (scalecanvas)
	{
		delete scalecanvas;
		scalecanvas = NULL;
	}
	if (isInited() && r != ratio && !isVirtual() && (!getParent() || !ratiolinked))
	{
		// must scale size & region accordingly
		RECT rc;
		BaseWnd::getWindowRect(&rc);
		rc.right = rc.left + rwidth;
		rc.bottom = rc.top + rheight;
		ratio = r;

		resize(&rc);

		invalidate();
		if (isPostOnInit())
			onRatioChanged();
	}
}

void BaseWnd::setRatioLinked(int l)
{
	ratiolinked = l;
	if (isPostOnInit())
		setRenderRatio(ratio);
}

int BaseWnd::renderRatioActive()
{
	return ABS(getRenderRatio() - 1.0) > 0.01f;
}

void BaseWnd::multRatio(int *x, int *y)
{
	double rr = getRenderRatio();
	if (x) *x = (int)((double)(*x) * rr);
	if (y) *y = (int)((double)(*y) * rr);
}

void BaseWnd::multRatio(RECT *r)
{
	Wasabi::Std::scaleRect(r, getRenderRatio());
}

void BaseWnd::divRatio(int *x, int *y)
{
	double rr = getRenderRatio();
	if (x) *x = (int)((double)(*x) / rr + 0.5);
	if (y) *y = (int)((double)(*y) / rr + 0.5);
}

void BaseWnd::divRatio(RECT *r)
{
	double rr = getRenderRatio();
	Wasabi::Std::scaleRect(r, 1./rr);
}

void BaseWnd::bringVirtualToFront(ifc_window *w)
{
	changeChildZorder(w, 0);
}

void BaseWnd::bringVirtualToBack(ifc_window *w)
{
	changeChildZorder(w, virtualChildren.getNumItems()-1);
}

void BaseWnd::bringVirtualAbove(ifc_window *w, ifc_window *b)
{
	ASSERT(b->isVirtual());
	int p = virtualChildren.searchItem(b);
	if (p == -1) return ;
	changeChildZorder(w, p);
}

void BaseWnd::bringVirtualBelow(ifc_window *w, ifc_window *b)
{
	ASSERT(b->isVirtual());
	int p = virtualChildren.searchItem(b);
	if (p == -1) return ;
	changeChildZorder(w, p + 1);
}

void BaseWnd::changeChildZorder(ifc_window *w, int newpos)
{
	int p = newpos;
	p = MAX(p, (int)0);
	p = MIN(p, virtualChildren.getNumItems()-1);
	RECT cr;
	w->getClientRect(&cr);

	PtrList<ifc_window> l;
	int i;
	for (i = 0;i < virtualChildren.getNumItems();i++)
		if (virtualChildren[i] != w)
			l.addItem(virtualChildren[i]);

	p = virtualChildren.getNumItems() - newpos - 1;
	virtualChildren.removeAll();

	int done = 0;

	for (i = 0;i < l.getNumItems();i++)
		if (i == p && !done)
		{
			virtualChildren.addItem(w);
			i--;
			done++;
		}
		else
		{
			RECT dr, intersection;
			l.enumItem(i)->getClientRect(&dr);
			if (Wasabi::Std::rectIntersect(intersection, dr, &cr))
				l[i]->invalidateRect(&intersection);
			virtualChildren.addItem(l.enumItem(i));
		}
	if (i == p && !done)
		virtualChildren.addItem(w);
	w->invalidate();
}

int BaseWnd::onActivate()
{
	if (hasVirtualChildren())
	{
		int l = getNumVirtuals();
		for (int i = 0; i < l; i++)
		{
			ifc_window *r = enumVirtualChild(i);
			r->onActivate();
		}
	}
		
	return 0;
}

int BaseWnd::onDeactivate()
{
	if (hasVirtualChildren())
	{
		int l = getNumVirtuals();
		for (int i = 0; i < l; i++)
		{
			ifc_window *r = enumVirtualChild(i);
			r->onDeactivate();
		}
	}
	return 0;
}

int BaseWnd::getDesktopAlpha()
{
	return w2k_alpha;
}

api_region *BaseWnd::getRegion()
{
	return NULL;
}

//CUT int BaseWnd::isTransparencyAvailable() {
//CUT #ifdef WIN32
//CUT #else
//CUT #pragma warning port me!
//CUT #endif
//CUT   return 0;
//CUT }

int BaseWnd::handleTransparency()
{
	return 1; // by default all windows handle transparency, only windows blitting directly on the SCREEN (if you blit directly on the DC it's still ok),
}           // for instance, a vis or a video using overlay should return 0, this will let the layout auto manage its alpha as that window is shown/hiden

void BaseWnd::setAlpha(int active, int inactive)
{
	if (active == 254) active = 255;
	if (active == 1) active = 0;
	if (inactive == 254) inactive = 255;
	if (inactive == 1) inactive = 0;
	int oldactivealpha = activealpha;
	active = MIN(255, MAX(0, active));
	if (inactive != -1) inactive = MIN(255, MAX(0, inactive));

	if (active != activealpha)
	{
		activealpha = active;
		if (isActive())
		{
			invalidate();
			if ((oldactivealpha == 0 || activealpha == 0) && (oldactivealpha != 0 || activealpha != 0))
				invalidateWindowRegion();
		}
	}
	if (inactive == -1) inactive = active;
	if (inactive != inactivealpha)
	{
		inactivealpha = inactive;
		if (!isActive())
		{
			invalidate();
			if ((oldactivealpha == 0 || activealpha == 0) && (oldactivealpha != 0 || activealpha != 0))
				invalidateWindowRegion();
		}
	}
}

void BaseWnd::getAlpha(int *active, int *inactive)
{
	if (active) *active = activealpha;
	if (inactive) *inactive = inactivealpha;
}

int BaseWnd::getPaintingAlpha(void)
{
	int a = isActive() ? MIN(255, MAX(0, activealpha)) : MIN(255, MAX(0, inactivealpha));
	ASSERT(a >= 0 && a <= 255);
	if (getParent() && getParent()->isVirtual())
	{
		int b = getParent()->getPaintingAlpha();
		a = (int)((double)a / 255.0 * (double)b);
	}
	if (a == 254) a = 255;
	if (a == 1) a = 0;
	if (!isEnabled()) a = (int)(a*0.6);
	return a;
}

void BaseWnd::setClickThrough(int ct)
{
	clickthrough = ct;
}

int BaseWnd::isClickThrough()
{
	return clickthrough;
}

int BaseWnd::handleRatio()
{
	return 1;
}

#include <api/script/objects/c_script/c_rootobj.h>

int BaseWnd::createTip()
{
	destroyTip();
	tooltip = new Tooltip(getTip());
	return -1;
}

void BaseWnd::destroyTip()
{
	// this is to avoid pb if destroytip() is being called by a time while destroying tip
	Tooltip *tt = tooltip;
	tooltip = NULL;
	delete tt;
}


int BaseWnd::runModal()
{
	//PORTME
#ifdef _WIN32
	ifc_window *dp = getDesktopParent();
	if (dp && dp != this)
		return dp->runModal();

	MSG msg;

	//  SetCapture(NULL);
	SetFocus(getOsWindowHandle());

	WASABI_API_WND->pushModalWnd(this);
	returnvalue = 0;
	mustquit = 0;

	// Main message loop:
	while (!mustquit)
	{
		mustquit = !GetMessage(&msg, NULL, 0, 0);
		if (!msg.hwnd || !TranslateAccelerator(msg.hwnd, NULL, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WASABI_API_WND->popModalWnd(this);
	//  SetCapture(NULL);
	return returnvalue;
#else
#warning port me
	return 0;
#endif
}

void BaseWnd::endModal(int ret)
{
	ifc_window *dp = getDesktopParent();
	if (dp && dp != this)
	{
		dp->endModal(ret);
		return ;
	}
	returnvalue = ret;
	mustquit = 1;
}

int BaseWnd::wantAutoContextMenu()
{
	return 1;
}

void BaseWnd::onCancelCapture()
{}

ifc_window *BaseWnd::getNextVirtualFocus(ifc_window *w)
{
	if (w == NULL)
	{
		if (childtabs.getNumItems() > 0)
			return childtabs.getFirst()->wnd;
	}

	int a = getTabOrderEntry(w) + 1;

	if (a < childtabs.getNumItems())
		return childtabs.enumItem(a)->wnd;

	return NULL;
}


void BaseWnd::setVirtualChildFocus(ifc_window *w)
{
	ASSERT(w && w->isVirtual());
	if (curVirtualChildFocus)
		curVirtualChildFocus->onKillFocus();
	curVirtualChildFocus = w;
	onSetRootFocus(w);
	Wasabi::Std::Wnd::setFocus(getOsWindowHandle());
	if (curVirtualChildFocus)
	curVirtualChildFocus->onGetFocus();
}

int BaseWnd::ptInRegion(int x, int y)
{
	RECT cr;
	getNonClientRect(&cr);
	POINT pt = {x - cr.left, y - cr.top};
	api_region *reg = getRegion();
	if (isRectRgn())
		return (x >= cr.left && x <= cr.right && y >= cr.top && y <= cr.bottom);
	return reg ? reg->ptInRegion(&pt) : 0;
}

api_region *BaseWnd::getComposedRegion()
{
	ensureWindowRegionValid();
	return composedrgn;
}

api_region *BaseWnd::getSubtractorRegion()
{
	ensureWindowRegionValid();
	return subtractorrgn;
}

int BaseWnd::ensureWindowRegionValid()
{
	if (!isInited()) return 0;
	if (wndregioninvalid)
	{
		computeComposedRegion();
		return 1;
	}
	return 0;
}

void BaseWnd::invalidateWindowRegion()
{
	wndregioninvalid = 1;
	if (getParent()) getParent()->invalidateWindowRegion();
}

void BaseWnd::computeComposedRegion()
{
	if (!isPostOnInit()) return ;

	wndregioninvalid = 0;

	RECT r;
	getNonClientRect(&r);

	api_region *reg = getRegion();
	RegionI *_reg = NULL;

	if (!reg)
	{
		_reg = new RegionI;
		reg = _reg;
		if (isRectRgn())
			reg->setRect(&r);
	}
	else
		if (isRectRgn())
			reg->setRect(&r);

	api_region *wr = reg->clone();

	if (!subtractorrgn) subtractorrgn = new RegionI();
	subtractorrgn->empty();
	if (!composedrgn) composedrgn = new RegionI;
	composedrgn->empty();

	RegionI *subme = NULL;
	RegionI *andme = NULL;
	RegionI *orme = NULL;

	// if subregion is now empty, we need to only use our region
	RECT gr;
	getNonClientRect(&gr);
	for (int i = 0;i < virtualChildren.getNumItems();i++)
	{
		ifc_window *srw = virtualChildren.enumItem(i);
		if (!srw->isVisible(1) || srw->getPaintingAlpha() == 0) continue;
		if (srw->getRegionOp() != REGIONOP_NONE)
		{
			api_region *sr = srw->getComposedRegion();
			if (sr)
			{
				api_region *osr = sr->clone();
				RECT r;
				srw->getNonClientRect(&r);
				r.left -= gr.left;
				r.top -= gr.top;
				osr->offset(r.left, r.top);
				/*        sr->debug();
				        osr->debug();*/
				if (srw->getRegionOp() == REGIONOP_OR)
				{
					if (!orme) orme = new RegionI;
					orme->addRegion(osr);
				}
				else if (srw->getRegionOp() == REGIONOP_AND)
				{
					if (!andme) andme = new RegionI;
					andme->addRegion(osr);
				}
				else if (srw->getRegionOp() == REGIONOP_SUB)
				{
					if (!subme) subme = new RegionI;
					subme->addRegion(osr);
				}
				else if (srw->getRegionOp() == REGIONOP_SUB2)
				{
					if (!subme) subme = new RegionI;
					subtractorrgn->addRegion(osr);
				}
				sr->disposeClone(osr);
			}
		}
		api_region *sr = srw->getSubtractorRegion();
		if (sr != NULL && !sr->isEmpty())
		{
			api_region *osr = sr->clone();
			RECT r;
			srw->getNonClientRect(&r);
			r.left -= gr.left;
			r.top -= gr.top;
			osr->offset(r.left, r.top);
			subtractorrgn->addRegion(osr);
			sr->disposeClone(osr);
		}
	}

	if (andme)
	{
		wr->andRegion(andme);
		delete andme;
	}
	if (orme)
	{
		wr->addRegion(orme);
		delete orme;
	}
	if (subme)
	{
		wr->subtractRgn(subme);
		delete subme;
	}

	composedrgn->addRegion(wr);
	reg->disposeClone(wr);
	delete _reg;
}

void BaseWnd::updateWindowRegion()
{
	if (!isPostOnInit() || isVirtual()) return ;
	if (getDesktopAlpha())
	{
		// if desktopalpha is on, we can't use regions (thanks MS), we have to rely on the framebuffer correctness
		//CUT    SetWindowRgn(getOsWindowHandle(), NULL, FALSE);
		Wasabi::Std::Wnd::setWndRegion(getOsWindowHandle(), NULL);
		return ;
	}
	api_region *_r = getComposedRegion();
	api_region *_s = getSubtractorRegion();
	ASSERT(_r != NULL && _s != NULL);

	api_region *z = _r->clone();
	z->subtractRgn(_s);

	assignWindowRegion(z);
	_r->disposeClone(z);
}

// wr is NOT scaled!!!
void BaseWnd::assignWindowRegion(api_region *wr)
{
	ASSERT(wr != NULL);

	if (!isPostOnInit()) return ;

	int isrect = wr->isRect();

	RECT r;
	BaseWnd::getWindowRect(&r);

	//DebugStringW( L"\nBaseWnd::assignWindowRegion() r  before - x = %d, y = %d, w = %d, h = %d \n", r.left, r.top, r.right - r.left, r.bottom - r.top );

	r.right  -= r.left;
	r.left    = 0;
	r.bottom -= r.top;
	r.top     = 0;

	//DebugStringW( L"BaseWnd::assignWindowRegion() r  after  - x = %d, y = %d, w = %d, h = %d \n", r.left, r.top, r.right - r.left, r.bottom - r.top );

	RECT z;
	wr->getBox(&z);

	//DebugStringW( L"BaseWnd::assignWindowRegion() z  before - x = %d, y = %d, w = %d, h = %d \n", z.left, z.top, z.right - z.left, z.bottom - z.top );

	z.left = 0;
	z.top = 0;

	//DebugStringW( L"BaseWnd::assignWindowRegion() z  after  - x = %d, y = %d, w = %d, h = %d \n", z.left, z.top, z.right - z.left, z.bottom - z.top );

	if (renderRatioActive())
	{
		double i = getRenderRatio();
		wr->scale(i, i, FALSE);
	}

	RECT sz;
	wr->getBox(&sz);

	//DebugStringW( L"BaseWnd::assignWindowRegion() sz before - x = %d, y = %d, w = %d, h = %d \n", sz.left, sz.top, sz.right - sz.left, sz.bottom - sz.top );

	sz.right  -= sz.left;
	sz.bottom -= sz.top;
	sz.left    = 0;
	sz.top     = 0;

	//DebugStringW( L"BaseWnd::assignWindowRegion() sz after  - x = %d, y = %d, w = %d, h = %d \n", sz.left, sz.top, sz.right - sz.left, sz.bottom - sz.top );

	if (isrect &&
	    ((z.right == rwidth && z.bottom == rheight) ||
	     (sz.left == r.left && sz.right == r.right && sz.top == r.top && sz.bottom == r.bottom) ||
	     (0)
	    )
	   )
	{
		//CUT    SetWindowRgn(getOsWindowHandle(), NULL, TRUE);
		if (!lastnullregion)
		{
			Wasabi::Std::Wnd::setWndRegion(getOsWindowHandle(), NULL, TRUE);
			lastnullregion = 1;
		}
	}
	else
	{
		//DebugStringW(L"setting region, rwidth = %d, rheight = %d, z.right = %d, z.bottom = %d\n", rwidth, rheight, z.right, z.bottom);
		//CUT    SetWindowRgn(getOsWindowHandle(), wr->makeWindowRegion(), TRUE);
		Wasabi::Std::Wnd::setWndRegion(getOsWindowHandle(), wr->makeWindowRegion(), TRUE);
		lastnullregion = 0;
	}
}

void BaseWnd::performBatchProcesses()
{
	// recompute the window region if needed and apply it to the HWND
	if (wndregioninvalid && !isVirtual())
		if (ensureWindowRegionValid())
			updateWindowRegion();
	if (need_flush_cascaderepaint)
	{
		_cascadeRepaintRgn(deferedCascadeRepaintRgn);
		deferedCascadeRepaintRgn->empty();
		need_flush_cascaderepaint = 0;
	}
}

int BaseWnd::getRegionOp()
{
	return regionop;
}

void BaseWnd::setRegionOp(int op)
{
	if (regionop != op)
	{
		regionop = op;
		invalidateWindowRegion();
	}
}

int BaseWnd::isRectRgn()
{
	return rectrgn;
}

void BaseWnd::setRectRgn(int i)
{
	rectrgn = i;
}

TimerClient *BaseWnd::timerclient_getMasterClient()
{
	if (!isVirtual()) return this;
	ifc_window *w = getParent();
	if (w)
	{
		TimerClient *tc = w->getTimerClient();
		if (tc)
			return tc->timerclient_getMasterClient();
	}
	return NULL;
}

void BaseWnd::timerclient_onMasterClientMultiplex()
{
	performBatchProcesses();
}

TimerClient *BaseWnd::getTimerClient()
{
	return this;
}

ifc_dependent *BaseWnd::rootwnd_getDependencyPtr()
{
	return this;
}

ifc_dependent *BaseWnd::timerclient_getDependencyPtr()
{
	return this;
}

void BaseWnd::addMinMaxEnforcer(ifc_window *w)
{
	minmaxEnforcers.addItem(w);
	signalMinMaxEnforcerChanged();
}

void BaseWnd::removeMinMaxEnforcer(ifc_window *w)
{
	minmaxEnforcers.removeItem(w);
	signalMinMaxEnforcerChanged();
}

void BaseWnd::signalMinMaxEnforcerChanged(void)
{
	ifc_window *w = getDesktopParent();
	if (w == NULL || w == this) onMinMaxEnforcerChanged();
	else w->signalMinMaxEnforcerChanged();
}

int BaseWnd::getNumMinMaxEnforcers()
{
	return minmaxEnforcers.getNumItems();
}

ifc_window *BaseWnd::enumMinMaxEnforcer(int n)
{
	return minmaxEnforcers.enumItem(n);
}

int BaseWnd::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source)
{
	return 1;
}

int BaseWnd::sendAction(ifc_window *target, const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen)
{
	ASSERT(target != NULL);
	return target->onAction(action, param, x, y, p1, p2, data, datalen, this);
}

int BaseWnd::virtualBeforePaint(api_region *r)
{
	if (!virtualCanvas) return 0;
	PaintCallbackInfoI pc(virtualCanvas, r);
	dependent_sendEvent(BaseWnd::depend_getClassGuid(), Event_ONPAINT, PaintCallback::BEFOREPAINT, &pc);
	return 1;
}

int BaseWnd::virtualAfterPaint(api_region *r)
{
	if (!virtualCanvas) return 0;
	PaintCallbackInfoI pc(virtualCanvas, r);
	dependent_sendEvent(BaseWnd::depend_getClassGuid(), Event_ONPAINT, PaintCallback::AFTERPAINT, &pc);
	return 1;
}

int BaseWnd::timerclient_onDeferredCallback(intptr_t p1, intptr_t p2)
{
	TimerClientI::timerclient_onDeferredCallback(p1, p2);
	return onDeferredCallback(p1, p2);
}

void BaseWnd::timerclient_timerCallback(int id)
{
	TimerClientI::timerclient_timerCallback(id);
	timerCallback(id);
}

int BaseWnd::setTimer(int id, int ms)
{
	return timerclient_setTimer(id, ms);
}

int BaseWnd::killTimer(int id)
{
	return timerclient_killTimer(id);
}

void BaseWnd::postDeferredCallback(intptr_t p1, intptr_t p2, int mindelay)
{
#ifdef _WIN32
	// TODO: re-enable, but post to some other window (e.g. some singleton window), not this window
	// because our message pump might be blocked
	// maybe make a hidden window in api_timer for this purpose

	//if (mindelay)
	timerclient_postDeferredCallback(p1, p2, mindelay);
	//else
	//PostMessage(hwnd, WM_DEFER_CALLBACK, p1, p2);
#else
#warning "port me - I can be optimized - don't use timers for this, use mac os x equiv of PostMessage!"
	timerclient_postDeferredCallback(p1, p2, mindelay);
#endif
}

int BaseWnd::bypassModal()
{
	return 0;
}

void BaseWnd::setRenderBaseTexture(int r)
{
	renderbasetexture = r;
	if (isInited()) invalidate();
}

int BaseWnd::getRenderBaseTexture()
{
	return renderbasetexture;
}

GuiObject *BaseWnd::getGuiObject()
{
	if (my_guiobject == NULL)
	{
		my_guiobject = static_cast<GuiObject *>(getInterface(guiObjectGuid));
	}
	return my_guiobject;
}

//CUT someday
int BaseWnd::getFlag(int flag)
{
	/*  switch (flag) {
	  }*/
	return 0;
}

int BaseWnd::triggerEvent(int event, intptr_t p1, intptr_t p2)
{
	//PORTME
	switch (event)
	{
	case TRIGGER_ONRESIZE:
		if (isPostOnInit())
			onResize();
		break;
	case TRIGGER_INVALIDATE:
		if (isPostOnInit())
			invalidate();
		break;
	}
	return 0;
}

void BaseWnd::registerAcceleratorSection(const wchar_t *name, int global)
{
#if defined(WASABI_COMPILE_LOCALES)
	WASABI_API_LOCALE->locales_registerAcceleratorSection(name, this, global);
#endif
}

int BaseWnd::onAcceleratorEvent(const wchar_t *name)
{
	for (int i = 0;i < getNumRootWndChildren();i++)
		if (enumRootWndChildren(i)->onAcceleratorEvent(name))
			return 1;
	return 0;
}

int BaseWnd::allowDeactivation()
{
	return allow_deactivate & ((WASABI_API_WND == NULL) || WASABI_API_WND->appdeactivation_isallowed(this));
}

void BaseWnd::onMinimize()
{
	if (!isVirtual())
	{
		dropVirtualCanvas();
	}
}

void BaseWnd::dropVirtualCanvas()
{
	deferedInvalidate();
	delete virtualCanvas;
	virtualCanvas = NULL;
}

void BaseWnd::onRestore()
{
	if (getDesktopParent() == this)
	{
		cascadeRepaint(TRUE);
	}
}

ifc_window *BaseWnd::findWindow(const wchar_t *id)
{
	RootWndFinder find_object;
	find_object.reset();
	find_object.setFindId(id);
	ifc_window *ret = findWindowChain(&find_object);

#ifdef _DEBUG
	if (ret == NULL)
		DebugStringW(L"findWindow : window not found by id ! %s \n", id);
#endif

	return ret;
}

ifc_window *BaseWnd::findWindowByInterface(GUID interface_guid)
{
	RootWndFinder find_object;
	find_object.reset();
	find_object.setFindGuid(interface_guid);
	ifc_window *ret = findWindowChain(&find_object);

#ifdef _DEBUG
	char str[256] = {0};
	nsGUID::toChar(interface_guid, str);
	if (ret == NULL)
		DebugStringW(L"findWindow : object not found by guid ! %s \n", str);
#endif

	return ret;
}

ifc_window *BaseWnd::findWindowByCallback(FindObjectCallback *cb)
{
	ifc_window *ret = findWindowChain(cb);
#ifdef _DEBUG
	if (ret == NULL)
		DebugStringW(L"findWindow : object not found by callback!\n");
#endif

	return ret;
}

ifc_window *BaseWnd::findWindowChain(FindObjectCallback *cb, ifc_window *wcaller)
{

	if (!cb) return NULL;

	if (cb->findobjectcb_matchObject(this)) return this;

	// first lets not look in subdirectories

	for (int i = 0;i < getNumRootWndChildren();i++)
	{
		ifc_window *child = enumRootWndChildren(i);
		if (!child || child == wcaller) continue;
		if (cb->findobjectcb_matchObject(child))
			return child;
	}

	// ok so it wasn't in our content, lets try to find it as a grandchildren

	for (int i = 0;i < getNumRootWndChildren();i++)
	{
		ifc_window *child = enumRootWndChildren(i);
		if (child->getNumRootWndChildren() > 0)
		{
			ifc_window *ret = child->findWindowChain(cb, this);
			if (ret) return ret;
		}
	}

	// so it wasnt one of our children, we'll hop the tree up one level and ask our parent to find it
	// for us. of course, our parents are smart, they won't ask us back when asking our sibblings
	ifc_window *p = getParent();
	if (p != NULL && wcaller != p)
	{
		return p->findWindowChain(cb, this);
	}

	return NULL;
}


const wchar_t *BaseWnd::timerclient_getName()
{
	tcname = StringPrintfW(L"name=\"%S\", id=\"%s\"", getRootWndName(), getId());
	return tcname;
}

void BaseWnd::setTabOrder(int a)
{
	if (getParent() != NULL)
		getParent()->setVirtualTabOrder(this, a);
}

int BaseWnd::getTabOrder()
{
	if (getParent() != NULL)
		return getParent()->getVirtualTabOrder(this);
	return -1;
}

void BaseWnd::recursive_setVirtualTabOrder(ifc_window *w, float a, float lambda)
{
	ASSERT(w != NULL);
	childtabs.setAutoSort(0);
	int i = getTabOrderEntry(a);
	if (i != -1)
	{
		TabOrderEntry *toe = childtabs.enumItem(i);
		if (toe->wnd != w)
		{
			lambda += TABORDER_K;
			if (lambda != 1.0)
				recursive_setVirtualTabOrder(toe->wnd, a + lambda, lambda);
		}
		else
		{
			return ;
		}
	}
	i = getTabOrderEntry(w);
	if (i != -1)
	{
		delete childtabs.enumItem(i);
		childtabs.removeByPos(i);
	}
	TabOrderEntry *toe = new TabOrderEntry;
	toe->wnd = w;
	toe->order = a;
	childtabs.addItem(toe);
}

void BaseWnd::setVirtualTabOrder(ifc_window *w, int a)
{
	if (a == -1)
	{
		delTabOrderEntry(w);
		return ;
	}
	recursive_setVirtualTabOrder(w, (float)a);
}

int BaseWnd::getVirtualTabOrder(ifc_window *w)
{
	int a = (int)getTabOrderEntry(w);
	if (a == -1) return -1;
	return (int)childtabs.enumItem(a);
}

int BaseWnd::getTabOrderEntry(ifc_window *w)
{
	foreach(childtabs)
	if (childtabs.getfor()->wnd == w)
		return foreach_index;
	endfor;
	return -1;
}

void BaseWnd::delTabOrderEntry(int i)
{
	int a = getTabOrderEntry((float)i);
	if (a == -1) return ;
	childtabs.removeByPos(a);
}

void BaseWnd::delTabOrderEntry(ifc_window *w)
{
	int a = getTabOrderEntry(w);
	if (a == -1) return ;
	delete childtabs.enumItem(a);
	childtabs.removeByPos(a);
}

int BaseWnd::getTabOrderEntry(float order)
{
	foreach(childtabs)
	if (childtabs.getfor()->order == order)
		return foreach_index;
	endfor;
	return -1;
}

void BaseWnd::setAutoTabOrder()
{
	if (!getParent()) return ;
	getParent()->setVirtualAutoTabOrder(this);
}

void BaseWnd::setVirtualAutoTabOrder(ifc_window *w)
{
	delTabOrderEntry(w);
	float o = 0;
	for (int i = 0;i < childtabs.getNumItems();i++)
	{
		o = MAX(o, childtabs.enumItem(i)->order);
	}
	setVirtualTabOrder(w, ((int)o) + 1);
}

void BaseWnd::focusNext()
{
	ifc_window *dp = getDesktopParent();
	if (dp != this)
	{
		if (dp != NULL)
			dp->focusNext();
		return ;
	}
	ifc_window *w = getTab(TAB_GETNEXT);
	if (w != NULL) w->setFocus();
}

void BaseWnd::focusPrevious()
{
	ifc_window *dp = getDesktopParent();
	if (dp != this)
	{
		if (dp != NULL)
			getDesktopParent()->focusPrevious();
		return ;
	}
	ifc_window *w = getTab(TAB_GETPREVIOUS);
	if (w != NULL) w->setFocus();
}

void BaseWnd::recursive_buildTabList(ifc_window *from, PtrList<ifc_window> *list)
{
	for (int i = 0;i < from->getNumTabs();i++)
	{
		ifc_window *r = from->enumTab(i);
		if (r->isVisible() && r->getPaintingAlpha() > 0)
		{
			if (r->wantFocus())
				list->addItem(r);
			recursive_buildTabList(r, list);
		}
	}
}

ifc_window *BaseWnd::getTab(int what)
{
	PtrList<ifc_window> listnow;

	recursive_buildTabList(this, &listnow);

	int p = listnow.searchItem(rootfocus);

	if (p == -1)
		for (int i = 0; i < listnow.getNumItems(); i++)
		{
			ifc_window *r = listnow.enumItem(i);
			if (r->gotFocus())
			{
				//DebugString("desync of rootfocus, fixing\n");
				p = i;
				assignRootFocus(r);
				break;
			}
		}

	if (what == TAB_GETNEXT && rootfocus != NULL)
	{

		p++;
		if (p >= listnow.getNumItems())
			p = 0;
		return listnow.enumItem(p);

	}
	else if (what == TAB_GETPREVIOUS && rootfocus != NULL)
	{

		p--;
		if (p < 0) p = listnow.getNumItems() - 1;
		return listnow.enumItem(p);

	}
	else if (what == TAB_GETCURRENT)
	{

		return rootfocus;

	}
	else if (what == TAB_GETFIRST || (what == TAB_GETNEXT && rootfocus == NULL))
	{

		return listnow.getFirst();

	}
	else if (what == TAB_GETLAST || (what == TAB_GETPREVIOUS && rootfocus == NULL))
	{

		return listnow.getLast();

	}

	return NULL;
}

int BaseWnd::getNumTabs()
{
	return childtabs.getNumItems();
}

ifc_window *BaseWnd::enumTab(int i)
{
	childtabs.sort();
	return childtabs.enumItem(i)->wnd;
}

void BaseWnd::onSetRootFocus(ifc_window *w)
{
	assignRootFocus(w);
	ifc_window *dp = getDesktopParent();
	if (dp && dp != this) dp->onSetRootFocus(w);
}

void BaseWnd::autoFocus(ifc_window *w)
{
	if (w->getFocusOnClick() && w->wantFocus())
	{
		w->setFocus();
		return ;
	}
	ifc_window *g = w;
	while (1)
	{
		ifc_window *p = g->getParent();
		if (p == NULL) break;
		ifc_window *dp = p->getDesktopParent();
		if (dp && dp != p)
		{
			if (p->wantFocus() && p->getFocusOnClick())
			{
				p->setFocus();
				return ;
			}
			g = p;
		}
		else
			break;
	}
}

void BaseWnd::setNoLeftClicks(int no)
{
	noleftclick = no;
}

void BaseWnd::setNoRightClicks(int no)
{
	norightclick = no;
}

void BaseWnd::setNoDoubleClicks(int no)
{
	nodoubleclick = no;
}

void BaseWnd::setNoMouseMoves(int no)
{
	nomousemove = no;
}

void BaseWnd::setNoContextMenus(int no)
{
	nocontextmnu = no;
}

void BaseWnd::setDefaultCursor(Cursor *c)
{
	customdefaultcursor = c;
}


OSCURSORHANDLE BaseWnd::getCustomCursor(int x, int y)
{
#ifdef _WIN32
	return customdefaultcursor ? customdefaultcursor->getOSHandle() : NULL;
#else
#warning port me
	return 0;
#endif
}

Accessible *BaseWnd::createNewAccObj()
{
	waServiceFactory *f = WASABI_API_SVC->service_enumService(WaSvc::ACCESSIBILITY, 0);
	if (f != NULL)
	{
		svc_accessibility *svc = castService<svc_accessibility>(f);
		if (svc != NULL)
		{
			Accessible *a = svc->createAccessibleObject(this);
			WASABI_API_SVC->service_release(svc);
			return a;
		}
	}
	return NULL;
}

Accessible *BaseWnd::getAccessibleObject(int createifnotexist)
{
	if (!createifnotexist) return accessible;
	if (!accessible)
		accessible = createNewAccObj();
	else
		accessible->addRef();
	return accessible;
}

int BaseWnd::accessibility_getState()
{
	int state = 0;
	if (!isVisible()) state |= STATE_SYSTEM_INVISIBLE;
	//if (isVirtual() && !wantFocus()) state |= STATE_SYSTEM_INVISIBLE;
	if (gotFocus()) state |= STATE_SYSTEM_FOCUSED;
	return state;
}

void BaseWnd::activate()
{
	Wasabi::Std::Wnd::setActiveWindow(getRootParent()->getOsWindowHandle());
}

void BaseWnd::setOSWndName(const wchar_t *name)
{
	if (isVirtual()) return ;
//#ifdef COMPILE_WASABI_SKIN // for some reason this isn't being correctly defined
	if (name)
	{
		Wasabi::Std::Wnd::setWndName(getOsWindowHandle(), name);
	}
	else
		Wasabi::Std::Wnd::setWndName(getOsWindowHandle(), L"");

}

const wchar_t *BaseWnd::getOSWndName()
{
	if (isVirtual()) return NULL;
	wchar_t str[4096] = {0};
	Wasabi::Std::Wnd::getWndName(getOsWindowHandle(), str, 4095);
	str[4095] = '\0';
	osname = str;
	return osname;
}

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
void BaseWnd::setAlwaysOnTop(int i)
{
	// this function should not optimize itself
	if (getDesktopParent() == this)
	{
		if (i)
		{
			//CUT      SetWindowPos(getOsWindowHandle(), HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER);
			Wasabi::Std::Wnd::setTopmost(getOsWindowHandle(), TRUE);
		}
		else
		{
			saveTopMosts();
			//CUT      SetWindowPos(getOsWindowHandle(), HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER);
			Wasabi::Std::Wnd::setTopmost(getOsWindowHandle(), FALSE);
			restoreTopMosts();
		}
		alwaysontop = i;
		return ;
	}
	ifc_window *p = getParent();
	if (p != NULL)
		p->setAlwaysOnTop(i);
}

int BaseWnd::getAlwaysOnTop()
{
	if (getDesktopParent() == this)
		return alwaysontop;
	ifc_window *p = getParent();
	if (!p) return 0;
	return p->getAlwaysOnTop();
}
#endif

void BaseWnd::wndwatcher_onDeleteWindow(ifc_window *w)
{
	if (w == rootfocus)
	{
		rootfocus = NULL;
	}
}

void BaseWnd::assignRootFocus(ifc_window *w)
{
	rootfocuswatcher.watchWindow(w);
	rootfocus = w;
}


Canvas *BaseWnd::getFrameBuffer()
{
	return virtualCanvas;
}

void BaseWnd::setForeignWnd(int i)
{
	m_takenOver = i;
}

int BaseWnd::bufferizeLockedUIMsg(int uMsg, int wParam, int lParam)
{
	if (WASABI_API_SKIN && !WASABI_API_SKIN->skin_getLockUI()) return 0;
	if (!uiwaslocked)
	{
		uiwaslocked = 1;
		setTimer(BUFFEREDMSG_TIMER_ID, 20);
	}
	bufferedMsgStruct msg;
	msg.msg = uMsg;
	msg.wparam = wParam;
	msg.lparam = lParam;
	bufferedmsgs.addItem(msg);
	return 1;
}

void BaseWnd::checkLockedUI()
{
	//PORTME :(
#ifdef _WIN32

	if (WASABI_API_SKIN && !WASABI_API_SKIN->skin_getLockUI())
	{
		uiwaslocked = 0;
		killTimer(BUFFEREDMSG_TIMER_ID);
		while (bufferedmsgs.getNumItems() > 0)
		{
			bufferedMsgStruct msg = bufferedmsgs.enumItem(0);
			bufferedmsgs.delByPos(0);
			SendMessageW(gethWnd(), msg.msg, msg.wparam, msg.lparam);
		}
		uiwaslocked = 0;
		killTimer(BUFFEREDMSG_TIMER_ID);
	}
#else
#warning port me
#endif
}

int BaseWnd::isMinimized()
{
	ifc_window *w = getDesktopParent();
	if (w == this || w == NULL) return minimized;
	return w->isMinimized();
}

int BaseWnd::reinit()
{
#ifdef _WIN32
	int nochild = (GetWindowLong(gethWnd(), GWL_STYLE) & WS_POPUP) ? 1 : 0;
	int r = reinit(parentWnd ? parentWnd : WASABI_API_WND->main_getRootWnd(), nochild);

	if (w2k_alpha)
		setLayeredWindow(1);

	return r;
#else
#warning port me!
#endif
}


int BaseWnd::reinit(ifc_window *parWnd, int nochild)
{
	OSWINDOWHANDLE phwnd = parWnd->getOsWindowHandle();
	ASSERT(phwnd != NULL);
	int ret;
	if (!nochild) parentWnd = parWnd;
	else parentWnd = NULL;
	ret = reinit(parWnd->getOsModuleHandle(), phwnd, nochild);
	if (!ret) parentWnd = NULL;	// abort
	return ret;
}

int BaseWnd::reinit(OSMODULEHANDLE moduleHandle, OSWINDOWHANDLE parent, int nochild)
{
	RECT r;
	int w, h;

	onBeforeReinit();

	pushWindowRect();
	preventcancelcapture = 1;

	int _isvisible = isVisible(1);
	int hadcapture = inputCaptured;
	//DebugString("had capture = %d\n", hadcapture);
	Wasabi::Std::Wnd::releaseCapture();

	unparentHWNDChildren();

	BaseWnd::getClientRect(&r);

	hinstance = moduleHandle;

	ASSERT(hinstance != NULL);

	w = (r.right - r.left);
	h = (r.bottom - r.top);

	rwidth = w;
	rheight = h;
	rx = r.left;
	ry = r.top;

	WASABI_API_WND->appdeactivation_push_disallow(this);

	// destroy old window
	Wasabi::Std::Wnd::hideWnd(hwnd); //Wasabi::Std::Wnd::destroyWnd(hwnd);
	ghosthwnd.addItem(hwnd);

	hwnd = Wasabi::Std::Wnd::createWnd(&r, nochild, acceptExternalDrops(), parent, hinstance, static_cast<ifc_window*>(this));
#ifdef __APPLE__
#warning remove me
	Wasabi::Std::Wnd::showWnd(hwnd);
#endif

	WASABI_API_WND->appdeactivation_pop_disallow(this);

	//ASSERT(hwnd != NULL); // lets fail nicely, this could happen for some win32 reason, we don't want to fail the whole app for it, so lets just fail the wnd
	if (hwnd == NULL)
	{
		preventcancelcapture = 0;
		return 0;
	}

	//CUT  nreal++;

#ifdef _WIN32
	//FUCKO
#ifdef URLDROPS
	if (acceptExternalDrops()) RegisterDragDrop(hwnd, &m_target);
#elif !defined(WA3COMPATIBILITY)
	if (!m_target && WASABI_API_WND != NULL)
		m_target = WASABI_API_WND->getDefaultDropTarget();
	if (m_target != NULL)
	{
		RegisterDragDrop(hwnd, (IDropTarget *)m_target);
	}
#endif
#else
#warning port me - register drag & drop
#endif

	this_visible = _isvisible;

	//onInit();
	//this_visible = !start_hidden;

	reparentHWNDChildren();

	popWindowRect();

	invalidateWindowRegion();
	updateWindowRegion();

	if (this_visible)
		Wasabi::Std::Wnd::showWnd(hwnd, FALSE);

	if (hadcapture)
	{
		Wasabi::Std::Wnd::setCapture(hwnd);
	}
	preventcancelcapture = 0;

	forcedOnResize();
	redrawHWNDChildren();
	//onPostOnInit();

	onAfterReinit();

#ifdef WASABI_ON_REPARENT
	WASABI_ON_REINIT(getOsWindowHandle());
#endif

	return 1;
}

ReparentWndEntry::ReparentWndEntry(OSWINDOWHANDLE _wnd, OSWINDOWHANDLE parentwnd)
{
	wnd = _wnd;
	Wasabi::Std::Wnd::getWindowRect(wnd, &rect);
	Wasabi::Std::Wnd::screenToClient(wnd, (int *)&(rect.left), (int *)&(rect.top));
	Wasabi::Std::Wnd::clientToScreen(parentwnd, (int *)&(rect.left), (int *)&(rect.top));
}

void ReparentWndEntry::unparent()
{
	Wasabi::Std::Wnd::setWndPos(wnd, NULL, rect.left, -30000, 0, 0, TRUE, TRUE, FALSE, FALSE, TRUE);
	Wasabi::Std::Wnd::setParent(wnd, NULL);
}

void ReparentWndEntry::reparent(OSWINDOWHANDLE newparent)
{
	Wasabi::Std::Wnd::setParent(wnd, newparent);
	Wasabi::Std::Wnd::setWndPos(wnd, NULL, rect.left, rect.top, 0, 0, TRUE, TRUE, FALSE, FALSE, TRUE);
}

#ifdef _WIN32
void BaseWnd::unparentHWNDChildren()
{
	// just in case
	reparentwnds.deleteAll();

#ifndef WIN32
#error port me ! // make a list of all the children oswindows and reparent them to the desktop somewhere we can't see
#endif

	OSWINDOWHANDLE wnd = GetWindow(getOsWindowHandle(), GW_CHILD);
	while (wnd)
	{
		reparentwnds.addItem(new ReparentWndEntry(wnd, getOsWindowHandle()));
		wnd = GetWindow(wnd, GW_HWNDNEXT);
	}
	foreach(reparentwnds)
	reparentwnds.getfor()->unparent();
	endfor;
}
#endif

void BaseWnd::reparentHWNDChildren()
{
	// reparent to the new oswindowhandle
	foreach(reparentwnds)
	reparentwnds.getfor()->reparent(getOsWindowHandle());
	endfor;
}

void BaseWnd::redrawHWNDChildren()
{
	// reparent to the new oswindowhandle
	foreach(reparentwnds)
	Wasabi::Std::Wnd::update(getOsWindowHandle());
	endfor;
}

void BaseWnd::maximize(int axis)
{
	//DebugString("maximize!\n");
	// if already maximized, don't use current rect, use restore_rect
	if (!maximized)
	{
		restore_rect.left = rx;
		restore_rect.top = ry;
		restore_rect.right = rx + rwidth;
		restore_rect.bottom = ry + rheight;
	}

	RECT nr = restore_rect;
	RECT dr;

	Wasabi::Std::getViewport(&dr, NULL, NULL, getOsWindowHandle(), 0);

	if (axis & MAXIMIZE_WIDTH)
	{
		nr.left = dr.left;
		nr.right = dr.right;
	}
	if (axis & MAXIMIZE_HEIGHT)
	{
		nr.top = dr.top;
		nr.bottom = dr.bottom;
	}
	maximized = 1;
	if (axis != 0) resize(&nr);
	onMaximize();
}

void BaseWnd::restore(int what)
{
	if (maximized)
	{
		//DebugString("restore!\n");
		if (what == (RESTORE_X | RESTORE_Y | RESTORE_WIDTH | RESTORE_HEIGHT))
			resize(&restore_rect);
		else
		{
			resize((what & RESTORE_X) ? restore_rect.left : NOCHANGE,
			       (what & RESTORE_Y) ? restore_rect.top : NOCHANGE,
			       (what & RESTORE_WIDTH) ? restore_rect.right - restore_rect.left : NOCHANGE,
			       (what & RESTORE_HEIGHT) ? restore_rect.bottom - restore_rect.top : NOCHANGE);
		}
		maximized = 0;
		onRestore();
	}
}

void BaseWnd::pushWindowRect()
{
	//DebugString("pushWindowRect\n");
	RECT wr;
	getWindowRect(&wr);
	wr.right = wr.left + rwidth;
	wr.bottom = wr.top + rheight;
	windowrectstack.push(wr);
}

int BaseWnd::popWindowRect(RECT *rc, int applyhow)
{
	//DebugString("popWindowRect\n");
	if (windowrectstack.peek() == 0) return 0;
	RECT _rc;
	windowrectstack.pop(&_rc);
	RECT r;
	getWindowRect(&r);
	divRatio(&r);
	if (applyhow)
	{
		if (applyhow == PWR_POSITION)
		{
			move(_rc.left, _rc.top);
			if (rc)
			{
				int w = r.right - r.left;
				int h = r.bottom - r.top;
				rc->left = _rc.left;
				rc->top = _rc.top;
				rc->right = rc->left + w;
				rc->bottom = rc->top + h;
			}
		}
		else
		{
			if (applyhow & PWR_X) r.left = _rc.left;
			if (applyhow & PWR_Y) r.top = _rc.top;
			if (applyhow & PWR_WIDTH) r.right = r.left + (_rc.right - _rc.left);
			if (applyhow & PWR_HEIGHT) r.bottom = r.top + (_rc.bottom - _rc.top);
			resizeToRect(&r);
			if (rc) *rc = _rc;
		}
	}
	else if (rc) *rc = _rc;
	return 1;
}

void BaseWnd::setRestoredRect(RECT *r)
{
	if (!r)
		return ;

	restore_rect = *r;
	maximized = 1;
}

int BaseWnd::getRestoredRect(RECT *r)
{
	if (!r)
		return 0;

	if (!maximized)
		return 0;

	*r = restore_rect;

	return 1;
}

void BaseWnd::notifyDeferredMove(int x, int y)
{
	rx = x;
	ry = y;
}

void BaseWnd::setWindowTitle(const wchar_t *title)
{
	Layout *l = static_cast<Layout *>(getInterface(layoutGuid));
	if (l) 
	{
		Container *c = l->getParentContainer();
		if (c)
		{
			c->setName(title);
		}
	}
}

#ifdef __APPLE__
OSStatus BaseWnd::eventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	return eventNotHandledErr;
}
#endif
