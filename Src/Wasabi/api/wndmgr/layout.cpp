#include <precomp.h>
#include <api/skin/widgets/mb/scriptbrowser.h>
#include <tataki/bitmap/bitmap.h>
#include <api/wnd/popup.h>
#include <api/wndmgr/msgbox.h>
#include <api/skin/widgets/group.h>
#include "layout.h"
#include <api/skin/skinparse.h>
#include <api/skin/widgets/button.h>
#include <api/core/buttons.h>
#include <api/wnd/wndtrack.h>
//#include <api/wac/main.h> //CUT!
#include <api/skin/widgets/compbuck2.h>
#include <api/wac/compon.h>
#include <api/skin/skin.h>
#include <api/wnd/notifmsg.h>
#include <api/config/items/intarray.h>
#include <api/config/items/cfgitem.h>
#include <api/config/options.h>
#include <api/script/script.h>
#include <api/script/scriptmgr.h>
#include <bfc/parse/pathparse.h>
#include <api/skin/groupmgr.h>
#include <api/wndmgr/skinembed.h>
#include <api/skin/skinparse.h>
#include <api/service/svcs/svc_action.h>
#include <api/config/items/attrbool.h>
#include <api/config/items/attrint.h>
#include <api/wndmgr/alphamgr.h>
#include <bfc/wasabi_std_wnd.h>
#include <api/wnd/PaintCanvas.h>
#ifdef _WIN32
#include <windowsx.h> // for SetWindowRedraw
#endif

#ifdef WASABINOMAINAPI
#include <api/wndmgr/gc.h>
#include <api/syscb/callbacks/gccb.h>
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x20A
#endif

#define TIMER_SETTINGCHANGED 0x8791
#define TIMER_AUTOOPACIFY 0x8792
#define TIMER_AUTOOPACIFY2 0x8793
#define TIMER_OFFSCREENCHECK 0x8794
#define TIMER_TRANSPARENCY_AUTOON 0x8795
#define TIMER_SAVE_POSITION 0x8796
#define TIMER_SAVE_ALL_POSITIONS 0x8797

#ifdef WASABI_COMPILE_CONFIG
extern _bool cfg_uioptions_linkallalpha;
extern _bool cfg_uioptions_linkallratio;
extern _int cfg_uioptions_autoopacitylinked;
extern _int cfg_uioptions_autoopacitytime;
extern _int cfg_uioptions_extendautoopacity;
extern _int cfg_uioptions_autoopacityfadein;
extern _int cfg_uioptions_autoopacityfadeout;
extern _int cfg_uioptions_linkedalpha;
extern _bool cfg_options_appbarondrag;
#endif

AlphaMgr *alphaMgr = NULL;
XMLParamPair Layout::params[] =
    {
        {LAYOUT_SETALPHA, L"ALPHA"},
        {LAYOUT_SETALPHABACKGROUND, L"ALPHABACKGROUND"},
        {LAYOUT_SETDESKTOPALPHA, L"DESKTOPALPHA"},
        {LAYOUT_FORCEALPHA, L"FORCEALPHA"},
        {LAYOUT_SETINDESKTOP, L"INDESKTOP"},
        {LAYOUT_SETLINKHEIGHT, L"LINKHEIGHT"},
        {LAYOUT_SETLINKWIDTH, L"LINKWIDTH"},
        {LAYOUT_SETLOCKTO, L"LOCKTO"},
        {LAYOUT_SETNOACTIVATION, L"NOACTIVATION"},
        {LAYOUT_NODOCK, L"NODOCK"},
        {LAYOUT_NOOFFSCREENCHECK, L"NOOFFSCREENCHECK"},
        {LAYOUT_NOPARENT, L"NOPARENT"},
        {LAYOUT_SETONTOP, L"ONTOP"},
        {LAYOUT_SETOSFRAME, L"OSFRAME"},
        {LAYOUT_SETOWNER, L"OWNER"},
        {LAYOUT_SNAPADJUSTBOTTOM, L"SNAPADJUSTBOTTOM"},
        {LAYOUT_SNAPADJUSTLEFT, L"SNAPADJUSTLEFT"},
        {LAYOUT_SNAPADJUSTRIGHT, L"SNAPADJUSTRIGHT"},
        {LAYOUT_SNAPADJUSTTOP, L"SNAPADJUSTTOP"},
        {LAYOUT_UNLINKED, L"UNLINKED"},
		{LAYOUT_RESIZABLE, L"RESIZABLE"},
		{LAYOUT_SCALABLE, L"SCALABLE"},
    };

Layout::Layout()
{
	getScriptObject()->vcpu_setInterface(layoutGuid, (void *)static_cast<Layout *>(this));
	getScriptObject()->vcpu_setInterface(guiResizableGuid, (void *)static_cast<GuiResizable *>(this));
	getScriptObject()->vcpu_setClassName(L"Layout");
	getScriptObject()->vcpu_setController(layoutController);
#ifdef _WIN32
	forwardMsgWnd = INVALIDOSWINDOWHANDLE;
#endif
	transparency_reenabled_at = 0;
	transparency_autooff = 0;
	captured = 0; resizing = 0;
	m_forceunlink = 0;
	inresize = 0;
	autoopacify = 0;
	x = 0; y = 0;
#ifdef USEAPPBAR
	appbar_want_autohide = 1;
	appbar_want_alwaysontop = 1;
	m_allowsavedock = 0;
#endif
	size_w = 0; size_h = 0;
	reg = NULL;
	//subregionlayers = new PtrList<Layer>;
	setStartHidden(TRUE);
	moving = 0;
	setVirtual(0);
	indesktop = 0;
	p_container = NULL;
	alpha = 255;
	linkedheight = NULL;
	linkedwidth = NULL;
	inlinkwidth = 0;
	inlinkheight = 0;
	wantactiv = 1;
	wantdesktopalpha = 0;
	alllayouts.addItem(this);
	galphadisabled = 0;
	lockedto = NULL;
	osframe = 0;
	scalelocked = 0;
	initontop = 0;
	wantredrawonresize = 1;
	getGuiObject()->guiobject_setMover(1);
	snap_adjust_left = snap_adjust_right = snap_adjust_top = snap_adjust_bottom = 0;
	disable_auto_alpha = 0;
	unlinked = 0;
	scaling = 0;
	transparencyoverride = -1;
	noparent = 0;
	forcealpha = 0;
	nodock = 0;
	resizable = 1;
	scalable = 1;
	nooffscreencheck = 0;
	m_w = m_h = m_endmovesize = 0;

	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);

	if (!alphaMgr)
	{
		alphaMgr = new AlphaMgr();
    #ifdef WASABI_COMPILE_CONFIG
		alphaMgr->setAllLinked(cfg_uioptions_linkallalpha.getValueAsInt());
		alphaMgr->setAutoOpacify(cfg_uioptions_autoopacitylinked.getValueAsInt());
		alphaMgr->setGlobalAlpha(cfg_uioptions_linkedalpha.getValueAsInt());
		alphaMgr->setHoldTime(cfg_uioptions_autoopacitytime.getValueAsInt());
		alphaMgr->setFadeInTime(cfg_uioptions_autoopacityfadein.getValueAsInt());
		alphaMgr->setFadeOutTime(cfg_uioptions_autoopacityfadeout.getValueAsInt());
		alphaMgr->setExtendAutoOpacity(cfg_uioptions_extendautoopacity.getValueAsInt());
#endif
	}
	alphaMgr->addLayout(this);
}

void Layout::CreateXMLParameters(int master_handle)
{
	//LAYOUT_SCRIPTPARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

Layout::~Layout()
{
	if (transparency_reenabled_at) killTimer(TIMER_TRANSPARENCY_AUTOON);
	alphaMgr->removeLayout(this);
	killTimer(TIMER_OFFSCREENCHECK);
	if (lockedto) lockedto->removeLockedLayout(this);
	WASABI_API_WNDMGR->wndTrackRemove(this);
	if (reg) delete reg;
	alllayouts.removeItem(this);
}

int Layout::setXuiParam(int _xuihandle, int attrid, const wchar_t *paramname, const wchar_t *strvalue)
{
	if (xuihandle != _xuihandle) return LAYOUT_PARENT::setXuiParam(_xuihandle, attrid, paramname, strvalue);
	switch (attrid)
	{
	case LAYOUT_SETDESKTOPALPHA:
		setWantDesktopAlpha(WTOI(strvalue));
		break;
	case LAYOUT_SETINDESKTOP:
		setInDesktop(WTOI(strvalue));
		break;
	case LAYOUT_SETALPHA:
		setAlpha(WTOI(strvalue));
		break;
	case LAYOUT_SETLINKWIDTH:
		setLinkWidth(strvalue);
		if (*strvalue && isPostOnInit() && isVisible()) onResize();
		break;
	case LAYOUT_SETLINKHEIGHT:
		setLinkHeight(strvalue);
		if (*strvalue && isPostOnInit() && isVisible()) onResize();
		break;
	case LAYOUT_SETOWNER:
		owner = strvalue;
		break;
	case LAYOUT_NOPARENT:
		setNoParent(WTOI(strvalue));
		break;
	case LAYOUT_FORCEALPHA:
		forcealpha = WTOI(strvalue);
		break;
	case LAYOUT_SETLOCKTO:
		lockto = strvalue;
		break;
	case LAYOUT_SETOSFRAME:
		osframe = WTOI(strvalue);
		break;
	case LAYOUT_SETALPHABACKGROUND:
		setAlphaBackground(strvalue);
		setWantDesktopAlpha(1);
		setDrawBackground(1);
		break;
	case LAYOUT_SETNOACTIVATION:
		wantactiv = WTOI(strvalue) ? 0 : 1;
		break;
	case LAYOUT_SETONTOP:
		initontop = WTOI(strvalue);
		if (isPostOnInit()) updateOnTop();
		break;
	case LAYOUT_SNAPADJUSTLEFT:
		snap_adjust_left = WTOI(strvalue);
		script_vcpu_onSnapAdjustChanged(SCRIPT_CALL, getScriptObject());
#ifdef USEAPPBAR
		if (appbar_isDocked()) appbar_posChanged();
#endif
		break;
	case LAYOUT_SNAPADJUSTTOP:
		snap_adjust_top = WTOI(strvalue);
		script_vcpu_onSnapAdjustChanged(SCRIPT_CALL, getScriptObject());
    #ifdef USEAPPBAR
		if (appbar_isDocked()) appbar_posChanged();
#endif
		break;
	case LAYOUT_SNAPADJUSTRIGHT:
		snap_adjust_right = WTOI(strvalue);
		script_vcpu_onSnapAdjustChanged(SCRIPT_CALL, getScriptObject());
    #ifdef USEAPPBAR
		if (appbar_isDocked()) appbar_posChanged();
#endif
		break;
	case LAYOUT_SNAPADJUSTBOTTOM:
		snap_adjust_bottom = WTOI(strvalue);
		script_vcpu_onSnapAdjustChanged(SCRIPT_CALL, getScriptObject());
    #ifdef USEAPPBAR
		if (appbar_isDocked()) appbar_posChanged();
#endif
		break;
	case LAYOUT_UNLINKED:
		unlinked = WTOI(strvalue);
		if (!unlinked && isPostOnInit() && isVisible()) onResize();
		break;
	case LAYOUT_NODOCK:
		setNoDock(WTOI(strvalue));
		break;
	case LAYOUT_NOOFFSCREENCHECK:
		setNoOffscreenCheck(WTOI(strvalue));
		break;
	case LAYOUT_RESIZABLE:
		resizable = WTOI(strvalue);
		break;
	case LAYOUT_SCALABLE:
		scalable = WTOI(strvalue);
		break;
	default:
		return 0;
	}
	return 1;
}

void Layout::setLinkHeight(const wchar_t *layoutid)
{
	linkedheight = layoutid;
}

void Layout::setLinkWidth(const wchar_t *layoutid)
{
	linkedwidth = layoutid;
}

int Layout::onPostedMove()
{
	updateLockedLayouts();
	int r = LAYOUT_PARENT::onPostedMove();
	Container *c = getParentContainer();
	if (isVisible() && c && c->isMainContainer())
	{
#ifdef WASABI_ON_MAIN_MOVE
		WASABI_ON_MAIN_MOVE(gethWnd());
#endif

	}
	return r;
}

void Layout::cancelCapture()
{
	if (getCapture())
		endCapture();
	captured = 0;
}

int Layout::isOffscreen(ifc_window *w)
{
	if (!w->isVisible()) return 0;
	if (nooffscreencheck) return 0;
#ifdef USEAPPBAR
	if (appbar_isDocked()) return 0;
#endif
	if (lockedto) return 0;
	RECT wr;
	w->getWindowRect(&wr);
	windowTracker->snapAdjustWindowRect(w, &wr);
	RECT r;

	int isontop = 0;
	if (w->getGuiObject())
	{
		isontop = WTOI(w->getGuiObject()->guiobject_getXmlParam(L"ontop"));
#ifndef EXPERIMENTAL_INDEPENDENT_AOT
		if (WTOI(w->getGuiObject()->guiobject_getXmlParam(L"noparent")) == 0)
			isontop = 0;
#endif

	}
	if (!isontop)
	{
#ifdef WASABI_COMPILE_CONFIG
		extern _bool cfg_options_alwaysontop;
		isontop |= cfg_options_alwaysontop.getValueAsInt();
#endif
	}
	Wasabi::Std::getViewport(&r, NULL, &wr, NULL, isontop);
	if (wr.right < r.left + 10 ||
	        wr.bottom < r.top + 10 ||
	        wr.left > r.right - 10 ||
	        wr.top > r.bottom - 10)
	{
		return 1;
	}  //////TO CONTINUE
	return 0;
}

void Layout::offscreenCheck()
{
	int disabled = 0;
#ifdef WASABI_CHECK_OFFSCREENCHECK_DISABLE
	WASABI_CHECK_OFFSCREENCHECK_DISABLE(disabled);
#endif
	if (disabled) return ;
	if (!isVisible()) return ;
	if (moving || scaling || resizing) return ;
	if (isOffscreen(this))
	{
		windowTracker->startCooperativeMove(this);
		for (int i = 0;i < windowTracker->getNumDocked();i++)
		{
			ifc_window *w = windowTracker->enumDocked(i);
			Layout *l = static_cast<Layout*>(w->getInterface(layoutGuid));
			if (l != NULL)
			{
				if (l->getParentContainer() && l->getParentContainer()->isMainContainer())
				{
					if (!isOffscreen(l))
					{
						windowTracker->endCooperativeMove();
						return ;
					}
				}
			}
		}
		windowTracker->endCooperativeMove();
		RECT wr;
		getWindowRect(&wr);
		RECT nr = windowTracker->findOpenRect(wr);
		move(nr.left, nr.top);
	}
}

/**
 * This one only prevents UI resizings from layers!
 */
int Layout::getResizable ()
{
	return this->resizable;
}

/**
 * This one only prevents UI scalings from layers!
 */
int Layout::getScalable ()
{
	return this->scalable;
}

void Layout::setTransparencyOverride(int v)
{
	transparencyoverride = v;
	if (v != -1) setTransparency(v); else updateTransparency();
}

#ifndef PI
#define PI 3.1415926536
#endif

void Layout::timerCallback(int id)
{
	if (id == TIMER_TRANSPARENCY_AUTOON)
	{
		if (!isTransparencySafe(1))
		{
			transparency_reenabled_at = 0;
			return ;
		}
		int n = Wasabi::Std::getTickCount() - transparency_reenabled_at;
		if (n < 1000) return ;
		else
		{
			killTimer(TIMER_TRANSPARENCY_AUTOON);
			transparency_autooff = 0;
			updateTransparency();
			transparency_reenabled_at = 0;
		}
		return ;
	}
	else if (id == TIMER_OFFSCREENCHECK)
	{
		offscreenCheck();
		return ;
	}
	else if (id == TIMER_SETTINGCHANGED)
	{
		killTimer(TIMER_SETTINGCHANGED);
#ifdef _WIN32
		HWND parent;
		webserver = INVALIDOSWINDOWHANDLE;
		listview = INVALIDOSWINDOWHANDLE;
		getExplorerWindows(&parent, &listview, &webserver);
		if (!webserver) // active desktop now off, reposition this window zorder otherwise it will be behind the icons
#ifdef WIN32
			SetWindowPos(gethWnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_DEFERERASE);
#else
			bringToFront();
#endif
#endif
	}
	else if (id == TIMER_SAVE_ALL_POSITIONS)
	{
		killTimer(TIMER_SAVE_ALL_POSITIONS);
		saveAllPositions();
		// TODO: benski> not 100% sure we want to call into the script on the timer (maybe do it in endMove)
		//script_vcpu_onEndMove(SCRIPT_CALL, getScriptObject());
		return;
	}
	else if (id == TIMER_SAVE_POSITION)
	{
			killTimer(TIMER_SAVE_POSITION);
			savePosition();
			// TODO: benski> not 100% sure we want to call into the script on the timer (maybe do it in endMove)
			//script_vcpu_onEndMove(SCRIPT_CALL, getScriptObject());
		return;
	}
	else LAYOUT_PARENT::timerCallback(id);
}

void Layout::onMouseEnterLayout()
{
	script_vcpu_onMouseEnterLayout(SCRIPT_CALL, getScriptObject());
}

void Layout::onMouseLeaveLayout()
{
	script_vcpu_onMouseLeaveLayout(SCRIPT_CALL, getScriptObject());
}

void Layout::beginMove()
{
	moving = 1;
}

void Layout::beginScale()
{
	scaling = 1;
}

void Layout::beginResize()
{
	resizing = 1;
}

void Layout::endMove()
{
	if (m_endmovesize)
	{
		m_endmovesize = 0;
		RECT r;
		guiresizable_getRootWnd()->getWindowRect(&r);
		r.right = r.left + m_w;
		r.bottom = r.top + m_h;
		guiresizable_getRootWnd()->resizeToRect(&r);
	}
	moving = 0;
	// TODO: benski> do this on a resetable timer so it doesn't go so slowly
	if (WASABI_API_WNDMGR->wndTrackWasCooperative())
		setTimer(TIMER_SAVE_ALL_POSITIONS, 1000);
//		saveAllPositions();
	else
		setTimer(TIMER_SAVE_POSITION, 1000);
		//savePosition();
	// TODO: benski> not 100% sure we want to call into the script on the timer (maybe do it in endMove)
	script_vcpu_onEndMove(SCRIPT_CALL, getScriptObject());
}

void Layout::endScale()
{
	scaling = 0;
	savePosition();
	fixPosition();
}

void Layout::endResize()
{
	resizing = 0;
	RECT r;
	getClientRect(&r);
	RECT wr;
	getWindowRect(&wr);
	script_vcpu_onUserResize(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(wr.left), MAKE_SCRIPT_INT(wr.top), MAKE_SCRIPT_INT(r.right - r.left), MAKE_SCRIPT_INT(r.bottom - r.top));
	savePosition();
}

void Layout::onMove()
{
	script_vcpu_onMove(SCRIPT_CALL, getScriptObject());
}

void Layout::setAutoOpacify(int a)
{
	if (autoopacify == a) return ;
	autoopacify = a != -1 ? a : autoopacify;
	if (getParentContainer() && !getParentContainer()->isTranscient())
	{
		StringW tmp;
		tmp.printf(L"Skin:%s/Container:%s/Layout:%s/autoopacify", WASABI_API_SKIN->getSkinName(), getParentContainer()->getDescriptor(), getGuiObject()->guiobject_getId());
		IntArray::write(tmp, autoopacify);
	}
	updateTransparency();
}

void Layout::controlMenu()
{
	PopupMenu pop;
	int x = 1;
#ifdef WASABI_CHECK_CAN_EXIT
	WASABI_CHECK_CAN_EXIT(x)
#endif
	if (getParentContainer() && getParentContainer()->isMainContainer())
	{
		if (x)
		{
			pop.addCommand(StringPrintfW(L"Exit %s", WASABI_API_APP->main_getAppName()), ACTION_CLOSE);
			pop.addSeparator();
		}
	}
	else
	{
		if (!getParentContainer() || getParentContainer()->canClose())
		{
			pop.addCommand(L"Close window", ACTION_CLOSE_WINDOW);
			pop.addSeparator();
		}
	}
	PopupMenu *scalemenu = new PopupMenu();
	scalemenu->addCommand(L"50%", ACTION_SCALE_50);
	scalemenu->addCommand(L"75%", ACTION_SCALE_75, FALSE, FALSE);
	scalemenu->addCommand(L"100%", ACTION_SCALE_100);
	scalemenu->addCommand(L"125%", ACTION_SCALE_125);
	scalemenu->addCommand(L"150%", ACTION_SCALE_150);
	scalemenu->addCommand(L"200%", ACTION_SCALE_200);
	scalemenu->addCommand(L"400%", ACTION_SCALE_400, FALSE, FALSE);
	pop.addSubMenu(scalemenu, L"Scale");

	PopupMenu *opaqmenu = new PopupMenu();
	opaqmenu->addCommand(L"Auto-fade", ACTION_AUTOOPACIFY, getAutoOpacify(), FALSE);
	opaqmenu->addSeparator();
	opaqmenu->addCommand(L"100%", ACTION_ALPHA_100, FALSE, FALSE);
	opaqmenu->addCommand(L"90%", ACTION_ALPHA_90, FALSE, FALSE);
	opaqmenu->addCommand(L"80%", ACTION_ALPHA_80, FALSE, FALSE);
	opaqmenu->addCommand(L"70%", ACTION_ALPHA_70, FALSE, FALSE);
	opaqmenu->addCommand(L"60%", ACTION_ALPHA_60, FALSE, FALSE);
	opaqmenu->addCommand(L"50%", ACTION_ALPHA_50, FALSE, FALSE);
	opaqmenu->addCommand(L"40%", ACTION_ALPHA_40, FALSE, FALSE);
	opaqmenu->addCommand(L"30%", ACTION_ALPHA_30, FALSE, FALSE);
	opaqmenu->addCommand(L"20%", ACTION_ALPHA_20, FALSE, FALSE);
	opaqmenu->addCommand(L"10%", ACTION_ALPHA_10, FALSE, FALSE);

	pop.addSubMenu(opaqmenu, L"Opacity", !isTransparencySafe());

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
	pop.addCommand(L"Always on top", ACTION_AOT, getAlwaysOnTop());
#endif

	int r = pop.popAtMouse();
	if (r < 0) return ;
	onActionNotify(r, 0);
}

void Layout::fixPosition()
{
	RECT r;
	getWindowRect(&r);
	RECT orig_r = r;

	RECT vr;
	RegionI reg;

	int i = 0;
	while (Wasabi::Std::enumViewports(i, &vr))
	{
		reg.addRect(&vr);
		i++;
	}

	RECT full;
	reg.getBox(&full);


	int dif = r.bottom - full.bottom;
	if (dif > 0) r.top -= dif;
	dif = r.right - full.right;
	if (dif > 0) r.left -= dif;
	dif = full.top - r.top;
	if (dif > 0) r.top += dif;
	dif = full.left - r.left;
	if (dif > 0) r.left += dif;

	if (orig_r.left != r.left || orig_r.top != r.top)
		resize(r.left, r.top, NOCHANGE, NOCHANGE);
}

void Layout::saveAllPositions()
{
	// The positions are saved from the lastest to the first one
	// The 'Player' windows and the 'Main' windows are using the same prefix
	// In saving in decreasing order we are sure the lastest saved is the one for the Player

	for (int i = 0;i < SkinParser::script_getNumContainers();i++)
	{
		Container *c = SkinParser::script_enumContainer(i);
		for (int j = c->getNumLayouts() - 1;j >= 0 ;--j)
		{
			Layout *l = c->enumLayout(j);
			l->savePosition();
		}
	}
}

int Layout::forceTransparencyFlag()
{
	return alphaMgr->needForcedTransparencyFlag(this);
}

void Layout::savePosition()
{
#ifdef WASABI_COMPILE_CONFIG
	if (!getParentContainer() || getParentContainer()->isTranscient())
		return ;

	if (!Wasabi::Std::Wnd::isValidWnd(getOsWindowHandle()))
		return ;


	StringW        prefix       = MakePrefix();
	StringPrintfW  tmp( L"%s/odim", prefix.v() );

	SkinBitmap    *baseTexture = getBaseTexture();

	if ( baseTexture )
	{
		IntArray::write( tmp, baseTexture->getWidth(), baseTexture->getHeight() );
	}
	else
	{
		IntArray::write( tmp, -2, -2 );
	}

	RECT r, rs;
	getClientRect(&r); // unscaled
	//DebugStringW( L"\nLayout::savePosition()              - x = %d, y = %d, w = %d, h = %d \n", r.left, r.top, r.right - r.left, r.bottom - r.top );

	if (r.left == 0 && r.top == 0 && r.right == 0 && r.bottom == 0)
		return ;

	getWindowRect(&rs); // screen scaled (for x/y)
	tmp.printf(L"%s/rect", prefix.v());
	IntArray::write(tmp, rs.left, rs.top, r.right - r.left, r.bottom - r.top);
	//DebugStringW( L"Layout::savePosition() rect         - x = %d, y = %d, w = %d, h = %d \n", rs.left, rs.top, r.right - r.left, r.bottom - r.top );

	getRestoredRect(&rs);
	tmp.printf(L"%s/restoredrect", prefix.v());
	IntArray::write(tmp, rs.left, rs.top, rs.right - rs.left, rs.bottom - rs.top);

	//DebugStringW( L"Layout::savePosition() restoredrect - x = %d, y = %d, w = %d, h = %d \n", rs.left, rs.top, rs.right - rs.left, rs.bottom - rs.top );

	tmp.printf(L"%s/maximized", prefix.v());
	//  DebugString("isMaximized = %d\n", isMaximized());
	WASABI_API_CONFIG->setIntPrivate(tmp, isMaximized());

	tmp.printf(L"%s/r", prefix.v());
	WASABI_API_CONFIG->setStringPrivate(tmp, StringPrintfW(getRenderRatio()));

	tmp.printf(L"%s/sm", prefix.v());
	TextInfoCanvas textInfoCanvas(this);
	WASABI_API_CONFIG->setStringPrivate(tmp, StringPrintfW(textInfoCanvas.getSystemFontScale()));

	tmp.printf(L"%s/sl", prefix.v());
	WASABI_API_CONFIG->setStringPrivate(tmp, StringPrintfW(scalelocked));

	tmp.printf(L"%s/autoopacify", prefix.v());
	IntArray::write(tmp, autoopacify);

#ifdef USEAPPBAR
	saveAppBarPosition();
#endif

#endif
}
#ifdef USEAPPBAR
void Layout::saveAppBarPosition()
{
	if (m_allowsavedock)
	{
		StringW prefix = MakePrefix();
		StringW wtmp = StringPrintfW(L"%s/appbar", prefix.v());
		WASABI_API_CONFIG->setIntPrivate(wtmp, appbar_getSide());
		wtmp.printf(L"%s/appbarontop", prefix.v());
		WASABI_API_CONFIG->setIntPrivate(wtmp, appbar_wantAlwaysOnTop());
		wtmp.printf(L"%s/appbarhidden", prefix.v());
		WASABI_API_CONFIG->setIntPrivate(wtmp, appbar_isHiding());
		wtmp.printf(L"%s/appbarisautohide", prefix.v());
		WASABI_API_CONFIG->setIntPrivate(wtmp, appbar_isAutoHiding());
		wtmp.printf(L"%s/appbarwantautohide", prefix.v());
		WASABI_API_CONFIG->setIntPrivate(wtmp, appbar_wantAutoHide());
	}
}
#endif
void Layout::lockScale(int locked)
{
	scalelocked = locked;
	StringW tmp;
	StringW prefix = MakePrefix();
	tmp.printf(L"%s/sl", prefix.v());
	WASABI_API_CONFIG->setStringPrivate(tmp, StringPrintfW(scalelocked));
}

void Layout::resize(int x, int y, int w, int h, int wantcb)
{
	if (inresize) return ;
	inresize = 1;
	LAYOUT_PARENT::resize(x, y, w, h, wantcb);
	if (!lockedto)
		getGuiObject()->guiobject_setGuiPosition(&x, &y, &w, &h, NULL, NULL, NULL, NULL);
	inresize = 0;
}

void Layout::move(int x, int y)
{
	//DebugStringW( L"Layout::move( x = %d, y = %d )\n", x, y );

	LAYOUT_PARENT::move(x, y);
	getGuiObject()->guiobject_setGuiPosition(&x, &y, NULL, NULL, NULL, NULL, NULL, NULL);
}

#define DC_CHECKLASTRESIZE 0x109
#ifdef USEAPPBAR
#define DC_LOADSAVEDDOCK 0x10A
#endif
#define DC_INVALIDATE 0x10B

int Layout::onResize()
{
	LAYOUT_PARENT::onResize();
	if (!abortSaving()) savePosition();
	RECT r;
	getClientRect(&r);
	if (!linkedwidth.isempty())
	{
		if (!inlinkwidth)
		{
			inlinkwidth = 1;
			RECT cr;
			Layout *l = getParentContainer() ? getParentContainer()->getLayout(linkedwidth) : NULL;
			if (l)
			{
				int _unlinked = l->isUnlinked();
				_unlinked |= isUnlinked();
				if (!_unlinked)
				{
					l->getClientRect(&cr);
					POINT pt;
					l->getPosition(&pt);
					l->resize(pt.x, pt.y, r.right - r.left, cr.bottom - cr.top);
				}
			}
			inlinkwidth = 0;
		}
	}
	if (!linkedheight.isempty())
	{
		if (!inlinkheight)
		{
			inlinkheight = 1;
			RECT cr;
			Layout *l = getParentContainer() ? getParentContainer()->getLayout(linkedheight) : NULL;
			if (l)
			{
				int _unlinked = l->isUnlinked();
				_unlinked |= isUnlinked();
				if (!_unlinked)
				{
					l->getClientRect(&cr);
					POINT pt;
					l->getPosition(&pt);
					l->resize(pt.x, pt.y, cr.right - cr.left, r.bottom - r.top);
				}
			}
		}
		inlinkheight = 0;
	}
	updateLockedLayouts();

	postDeferredCallback(DC_CHECKLASTRESIZE);

	return 1;
}

int Layout::onDeferredCallback(intptr_t p1, intptr_t p2)
{
	/*  if (p1 == DC_CHECKLASTRESIZE) {
	    RECT vr;
	    RECT r;
	    RECT wr;
	    Std::getViewport(&vr, NULL, NULL, gethWnd(), 1);
	    if (renderRatioActive())
	      divRatio(&vr);
	    getClientRect(&r);
	    r.bottom -= snap_adjust_bottom;
	    r.top += snap_adjust_top;
	    r.right -= snap_adjust_right;
	    r.left += snap_adjust_left;
	    getWindowRect(&wr);
	    int n = 0;
	    if (r.right-r.left > vr.right-vr.left) { r.right = r.left + vr.right-vr.left; n++; } 
	    if (r.bottom-r.top > vr.bottom-vr.top) { r.bottom = r.top + vr.bottom-vr.top; n++; }
	    if (n) resize(wr.left, wr.top, r.right-r.left+snap_adjust_left+snap_adjust_right, r.bottom-r.top+snap_adjust_top+snap_adjust_bottom);
	  } else */
#ifdef USEAPPBAR
	if (p1 == DC_LOADSAVEDDOCK)
	{}
#endif

	if (p1 == DC_INVALIDATE)
	{
		invalidate();
	}
	return LAYOUT_PARENT::onDeferredCallback(p1, p2);
	//  return 1;
}

void Layout::updateLockedLayouts()
{
	for (int i = 0;i < getNumLockedLayouts();i++)
	{
		Layout *l = enumLockedLayout(i);

		if (l->getRenderRatio() != getRenderRatio())
			l->setRenderRatio(getRenderRatio());

		int x, y, w, h;
		l->getGuiObject()->guiobject_getGuiPosition(&x, &y, &w, &h, NULL, NULL, NULL, NULL);
		RECT r;
		l->getClientRect(&r);
		if (w == AUTOWH) w = r.right - r.left;
		if (h == AUTOWH) h = r.bottom - r.top;
		POINT pt;
		l->getPosition(&pt);
		if (x == AUTOWH) x = pt.x; else clientToScreen(&x, NULL);
		if (y == AUTOWH) y = pt.y; else clientToScreen(NULL, &y);
		RECT cr;
		l->getWindowRect(&cr);
		if (cr.left != x || cr.top != y || cr.right != cr.left + w || cr.bottom != cr.top + h)
			l->Group::resize(x, y, w, h);
	}
}

int Layout::isClickThrough()
{
	return 0;
}
#ifdef _WIN32
LRESULT Layout::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

#ifdef ON_CUSTOM_ALTF4
	if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
		if (wParam == VK_F4)
			if (GetAsyncKeyState(VK_MENU)&0x8000) ON_CUSTOM_ALTF4;
#endif

	if (uMsg == WM_KILLFOCUS || uMsg == WM_SETFOCUS)
	{
		alphaMgr->hoverCheck(this);
	}

	if (forwardMsgWnd != INVALIDOSWINDOWHANDLE)
	{
		switch (uMsg)
		{
		case WM_MOUSEWHEEL:
			return (SendMessageW(forwardMsgWnd, uMsg, wParam, 0x31337)); // 0x31337 avoid main window callback
			//case WM_DESTROY: //FG> commented out, makes wa quit when layout is destroyed
		case WM_CLOSE:
			return (SendMessageW(forwardMsgWnd, uMsg, wParam, lParam));

		case WM_SETTINGCHANGE:
			{
				if (!indesktop) break;
				setTimer(TIMER_SETTINGCHANGED, 1000);
			}
			break; //BU think this should be here eh

#ifdef WIN32
		case 0x0319:
			{	// hehe --BU
				if (lParam & 0x20000)
					FireAction(L"NEXT", NULL, 0, 0, NULL, 0, NULL, FALSE);
				else
					FireAction(L"PREV", NULL, 0, 0, NULL, 0, NULL, FALSE);
			}
			break;
#endif 
			/*
			      WIP: This won't work anymore.  What should we do about it? -- mig
			 
			        childNotify(NULL, CHILD_NOTIFY_LEFTPUSH, ACTION_NEXT);
			        break;
			*/
		}
	}
	return LAYOUT_PARENT::wndProc(hWnd, uMsg, wParam, lParam);
}
#else
OSStatus Layout::eventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData)
{
  return LAYOUT_PARENT::eventHandler(inHandlerCallRef, inEvent, inUserData);
}
#warning port me
#endif

/*int Layout::onRightButtonDown(int x, int y) {
  LAYOUT_PARENT::onRightButtonDown(x, y);
//  Main::appContextMenu(this, TRUE, isTransparencySafe());
  return 1;
}*/

ifc_window *Layout::getCustomOwner()
{
	if (noparent) return NULL;
	if (owner.isempty()) return NULL;
	return SkinParser::getLayout(owner);
}

void Layout::addLockedLayout(Layout *l)
{
	locked.addItem(l);
	updateLockedLayouts();
}

void Layout::removeLockedLayout(Layout *l)
{
	locked.removeItem(l);
}

int Layout::getNumLockedLayouts()
{
	return locked.getNumItems();
}

Layout *Layout::enumLockedLayout(int n)
{
	return locked.enumItem(n);
}

int Layout::isLocked()
{
	return (!lockto.isempty() 
          #ifdef USEAPPBAR
          || cfg_options_appbarondrag == 0 && appbar_isDocked()
#endif
          );
}

void Layout::lockTo(Layout *t)
{
	if (t == NULL) return ;
	if (lockedto) lockedto->removeLockedLayout(this);
	lockedto = t;
	t->addLockedLayout(this);
}

Layout *Layout::getLockedTo()
{
	return lockedto;
}

int Layout::reinit(OSMODULEHANDLE inst, OSWINDOWHANDLE parent, int nochild)
{
	StringW osname = getOSWndName();
	ASSERTPR(!indesktop, "in desktop reinit failed");
	if (noparent)
		parent = NULL;
	int r = LAYOUT_PARENT::reinit(inst, parent, nochild);
	setOSWndName(osname);
	return r;
}

int Layout::init(OSMODULEHANDLE inst, OSWINDOWHANDLE parent, int nochild)
{
	if (noparent) parent = NULL;
	if (!indesktop) return LAYOUT_PARENT::init(inst, parent, nochild);

#ifdef _WIN32
	webserver = INVALIDOSWINDOWHANDLE;
	listview = INVALIDOSWINDOWHANDLE;

	getExplorerWindows(&parent, &listview, &webserver);

	if (!parent)
	{
		indesktop = 0;
		return LAYOUT_PARENT::init(inst, parent, nochild);
	}
#endif
	nochild = 0;

#ifdef _WIN32
	// if active desktop is off, that's all we can do, we'll be on top of the icons
	// if active desktop is on, but listview is not, we'll be on top of active desktop
	if (!webserver || !listview) return LAYOUT_PARENT::init(inst, parent, nochild);
#endif

#ifdef WIN32
	// find who we're gonna sit on top of
	HWND behind = listview;
	if (GetWindow(listview, GW_HWNDPREV))
		behind = GetWindow(listview, GW_HWNDPREV);
#else
	DebugString( "portme -- Layout::init\n" );
#endif

	int r = LAYOUT_PARENT::init(inst, parent, nochild);
#ifdef WIN32
	SetWindowPos(gethWnd(), behind, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_DEFERERASE);
#endif

	return r;
}

#ifdef WIN32
void Layout::getExplorerWindows(OSWINDOWHANDLE *parent, OSWINDOWHANDLE *listview, OSWINDOWHANDLE *webserver)
{
	*parent = NULL;
	*listview = NULL;
	*webserver = NULL;

	char classname[256] = "";
	HWND w;

	// find explorer window. todo: litestep support ?
	*parent = FindWindowA(NULL, "Program Manager");
	if (!*parent) return ;

	// find its first child for parenting
	*parent = GetWindow(*parent, GW_CHILD);
	if (!*parent) return ;

	// find the children
	w = GetWindow(*parent, GW_CHILD);

	while (w)
	{
		GetClassNameA(w, classname, 255);
		if (!*webserver && STRCASEEQL("Internet Explorer_Server", classname))
			*webserver = w;
		if (!*listview && STRCASEEQL("SysListView32", classname))
			*listview = w;
		w = GetWindow(w, GW_HWNDNEXT);
	}
}
#endif

int Layout::onInit()
{
	LAYOUT_PARENT::onInit();

	Container *c = getParentContainer();
	if (c != NULL)
	{
		const wchar_t *s = c->getName();
		if (s != NULL)
		{
			setOSWndName(s);
		}
	}

	loadSavedState();

#ifdef WA3COMPATIBILITY
	setIcon(WASABI_API_APP->main_getIcon(TRUE), TRUE);
	setIcon(WASABI_API_APP->main_getIcon(FALSE), FALSE);
#endif

	WASABI_API_WNDMGR->wndTrackAdd(this);

	if (wantDesktopAlpha() && isDesktopAlphaSafe())
		desktopAlpha_autoTurnOn();

	if (getBaseTexture())
	{
		//RECT r = {0, 0, getBaseTexture()->getWidth(), getBaseTexture()->getHeight()};
		delete reg;
#ifdef _WIN32
		reg = new RegionI(getBaseTexture(), NULL, 0, 0, 0, 0, 0, 0, getDesktopAlpha() ? 1 : 255);
		setRegion(reg);
#else
#warning port me
#endif
	}

	SystemObject::onCreateLayout(this);

	if (!lockto.isempty()) lockTo(SkinParser::getLayout(lockto));

	updateOnTop();

	setNoOffscreenCheck( -1);

	return 1;
}

void Layout::updateOnTop()
{
  #ifdef USEAPPBAR
	if (!appbar_isDocked())
#endif
	{
#ifdef EXPERIMENTAL_INDEPENDENT_AOT
		setAlwaysOnTop(initontop);
#else
		if (noparent)
		{
			int disable = 0;
#ifdef WASABI_GET_TEMPDISABLE_AOT
			WASABI_GET_TEMPDISABLE_AOT(disable);
#endif
#ifdef _WIN32
			if (initontop && !disable) SetWindowPos(getOsWindowHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
			else SetWindowPos(getOsWindowHandle(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
#else
#warning port me
#endif
		}
#endif

	}
}

void Layout::loadSavedState()
{
#ifdef WASABI_COMPILE_CONFIG
	StringW tmp;
#endif
	wchar_t t[64] = L"";

	int ow = -1;
	int oh = -1;
	int ra = -1;

#ifdef WASABI_COMPILE_CONFIG
	if (getParentContainer() && !getParentContainer()->isTranscient())
	{
		StringW prefix = MakePrefix();
		tmp.printf(L"%s/odim", prefix.v());
		IntArray::read(tmp, &ow, &oh);

		tmp.printf(L"%s/alpha", prefix.v());
		IntArray::read(tmp, &ra);

		tmp.printf(L"%s/autoopacify", prefix.v());
		IntArray::read(tmp, &autoopacify);

	}
#endif

	int from_scratch = 0;
	if (Std::keyModifier(STDKEY_SHIFT) && Std::keyModifier(STDKEY_ALT) && Std::keyModifier(STDKEY_CONTROL)) from_scratch = 1;
#ifdef WASABI_COMPILE_CONFIG
	if (getParentContainer() && !getParentContainer()->isTranscient())
	{
		StringW prefix = MakePrefix();
		tmp.printf(L"%s/sl", prefix.v());
		WASABI_API_CONFIG->getStringPrivate(tmp, t, 63, L"0");
		lockScale(WTOI(t));
	}
#endif

	*t = 0;
#ifdef WASABI_COMPILE_CONFIG
	if (getParentContainer() && !getParentContainer()->isTranscient())
	{
		StringW prefix = MakePrefix();
		tmp.printf(L"%s/sm", prefix.v());
	}

	 WASABI_API_CONFIG->getStringPrivate(tmp, t, 63, L"1.");
	//double sm = (float)WTOF(t);
#else
	//double sm = 1.0;
#endif
	/*if (ABS(sm - Canvas::getSystemFontScale()) > 0.001)
	  from_scratch = 1;*/

	if (Group::getBaseTexture() && alphabackground.getBitmap() != NULL)
	{
		if (Group::getBaseTexture()->getWidth() != alphabackground.getWidth() || Group::getBaseTexture()->getHeight() != alphabackground.getHeight())
		{
			ASSERTPR(0, StringPrintf("layout %S should have same size for background and alphabackground", getGuiObject()->guiobject_getParentLayout()->getGuiObject()->guiobject_getId()));
		}
	}

	if ((ow != -2 && oh != -2) && (ow == -1 || oh == -1 || (getBaseTexture() && (ow != getBaseTexture()->getWidth() || oh != getBaseTexture()->getHeight()))))
		from_scratch = 1;

	int w = 0, h = 0;
	int findrect = 0;

	if (from_scratch || (getParentContainer() && getParentContainer()->isTranscient()))
	{
		if (getParentContainer())
		{
			int _x = getParentContainer()->getDefaultPositionX();
			int _y = getParentContainer()->getDefaultPositionY();
			x = _x == -1 ? x : _x;
			y = _y == -1 ? y : _y;
			if (x == -1 && y == -1) findrect = 1;
		}
		else
		{
			int _x, _y;
			getGuiObject()->guiobject_getGuiPosition(&_x, &_y, NULL, NULL, NULL, NULL, NULL, NULL);
			if (_x != AUTOWH && _x != NOCHANGE) x = _x;
			if (_y != AUTOWH && _y != NOCHANGE) y = _y;
		}
		getGuiObject()->guiobject_getGuiPosition(NULL, NULL, &w, &h, NULL, NULL, NULL, NULL);
		if (w == AUTOWH || h == AUTOWH)
		{
			w = getPreferences(SUGGESTED_W);
			h = getPreferences(SUGGESTED_H);
			int _minx = getPreferences(MINIMUM_W);
			int _maxx = getPreferences(MAXIMUM_W);
			int _miny = getPreferences(MINIMUM_H);
			int _maxy = getPreferences(MAXIMUM_H);
			if (_minx != AUTOWH && _maxx != AUTOWH && _minx > _maxx) _minx = _maxx;
			if (_miny != AUTOWH && _maxy != AUTOWH && _miny > _maxy) _miny = _maxy;
			if (w == AUTOWH && _minx != AUTOWH) w = _minx;
			if (h == AUTOWH && _miny != AUTOWH) h = _miny;
			TextInfoCanvas textInfoCanvas(this);
			double fontScale = textInfoCanvas.getSystemFontScale();
			if (w != AUTOWH && getGuiObject()->guiobject_getAutoSysMetricsW())
				w = (int)((float)w * fontScale);
			if (h != AUTOWH && getGuiObject()->guiobject_getAutoSysMetricsH())
				h = (int)((float)h * fontScale);
		}
	}
	else
	{
		if (getParentContainer())
		{

			if (getParentContainer()->isMainContainer()) //FG> could be a new component installed
				SkinParser::noCenterSkin();

#ifdef WASABI_COMPILE_CONFIG
			StringW prefix = MakePrefix();

			tmp.printf(L"%s/rect", prefix.v());
			IntArray::read(tmp, &x, &y, &w, &h);

			RECT rr;
			tmp.printf(L"%s/restoredrect", prefix.v());
			IntArray::read(tmp, (int*)&rr.left, (int*)&rr.top, (int*)&rr.right, (int*)&rr.bottom);
			rr.right += rr.left;
			rr.bottom += rr.top;

			tmp.printf(L"%s/maximized", prefix.v());
			if (WASABI_API_CONFIG->getIntPrivate(tmp, 0)) setRestoredRect(&rr);

#endif

		}
	}

	*t = 0;
	double ratio = 1.0;

	if (!from_scratch)
	{
		if (getParentContainer())
		{
#ifdef WASABI_COMPILE_CONFIG
			StringW prefix = MakePrefix();
			tmp.printf(L"%s/r", prefix.v());

			WASABI_API_CONFIG->getStringPrivate(tmp, t, 63, L"1.");
			ratio = WTOF(t);
#else
			ratio = 1.0f;
#endif

		}
	}

#ifdef ON_TWEAK_RENDER_RATIO
	if (!scalelocked) ON_TWEAK_RENDER_RATIO(ratio);
#endif

	if (w == 0 || h == 0 || from_scratch)
	{
		int _w = getPreferences(SUGGESTED_W);
		w = _w == AUTOWH ? w : _w;
		int _h = getPreferences(SUGGESTED_H);
		h = _h == AUTOWH ? h : _h;

		TextInfoCanvas textInfoCanvas(this);
		double fontScale = textInfoCanvas.getSystemFontScale();

		if (getGuiObject()->guiobject_getAutoSysMetricsW())
			w = (int)((float)w * fontScale);
		if (getGuiObject()->guiobject_getAutoSysMetricsH())
			h = (int)((float)h * fontScale);
		if (w == 0 || h == 0)
		{
			w = getBaseTexture()->getWidth();
			h = getBaseTexture()->getHeight();
		}
		RECT sr = {x, y, x + w, y + h};
		if (findrect)
			sr = windowTracker->findOpenRect(Wasabi::Std::makeRect(0, 0, (int)((float)w * getRenderRatio()), (int)((float)h * getRenderRatio())), this);
		sr.right = sr.left + w;
		sr.bottom = sr.top + h;
		BaseWnd::resize(&sr);
	}
	else
	{
		RECT cr;
		getClientRect(&cr);
		RECT r = {x, y, x + w, y + h};
		BaseWnd::resize(&r);
	}

	setAlpha(from_scratch || ra == -1 ? alpha : ((ra < 254) ? ra : 255));

	Layout *main = SkinParser::getMainLayout();
	if (main != this && main 
      #ifdef WASABI_COMPILE_CONFIG
      && cfg_uioptions_linkallratio.getValueAsInt() 
#endif
      && !noparent)
	{
		setRenderRatio(main->getRenderRatio());
	}
	else setRenderRatio(ratio);

	setAutoOpacify(autoopacify);

#ifdef USEAPPBAR
	postDeferredCallback(DC_LOADSAVEDDOCK);

	if (getParentContainer() && !getParentContainer()->isTranscient())
	{
		StringW prefix = MakePrefix();
		StringW tmp = StringPrintfW(L"%s/appbar", prefix.v());
		int side = WASABI_API_CONFIG->getIntPrivate(tmp, APPBAR_NOTDOCKED);
		tmp.printf(L"%s/appbarisautohide", prefix.v());
		int is_autohide = WASABI_API_CONFIG->getIntPrivate(tmp, 0);
		tmp.printf(L"%s/appbarwantautohide", prefix.v());
		appbar_want_autohide = is_autohide || WASABI_API_CONFIG->getIntPrivate(tmp, 1);
		tmp.printf(L"%s/appbarontop", prefix.v());
		appbar_want_alwaysontop = WASABI_API_CONFIG->getIntPrivate(tmp, 1);
		tmp.printf(L"%s/appbarhidden",prefix.v());
		int curside = appbar_getSide();
		if (side != curside)
		{
			if (side == APPBAR_NOTDOCKED)
				// starting up docked ??
				appbar_dock(side);
			else
			{
				setNoParent(2);
				BaseWnd::reinit();
				appbar_dock(side);
			}
		}
	}
	m_allowsavedock = 1;

#endif
}

void Layout::setBaseTexture(const wchar_t *b, int regis)
{
	LAYOUT_PARENT::setBaseTexture(b, 0); // 0, not regis!
	if (!getDesktopAlpha() || ((wantDesktopAlpha()) && alphabackground.getBitmap() == NULL))
	{
		if (regis) WASABI_API_WND->skin_unregisterBaseTextureWindow(this);
		if (getBaseTexture())
		{
			//RECT r = {0, 0, getBaseTexture()->getWidth(), getBaseTexture()->getHeight()};
			delete reg;
#ifdef _WIN32
			reg = new RegionI(getBaseTexture(), NULL, 0, 0, 0, 0, 0, 0, getDesktopAlpha() ? 1 : 255);
			setRegion(reg);
#else
#warning port me
#endif
		}
		if (regis)
			WASABI_API_WND->skin_registerBaseTextureWindow(this, getBackgroundStr());
	}
}

void Layout::setWantDesktopAlpha(int want)
{
	if (want && !Wasabi::Std::Wnd::isDesktopAlphaAvailable())
	{
		//    WASABI_API_WNDMGR->messageBox("Desktop Alpha Blending is only available for Win2000/WinXP and above", "Sorry", MSGBOX_OK, NULL, NULL);
		return ;
	}
	wantdesktopalpha = want;
	if (wantdesktopalpha && isInited() && isDesktopAlphaSafe())
		desktopAlpha_autoTurnOn();
	else if (!wantdesktopalpha && getDesktopAlpha())
		desktopAlpha_autoTurnOff();
}

int Layout::wantDesktopAlpha()
{
	return wantdesktopalpha;
}

int Layout::handleDesktopAlpha()
{
	return 1;
}

void Layout::onGuiObjectSetVisible(GuiObject *o, int visible)
{
	if (disable_auto_alpha) return ;
	if (!o || !o->guiobject_getRootWnd()) return ;
	if (visible)
	{
		if (getDesktopAlpha())
		{
			if (!o->guiobject_getRootWnd()->handleDesktopAlpha())
			{
				desktopAlpha_autoTurnOff();
			}
		}
		if (!transparency_autooff)
		{
			if (!o->guiobject_getRootWnd()->handleTransparency())
			{
				transparency_autoTurnOff();
			}
		}
	}
	else
	{ // !visible
		if (!getDesktopAlpha() && wantDesktopAlpha())
		{
			if (isDesktopAlphaSafe())
				desktopAlpha_autoTurnOn();
		}
		if (transparency_autooff)
		{
			if (isTransparencySafe(1))
				transparency_autoTurnOn();
		}
	}
}

void Layout::desktopAlpha_autoTurnOn()
{

	setDesktopAlpha(1);

	WASABI_API_WND->skin_unregisterBaseTextureWindow(this);

	if (getBaseTexture())
	{
		//RECT r = {0, 0, getBaseTexture()->getWidth(), getBaseTexture()->getHeight()};
		delete reg;
#ifdef _WIN32
		reg = new RegionI(getBaseTexture(), NULL, 0, 0, 0, 0, 0, 0, 1);
		setRegion(reg);
#else
#warning port me
#endif
	}

	WASABI_API_WND->skin_registerBaseTextureWindow(this, alphabackground.getBitmap() ? alphabackgroundstr.getValue() : getBackgroundStr());

	invalidate();
}

void Layout::desktopAlpha_autoTurnOff()
{
#ifdef WIN32
	SetWindowRedraw(gethWnd(), FALSE);//LockWindowUpdate(gethWnd());
	setDesktopAlpha(0);

	WASABI_API_WND->skin_unregisterBaseTextureWindow(this);

	if (getBaseTexture())
	{
		//RECT r = {0, 0, getBaseTexture()->getWidth(), getBaseTexture()->getHeight()};
		delete reg;
		reg = new RegionI(getBaseTexture(), NULL, 0, 0, 0, 0, 0, 0, 255);
		setRegion(reg);
	}

	WASABI_API_WND->skin_registerBaseTextureWindow(this, getBackgroundStr());
	SetWindowRedraw(gethWnd(), TRUE);//LockWindowUpdate(NULL);

	invalidate();
#else
	DebugString( "portme -- Layout::init\n" );
#endif
}

void Layout::transparency_autoTurnOn()
{
	transparency_reenabled_at = Wasabi::Std::getTickCount();
	setTimer(TIMER_TRANSPARENCY_AUTOON, 500);
}

void Layout::transparency_autoTurnOff()
{
	transparency_autooff = 1;
	killTimer(TIMER_TRANSPARENCY_AUTOON);
	updateTransparency();
}

void Layout::onGlobalEnableDesktopAlpha(int enabled)
{
	foreach(alllayouts)
	alllayouts.getfor()->globalEnableDesktopAlpha(enabled);
	endfor
#ifdef ON_TOGGLE_DESKTOPALPHA
	ON_TOGGLE_DESKTOPALPHA(enabled);
#endif
}

void Layout::globalEnableDesktopAlpha(int enabled)
{
	if (enabled)
	{
		galphadisabled = 0;
		setWantDesktopAlpha(wantdesktopalpha);
	}
	else
	{
		galphadisabled = 1;
		if (getDesktopAlpha())
			desktopAlpha_autoTurnOff();
	}
}

void Layout::setWantRedrawOnResize(int v)
{
	if (wantredrawonresize == v) return ;
	wantredrawonresize = v;
}

PtrList<Layout> Layout::alllayouts;

int Layout::onPaint(Canvas *canvas)
{
	PaintCanvas paintcanvas;
	if (canvas == NULL)
	{
		if (!paintcanvas.beginPaint(this)) return 0;
		canvas = &paintcanvas;
	}

	RECT r;

	if (!canvas->getClipBox(&r))
		getClientRect(&r);

	canvas->fillRect(&r, 0);

	LAYOUT_PARENT::onPaint(canvas);

	return 1;
}

/*WndHolder *Layout::getHolder() {
  return this;
}*/

SkinBitmap *Layout::getBaseTexture()
{
#ifdef WASABI_COMPILE_CONFIG
	// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
	const GUID uioptions_guid =
	    { 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };
	int alphaenabled = _intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid), L"Enable desktop alpha");
#else
	int alphaenabled = WASABI_WNDMGR_DESKTOPALPHA;
#endif
	if (!alphabackground.getBitmap() || !getDesktopAlpha() || !alphaenabled) return LAYOUT_PARENT::getBaseTexture();
	return alphabackground.getBitmap();
}

int Layout::runAction(int actionid, const wchar_t *param)
{
	switch (actionid)
	{
	case ACTION_SWITCH:
		if (getParentContainer())
		{
			getParentContainer()->switchToLayout(param);
		}
		break;
	case ACTION_TOGGLE:
		SkinParser::toggleContainer(param);
		break;
#ifdef WA3COMPATIBILITY
	case ACTION_MENU:
		Main::doMenu(param);
		break;
#endif
	case ACTION_ENDMODAL:
		endModal(WTOI(param));
		break;
	case ACTION_CLOSE:
		onActionNotify(ACTION_CLOSE);
		break;
#ifdef WA3COMPATIBILITY
	default:
		if (actionid != 0)
			Main::doAction(actionid);
		break;
#else
	case ACTION_MINIMIZE:
#ifdef _WIN32
		ShowWindow(WASABI_API_WND->main_getRootWnd()->gethWnd(), SW_MINIMIZE);
#endif
		break;
#endif

	}
	return 0;
}

int Layout::childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2)
{
	if (msg == ACTION_ENFORCEMINMAX)
	{
		loadSavedState();
		return 0;
	}
	if (child != NULL)
	{
		int objId = child->getNotifyId();
		if (msg == ChildNotify::BUTTON_LEFTPUSH)
		{
			switch (objId)
			{
			case ACTION_SWITCH:
			case ACTION_TOGGLE:
			case ACTION_MENU:
				runAction(objId, ((Wasabi::Button *)child)->getParam());
				break;
			case ACTION_ENDMODAL:
				{
					StringPrintfW s(L"%d", (static_cast<ButtonWnd *>(child))->getModalRetCode());
					runAction(ACTION_ENDMODAL, s);
				}
				break;
			case ACTION_CLOSE:
				runAction(ACTION_CLOSE);
				break;
				/*
				#ifdef WA3COMPATIBILITY
				        case ACTION_EJECT:
				          runAction(ACTION_EJECT, "0");
				          break;
				#endif
				*/
			default:
				runAction(objId);
				break;
			}
			return 0;
		}
		if (msg == ChildNotify::BUTTON_RIGHTPUSH)
		{
			//FUCKO    child->abortTip();
			//	  POINT pt;
			//	  Std::getMousePos(&pt);
			//	  screenToClient(&pt);
			//	  onRightButtonDown(pt.x, pt.y);
#pragma warning (disable: 4060)
			switch (objId)
			{
				/*
				#ifdef WA3COMPATIBILITY
				        case ACTION_EJECT: {
				          runAction(ACTION_EJECT, "1");
				        }
				        break;
				#endif
				*/
			}
#pragma warning (default: 4060)
			return 0;
		}
	}
	return LAYOUT_PARENT::childNotify(child, msg, param1, param2);
}

int Layout::onActionNotify(int action, intptr_t param)
{
	switch (action)
	{
#ifdef WA3COMPATIBILITY
	case ACTION_SYSMENU:
		Main::appContextMenu(this, TRUE, isDesktopAlphaSafe());
		break;
	case ACTION_CONTROLMENU:
		controlMenu();
		break;
	case ACTION_WINDOWMENU:
		Main::thingerContextMenu(this);
		break;
#elif defined(WASABI_CUSTOM_CONTEXTMENUS)
	case ACTION_SYSMENU:
		extern void appContextMenu(ifc_window *w);
		appContextMenu(this);
		break;
	case ACTION_CONTROLMENU:
		extern void appControlMenu(ifc_window *w);
		appControlMenu(this);
		break;
#endif
	case ACTION_SCALE_50: scaleTo(50); break;
	case ACTION_SCALE_75: scaleTo(75); break;
	case ACTION_SCALE_100: scaleTo(100); break;
	case ACTION_SCALE_125: scaleTo(125); break;
	case ACTION_SCALE_150: scaleTo(150); break;
	case ACTION_SCALE_200: scaleTo(200); break;
	case ACTION_SCALE_400: scaleTo(400); break;
	case ACTION_ALPHA_10: setAlpha(25); break;
	case ACTION_ALPHA_20: setAlpha(51); break;
	case ACTION_ALPHA_30: setAlpha(76); break;
	case ACTION_ALPHA_40: setAlpha(102); break;
	case ACTION_ALPHA_50: setAlpha(127); break;
	case ACTION_ALPHA_60: setAlpha(153); break;
	case ACTION_ALPHA_70: setAlpha(178); break;
	case ACTION_ALPHA_80: setAlpha(204); break;
	case ACTION_ALPHA_90: setAlpha(229); break;
	case ACTION_ALPHA_100: setAlpha(255); break;
	case ACTION_AUTOOPACIFY: setAutoOpacify(!getAutoOpacify()); break;
#ifdef EXPERIMENTAL_INDEPENDENT_AOT
	case ACTION_AOT: setAlwaysOnTop(!getAlwaysOnTop()); break;
#endif
	case ACTION_CLOSE_WINDOW:
	case ACTION_CLOSE:
		if (getParentContainer() && getParentContainer()->isMainContainer())
		{
			int x = 1;
#ifdef WASABI_CHECK_CAN_EXIT
			WASABI_CHECK_CAN_EXIT(x)
#endif
			if (x)
			{
				WASABI_API_APP->main_shutdown();
			}
		}
		else if (WASABI_API_WNDMGR->getModalWnd() != this)
		{
			if (!getParentContainer() || !getParentContainer()->isDynamic() || !getParentContainer()->canClose())
					if (getParentContainer()) getParentContainer()->setVisible(FALSE); else setVisible(FALSE);
			else
				skinEmbedder->destroyContainer(getParentContainer());
		}
		else
		{
			endModal(MSGBOX_ABORTED);
		}
		break;
	default: return 0;
	}
	return 1;
}

void Layout::containerToggled(const wchar_t *id, int visible)
{
  #ifdef _WIN32
	sendNotifyToAllChildren(WM_WA_CONTAINER_TOGGLED, (intptr_t)id, visible);
#else
#warning port me
#endif
}

void Layout::componentToggled(GUID *guid, int visible)
{
#ifdef _WIN32
	sendNotifyToAllChildren(WM_WA_COMPONENT_TOGGLED, (intptr_t)guid, visible);
#else
#warning port me
#endif
}

void Layout::setAlphaBackground(const wchar_t *b)
{
	alphabackgroundstr = b;
	alphabackground.setBitmap(b);
#ifdef _WIN32
	RegionI r(alphabackground.getBitmap());
	setRegion(&r);
#else
#warning port me
#endif
}

/*void Layout::setWindowRegion(api_region *reg) {
  LAYOUT_PARENT::setWindowRegion(reg);
  if (getDesktopAlpha()) {
    SetWindowRgn(gethWnd(), NULL, FALSE);
    return;
  }
  if (!isInited()) return;
  api_region *_r = getRegion();
  if (getRenderRatio() != 1.0 && reg) {
    api_region *clone = _r->clone();
    clone->scale(getRenderRatio());
    SetWindowRgn(gethWnd(), clone->makeWindowRegion(), TRUE); 
    _r->disposeClone(clone);
  } else {
    SetWindowRgn(gethWnd(), _r ? _r->makeWindowRegion() : NULL, TRUE); 
  }
}*/

void Layout::onSetDesktopAlpha(int a)
{
	invalidateWindowRegion();
}

void Layout::onShow(void)
{
	savePosition();
	if (!WASABI_API_MAKI->vcpu_getComplete()) SystemObject::onShowLayout(this);
}

void Layout::onHide(void)
{
	savePosition();
	if (!WASABI_API_MAKI->vcpu_getComplete()) SystemObject::onHideLayout(this);
#ifndef WASABINOMAINAPI
	api->hintGarbageCollect();
#else
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::GC, GarbageCollectCallback::GARBAGECOLLECT);
#endif
	if (getParentContainer() && getParentContainer()->wantRefocusApp()) SkinParser::focusFirst();
}

#ifdef _WIN32
LPARAM Layout::wndHolder_getParentParam(int i)
{
	switch (i)
	{
	case 0: return (LPARAM)gethWnd();
	case 1: return (LPARAM)static_cast<BaseWnd*>(this);
	}
	return 0;
}
#endif

void Layout::onSetVisible( int show )
{
	disable_auto_alpha = 1;
	LAYOUT_PARENT::onSetVisible( show );

	if ( show )
		onShow();
	else
		onHide();

	Container *p = getParentContainer();

	if ( p )
		p->onChildSetLayoutVisible( this, show );

	disable_auto_alpha = 0;

	if ( wantDesktopAlpha() && isDesktopAlphaSafe() )
		desktopAlpha_autoTurnOn();
}

void Layout::scaleTo(int s)
{
	beginScale();
	setRenderRatio((double)s / 100.0);
	endScale();
}

void Layout::setRenderRatio(double s)
{
	if (isPostOnInit()
      #ifdef WASABI_COMPILE_CONFIG
      && cfg_uioptions_linkallratio.getValueAsInt() == 1 
#endif
      && !broadcasting)
	{
		broadcasting = 1;
		SkinParser::setAllLayoutsRatio(s);
		broadcasting = 0;
		return ;
	}
	if (getRenderRatio() == s) return ;
	LAYOUT_PARENT::setRenderRatio(s);
	if (reg) invalidateWindowRegion();
	invalidate();
	foreach(locked)
	locked.getfor()->setRenderRatio(s);
	endfor;
	if (lockedto && lockedto->getRenderRatio() != getRenderRatio())
		lockedto->setRenderRatio(getRenderRatio());
	script_vcpu_onScale(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_DOUBLE(s));
	if (!abortSaving()) savePosition();
}

void Layout::setAlpha(int a)
{
	int old = alpha;
	alpha = a;
	updateTransparency();
#ifdef WASABI_COMPILE_CONFIG
	if (old != alpha && getParentContainer())
	{
		StringPrintfW tmp(L"Skin:%s/Container:%s/Layout:%s/alpha", WASABI_API_SKIN->getSkinName(), getParentContainer()->getDescriptor(), getGuiObject()->guiobject_getId());
		IntArray::write(tmp, alpha);
	}
#endif
}

int Layout::getAlpha()
{
	return alpha;
}

void Layout::setPaintingAlpha(int activealpha, int inactivealpha)
{ // from basewnd, called by gui object
	LAYOUT_PARENT::setAlpha(activealpha, inactivealpha);
	updateTransparency();
}

int Layout::getPaintingAlpha()
{ // from basewnd, called by gui object
	return getAlpha();
}

int Layout::onActivate()
{
	LAYOUT_PARENT::onActivate();
	//  activateChildren(1);
	updateTransparency();
	return 1;
}

int Layout::onKillFocus()
{
	alphaMgr->hoverCheck(this);
	int r = LAYOUT_PARENT::onKillFocus();
	return r;
}

int Layout::onGetFocus()
{
	alphaMgr->hoverCheck(this);
	int r = LAYOUT_PARENT::onGetFocus();
	return r;
}

int Layout::onDeactivate()
{
	LAYOUT_PARENT::onDeactivate();
	//  activateChildren(0);
	updateTransparency();
	return 1;
}

void Layout::updateTransparency()
{
	alphaMgr->updateTransparency(this);
}

/*
void Layout::activateChildren(int act) {
  for (int i=0;i<gui_objects.getNumItems();i++) {
    GuiObject *o = gui_objects.enumItem(i);
    BaseWnd *b = NULL;
    if (o) 
      b = o->getBaseWnd();
    else
      ASSERT(0);
    if (b) {
      if (act)
        b->onActivate();
      else
        b->onDeactivate();
    }
  }
}*/

void Layout::center()
{
	RECT r;
	getNonClientRect(&r);
	RECT vw;
	Wasabi::Std::getViewport(&vw, NULL, NULL, gethWnd());
	int x = ((vw.right - vw.left) - (r.right - r.left)) / 2;
	int y = ((vw.bottom - vw.top) - (r.bottom - r.top)) / 2;
	move(x, y);
}

void Layout::setParentContainer(Container *c)
{
	p_container = c;
}

Container *Layout::getParentContainer()
{
	return p_container;
}

int Layout::isLayout()
{
	return 1;
}

void Layout::setInDesktop(int a)
{
	ASSERTPR(!isInited(), "cannot change indesktop after init");
	indesktop = a;
}

int Layout::getInDesktop()
{
	return indesktop;
}

int Layout::isDesktopAlphaSafe()
{
#ifdef WASABI_COMPILE_CONFIG
	if (galphadisabled) return 0;
	// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
	const GUID uioptions_guid =
	    { 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };
	if (!_intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid), L"Enable desktop alpha")) return 0;
#else
	if (!WASABI_WNDMGR_DESKTOPALPHA) return 0;
#endif
	return LAYOUT_PARENT::isDesktopAlphaSafe();
}

void Layout::setStatusText(const wchar_t *txt, int overlay)
{
	foreach(statuscbs)
	statuscbs.getfor()->onSetStatusText(txt, overlay);
	endfor;
}

void Layout::addAppCmds(AppCmds *commands)
{
	foreach(statuscbs)
	statuscbs.getfor()->onAddAppCmds(commands);
	endfor;
}

void Layout::removeAppCmds(AppCmds *commands)
{
	foreach(statuscbs)
	statuscbs.getfor()->onRemoveAppCmds(commands);
	endfor;
}

void Layout::pushCompleted(int max)
{
	foreach(statuscbs)
	statuscbs.getfor()->pushCompleted(max);
	endfor;
}
void Layout::incCompleted(int add)
{
	foreach(statuscbs)
	statuscbs.getfor()->incCompleted(add);
	endfor;
}
void Layout::setCompleted(int pos)
{
	foreach(statuscbs)
	statuscbs.getfor()->setCompleted(pos);
	endfor;
}
void Layout::popCompleted()
{
	foreach(statuscbs)
	statuscbs.getfor()->popCompleted();
	endfor;
}

void Layout::registerStatusCallback(GuiStatusCallback *lcb)
{
	statuscbs.addItem(lcb);
	viewer_addViewItem(lcb->status_getDependencyPtr());
}

int Layout::viewer_onItemDeleted(api_dependent *item)
{
	for (int i = 0;i < statuscbs.getNumItems();i++)
		if (statuscbs.enumItem(i)->status_getDependencyPtr() == item)
		{
			statuscbs.removeByPos(i--);
		}
	return 1;
}

void Layout::snapAdjust(int left, int right, int top, int bottom)
{
	snap_adjust_left = left;
	snap_adjust_top = top;
	snap_adjust_right = right;
	snap_adjust_bottom = bottom;
	script_vcpu_onSnapAdjustChanged(SCRIPT_CALL, getScriptObject());
  #ifdef USEAPPBAR
	if (appbar_isDocked()) appbar_posChanged();
#endif
	if (forceTransparencyFlag() || getAlpha() < 255) postDeferredCallback(DC_INVALIDATE);
}

void Layout::getSnapAdjust(RECT *r)
{
	if (!r) return ;
	r->left = snap_adjust_left;
	r->top = snap_adjust_top;
	r->right = snap_adjust_right;
	r->bottom = snap_adjust_bottom;
}

int Layout::abortSaving()
{
	GuiObject *o = getGuiObject();
	if (o->guiobject_movingToTarget()) return 1;
	return 0;
}

void Layout::setNoOffscreenCheck(int nocheck)
{
	killTimer(TIMER_OFFSCREENCHECK);

	if (nocheck != -1) nooffscreencheck = nocheck;

	setTimer(TIMER_OFFSCREENCHECK, 2500);
}

#ifdef USEAPPBAR

void Layout::onDock(int side)
{
	script_vcpu_onDock(SCRIPT_CALL, getScriptObject(), MAKE_SCRIPT_INT(side));
}

void Layout::onUnDock()
{
	script_vcpu_onUndock(SCRIPT_CALL, getScriptObject());
}

void Layout::appbar_onDock(int side)
{
	onDock(side);
	savePosition();
}

void Layout::appbar_onUnDock()
{
	onUnDock();
	savePosition();
}

void Layout::appbar_onSlide()
{
	savePosition();
}

int Layout::getAppBarAutoHide()
{
	return appbar_want_autohide;
}

void Layout::setAppBarAutoHide(int ah)
{
	appbar_want_autohide = ah;
	if (appbar_isDocked()) appbar_updateAutoHide();
	savePosition();
}

int Layout::getAppBarAlwaysOnTop()
{
	return appbar_want_alwaysontop;
}

void Layout::setAppBarAlwaysOnTop(int aot)
{
	appbar_want_alwaysontop = aot;
	if (appbar_isDocked()) appbar_updateAlwaysOnTop();
	savePosition();
}
#endif


// ------------------------------------------------------------------------

LayoutScriptController _layoutController;
LayoutScriptController *layoutController = &_layoutController;

// -- Functions table -------------------------------------
function_descriptor_struct LayoutScriptController::exportedFunction[] = {
            {L"onDock", 1, (void*)Layout::script_vcpu_onDock },
            {L"onUndock", 0, (void*)Layout::script_vcpu_onUndock },
            {L"getScale", 0, (void*)Layout::script_vcpu_getScale },
            {L"setScale", 1, (void*)Layout::script_vcpu_setScale },
            {L"onScale", 1, (void*)Layout::script_vcpu_onScale },
            {L"setDesktopAlpha", 1, (void*)Layout::script_vcpu_setDesktopAlpha },
            {L"getDesktopAlpha", 0, (void*)Layout::script_vcpu_getDesktopAlpha },
            {L"isTransparencySafe", 0, (void*)Layout::script_vcpu_isTransparencySafe},
            {L"isLayoutAnimationSafe", 0, (void*)Layout::script_vcpu_isLayoutAnimationSafe},
            {L"getContainer", 0, (void*)Layout::script_vcpu_getContainer },
            {L"center", 0, (void*)Layout::script_vcpu_center},
            {L"onMove", 0, (void*)Layout::script_vcpu_onMove},
            {L"onEndMove", 0, (void*)Layout::script_vcpu_onEndMove},
            {L"snapAdjust", 4, (void*)Layout::script_vcpu_snapAdjust},
            {L"getSnapAdjustTop", 0, (void*)Layout::script_vcpu_getSnapAdjustTop},
            {L"getSnapAdjustLeft", 0, (void*)Layout::script_vcpu_getSnapAdjustLeft},
            {L"getSnapAdjustRight", 0, (void*)Layout::script_vcpu_getSnapAdjustRight},
            {L"getSnapAdjustBottom", 0, (void*)Layout::script_vcpu_getSnapAdjustBottom},
            {L"onUserResize", 4, (void*)Layout::script_vcpu_onUserResize},
            {L"setRedrawOnResize", 1, (void*)Layout::script_vcpu_setRedrawOnResize},
            {L"beforeRedock", 0, (void*)Layout::script_vcpu_beforeRedock},
            {L"redock", 0, (void*)Layout::script_vcpu_redock},
            {L"onMouseEnterLayout", 0, (void*)Layout::script_vcpu_onMouseEnterLayout},
            {L"onMouseLeaveLayout", 0, (void*)Layout::script_vcpu_onMouseLeaveLayout},
            {L"onSnapAdjustChanged", 0, (void*)Layout::script_vcpu_onSnapAdjustChanged},
        };
// --------------------------------------------------------


const wchar_t *LayoutScriptController::getClassName()
{
	return L"Layout";
}

const wchar_t *LayoutScriptController::getAncestorClassName()
{
	return L"Group";
}

int LayoutScriptController::getInstantiable()
{
	return 1;
}

ScriptObject *LayoutScriptController::instantiate()
{
	Layout *l = new Layout;
	return l->getScriptObject();
}

void LayoutScriptController::destroy(ScriptObject *o)
{
	Group *g = static_cast<Group *>(o->vcpu_getInterface(groupGuid));
	if (g && GroupMgr::hasGroup(g))
	{
		GroupMgr::destroy(g);
		return ;
	}
	ASSERTALWAYS("you cannot destroy a static layout");
}

void *LayoutScriptController::encapsulate(ScriptObject *o)
{
	return NULL;
}

void LayoutScriptController::deencapsulate(void *o)
{}

int LayoutScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *LayoutScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID LayoutScriptController::getClassGuid()
{
	return layoutGuid;
}

// -------------------------------------------------------------------------

scriptVar Layout::script_vcpu_onDock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar side)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, layoutController, side);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, side);
}

scriptVar Layout::script_vcpu_onUndock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, layoutController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Layout::script_vcpu_getScale(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	double r = 0;
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) r = l->getRenderRatio();
	return MAKE_SCRIPT_DOUBLE(r);
}

scriptVar Layout::script_vcpu_setScale(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT
	double a = GET_SCRIPT_DOUBLE(s);
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) l->setRenderRatio(a);
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_onScale(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS1(o, layoutController, s);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT1(o, s);
}

scriptVar Layout::script_vcpu_setDesktopAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s)
{
	SCRIPT_FUNCTION_INIT
	bool a = GET_SCRIPT_BOOLEAN(s);
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) l->setWantDesktopAlpha(a);
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_getDesktopAlpha(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) return MAKE_SCRIPT_BOOLEAN(l->wantDesktopAlpha() && l->isDesktopAlphaSafe());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layout::script_vcpu_isTransparencySafe(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) return MAKE_SCRIPT_BOOLEAN(l->isTransparencySafe());
	RETURN_SCRIPT_ZERO;
}

scriptVar Layout::script_vcpu_isLayoutAnimationSafe(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) return MAKE_SCRIPT_BOOLEAN(!l->forceTransparencyFlag() && (l->getAlphaMgr()->getAlpha(l) == 255));
	RETURN_SCRIPT_ZERO;
}

scriptVar Layout::script_vcpu_getContainer(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l)
	{
		Container *c = l->getParentContainer();
		return MAKE_SCRIPT_OBJECT(c ? c->getScriptObject() : NULL);
	}
	return MAKE_SCRIPT_OBJECT(NULL);
}

scriptVar Layout::script_vcpu_center(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) l->center();
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_setRedrawOnResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) l->setWantRedrawOnResize(GET_SCRIPT_INT(v));
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_beforeRedock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) windowTracker->beforeRedock(l, &l->redock);
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_redock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) windowTracker->afterRedock(l, &l->redock);
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_onEndMove(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, layoutController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Layout::script_vcpu_onMouseEnterLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, layoutController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Layout::script_vcpu_onMouseLeaveLayout(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, layoutController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Layout::script_vcpu_onSnapAdjustChanged(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, layoutController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Layout::script_vcpu_onUserResize(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y, scriptVar w, scriptVar h)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS4(o, layoutController, x, y, w, h);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT4(o, x, y, w, h);
}

scriptVar Layout::script_vcpu_onMove(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT;
	PROCESS_HOOKS0(o, layoutController);
	SCRIPT_FUNCTION_CHECKABORTEVENT;
	SCRIPT_EXEC_EVENT0(o);
}

scriptVar Layout::script_vcpu_snapAdjust(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar left, scriptVar top, scriptVar right, scriptVar bottom)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l) l->snapAdjust(GET_SCRIPT_INT(left), GET_SCRIPT_INT(top), GET_SCRIPT_INT(right), GET_SCRIPT_INT(bottom));
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_getSnapAdjustLeft(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l)
	{
		RECT r;
		l->getSnapAdjust(&r);
		return MAKE_SCRIPT_INT(r.left);
	}
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_getSnapAdjustTop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l)
	{
		RECT r;
		l->getSnapAdjust(&r);
		return MAKE_SCRIPT_INT(r.top);
	}
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_getSnapAdjustRight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l)
	{
		RECT r;
		l->getSnapAdjust(&r);
		return MAKE_SCRIPT_INT(r.right);
	}
	RETURN_SCRIPT_VOID;
}

scriptVar Layout::script_vcpu_getSnapAdjustBottom(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	Layout *l = static_cast<Layout *>(o->vcpu_getInterface(layoutGuid));
	if (l)
	{
		RECT r;
		l->getSnapAdjust(&r);
		return MAKE_SCRIPT_INT(r.bottom);
	}
	RETURN_SCRIPT_VOID;
}

int Layout::broadcasting = 0;

StringW Layout::MakePrefix()
{
	return StringPrintfW(L"Skin:%s/Container:%s/Layout:%s", WASABI_API_SKIN->getSkinName(), getParentContainer()->getDescriptor(), getGuiObject()->guiobject_getId());
}