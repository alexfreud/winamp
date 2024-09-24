#include <precomp.h>
#include "layer.h"
#include <api/wndmgr/resize.h>
#include <api/wnd/cwndtrack.h>
#ifdef WASABI_COMPILE_WNDMGR
#include <api/wndmgr/layout.h>
#include <api/wac/compon.h>
#endif
#ifdef WASABI_COMPILE_COMPONENTS
#include <api/script/objects/compoobj.h>
#endif
#include <api/skin/skinparse.h>
#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
#include <api/skin/widgets/fx_dmove.h>
#endif
#include <api/script/objects/sregion.h>

#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <api/service/svcs/svc_action.h>
#include <api/script/vcpu.h>
#include <api/wnd/notifmsg.h>
#include <tataki/canvas/bltcanvas.h>
#include <api/wnd/PaintCanvas.h>

#define SNAP_X 1
#define SNAP_Y 1
#define FX_TIMER 0x0A1C

const wchar_t layerXuiObjectStr[] = L"Layer"; // This is the xml tag
char layerXuiSvcName[] = "Layer xui object"; // this is the name of the xuiservice

XMLParamPair Layer::params[] =
  {
		{LAYER_SETMYCURSOR, L"CURSOR"},
    {LAYER_SETDBLCLICKACTION, L"DBLCLICKACTION"},
		{LAYER_DBLCLICKPARAM, L"DBLCLICKPARAM"}, 
    {LAYER_SETIMAGE, L"IMAGE"},
    {LAYER_SETINACTIVEIMAGE, L"INACTIVEIMAGE"},
		{LAYER_SETQUALITY, L"QUALITY"},
    {LAYER_SETREGION, L"REGION"},
    {LAYER_SETRESIZE, L"RESIZE"},
    {LAYER_SETSCALE, L"SCALE"},
    {LAYER_SETTILE, L"TILE"},
	
  };

Layer::Layer()
		: resizer(0), resizeway(0), scaler(0), scalerway(0), scaling(0),
		  rgn(NULL), secrgn(NULL), rgnclone(NULL), fx_wrap(0), fx_rect(0),
		  fx_dmove(NULL)
{
	getScriptObject()->vcpu_setInterface(layerGuid, (void *)static_cast<Layer *>(this));
	getScriptObject()->vcpu_setClassName(L"Layer");
	getScriptObject()->vcpu_setController(layerController);
	resizerect = 1;
	tiling = 0;
	l_customcursor = false;
	hasInactiveImage = 0;
	fx_on = 0;
	fx_bgfx = 0;
	fx_grid_x = 16;
	fx_grid_y = 16;
	fx_bilinear = 1;
	fx_alphamode = 0;
	fx_delay = 35;
	fx_timeron = 0;
	fx_clear = 1;
	fx_local = 1;
	fx_realtime = 0;
	last_w = last_h = -1;
	getGuiObject()->guiobject_setMover(1);
	setRectRgn(1);
	xuihandle = newXuiHandle();
CreateXMLParameters(xuihandle);
	
}

void Layer::CreateXMLParameters(int master_handle)
{
	//LAYER_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

Layer::~Layer()
{
	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI*>(this));
	delete rgnclone;
	delete rgn;
	delete secrgn;
	delete fx_dmove;
}

int Layer::onInit()
{
	LAYER_PARENT::onInit();
	WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI*>(this));
	return 1;
}


int Layer::onPaint(Canvas *canvas)
{
	PaintCanvas paintcanvas;
	if (canvas == NULL)
	{
		if (!paintcanvas.beginPaint(this)) return 0;
		canvas = &paintcanvas;
	}
	LAYER_PARENT::onPaint(canvas);

	if (!getBitmap()) return 0;

	int talpha = getPaintingAlpha();

	RegionI *orig = NULL;

	if (secrgn)
	{
		orig = new RegionI(canvas);
		api_region *clone = orig->clone();
		api_region *secclone = secrgn->clone();
		RECT dr;
		getClientRect(&dr);
		secclone->offset(dr.left, dr.top);
		clone->andRegion(secclone);
		canvas->selectClipRgn(clone);
		orig->disposeClone(clone);
		secrgn->disposeClone(secclone);
	}

	RECT r, srcR, cb, cr;
	getClientRect(&srcR);
	r = cr = srcR;

	if (canvas->getClipBox(&cb))
	{
		r.left = MAX(r.left, cb.left);
		r.top = MAX(r.top, cb.top);
		r.right = MIN(r.right, cb.right);
		r.bottom = MIN(r.bottom, cb.bottom);
		srcR = r;
	}

	layer_adjustDest(&cr);
	layer_adjustDest(&r);


#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
	int bmw = getBitmap()->getWidth();
	int bmh = getBitmap()->getHeight();
	int bmpitch = getBitmap()->getFullWidth();
	int *bmbits = (int*)getBitmap()->getBits();

	if (bmbits) bmbits += getBitmap()->getX() + getBitmap()->getY() * bmpitch;
#endif

	if (getWidth() == (cr.right - cr.left) && getHeight() == (cr.bottom - cr.top))
	{
		int w = r.right - r.left, h = r.bottom - r.top;
		srcR.top = r.top - cr.top;
		srcR.left = r.left - cr.left;
		srcR.top += getSourceOffsetY();
		srcR.left += getSourceOffsetX();
		srcR.bottom = srcR.top + h;
		srcR.right = srcR.left + w;
		// FG> NOT READY FOR OPTIMIZATION YET
#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
		if (fx_on && fx_dmove && bmbits)
		{

			if (fx_bgfx)
			{
				if (!fx_dmove->getBltCanvas() || !fx_dmove->getBltCanvas()->getBits())
					fx_dmove->render(this, getWidth(), getHeight(), bmbits, bmw, bmh, bmpitch);

				int w, h, p, *bits = (int*)canvas->getBits();
				if (bits && !canvas->getDim(&w, &h, &p) &&
				    cr.left + getWidth() < w && cr.top + getHeight() < h && cr.left >= 0 && cr.top >= 0)
				{
					fx_dmove->render(this, getWidth(), getHeight(), bits + cr.left + cr.top*w, getWidth(), getHeight(), w);
				}
				else
				{
					BltCanvas c(getWidth(), getHeight());
					canvas->blit(cr.left, cr.top, &c, 0, 0, getWidth(), getHeight());
					fx_dmove->render(this, getWidth(), getHeight(), (int *)c.getBits(),
					                 getWidth(), getHeight(), getWidth());
				}
				if (fx_dmove->getBltCanvas())
					fx_dmove->getBltCanvas()->blit(0, 0, canvas, cr.left, cr.top, getWidth(), getHeight());

			}
			else
			{ // no bgfx
				if (fx_getClear() && fx_dmove->getBltCanvas() && fx_dmove->getBltCanvas()->getBits())
					fx_dmove->getBltCanvas()->fillBits(0);
				fx_dmove->render(this, getWidth(), getHeight(), bmbits, bmw, bmh, bmpitch);
				//SkinBitmap *b = NULL;
				if (fx_dmove->getBltCanvas())
				{
					fx_dmove->getBltCanvas()->stretchToRectAlpha(canvas, &srcR, &r, talpha);
//					b = fx_dmove->getBltCanvas()->getSkinBitmap();
//					if (b)
//						b->stretchToRectAlpha(canvas, &srcR, &r, talpha);
				}
			}
		}
		else
#endif
			getBitmap()->stretchToRectAlpha(canvas, &srcR, &r, talpha);
	}
	else
	{
		if (r.right == r.left || r.bottom == r.top)
		{
			if (orig)
			{
				canvas->selectClipRgn(orig);
				delete orig;				orig = 0;
			}
			return 0;
		}

		srcR.top = getSourceOffsetY();
		srcR.left = getSourceOffsetX();
		srcR.bottom = srcR.top + getHeight();
		srcR.right = srcR.left + getWidth();

		if (!tiling)
		{

#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
			if (fx_on && fx_dmove && bmbits)
			{
				if (fx_bgfx)
				{
					if (!fx_dmove->getBltCanvas() || !fx_dmove->getBltCanvas()->getBits())
						fx_dmove->render(this, cr.right - cr.left, cr.bottom - cr.top, bmbits, bmw, bmh, bmpitch);

					int w, h, p;
					if (canvas->getBits() && !canvas->getDim(&w, &h, &p) &&
					    cr.right < w && cr.bottom < h && cr.left >= 0 && cr.top >= 0)
					{
						fx_dmove->render(this, cr.right - cr.left, cr.bottom - cr.top, (int *)canvas->getBits() + cr.left + cr.top*w,
						                 cr.right - cr.left, cr.bottom - cr.top, w);
					}
					else
					{
						BltCanvas c(cr.right - cr.left, cr.bottom - cr.top);
						canvas->blit(cr.left, cr.top, &c, 0, 0, cr.right - cr.left, cr.bottom - cr.top);
						fx_dmove->render(this, cr.right - cr.left, cr.bottom - cr.top, (int *)c.getBits(),
						                 cr.right - cr.left, cr.bottom - cr.top, cr.right - cr.left);

					}

					if (fx_dmove->getBltCanvas())
						fx_dmove->getBltCanvas()->blit(0, 0, canvas, cr.left, cr.top, cr.right - cr.left, cr.bottom - cr.top);
				}
				else
				{
					if (fx_getClear() && fx_dmove->getBltCanvas() && fx_dmove->getBltCanvas()->getBits())
						fx_dmove->getBltCanvas()->fillBits(0);
					fx_dmove->render(this, getWidth(), getHeight(), bmbits, bmw, bmh, bmpitch);
					//SkinBitmap *b = NULL;
					if (fx_dmove->getBltCanvas())
					{
						fx_dmove->getBltCanvas()->stretchToRectAlpha(canvas, &srcR, &cr, talpha);
//						b = fx_dmove->getBltCanvas()->getSkinBitmap();
//						if (b)
//							b->stretchToRectAlpha(canvas, &srcR, &cr, talpha);

					}
				}
			}
			else
#endif
			{
				getBitmap()->stretchToRectAlpha(canvas, &srcR, &cr, talpha);
			}

		}
		else
		{
			getBitmap()->blitRectToTile(canvas, &cr, &srcR, 0, 0, talpha);
		}
	}

	if (orig)
	{
		canvas->selectClipRgn(orig);
		delete orig; orig = 0;
	}
	return 1;
}

int Layer::onResize()
{
	LAYER_PARENT::onResize();
	RECT r;
	getClientRect(&r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	if (w != last_w || h != last_h)
		invalidateRegionCache();
	return 1;
}

#pragma warning (disable : 4065)
void Layer::timerCallback(int id)
{
	switch (id)
	{
#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
		case FX_TIMER:
			fx_onFrame();
			invalidate();
			break;
#endif

		default:
			LAYER_PARENT::timerCallback(id);
			break;
	}
}
#pragma warning (default: 4065)

void Layer::invalidateRegionCache()
{
	delete rgnclone;
	rgnclone = NULL;
	invalidateWindowRegion();
}

int Layer::setXuiParam(int _xuihandle, int attribid, const wchar_t *paramname, const wchar_t *strvalue)
{
	if (xuihandle != _xuihandle) return LAYER_PARENT::setXuiParam(_xuihandle, attribid, paramname, strvalue);
	switch (attribid)
	{
		case LAYER_SETIMAGE:
			setBitmap(strvalue);
			break;
		case LAYER_SETRESIZE:
			setResize(SkinParser::parseResize(strvalue));
			if (resizer) getGuiObject()->guiobject_setMover(0);
			break;
		case LAYER_SETSCALE:
			setScaler(SkinParser::parseResize(strvalue));
			if (scaler) getGuiObject()->guiobject_setMover(0);
			break;
		case LAYER_SETREGION:
			setRegionFromBitmap(strvalue);
			break;
		case LAYER_SETTILE:
			setTiling(WTOI(strvalue));
			break;
		case LAYER_SETDBLCLICKACTION:
			dblClickAction = strvalue;
			break;
		case LAYER_DBLCLICKPARAM: 
			setDblClickParam(strvalue);
			break;
		case LAYER_SETINACTIVEIMAGE:
			setInactiveBitmap(strvalue);
			break;
		case LAYER_SETMYCURSOR:
			l_customcursor = true;
			getGuiObject()->guiobject_setCursor(strvalue);
			break;
		case LAYER_SETQUALITY:
			bitmap.setResamplingMode(WTOI(strvalue));
			break;
		default:
			return 0;

	}
	return 1;
}

int Layer::getPreferences(int what)
{
	if (what == SUGGESTED_W)
		return getWidth();
	if (what == SUGGESTED_H)
		return getHeight();
	return LAYER_PARENT::getPreferences(what);
}

int Layer::getSourceOffsetX()
{
	return 0;
}

int Layer::getSourceOffsetY()
{
	return 0;
}

void Layer::setBitmap(const wchar_t *name)
{
	bitmapname = name;
	bitmap = name;
	deleteRegion();
	makeRegion();
	notifyParent(ChildNotify::AUTOWHCHANGED);
	if (isInited()) invalidate();
}

void Layer::setInactiveBitmap(const wchar_t *name)
{
	inactiveImageName = name;
	inactiveBitmap = name;
	hasInactiveImage = 1;
}

void Layer::setDblClickParam(const wchar_t *p) 
{
	dblclickparam=p;
}

const wchar_t *Layer::getDblClickParam() 
{
	return dblclickparam;
}

int Layer::getWidth()
{
	if (getBitmap()) return getBitmap()->getWidth();
	return 0;
}

int Layer::getHeight()
{
	if (getBitmap()) return getBitmap()->getHeight();
	return 0;
}

int Layer::onLeftButtonDown(int x, int y)
{
	LAYER_PARENT::onLeftButtonDown(x, y);
	ifc_window *d = getDesktopParent();

#ifdef WASABI_COMPILE_WNDMGR
	Layout *_l = NULL;
	if (d != NULL)
	{
		_l = static_cast<Layout *>(d->getInterface(layoutGuid));
		if (_l && _l->isLocked()) return 1;
	}

	int need_resize = 0, need_scale = 0, need_move = 0;

	int dock = APPBAR_NOTDOCKED;
	int rw = applyResizeRestrictions(resizeway, &dock);
	int sw = (dock == APPBAR_NOTDOCKED) ? scalerway : RESIZE_NONE;

	if (resizer && rw != RESIZE_NONE && (!_l || _l->getResizable()))
	{
		if (scaler && sw != RESIZE_NONE && Std::keyModifier(STDKEY_ALT) && (!_l || _l->getScalable()))
			need_scale = 1;
		else
			need_resize = 1;
	}
	else if (scaler && sw != RESIZE_NONE && (!_l || _l->getScalable()))
	{
		need_scale = 1;
	}
	else if (getGuiObject()->guiobject_getMover())
	{
		need_move = 1;
	}

	Group *l = static_cast<Group *>(_l);

	if (need_move)
	{
		// do nothin'
	}
	else if (need_scale)
	{
		if (l)
		{
			l->beginScale();
			scaling = 1;
		}
		return 1;
	}
	else if (need_resize)
	{
		resizerect = 1;
		if (l)
		{
			clientToScreen(&x, &y);
			l->mouseResize(x, y, resizeway);
		}
	}
#endif
	return 1;
}

int Layer::onLeftButtonUp(int x, int y)
{
	LAYER_PARENT::onLeftButtonUp(x, y);
#ifdef WASABI_COMPILE_WNDMGR
	if (scaling)
	{
		getGuiObject()->guiobject_getParentGroup()->endScale();
		scaling = 0;
	}
#endif
	return 1;
}

void Layer::onCancelCapture()
{
	LAYER_PARENT::onCancelCapture();
	scaling = 0;
}

int Layer::onMouseMove(int x, int y)
{
	LAYER_PARENT::onMouseMove(x, y);
	POINT pos = {x, y};
	clientToScreen(&pos);

#ifdef WASABI_COMPILE_WNDMGR
	if (scaling && getCapture())
	{ // todo: resizing w/no rect
		if (!Std::keyDown(MK_LBUTTON))
			scaling = 0;
		else
		{
			RECT oldRect;
			RECT newRect;
			ifc_window *b = getRootParent();
			b->getWindowRect(&oldRect);
			newRect = oldRect;
			clientToScreen(&x, &y);
			int mask = 0;
			if (scalerway & RESIZE_BOTTOM)
			{
				newRect.bottom = y - y % SNAP_Y;
				mask |= BOTTOM;
			}
			if (scalerway & RESIZE_RIGHT)
			{
				newRect.right = x - x % SNAP_X;
				mask |= RIGHT;
			}
			if (scalerway & RESIZE_TOP)
			{
				newRect.top = y - y % SNAP_Y;
				mask |= TOP;
			}
			if (scalerway & RESIZE_LEFT)
			{
				newRect.left = x - x % SNAP_X;
				mask |= LEFT;
			}

			RECT dr = newRect;
			WASABI_API_WNDMGR->wndTrackDock(b, &dr, &newRect, mask | NOINTERSECT);
			if (dr.right - dr.left < 32) dr.right = dr.left + 32;
			if (dr.bottom - dr.top < 32) dr.bottom = dr.top + 32;
			if (MEMCMP(&dr, &oldRect, sizeof(RECT)))
			{
				RECT origRect;
				b->getClientRect(&origRect);
				double rx = (double)(dr.right - dr.left) / (double)(origRect.right - origRect.left);
				double ry = (double)(dr.bottom - dr.top) / (double)(origRect.bottom - origRect.top);
				double r = MAX(rx, ry);
				if (r != 0) b->setRenderRatio(r);
			}
		}
	}
#endif

	return 1;
}

SkinBitmap *Layer::getBitmap()
{
	return layer_getBitmap();
}

SkinBitmap *Layer::layer_getBitmap()
{
	if (hasInactiveImage)
		if (!isActive())
			return inactiveBitmap.getBitmap();
	return bitmap.getBitmap();
}

const wchar_t *Layer::layer_getBitmapName()
{
	return bitmapname;
}

int Layer::applyResizeRestrictions(int way, int *side)
{
	ifc_window *rw = getDesktopParent();
	int dock = APPBAR_NOTDOCKED;
	if (side) *side = dock;
	if (rw)
	{
		Layout *l = (Layout *)rw->getInterface(layoutGuid);
		if (l) dock = l->appbar_getSide();
	}
	if (dock == APPBAR_NOTDOCKED) return way;
	/*
	switch (way) {
	 case RESIZE_TOPLEFT:
	  if (dock == APPBAR_TOP || dock == APPBAR_LEFT) return RESIZE_NONE;
	  break;
	 case RESIZE_BOTTOMRIGHT:
	  if (dock == APPBAR_RIGHT || dock == APPBAR_BOTTOM) return RESIZE_NONE;
	  break;
	 case RESIZE_TOPRIGHT:
	  if (dock == APPBAR_TOP || dock == APPBAR_RIGHT) return RESIZE_NONE;
	  break;
	 case RESIZE_BOTTOMLEFT:
	  if (dock == APPBAR_BOTTOM || dock == APPBAR_LEFT) return RESIZE_NONE;
	  break;
	 case RESIZE_TOP:
	  if (dock == APPBAR_TOP) return RESIZE_NONE;
	  break;
	 case RESIZE_BOTTOM:
	  if (dock == APPBAR_BOTTOM) return RESIZE_NONE;
	  break;
	 case RESIZE_LEFT:
	  if (dock == APPBAR_LEFT) return RESIZE_NONE;
	  break;
	 case RESIZE_RIGHT:
	  if (dock == APPBAR_RIGHT) return RESIZE_NONE;
	  break;
	}
	return way;*/
	if (dock != APPBAR_NOTDOCKED) return RESIZE_NONE;
	return way;
}

int Layer::getCursorType(int x, int y)
{
	// Return our user cursor if we have one (even when we are resizable!)
	if (l_customcursor)
	{
		return BASEWND_CURSOR_USERSET;
	}
	// Then check if our layer is used to resize or scale the wnd
	ifc_window *d = getDesktopParent();
	int scalable = 1;
	int resizable = 1;
	Layout *_l = NULL;
	if (d != NULL)
	{
		_l = static_cast<Layout *>(d->getInterface(layoutGuid));
		if (_l)
		{
			scalable = _l->getScalable();
			resizable = _l->getResizable();
		}
	}

	if (!_l || (!_l->isLocked() && (resizer || scaler)))
		{
		int dock = APPBAR_NOTDOCKED;

		int r = 0;
		if (resizer && resizable)
		{
			if (scaler && scalable)
			{
				if (Std::keyModifier(STDKEY_ALT))
				{
					r = applyResizeRestrictions(scalerway, &dock);
					if (dock != APPBAR_NOTDOCKED) r = RESIZE_NONE;
				}
				else
				{
					r = applyResizeRestrictions(resizeway);
				}
			}
			else
			{
				r = applyResizeRestrictions(resizeway);
			}
		}
		if (scaler && scalable)
		{
			r = applyResizeRestrictions(scalerway, &dock);
			if (dock != APPBAR_NOTDOCKED) r = RESIZE_NONE;
		}

		switch (r)
		{
			case RESIZE_TOPLEFT:
			case RESIZE_BOTTOMRIGHT:
				return BASEWND_CURSOR_NORTHWEST_SOUTHEAST;
			case RESIZE_TOPRIGHT:
			case RESIZE_BOTTOMLEFT:
				return BASEWND_CURSOR_NORTHEAST_SOUTHWEST;
				break;
			case RESIZE_TOP:
			case RESIZE_BOTTOM:
				return BASEWND_CURSOR_NORTHSOUTH;
				break;
			case RESIZE_LEFT:
			case RESIZE_RIGHT:
				return BASEWND_CURSOR_EASTWEST;
				break;
		}
	}
	// At leat we want to have a cursor so spit our the normal pointer
	return BASEWND_CURSOR_POINTER;
}

void Layer::setScaler(int r)
{
	if (r != 0)
	{
		scaler = 1;
		scalerway = r;
	}
	else
	{
		resizer = 0;
		scalerway = 0;
	}
}

void Layer::setResize(int r)
{
	if (r != 0)
	{
		resizer = 1;
		resizeway = r;
	}
	else
	{
		resizer = 0;
		resizeway = 0;
	}
}

api_region *Layer::getBitmapRegion()
{
	return rgn;
}

api_region *Layer::getRegion()
{
	api_region *_rgn = getBitmapRegion();
	if (!_rgn) return NULL;
	if (!rgnclone)
	{
		rgnclone = new RegionI();
		rgnclone->addRegion(_rgn);
	}
	else
		return rgnclone;
	float w, h;
	RECT r;
	getClientRect(&r);
	if (r.right - r.left == 0 || r.bottom - r.top == 0) return NULL;
	last_w = r.right - r.left;
	last_h = r.bottom - r.top;
	if (getWidth() == 0 || getHeight() == 0) return NULL;
	w = (float)(last_w) / (float)getWidth();
	h = (float)(last_h) / (float)getHeight();
	if (!w || !h) return NULL;
	if (!tiling)
	{
		if (w != 1.0 || h != 1.0)
		{
			rgnclone->scale(w, h, TRUE);
		}
	}
	else
	{
		for (int j = 0;j < h;j++)
		{
			for (int i = 0;i < w;i++)
			{
				api_region *t = _rgn->clone();
				t->offset(i*getWidth(), j*getHeight());
				rgnclone->addRegion(t);
				_rgn->disposeClone(t);
			}
		}
	}
	RegionI _r(&r);
	_r.offset(-r.left, -r.top);
	rgnclone->andRegion(&_r);

	return rgnclone;
}

void Layer::setRegionOp(int i)
{
	int old = getRegionOp();
	LAYER_PARENT::setRegionOp(i);
	if (!old && i) setRectRgn(0);
}

void Layer::makeRegion()
{
	if (!bitmap.getBitmap())
		return;
	delete rgn;
	rgn = new RegionI(bitmap);
	invalidateRegionCache();
}

void Layer::deleteRegion()
{
	if (!rgn) return;
	invalidateRegionCache();
	delete rgn;
	rgn = NULL;
}

int Layer::onLeftButtonDblClk(int x, int y)
{
	// Martin> copied and adopted from textbase.cpp
	//if (!dblClickAction.isempty() && !VCPU::getComplete())
	if (dblClickAction)
	{
#ifdef WASABI_COMPILE_WNDMGR
		const wchar_t *toCheck = L"SWITCH;";
		if (!_wcsnicmp(dblClickAction, toCheck, 7))
		{
			onLeftButtonUp(x, y);
			getGuiObject()->guiobject_getParentGroup()->getParentContainer()->switchToLayout(dblClickAction.getValue() + 7);
			return 0;
		}
		else
		{
#endif
			svc_action *act = ActionEnum(dblClickAction).getNext();
			if (act)
			{
				int _x = x;
				int _y = y;
				clientToScreen(&_x, &_y);
				act->onAction(dblClickAction, getDblClickParam(), _x, _y, NULL, 0, this);
				//WASABI_API_WNDMGR->messageBox(L"on action; passed", L"debug", 0, NULL, NULL);
				SvcEnum::release(act);
			}
#ifdef WASABI_COMPILE_WNDMGR

		}
#endif

	}
	int r = LAYER_PARENT::onLeftButtonDblClk(x, y);
	return r;
}


void Layer::setRegionFromMap(SMap *map, int byte, int inversed)
{
	if (secrgn)
	{
		invalidate();
		delete secrgn;
		secrgn = NULL;
	}
	//RECT r = {map->getBitmap()->getX(), map->getBitmap()->getY(), map->getBitmap()->getWidth(), map->getBitmap()->getHeight()};
	secrgn = new RegionI(map->getBitmap(), NULL, 0, 0, FALSE, 1, (unsigned char)byte, inversed);
	if (secrgn)
		invalidate();
}

void Layer::setRegion(SRegion *reg)
{
	invalidate();
	delete secrgn;
	if (!reg)
	{
		secrgn = NULL; invalidate(); return;
	}
	secrgn = new RegionI();
	secrgn->addRegion(reg->getRegion());
	invalidate();
}

int Layer::wantSiblingInvalidations()
{
	return fx_on;
}

int Layer::onSiblingInvalidateRgn(api_region *r, ifc_window *who, int who_idx, int my_idx)
{
	if (!fx_on || !fx_bgfx || who_idx >= my_idx) return 0;
	RECT rr;
	getClientRect(&rr);
	if (rgn)
	{
		api_region *_r = rgn->clone();
		_r->offset(rr.left, rr.top);
		if (fx_local && !_r->doesIntersectRgn(r))
		{
			rgn->disposeClone(_r);
			return 0;
		}
		if (secrgn)
		{
			api_region *_rs = secrgn->clone();
			_rs->offset(rr.left, rr.top);
			_r->andRegion(_rs);
			secrgn->disposeClone(_rs);
		}
		r->addRegion(_r);
		rgn->disposeClone(_r);
	}
	else
	{
		RegionI _r(&rr);
		if (secrgn)
		{
			api_region *_rs = secrgn->clone();
			_rs->offset(rr.left, rr.top);
			_r.andRegion(_rs);
			secrgn->disposeClone(_rs);
		}
		if (fx_local && !_r.doesIntersectRgn(r))
		{
			return 0;
		}
		r->addRegion(&_r);
	}
	return 1;
}

void Layer::onSetVisible(int show)
{
	LAYER_PARENT::onSetVisible(show);
	getGuiObject()->guiobject_onSetVisible(show);
}

int Layer::onActivate()
{
	invalidate();
	return 0;
}

int Layer::onDeactivate()
{
	invalidate();
	return 0;
}

void Layer::setTiling(int t)
{
	if (tiling == t) return;
	tiling = t;
	invalidate();
}

int Layer::getTiling()
{
	return tiling;
}


void Layer::onBeginResize(RECT r)
{
	scriptVar l = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&l, r.left);
	scriptVar t = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&t, r.top);
	scriptVar w = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&w, r.right - r.left);
	scriptVar h = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&h, r.bottom - r.top);
	script_vcpu_onBeginResize(SCRIPT_CALL, getScriptObject(), l, t, w, h);
}

void Layer::onEndResize(RECT r)
{
	scriptVar l = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&l, r.left);
	scriptVar t = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&t, r.top);
	scriptVar w = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&w, r.right - r.left);
	scriptVar h = SOM::makeVar(SCRIPT_INT);
	SOM::assign(&h, r.bottom - r.top);
	script_vcpu_onEndResize(SCRIPT_CALL, getScriptObject(), l, t, w, h);
}

#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
void Layer::fx_setEnabled(int i)
{
	static int inited = 0;
	if (fx_getEnabled() == !!i) return;
	fx_on = !!i;
	if (fx_on)
	{
		if (!inited)
			fx_onInit();
		fx_dmove = new FxDynamicMove;
		fx_dmove->setBilinear(fx_getBilinear());
		if (fx_getAlphaMode())
			fx_dmove->setAlphaMode(1 | (fx_getBgFx() ? 2 : 0));
		else fx_setAlphaMode(0);
		fx_dmove->setAlphaOnce(fx_alphaonce);
		fx_dmove->setRect(fx_getRect());
		fx_dmove->setWrap(fx_getWrap());
		fx_dmove->setGridSize(fx_grid_x, fx_grid_y);
		if (fx_realtime)
		{
			setTimer(FX_TIMER, fx_delay);
			fx_timeron = 1;
		}
	}
	else
	{
		if (fx_timeron)
		{
			killTimer(FX_TIMER);
			fx_timeron = 0;
		}
		delete fx_dmove;
		fx_dmove = NULL;
	}
	invalidate();
}

int Layer::fx_getEnabled(void)
{
	return fx_on;
}

void Layer::fx_setWrap(int i)
{
	if (fx_getWrap() == !!i) return;
	fx_wrap = !!i;
	if (fx_dmove) fx_dmove->setWrap(fx_wrap);
	invalidate();
}

int Layer::fx_getWrap(void)
{
	return fx_wrap;
}

void Layer::fx_setRect(int i)
{
	if (fx_getRect() == !!i) return;
	fx_rect = !!i;
	if (fx_dmove) fx_dmove->setRect(fx_rect);
	invalidate();
}

int Layer::fx_getRect(void)
{
	return fx_rect;
}

int Layer::fx_getAlphaMode()
{
	if (!fx_alphamode) return 0;
	if (fx_alphaonce) return 2;
	return 1;
}

void Layer::fx_setAlphaMode(int i)
{

	if (i == 2)
	{
		i = 1;
		fx_alphaonce = 1;
		if (fx_dmove) fx_dmove->setAlphaOnce(1);
	}
	else
	{
		fx_alphaonce = 0;
		if (fx_dmove) fx_dmove->setAlphaOnce(0);
	}

	if (fx_getAlphaMode() == !!i)  return;
	fx_alphamode = !!i;
	if (fx_dmove)
	{
		if (fx_getAlphaMode())
			fx_dmove->setAlphaMode(1 | (fx_getBgFx() ? 2 : 0));
		else
			fx_dmove->setAlphaMode(0);
	}
	invalidate();
}


int Layer::fx_getBilinear()
{
	return fx_bilinear;
}

void Layer::fx_setBilinear(int i)
{
	if (fx_getBilinear() == !!i)  return;
	fx_bilinear = !!i;
	if (fx_dmove) fx_dmove->setBilinear(fx_bilinear);
	invalidate();
}

int Layer::fx_getBgFx()
{
	return fx_bgfx;
}

void Layer::fx_setBgFx(int i)
{
	if (fx_getBgFx() == !!i) return;
	fx_bgfx = !!i;
	if (fx_dmove) fx_dmove->setCanCache(!i);
	if (fx_dmove && fx_getAlphaMode())
	{
		fx_dmove->setAlphaMode(1 | (fx_getBgFx() ? 2 : 0));
	}
	invalidate();
}

int Layer::fx_getSpeed()
{
	return fx_delay;
}

void Layer::fx_setSpeed(int s)
{
	if (fx_getSpeed() == s) return;
	fx_delay = s;
	if (fx_timeron)
	{
		killTimer(FX_TIMER);
		fx_timeron = 0;
	}
	if (fx_on && fx_realtime)
	{
		setTimer(FX_TIMER, fx_delay);
		fx_timeron = 1;
	}
}

int Layer::fx_getClear()
{
	return fx_clear;
}

void Layer::fx_setClear(int i)
{
	if (fx_getClear() == !!i) return;
	fx_clear = !!i;
}

int Layer::fx_getRealtime()
{
	return fx_realtime;
}

void Layer::fx_setRealtime(int i)
{
	if (fx_getRealtime() == !!i) return;
	fx_realtime = !!i;
}

int Layer::fx_getLocalized()
{
	return fx_local;
}

void Layer::fx_setLocalized(int i)
{
	if (fx_getLocalized() == !!i) return;
	fx_local = !!i;
}

void Layer::fx_setGridSize(int x, int y)
{
	fx_grid_x = x;
	fx_grid_y = y;
	if (!fx_dmove) return;
	fx_dmove->setGridSize(x, y);
}

void Layer::fx_update(void)
{
	RECT r;
	getClientRect(&r);
	if (fx_realtime && !isMinimized())
		cascadeRepaintRect(&r);
	else
		invalidateRect(&r);
}

void Layer::fx_restart(void)
{
	fx_onInit();
	if (fx_dmove && fx_dmove->getBltCanvas() && fx_dmove->getBltCanvas()->getBits())
		fx_dmove->getBltCanvas()->fillBits(0);
	// todo: reset need_alpha in fx_dmove
}

void Layer::fx_onInit(void)
{
	script_vcpu_fx_onInit(SCRIPT_CALL, getScriptObject());
}

void Layer::fx_onFrame(void)
{
	script_vcpu_fx_onFrame(SCRIPT_CALL, getScriptObject());
}

double Layer::fx_onGetPixelA(double r, double d, double x, double y)
{
	scriptVar _x = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _y = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _r = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _d = SOM::makeVar(SCRIPT_DOUBLE);
	SOM::assign(&_x, x);
	SOM::assign(&_y, y);
	SOM::assign(&_r, r);
	SOM::assign(&_d, d);
	scriptVar v = script_vcpu_fx_onGetPixelA(SCRIPT_CALL, getScriptObject(), _r, _d, _x, _y);
	return (v.type == 0) ? 0.5 : v.data.ddata;
}


double Layer::fx_onGetPixelX(double r, double d, double x, double y)
{
	scriptVar _x = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _y = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _r = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _d = SOM::makeVar(SCRIPT_DOUBLE);
	SOM::assign(&_x, x);
	SOM::assign(&_y, y);
	SOM::assign(&_r, r);
	SOM::assign(&_d, d);
	scriptVar v = script_vcpu_fx_onGetPixelX(SCRIPT_CALL, getScriptObject(), _r, _d, _x, _y);
	return (v.type == 0) ? x : v.data.ddata;
}

double Layer::fx_onGetPixelY(double r, double d, double x, double y)
{
	scriptVar _x = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _y = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _r = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _d = SOM::makeVar(SCRIPT_DOUBLE);
	SOM::assign(&_x, x);
	SOM::assign(&_y, y);
	SOM::assign(&_r, r);
	SOM::assign(&_d, d);
	scriptVar v = script_vcpu_fx_onGetPixelY(SCRIPT_CALL, getScriptObject(), _r, _d, _x, _y);
	return (v.type == 0) ? y : v.data.ddata;
}

double Layer::fx_onGetPixelR(double r, double d, double x, double y)
{
	scriptVar _x = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _y = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _r = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _d = SOM::makeVar(SCRIPT_DOUBLE);
	SOM::assign(&_x, x);
	SOM::assign(&_y, y);
	SOM::assign(&_r, r);
	SOM::assign(&_d, d);
	scriptVar v = script_vcpu_fx_onGetPixelR(SCRIPT_CALL, getScriptObject(), _r, _d, _x, _y);
	return (v.type == 0) ? r : v.data.ddata;
}

double Layer::fx_onGetPixelD(double r, double d, double x, double y)
{
	scriptVar _x = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _y = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _r = SOM::makeVar(SCRIPT_DOUBLE);
	scriptVar _d = SOM::makeVar(SCRIPT_DOUBLE);
	SOM::assign(&_x, x);
	SOM::assign(&_y, y);
	SOM::assign(&_r, r);
	SOM::assign(&_d, d);
	scriptVar v = script_vcpu_fx_onGetPixelD(SCRIPT_CALL, getScriptObject(), _r, _d, _x, _y);
	return (v.type == 0) ? d : v.data.ddata;
}
#endif

int Layer::skincb_onColorThemeChanged(const wchar_t *newcolortheme)
{
  #if defined(WA3COMPATIBILITY) || defined(GEN_FF)
	if (fx_getEnabled())
	{
		fx_dmove->flushCache();
		fx_update();
	}
#endif
	return 0;
}

void Layer::setRegionFromBitmap(const wchar_t *bmpid)
{
	delete secrgn;
	secrgn = NULL;
	if (bmpid == NULL || *bmpid == '\0')
		return;
	bool invert = (*bmpid == '!');
	if (invert) bmpid++;
	SkinBitmap b(bmpid);
	secrgn = new RegionI(&b, NULL, 0, 0, invert);
}

LayerScriptController _layerController;
LayerScriptController *layerController = &_layerController;

// -- Functions table -------------------------------------
function_descriptor_struct LayerScriptController::exportedFunction[] = {
      {L"setRegionFromMap",  3, (void*)Layer::script_vcpu_setRegionFromMap },
      {L"setRegion",         1, (void*)Layer::script_vcpu_setRegion },
			{L"isInvalid",       0, (void*)Layer::script_vcpu_isInvalid },
#ifdef WASABI_COMPILE_WNDMGR
      {L"onBeginResize",     4, (void*)Layer::script_vcpu_onBeginResize },
      {L"onEndResize",       4, (void*)Layer::script_vcpu_onEndResize },
#endif
#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
      {
        L"fx_setEnabled",     1, (void*)Layer::script_vcpu_fx_setEnabled
      },
      {L"fx_getEnabled",     0, (void*)Layer::script_vcpu_fx_getEnabled },
      {L"fx_onInit",         0, (void*)Layer::script_vcpu_fx_onInit },
      {L"fx_onFrame",        0, (void*)Layer::script_vcpu_fx_onFrame },
      {L"fx_onGetPixelR",    4, (void*)Layer::script_vcpu_fx_onGetPixelR },
      {L"fx_onGetPixelD",    4, (void*)Layer::script_vcpu_fx_onGetPixelD },
      {L"fx_onGetPixelX",    4, (void*)Layer::script_vcpu_fx_onGetPixelX },
      {L"fx_onGetPixelY",    4, (void*)Layer::script_vcpu_fx_onGetPixelY },
      {L"fx_onGetPixelA",    4, (void*)Layer::script_vcpu_fx_onGetPixelA },
      {L"fx_setWrap",        1, (void*)Layer::script_vcpu_fx_setWrap },
      {L"fx_getWrap",        0, (void*)Layer::script_vcpu_fx_getWrap },
      {L"fx_setRect",        1, (void*)Layer::script_vcpu_fx_setRect },
      {L"fx_getRect",        0, (void*)Layer::script_vcpu_fx_getRect },
      {L"fx_setBgFx",        1, (void*)Layer::script_vcpu_fx_setBgFx },
      {L"fx_getBgFx",        0, (void*)Layer::script_vcpu_fx_getBgFx },
      {L"fx_setClear",       1, (void*)Layer::script_vcpu_fx_setClear },
      {L"fx_getClear",       0, (void*)Layer::script_vcpu_fx_getClear },
      {L"fx_setSpeed",       1, (void*)Layer::script_vcpu_fx_setSpeed },
      {L"fx_getSpeed",       0, (void*)Layer::script_vcpu_fx_getSpeed },
      {L"fx_setRealtime",    1, (void*)Layer::script_vcpu_fx_setRealtime },
      {L"fx_getRealtime",    0, (void*)Layer::script_vcpu_fx_getRealtime },
      {L"fx_setLocalized",   1, (void*)Layer::script_vcpu_fx_setLocalized },
      {L"fx_getLocalized",   0, (void*)Layer::script_vcpu_fx_getLocalized },
      {L"fx_setBilinear",    1, (void*)Layer::script_vcpu_fx_setBilinear },
      {L"fx_getBilinear",    0, (void*)Layer::script_vcpu_fx_getBilinear },
      {L"fx_setAlphaMode",   1, (void*)Layer::script_vcpu_fx_setAlphaMode },
      {L"fx_getAlphaMode",   0, (void*)Layer::script_vcpu_fx_getAlphaMode },
      {L"fx_setGridSize",    2, (void*)Layer::script_vcpu_fx_setGridSize },
      {L"fx_update",         0, (void*)Layer::script_vcpu_fx_update },
      {L"fx_restart",        0, (void*)Layer::script_vcpu_fx_restart },
#endif
    };
// --------------------------------------------------------

const wchar_t *LayerScriptController::getClassName()
{
	return L"Layer";
}

const wchar_t *LayerScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

ScriptObject *LayerScriptController::instantiate()
{
	Layer *l = new Layer;
	ASSERT(l != NULL);
	return l->getScriptObject();
}

void LayerScriptController::destroy(ScriptObject *o)
{
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	ASSERT(l != NULL);
	delete l;
}

void *LayerScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for layer yet
}

void LayerScriptController::deencapsulate(void *o)
{}

int LayerScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *LayerScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID LayerScriptController::getClassGuid()
{
	return layerGuid;
}


//------------------------------------------------------------------------

scriptVar Layer::script_vcpu_onBeginResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar left, scriptVar top, scriptVar width, scriptVar height)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS4(o, layerController, left, top, width, height);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, left, top, width, height);
}

scriptVar Layer::script_vcpu_onEndResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar left, scriptVar top, scriptVar width, scriptVar height)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS4(o, layerController, left, top, width, height);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, left, top, width, height);
}

scriptVar Layer::script_vcpu_setRegionFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&byte));
	ASSERT(SOM::isNumeric(&inv));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	SMap *m = static_cast<SMap *>(GET_SCRIPT_OBJECT_AS(map, mapGuid));
	if (l) l->setRegionFromMap(m, GET_SCRIPT_INT(byte), GET_SCRIPT_INT(inv));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_setRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar reg)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	SRegion *r = static_cast<SRegion *>(GET_SCRIPT_OBJECT_AS(reg, regionGuid));
	if (l) l->setRegion(r);
	RETURN_SCRIPT_VOID;
}

bool Layer::layer_isInvalid()
{
		SkinBitmap *b =  layer_getBitmap();
		if (b)
			return !!b->isInvalid();
		else
			return true;
}

scriptVar Layer::script_vcpu_isInvalid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l)
	{
		return MAKE_SCRIPT_INT(l->layer_isInvalid()?1:0);
	}
	return MAKE_SCRIPT_INT(1);
}


#if defined(WA3COMPATIBILITY) || defined(GEN_FF)
scriptVar Layer::script_vcpu_fx_setEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&a));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setEnabled(GET_SCRIPT_INT(a));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getEnabled(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(l->fx_getEnabled());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setWrap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&a));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setWrap(SOM::makeInt(&a));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getWrap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(l->fx_getWrap());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setRect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&a));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setRect(SOM::makeInt(&a));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getRect(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(l->fx_getRect());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setAlphaMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&a));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setAlphaMode(SOM::makeInt(&a));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getAlphaMode(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(l->fx_getAlphaMode());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setBilinear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&a));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setBilinear(SOM::makeInt(&a));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getBilinear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(l->fx_getBilinear());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setBgFx(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&a));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setBgFx(SOM::makeInt(&a));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getBgFx(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(((Layer *)o)->fx_getBgFx());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&s));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setSpeed(SOM::makeInt(&s));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(((Layer *)o)->fx_getSpeed());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&s));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setRealtime(SOM::makeInt(&s));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(((Layer *)o)->fx_getRealtime());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setClear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&s));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setClear(SOM::makeInt(&s));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getClear(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(l->fx_getClear());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setLocalized(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&s));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setLocalized(SOM::makeInt(&s));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_getLocalized(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) return MAKE_SCRIPT_INT(l->fx_getLocalized());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layer::script_vcpu_fx_setGridSize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&x));
	ASSERT(SOM::isNumeric(&y));
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_setGridSize(SOM::makeInt(&x), SOM::makeInt(&y));
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_update(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_update();
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_restart(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layer *l = static_cast<Layer *>(o->vcpu_getInterface(layerGuid));
	if (l) l->fx_restart();
	RETURN_SCRIPT_VOID;
}

scriptVar Layer::script_vcpu_fx_onInit(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, layerController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Layer::script_vcpu_fx_onFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, layerController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Layer::script_vcpu_fx_onGetPixelA(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS4(o, layerController, r, d, x, y);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, r, d, x, y);
}


scriptVar Layer::script_vcpu_fx_onGetPixelX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS4(o, layerController, r, d, x, y);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, r, d, x, y);
}

scriptVar Layer::script_vcpu_fx_onGetPixelY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS4(o, layerController, r, d, x, y);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, r, d, x, y);
}

scriptVar Layer::script_vcpu_fx_onGetPixelR(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS4(o, layerController, r, d, x, y);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, r, d, x, y);
}

scriptVar Layer::script_vcpu_fx_onGetPixelD(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r, scriptVar d, scriptVar x, scriptVar y)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS4(o, layerController, r, d, x, y);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, r, d, x, y);
}

#endif //WA3COMPATIBILITY || GEN_FF


