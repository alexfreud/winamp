#include "precomp.h"
#include <api/wnd/api_wnd.h>

#include "clickwnd.h"
#include <api/wnd/notifmsg.h>
#include <api/wnd/wndclass/guiobjwnd.h>


enum
{
  CLICKWND_LBUTTONDOWN = 0,
  CLICKWND_LBUTTONUP,
  CLICKWND_RBUTTONDOWN,
  CLICKWND_RBUTTONUP,
};

ClickWnd::ClickWnd() 
{
  handleRight = TRUE;
  button = -1;
  mousedown = 0;
  mcaptured = 0;
  hilite = 0;
  down = 0;
  areacheck = 0;
}

ClickWnd::~ClickWnd() 
{
  BaseWnd::hintDestroying(); // so basewnd doesn't call onCancelCapture
  if (getCapture()) endCapture();
}

void ClickWnd::setHandleRightClick(int tf) 
{
  handleRight=tf;
}

int ClickWnd::getHandleRightClick() 
{
  return handleRight;
}

int ClickWnd::onLeftButtonDown(int x, int y) 
{
  notifyParent(ChildNotify::CLICKWND_LEFTDOWN, x, y);
  CLICKWND_PARENT::onLeftButtonDown(x, y);
  abortTip();
#ifdef _WIN32
  ifc_window *dp = getDesktopParent();
  if (dp != NULL) 
	{
    if (dp->wantActivation()) 
		{
      SetActiveWindow(getRootParent()->gethWnd());
      SetFocus(getRootParent()->gethWnd());
    }
		else 
		{
      HWND w = dp->gethWnd();
      HWND owner = GetWindow(w, GW_OWNER);
      if (owner != NULL) {
        SetActiveWindow(owner);
        SetFocus(owner);
      }
    }
  }
	else 
	{
    SetActiveWindow(getRootParent()->gethWnd()); 
  }
#else
#warning port me or remove me
#endif
  if (ptInRegion(x, y))
    return onButtonDown(CLICKWND_LBUTTONDOWN, x, y);
  else
    return 1;
}

int ClickWnd::onRightButtonDown(int x, int y) 
{
  notifyParent(ChildNotify::CLICKWND_RIGHTDOWN, x, y);
  CLICKWND_PARENT::onRightButtonDown(x, y);
  abortTip();
  if (!handleRight) return 1;
  if (ptInRegion(x, y))
    return onButtonDown(CLICKWND_RBUTTONDOWN, x, y);
  else
    return 1;
}

int ClickWnd::onLeftButtonUp(int x, int y) 
{
  notifyParent(ChildNotify::CLICKWND_LEFTUP, x, y);
  CLICKWND_PARENT::onLeftButtonUp(x, y);
//jf
//  if (ptInRegion())
    return onButtonUp(CLICKWND_LBUTTONUP, x, y);
//  else
//    return 1;
}

int ClickWnd::onRightButtonUp(int x, int y) 
{
  notifyParent(ChildNotify::CLICKWND_RIGHTUP, x, y);
  CLICKWND_PARENT::onRightButtonUp(x, y);
  //jf
  //if (ptInRegion()) 
  if (!handleRight) {
    onRightPush(x, y);
    return 1;
  }
    return onButtonUp(CLICKWND_RBUTTONUP, x, y);
//  else
//    return 1;
}

int ClickWnd::onMouseMove(int x, int y) 
{
  POINT pos, rpos={x,y};
  int mouseover;

  CLICKWND_PARENT::onMouseMove(x, y);

  pos=rpos;
  clientToScreen(&pos);

  int lasthilite = hilite;

  mouseover = (WASABI_API_WND->rootWndFromPoint(&pos) == static_cast<ifc_window *>(this) && ptInRegion(x, y));
  if (!mouseover && (!mousedown
#ifdef _WIN32
                     || !Std::keyDown(button?MK_RBUTTON:MK_LBUTTON)
#else
#warning port me
#endif
                     )) {
    if (mcaptured || getCapture()) {
      endCapture();
      mcaptured = 0;
    }
    mousedown = 0;
    down = 0;
    if (wantClickWndAutoInvalidate()) invalidate();
    if (hilite) _onLeaveArea();
    hilite = 0;
    return 1;
  } else if (!mouseover && hilite) {
    hilite = 0;
    _onLeaveArea();
  } else if (mouseover && !hilite) {
    hilite = 1;
    _onEnterArea();
  }

  if (!getCapture() && mouseover) {	// capture to see when leave
    _enterCapture();
  }

  int lastdown = down;
  hilite = mouseover;
#ifdef WASABI_COMPILE_WNDMGR
  int m = getGuiObject() ? getGuiObject()->guiobject_getMover() : 0;
#else
  int m = 0;
#endif
  if (!m) {
    down = userDown() || (mouseover && mousedown);
  } else 
    down = userDown() || mousedown;
  
  // FG> note to self now that i finally fixed this... :
  // there is a potential bottleneck here, if for some reason this test is always true when moving the windows around like crazy.
  if (down != lastdown || (hilite != lasthilite && !m)) {
    if (wantClickWndAutoInvalidate()) invalidate(); 
  }

//invalidate();
  return 1;
}

void ClickWnd::_enterCapture() 
{
  //gee!! if (!hilite) _onEnterArea();
  if (!getCapture()) beginCapture();     
  mcaptured = 1;
}

int ClickWnd::onButtonDown(int which, int x, int y) 
{
  if (!wantClicks()) return 1;

  if (!getCapture()) {
    _enterCapture();
  }
  mousedown = 1;
  down = 1;
  button = -1;
  if (which == CLICKWND_LBUTTONDOWN) button = 0;
  else if (which == CLICKWND_RBUTTONDOWN) button = 1;
  if (wantClickWndAutoInvalidate()) invalidate();

  return 1;
}

int ClickWnd::onButtonUp(int which, int x, int y) 
{
    // make sure same button
  if (button == 0 && which == CLICKWND_RBUTTONUP) return 1;
  if (button == 1 && which == CLICKWND_LBUTTONUP) return 1;

  if (!down) {
    if (mcaptured) {
      endCapture();
      mcaptured = 0;
    }
    if (hilite) _onLeaveArea();
    hilite = 0; 
    mousedown = 0;
    return 1;
  }

    POINT pos={x,y};
    clientToScreen(&pos);

    int mouseover = (WASABI_API_WND->rootWndFromPoint(&pos) == (ifc_window *)this && ptInRegion(x, y));
    if (!mouseover) {
      if (mcaptured) {
        endCapture();
        mcaptured = 0;
      }
      if (hilite) _onLeaveArea();
      hilite = 0;
    }

    // it was down, process the event
    int a = down;
    down = 0;
    mousedown = 0;
    if (wantClickWndAutoInvalidate()) invalidate();

    if (a) {
      if (button == 0) onLeftPush(x, y);
      else if (button == 1) onRightPush(x, y);
    } 
    
    // we need to do this again (and get the new mouse pos) because onLeft/RightPush may have called a 
    // message loop and let the mouse leave without us being aware of it
    Wasabi::Std::getMousePos(&x, &y);
    pos.x = x;
    pos.y = y;
    screenToClient(&x, &y);
    mouseover = (WASABI_API_WND->rootWndFromPoint(&pos) == (ifc_window *)this && ptInRegion(x, y));
    if (!mouseover && hilite) _onLeaveArea();
    else if (mouseover && !hilite) _onEnterArea();
    hilite = mouseover;
    
    return 1;
}

void ClickWnd::onSetVisible( int show )
{
    CLICKWND_PARENT::onSetVisible( show );
    if ( !show )
    {
        if ( getCapture() )
        {
            mcaptured = 0;
            endCapture();
        }
        down = 0;
        mousedown = 0;

        if ( hilite )
            _onLeaveArea();

        hilite = 0;
    }
}

void ClickWnd::_onEnterArea() 
{
  if (areacheck == 0) {
    areacheck++;
    onEnterArea();
  } else 
    DebugString("onEnterArea check failed %08X \n", this);
}

void ClickWnd::_onLeaveArea() 
{
  if (areacheck == 1) {
    areacheck--;
    onLeaveArea();
  } else 
    DebugString("onLeaveArea check failed %08X\n", this);
}

void ClickWnd::onEnterArea() 
{
//  DebugString("onEnterArea %08X\n", this);
}

void ClickWnd::onLeaveArea() 
{
//  DebugString("onLeaveArea %08X\n", this);
}

void ClickWnd::onCancelCapture() 
{
  CLICKWND_PARENT::onCancelCapture();
  mcaptured=0;
  down = 0;
  mousedown = 0;
  if (hilite) _onLeaveArea(); 
  hilite = 0; 
}

