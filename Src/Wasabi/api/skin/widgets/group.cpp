#include <precomp.h>
#include <tataki/bitmap/bitmap.h>
#include <api/wnd/popup.h>
#include <api/wndmgr/layout.h>
#include <api/skin/skinparse.h>
//#include <api/skin/widgets/button.h>
//#include <api/core/buttons.h>
#include <api/wnd/wndtrack.h>
#include <api/wac/compon.h>
#include <api/skin/skin.h>
#include <api/wnd/notifmsg.h>
#include <api/config/items/intarray.h>
#include <api/config/items/cfgitem.h>
#include <api/config/items/attribute.h>
#include <api/wndmgr/layout.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <bfc/util/profiler.h>
#include <api/wndmgr/resize.h>
#include <bfc/wasabi_std_wnd.h>
#include <api/wnd/PaintCanvas.h>

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x20A
#endif

const wchar_t groupXuiObjectStr[] = L"Group"; // This is the xml tag
char groupXuiSvcName[] = "Group xui object"; // this is the name of the xuiservice

#ifdef WASABI_COMPILE_CONFIG
const wchar_t cfgGroupXuiObjectStr[] = L"CfgGroup"; // This is the xml tag
char cfgGroupXuiSvcName[] = "CfgGroup xui object"; // this is the name of the xuiservice
#endif

XMLParamPair Group::groupParams[] =
    {
        {XUIGROUP_AUTOHEIGHTSOURCE, L"AUTOHEIGHTSOURCE"},
        {XUIGROUP_AUTOWIDTHSOURCE, L"AUTOWIDTHSOURCE"},
        {XUIGROUP_BACKGROUND, L"BACKGROUND"},
        {XUIGROUP_DRAWBACKGROUND, L"DRAWBACKGROUND"},
        {XUIGROUP_DEFAULT_W, L"DEFAULT_W"},
        {XUIGROUP_DEFAULT_H, L"DEFAULT_H"},
        {XUIGROUP_DESIGN_H, L"DESIGN_H"},
        {XUIGROUP_DESIGN_W, L"DESIGN_W"},
        {XUIGROUP_EMBED_XUI, L"EMBED_XUI"},
        {XUIGROUP_INHERIT_CONTENT, L"INHERIT_CONTENT"},
        {XUIGROUP_INHERIT_GROUP, L"INHERIT_GROUP"},
        {XUIGROUP_INSTANCEID, L"INSTANCEID"},
        {XUIGROUP_LOCKMINMAX, L"LOCKMINMAX"},
        {XUIGROUP_MAXIMUM_H, L"MAXIMUM_H"},
        {XUIGROUP_MAXIMUM_W, L"MAXIMUM_W"},
        {XUIGROUP_MINIMUM_H, L"MINIMUM_H"},
        {XUIGROUP_MINIMUM_W, L"MINIMUM_W"},
        {XUIGROUP_NAME, L"NAME"},
        {XUIGROUP_PROPAGATESIZE, L"PROPAGATESIZE"},
        {XUIGROUP_XUITAG, L"XUITAG"},
    };

Group::Group()
{
	scripts_enabled = 1;
	getScriptObject()->vcpu_setInterface(groupGuid, (void *)static_cast<Group *>(this));
	getScriptObject()->vcpu_setClassName(L"Group");
	getScriptObject()->vcpu_setController(groupController);
	background = NULL;
	skinpart = 0;
	captured = 0; resizing = 0;
	x = 0; y = 0;
	size_w = 0; size_h = 0;
	lockminmax = 0;
	propagatesize = 0;
	reg = NULL;
	default_h = AUTOWH;
	default_w = AUTOWH;
	//  allreg = NULL;
	//	subregionlayers = new PtrList<api_window>;
	//	subregiongroups = new PtrList<api_window>;
	deleting = 0;
	moving = 0;
	drawbackground = 0;
	groupmaxheight = AUTOWH;
	groupmaxwidth = AUTOWH;
	groupminheight = AUTOWH;
	groupminwidth = AUTOWH;
	//  regionop = 0;
	//  allsubreg = NULL;
	groups.addItem(this);
	scaledreg = NULL;
	scaledregionvalid = 0;
	autoregionop = 1;
	setRectRgn(0);
	disable_update_pos = 0;
	no_init_on_addchild = 0;
	lastheightsource = lastwidthsource = NULL;
	lastgetwidthbasedon = lastgetheightbasedon = AUTOWH;
	content_item = NULL;

	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);

	design_w = AUTOWH;
	design_h = AUTOWH;
}

void Group::CreateXMLParameters(int master_handle)
{
	//GROUP_PARENT::CreateXMLParameters(master_handle);
		int numParams = sizeof(groupParams) / sizeof(groupParams[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
	{
		addParam(xuihandle, groupParams[i], XUI_ATTRIBUTE_IMPLIED);
	}
}

Group::~Group()
{
	deleteScripts();
	deleting = 1;
	WASABI_API_WND->skin_unregisterBaseTextureWindow(this);
	while (gui_objects.getNumItems() > 0)
	{
		SkinParser::destroyGuiObject(gui_objects.enumItem(0));
		gui_objects.removeByPos(0);
	}
	delete background;
	delete reg;
	delete scaledreg;
	xuiparams.deleteAll();
	/*  subregionlayers->removeAll();
	  delete subregionlayers;
	  subregiongroups->removeAll();
	  delete subregiongroups;*/
	groups.removeItem(this);
	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<WndCallbackI*>(this));
}

int Group::isGroup(Group *o)
{
	return groups.haveItem(o);
}

int Group::setXmlParam(const wchar_t *paramname, const wchar_t *strvalue)
{
	if (!WCSICMP(paramname, L"id") && !instanceid.isempty())
		return GROUP_PARENT::setXmlParam(paramname, instanceid);
	return GROUP_PARENT::setXmlParam(paramname, strvalue);
}

int Group::setXuiParam(int _xuihandle, int xuiid, const wchar_t *paramname, const wchar_t *strvalue)
{
	if (xuihandle == _xuihandle)
	{
		switch (xuiid)
		{
		case XUIGROUP_INSTANCEID:
			instanceid = strvalue;
			getGuiObject()->guiobject_setId(instanceid);
			return 1;
		case XUIGROUP_BACKGROUND:
			setBaseTexture(strvalue);
			setDrawBackground(1);
			return 1;
		case XUIGROUP_DRAWBACKGROUND:
			setDrawBackground(WTOI(strvalue));
			return 1;
		case XUIGROUP_DEFAULT_W:
			{
				int w = WTOI(strvalue);
				//getGuiObject()->guiobject_setGuiPosition(NULL, NULL, &w, NULL, NULL, NULL, NULL, NULL);
				default_w = w;
				return 1;
			}
		case XUIGROUP_DEFAULT_H:
			{
				int h = WTOI(strvalue);
				//getGuiObject()->guiobject_setGuiPosition(NULL, NULL, NULL, &h, NULL, NULL, NULL, NULL);
				default_h = h;
				return 1;
			}

		case XUIGROUP_MAXIMUM_H:
			groupmaxheight = WTOI(strvalue);
			return 1;
		case XUIGROUP_MAXIMUM_W:
			groupmaxwidth = WTOI(strvalue);
			return 1;
		case XUIGROUP_MINIMUM_H:
			groupminheight = WTOI(strvalue);
			return 1;
		case XUIGROUP_MINIMUM_W:
			groupminwidth = WTOI(strvalue);
			return 1;
		case XUIGROUP_PROPAGATESIZE:
			propagatesize = WTOI(strvalue);
			return 1;
		case XUIGROUP_LOCKMINMAX:
			lockminmax = WTOI(strvalue);
			return 1;
		case XUIGROUP_NAME:
			setName(strvalue);
			return 1;
		case XUIGROUP_AUTOWIDTHSOURCE:
			setAutoWidthSource(strvalue);
			return 1;
		case XUIGROUP_AUTOHEIGHTSOURCE:
			setAutoHeightSource(strvalue);
			return 1;
		case XUIGROUP_EMBED_XUI:
			xui_embedded_id = strvalue;
			return 1;
		case XUIGROUP_XUITAG:
			return 1;
		case XUIGROUP_INHERIT_GROUP:
			return 1;
		case XUIGROUP_INHERIT_CONTENT:
			return 1;
		case XUIGROUP_DESIGN_W:
			setDesignWidth(WTOI(strvalue));
			return 1;
		case XUIGROUP_DESIGN_H:
			setDesignHeight(WTOI(strvalue));
			return 1;
		}
	}
	return GROUP_PARENT::setXuiParam(_xuihandle, xuiid, paramname, strvalue);
}

void Group::setDesignWidth(int w)
{
	design_w = w;
	if (isPostOnInit())
		onResize();
}

void Group::setDesignHeight(int h)
{
	design_h = h;
	if (isPostOnInit())
		onResize();
}

int Group::getDesignWidth()
{
	return design_w;
}

int Group::getDesignHeight()
{
	return design_h;
}

int Group::onPostedMove()
{
	return GROUP_PARENT::onPostedMove();
}

#ifdef WASABI_COMPILE_WNDMGR
void Group::beginMove()
{
	if (getGuiObject()->guiobject_getParentGroup())
		getGuiObject()->guiobject_getParentGroup()->beginMove();
}

void Group::beginScale()
{
	if (getGuiObject()->guiobject_getParentGroup())
		getGuiObject()->guiobject_getParentGroup()->beginScale();
}

void Group::beginResize()
{
	if (getGuiObject()->guiobject_getParentGroup())
		getGuiObject()->guiobject_getParentGroup()->beginResize();
}

void Group::endMove()
{
	if (getGuiObject()->guiobject_getParentGroup())
		getGuiObject()->guiobject_getParentGroup()->endMove();
}

void Group::endScale()
{
	if (getGuiObject()->guiobject_getParentGroup())
		getGuiObject()->guiobject_getParentGroup()->endScale();
}

void Group::endResize()
{
	if (getGuiObject()->guiobject_getParentGroup())
		getGuiObject()->guiobject_getParentGroup()->endResize();
}
#endif

void Group::onMinMaxEnforcerChanged()
{
	if (!isPostOnInit()) return ;
	int min_x = getPreferences(MINIMUM_W);
	int min_y = getPreferences(MINIMUM_H);
	int max_x = getPreferences(MAXIMUM_W);
	int max_y = getPreferences(MAXIMUM_H);
	int sug_x = getPreferences(SUGGESTED_W);
	int sug_y = getPreferences(SUGGESTED_H);
	min_x = MAX(RESIZE_MINW, min_x);
	min_y = MAX(RESIZE_MINH, min_y);

	RECT r;
	POINT pt;
	getClientRect(&r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	getPosition(&pt);
	if ((w < min_x || h < min_y || w > max_x || h > max_y) && (w != sug_x || h != sug_y))
	{
		//DebugString("reapplying minmax constraints\n");
		resize(pt.x, pt.y, sug_x, sug_y);
	}
}

#ifdef WASABI_COMPILE_WNDMGR
void Group::mouseResize(int x, int y, int resizeway)
{ // screen coords!

	int min_x = getPreferences(MINIMUM_W);
	int min_y = getPreferences(MINIMUM_H);
	int max_x = getPreferences(MAXIMUM_W);
	int max_y = getPreferences(MAXIMUM_H);
	int sug_x = getPreferences(SUGGESTED_W);
	int sug_y = getPreferences(SUGGESTED_H);

	if (max_x != AUTOWH && min_x != AUTOWH && max_x < min_x) max_x = min_x;
	if (max_y != AUTOWH && min_y != AUTOWH && max_y < min_y) max_y = min_y;
	if (min_x != AUTOWH && max_x != AUTOWH && min_x > max_x) min_x = max_x;
	if (min_y != AUTOWH && max_y != AUTOWH && min_y > max_y) min_y = max_y;
	if (sug_x != AUTOWH && min_x != AUTOWH && sug_x < min_x) sug_x = min_x;
	if (sug_y != AUTOWH && min_y != AUTOWH && sug_y < min_y) sug_y = min_y;
	if (sug_x != AUTOWH && max_x != AUTOWH && sug_x > max_x) sug_x = max_x;
	if (sug_y != AUTOWH && max_y != AUTOWH && sug_y > max_y) sug_y = max_y;

	beginResize();

	int mask = 0;
	if (resizeway & RESIZE_BOTTOM)
	{
		mask |= BOTTOM;
	}
	if (resizeway & RESIZE_RIGHT)
	{
		mask |= RIGHT;
	}
	if (resizeway & RESIZE_TOP)
	{
		mask |= TOP;
	}
	if (resizeway & RESIZE_LEFT)
	{
		mask |= LEFT;
	}

	min_x = MAX(RESIZE_MINW, min_x);
	min_y = MAX(RESIZE_MINH, min_y);

	if (renderRatioActive())
	{
		if (min_x != AUTOWH) multRatio(&min_x);
		if (min_y != AUTOWH) multRatio(NULL, &min_y);
		if (max_x != AUTOWH) multRatio(&max_x);
		if (max_y != AUTOWH) multRatio(NULL, &max_y);
		if (sug_x != AUTOWH) multRatio(&sug_x);
		if (sug_y != AUTOWH) multRatio(NULL, &sug_y);
	}

	if (min_x == AUTOWH) min_x = -1;
	if (max_x == AUTOWH) max_x = -1;
	if (min_y == AUTOWH) min_y = -1;
	if (max_y == AUTOWH) max_y = -1;
	if (sug_x == AUTOWH) sug_x = -1;
	if (sug_y == AUTOWH) sug_y = -1;

	resizeClass rsize(this, min_x, min_y, max_x, max_y, sug_x, sug_y);
	if (rsize.resizeWindow(this, mask | NOINTERSECT))
	{
		RECT r = rsize.getRect();

		if (renderRatioActive())
		{
			r.right = (int)(((double)(r.right - r.left) / getRenderRatio()) + r.left + 0.5f);
			r.bottom = (int)(((double)(r.bottom - r.top) / getRenderRatio()) + r.top + 0.5f);
		}

		int _min_x = getPreferences(MINIMUM_W);
		int _min_y = getPreferences(MINIMUM_H);
		int _max_x = getPreferences(MAXIMUM_W);
		int _max_y = getPreferences(MAXIMUM_H);

		if (_max_x != AUTOWH && _min_x != AUTOWH && _max_x < _min_x) _max_x = _min_x;
		if (_max_y != AUTOWH && _min_y != AUTOWH && _max_y < _min_y) _max_y = _min_y;
		if (_min_x != AUTOWH && _max_x != AUTOWH && _min_x > _max_x) _min_x = _max_x;
		if (_min_y != AUTOWH && _max_y != AUTOWH && _min_y > _max_y) _min_y = _max_y;

		if (r.right - r.left < _min_x) r.right = r.left + _min_x;
		if (r.bottom - r.top < _min_y) r.bottom = r.top + _min_y;
		if (r.right - r.left > _max_x) r.right = r.left + _max_x;
		if (r.bottom - r.top > _max_y) r.bottom = r.top + _max_y;

		resizeToRect(&r);
		invalidate();
		endResize();
	}
}
#endif

void Group::setAutoWidthSource(const wchar_t *obj)
{
	autowidthsource = obj;
}

void Group::setAutoHeightSource(const wchar_t *obj)
{
	autoheightsource = obj;
}

int Group::getAutoWidth()
{
	return default_w == AUTOWH ? GROUP_PARENT::getPreferences(SUGGESTED_W) : default_w;
}

int Group::getAutoHeight()
{
	return default_h == AUTOWH ? GROUP_PARENT::getPreferences(SUGGESTED_H) : default_h;
}

int Group::getWidthBasedOn(GuiObject *o)
{
	if (o == NULL)
	{
		if (lastwidthsource == NULL)
		{
			if (!autowidthsource.isempty())
			{
				lastwidthsource = getObject(autowidthsource);
			}
		}
		o = lastwidthsource;
	}
	if (o == NULL) return AUTOWH;
	if (lastgetwidthbasedon != AUTOWH) return lastgetwidthbasedon;
	int x, rx, w, rw;
	o->guiobject_getGuiPosition(&x, NULL, &w, NULL, &rx, NULL, &rw, NULL);
	int p = o->guiobject_getAutoWidth();
if (w == AUTOWH) { w = p; rw = 0; }
	if (rx == 0 && rw == 1)
		lastgetwidthbasedon = p - w;
	else if (rx == 0 && rw == 0)
		lastgetwidthbasedon = p + x;
	else
		lastgetwidthbasedon = AUTOWH;

	return lastgetwidthbasedon;
}

int Group::getHeightBasedOn(GuiObject *o)
{
	if (o == NULL)
	{
		if (lastheightsource == NULL)
		{
			if (!autoheightsource.isempty())
			{
				lastheightsource = getObject(autoheightsource);
			}
		}
		o = lastheightsource;
	}
	if (o == NULL) return AUTOWH;
	if (lastgetheightbasedon != AUTOWH) return lastgetheightbasedon;
	int y, ry, h, rh;
	o->guiobject_getGuiPosition(NULL, &y, NULL, &h, NULL, &ry, NULL, &rh);
	int p = o->guiobject_getAutoHeight();
if (h == AUTOWH) { h = p; rh = 0; }
	if (ry == 0 && rh == 1)
		lastgetheightbasedon = p - h;
	else if (ry == 0 && rh == 0)
		lastgetheightbasedon = h + y;
	else
		lastgetheightbasedon = AUTOWH;

	return lastgetheightbasedon;
}

int Group::getPreferences(int what)
{
	int _what = what;
	if (lockminmax)
	{
		if (_what == MAXIMUM_W || _what == MINIMUM_W)
			_what = SUGGESTED_W;
		if (_what == MAXIMUM_H || _what == MINIMUM_H)
			_what = SUGGESTED_H;
	}
	switch (_what)
	{
	case SUGGESTED_W:
		{
			int w, rw;
			getGuiObject()->guiobject_getGuiPosition(NULL, NULL, &w, NULL, NULL, NULL, &rw, NULL);

			if (w == AUTOWH)
				w = getWidthBasedOn();

			if (w == AUTOWH || rw == 1)
				w = getAutoWidth();

			if (w == AUTOWH && getBaseTexture())
				w = getBaseTexture()->getWidth();

			if (groupmaxwidth != AUTOWH)
			{
				if (groupminwidth != AUTOWH)
				{
					return MIN(groupmaxwidth, MAX(groupminwidth, w));
				}
				else
				{
					return MIN(groupmaxwidth, w);
				}
			}
			else if (groupminwidth != AUTOWH)
			{
				return MAX(groupminwidth, w);
			}
			return w;
		}
	case SUGGESTED_H:
		{
			int h, rh;
			getGuiObject()->guiobject_getGuiPosition(NULL, NULL, NULL, &h, NULL, NULL, NULL, &rh);

			if (h == AUTOWH)
				h = getHeightBasedOn();

			if (h == AUTOWH || rh == 1)
				h = getAutoHeight();

			if (h == AUTOWH && getBaseTexture())
				h = getBaseTexture()->getHeight();

			if (groupmaxheight != AUTOWH)
			{
				if (groupminheight != AUTOWH)
				{
					return MIN(groupmaxheight, MAX(groupminheight, h));
				}
				else
				{
					return MIN(groupmaxheight, h);
				}
			}
			else if (groupminheight != AUTOWH)
			{
				return MAX(groupminheight, h);
			}
			return h;
		}
	case MAXIMUM_H:
		{
			int h = GROUP_PARENT::getPreferences(what);

			if (h != AUTOWH)
				return MIN(h, groupmaxheight);

			return groupmaxheight;
		}
	case MAXIMUM_W:
		{
			int w = GROUP_PARENT::getPreferences(what);

			if (w != AUTOWH)
				return MIN(w, groupmaxwidth);

			return groupmaxwidth;
		}
	case MINIMUM_H:
		{
			int h = GROUP_PARENT::getPreferences(what);

			if (h != AUTOWH)
				return MAX(h, groupminheight);

			return groupminheight;
		}
	case MINIMUM_W:
		{
			int w = GROUP_PARENT::getPreferences(what);

			if (w != AUTOWH)
				return MAX(w, groupminwidth);

			return groupminwidth;
		}
	}
	return GROUP_PARENT::getPreferences(what);
}

void Group::updatePos(GuiObject *o, RECT *r2)
{

	if (disable_update_pos) return ;

	RECT r;
	if (r2 == NULL)
	{
		getClientRect(&r);
		r2 = &r;
	}


	double d = getRenderRatio();
	int w, h;
	int ox, oy, ow, oh, orx, ory, orw, orh;
	int ox1, ox2, oy1, oy2, oanchor;

	if (o->guiobject_getAnchoragePosition(&ox1, &oy1, &ox2, &oy2, &oanchor))
	{
		// anchorage values have not been translated into native values yet, do it now
		int x, y, w, h, rx, ry, rw, rh;
		x = y = w = h = rx = ry = rw = rh = AUTOWH;
		int lw = ox2 - ox1;
		int lh = oy2 - oy1;
		int iw = getDesignWidth();
		int ih = getDesignHeight();
		if (iw == AUTOWH || ih == AUTOWH)
		{
			Wasabi::Std::messageBox(L"anchor coordinate system used without design size for the parent group.\nYour parent group needs the design_w/design_h parameters if you are using x1/y1/x2/y2/anchor parameters on one of its children\nDefaulting to 320x200", L"XML Error", 0);
			iw = 320;
			ih = 200;
		}
		int right_m = iw - ox2;
		int bottom_m = ih - oy2;

		if ((oanchor & ANCHOR_LEFT) == 0 && (oanchor & ANCHOR_RIGHT) == 0) oanchor |= ANCHOR_LEFT;
		if ((oanchor & ANCHOR_TOP) == 0 && (oanchor & ANCHOR_BOTTOM) == 0) oanchor |= ANCHOR_TOP;

		if (oanchor & ANCHOR_LEFT)
		{
			x = ox1;
			rx = 0;
			if (oanchor & ANCHOR_RIGHT)
			{
				w = -((iw - ox2) + ox1);
				rw = 1;
			}
			else
			{
				w = lw;
				rw = 0;
			}
		}
		else
		{
			if (oanchor & ANCHOR_RIGHT)
			{
				x = -(right_m + lw);
				rx = 1;
				w = lw;
				rw = 0;
			}
		}
		if (oanchor & ANCHOR_TOP)
		{
			y = oy1;
			ry = 0;
			if (oanchor & ANCHOR_BOTTOM)
			{
				h = -((ih - oy2) + oy1);
				rh = 1;
			}
			else
			{
				h = lh;
				rh = 0;
			}
		}
		else
		{
			if (oanchor & ANCHOR_BOTTOM)
			{
				y = -(bottom_m + lh);
				ry = 1;
				h = lh;
				rh = 0;
			}
		}
		disable_update_pos = 1;
		o->guiobject_setGuiPosition(&x, &y, &w, &h, &rx, &ry, &rw, &rh);
		o->guiobject_validateAnchorage();
		disable_update_pos = 0;
	}

	o->guiobject_getGuiPosition(&ox, &oy, &ow, &oh, &orx, &ory, &orw, &orh);
	if (ow == AUTOWH) { ow = o->guiobject_getAutoWidth(); orw = 0; }
	if (oh == AUTOWH) { oh = o->guiobject_getAutoHeight(); orh = 0; }

	TextInfoCanvas fontInfoCanvas(this);
	double fontScale = fontInfoCanvas.getSystemFontScale();
	if (o->guiobject_getAutoSysMetricsX())
		ox = (int)((float)ox * fontScale);

	if (o->guiobject_getAutoSysMetricsY())
		oy = (int)((float)oy * fontScale);

	if (o->guiobject_getAutoSysMetricsW())
		ow = (int)((float)ow * fontScale);

	if (o->guiobject_getAutoSysMetricsH())
		oh = (int)((float)oh * fontScale);

	if (!o->guiobject_getRootWnd()->handleRatio())
	{
		if (orw == 1)
			w = (int)((float)(r2->right - r2->left + ow) * d);
		else if (orw == 2)
			w = (int)(((float)(r2->right - r2->left) * ((float)ow / 100.0f)) * d);
		else
			w = (int)((float)(ow) * d);
		if (orh == 1)
			h = (int)((float)(r2->bottom - r2->top + oh) * d);
		else if (orh == 2)
			h = (int)(((float)(r2->bottom - r2->top) * ((float)oh / 100.0f)) * d);
		else
			h = (int)((float)(oh) * d);
		if (orx == 1)
			x = (int)((float)(r2->right - r2->left + ox) * d);
		else if (orx == 2)
			x = (int)(((float)(r2->right - r2->left) * ((float)ox / 100.0f)) * d);
		else
			x = (int)((float)(ox) * d);
		if (ory == 1)
			y = (int)((float)(r2->bottom - r2->top + oy) * d);
		else if (ory == 2)
			y = (int)(((float)(r2->bottom - r2->top) * ((float)oy / 100.0f)) * d);
		else
			y = (int)((float)(oy) * d);
		x += (int)((float)(r2->left) * d);
		y += (int)((float)(r2->top) * d);
	}
	else
	{
		if (orw == 1)
			w = r2->right - r2->left + ow;
		else if (orw == 2)
			w = (int)((float)(r2->right - r2->left) * ((float)ow / 100.0f));
		else
			w = ow;
		if (orh == 1)
			h = r2->bottom - r2->top + oh;
		else if (orh == 2)
			h = (int)((float)(r2->bottom - r2->top) * ((float)oh / 100.0f));
		else
			h = oh;
		if (orx == 1)
			x = r2->right - r2->left + ox;
		else if (orx == 2)
			x = (int)((float)(r2->right - r2->left) * ((float)ox / 100.0f));
		else
			x = ox;
		if (ory == 1)
			y = r2->bottom - r2->top + oy;
		else if (ory == 2)
			y = (int)((float)(r2->bottom - r2->top) * ((float)oy / 100.0f));
		else
			y = oy;
		x += r2->left;
		y += r2->top;
	}

	o->guiobject_getRootWnd()->resize(x, y, w, h);
}


int Group::onResize()
{
	GROUP_PARENT::onResize();
	if (!isInited()) return 1;

	RECT wr;
	getWindowRect(&wr);
	RECT r2;
	getClientRect(&r2);
	size_w = r2.right - r2.left; size_h = r2.bottom - r2.top;
	for (int i = 0;i < gui_objects.getNumItems();i++)
	{
		GuiObject *o = gui_objects.enumItem(i);
		updatePos(o, &r2);
	}
	invalidateScaledReg();
	return 1;
}

void Group::invalidateScaledReg()
{
	scaledregionvalid = 0;
	invalidateWindowRegion();
}

int Group::onInit()
{
	GROUP_PARENT::onInit();

	WASABI_API_SYSCB->syscb_registerCallback(static_cast<WndCallbackI*>(this));

	disable_update_pos = 1;
	no_init_on_addchild = 1;

	const wchar_t *id = getGroupContentId();
	SkinItem *item = getGroupContentSkinItem();
	if ((id && *id) || item != NULL)
	{
		SkinParser::fillGroup(this, id, item, 0, 1, scripts_enabled);
	}

	disable_update_pos = 0;
	no_init_on_addchild = 0;

	onFillGroup();

	return 1;
}

void Group::onFillGroup()
{
	reloadDefaults();
	if (xui_embedded_id.len() > 0)
		embeddedxui_onNewEmbeddedContent();
}

int Group::onGroupChange(const wchar_t *id)
{
	GROUP_PARENT::onGroupChange(id);

	if (!isInited()) return 1;
	const wchar_t *cid = getGroupContentId();
	SkinItem *item = getGroupContentSkinItem();
	if (!(cid && *cid) || item != NULL) return 1;

	if (!WCSICMP(id, cid)) return 1;

	deleteScripts();

	WASABI_API_WND->skin_unregisterBaseTextureWindow(this);
	while (gui_objects.getNumItems() > 0)
	{
		SkinParser::destroyGuiObject(gui_objects.enumItem(0));
	}

	delete background; background = NULL;
	delete reg; reg = NULL;
	delete scaledreg; scaledreg = NULL;

	disable_update_pos = 1;
	no_init_on_addchild = 1;

	SkinParser::fillGroup(this, cid, item, 0, 1, scripts_enabled);

	disable_update_pos = 0;
	no_init_on_addchild = 0;

	onFillGroup();
	if (isInited())
		onResize();

	getGuiObject()->guiobject_onStartup();

	return 1;
}

void Group::reloadDefaults()
{
	getGuiObject()->guiobject_getGuiPosition(NULL, NULL, &size_w, &size_h, NULL, NULL, NULL, NULL);
	if (size_w == AUTOWH && size_h == AUTOWH)
	{
		/*    if (!background) {
		      size_w = 250;
		      size_h = 100;
		    } else {
		      size_w = background->getWidth();
		      size_h = background->getHeight();
		    }*/
		if (background)
		{
			setPreferences(SUGGESTED_W, background->getWidth());
			setPreferences(SUGGESTED_H, background->getHeight());
		}
		size_w = GROUP_PARENT::getPreferences(SUGGESTED_W);
		size_h = GROUP_PARENT::getPreferences(SUGGESTED_H);
	}
	//setName(getGuiObject()->guiobject_getId());

	if (propagatesize)
	{
		Layout *l = getGuiObject()->guiobject_getParentLayout();
		if (l) l->addMinMaxEnforcer(this);
	}

	int i;
	for (i = 0;i < gui_objects.getNumItems();i++)
	{

		GuiObject *o = gui_objects.enumItem(i);

		if (o->guiobject_getParentGroup())
		{
			o->guiobject_getRootWnd()->setParent(o->guiobject_getParentGroup());
			if (!o->guiobject_getRootWnd()->isInited())
			{
				o->guiobject_getRootWnd()->init(o->guiobject_getParentGroup());
				o->guiobject_getParentGroup()->onCreateObject(getGuiObject());
			}
		}
	}

	/*  if (getGuiObject()->guiobject_getParentGroup() && regionop)
	    getGuiObject()->guiobject_getParentGroup()->addSubRegionGroup(this);*/

	invalidateWindowRegion();

	startScripts();

	for (i = 0;i < gui_objects.getNumItems();i++)
	{
		GuiObject *o = gui_objects.enumItem(i);
		o->guiobject_onStartup();
	}

	autoResize();

	const wchar_t *name = getName();
	if (name != NULL)
		onSetName();
}

void Group::startScripts()
{
	for (int i = 0;i < getNumScripts();i++)
	{
		int vcpuid = enumScript(i);
		SystemObject *o = SOM::getSystemObjectByScriptId(vcpuid);
		if (o != NULL)
		{
			o->onLoad();
			foreach(xuiparams)
			XuiParam *p = xuiparams.getfor();
			o->onSetXuiParam(p->param, p->value);
			endfor;
		}
	}
}

int Group::onPaint(Canvas *canvas)
{
	PaintCanvas paintcanvas;
	if (canvas == NULL)
	{
		if (!paintcanvas.beginPaint(this)) return 0;
		canvas = &paintcanvas;
	}
	GROUP_PARENT::onPaint(canvas);
	RECT r;
	if (!canvas->getClipBox(&r))
		getClientRect(&r);


	if (getDrawBackground()) RenderBaseTexture(canvas, r);
	return 1;
}

ifc_window *Group::getBaseTextureWindow()
{
	if (reg && !reg->isEmpty()) return this;
	if (getParent()) return getParent()->getBaseTextureWindow();
	return this;
}

void Group::autoResize()
{
	int w = getWidthBasedOn();
	int h = getHeightBasedOn();
	if (h == AUTOWH && w == AUTOWH) return ;
	resize(NOCHANGE, NOCHANGE, w == AUTOWH ? NOCHANGE : w, h == AUTOWH ? NOCHANGE : h);
}

int Group::childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2)
{
	if (msg == ChildNotify::AUTOWHCHANGED)
	{
		if (lastwidthsource && child == lastwidthsource->guiobject_getRootWnd() ||
		    lastheightsource && child == lastheightsource->guiobject_getRootWnd())
		{
			lastgetwidthbasedon = AUTOWH;
			lastgetheightbasedon = AUTOWH;
			autoResize();
		}
	}
	if (!getParent())
		return 0;
	return getParent()->childNotify(child, msg, p1, p2);
}


void Group::sendNotifyToAllChildren(int notifymsg, intptr_t param1, intptr_t param2)
{
	for (int i = 0;i < gui_objects.getNumItems();i++)
		gui_objects.enumItem(i)->guiobject_getRootWnd()->childNotify(this, notifymsg, param1, param2);
}

void Group::addScript(int scriptid)
{
	scripts.addItem(scriptid);
}

int Group::getNumScripts()
{
	return scripts.getNumItems();
}

int Group::enumScript(int n)
{
	return scripts.enumItem(n);
}

void Group::deleteScripts()
{
	for (int i = 0;i < scripts.getNumItems();i++)
		Script::unloadScript(scripts.enumItem(i));
	scripts.removeAll();
}

SkinBitmap *Group::getBaseTexture()
{
	if (!background) return NULL;
	return background->getBitmap();
}

void Group::setBaseTexture(const wchar_t *b, int regis)
{
	backgroundstr = b;
	if (regis) WASABI_API_WND->skin_unregisterBaseTextureWindow(this);
	delete background;
	background = NULL;
	if (b != NULL && *b)
	{
		background = new AutoSkinBitmap();
		background->setBitmap(b);
#ifdef _WIN32
		RegionI r(background->getBitmap());
		setRegion(&r);
#else
#warning port me!
#endif

		if (regis) WASABI_API_WND->skin_registerBaseTextureWindow(this, b);
	}
	else
	{
		delete reg;
		reg = NULL;
		delete scaledreg;
		scaledreg = NULL;
		invalidateWindowRegion();
	}

}

const wchar_t *Group::getBackgroundStr()
{
	return backgroundstr;
}

int Group::onUnknownXuiParam(const wchar_t *xmlattributename, const wchar_t *value)
{
	int r = GROUP_PARENT::onUnknownXuiParam(xmlattributename, value);
	if (r == 0)
	{
		if (!isInited())
		{
			xuiparams.addItem(new XuiParam(xmlattributename, value));
		}
		else
		{
			for (int i = 0;i < getNumScripts();i++)
			{
				int vcpuid = enumScript(i);
				SystemObject *o = SOM::getSystemObjectByScriptId(vcpuid);
				if (o != NULL) o->onSetXuiParam(xmlattributename, value);
			}
		}
	}
	return r;
}

api_region *Group::getRegion()
{
	ensureScaledRegValid();
	return scaledreg;
}

void Group::ensureScaledRegValid()
{
	if (!scaledregionvalid)
	{
		scaledregionvalid = 1;
		if (!reg) return ;
		if (!scaledreg) scaledreg = new RegionI;
		scaledreg->empty();
		scaledreg->addRegion(reg);
		RECT rr;
		getNonClientRect(&rr);
		if (background)
		{
			float w = (float)(rr.right - rr.left) / (float)background->getWidth();
			float h = (float)(rr.bottom - rr.top) / (float)background->getHeight();
			if (w && h && (ABS(w - 1.0f) > 0.01f || ABS(h - 1.0f) > 0.01f))
				scaledreg->scale(w, h, 1);
			else if (w == 0 || h == 0)
			{
				RECT r = {0, 0, 0, 0};
				scaledreg->setRect(&r);
			}
		}
	}
}

void Group::setRegion(api_region *r)
{
	ASSERT(r != NULL);
	if (!reg) reg = new RegionI;

	reg->empty();
	reg->addRegion(r); // background

	invalidateWindowRegion();
	invalidateScaledReg();
}

Container *Group::getParentContainer()
{
	if (!getGuiObject()->guiobject_getParentGroup())
	{
		ifc_window *r = getDesktopParent();
		if (r != NULL)
		{
			//ASSERT(r != NULL);
			Layout *l = static_cast<Layout *>(r->getInterface(layoutGuid));
			if (!l) return NULL;
			return l->getParentContainer();
		}
	}
	Group *g = getGuiObject()->guiobject_getParentGroup();
	if (g) return g->getParentContainer();
	return NULL;
}

ifc_window *Group::enumObjects(int i)
{
	if (i >= gui_objects.getNumItems()) return NULL;
	return gui_objects.enumItem(i)->guiobject_getRootWnd();
}

int Group::getNumObjects()
{
	return gui_objects.getNumItems();
}

void Group::addChild(GuiObject *g)
{
	gui_objects.addItem(g);
	script_objects.addItem((ScriptObject *)g);
	g->guiobject_setParentGroup(this);
	g->guiobject_getRootWnd()->setParent(this);
	if (!no_init_on_addchild && isInited()
		&& !g->guiobject_getRootWnd()->isInited()) 
		g->guiobject_getRootWnd()->init(this);
}

void Group::removeChild(GuiObject *g)
{
	if (!deleting)
		gui_objects.removeItem(g);
	script_objects.removeItem((ScriptObject *)g);
}

void Group::onCreateObject(GuiObject *o)
{
	script_vcpu_onCreateObject(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_OBJECT(o->guiobject_getScriptObject()));
}

int Group::getDrawBackground()
{
	return drawbackground;
}

void Group::setDrawBackground(int t)
{
	drawbackground = t;
}

int Group::isLayout()
{
	return 0;
}

int Group::isDesktopAlphaSafe()
{
	if (!Wasabi::Std::Wnd::isDesktopAlphaAvailable()) return 0;
	for (int i = 0;i < getNumObjects();i++)
	{
		ifc_window *w = enumObjects(i);
		Group *cg = static_cast<Group *>(w);
		if (Group::isGroup(cg))
			if (!cg->isDesktopAlphaSafe()) return 0;
		if (!w->handleDesktopAlpha()) return 0;
	}
	return 1;
}

int Group::isTransparencySafe(int excludeme)
{
	if (!Wasabi::Std::Wnd::isTransparencyAvailable()) return 0;
	if (!excludeme && isTransparencyForcedOff()) return 0;
	for (int i = 0;i < getNumObjects();i++)
	{
		ifc_window *w = enumObjects(i);
		Group *cg = static_cast<Group *>(w);
		if (Group::isGroup(cg))
			if (!cg->isTransparencySafe()) return 0;
		if (!w->handleTransparency()) return 0;
	}
	return 1;
}

GuiObject *Group::getObject(const wchar_t *id)
{
	for (int i = 0;i < script_objects.getNumItems();i++)
	{
		if (!WCSICMP(id, gui_objects.enumItem(i)->guiobject_getId()))
		{
			return gui_objects.enumItem(i);
		}
	}
	return NULL;
}

void Group::setGroupContent(const wchar_t *id, SkinItem *item, int allowscripts)
{
	content_id = id;
	content_item = item;
	scripts_enabled = allowscripts;
}

const wchar_t *Group::getGroupContentId()
{
	return content_id;
}

SkinItem *Group::getGroupContentSkinItem()
{
	return content_item;
}

ScriptObject *Group::script_cast(GUID g)
{
	GuiObject *o = embeddedxui_getEmbeddedObject();
	if (o != NULL)
	{
		void *r = o->guiobject_getScriptObject()->vcpu_getInterface(g);
		if (r != NULL)
		{
			return o->guiobject_getScriptObject();
		}
	}
	return NULL;
}

// -----------------------------------------------------------------------

GroupScriptController _groupController;
GroupScriptController *groupController = &_groupController;

// -- Functions table -------------------------------------
function_descriptor_struct GroupScriptController::exportedFunction[] = {
            {L"getObject", 1, (void*)Group::script_vcpu_getObject },
            {L"enumObject", 1, (void*)Group::script_vcpu_enumObject },
            {L"getNumObjects", 0, (void*)Group::script_vcpu_getNumObjects },
            {L"onCreateObject", 1, (void*)Group::script_vcpu_onCreateObject },
            {L"getMousePosX", 0, (void*)Group::script_vcpu_getMousePosX },
            {L"getMousePosY", 0, (void*)Group::script_vcpu_getMousePosY },
            {L"isLayout", 0, (void*)Group::script_vcpu_isLayout },
            {L"autoResize", 0, (void*)Group::script_vcpu_autoResize},
        };
// --------------------------------------------------------

ScriptObject *GroupScriptController::cast(ScriptObject *o, GUID g)
{
	Group *grp = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	if (grp != NULL)
		return grp->script_cast(g);
	return NULL;
}

const wchar_t *GroupScriptController::getClassName()
{
	return L"Group";
}

const wchar_t *GroupScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

int GroupScriptController::getInstantiable()
{
	return 1;
}

ScriptObject *GroupScriptController::instantiate()
{
	return NULL;
}

void GroupScriptController::destroy(ScriptObject *o)
{}

void *GroupScriptController::encapsulate(ScriptObject *o)
{
	return NULL;
}

void GroupScriptController::deencapsulate(void *o)
{}

int GroupScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *GroupScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID GroupScriptController::getClassGuid()
{
	return groupGuid;
}


const wchar_t *Group::vcpu_getClassName()
{
	return L"Group";
}


void Group::addObject(GuiObject *o)
{
	addChild(o);
}

void Group::removeObject(GuiObject *o)
{
	removeChild(o);
}

void Group::setRegionOp(int o)
{
	GROUP_PARENT::setRegionOp(o);
	autoregionop = 0;
}

void Group::invalidateWindowRegion()
{
	GROUP_PARENT::invalidateWindowRegion();
	if (!isInited() || isLayout()) return ;
	if (autoregionop)
	{
		int yes = 0;
		for (int i = 0;i < gui_objects.getNumItems();i++)
		{
			if (gui_objects.enumItem(i)->guiobject_getRegionOp() != REGIONOP_NONE)
			{
				yes = 1;
				break;
			}
		}
		if (yes)
			GROUP_PARENT::setRegionOp(REGIONOP_OR);
		else
			GROUP_PARENT::setRegionOp(REGIONOP_NONE);
	}
}

// VCPU

scriptVar Group::script_vcpu_onCreateObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar ob)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, groupController, ob);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, ob);
}

// Get an object from its ID
scriptVar Group::script_vcpu_getObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(obj.type == SCRIPT_STRING); // compiler discarded
	Group *g = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	GuiObject *ob = NULL;
	if (g)
		ob = g->getObject(GET_SCRIPT_STRING(obj));
	return MAKE_SCRIPT_OBJECT(ob ? ob->guiobject_getScriptObject() : NULL);
}

scriptVar Group::script_vcpu_getMousePosX(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Group *g = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	if (g)
	{
		POINT pt;
		Wasabi::Std::getMousePos(&pt);
		g->screenToClient(&pt);
		return MAKE_SCRIPT_INT(pt.x);
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar Group::script_vcpu_getMousePosY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Group *g = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	if (g)
	{
		POINT pt;
		Wasabi::Std::getMousePos(&pt);
		g->screenToClient(&pt);
		return MAKE_SCRIPT_INT(pt.y);
	}
	RETURN_SCRIPT_ZERO;
}

scriptVar Group::script_vcpu_getNumObjects(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Group *g = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	if (g) return MAKE_SCRIPT_INT(g->getNumObjects());
	RETURN_SCRIPT_ZERO;
}

scriptVar Group::script_vcpu_enumObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar i)
{
	SCRIPT_FUNCTION_INIT
	Group *g = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	GuiObject *obj = NULL;
	if (g)
		obj = g->gui_objects.enumItem(GET_SCRIPT_INT(i));
	return MAKE_SCRIPT_OBJECT(obj ? obj->guiobject_getScriptObject() : NULL);
}

scriptVar Group::script_vcpu_isLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Group *g = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	return MAKE_SCRIPT_BOOLEAN(g ? g->isLayout() : NULL);
}

scriptVar Group::script_vcpu_autoResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Group *g = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	if (g != NULL) g->autoResize();
	RETURN_SCRIPT_VOID
}

#ifdef WASABI_COMPILE_CONFIG

int Group::isCfgGroup(Group *ptr)
{
	CfgGroup *g = static_cast<CfgGroup *>(ptr);
	return cfggrouplist.haveItem(g);
}

#endif

PtrList<CfgGroup> Group::cfggrouplist;
PtrList<Group> Group::groups;

// ---------------------------------------------------------------
// Config Groups


#ifdef WASABI_COMPILE_CONFIG

CfgGroup::CfgGroup()
{
	getScriptObject()->vcpu_setInterface(cfgGroupGuid, (void *)static_cast<CfgGroup *>(this));
	getScriptObject()->vcpu_setClassName(L"CfgGroup");
	getScriptObject()->vcpu_setController(cfgGroupController);
	cfgitem = NULL;
	cfggrouplist.addItem(this);
}

CfgGroup::~CfgGroup()
{
	if (cfgitem) viewer_delViewItem(cfgitem);
	cfggrouplist.removeItem(this);
}

const wchar_t *CfgGroup::vcpu_getClassName()
{
	return L"CfgGroup";
}

void CfgGroup::setAttr(CfgItem *item, const wchar_t *name)
{
	if (cfgitem) viewer_delViewItem(cfgitem);
	cfgitem = item;
	attrname = name;
	if (cfgitem != NULL)
	{
		wchar_t t[256] = L"";
		nsGUID::toCharW(cfgitem->getGuid(), t);
		cfgguid = t;
	}
	if (cfgitem) viewer_addViewItem(cfgitem);
	dataChanged();
}

int CfgGroup::viewer_onEvent(CfgItem *item, int event, intptr_t param, void *ptr, size_t ptrlen)
{
	dataChanged();
	return 1;
}

int CfgGroup::onInit()
{
	int r = Group::onInit();
	dataChanged();
	return r;
}

void CfgGroup::dataChanged()
{
	script_vcpu_onCfgChanged(SCRIPT_CALL, getScriptObject());
}

CfgItem *CfgGroup::getCfgItem()
{
	return cfgitem;
}

const wchar_t *CfgGroup::getAttributeName()
{
	return attrname;
}

scriptVar CfgGroup::script_vcpu_cfgGetInt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	CfgGroup *cg = static_cast<CfgGroup *>(o->vcpu_getInterface(cfgGroupGuid));
	if (cg)
	{
		CfgItem *i = cg->getCfgItem();
		const wchar_t *attrname = cg->getAttributeName();
		if (o != NULL && attrname != NULL)
			return MAKE_SCRIPT_INT(i->getDataAsInt(attrname, 0));
	}
	return MAKE_SCRIPT_INT(0);
}

scriptVar CfgGroup::script_vcpu_cfgSetInt(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&v));
	CfgGroup *cg = static_cast<CfgGroup *>(o->vcpu_getInterface(cfgGroupGuid));
	if (cg)
	{
		CfgItem *i = cg->getCfgItem();
		const wchar_t *attrname = cg->getAttributeName();
		if (o != NULL && attrname != NULL)
		{
			i->setDataAsInt(attrname, GET_SCRIPT_INT(v));
		}
	}
	RETURN_SCRIPT_VOID;
}

scriptVar CfgGroup::script_vcpu_cfgGetString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	CfgGroup *cg = static_cast<CfgGroup *>(o->vcpu_getInterface(cfgGroupGuid));
	if (cg)
	{
		CfgItem *i = cg->getCfgItem();
		const wchar_t *a = cg->getAttributeName();
		*txt = 0;
		if (!i || !a) 
			return MAKE_SCRIPT_STRING(txt);
		if (o != NULL && a != NULL)
		{
			i->getData(a, txt, 512);
		}
		return MAKE_SCRIPT_STRING(txt);
	}
	return MAKE_SCRIPT_STRING(L"");
}

scriptVar CfgGroup::script_vcpu_cfgSetString(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(!SOM::isNumeric(&v));
	CfgGroup *cg = static_cast<CfgGroup *>(o->vcpu_getInterface(cfgGroupGuid));
	if (cg)
	{
		CfgItem *i = cg->getCfgItem();
		const wchar_t *a = cg->getAttributeName();
		if (!i || !a) RETURN_SCRIPT_VOID;
		if (o != NULL && a != NULL)
		{
			i->setData(a, GET_SCRIPT_STRING(v));
		}
	}
	RETURN_SCRIPT_VOID;
}

scriptVar CfgGroup::script_vcpu_cfgGetFloat(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	CfgGroup *cg = static_cast<CfgGroup *>(o->vcpu_getInterface(cfgGroupGuid));
	if (cg)
	{
		CfgItem *i = cg->getCfgItem();
		const wchar_t *attrname = cg->getAttributeName();
		if (o != NULL && attrname != NULL)
			return MAKE_SCRIPT_FLOAT((float)i->getDataAsFloat(attrname, 0));
	}
	return MAKE_SCRIPT_FLOAT(0);
}

scriptVar CfgGroup::script_vcpu_cfgSetFloat(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(SOM::isNumeric(&v));
	CfgGroup *cg = static_cast<CfgGroup *>(o->vcpu_getInterface(cfgGroupGuid));
	if (cg)
	{
		CfgItem *i = cg->getCfgItem();
		const wchar_t *attrname = cg->getAttributeName();
		if (o != NULL && attrname != NULL)
		{
			i->setDataAsFloat(attrname, GET_SCRIPT_FLOAT(v));
		}
	}
	RETURN_SCRIPT_VOID;
}

scriptVar CfgGroup::script_vcpu_cfgGetName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	CfgGroup *cg = static_cast<CfgGroup *>(o->vcpu_getInterface(cfgGroupGuid));

	if (cg) 
	{
		return MAKE_SCRIPT_STRING(cg->getAttributeName()); 
	}
	return MAKE_SCRIPT_STRING(L"");
}

scriptVar CfgGroup::script_vcpu_cfgGetGuid(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	CfgGroup *cg = static_cast<CfgGroup *>(o->vcpu_getInterface(cfgGroupGuid));
	if (cg) 
		return MAKE_SCRIPT_STRING(cg->getCfgGuid()); 
	return MAKE_SCRIPT_STRING(L"");
}


scriptVar CfgGroup::script_vcpu_onCfgChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, cfgGroupController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

CfgGroupScriptController _cfgGroupController;
CfgGroupScriptController *cfgGroupController = &_cfgGroupController;

// -- Functions table -------------------------------------
function_descriptor_struct CfgGroupScriptController::exportedFunction[] = {
            {L"cfgGetInt", 0, (void*)CfgGroup::script_vcpu_cfgGetInt },
            {L"cfgSetInt", 1, (void*)CfgGroup::script_vcpu_cfgSetInt },
            {L"cfgGetFloat", 0, (void*)CfgGroup::script_vcpu_cfgGetFloat },
            {L"cfgSetFloat", 1, (void*)CfgGroup::script_vcpu_cfgSetFloat },
            {L"cfgGetString", 0, (void*)CfgGroup::script_vcpu_cfgGetString },
            {L"cfgSetString", 1, (void*)CfgGroup::script_vcpu_cfgSetString },
            {L"onCfgChanged", 0, (void*)CfgGroup::script_vcpu_onCfgChanged },
            {L"cfgGetName", 0, (void*)CfgGroup::script_vcpu_cfgGetName },
            {L"cfgGetGuid", 0, (void*)CfgGroup::script_vcpu_cfgGetGuid},
        };

/*SET_HIERARCHY(Group, SCRIPT_LAYOUT);
SET_HIERARCHY2(Group, SCRIPT_LAYOUT, LAYOUT_SCRIPTPARENT);*/

const wchar_t *CfgGroupScriptController::getClassName()
{
	return L"CfgGroup";
}

const wchar_t *CfgGroupScriptController::getAncestorClassName()
{
	return L"Group";
}

int CfgGroupScriptController::getInstantiable()
{
	return 1;
}

ScriptObject *CfgGroupScriptController::instantiate()
{
	return NULL;
}

int CfgGroupScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *CfgGroupScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID CfgGroupScriptController::getClassGuid()
{
	return cfgGroupGuid;
}

wchar_t CfgGroup::txt[512] = L"";

#endif // config
