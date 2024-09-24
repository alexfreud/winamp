#ifndef __LAYOUT_H
#define __LAYOUT_H

class Layout;
class Group;
class Container;
class Layer;

#include <bfc/tlist.h>
#include <bfc/depend.h>
#include <tataki/bitmap/bitmap.h>
#include <api/wnd/wndclass/buttwnd.h>
#include <tataki/region/region.h>
#include <api/wndmgr/container.h>
#include <api/skin/group.h>
#include <api/skin/widgets/layer.h>
#include <api/wndmgr/guistatuscb.h>
#include <api/script/script.h>
#include <api/script/scriptobj.h>
#ifdef WASABI_WIDGETS_GUIOBJECT
#include <api/script/objects/guiobj.h>
#endif
#include <api/wnd/accessible.h>
#include <api/wndmgr/alphamgr.h>
#include <api/wnd/resizable.h>

class XmlObject;
class Layout;

extern AlphaMgr *alphaMgr;

class LayoutScriptController : public GroupScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return groupController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual int getInstantiable();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern LayoutScriptController *layoutController;

#ifndef _NOSTUDIO

class AutoOpacityLinker;

#define LAYOUT_PARENT Group
#define LAYOUT_SCRIPTPARENT Group

class Layout : public LAYOUT_SCRIPTPARENT, public DependentViewerI, public GuiResizable 
{ 

public:
	Layout();
	virtual ~Layout();

#ifdef _WIN32
  virtual LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
  virtual OSStatus eventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData);
#endif

//	int onPaint(Canvas *canvas);
	virtual int onInit();
  virtual int init(OSMODULEHANDLE inst, OSWINDOWHANDLE parent, int nochild);
  virtual int reinit(OSMODULEHANDLE inst, OSWINDOWHANDLE parent, int nochild);

  virtual int childNotify(ifc_window *child, int msg, intptr_t param1=0, intptr_t param2=0);
  virtual int onActionNotify(int action, intptr_t param=0);
  
  virtual void resize(int x, int y, int w, int h, int wantcb=1);
  virtual void move(int x, int y);

//  virtual int onRightButtonDown(int x, int y);
//  virtual int onLeftButtonDown(int x, int y);
//  virtual int onMouseMove(int x, int y);	
//  virtual int onLeftButtonUp(int x, int y);
  virtual int onResize();
  virtual int onPostedMove();
  virtual int onPaint(Canvas *canvas);
  virtual void onSetDesktopAlpha(int a);
  virtual int isLayout();
  virtual void setAlphaBackground(const wchar_t *txture);
  virtual SkinBitmap *getBaseTexture();
  virtual void onGuiObjectSetVisible(GuiObject *o, int status); // called whenever a child shows/hide
  virtual ifc_window *getCustomOwner();
  virtual void addLockedLayout(Layout *l);
  virtual void removeLockedLayout(Layout *l);
  virtual int getNumLockedLayouts();
  virtual Layout *enumLockedLayout(int n);
  virtual int isLocked();
  virtual void lockTo(Layout *l);
  virtual Layout *getLockedTo();
  void updateLockedLayouts();
  virtual int onGetFocus();
  virtual int onKillFocus();
  virtual void snapAdjust(int left, int right, int top, int bottom);

  virtual void onShow(void);
  virtual void onHide(void);

  virtual void center();

  virtual int wantDesktopAlpha();
  virtual void setWantDesktopAlpha(int want);
  virtual int handleDesktopAlpha();

  virtual int setXuiParam(int _xuihandle, int attribid, const wchar_t *paramname, const wchar_t *strvalue);

  void setWindowRegion(api_region *reg);
  virtual int allowResize() { 
    return !isLocked()
#ifdef USEAPPBAR
           && !appbar_isDocked() 
#endif
           ;
  }

  // container/component callbacks to get notified that a container 
  // has been set visible/invisible
	void containerToggled(const wchar_t *id,int visible);	
  void componentToggled(GUID *guid, int visible);

  void setParentContainer(Container *c);
  virtual Container *getParentContainer();

  virtual int isClickThrough();

  void onSetVisible(int show);
  virtual void cancelCapture();

  virtual int onActivate();
  virtual int onDeactivate();

  virtual int forceTransparencyFlag();

	int x, y;

#ifdef _WIN32
  void setForwardMsgWnd(HWND wnd) { forwardMsgWnd = wnd; }

  LPARAM wndHolder_getParentParam(int i=0);
#endif

  void scaleTo(int s);
  virtual void setRenderRatio(double s);

  virtual void beginMove();
  virtual void beginScale();
  virtual void beginResize();
  virtual void endMove();
  virtual void endScale();
  virtual void endResize();

  virtual void setEndMoveResize(int w, int h) {
    m_w = w;
    m_h = h;
    m_endmovesize = 1;
  };

  virtual ifc_window *guiresizable_getRootWnd() { return (this); }
  
  
  virtual void lockScale(int locked);
  virtual int isScaleLocked() { return scalelocked; }

  virtual void onMove();

  virtual int isDesktopAlphaSafe();

  void addSubRegionLayer(Layer *l);
  void removeSubRegionLayer(Layer *l);

  virtual void setInDesktop(int a);
  virtual int getInDesktop();

  virtual void setAlpha(int a);
  virtual int getAlpha();
  virtual int getPaintingAlpha();
  virtual void timerCallback(int id);

  virtual void setLinkWidth(const wchar_t *layoutid);
  virtual void setLinkHeight(const wchar_t *layoutid);

  virtual void setBaseTexture(const wchar_t *b, int regis=1);
  virtual void setPaintingAlpha(int activealpha, int inactivealpha=-1);

  static void onGlobalEnableDesktopAlpha(int enabled);

  void savePosition();
#ifdef USEAPPBAR
  void saveAppBarPosition();
#endif
  virtual void setStatusText(const wchar_t *txt, int overlay=0);
  virtual void addAppCmds(AppCmds *commands);
  virtual void removeAppCmds(AppCmds *commands);

  void pushCompleted(int max=100);
  void incCompleted(int add=1);
  void setCompleted(int pos);
  void popCompleted();

  virtual void registerStatusCallback(GuiStatusCallback *lcb);
  virtual int viewer_onItemDeleted(api_dependent *item);
  virtual int wantActivation() { return wantactiv && LAYOUT_PARENT::wantActivation(); }
  void loadSavedState();
  virtual void updateOnTop();
  
  virtual int runAction(int actionid, const wchar_t *param=NULL);

  virtual void getSnapAdjust(RECT *r);

  virtual void updateTransparency();
  virtual int onDeferredCallback(intptr_t p1, intptr_t p2);
  virtual int wantRedrawOnResize() { return wantredrawonresize; }
  virtual void setWantRedrawOnResize(int v);

#ifdef USEAPPBAR
  virtual int appbar_wantAutoHide() { return getAppBarAutoHide(); }
  virtual int appbar_wantAlwaysOnTop() { return getAppBarAlwaysOnTop(); }
  
  virtual int getAppBarAutoHide();
  virtual void setAppBarAutoHide(int ah);
  
  virtual int getAppBarAlwaysOnTop();
  virtual void setAppBarAlwaysOnTop(int aot);
#endif

  virtual void pushForceUnlink() { m_forceunlink++; }
  virtual void popForceUnlink() { m_forceunlink--; }

  virtual int isUnlinked() { 
#ifdef USEAPPBAR
      return unlinked || appbar_isDocked() || m_forceunlink;
#else
      return unlinked || m_forceunlink;
#endif
  }

  void setAutoOpacify(int a);
  int getAutoOpacify() { return autoopacify; }

  void offscreenCheck();
  int isOffscreen(ifc_window *w);

  int getResizable();
  int getScalable();

  void setTransparencyOverride(int v);
  int getTransparencyOverride() { return transparencyoverride; }

  enum {
    LAYOUT_SETDESKTOPALPHA=0,
    LAYOUT_SETINDESKTOP,
    LAYOUT_SETALPHA,
    LAYOUT_SETLINKWIDTH,
    LAYOUT_SETLINKHEIGHT,
    LAYOUT_SETOWNER,
    LAYOUT_SETLOCKTO,
    LAYOUT_SETOSFRAME,
    LAYOUT_SETALPHABACKGROUND,
    LAYOUT_SETNOACTIVATION,
    LAYOUT_SETONTOP,
    LAYOUT_SNAPADJUSTLEFT,
    LAYOUT_SNAPADJUSTTOP,
    LAYOUT_SNAPADJUSTRIGHT,
    LAYOUT_SNAPADJUSTBOTTOM,
    LAYOUT_UNLINKED,
    LAYOUT_NOPARENT,
    LAYOUT_FORCEALPHA,
    LAYOUT_NODOCK,
    LAYOUT_NOOFFSCREENCHECK,
	LAYOUT_RESIZABLE,
	LAYOUT_SCALABLE,
  };

	
  void onMouseEnterLayout();
  void onMouseLeaveLayout();

  int getNoParent() { return noparent; }
  void setNoParent(int np) { noparent = np; }
  int isAlphaForced() { return forcealpha; }

  AlphaMgr *getAlphaMgr() { return alphaMgr; }

  int getNoDock() { return nodock; }
  void setNoDock(int nd) { nodock = nd; }
  int isTransparencyForcedOff() { return transparency_autooff; }

  void controlMenu();

  void setNoOffscreenCheck(int nocheck);
  
#ifdef USEAPPBAR
  void onDock(int side);
  void onUnDock();

  virtual void appbar_onDock(int side);
  virtual void appbar_onUnDock();
  virtual void appbar_onSlide();
#endif

protected:
	/*static */void CreateXMLParameters(int master_handle);
/*  virtual int dragEnter(ifc_window *sourceWnd);
  virtual int dragOver(int x, int y, ifc_window *sourceWnd);
  virtual int dragDrop(ifc_window *sourceWnd, int x, int y);
  virtual int acceptExternalDrops() { return 1; }*/
  virtual int wantClickWndAutoInvalidate() { return 0; }


private:
	StringW MakePrefix();
	static XMLParamPair params[];
  void fixPosition();
	void saveAllPositions();
	void activateChildren(int act);
#ifdef _WIN32
  void getExplorerWindows(HWND *parent, HWND *listview, HWND *webserver);
#endif
  void desktopAlpha_autoTurnOn();
  void desktopAlpha_autoTurnOff();
  void transparency_autoTurnOn();
  void transparency_autoTurnOff();
  void globalEnableDesktopAlpha(int enabled);

#ifdef _WIN32
	HWND forwardMsgWnd;
#endif
	int resizing;
  int wantactiv;
	int size_w,size_h;
	int cX,cY;
  int captured;
  POINT mousepos;
#ifdef _WIN32
  HWND webserver;
  HWND listview; 
#endif
  int alphagoingon;
  int alphagoingoff;
  int scalelocked;
  int wantredrawonresize;

  int xuihandle;

  RegionI *reg;
  //PtrList<Layer> *subregionlayers;
  Container *p_container;
  StringW alphabackgroundstr;
  ifc_window *wndholders;
  int abortSaving();
  int transparencyoverride;

  int default_x;
  int default_y;
  int moving;
  int scaling;
  int mover;
  int indesktop;
  int alpha;
  StringW linkedheight, linkedwidth;
  int inlinkwidth, inlinkheight;
  AutoSkinBitmap alphabackground;
  int wantdesktopalpha;
  int galphadisabled;
  static PtrList<Layout> alllayouts;
  StringW owner;
  PtrList<Layout> locked;
  StringW lockto;
  Layout *lockedto;
  int inpostedmove;
  int osframe;
  PtrList<GuiStatusCallback> statuscbs;
  int initontop;
//  GarbageCollector gc;
  PtrList<AppCmds> appcmds;
  int inresize;
  int unlinked;

  int snap_adjust_left;
  int snap_adjust_top;
  int snap_adjust_right;
  int snap_adjust_bottom;

  int disable_auto_alpha;
  int autoopacify;
  int noparent;
  int forcealpha;
  redock_struct redock;
  static int broadcasting;      
  int nodock;
  uint32_t transparency_reenabled_at;
  int transparency_autooff;
  int nooffscreencheck;
  int resizable;
  int scalable;

  int m_w, m_h;
  int m_endmovesize;
  int m_allowsavedock;
  int m_forceunlink;
#ifdef USEAPPBAR
  int appbar_want_autohide;
  int appbar_want_alwaysontop;
#endif

// FG>
// -- SCRIPT -----------------------------------------------------
public:

  static scriptVar script_vcpu_onDock(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o, scriptVar side);
  static scriptVar script_vcpu_onUndock(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o);
  static scriptVar script_vcpu_getScale(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o);
  static scriptVar script_vcpu_setScale(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o, scriptVar s);
  static scriptVar script_vcpu_onScale(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o, scriptVar s);
  static scriptVar script_vcpu_setDesktopAlpha(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o, scriptVar s);
  static scriptVar script_vcpu_getDesktopAlpha(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o);
  static scriptVar script_vcpu_isTransparencySafe(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o);
  static scriptVar script_vcpu_isLayoutAnimationSafe(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o);
  static scriptVar script_vcpu_getContainer(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o);
  static scriptVar script_vcpu_center(SCRIPT_FUNCTION_PARAMS,  ScriptObject *o);
  static scriptVar script_vcpu_onMove(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onEndMove(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onUserResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y, scriptVar w, scriptVar h);
  static scriptVar script_vcpu_snapAdjust(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar left, scriptVar top, scriptVar right, scriptVar bottom);
  static scriptVar script_vcpu_setRedrawOnResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
  static scriptVar script_vcpu_beforeRedock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_redock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getSnapAdjustTop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getSnapAdjustLeft(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getSnapAdjustRight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getSnapAdjustBottom(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onMouseEnterLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onMouseLeaveLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onSnapAdjustChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

#else
class Layout : public LAYOUT_SCRIPTPARENT {

public:

#endif

//  INSERT_SCRIPT_OBJECT_CONTROL


};

// END SCRIPT

#endif
