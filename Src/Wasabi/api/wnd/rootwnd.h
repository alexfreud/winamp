#ifndef _ROOTWND_H
#define _ROOTWND_H

#include "api_window.h"
#include <bfc/wasabi_std.h>
#include <bfc/dispatch.h>
#include <api/wnd/cursor.h>

class ifc_canvas;
class api_region;
class TimerClient;
class ifc_dependent;
class GuiObject;
class FindObjectCallback;
class Cursor;
class Accessible;
class Canvas;

// only these methods are safe across dll boundaries
// this is the pointer you find in the GWL_USERDATA of a window

class DragInterface;

class NOVTABLE RootWndI : public ifc_window 
{
protected:
  RootWndI() {}	// protect constructor

public:
  virtual OSWINDOWHANDLE getOsWindowHandle()=0;
  virtual OSMODULEHANDLE getOsModuleHandle()=0;
  virtual void performBatchProcesses()=0;
  virtual TimerClient *getTimerClient()=0;
  virtual const wchar_t *getRootWndName()=0;
  virtual const wchar_t *getId()=0;
  virtual int init(ifc_window *parent, int nochild)=0;
  virtual int isInited()=0;
  virtual int isPostOnInit()=0;

  virtual int setVirtual(int i)=0;

  virtual int isClickThrough()=0;
  virtual int onMouseMove(int x, int y)=0;
  virtual int onLeftButtonUp(int x, int y)=0;
  virtual int onRightButtonUp(int x, int y)=0;
  virtual int onLeftButtonDown(int x, int y)=0;  
  virtual int onRightButtonDown(int x, int y)=0;
  virtual int onLeftButtonDblClk(int x, int y)=0;
  virtual int onRightButtonDblClk(int x, int y)=0;

  virtual DragInterface *getDragInterface()=0;

  virtual int getCursorType(int x, int y)=0;
  virtual OSCURSORHANDLE getCustomCursor(int x, int y)=0;
  virtual ifc_window *rootWndFromPoint(POINT *pt)=0;

  virtual void getClientRect(RECT *r)=0;
  virtual void getNonClientRect(RECT *rect)=0;
  virtual void getWindowRect(RECT *r)=0;

  virtual void setVisible(int show)=0;
  virtual void setCloaked(int cloak)=0;
  virtual void onSetVisible(int show)=0;
  virtual int isVisible(int within=0)=0;

  virtual void *getInterface(GUID interface_guid)=0;

  virtual void invalidate()=0;
  virtual void invalidateRect(RECT *r)=0;
  virtual void invalidateRgn(api_region *r)=0;
  virtual void invalidateFrom(ifc_window *who)=0;
  virtual void invalidateRectFrom(RECT *r, ifc_window *who)=0;
  virtual void invalidateRgnFrom(api_region *r, ifc_window *who)=0;
  virtual void validate()=0;
  virtual void validateRect(RECT *r)=0;
  virtual void validateRgn(api_region *reg)=0;
  virtual void onChildInvalidate(api_region *r, ifc_window *who)=0;

  virtual int rootwnd_paintTree(ifc_canvas *canvas, api_region *r)=0;
  virtual int paint (Canvas *canvas, api_region *r)=0;
  virtual Canvas *getFrameBuffer()=0;

  virtual ifc_window *getParent()=0;
  virtual ifc_window *getRootParent()=0;
  virtual ifc_window *getDesktopParent()=0;
  virtual void setParent(ifc_window *parent)=0;

  virtual int onSiblingInvalidateRgn(api_region *r, ifc_window *who, int who_idx, int my_idx)=0;
  virtual int wantSiblingInvalidations()=0;

  virtual int cascadeRepaintFrom(ifc_window *who, int pack=1)=0;
  virtual int cascadeRepaintRgnFrom(api_region *reg, ifc_window *who, int pack=1)=0;
  virtual int cascadeRepaintRectFrom(RECT *r, ifc_window *who, int pack=1)=0;
  virtual int cascadeRepaint(int pack=1)=0;
  virtual int cascadeRepaintRgn(api_region *reg, int pack=1)=0;
  virtual int cascadeRepaintRect(RECT *r, int pack=1)=0;
  virtual void repaint()=0;

  virtual void setClickThrough(int ct)=0;
  virtual ifc_window *getBaseTextureWindow()=0;
  virtual void rootwnd_renderBaseTexture(ifc_canvas *c, const RECT *r, int alpha=255)=0;
#if defined (_WIN64)
  virtual int childNotify(ifc_window* child, int msg, int p1, int p2) = 0;
#else
  virtual int childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2)=0;
#endif
  virtual int getPreferences(int what)=0;
  virtual void setPreferences(int what, int v)=0;

  virtual api_region *getRegion()=0;
  virtual int getRegionOp()=0;
  virtual void setRegionOp(int op)=0;
  virtual int isRectRgn()=0;
  virtual void setRectRgn(int rrgn)=0;
  virtual void invalidateWindowRegion()=0;
  virtual api_region *getComposedRegion()=0;
  virtual api_region *getSubtractorRegion()=0;

  virtual void setStartHidden(int sh)=0;
  virtual double getRenderRatio()=0;
  virtual void setRenderRatio(double r)=0;
  virtual void setRatioLinked(int l)=0;
  virtual int handleRatio()=0;
  virtual void resize(int x, int y, int w, int h, int wantcb=1)=0;
  virtual void move(int x, int y)=0;
  virtual void notifyDeferredMove(int x, int y)=0;

  virtual void getPosition(POINT *pt)=0;
  virtual ifc_window *getForwardWnd()=0;

  virtual void registerRootWndChild(ifc_window *child)=0;
  virtual void unregisterRootWndChild(ifc_window *child)=0;
  virtual ifc_window *findRootWndChild(int x, int y, int only_virtuals=0)=0;
  virtual ifc_window *enumRootWndChildren(int _enum)=0;
  virtual int getNumRootWndChildren()=0;

  virtual int isVirtual()=0;

  virtual void bringVirtualToFront(ifc_window *w)=0;
  virtual void bringVirtualToBack(ifc_window *w)=0;
  virtual void bringVirtualAbove(ifc_window *w, ifc_window *b)=0;
  virtual void bringVirtualBelow(ifc_window *w, ifc_window *b)=0;
  virtual int checkDoubleClick(int button, int x, int y)=0;
  
  virtual void onCancelCapture()=0;
  virtual void cancelCapture()=0;

  virtual void clientToScreen(int *x, int *y)=0; 
  virtual void screenToClient(int *x, int *y)=0; 

  virtual void setVirtualChildCapture(ifc_window *child)=0;
  virtual ifc_window *getVirtualChildCapture()=0;

  virtual int ptInRegion(int x, int y)=0;

  virtual int onActivate()=0;
  virtual void activate()=0;
  virtual int onDeactivate()=0;
  virtual int isActive()=0;
  virtual int handleTransparency()=0;
  virtual int handleDesktopAlpha()=0;

  virtual int getNotifyId()=0;
  virtual void setEnabled(int en)=0;
  virtual int isEnabled(int within=0)=0;
  virtual int onEnable(int e)=0;

  virtual int getPaintingAlpha()=0; 
  virtual void getAlpha(int *active=NULL, int *inactive=NULL)=0; 
  virtual void setAlpha(int activealpha, int inactivealpha=-1)=0; 

  virtual void setTip(const wchar_t *tip)=0;
  virtual int runModal()=0;
  virtual void endModal(int retcode)=0;
  virtual int wantAutoContextMenu()=0;
  virtual int wantActivation()=0;

  virtual void bringToFront()=0;
  virtual void bringToBack()=0;
  virtual void setFocus()=0;
  virtual int gotFocus()=0;
  virtual ifc_window *getNextVirtualFocus(ifc_window *w)=0;
  virtual void setVirtualChildFocus(ifc_window *w)=0;
  virtual void setVirtualTabOrder(ifc_window *w, int a)=0;
  virtual int getVirtualTabOrder(ifc_window *w)=0;
  virtual int wantFocus()=0;
  virtual void setTabOrder(int a)=0;
  virtual int getTabOrder()=0;
  virtual ifc_window *getTab(int what=TAB_GETCURRENT)=0;
  virtual void setAutoTabOrder()=0;
  virtual void setVirtualAutoTabOrder(ifc_window *w)=0;
  virtual ifc_window *getCurVirtualChildFocus()=0;
  virtual void onSetRootFocus(ifc_window *w)=0;

  virtual int onAcceleratorEvent(const wchar_t *name)=0;

  virtual int onChar(unsigned int c)=0;
  virtual int onKeyDown(int keycode)=0;
  virtual int onKeyUp(int keycode)=0;
  virtual int onSysKeyDown(int keyCode, int keyData)=0;
  virtual int onSysKeyUp(int keyCode, int keyData)=0;
  virtual int onKillFocus()=0;
  virtual int onGetFocus()=0;
  virtual ifc_dependent *rootwnd_getDependencyPtr()=0;
  virtual void addMinMaxEnforcer(ifc_window *w)=0;
  virtual void removeMinMaxEnforcer(ifc_window *w)=0;
  virtual ifc_window *enumMinMaxEnforcer(int n)=0;
  virtual int getNumMinMaxEnforcers()=0;
  virtual void signalMinMaxEnforcerChanged()=0;

  virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL)=0;

  virtual void setRenderBaseTexture(int r)=0;
  virtual int getRenderBaseTexture()=0;
  virtual GuiObject *getGuiObject()=0; // not guaranteed non null

  virtual int getFlag(int flag)=0;
  virtual int triggerEvent(int event, intptr_t p1, intptr_t p2)=0;


  virtual int allowDeactivation()=0;
  virtual void setAllowDeactivation(int allow)=0;

  virtual ifc_window *findWindow(const wchar_t *id)=0;
  virtual ifc_window *findWindowByInterface(GUID interface_guid)=0;
  virtual ifc_window *findWindowByCallback(FindObjectCallback *cb)=0;
  virtual ifc_window *findWindowChain(FindObjectCallback *cb, ifc_window *wcaller)=0;

  virtual void focusNext()=0;
  virtual void focusPrevious()=0;

  virtual void setWindowTitle(const wchar_t *title) = 0;
  virtual ifc_window *enumTab(int i)=0;
  virtual int getNumTabs()=0;

  virtual int getFocusOnClick()=0;
  virtual void setFocusOnClick(int i)=0;

  virtual void setNoDoubleClicks(int no)=0;
  virtual void setNoLeftClicks(int no)=0;
  virtual void setNoRightClicks(int no)=0;
  virtual void setNoMouseMoves(int no)=0;
  virtual void setNoContextMenus(int no)=0;

  virtual int wantDoubleClicks()=0;
  virtual int wantLeftClicks()=0;
  virtual int wantRightClicks()=0;
  virtual int wantMouseMoves()=0;
  virtual int wantContextMenus()=0;

  virtual void setDefaultCursor(Cursor *c)=0;

  virtual Accessible *getAccessibleObject(int createifnotexist=1)=0;
  virtual int accessibility_getState()=0;

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
  virtual void setAlwaysOnTop(int i)=0;
  virtual int getAlwaysOnTop()=0;
#endif

#ifndef WA3COMPATIBILITY
  virtual void setDropTarget(void *dt)=0;
  virtual void *getDropTarget()=0;
#endif

  virtual int isMinimized()=0;
  virtual void maximize(int axis=MAXIMIZE_WIDTH|MAXIMIZE_HEIGHT)=0;
  virtual void restore(int what=RESTORE_X|RESTORE_Y|RESTORE_WIDTH|RESTORE_HEIGHT)=0;
  virtual int getRestoredRect(RECT *r)=0;

protected:
  RECVS_DISPATCH;
};

#endif
