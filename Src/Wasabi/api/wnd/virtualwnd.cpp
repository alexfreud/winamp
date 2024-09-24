#include <precomp.h>
#include "virtualwnd.h"
#include <tataki/region/api_region.h>

#include <api/wnd/usermsg.h>
#include <api/wnd/accessible.h>

VirtualWnd::VirtualWnd()
{
	virtualX = virtualY = virtualH = virtualW = 0;
	bypassvirtual = 0;
	focus = 0;
	resizecount = 0;
	lastratio = 1;
}

VirtualWnd::~VirtualWnd()
{}

int VirtualWnd::init(ifc_window *parWnd, int nochild)
{
	if (!bypassvirtual)
		setParent(parWnd);

	return (VIRTUALWND_PARENT::init(parWnd, nochild));
}

int VirtualWnd::init(OSMODULEHANDLE moduleHandle, OSWINDOWHANDLE parent, int nochild)
{
	if (!bypassvirtual)
	{
		ASSERTPR(getParent() != NULL, "Virtual window created without specifying BaseWnd parent");

		if (getStartHidden())
			this_visible = 0;
		else
			this_visible = 1;

		onInit();

		onPostOnInit();

		if (isVisible())
			onSetVisible(1);

		return 1;
	}
	else
		return VIRTUALWND_PARENT::init(moduleHandle, parent, nochild);
}

OSWINDOWHANDLE VirtualWnd::getOsWindowHandle()
{
	//	ASSERTPR(getParent() != NULL, "Virtual window used as base parent !");
	if (!bypassvirtual)
	{
		if (!getParent())
			return INVALIDOSWINDOWHANDLE;

		return getParent()->getOsWindowHandle();
	}
	else
	{
		return VIRTUALWND_PARENT::getOsWindowHandle();
	}
}

OSMODULEHANDLE VirtualWnd::getOsModuleHandle()
{
	if (!bypassvirtual)
	{
		if (!getParent())
			return INVALIDOSMODULEHANDLE;

		return getParent()->getOsModuleHandle();
	}
	else
		return VIRTUALWND_PARENT::getOsModuleHandle();
}

void VirtualWnd::resize(RECT *r, int wantcb)
{
	if (!bypassvirtual)
	{
		resize(r->left, r->top, r->right - r->left, r->bottom - r->top, wantcb);
	}
	else
	{
		VIRTUALWND_PARENT::resize( r, wantcb );

		//virtualX = rx;
		//virtualY = ry;
		//virtualW = rwidth;
		//virtualH = rheight;
	}
}

// fg> the resizecount > 1 is a hack, it should be 0 but i need more time to fix this thing, at least this way we don't lose the optim
void VirtualWnd::resize(int x, int y, int w, int h, int wantcb)
{
	if (!bypassvirtual)
	{
		if (x == NOCHANGE)
			x = virtualX;

		if (y == NOCHANGE)
			y = virtualY;

		if (w == NOCHANGE)
			w = virtualW;

		if (h == NOCHANGE)
			h = virtualH;

		double thisratio = getRenderRatio();

		if (resizecount > 1 && virtualX == x && virtualY == y && virtualW == w && virtualH == h && lastratio == thisratio)
			return ;

		lastratio = thisratio;

		if (isVisible())
		{
			RECT r;
			getNonClientRect(&r);
			invalidateRect(&r);
		}

		virtualX = x;
		virtualY = y;
		virtualW = w;
		virtualH = h;

		if (isVisible())
		{
			RECT r;
			getNonClientRect(&r);
			invalidateRect(&r);
		}

		setRSize(x, y, w, h);

		if (wantcb && isPostOnInit())
		{
			resizecount = MIN(resizecount + 1, 2);
			onResize();
		}
	}
	else
	{
		VIRTUALWND_PARENT::resize( x, y, w, h, wantcb );

		//virtualX = rx;
		//virtualY = ry;
		//virtualW = rwidth;
		//virtualH = rheight;
	}
}

//CUTvoid VirtualWnd::resize(RECT *r) {
//CUT  resize(r->left, r->top, r->right-r->left, r->bottom-r->top);
//CUT}

void VirtualWnd::move(int x, int y)
{
	//DebugStringW( L"VirtualWnd::move( x = %d, y = %d )\n", x, y );

	if (!bypassvirtual)
	{
		if (isVisible())
		{
			RECT r;
			getNonClientRect(&r);
			invalidateRect(&r);
		}

		virtualX = x;
		virtualY = y;

		if (isVisible())
		{
			RECT r;
			getNonClientRect(&r);
			invalidateRect(&r);
		}
	}
	else
	{
		VIRTUALWND_PARENT::move( x, y );

		//virtualX = x;
		//virtualY = y;
	}
}

void VirtualWnd::invalidate()
{
	if (!bypassvirtual)
	{
		if (!getRootParent()) return ;
		RECT r(clientRect());
		getRootParent()->invalidateRectFrom(&r, this);
		//	VIRTUALWND_PARENT::invalidate();
	}
	else
		VIRTUALWND_PARENT::invalidate();
}

void VirtualWnd::invalidateRect(RECT *r)
{
	if (!bypassvirtual)
	{
		if (!getRootParent()) return ;
		getRootParent()->invalidateRectFrom(r, this);
	}
	else
		VIRTUALWND_PARENT::invalidateRect(r);
}

void VirtualWnd::invalidateRgn(api_region *reg)
{
	if (!bypassvirtual)
	{
		if (!getRootParent()) return ;
		getRootParent()->invalidateRgnFrom(reg, this);
	}
	else
		VIRTUALWND_PARENT::invalidateRgn(reg);
}

void VirtualWnd::validate()
{
	if (!bypassvirtual)
	{
		if (!getRootParent()) return ;
		RECT r;
		getClientRect(&r);
		getRootParent()->validateRect(&r);
	}
	else
		VIRTUALWND_PARENT::validate();
}

void VirtualWnd::validateRect(RECT *r)
{
	if (!bypassvirtual)
	{
		if (!getRootParent())
			return ;

		getRootParent()->validateRect(r);
	}
	else
		VIRTUALWND_PARENT::validateRect( r );
}

void VirtualWnd::validateRgn(api_region *reg)
{
	if (!bypassvirtual)
	{
		if (!getRootParent()) return ;
		getRootParent()->validateRgn(reg);
	}
	else
		VIRTUALWND_PARENT::validateRgn(reg);
}

void VirtualWnd::getClientRect(RECT *rect)
{
	if (!bypassvirtual)
	{
		// CT:getClientRect behaves differently here for virtual windows
		//    so we can use onPaint directly on the destination canvas
		//    without using another temporary canvas.
		Wasabi::Std::setRect(rect, virtualX, virtualY, virtualX + virtualW, virtualY + virtualH);
		//	rect->left=0; rect->right=virtualW;
		//	rect->top=0; rect->bottom=virtualH;
	}
	else
		VIRTUALWND_PARENT::getClientRect(rect);
}

void VirtualWnd::getNonClientRect(RECT *rect)
{
	VirtualWnd::getClientRect(rect);
}

void VirtualWnd::getWindowRect(RECT *rect)
{
	if (!bypassvirtual)
	{
		RECT a;
		getRootParent()->getWindowRect(&a);

		int x = virtualX, y = virtualY, w = virtualW, h = virtualH;

		if (renderRatioActive())
		{
			multRatio(&x, &y);
			multRatio(&w, &h);
		}

		rect->left = a.left + x; rect->right = rect->left + w;
		rect->top  = a.top + y; rect->bottom = rect->top + h;
	}
	else
		VIRTUALWND_PARENT::getWindowRect(rect);
}

int VirtualWnd::beginCapture()
{
	if (!bypassvirtual)
	{
		disable_tooltip_til_recapture = 0;
		getRootParent()->setVirtualChildCapture(this);
		return 1;
	}
	else
		return VIRTUALWND_PARENT::beginCapture();
}

int VirtualWnd::endCapture()
{
	if (!bypassvirtual)
	{
		if (getRootParent() == NULL) return 0;
		getRootParent()->setVirtualChildCapture(NULL);
		return 1;
	}
	else
		return VIRTUALWND_PARENT::endCapture();
}

int VirtualWnd::getCapture()
{
	if (!bypassvirtual)
	{
		if (getRootParent() == NULL) return 0;
		return getRootParent()->getVirtualChildCapture() == this;
	}
	else
		return VIRTUALWND_PARENT::getCapture();
}

void VirtualWnd::setVirtualChildCapture(BaseWnd *child)
{
	if (!bypassvirtual)
	{
		getParent()->setVirtualChildCapture(child);
	}
	else
		VIRTUALWND_PARENT::setVirtualChildCapture(child);
}

// eek
void VirtualWnd::repaint()
{
	if (!bypassvirtual)
	{
		if (!getParent()) return ;
		getParent()->repaint();
	}
	else
		VIRTUALWND_PARENT::repaint();
}

/*int VirtualWnd::focusNextSibbling(int dochild) {
  return 1;
}
 
int VirtualWnd::focusNextVirtualChild(BaseWnd *child) {
  return 1;
}*/

int VirtualWnd::cascadeRepaint(int pack)
{
	if (!bypassvirtual)
	{
		if (getRootParent())
		{
			RECT r;
			VirtualWnd::getNonClientRect(&r);
			getRootParent()->cascadeRepaintRectFrom(&r, this, pack);
		}
		return 1;
	}
	else
		return VIRTUALWND_PARENT::cascadeRepaint(pack);
}

int VirtualWnd::cascadeRepaintRect(RECT *r, int pack)
{
	if (!bypassvirtual)
	{
		if (getRootParent())
		{
			getRootParent()->cascadeRepaintRectFrom(r, this, pack);
		}
		return 1;
	}
	else
		return VIRTUALWND_PARENT::cascadeRepaintRect(r, pack);
}

int VirtualWnd::cascadeRepaintRgn(api_region *r, int pack)
{
	if (!bypassvirtual)
	{
		if (getRootParent())
		{
			getRootParent()->cascadeRepaintRgnFrom(r, this, pack);
		}
		return 1;
	}
	else
		return VIRTUALWND_PARENT::cascadeRepaintRgn(r, pack);
}

/*api_window *VirtualWnd::getWindowBehindMyself(int x, int y) {
  RECT r;
  if (!bypassvirtual) {
    if (!getParent()) return NULL;
    int n = getParent()->getNumVirtuals();
 
    api_window *c = NULL;
 
    for (int i=n-1;i>=0;i++) {
      c = getParent()->getVirtualChild(i);
      if (c == this) break;
    }
 
    i--;
    if (i < 0) return getParent();
 
    for (;i>=0; i--) {
      c = getParent()->getVirtualChild(i);
      c->getNonClientRect(&r);
      if (x>=r.left&&x<=r.right&&y>=r.top&&y<=r.bottom)
        return c;
    }
    return getParent();
  } else
   return NULL;
}*/

ifc_window *VirtualWnd::rootWndFromPoint(POINT *pt)
{
	if (!bypassvirtual)
	{
		if (!getParent()) return NULL;
		return getParent()->rootWndFromPoint(pt);
	}
	else
		return VIRTUALWND_PARENT::rootWndFromPoint(pt);
}

double VirtualWnd::getRenderRatio()
{
	if (!bypassvirtual)
	{
		if (!getParent()) return 1.0;
		return getParent()->getRenderRatio();
	}
	else
		return VIRTUALWND_PARENT::getRenderRatio();
}

void VirtualWnd::bringToFront()
{
	if (!bypassvirtual)
	{
		if (!getParent()) return ;
		//getParent()->bringVirtualToFront(this); TODO: FIX!!!
	}
	else
		VIRTUALWND_PARENT::bringToFront();
}

void VirtualWnd::bringToBack()
{
	if (!bypassvirtual)
	{
		if (!getParent()) return ;
		//getParent()->bringVirtualToBack(this); TODO: FIX!!!
	}
	else
		VIRTUALWND_PARENT::bringToBack();
}

void VirtualWnd::bringAbove(BaseWnd *o)
{
	if (!bypassvirtual)
	{
		if (!getParent()) return ;
		getParent()->bringVirtualAbove(this, o);
	} /* else
	   VIRTUALWND_PARENT::bringAbove();*/
}

void VirtualWnd::bringBelow(BaseWnd *o)
{
	if (!bypassvirtual)
	{
		if (!getParent()) return ;
		getParent()->bringVirtualBelow(this, o);
	} /* else
	   VIRTUALWND_PARENT::bringBelow();*/
}

int VirtualWnd::reparent(ifc_window *newparent)
{
	if (!bypassvirtual)
	{
		if (getParent())
			getParent()->unregisterRootWndChild(this);
		parentWnd = NULL;
		newparent->registerRootWndChild(this);
		onSetParent(newparent);
		newparent->invalidate();
		return 1;
	}
	else
	{
		return VIRTUALWND_PARENT::reparent(newparent);
	}
}

int VirtualWnd::setVirtual(int i)
{
	//  ASSERT(!isInited()); // cut

	if (isInited()) return 0;
	bypassvirtual = !i;
	return 1;
}

ifc_window *VirtualWnd::getRootParent()
{
	if (!bypassvirtual)
	{
		if (!getParent()) return NULL;
		ifc_window *t = this;
		while (t->isVirtual())
		{
			if (!t->getParent()) return NULL;
			t = t->getParent();
		}
		return t;
	}
	else
	{
		return VIRTUALWND_PARENT::getRootParent();
	}
}

int VirtualWnd::gotFocus()
{
	if (!bypassvirtual)
		return focus;
	else
		return VIRTUALWND_PARENT::gotFocus();
}

int VirtualWnd::onGetFocus()
{
	if (!bypassvirtual)
	{
		focus = 1;
		getRootParent()->onSetRootFocus(this);
		invalidate();
		Accessible *a = getAccessibleObject();
		if (a != NULL)
			a->onGetFocus();
	}
	else
		return VIRTUALWND_PARENT::onGetFocus();
	return 1;
}

int VirtualWnd::onKillFocus()
{
	if (!bypassvirtual)
	{
		focus = 0;
		invalidate();
	}
	else
		return VIRTUALWND_PARENT::onKillFocus();
	return 1;
}

void VirtualWnd::setFocus()
{
	ifc_window *f = this;
	if (!f->wantFocus() && f->getParent())
	{
		while (f)
		{
			ifc_window *rp = f->getRootParent();
			if (rp == f) rp = f->getParent();
			f = rp;
			if (f && (!f->getParent() || f->wantFocus() || f == WASABI_API_WND->main_getRootWnd()))
			{
				f->setFocus();
				break;
			}
		}
	}
	else
	{
		if (!bypassvirtual)
		{
			if (getParent())
			{
				getParent()->setVirtualChildFocus(this);
			}
		}
		else
			VIRTUALWND_PARENT::setFocus();
	}
}

void VirtualWnd::setVirtualChildFocus(ifc_window *child)
{
	if (!bypassvirtual)
	{
		getParent()->setVirtualChildFocus(child);
	}
	else
		VIRTUALWND_PARENT::setVirtualChildFocus(child);
}

int VirtualWnd::onActivate()
{
	if (bypassvirtual)
		return VIRTUALWND_PARENT::onActivate();
	return 1;
}

int VirtualWnd::onDeactivate()
{
	if (bypassvirtual)
		return VIRTUALWND_PARENT::onDeactivate();
	return 1;
}

void VirtualWnd::setAllowDeactivation(int allow)
{
	ifc_window *w = getDesktopParent();
	if (w != NULL && w != this)
		w->setAllowDeactivation(allow);
	else VIRTUALWND_PARENT::setAllowDeactivation(allow);
}

int VirtualWnd::allowDeactivation()
{
	ifc_window *w = getDesktopParent();
	if (w != NULL && w != this)
		return w->allowDeactivation();
	return VIRTUALWND_PARENT::allowDeactivation();
}



/* todo:  setCursor
          
   + real childs going invisible should deferedInvalidate their rect on their parent window if it has a virtualCanvas
*/



// No need for screenToClient/clientToScreen overrides since the virtual's origin is the same as it's parent
