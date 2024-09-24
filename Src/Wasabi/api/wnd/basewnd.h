//NONPORTABLE
#ifndef _BASEWND_H
#define _BASEWND_H

#include <api/wnd/api_wnd.h>
#include <api/timer/api_timer.h>
#include <bfc/wasabi_std.h>
#include <bfc/common.h>
#include <api/wnd/api_window.h>
#include <api/wnd/drag.h>
#include <bfc/ptrlist.h>
#include <bfc/tlist.h>
#include <bfc/named.h>
#include <api/timer/timerclient.h>
#include <api/wnd/findobjectcb.h>
#include <bfc/stack.h>
#include <api/wnd/rootwnd.h>
class BaseWnd;
#include <tataki/canvas/bltcanvas.h>
class FilenamePS;
class Canvas;
class ifc_canvas;
class RegionI;
class DragSet;
class VirtualWnd;
class Tooltip;
class svc_toolTipsRenderer;
class Accessible;

// for triggerEvent()
#define TRIGGER_ONRESIZE	1000
#define TRIGGER_INVALIDATE	2000

// it is safe to use anything higher than this for your own funky messages
#define BASEWND_NOTIFY_USER	NUM_NOTIFY_MESSAGES

#define BASEWND_CURSOR_USERSET			-1// window will do own setting
#define BASEWND_CURSOR_POINTER			1
#define BASEWND_CURSOR_NORTHSOUTH		2
#define BASEWND_CURSOR_EASTWEST			3
#define BASEWND_CURSOR_NORTHWEST_SOUTHEAST	4
#define BASEWND_CURSOR_NORTHEAST_SOUTHWEST	5
#define BASEWND_CURSOR_4WAY			6
#define BASEWND_CURSOR_EDIT			7

// Our own defined window messages

#define WM_WA_CONTAINER_TOGGLED WM_USER+0x1338
#define WM_WA_COMPONENT_TOGGLED WM_USER+0x1339
#define WM_WA_RELOAD            WM_USER+0x133A
#define WM_WA_GETFBSIZE         WM_USER+0x133B

#define DEFERREDCB_FLUSHPAINT   0x200

#define SYSRGN  4

#define REGIONOP_NONE 0
#define REGIONOP_OR   1
#define REGIONOP_AND  2
#define REGIONOP_SUB  -1
#define REGIONOP_SUB2 -2

#define TABORDER_K 0.0001f

class ReparentWndEntry {
  public:
    ReparentWndEntry(OSWINDOWHANDLE wnd, OSWINDOWHANDLE parentwnd);
    virtual ~ReparentWndEntry() {}
    void unparent();
    void reparent(OSWINDOWHANDLE newparent);
  private:
    RECT rect;
    OSWINDOWHANDLE wnd;
};

class virtualChildTimer {
  public:
    virtualChildTimer(int _id, ifc_window *_child) : id(_id), child(_child) { }
    virtual ~virtualChildTimer() {}
		int id;
		ifc_window *child;
};

class RootWndFinder : public FindObjectCallbackI {
  public:

    RootWndFinder() {  findobject_guid = INVALID_GUID; }
    virtual ~RootWndFinder() {  }

    void reset() { findobject_id = L""; findobject_guid = INVALID_GUID; }
    void setFindId(const wchar_t *id) { findobject_id = id; }
    void setFindGuid(GUID guid) { findobject_guid = guid; }

    virtual int findobjectcb_matchObject(ifc_window *w) {
      if (!findobject_id.isempty()) {
        if (w != NULL) {
          const wchar_t *id = w->getId();
          return WCSCASEEQLSAFE(id, findobject_id);
        }
      } else if (findobject_guid != INVALID_GUID) {
        return (w->getInterface(findobject_guid) != NULL);
      }
      return 0;
    }

  private:

    StringW findobject_id;
    GUID findobject_guid;
};

#ifdef _WIN32
typedef struct {
  HWND owner;
  HWND hthis;
  PtrList<ifc_window> *hlist;
} enumownedstruct;
#endif

class WndWatcher: public DependentViewerI {
  public:
    WndWatcher(BaseWnd *b) : watched(NULL), dep(NULL), watcher(b) { ASSERT(watcher != NULL); }
    WndWatcher() : watched(NULL), dep(NULL), watcher(NULL) { }
    virtual ~WndWatcher() {}

    virtual void setWatcher(BaseWnd *w) { 
      if (watcher != NULL)
        watchWindow(NULL);
      watcher = w; 
    }

    virtual void watchWindow(ifc_window *w) { 
      ASSERT(watcher != NULL);
      if (dep != NULL) {
        viewer_delViewItem(dep);
        dep = NULL;
        watched = NULL;
      }
      if (w != NULL) {
        watched = w; 
        dep = w->getDependencyPtr(); 
        viewer_addViewItem(dep);
      }
    }
    
    virtual int viewer_onItemDeleted(ifc_dependent *item);

  private:
    ifc_window *watched;
    ifc_dependent *dep;
    BaseWnd *watcher;
};

#ifdef _WIN32
BOOL CALLBACK EnumOwnedTopMostWindows(HWND hwnd, LPARAM lParam);
#endif

class TabOrderEntry {
  public:
    ifc_window *wnd;
    float order; // yes, float. if a wnd tries to set order n and n is already set for another wnd, that other wnd will be pushed to n+k
                 // with k = 0.0001 . recursively, if n+k is taken, it'll push that wnd to n+2k, which if taken has its wnd pushed to n+3k, etc
                 // if n+xk >= n+1 (when x = 10000), the 10000th entry is discarded (if you manage to make a dialog with 10000 keyboard fields inside
                 // a single group, you're nuts anyway, and you should die a painful death)
};

class TabOrderSort {
public:
  static int compareItem(TabOrderEntry *p1, TabOrderEntry *p2) {
    if (p1->order < p2->order) return -1;
    if (p1->order > p2->order) return 1;
    return 0; 
  }
};


// base class
#define BASEWND_PARENT RootWndI
class NOVTABLE BaseWnd :
  public RootWndI,
  public DragInterface,
  public NamedW,
  public DependentI, 
  public TimerClientI 
{

  friend class VirtualWnd;

protected:
  // override constructor to init your data, but don't create anything yet
  BaseWnd();
	
public:

  virtual ~BaseWnd();

//INITIALIZATION
  // these actually create the window
  // try REALLY hard to not have to override these, and if you do,
  // override the second one
  virtual int init(ifc_window *parent, int nochild=FALSE);
  virtual int init(OSMODULEHANDLE hInstance, OSWINDOWHANDLE parent, int nochild=FALSE);
  virtual int isInited();	// are we post init() ?
  virtual int isPostOnInit() { return postoninit; }

  virtual int setVirtual(int i) { return 0; }

  // if at all possible put your init stuff in this one, and call up the
  // heirarchy BEFORE your code
  virtual int onInit();
  virtual int onPostOnInit(); // this one is called after onInit so you get a chance to do something after your inheritor override

  // use this to return the cursor type to show
  virtual int getCursorType(int x, int y);

// WINDOW SIZING/MOVEMENT/CONTROL
  virtual int getFontSize() { return 0; }
  virtual int setFontSize(int size) { return -1; }

  // if you override it, be sure to call up the heirarchy
  virtual void resize(int x, int y, int w, int h, int wantcb=1);	// resize yourself
  void resize(RECT *r, int wantcb=1);
  void resizeToRect(RECT *r) { resize(r); }//helper

  // called after resize happens, return TRUE if you override
  virtual int onResize();
	virtual int onAfterResize() { return 1; }
  void resizeToClient(BaseWnd *wnd);	// resize a window to match you
  virtual int onPostedMove(); // whenever WM_WINDOWPOSCHANGED happens, use mainly to record positions when moved by the window tracker, avoid using for another prupose, not portable

  // move only, no resize
  virtual void move(int x, int y);
  
  virtual void notifyDeferredMove(int x, int y);

  // puts window on top of its siblings
  virtual void bringToFront();
  // puts window behind its siblings
  virtual void bringToBack();

  // fired when a sibbling invalidates a region. you can change the region before returning, use with care, can fuck up everything if not used well
  virtual int onSiblingInvalidateRgn(api_region *r, ifc_window *who, int who_idx, int my_idx) { return 0; }

  // set window's visibility
  virtual void setVisible(int show);
  virtual void setCloaked(int cloak);
  virtual void onSetVisible(int show);	// override this one
  virtual void onChildInvalidate(api_region *r, ifc_window *w) {}

  // enable/disable window for input
  virtual void setEnabled(int en);
  // grab the keyboard focus
  virtual void setFocus();
  virtual void setFocusOnClick(int f);
  virtual int getFocusOnClick() { return focus_on_click; }

  virtual int pointInWnd(POINT *p);

  // repaint yourself
  virtual void invalidate();	// mark entire window for repainting
  virtual void invalidateRect(RECT *r);// mark specific rect for repainting
  virtual void invalidateRgn(api_region *r);// mark specific rgn for repainting
  virtual void invalidateFrom(ifc_window *who);	// mark entire window for repainting
  virtual void invalidateRectFrom(RECT *r, ifc_window *who);// mark specific rect for repainting
  virtual void invalidateRgnFrom(api_region *r, ifc_window *who);// mark specific rgn for repainting
  virtual void validate(); // unmark the entire window from repainting
  virtual void validateRect(RECT *r); // unmark specific rect from repainting
  virtual void validateRgn(api_region *reg); // unmark specific region from repainting

  // no virtual on these please
  void deferedInvalidateRgn(api_region *h);
  void deferedInvalidateRect(RECT *r);
  void deferedInvalidate();
  void deferedValidateRgn(api_region *h); 
  void deferedValidateRect(RECT *r);
  void deferedValidate();
  api_region *getDeferedInvalidRgn();
  int hasVirtualChildren();
  virtual void setVirtualChildFocus(ifc_window *w);
  virtual ifc_window *getNextVirtualFocus(ifc_window *w);
  void setVirtualTabOrder(ifc_window *w, int a);
  int getVirtualTabOrder(ifc_window *w);
  virtual void setTabOrder(int a);
  virtual int getTabOrder();
  virtual void setAutoTabOrder();
  virtual void setVirtualAutoTabOrder(ifc_window *w);
  virtual void focusNext();
  virtual void focusPrevious();
  virtual ifc_window *getCurVirtualChildFocus() { return curVirtualChildFocus; }
  virtual ifc_window *getTab(int what=TAB_GETNEXT);

private: 
  //virtual int focusVirtualChild(api_window *child);
  virtual void physicalInvalidateRgn(api_region *r); 
  void autoFocus(ifc_window *w);
protected:
  virtual int ensureVirtualCanvasOk();
  virtual void setVirtualCanvas(Canvas *c);
  virtual void setRSize(int x, int y, int w, int h);

public:
  virtual void repaint();	// repaint right now!

  // override this to add decorations
  virtual void getClientRect(RECT *);
  RECT clientRect();	// helper
  virtual void getNonClientRect(RECT *);
  RECT nonClientRect();	// helper
  virtual void getWindowRect(RECT *);	// windows coords in screen system
  RECT windowRect();	// helper
  virtual void getPosition(POINT *pt); // window coord relative to os window (instead of rootparent)

  virtual void *getInterface(GUID interface_guid) { return NULL; }
  virtual void *dependent_getInterface(const GUID *classguid);

  virtual void clientToScreen(int *x, int *y);		// convenience fn
  virtual void screenToClient(int *x, int *y);		// convenience fn
  virtual void clientToScreen(POINT *pt);		// convenience fn
  virtual void screenToClient(POINT *pt);		// convenience fn
  virtual void clientToScreen(RECT *r);		// convenience fn
  virtual void screenToClient(RECT *r);		// convenience fn

  void setIcon(OSICONHANDLE icon, int _small);
  virtual void setSkinId(int id);

  virtual int getPreferences(int what);
  virtual void setPreferences(int what, int v);
  virtual void setStartHidden(int wtf);

  virtual void setDefaultCursor(Cursor *c);
  virtual Canvas *getFrameBuffer();

  void setWindowTitle(const wchar_t *title);


// from api_window
protected:
  virtual DragInterface *getDragInterface();
  virtual ifc_window *rootWndFromPoint(POINT *pt);
  virtual int rootwnd_paintTree(ifc_canvas *canvas, api_region *r);

  void assignRootFocus(ifc_window *w);

public:
  // override for custom processing (NONPORTABLE!)
#ifdef _WIN32
  virtual LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(__APPLE__)
  virtual OSStatus eventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData);
#endif
  virtual void performBatchProcesses(); // this is called after wndProc is called (under win32) to recompute batch operations such as calculating window regions, cascaderepainting, etc. this prevents N children from independently calling repaintTree for the whole gui on overlaping zones of the framebuffer. under OSes other than win32, this should be called after you've executed all your window events for this poll, try not to use it, or use a dirty bit to cut down on your job
  virtual const wchar_t *getRootWndName();
  virtual const wchar_t *getId();
// end from api_window

  // or override these, they're more portable
  virtual int onContextMenu(int x, int y);	// always return 1 if accept msg
  virtual int onChildContextMenu(int x, int y);	// always return 1 if accept msg

  // called on WM_PAINT by onPaint(Canvas, api_region *), if the canvas is null, create your own, if not, it will have update regions clipped for you
  virtual int onPaint(Canvas *canvas);
  void setTransparency(int amount);	// 0..255
  int getTransparency();

  // override those two
  virtual void timerCallback(int id);
  virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

  int setTimer(int id, int ms);
  int killTimer(int id);
  void postDeferredCallback(intptr_t p1, intptr_t p2=0, int mindelay=0);

  // from timerclient
  virtual int timerclient_onDeferredCallback(intptr_t p1, intptr_t p2);
  virtual void timerclient_timerCallback(int id);
  virtual TimerClient *timerclient_getMasterClient();
  virtual void timerclient_onMasterClientMultiplex();
  virtual TimerClient *getTimerClient();
  virtual const wchar_t *timerclient_getName();

private:
  virtual int onPaint(Canvas *canvas, api_region *r);

public:  
  // click-drag. FYI, the drag-drop handling overrides these
  virtual int onLeftButtonDown(int x, int y); 
  virtual int onRightButtonDown(int x, int y) { return 0; }
  virtual int onMouseMove(int x, int y);	// only called when mouse captured
  virtual int onLeftButtonUp(int x, int y);
  virtual int onRightButtonUp(int x, int y) { return 0; }
  virtual int onMouseWheelUp(int click, int lines);
  virtual int onMouseWheelDown(int click, int lines);
  virtual int beginCapture();
  virtual int endCapture();
  virtual int getCapture();	// returns 1 if this window has mouse/keyb captured
  virtual void onCancelCapture(); // called when someone steals the capture from you
  virtual void cancelCapture();

  // these will not be called in the middle of a drag operation
  virtual int onLeftButtonDblClk(int x, int y);
  virtual int onRightButtonDblClk(int x, int y);

  virtual int onGetFocus();	// you have the keyboard focus
  virtual int onKillFocus();	// you lost the keyboard focus
  virtual int gotFocus();
  virtual int isActive();
  virtual int onActivate();
  virtual void activate();
  virtual int onDeactivate();
  virtual int onEnable(int en);
  virtual int isEnabled(int within=0);

  virtual void registerAcceleratorSection(const wchar_t *name, int global=0);
  virtual int onAcceleratorEvent(const wchar_t *name);

  virtual int onChar(unsigned int c);
  virtual int onKeyDown(int keyCode) { return 0; }
  virtual int onKeyUp(int keyCode) { return 0; }
  virtual int onSysKeyDown(int keyCode, int keyData) { return 0; }
  virtual int onSysKeyUp(int keyCode, int keyData) { return 0; }

  virtual int onEraseBkgnd(HDC dc);// override and return 0 to authorize bkg painting, 1 to avoid it (default to 1)
  virtual int onUserMessage(int msg, int w, int l, int *r);

//CHILD->PARENT MESSAGES
  // feel free to override for your own messages, but be sure to call up the
  // chain for unhandled msgs
  // children should not call this directly if they don't have to; use
  // notifyParent on yourself instead
  // message ids should be put in notifmsg.h to avoid conflicts
  virtual int childNotify(ifc_window *child, int msg,
                          int param1=0, int param2=0);


  // don't try to override these
  void setParent(ifc_window *newparent);
  ifc_window *getParent();
  virtual ifc_window *getRootParent(); 
  virtual ifc_window *getDesktopParent();
  
  // avoid overriding this one if you can
  virtual int reparent(ifc_window *newparent);

  // override this one
  virtual void onSetParent(ifc_window *newparent) {}

  virtual api_region *getRegion(); // override to return your client region
  virtual void setRegionOp(int op); // 0 = none, 1 == or, 2=and, 3=xor, -1 = sub, -2 = sub thru groups
  virtual void setRectRgn(int i); // set to 1 if you don't want your region to clip your clicks

  virtual void invalidateWindowRegion(); // call this when your region has changed

  api_region *getComposedRegion(); // returns the result of and/or/subing your children regions with your region
  api_region *getSubtractorRegion(); // returns the composed subtracting region, that region is automatically subtracted from the desktop root parent's region
  int ptInRegion(int x, int y); // handled automatically if you override getRegion and isRectRgn, but you can still override it if you need
  virtual int getRegionOp(); //
  virtual int isRectRgn();

  // call this to notify your parent via its childNotify method
  virtual int notifyParent(int msg, int param1=0, int param2=0);

  // call this when you have received a childNotify and wish to
  // defer the notification to your own notify object.
  virtual int passNotifyUp(ifc_window *child, int msg,
                          int param1=0, int param2=0);

  // This allows you to set a custom integer ID for any object you control,
  // such that you can use its ID in a switch statement by calling getNotifyId()
  // which is dispatched through api_window.
  void setNotifyId(int id);
  virtual int getNotifyId();

  // from class Named
  virtual void onSetName();
  // non-virtuals only: sets the exported name of the OS window WITHOUT changing the Named member (getRootWndName() will not return this string)
  virtual void setOSWndName(const wchar_t *name);
  // non-virtuals only: retreive the exported name of the OS window. This is NOT the same as getRootWndName().
  virtual const wchar_t *getOSWndName();

  virtual const wchar_t *getTip();
  virtual void setTip(const wchar_t *tooltip);
  virtual int getStartHidden();
  virtual void abortTip();
  virtual int isVisible(int within=0);

	// Virtual windows functions
  virtual Canvas *createFrameBuffer(int w, int h);
  virtual void prepareFrameBuffer(Canvas *canvas, int w, int h);
  virtual void deleteFrameBuffer(Canvas *canvas);

  virtual void registerRootWndChild(ifc_window *child);
  virtual void unregisterRootWndChild(ifc_window *child);
  virtual ifc_window *findRootWndChild(int x, int y, int only_virtuals=0);
  virtual ifc_window *enumRootWndChildren(int _enum);
  virtual int getNumRootWndChildren();

  virtual int isVirtual();
  virtual ifc_window *enumVirtualChild(int _enum);
  virtual int getNumVirtuals();

  virtual int handleVirtualChildMsg(UINT uMsg,int x, int y, void *p=NULL, void *d=NULL);
  virtual void setVirtualChildCapture(ifc_window *child);
  virtual ifc_window *getVirtualChildCapture();

  virtual int cascadeRepaintFrom(ifc_window *who, int pack=1);
  virtual int cascadeRepaintRgnFrom(api_region *reg, ifc_window *who, int pack=1);
  virtual int cascadeRepaintRectFrom(RECT *r, ifc_window *who, int pack=1);
  virtual int cascadeRepaint(int pack=1);
  virtual int cascadeRepaintRgn(api_region *reg, int pack=1);
  virtual int cascadeRepaintRect(RECT *r, int pack=1);
  virtual void flushPaint();

  virtual void onMinimize();
  virtual void onRestore();
  virtual int isMinimized();
  virtual int isMaximized() { return maximized; }
  virtual void onMaximize() { }
                                                 
  virtual void freeResources();
  virtual void reloadResources();
  virtual int getDesktopAlpha();
  virtual int handleDesktopAlpha() { return isVirtual(); }
  virtual int getPaintingAlpha();           // this one is a hint for painting, it returns either activealpha or inactivealpha
  virtual void setAlpha(int activealpha, int inactivealpha=-1); // -1 means same as activealpha
  virtual void getAlpha(int *activealpha=NULL, int *inactivealpha=NULL);
  virtual int getFlag(int flag);
  virtual int triggerEvent(int event, intptr_t p1, intptr_t p2);

  void commitFrameBuffer(Canvas *canvas, RECT *r, double ratio);

  virtual int paint(Canvas *canvas, api_region *r);

protected:
  void do_flushPaint();
  virtual int paintTree(Canvas *canvas, api_region *r);
  virtual int virtualBeforePaint(api_region *r);
  virtual int virtualAfterPaint(api_region *r);
  int virtualOnPaint();
  virtual void setDesktopAlpha(int do_alpha);
  virtual void onSetDesktopAlpha(int a);

public:
	
  virtual OSWINDOWHANDLE getOsWindowHandle();
  virtual OSMODULEHANDLE getOsModuleHandle();

public:
	
  bool getNoCopyBits(void);
  void setNoCopyBits(bool ncb);
  BltCanvas *scalecanvas;

protected:
  virtual int checkDoubleClick(int button, int x, int y);

//MISC
public:
  virtual int isDestroying();	// in the middle of dying

//DRAGGING AND DROPPING -- (derived from DragInterface)

  // derived windows should call this if they detect a drag beginning
  // call once per datum per type of data being exposed. order is maintained
  int addDragItem(const wchar_t *droptype, void *item);
  // returns TRUE if drop was accepted
  int handleDrag();
  int resetDragSet();	// you don't need to call this

  // (called on dest) when dragged item enters the winder
  virtual int dragEnter(ifc_window *sourceWnd);
  // (called on dest) during the winder
  // FG> x/y are in screen corrdinates because target is a rootwnd
  virtual int dragOver(int x, int y, ifc_window *sourceWnd) { return 0; }
  // (called on src)
  virtual int dragSetSticky(ifc_window *wnd, int left, int right, int up, int down);
  // (called on dest) when dragged item leaves the winder
  virtual int dragLeave(ifc_window *sourceWnd) { return 0; }

  // when it finally is dropped:
 
  // called on destination window
  // FG> x/y are in screen corrdinates because target is a rootwnd
  virtual int dragDrop(ifc_window *sourceWnd, int x, int y) { return 0; }
  // called on source window
  virtual int dragComplete(int success) { return 0; }
  // in that order
  // must be called right before handleDrag();		(sender)
  void setSuggestedDropTitle(const wchar_t *title);

  // must be called from dragDrop();			(receiver)
  virtual const wchar_t *dragGetSuggestedDropTitle(void);
  virtual int dragCheckData(const wchar_t *type, int *nitems=NULL);
  virtual void *dragGetData(int slot, int itemnum);
  virtual int dragCheckOption(int option) { return 0; }

  // return TRUE if you support any of the datatypes this window is exposing
  virtual int checkDragTypes(ifc_window *sourceWnd) { return 0; }

// external drops 
  // override this and return 1 to receive drops from the OS
  virtual int acceptExternalDrops() { return 0; }

  virtual void onExternalDropBegin() {}
  virtual void onExternalDropDirScan(const wchar_t *dirname) {}
  virtual void onExternalDropEnd() {}

  virtual int bypassModal();

  virtual ifc_window *findWindow(const wchar_t *id);
  virtual ifc_window *findWindowByInterface(GUID interface_guid);
  virtual ifc_window *findWindowByCallback(FindObjectCallback *cb);
  virtual ifc_window *findWindowChain(FindObjectCallback *cb, ifc_window *wcaller=NULL);

private:
  void addDroppedFile(const wchar_t *filename, PtrList<FilenamePS> *plist); // recursive
  void setLayeredWindow(int i);
  Accessible *createNewAccObj();

public:
//FG> alternate notify window
  virtual void setNotifyWindow(ifc_window *newnotify);
  virtual ifc_window *getNotifyWindow();

  virtual double getRenderRatio();
  virtual void setRenderRatio(double r);
  virtual void onRatioChanged() {}
  virtual void setRatioLinked(int l);
  virtual int handleRatio();
  int renderRatioActive();
  void multRatio(int *x, int *y=NULL);
  void multRatio(RECT *r);
  void divRatio(int *x, int *y=NULL);
  void divRatio(RECT *r);
  virtual int isClickThrough();
  virtual void setClickThrough(int ct);
  virtual ifc_window *getForwardWnd() { return this; }

  virtual void setNoLeftClicks(int no);
  virtual void setNoRightClicks(int no);
  virtual void setNoDoubleClicks(int no);
  virtual void setNoMouseMoves(int no);
  virtual void setNoContextMenus(int no);

  // these functions are override that can be changed via XML. They are not intended to describe how your wnd should receive its messages, they are here rather
  // to allow a skinner to disable part of the functionality of an object (ie: removing the context menu via nocontextmenu="1").
  virtual int wantDoubleClicks() { return !nodoubleclick; }
  virtual int wantLeftClicks() { return !noleftclick; }
  virtual int wantRightClicks() { return !norightclick; }
  virtual int wantMouseMoves() { return !nomousemove; }
  virtual int wantContextMenus() { return !nocontextmnu; } 


// DERIVED WINDOW BEHAVIORAL PREFERENCES
protected:
  // return 1 to get onMouseOver even if mouse isn't captured
  virtual int wantSiblingInvalidations();

  virtual int wantFocus();
  virtual int wantAutoContextMenu(); // return 1 if you want to auto popup the main app context menu

protected: 

  void onTipMouseMove();
  void renderBaseTexture(ifc_canvas *c, const RECT &r, int alpha=255);
	void rootwnd_renderBaseTexture(ifc_canvas *c, const RECT *r, int alpha=255) { renderBaseTexture(c, *r, alpha); }
	
  int getTabOrderEntry(ifc_window *w);
  void delTabOrderEntry(int i);
  int getTabOrderEntry(float order);
  void delTabOrderEntry(ifc_window *w);

  virtual OSCURSORHANDLE getCustomCursor(int x, int y);

public:
  virtual ifc_window *getBaseTextureWindow();
  void setBaseTextureWindow(ifc_window *w);
  virtual int isMouseOver(int x, int y);

  virtual void bringVirtualToFront(ifc_window *w);
  virtual void bringVirtualToBack(ifc_window *w);
  virtual void bringVirtualAbove(ifc_window *w, ifc_window *b);
  virtual void bringVirtualBelow(ifc_window *w, ifc_window *b);
  void changeChildZorder(ifc_window *w, int newpos);

//CUT  static int isDesktopAlphaAvailable();
//CUT  static int isTransparencyAvailable();
  virtual int handleTransparency(); // return 0 if you use overlay mode to render your stuff
  virtual int runModal();
  virtual int exec() { return runModal(); }
  virtual void endModal(int ret);

  ifc_dependent *rootwnd_getDependencyPtr();
  ifc_dependent *timerclient_getDependencyPtr();

  virtual void signalMinMaxEnforcerChanged(void);
  virtual void onMinMaxEnforcerChanged(void) {}
  virtual void addMinMaxEnforcer(ifc_window *w);
  virtual void removeMinMaxEnforcer(ifc_window *w);
  virtual ifc_window *enumMinMaxEnforcer(int n);
  virtual int getNumMinMaxEnforcers();
  virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);
  virtual int sendAction(ifc_window *target, const wchar_t *action, const wchar_t *param=NULL, int x=0, int y=0, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0);

  virtual void setRenderBaseTexture(int r);
  virtual int getRenderBaseTexture();

  virtual GuiObject *getGuiObject();

  void setAutoResizeAfterInit(int tf) { want_autoresize_after_init = tf; }
  virtual void setAllowDeactivation(int allow) { allow_deactivate = allow; }
  virtual int allowDeactivation();

  int getNumTabs();
  ifc_window *enumTab(int i);
  virtual void onSetRootFocus(ifc_window *w);

  virtual int wantActivation() { return 1; } // return 0 if you don't want activation upon click

  virtual Accessible *getAccessibleObject(int createifnotexists=1);

  virtual int accessibility_getState();

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
  virtual void setAlwaysOnTop(int i);
  virtual int getAlwaysOnTop();
#endif
  virtual void wndwatcher_onDeleteWindow(ifc_window *w);

    virtual void setOSModuleHandle(OSMODULEHANDLE module) { hinstance = module; }

#ifndef WA3COMPATIBILITY
  virtual void setDropTarget(void *dt);
  virtual void *getDropTarget();
#endif

  virtual void pushWindowRect();
  virtual int popWindowRect(RECT *rc=NULL, int applyhow=PWR_DIMENTIONS|PWR_POSITION);

  virtual void maximize(int axis=MAXIMIZE_WIDTH|MAXIMIZE_HEIGHT);
  virtual void restore(int what=RESTORE_X|RESTORE_Y|RESTORE_WIDTH|RESTORE_HEIGHT);
  virtual int getRestoredRect(RECT *r);
  virtual void setRestoredRect(RECT *r); // turns maximized state on automatically
  virtual int forcedOnResize();
  virtual void forcedOnResizeChain(ifc_window *w);

protected:

  void setForeignWnd(int i); // set to 1 if this basewnd was wrapped around an OSWINDOWHANDLE 
                             // this means mainly that the destructor will not destroy the window handle.

protected:
  // ATTENTION: note the capitalization on these -- so as not to mix up with
  // wndProc()'s hWnd
  OSMODULEHANDLE hinstance;
  OSWINDOWHANDLE hwnd;

private:
  ifc_window *parentWnd;

  int inputCaptured;

  void onTab();
//CUT  HWND createWindow(int x, int y, int w, int h, int nochild, HWND parent, HINSTANCE hinstance);
  void recursive_setVirtualTabOrder(ifc_window *w, float a, float lambda=TABORDER_K);
  void recursive_buildTabList(ifc_window *from, PtrList<ifc_window> *list);

  RECT restore_rect;
  int maximized;

protected:

  void dropVirtualCanvas();
  int bufferizeLockedUIMsg(int uMsg, int wParam, int lParam);
  void clearBufferizedUI() { bufferedmsgs.removeAll(); }
  void checkLockedUI();
  int checkModal();
  void hintDestroying() { destroying=TRUE; } // call that in your destructor if you'd otherwise generate virtual calls after your destructor
  virtual int forceTransparencyFlag();

  int dragging;

	Canvas *virtualCanvas;

  void updateWindowRegion();
  int isWindowRegionValid() { return !wndregioninvalid; }
  virtual int wantRedrawOnResize() { return 1; }
  int ensureWindowRegionValid();
  int disable_tooltip_til_recapture;

  virtual int reinit(); // calls reinit(api_window *parWnd, int nochild);
  virtual int reinit(ifc_window *parWnd, int nochild); // calls reinit(OSMODULEHANDLE moduleHandle, OSWINDOWHANDLE parent, int nochild);
  virtual int reinit(OSMODULEHANDLE moduleHandle, OSWINDOWHANDLE parent, int nochild);

  int inonresize;

  virtual void onBeforeReinit() {}
  virtual void onAfterReinit() {} 

	 StringW tip;
private:
  void reparentHWNDChildren();
  void redrawHWNDChildren();
  void unparentHWNDChildren();
  
#ifdef EXPERIMENTAL_INDEPENDENT_AOT
  void saveTopMosts();
  void restoreTopMosts();
#endif
  void _cascadeRepaintRgn(api_region *rg);
  void packCascadeRepaintRgn(api_region *rg);
  int createTip();       
  void destroyTip();     // called in destructor, do not override
  PtrList<DragSet> dragsets;
  ifc_window *prevtarg;
  StringW suggestedTitle;

  typedef struct {
    int msg;
    int wparam;
    int lparam;
  } bufferedMsgStruct;
  
  TList<bufferedMsgStruct> bufferedmsgs;
  int uiwaslocked;

	void onTip();
  //FG
  int start_hidden;
  svc_toolTipsRenderer *tipsvc;

  int tipid;
  bool tip_done;
  bool tipawaytimer;
  bool tipshowtimer;
  bool tipbeenchecked;
  bool tipdestroytimer;

  //FG
  bool ncb;
  uint32_t lastClick[2];
  POINT lastClickP[2];

  //FG
  ifc_window *btexture;

  //FG
  ifc_window *notifyWindow;
  bool destroying;

	//CT:Virtual children
	PtrList<ifc_window> virtualChildren;
	PtrList<ifc_window> rootwndchildren; //FG
	int virtualCanvasHandled;
	int virtualCanvasH,virtualCanvasW;
	int rwidth, rheight, rx, ry;
	ifc_window *curVirtualChildCaptured;

	//FG>
	RegionI *deferedInvalidRgn;

  OSWINDOWHANDLE oldCapture;
  
	int hasfocus;
	ifc_window *curVirtualChildFocus;

	double ratio;
  int skin_id;
  int wndalpha;
  int w2k_alpha;
  int curframerate;

  int notifyid;
  int activealpha;
  int inactivealpha;
  int clickthrough;
  int triedtipsvc;
  int mustquit;
  int returnvalue;
  int postoninit;
  int inited;
  int skipnextfocus;
  RegionI *subtractorrgn, *composedrgn;
  void setWindowRegion(api_region *r);
  void computeComposedRegion();
  void assignWindowRegion(api_region *wr);
  int wndregioninvalid;
  int rectrgn, regionop;
  RegionI *deferedCascadeRepaintRgn;
  int need_flush_cascaderepaint;
  Tooltip *tooltip;
  PtrList<ifc_window> minmaxEnforcers;
  int this_visible;
  int renderbasetexture;
  GuiObject *my_guiobject;
  int want_autoresize_after_init;
  int resizecount;
  double lastratio;
  int suggested_w, suggested_h;
  int maximum_w, maximum_h;
  int minimum_w, minimum_h;
  int allow_deactivate;
  int minimized;
  int deleting;
  int preventcancelcapture;
  StringW tcname;
  int focus_on_click;
  PtrListQuickSorted<TabOrderEntry, TabOrderSort> childtabs;
  ifc_window *rootfocus;
  int ratiolinked;
  int nodoubleclick, noleftclick, norightclick, nomousemove, nocontextmnu;
  Cursor *customdefaultcursor;
  Accessible *accessible;
  StringW osname;
  int focusEventsEnabled;
  PtrList<ifc_window> ontoplist;
  int alwaysontop;
  WndWatcher rootfocuswatcher;

  int cloaked;
  int m_takenOver;
  int this_enabled;
  int has_alpha_flag;
  RECT *commitfb_rect;
  PtrList<ReparentWndEntry> reparentwnds;
#ifndef WA3COMPATIBILITY
  void *m_target;
#endif
  int lastnullregion;
  Stack<RECT> windowrectstack;
  TList<OSWINDOWHANDLE> ghosthwnd;
  int ghostbust;
  
  OSWINDOWHANDLE lastActiveWnd;

};

#ifdef WIN32
__inline HINSTANCE HINSTANCEfromHWND(HWND wnd) {
  if (wnd == NULL) return NULL;
  return (HINSTANCE)(LONG_PTR)GetWindowLongPtrW(wnd, GWLP_HINSTANCE);
}
#endif

#endif
