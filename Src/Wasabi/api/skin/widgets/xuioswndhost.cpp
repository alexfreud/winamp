#include <precomp.h>
#include "xuioswndhost.h"
#include <api/wnd/notifmsg.h>
#include <bfc/parse/paramparser.h>
#include <tataki/region/region.h>
#include <api/wndmgr/layout.h>
// -----------------------------------------------------------------------
const wchar_t OSWndHostXuiObjectStr[] = L"OSWndHost"; // This is the xml tag
char OSWndHostXuiSvcName[] = "OSWndHost xui object"; 

XMLParamPair XuiOSWndHost::params[] = {
  {XUIOSWNDHOST_SETHWND, L"HWND"},
  {XUIOSWNDHOST_SETOFFSETS, L"OFFSETS"},
	};
// -----------------------------------------------------------------------
XuiOSWndHost::XuiOSWndHost() {
	hosted=false;
	setStartHidden(1);
	setVisible(0);
	visible_start_state=1;
  setVirtual(0); // we are a real window, with an hwnd and shit
  hasregionrect = 0;
  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);

  ScriptObject *o = getGuiObject()->guiobject_getScriptObject();
  o->vcpu_setInterface(osWndHostGuid, static_cast<OSWndHost *>(this));
}

void XuiOSWndHost::CreateXMLParameters(int master_handle)
{
	//XUIOSWNDHOST_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
XuiOSWndHost::~XuiOSWndHost() {
}

void XuiOSWndHost::onSetVisible(int show)
{
	visible_start_state = show;
	XUIOSWNDHOST_PARENT::onSetVisible(show);
}

// -----------------------------------------------------------------------
int XuiOSWndHost::onPaint(Canvas *c) {
  XUIOSWNDHOST_PARENT::onPaint(c); 
  RECT r;
  getClientRect(&r);
  c->fillRect(&r, 0);
  return 1;
}

// -----------------------------------------------------------------------
int XuiOSWndHost::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return XUIOSWNDHOST_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case  XUIOSWNDHOST_SETHWND: {
#ifdef _WIN64
      HWND wnd = (HWND)_wtoi64(value);
#else
			HWND wnd = (HWND)WTOI(value);
#endif
      if (IsWindow(wnd))
        oswndhost_host(wnd);
      break;
    }
    case  XUIOSWNDHOST_SETOFFSETS: {
			ParamParser pp(value, L",");
      RECT r={0,0,0,0};
      if (pp.getNumItems() > 0)
        r.left = WTOI(pp.enumItem(0));
      if (pp.getNumItems() > 1)
        r.top = WTOI(pp.enumItem(1));
      if (pp.getNumItems() > 2)
        r.right = WTOI(pp.enumItem(2));
      if (pp.getNumItems() > 3)
        r.bottom = WTOI(pp.enumItem(3));
      oswndhost_setRegionOffsets(&r);
      break;
    }
    default:
      return 0;
  }
  return 1;
}

// -----------------------------------------------------------------------
void XuiOSWndHost::oswndhost_setRegionOffsets(RECT *r) {
  if (r == NULL) {
    hasregionrect = 0;
    if (wnd != NULL && isPostOnInit()) {
      SetWindowRgn(wnd, NULL, 0);
      onResize();
    }
    return;
  }
  regionrect = *r;
  hasregionrect = 1;
  //if (isPostOnInit())
    //onResize();
}

void XuiOSWndHost::doOnResize()
{
	  if (wnd != NULL) 
		{
    RECT r;
    getClientRect(&r);
    if (renderRatioActive()) 
		{
      //CUT: double ra = getRenderRatio();
      multRatio(&r);
    }

    if (hasregionrect) {
      r.left -= regionrect.left;
      r.top -= regionrect.top;
      r.right += regionrect.right;
      r.bottom += regionrect.bottom;
    }
    SetWindowPos(wnd, NULL, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOZORDER|SWP_NOACTIVATE/*|SWP_NOCOPYBITS|SWP_NOREDRAW*/);
		
    if (hasregionrect) {
      RECT cr={0,0,r.right-r.left,r.bottom-r.top};
      RECT wndr={cr.left+regionrect.left, cr.top+regionrect.top, cr.right-regionrect.right, cr.bottom-regionrect.bottom};
      
      RegionI reg(&wndr);
      SetWindowRgn(wnd, reg.makeWindowRegion(), TRUE);
			//InvalidateRgn(wnd, reg.getOSHandle(), FALSE);
    }
		//else
			//InvalidateRect(wnd, NULL, TRUE);

		//UpdateWindow(wnd);	
		//repaint();
  }
}
int XuiOSWndHost::onAfterResize()
{
	 if (!XUIOSWNDHOST_PARENT::onAfterResize()) return 0;
	 doHost();
	 doOnResize();
	 

	 return 1;
}
// -----------------------------------------------------------------------

#define STYLE_FILTER			(WS_OVERLAPPEDWINDOW | WS_POPUPWINDOW | WS_DLGFRAME | WS_CHILD)
#define EXSTYLE_FILTER		(WS_EX_OVERLAPPEDWINDOW | WS_EX_PALETTEWINDOW | WS_EX_CONTROLPARENT)

void XuiOSWndHost::doHost()
{
	if (wnd && !hosted)
	{
	  onBeforeReparent(1);
		oldparent = GetAncestor(wnd, GA_PARENT);
		/*
		bool blah=false;
		RECT r;
		if (IsWindowVisible(wnd))
		{
		
		GetWindowRect(wnd, &r);
		MapWindowPoints(HWND_DESKTOP, oldparent, (LPPOINT)&r, 2);
		blah=true;
		}
*/
    // remember if WS_POPUP or WS_CHILD was set so we can reset it when we unhost
	
	DWORD style;
	style = GetWindowLongPtrW(wnd, GWL_STYLE);
	savedStyle = (style & STYLE_FILTER);
	SetWindowLongPtrW(wnd, GWL_STYLE, (style & ~(STYLE_FILTER)) | WS_CHILD);
	style = GetWindowLongPtrW(wnd, GWL_EXSTYLE);
	savedExStyle = (style & EXSTYLE_FILTER);
	SetWindowLongPtrW(wnd, GWL_EXSTYLE, (style & ~(EXSTYLE_FILTER)) | WS_EX_CONTROLPARENT);

    GetWindowRect(wnd, &oldrect);
	
    SetParent(wnd, gethWnd());
		//if (blah)
		//InvalidateRect(oldparent, &r, FALSE); 
		//UpdateWindow(oldparent);
	SendMessageW(gethWnd(), 0x0127, MAKEWPARAM(3/*UIS_INITIALIZE*/, 3/*UISF_HIDEACCEL | UISF_HIDEFOCUS*/), 0L);
    onAfterReparent(1);
    dropVirtualCanvas(); // we don't need a canvas anymore, save the memory!
		hosted=true;
		doOnResize();
		setVisible(visible_start_state);
	}
}

// -----------------------------------------------------------------------
void XuiOSWndHost::oswndhost_host(HWND oswnd) 
{
  ASSERT(IsWindow(oswnd));
  wnd = oswnd;
  if (isPostOnInit())
		doHost();
}

// -----------------------------------------------------------------------
void XuiOSWndHost::oswndhost_unhost() {
  if (wnd == NULL) return;
  onBeforeReparent(0);
  
  if (IsWindow(wnd)) {
    // set back the old flags

	DWORD style;
	style = GetWindowLongPtrW(wnd, GWL_STYLE);
	SetWindowLongPtrW(wnd, GWL_STYLE, ((style & ~STYLE_FILTER) | savedStyle));
	style = GetWindowLongPtrW(wnd, GWL_EXSTYLE);
	SetWindowLongPtrW(wnd, GWL_EXSTYLE, ((style & ~EXSTYLE_FILTER) | savedExStyle));
    
    int config_aot = ((GetWindowLong(WASABI_API_WND->main_getRootWnd()->gethWnd(), GWL_EXSTYLE) & WS_EX_TOPMOST) != 0);
    
	SetWindowPos(wnd, config_aot ? HWND_TOPMOST : HWND_NOTOPMOST, 
		oldrect.left, oldrect.top, oldrect.right-oldrect.left, oldrect.bottom-oldrect.top, 
		SWP_NOZORDER | (( 0 == config_aot) ? SWP_NOACTIVATE : 0));

   SetParent(wnd, oldparent);

    if (hasregionrect) SetWindowRgn(wnd, NULL, 0);
//		InvalidateRect(wnd, NULL, TRUE);
		hosted=false;
	
  }

  onAfterReparent(0);

  hasregionrect = 0;
  wnd = NULL;
}

int XuiOSWndHost::onUserMessage(int msg, int w, int l, int *r) {
  switch (msg) {
    case OSWNDHOST_REQUEST_IDEAL_SIZE:
      onDeferredCallback/*postDeferredCallback*/(DCB_OSWNDHOST_REQUEST_IDEAL_SIZE, (intptr_t)(new DCBIdealSize(w, l)));
      *r = 1;
      return 1;
  }
  return XUIOSWNDHOST_PARENT::onUserMessage(msg, w, l, r); // do default handling
}

int XuiOSWndHost::onDeferredCallback(intptr_t p1, intptr_t p2) {
  switch (p1) {
    case DCB_OSWNDHOST_REQUEST_IDEAL_SIZE:
			{
      DCBIdealSize *ideal = (DCBIdealSize *)p2;
      if (ideal == NULL) break;

      ifc_window *p=getDesktopParent(); // Gets the group/layout at the base of the wnd tree, the desktop hwnd 
      if (p == NULL) {
        delete ideal;
        break;
      }
      Layout *l = static_cast<Layout *>(p->getInterface(layoutGuid));
      if (l) 
        l->beginResize();
      RECT r,r2;
      getClientRect(&r); // the size of this wnd
      if (renderRatioActive()) multRatio(&r);

      p->getWindowRect(&r2); // the size of the desktop parent
      
      // take borders into account
      int neww = ideal->m_idealwidth+((r2.right-r2.left)-(r.right-r.left)); 
      int newh = ideal->m_idealheight+((r2.bottom-r2.top)-(r.bottom-r.top));
      
      // take layout min/max values into account
      int min_w = p->getPreferences(MINIMUM_W);
      int min_h = p->getPreferences(MINIMUM_H);
      int max_w = p->getPreferences(MAXIMUM_W);
      int max_h = p->getPreferences(MAXIMUM_H);
      if (min_w != AUTOWH) neww = MAX(min_w, neww);
      if (min_h != AUTOWH) newh = MAX(min_h, newh);
      if (max_w != AUTOWH) neww = MIN(max_w, neww);
      if (max_h != AUTOWH) newh = MIN(max_h, newh);

      //CUT: RECT res={r2.left,r2.top,neww,newh};
      p->resize(r2.left, r2.top, neww, newh); 
      if (l) 
        l->endResize();
      delete ideal;
      return 1;
	}

  }
  return XUIOSWNDHOST_PARENT::onDeferredCallback(p1, p2);
}

void XuiOSWndHost::onBeforeReparent(int i) {
}

void XuiOSWndHost::onAfterReparent(int i) {
}

