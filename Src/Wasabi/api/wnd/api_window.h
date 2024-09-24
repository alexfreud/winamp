#ifndef __WASABI_API_WINDOW_H
#define __WASABI_API_WINDOW_H

#include <bfc/dispatch.h>
#include <api/wnd/drag.h>
#include <api/wnd/cursor.h>
#include <api/wnd/accessible.h>
#include <tataki/canvas/Canvas.h>
#include <api/dependency/api_dependent.h>
#include <api/timer/timerclient.h>
#include <api/script/objects/guiobject.h>
enum WndPreferences {
	SUGGESTED_X,
	SUGGESTED_Y,
	SUGGESTED_W,
	SUGGESTED_H,
	MINIMUM_W,
	MINIMUM_H,
	MAXIMUM_W,
	MAXIMUM_H,
};

enum GetTab {
	TAB_GETCURRENT,
	TAB_GETNEXT,
	TAB_GETPREVIOUS,
	TAB_GETFIRST,
	TAB_GETLAST,
};


#define MAXIMIZE_WIDTH   1
#define MAXIMIZE_HEIGHT  2

#define RESTORE_X        1
#define RESTORE_Y        2
#define RESTORE_HEIGHT   4
#define RESTORE_WIDTH    8

// popWindowRect flags
#define PWR_X             1
#define PWR_Y             2
#define PWR_WIDTH         4
#define PWR_HEIGHT        8
#define PWR_DIMENTIONS    (PWR_WIDTH|PWR_HEIGHT)
#define PWR_POSITION      (PWR_X|PWR_Y)

class FindObjectCallback;

class NOVTABLE ifc_window : public Dispatchable 
{
protected:
	ifc_window() {}	// protect constructor
public:
	static const GUID *depend_getClassGuid() {
		// {11981849-61F7-4158-8283-DA7DD006D732}
		static const GUID ret = 
		{ 0x11981849, 0x61f7, 0x4158, { 0x82, 0x83, 0xda, 0x7d, 0xd0, 0x6, 0xd7, 0x32 } };
		return &ret;
	}

	// this passes thru to the windows WndProc, if there is one -- NONPORTABLE
#if defined (_WIN32) || defined (_WIN64)
	virtual LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)=0;
#elif defined(__APPLE__)
  virtual OSStatus eventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData)=0;
#endif

	// get the OSWINDOWHANDLE (if there is one)
	OSWINDOWHANDLE gethWnd() { return getOsWindowHandle(); } // stay back compatibile
	OSWINDOWHANDLE getOsWindowHandle();
	OSMODULEHANDLE getOsModuleHandle();
	void performBatchProcesses(); // this is called after wndProc is called (under win32) to recompute batch operations such as calculating window regions, cascaderepainting, etc. this prevents N children from independently calling repaintTree for the whole gui on overlaping zones of the framebuffer. under OSes other than win32, this should be called after you've executed all your window events for this poll
	TimerClient *getTimerClient();

	const wchar_t *getRootWndName();
	const wchar_t *getId();

	int init(ifc_window *parent, int nochild=FALSE);
	int isInited();	// are we post init() ? USE THIS INSTEAD OF gethWnd()==1
	int isPostOnInit();	// are we post onInit() ? USE THIS INSTEAD OF ISINITED TO KNOW IF YOU CAN CALL ONRESIZE (if a function can be called by onInit and other parts of the code at the same time)

	int setVirtual(int i);

	int isClickThrough();
	int onMouseMove(int x, int y);
	int onLeftButtonUp(int x, int y);
	int onRightButtonUp(int x, int y);
	int onLeftButtonDown(int x, int y);
	int onRightButtonDown(int x, int y);
	int onLeftButtonDblClk(int x, int y);
	int onRightButtonDblClk(int x, int y);

	// fetch the DragInterface of the api_window here, can be NULL
	DragInterface *getDragInterface();

	int getCursorType(int x, int y);
	OSCURSORHANDLE getCustomCursor(int x, int y);

	// returns deepest child for point or yourself if no child there
	ifc_window *rootWndFromPoint(POINT *pt);

	void getClientRect(RECT *);
	void getNonClientRect(RECT *rect);
	// the onscreen coordinates
	int getWindowRect(RECT *r);

	void setVisible(int show);
	void setCloaked(int cloak);
	void onSetVisible(int show);
	int isVisible(int within=0);

	void *getInterface(GUID interface_guid);

	// painting stuff
	void invalidate();
	void invalidateRect(RECT *r);
	void invalidateRgn(api_region *r);
	void invalidateFrom(ifc_window *who);
	void invalidateRectFrom(RECT *r, ifc_window *who);
	void invalidateRgnFrom(api_region *r, ifc_window *who);
	void onChildInvalidate(api_region *r, ifc_window *who);
	void validate();
	void validateRect(RECT *r);
	void validateRgn(api_region *reg);
	int paintTree(ifc_canvas *canvas, api_region *r);
	int paint(Canvas *canvas=NULL, api_region *r=NULL);
	Canvas *getFrameBuffer();
	ifc_window *getParent();
	ifc_window *getRootParent();
	ifc_window *getDesktopParent();
	void setParent(ifc_window *newparent);
	int onSiblingInvalidateRgn(api_region *r, ifc_window *who, int who_idx, int my_idx);
	int wantSiblingInvalidations();
	int wantAutoContextMenu();
	int wantActivation();
	void activate();
	int cascadeRepaintFrom(ifc_window *who, int pack=1);
	int cascadeRepaintRgnFrom(api_region *reg, ifc_window *who, int pack=1);
	int cascadeRepaintRectFrom(RECT *r, ifc_window *who, int pack=1);
	int cascadeRepaint(int pack=1);
	int cascadeRepaintRgn(api_region *reg, int pack=1);
	int cascadeRepaintRect(RECT *r, int pack=1);
	void repaint();
	ifc_window *getBaseTextureWindow();
	int childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2);
	int getPreferences(int what);
	void setPreferences(int what, int v);
	api_region *getRegion();
	void invalidateWindowRegion();
	api_region *getComposedRegion();
	api_region *getSubtractorRegion();
	int getRegionOp();
	void setRegionOp(int op);
	int isRectRgn();
	void setRectRgn(int rrgn);
	void setStartHidden(int sh);
	void setClickThrough(int ct);
	void focusNext();
	void focusPrevious();

	void setWindowTitle(const wchar_t *name);

	double getRenderRatio();
	void setRenderRatio(double r);
	void setRatioLinked(int l); // 1 = getRenderRatio asks parent, 0 = returns this ratio, default is 1, 0 should only be used for non virtuals
	int handleRatio();
	void resize(int x, int y, int w, int h, int wantcb=1);
	inline void resizeToRect(RECT *r) 
  {
		resize(r->left, r->top, r->right - r->left, r->bottom - r->top);
	}
	void move(int x, int y);
	void notifyDeferredMove(int x, int y);
	void getPosition(POINT *pt);
	ifc_window *getForwardWnd();

	// children registration/enumeration
	void registerRootWndChild(ifc_window *child);
	void unregisterRootWndChild(ifc_window *child);
	ifc_window *findRootWndChild(int x, int y, int only_virtuals=0);
	ifc_window *enumRootWndChildren(int _enum);
	int getNumRootWndChildren();

	// virtual child stuff
	int isVirtual();
	void bringVirtualToFront(ifc_window *w);
	void bringVirtualToBack(ifc_window *w);
	void bringVirtualAbove(ifc_window *w, ifc_window *b);
	void bringVirtualBelow(ifc_window *w, ifc_window *b);
	int checkDoubleClick(int button, int x, int y);

	void onCancelCapture();
	void cancelCapture();
	void setVirtualChildCapture(ifc_window *child);
	ifc_window *getVirtualChildCapture();
	ifc_window *getCurVirtualChildFocus();
	ifc_window *getTab(int what=TAB_GETCURRENT);

	bool ptInRegion(int x, int y);

	void clientToScreen(int *x, int *y); // so rootWndFromPoint can map ratio
	void screenToClient(int *x, int *y); // ..	

	int getNotifyId();

	int onActivate();
	int onDeactivate();
	int isActive();
	int handleTransparency();
	int handleDesktopAlpha();
	void setEnabled(int e);
	int onEnable(int e);
	int isEnabled(int within=0);
	int getPaintingAlpha();           
	void getAlpha(int *activealpha=NULL, int *inactivealpha=NULL);
	void setAlpha(int activealpha, int inactivealpha=-1); // -1 means same as activealpha
	void setTip(const wchar_t *tip);

	int runModal();
	void endModal(int retcode);

	void bringToFront();
	void bringToBack();
	void setFocus();
	int gotFocus();
	ifc_window *getNextVirtualFocus(ifc_window *w);
	void setVirtualChildFocus(ifc_window *w);
	void setVirtualTabOrder(ifc_window *w, int a);
	int getVirtualTabOrder(ifc_window *w);
	int wantFocus();
	void setTabOrder(int a);
	int getTabOrder();
	void setAutoTabOrder();
	void setVirtualAutoTabOrder(ifc_window *w);

	int onAcceleratorEvent(const wchar_t *name);
	int onChar(unsigned int c);
	int onKeyDown(int keycode);
	int onKeyUp(int keycode);
	int onSysKeyDown(int keyCode, int keyData);
	int onSysKeyUp(int keyCode, int keyData);
	int onKillFocus();
	int onGetFocus();
	void onSetRootFocus(ifc_window *w);

	ifc_dependent *getDependencyPtr();
	void addMinMaxEnforcer(ifc_window *w);
	void removeMinMaxEnforcer(ifc_window *w);
	ifc_window *enumMinMaxEnforcer(int n);
	int getNumMinMaxEnforcers();
	void signalMinMaxEnforcerChanged();

	int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);

	void setRenderBaseTexture(int r);
	int getRenderBaseTexture();
	void renderBaseTexture(ifc_canvas *c, const RECT *r, int alpha=255);

	GuiObject *getGuiObject(); // not guaranteed non null
	int getFlag(int flag);

	// see basewnd.h for codes. this is to force an event
	int triggerEvent(int event, intptr_t p1=0, intptr_t p2=0);
	
	int allowDeactivation();
	void setAllowDeactivation(int allow);

	ifc_window *findWindow(const wchar_t *id);
	ifc_window *findWindowByInterface(GUID interface_guid);
	ifc_window *findWindowByCallback(FindObjectCallback *cb);
	ifc_window *findWindowChain(FindObjectCallback *cb, ifc_window *wcaller);

	ifc_window *enumTab(int i);
	int getNumTabs();

	int getFocusOnClick();
	void setFocusOnClick(int i);

	void setNoDoubleClicks(int no);
	void setNoLeftClicks(int no);
	void setNoRightClicks(int no);
	void setNoMouseMoves(int no);
	void setNoContextMenus(int no);

	int wantDoubleClicks();
	int wantLeftClicks();
	int wantRightClicks();
	int wantMouseMoves();
	int wantContextMenus();

	void setDefaultCursor(Cursor *c);

	Accessible *getAccessibleObject(int createifnotexist=1);

	int accessibility_getState();

#ifndef WA3COMPATIBILITY
	void setDropTarget(void *dt);
	void *getDropTarget();
#endif

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
	void setAlwaysOnTop(int i);
	int getAlwaysOnTop();
#endif

	void maximize(int axis=MAXIMIZE_WIDTH|MAXIMIZE_HEIGHT);
	void restore(int what=RESTORE_X|RESTORE_Y|RESTORE_WIDTH|RESTORE_HEIGHT);
	int getRestoredRect(RECT *r);
	int isMinimized();

	enum {
		Event_SETVISIBLE=100,	// param 2 is 1 or 0
		Event_ONPAINT=200, // param 2 is PaintCallback::BEFOREPAINT or PaintCallback::AFTERPAINT
		Event_ONINVALIDATE=300,
	};  

	enum {
		BATCHPROCESSES = 50,
		GETOSWINDOWHANDLE	= 100,
		GETOSMODULEHANDLE	= 105,
		GETROOTWNDNAME	= 110,
		GETID = 120,
		GETDRAGINTERFACE	= 200,
		FROMPOINT		= 300,
		GETWINDOWRECT	= 400,
		ISVISIBLE		= 501,
		ONMETRICCHANGE	= 700,
		PAINTTREE = 900,
		PAINT = 910,
		GETFRAMEBUFFER = 920,
		GETPARENT = 1000,
		GETROOTPARENT = 1001,
		SETPARENT = 1002,
		GETDESKTOPPARENT = 1003,
		CHILDNOTIFY = 1200,
		GETPREFERENCES = 1300,
		SETPREFERENCES = 1310,
		CLICKTHROUGH = 1500,
		GETFORWARDWND = 1501,
		SETCLICKTHROUGH = 1502,
		CLIENTSCREEN = 1600,
		SCREENCLIENT = 1601,
		INIT = 1700,
		ISINITED = 1701,
		ISPOSTONINIT = 1702,
		SETVIRTUAL = 1704,
		GETCURSORTYPE = 1800,
		GETCUSTOMCURSOR = 1801,
		GETINTERFACE = 1850,
		GETCLIENTRECT = 1900,
		GETNONCLIENTRECT = 1901,
		SETVISIBLE = 2000,
		ONSETVISIBLE = 2001,
		SETCLOAKED = 2002,
		INVALIDATE = 2100,
		INVALIDATERECT = 2101,
		INVALIDATERGN = 2102,
		INVALIDATEFROM = 2103,
		INVALIDATERECTFROM = 2104,
		INVALIDATERGNFROM = 2105,
		VALIDATE = 2200,
		VALIDATERECT = 2201,
		VALIDATERGN = 2202,
		ONSIBINVALIDATE = 2300,
		WANTSIBINVALIDATE = 2301,
		ONCHILDINVALIDATE = 2302,
		CASCADEREPAINTFROM = 2400,
		CASCADEREPAINTRECTFROM = 2401,
		CASCADEREPAINTRGNFROM = 2402,
		CASCADEREPAINT = 2403, 
		CASCADEREPAINTRECT = 2405,
		CASCADEREPAINTRGN = 2406,
		REPAINT = 2500,
		GETTEXTUREWND = 2600,
		ONACTIVATE = 2700,
		ACTIVATE = 2710,
		ONDEACTIVATE = 2800,
		ISACTIVATED = 2801,
		GETPOSITION = 2900,
		GETREGION = 3000,
		GETREGIONOP = 3001,
		INVALWNDRGN = 3002,
		GETCOMPOSEDRGN = 3003,
		GETSUBTRACTORRGN = 3010,
		SETREGIONOP = 3004,
		SETRECTRGN = 3005,
		ISRECTRGN = 3006,
		//SETPARAM = 3200, // deprecated, use the xmlobject interface
		HANDLETRANSPARENCY = 3300,
		HANDLEDESKTOPALPHA = 3400,
		SETSTARTHIDDEN = 3500,
		GETRENDERRATIO = 3600,
		SETRENDERRATIO = 3610,
		SETRATIOLINKED = 3615,
		HANDLERATIO = 3620,
		//_RESIZE = 3700, // deprecated, cut
		_RESIZE = 3701,
		_MOVE = 3710,
		NOTIFYDEFERREDMOVE = 3720,
		CHECKDBLCLK = 3800,
		REGISTERROOTWNDCHILD = 3960,
		UNREGISTERROOTWNDCHILD = 3965,
		FINDROOTWNDCHILD= 3970,
		ENUMROOTWNDCHILDREN = 3975,
		GETNUMROOTWNDCHILDREN = 3980,
		ISVIRTUAL = 3950,
		BRINGVTOFRONT = 4000,
		BRINGVTOBACK = 4010,
		BRINGVABOVE = 4020,
		BRINGVBELOW = 4030,
		SETVCAPTURE = 4100,
		GETVCAPTURE = 4110,
		SETVTIMER = 4200,
		KILLVTIMER = 4210,
		PTINREGION = 4400,
		ONLBDBLCLK = 4500,
		ONRBDBLCLK = 4510,
		ONLBUP = 4520,
		ONRBUP = 4530,
		ONLBDOWN = 4540,
		ONRBDOWN = 4550,
		ONMOUSEMOVE = 4560,
		CLIENTTOSCREEN = 4600,
		SCREENTOCLIENT = 4610,
		GETNOTIFYID = 4700,
		SETENABLED = 4800,
		ISENABLED = 4810,
		ONENABLE = 4811,
		SETALPHA = 4900,
		GETALPHA = 4910,
		GETPAINTALPHA = 4911,
		SETTOOLTIP = 4920,
		RUNMODAL = 5000,
		ENDMODAL = 5010,
		WANTAUTOCONTEXTMENU = 5100,
		ONCANCELCAPTURE = 5200,
		CANCELCAPTURE = 5210,
		BRINGTOFRONT = 5300,
		BRINGTOBACK = 5310,
		SETFOCUS = 5401,
		GOTFOCUS = 5410,
		GETNEXTVFOCUS = 5420,
		SETVFOCUS = 5430,
		ONKILLFOCUS = 5440,
		ONGETFOCUS = 5450,
		WANTFOCUS = 5460,
		ONKEYDOWN = 5500,
		ONKEYUP = 5510,
		ONCHAR = 5520,
		ONACCELERATOREVENT = 5530,
		GETTIMERCLIENT = 5600,
		GETDEPENDENCYPTR = 6000,
		ADDMINMAXENFORCER = 6400,
		REMOVEMINMAXENFORCER = 6410,
		GETNUMMINMAXENFORCERS = 6420,
		ENUMMINMAXENFORCER = 6430,
		SIGNALMINMAXCHANGED = 6440,
		ROOTWNDONACTION = 6300,
		SETRENDERBASETEXTURE = 6600,
		GETRENDERBASETEXTURE = 6610,
		RENDERBASETEXTURE=6611,
		GETGUIOBJECT = 6620,
		ONSYSKEYDOWN = 6630,
		ONSYSKEYUP = 6640,
		GETFLAG = 6650,
		TRIGGEREVENT = 6660,

		SETALLOWDEACTIVATE = 6700,
		ALLOWDEACTIVATE=6710,
		FINDWND_BYID=6800,
		FINDWND_BYGUID=6810,
		FINDWND_BYCB=6820,
		FINDWNDCHAIN=6830,
		SETTABORDER=6900,
		GETTABORDER=6910,
		SETAUTOTABORDER=6920,
		GETTAB=6940,
		SETVIRTUALTABORDER=7000,
		GETVIRTUALTABORDER=7010,
		SETVIRTUALAUTOTABORDER=7020,
		GETCURVIRTUALCHILDFOCUS=7030,
		FOCUSNEXT=7100,
		FOCUSPREVIOUS=7110,
		SETWINDOWTITLE=7120,
		GETNUMTABS = 7200,
		ENUMTAB = 7210,
		ONSETROOTFOCUS = 7300,
		GETFOCUSONCLICK = 7400,
		SETFOCUSONCLICK = 7410,
		SETNODOUBLECLICKS = 7500,
		SETNOLEFTCLICKS =  7510,
		SETNORIGHTCLICKS = 7520,
		SETNOMOUSEMOVES = 7530,
		SETNOCONTEXTMENUS = 7540,
		WANTDOUBLECLICKS = 7600,
		WANTLEFTCLICKS = 7610,
		WANTRIGHTCLICKS = 7620,
		WANTMOUSEMOVES = 7630,
		WANTCONTEXTMENUS = 7640,
		WANTACTIVATION = 7700,
		SETDEFAULTCURSOR = 7800,

		GETACCESSIBLEOBJECT = 8000,
		ACCGETSTATE = 8104,

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
		SETALWAYSONTOP = 8500,
		GETALWAYSONTOP = 8501,
#endif

#ifndef WA3COMPATIBILITY
		GETDROPTARGET = 8800,
		SETDROPTARGET = 8810,
#endif
		ISMINIMIZED = 8900,
		MAXIMIZE = 9200,
		RESTORE = 9210,
		GETRESTOREDRECT = 9220,
	};
};


// helper functions definitions
inline OSWINDOWHANDLE ifc_window::getOsWindowHandle() {
	return _call(GETOSWINDOWHANDLE, (OSWINDOWHANDLE)NULL);
}

inline OSMODULEHANDLE ifc_window::getOsModuleHandle() {
	return _call(GETOSMODULEHANDLE, (OSMODULEHANDLE)NULL);
}

inline void ifc_window::performBatchProcesses() {
	_voidcall(BATCHPROCESSES);
}

inline const wchar_t *ifc_window::getRootWndName() 
{
	return _call(GETROOTWNDNAME, (const wchar_t *)NULL);
}

inline const wchar_t *ifc_window::getId() {
	return _call(GETID, (const wchar_t *)NULL);
}

inline DragInterface *ifc_window::getDragInterface() {
	return _call(GETDRAGINTERFACE, (DragInterface*)0);
}

inline ifc_window *ifc_window::rootWndFromPoint(POINT *pt) {
	return _call(FROMPOINT, (ifc_window*)0, pt);
}

inline int ifc_window::getWindowRect(RECT *r) {
	return _voidcall(GETWINDOWRECT, r);
}

inline int ifc_window::isVisible(int within) {
	return _call(ISVISIBLE, 0, within);
}

inline int ifc_window::paintTree(ifc_canvas *canvas, api_region *r) {
	return _call(PAINTTREE, 0, canvas, r);
}

inline Canvas *ifc_window::getFrameBuffer() {
	return _call(GETFRAMEBUFFER, (Canvas *)NULL);
}

inline int ifc_window::paint(Canvas *canvas, api_region *r) {
	return _call(PAINT, 0, canvas, r);
}

inline ifc_window *ifc_window::getParent() {
	return _call(GETPARENT, (ifc_window *)0);
}

inline ifc_window *ifc_window::getRootParent() {
	return _call(GETROOTPARENT, (ifc_window *)0);
}

inline ifc_window *ifc_window::getDesktopParent() {
	return _call(GETDESKTOPPARENT, (ifc_window *)0);
}

inline void ifc_window::setParent(ifc_window *parent) {
	_voidcall(SETPARENT, parent);
}

inline int ifc_window::childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2) {
	return _call(CHILDNOTIFY, 0, child, msg, p1, p2);
}

inline int ifc_window::getPreferences(int what) {
	return _call(GETPREFERENCES, 0, what);
}

inline void ifc_window::setPreferences(int what, int v) {
	_voidcall(SETPREFERENCES, what, v);
}

inline void ifc_window::cancelCapture() {
	_voidcall(CANCELCAPTURE, 0);
}

inline void ifc_window::onCancelCapture() {
	_voidcall(ONCANCELCAPTURE, 0);
}

inline int ifc_window::isClickThrough() {
	return _call(CLICKTHROUGH, 0);
}

inline void ifc_window::setClickThrough(int ct) {
	_voidcall(SETCLICKTHROUGH, ct);
}

inline ifc_window *ifc_window::getForwardWnd() {
	return _call(GETFORWARDWND, this);
}

inline void ifc_window::clientToScreen(int *x, int *y) {
	_voidcall(CLIENTSCREEN, x, y);
}

inline void ifc_window::screenToClient(int *x, int *y) {
	_voidcall(SCREENCLIENT, x, y);
}

inline int ifc_window::init(ifc_window *parent, int nochild) {
	return _call(INIT, 0, parent, nochild);
}

inline int ifc_window::isInited() {
	return _call(ISINITED, 0);
}

inline int ifc_window::isPostOnInit() {
	return _call(ISPOSTONINIT, 0);
}

inline int ifc_window::setVirtual(int i) {
	return _call(SETVIRTUAL, 0, i);
}

inline int ifc_window::getCursorType(int x, int y) {
	return _call(GETCURSORTYPE, 0, x, y);
}

inline OSCURSORHANDLE ifc_window::getCustomCursor(int x, int y) {
	return _call(GETCUSTOMCURSOR, (OSCURSORHANDLE)NULL, x, y);
}

inline void ifc_window::getClientRect(RECT *r) {
	_voidcall(GETCLIENTRECT, r);
}

inline void ifc_window::getNonClientRect(RECT *rect) {
	_voidcall(GETNONCLIENTRECT, rect);
}

inline void ifc_window::getPosition(POINT *pt) {
	_voidcall(GETPOSITION, pt);
}

inline void ifc_window::setVisible(int show) {
	_voidcall(SETVISIBLE, show);
}

inline void ifc_window::setCloaked(int cloak) {
	_voidcall(SETCLOAKED, cloak);
}

inline void ifc_window::onSetVisible(int show) {
	_voidcall(ONSETVISIBLE, show);
}

inline void ifc_window::invalidate() {
	_voidcall(INVALIDATE);
}

inline void ifc_window::invalidateRect(RECT *r) {
	_voidcall(INVALIDATERECT, r);
}

inline void ifc_window::invalidateRgn(api_region *r) {
	_voidcall(INVALIDATERGN, r);
}

inline void ifc_window::onChildInvalidate(api_region *r, ifc_window *who) {
	_voidcall(ONCHILDINVALIDATE, r, who);
}

inline void ifc_window::invalidateFrom(ifc_window *who) {
	_voidcall(INVALIDATEFROM, who);
}

inline void ifc_window::invalidateRectFrom(RECT *r, ifc_window *who) {
	_voidcall(INVALIDATERECTFROM, r, who);
}

inline void ifc_window::invalidateRgnFrom(api_region *r, ifc_window *who) {
	_voidcall(INVALIDATERGNFROM, r, who);
}

inline void ifc_window::validate() {
	_voidcall(VALIDATE);
}

inline void ifc_window::validateRect(RECT *r) {
	_voidcall(VALIDATERECT, r);
}

inline void ifc_window::validateRgn(api_region *reg) {
	_voidcall(VALIDATERGN, reg);
}

inline int ifc_window::onSiblingInvalidateRgn(api_region *r, ifc_window *who, int who_idx, int my_idx) {
	return _call(ONSIBINVALIDATE, 0, r, who, who_idx, my_idx);
}

inline int ifc_window::wantSiblingInvalidations() {
	return _call(WANTSIBINVALIDATE, 0);
}

inline int ifc_window::cascadeRepaintFrom(ifc_window *who, int pack) {
	return _call(CASCADEREPAINTFROM, 0, who, pack);
}

inline int ifc_window::cascadeRepaintRgnFrom(api_region *reg, ifc_window *who, int pack) {
	return _call(CASCADEREPAINTRGNFROM, 0, reg, who, pack);
}

inline int ifc_window::cascadeRepaintRectFrom(RECT *r, ifc_window *who, int pack) {
	return _call(CASCADEREPAINTRECTFROM, 0, r, who, pack);
}

inline int ifc_window::cascadeRepaint(int pack) {
	return _call(CASCADEREPAINT, 0, pack);
}

inline int ifc_window::cascadeRepaintRgn(api_region *reg, int pack) {
	return _call(CASCADEREPAINTRGN, 0, reg, pack);
}

inline int ifc_window::cascadeRepaintRect(RECT *r, int pack) {
	return _call(CASCADEREPAINTRECT, 0, r, pack);
}

inline void ifc_window::repaint() {
	_voidcall(REPAINT);
}

inline ifc_window *ifc_window::getBaseTextureWindow() {
	return _call(GETTEXTUREWND, (ifc_window *)0);
}

inline int ifc_window::onActivate() {
	return _call(ONACTIVATE, 0);
}

inline void ifc_window::activate() {
	_voidcall(ACTIVATE);
}

inline int ifc_window::onDeactivate() {
	return _call(ONDEACTIVATE, 0);
}

inline int ifc_window::isActive() {
	return _call(ISACTIVATED, 0);
}

inline api_region *ifc_window::getRegion() {
	return _call(GETREGION, (api_region *)NULL);
} 

inline int ifc_window::handleTransparency() {
	return _call(HANDLETRANSPARENCY, 0);
}

inline int ifc_window::handleDesktopAlpha() {
	return _call(HANDLEDESKTOPALPHA, 0);
}

inline void ifc_window::setStartHidden(int sh) {
	_voidcall(SETSTARTHIDDEN, sh);
}

inline double ifc_window::getRenderRatio() {
	return _call(GETRENDERRATIO, 1.0);
}

inline void ifc_window::setRenderRatio(double r) {
	_voidcall(SETRENDERRATIO, r);
}

inline void ifc_window::setRatioLinked(int l) {
	_voidcall(SETRATIOLINKED, l);
}

inline int ifc_window::handleRatio() {
	return _call(HANDLERATIO, 0);
}

inline void ifc_window::resize(int x, int y, int w, int h, int wantcb) {
	_voidcall(_RESIZE, x, y, w, h, wantcb);
}

inline void ifc_window::move(int x, int y) {
	_voidcall(_MOVE, x, y);
}

inline void ifc_window::notifyDeferredMove(int x, int y) {
	_voidcall(NOTIFYDEFERREDMOVE, x, y);
}

inline int ifc_window::checkDoubleClick(int button, int x, int y) {
	return _call(CHECKDBLCLK, 0, button, x, y);
}

inline void ifc_window::registerRootWndChild(ifc_window *child) {
	_voidcall(REGISTERROOTWNDCHILD, child);
}

inline void ifc_window::unregisterRootWndChild(ifc_window *child) {
	_voidcall(UNREGISTERROOTWNDCHILD, child);
}

inline ifc_window *ifc_window::findRootWndChild(int x, int y, int only_virtuals) {
	return _call(FINDROOTWNDCHILD, (ifc_window *)NULL, x, y, only_virtuals);
}

inline ifc_window *ifc_window::enumRootWndChildren(int _enum) {
	return _call(ENUMROOTWNDCHILDREN, (ifc_window *)NULL, _enum);
}

inline int ifc_window::getNumRootWndChildren() {
	return _call(GETNUMROOTWNDCHILDREN, 0);  
}

inline int ifc_window::isVirtual() {
	return _call(ISVIRTUAL, 0);
}

inline void ifc_window::bringVirtualToFront(ifc_window *w) {
	_voidcall(BRINGVTOFRONT, w);
}

inline void ifc_window::bringVirtualToBack(ifc_window *w) {
	_voidcall(BRINGVTOBACK, w);
}

inline void ifc_window::bringVirtualAbove(ifc_window *w, ifc_window *b) {
	_voidcall(BRINGVABOVE, w, b);
}

inline void ifc_window::bringVirtualBelow(ifc_window *w, ifc_window *b) {
	_voidcall(BRINGVBELOW, w, b);
}

inline void ifc_window::setVirtualChildCapture(ifc_window *child) {
	_voidcall(SETVCAPTURE, child);
}

inline ifc_window *ifc_window::getVirtualChildCapture() {
	return _call(GETVCAPTURE, (ifc_window *)NULL);
}

inline bool ifc_window::ptInRegion(int x, int y) {
	return _call(PTINREGION, (bool)false, x, y);
}

inline int ifc_window::onLeftButtonDblClk(int x, int y) {
	return _call(ONLBDBLCLK, 0, x, y);
}

inline int ifc_window::onRightButtonDblClk(int x, int y) {
	return _call(ONRBDBLCLK, 0, x, y);
}

inline int ifc_window::onLeftButtonUp(int x, int y) {
	return _call(ONLBUP, 0, x, y);
}

inline int ifc_window::onRightButtonUp(int x, int y) {
	return _call(ONRBUP, 0, x, y);
}

inline int ifc_window::onLeftButtonDown(int x, int y) {  
	return _call(ONLBDOWN, 0, x, y);
}

inline int ifc_window::onRightButtonDown(int x, int y) {
	return _call(ONRBDOWN, 0, x, y);
}

inline int ifc_window::onMouseMove(int x, int y) {
	return _call(ONMOUSEMOVE, 0, x, y);
}

inline int ifc_window::getNotifyId() {
	return _call(GETNOTIFYID, 0);
}

inline void *ifc_window::getInterface(GUID interface_guid) {
	return _call(GETINTERFACE, (void *)NULL, interface_guid);
}

inline void ifc_window::setEnabled(int e) {
	_voidcall(SETENABLED, e);
}

inline int ifc_window::isEnabled(int within) {
	return _call(ISENABLED, 0, within);
}

inline int ifc_window::onEnable(int e) {
	return _voidcall(ONENABLE, e);
}

inline void ifc_window::setAlpha(int activealpha, int inactivealpha) {
	_voidcall(SETALPHA, activealpha, inactivealpha);
}

inline void ifc_window::getAlpha(int *active, int *inactive) {
	_voidcall(GETALPHA, active, inactive);
}

inline int ifc_window::getPaintingAlpha() {
	return _call(GETPAINTALPHA, 255);
}

inline void ifc_window::setTip(const wchar_t *tip) {
	_voidcall(SETTOOLTIP, tip);
}

inline int ifc_window::runModal() {
	return _call(RUNMODAL, 0);
}

inline void ifc_window::endModal(int retcode) {
	_voidcall(ENDMODAL, retcode);
}

inline int ifc_window::wantAutoContextMenu() {
	return _call(WANTAUTOCONTEXTMENU, 0);
}

inline int ifc_window::wantActivation() {
	return _call(WANTACTIVATION, 1); 
}

inline void ifc_window::bringToFront() {
	_voidcall(BRINGTOFRONT);
}

inline void ifc_window::bringToBack() {
	_voidcall(BRINGTOBACK);
}

inline void ifc_window::setFocus() {
	_voidcall(SETFOCUS);
}

inline int ifc_window::gotFocus() {
	return _call(GOTFOCUS, 0);
}

inline int ifc_window::onGetFocus() {
	return _call(ONGETFOCUS, 0);
}

inline int ifc_window::onKillFocus() {
	return _call(ONKILLFOCUS, 0);
}

inline ifc_window *ifc_window::getNextVirtualFocus(ifc_window *cur) {
	return _call(GETNEXTVFOCUS, (ifc_window *)NULL, cur);
}

inline int ifc_window::wantFocus() {
	return _call(WANTFOCUS, 0);
}

inline int ifc_window::onAcceleratorEvent(const wchar_t *name) {
	return _call(ONACCELERATOREVENT, 0, name);
}

inline int ifc_window::onChar(unsigned int c) {
	return _call(ONCHAR, 0, c);
}

inline int ifc_window::onKeyDown(int keycode) {
	return _call(ONKEYDOWN, 0, keycode);
}

inline int ifc_window::onKeyUp(int keycode) {
	return _call(ONKEYUP, 0, keycode);
}

inline int ifc_window::onSysKeyDown(int keycode, int keydata) {
	return _call(ONSYSKEYDOWN, 0, keycode, keydata);
}

inline int ifc_window::onSysKeyUp(int keycode, int keydata) {
	return _call(ONSYSKEYUP, 0, keycode, keydata);
}

inline void ifc_window::setVirtualChildFocus(ifc_window *w) {
	_voidcall(SETVFOCUS, w);
}

inline int ifc_window::getRegionOp() {
	return _call(GETREGIONOP, 0);
}

inline void ifc_window::invalidateWindowRegion() {
	_voidcall(INVALWNDRGN);
}

inline api_region *ifc_window::getComposedRegion() {
	return _call(GETCOMPOSEDRGN, (api_region *)NULL);
}

inline api_region *ifc_window::getSubtractorRegion() {
	return _call(GETSUBTRACTORRGN, (api_region *)NULL);
}

inline void ifc_window::setRegionOp(int op) {
	_voidcall(SETREGIONOP, op);
}

inline void ifc_window::setRectRgn(int rrgn) {
	_voidcall(SETRECTRGN, rrgn);
}

inline int ifc_window::isRectRgn() {
	return _call(ISRECTRGN, 0);
}

inline TimerClient *ifc_window::getTimerClient() {
	return _call(GETTIMERCLIENT, (TimerClient *)NULL);
}

inline ifc_dependent *ifc_window::getDependencyPtr() {
	return _call(GETDEPENDENCYPTR, (ifc_dependent *)NULL);
}

inline void ifc_window::addMinMaxEnforcer(ifc_window *w) {
	_voidcall(ADDMINMAXENFORCER, w);
}

inline void ifc_window::removeMinMaxEnforcer(ifc_window *w) {
	_voidcall(REMOVEMINMAXENFORCER, w);
}

inline ifc_window *ifc_window::enumMinMaxEnforcer(int n) {
	return _call(ENUMMINMAXENFORCER, (ifc_window *)NULL, n);
}

inline int ifc_window::getNumMinMaxEnforcers() {
	return _call(GETNUMMINMAXENFORCERS, 0);
}

inline void ifc_window::signalMinMaxEnforcerChanged() {
	_voidcall(SIGNALMINMAXCHANGED, 0);
}

inline int ifc_window::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
	return _call(ROOTWNDONACTION, 0, action, param, x, y, p1, p2, data, datalen, source);
}

inline void ifc_window::setRenderBaseTexture(int i) {
	_voidcall(SETRENDERBASETEXTURE, i);
}

inline int ifc_window::getRenderBaseTexture() {
	return _call(GETRENDERBASETEXTURE, 0);
}

inline void ifc_window::renderBaseTexture(ifc_canvas *c, const RECT *r, int alpha)
{
	_voidcall(RENDERBASETEXTURE, c, r, alpha);
}

inline GuiObject *ifc_window::getGuiObject() {
	return _call(GETGUIOBJECT, (GuiObject *) NULL);
}

inline int ifc_window::triggerEvent(int event, intptr_t p1, intptr_t p2) {
	return _call(TRIGGEREVENT, 0, event, p1, p2);
}

inline int ifc_window::getFlag(int flag) {
	return _call(GETFLAG, 0, flag);
}

inline int ifc_window::allowDeactivation() {
	return _call(ALLOWDEACTIVATE, 1);
}

inline void ifc_window::setAllowDeactivation(int allow) {
	_voidcall(SETALLOWDEACTIVATE, allow);
}

inline ifc_window *ifc_window::findWindow(const wchar_t *id) {
	return _call(FINDWND_BYID, (ifc_window *)NULL, id);
}

inline ifc_window *ifc_window::findWindowByInterface(GUID interface_guid) {
	return _call(FINDWND_BYGUID, (ifc_window *)NULL, interface_guid);
}

inline ifc_window *ifc_window::findWindowByCallback(FindObjectCallback *cb) {
	return _call(FINDWND_BYCB, (ifc_window *)NULL, cb);
}

inline ifc_window *ifc_window::findWindowChain(FindObjectCallback *cb, ifc_window *wcaller) {
	return _call(FINDWNDCHAIN, (ifc_window *)NULL, cb, wcaller);
}

inline void ifc_window::setTabOrder(int a) {
	_voidcall(SETTABORDER, a);
}

inline int ifc_window::getTabOrder() {
	return _call(GETTABORDER, -1);
}

inline void ifc_window::setAutoTabOrder() {
	_voidcall(SETAUTOTABORDER, 0);
}

inline void ifc_window::setVirtualTabOrder(ifc_window *w, int a) {
	_voidcall(SETVIRTUALTABORDER, w, a);
}

inline int ifc_window::getVirtualTabOrder(ifc_window *w) {
	return _call(GETVIRTUALTABORDER, 0, w);
}

inline void ifc_window::setVirtualAutoTabOrder(ifc_window *w) {
	_voidcall(SETVIRTUALAUTOTABORDER, w);
}

inline ifc_window *ifc_window::getCurVirtualChildFocus() {
	return _call(GETCURVIRTUALCHILDFOCUS, (ifc_window *)NULL);
}

inline ifc_window *ifc_window::getTab(int what) {
	return _call(GETTAB, (ifc_window *)NULL, what);
}

inline void ifc_window::focusNext() {
	_voidcall(FOCUSNEXT);
}

inline void ifc_window::focusPrevious() {
	_voidcall(FOCUSPREVIOUS);
}

inline void ifc_window::setWindowTitle(const wchar_t *title){
	_voidcall(SETWINDOWTITLE, title);
}
inline int ifc_window::getNumTabs() {
	return _call(GETNUMTABS, 0);
}

inline ifc_window *ifc_window::enumTab(int i) {
	return _call(ENUMTAB, (ifc_window *)NULL, i);
}

inline void ifc_window::onSetRootFocus(ifc_window *w) {
	_voidcall(ONSETROOTFOCUS, w);
}

inline int ifc_window::getFocusOnClick() {
	return _call(GETFOCUSONCLICK, 0);
}

inline void ifc_window::setFocusOnClick(int i) {
	_voidcall(SETFOCUSONCLICK, i);
}

inline void ifc_window::setNoDoubleClicks(int no) {
	_voidcall(SETNODOUBLECLICKS, no);
}

inline void ifc_window::setNoLeftClicks(int no) {
	_voidcall(SETNOLEFTCLICKS, no);
}

inline void ifc_window::setNoRightClicks(int no) {
	_voidcall(SETNORIGHTCLICKS, no);
}

inline void ifc_window::setNoMouseMoves(int no) {
	_voidcall(SETNOMOUSEMOVES, no);
}

inline void ifc_window::setNoContextMenus(int no) {
	_voidcall(SETNOCONTEXTMENUS, no);
}

inline int ifc_window::wantDoubleClicks() {
	return _call(WANTDOUBLECLICKS, 1);
}

inline int ifc_window::wantRightClicks() {
	return _call(WANTLEFTCLICKS, 1);
}

inline int ifc_window::wantLeftClicks() {
	return _call(WANTRIGHTCLICKS, 1);
}

inline int ifc_window::wantMouseMoves() {
	return _call(WANTMOUSEMOVES, 1);
}

inline int ifc_window::wantContextMenus() {
	return _call(WANTCONTEXTMENUS, 1);
}

inline void ifc_window::setDefaultCursor(Cursor *c) {
	_voidcall(SETDEFAULTCURSOR, c);
}

inline Accessible *ifc_window::getAccessibleObject(int createifnotexist) {
	return _call(GETACCESSIBLEOBJECT, (Accessible*)NULL, createifnotexist);
}


inline int ifc_window::accessibility_getState() {
	return _call(ACCGETSTATE, 0);
}

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
inline void ifc_window::setAlwaysOnTop(int i) {
	_voidcall(SETALWAYSONTOP, i);
}

inline int ifc_window::getAlwaysOnTop() {
	return _call(GETALWAYSONTOP, 0);
}
#endif

#ifndef WA3COMPATIBILITY
inline void ifc_window::setDropTarget(void *dt) {
	_voidcall(SETDROPTARGET, dt);
}

inline void *ifc_window::getDropTarget() {
	return _call(GETDROPTARGET, (void *)NULL);
}
#endif

inline int ifc_window::isMinimized() {
	return _call(ISMINIMIZED, 0);
}

inline void ifc_window::maximize(int axis) {
	_voidcall(MAXIMIZE, axis);
}

inline void ifc_window::restore(int what) {
	_voidcall(RESTORE, what);
}

inline int ifc_window::getRestoredRect(RECT *r)
{
	return _call(GETRESTOREDRECT, 0, r);
}

#endif