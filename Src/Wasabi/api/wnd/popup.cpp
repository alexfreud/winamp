#include <precomp.h>
#include <api/wnd/api_window.h>
#include <api/locales/xlatstr.h>

#ifndef WASABI_WANT_FF_POPUP

#include "popup.h"
#ifdef _WIN32
#include "../../../Plugins/General/gen_ff/wa2frontend.h"
extern HINSTANCE hInstance;
#endif
PopupMenu::PopupMenu()
{
	hmenu = NULL;
	parent = NULL;
}

PopupMenu::PopupMenu(ifc_window *_parent)
{
	hmenu = NULL;
	parent = _parent;
}

PopupMenu::PopupMenu(PopupMenu *_parent)
{
	hmenu = NULL;
	parent = _parent->getParent();
}

PopupMenu::~PopupMenu()
{
	invalidateMenu();
	entries.deleteAll();
	sortedentries.removeAll();
}

void PopupMenu::addSubMenu(PopupMenu *menu, const wchar_t *text, int disabled)
{
	invalidateMenu();
	PopupMenuEntry *e = new PopupMenuEntry(text, -1, menu, 0, disabled);
	entries.addItem(e);
	sortedentries.addItem(e);
}

void PopupMenu::addSubMenuCallback(const wchar_t *text, PopupMenuCallback *cb, int param)
{
	invalidateMenu();
	PopupMenuEntry *e = new PopupMenuEntry(text, -1, NULL, 0, 0, cb, param);
	entries.addItem(e);
	sortedentries.addItem(e);
}

void PopupMenu::addCommand(const wchar_t *text, int command, int checked, int disabled, int addpos)
{
	invalidateMenu();
	PopupMenuEntry *e = new PopupMenuEntry(text, command, NULL, checked, disabled);
	entries.addItem(e);
	sortedentries.addItem(e, addpos);
}

void PopupMenu::addSeparator(int addpos)
{
	invalidateMenu();
	PopupMenuEntry *e = new PopupMenuEntry(NULL, -1, NULL, 0, 0);
	entries.addItem(e);
	sortedentries.addItem(e, addpos);
};

void PopupMenu::checkCommand(int cmd, int check)
{
	invalidateMenu();
	PopupMenuEntry *e = sortedentries.findItem((const wchar_t *) & cmd);
	if (e) e->setChecked(check);
}

void PopupMenu::disableCommand(int cmd, int disable)
{
	invalidateMenu();
	PopupMenuEntry *e = sortedentries.findItem((const wchar_t *) & cmd);
	if (e) e->setDisabled(disable);
}

int PopupMenu::popAtXY(int x, int y, int native)
{
	rebuildMenu();
#ifdef _WIN32
	if (!native)
		return DoTrackPopup(getOSMenuHandle(), TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, x, y, (parent ? parent->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd())) - 1;
	else
		return TrackPopupMenuEx(getOSMenuHandle(), TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, x, y, (parent ? parent->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd()), NULL) - 1;
#elif defined(__APPLE__)
  return PopUpMenuSelect(getOSMenuHandle(), x, y, 0);
#endif
}

int PopupMenu::popAnchored(int type)
{	// dropped off the sourceWnd given above
#ifdef _WIN32
	int flag = 0;
	switch (type)
	{
	case POPUP_ANCHOR_UL: flag |= TPM_LEFTALIGN | TPM_TOPALIGN; break;
	case POPUP_ANCHOR_LL: flag |= TPM_LEFTALIGN | TPM_BOTTOMALIGN; break;
	case POPUP_ANCHOR_UR: flag |= TPM_RIGHTALIGN | TPM_TOPALIGN; break;
	case POPUP_ANCHOR_LR: flag |= TPM_RIGHTALIGN | TPM_BOTTOMALIGN; break;
	}
#endif
	rebuildMenu();
	int x, y;
	Wasabi::Std::getMousePos(&x, &y);
  #ifdef _WIN32
	return DoTrackPopup(getOSMenuHandle(), flag | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, x, y, (parent ? parent->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd())) - 1;
#elif defined(__APPLE__)
  return PopUpMenuSelect(getOSMenuHandle(), x, y, 0);
#endif
}

int PopupMenu::popAtMouse()
{
	rebuildMenu();
	int x, y;
	Wasabi::Std::getMousePos(&x, &y);
#ifdef _WIN32
	return DoTrackPopup(getOSMenuHandle(), TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, x, y, (parent ? parent->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd())) - 1;
#elif defined(__APPLE__)
  return PopUpMenuSelect(getOSMenuHandle(), x, y, 0);
#endif
}

int PopupMenu::getNumCommands()
{
	return entries.getNumItems();
}

const wchar_t *PopupMenu::getCommandText(int command)
{
	PopupMenuEntry *e = sortedentries.findItem((const wchar_t *) &command);
	if (e) return e->getText();
	return NULL;
}

OSMENUHANDLE PopupMenu::getOSMenuHandle()
{
	rebuildMenu();
	return hmenu;
}

void PopupMenu::rebuildMenu()
{
#ifdef WIN32
	if (hmenu != NULL) return ;
	hmenu = CreatePopupMenu();
	int i = 0;
	foreach(entries)
	PopupMenuEntry *e = entries.getfor();
	OSMENUHANDLE submenu = NULL;
	if (e->getCallback())
	{
		PopupMenu *m = e->getCallback()->popupMenuCallback(this, e->getCallbackParam());
		if (m) submenu = m->getOSMenuHandle();
	}
	else if (e->isSubmenu())
	{
		submenu = e->getSubmenu()->getOSMenuHandle();
	}
	InsertMenuW(hmenu, i++, MF_BYPOSITION | (e->getChecked() ? MF_CHECKED : MF_UNCHECKED) | (e->getDisabled() ? MF_GRAYED : 0) | (e->isSeparator() ? MF_SEPARATOR : (e->isSubmenu() ? MF_POPUP : 0) | (e->getText() ? MF_STRING : 0)), e->isSubmenu() ? (UINT_PTR)submenu : e->getCommand() + 1, e->getText());
	endfor;
#else
  if (hmenu != NULL) return ;
  CreateNewMenu(0, 0, &hmenu);

  foreach(entries)
  PopupMenuEntry *e = entries.getfor();
	OSMENUHANDLE submenu = NULL;
	if (e->getCallback())
	{
		PopupMenu *m = e->getCallback()->popupMenuCallback(this, e->getCallbackParam());
		if (m) submenu = m->getOSMenuHandle();
	}
	else if (e->isSubmenu())
	{
		submenu = e->getSubmenu()->getOSMenuHandle();
	}
  
  const wchar_t *name = e->getText();
  CFStringRef menuStr = CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)name, wcslen(name)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
  MenuItemIndex newMenuItem;
  MenuItemAttributes menuAttr = kMenuItemAttrIgnoreMeta;
  if (e->getDisabled())
    menuAttr|=kMenuItemAttrDisabled;
  if (e->isSeparator())
    menuAttr|=kMenuItemAttrSeparator;

  AppendMenuItemTextWithCFString(hmenu, menuStr, menuAttr, e->getCommand(), &newMenuItem);
  if (submenu)
    SetMenuItemHierarchicalMenu(hmenu, newMenuItem, submenu);

  if (e->getChecked())
    CheckMenuItem(hmenu, newMenuItem, true);
  
  CFRelease(menuStr);
  endfor;  
#endif
}

void PopupMenu::invalidateMenu()
{
#ifdef WIN32
	if (hmenu) DestroyMenu(hmenu);
#elif defined(__APPLE__)
  if (hmenu) DisposeMenu(hmenu);
#endif
	hmenu = NULL;
}

void PopupMenu::abort()
{
#ifdef WIN32
	HWND w = (parent ? parent->gethWnd() : WASABI_API_WND->main_getRootWnd()->gethWnd());

	PostMessage(w, WM_LBUTTONDOWN, 0, 0xdeadc0de);
	PostMessage(w, WM_LBUTTONUP, 0, 0xdeadc0de);

#elif defined(__APPLE__)
  CancelMenuTracking(hmenu, true, 0);
#endif
}

#else // WASABI_WANT_FF_POPUP

#include <bfc/api/api_wnd.h>
#include <bfc/api/api_syscb.h>
#include "popup.h"
#include <bfc/notifmsg.h>

#include <studio/assert.h>
#include <studio/api.h>

#include <bfc/wasabi_std.h>
#include <common/xlatstr.h>
#include <bfc/wnds/buttwnd.h>

#include <bfc/util/pathparse.h>
#include <bfc/attribs/cfgitem.h>

#include <common/script/c_script/c_guiobject.h>
#include <common/script/c_script/c_menubutton.h>
#include <common/script/scriptguid.h>
#include <common/menusurface.h>

#ifndef WANT_NEW_POPUPMENU
#define SELMARGIN 1

// todo:
// check marks
// more?

PopupMenu::PopupMenu(ifc_window *sourceWnd)
{
	ASSERT(sourceWnd != NULL);
	setStartHidden(1);
	setRenderRatio(1.0);
	parentRootWnd = sourceWnd;
	parentWnd = sourceWnd->getOsWindowHandle();
	reverse_side = 0;
	kbdhooked = 0;
	keyctrl = 0;
	toplevelmenu = 0;
	chainmenu = NULL;
	lastxy.x = lastxy.y = -1;
	WASABI_API_WND->appdeactivation_push_disallow(this);
	init(sourceWnd->getOsModuleHandle(), parentWnd, TRUE);
	WASABI_API_WND->appdeactivation_pop_disallow(this);
}

PopupMenu::PopupMenu()
{
	setStartHidden(1);
	setRenderRatio(1.0);
	parentRootWnd = NULL;
	parentWnd = INVALIDOSWINDOWHANDLE;
	chainmenu = NULL;
	reverse_side = 0;
	kbdhooked = 0;
	toplevelmenu = 0;
	keyctrl = 0;
	lastxy.x = lastxy.y = -1;
	WASABI_API_WND->appdeactivation_push_disallow(this);
	init(hInstance, WASABI_API_WND->main_getRootWnd()->gethWnd(), TRUE);
	WASABI_API_WND->appdeactivation_pop_disallow(this);
}

PopupMenu::PopupMenu(HWND sourceWnd)
{
	parentRootWnd = NULL;
	parentWnd = sourceWnd;
	setStartHidden(1);
	setRenderRatio(1.0);
	reverse_side = 0;
	kbdhooked = 0;
	toplevelmenu = 0;
	keyctrl = 0;
	chainmenu = NULL;
	lastxy.x = lastxy.y = -1;
	WASABI_API_WND->appdeactivation_push_disallow(this);
	init(hInstance, sourceWnd, TRUE);
	WASABI_API_WND->appdeactivation_pop_disallow(this);
}

PopupMenu::PopupMenu(PopupMenu *sourceWnd)
{
	parentRootWnd = sourceWnd;
	parentWnd = sourceWnd->gethWnd();
	setStartHidden(1);
	setRenderRatio(1.0);
	reverse_side = 0;
	kbdhooked = 0;
	toplevelmenu = 0;
	chainmenu = NULL;
	keyctrl = 0;
	lastxy.x = lastxy.y = -1;
	WASABI_API_WND->appdeactivation_push_disallow(this);
	init(sourceWnd, TRUE);
	WASABI_API_WND->appdeactivation_pop_disallow(this);
}

int PopupMenu::onInit()
{
	POPUPMENU_PARENT::onInit();
	bdown = 0;
	lastitem = -1;
	rcp = 0;
	openmenuid = -1;
	timerset = 0;
	timeritem = -1;
	rcode = -1;
	toplevelmenu = 0;
	disable_autopop = 0;
	popupdelay = 250; //TODO: Config
#ifdef WASABI_COMPILE_CONFIG
	// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
	const GUID uioptions_guid =
	    { 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };
	setTransparency(_intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid), "Popup menu alpha", 255));
#else
setTransparency(255);
#endif

	tex = "wasabi.popup.menu.background";
	ful = "wasabi.popup.menu.border.topLeft";
	fur = "wasabi.popup.menu.border.topRight";
	fll = "wasabi.popup.menu.border.bottomLeft";
	flr = "wasabi.popup.menu.border.bottomRight";
	fl = "wasabi.popup.menu.border.left";
	fr = "wasabi.popup.menu.border.right";
	ft = "wasabi.popup.menu.border.top";
	fb = "wasabi.popup.menu.border.bottom";
	sl = "wasabi.popup.menu.selection.left";
	sr = "wasabi.popup.menu.selection.right";
	sc = "wasabi.popup.menu.selection.center";

	return 1;
}

PopupMenu::~PopupMenu()
{
	if (kbdhooked)
	{
		WASABI_API_WND->unhookKeyboard(this);
		kbdhooked = 0;
	}
	int x, n;
	n = items.getNumItems();
	for (x = 0; x < n; x ++)
	{
		if (items[x])
		{
			if (items[x]->butt) delete items[x]->butt;
			if (items[x]->menu) delete items[x]->menu;
			delete items[x];
		}
	}
}

void PopupMenu::addSubMenu(PopupMenu *menu, const wchar_t *text, int disabled)
{
	ASSERT(text != NULL);
	ButtonWnd *b = new ButtonWnd();
	b->init(this);
	b->setParent(this);
	b->setNotifyId( -1);
	b->setButtonText(translateButtonText(text), 14);
	//  b->setTextJustification(BUTTONJUSTIFY_LEFT);
	b->setTextAlign(TEXTALIGN_LEFT);
	b->setColors("wasabi.popup.text", "wasabi.popup.hiliteText", "wasabi.button.dimmedText");
	if (disabled) b->enableButton(FALSE);
	ItemT *t = new ItemT;
	t->issep = 0;
	t->butt = b;
	t->menu = menu;
	t->cmd = -1;
	t->cb = NULL;
	items.addItem(t);
}

void PopupMenu::addSubMenuImage(PopupMenu *menu, const char *bitmap, const char *pushedbitmap, const char *highlightbitmap)
{
	ButtonWnd *b = new ButtonWnd();
	b->init(this);
	b->setParent(this);
	b->setBitmaps(bitmap, pushedbitmap, highlightbitmap);
	b->setBitmapCenter(1);
	b->setNotifyId( -1);
	b->setAutoDim(1);
	b->setColors("wasabi.popup.text", "wasabi.popup.hiliteText", "wasabi.button.dimmedText");
	ItemT *t = new ItemT;
	t->issep = 0;
	t->butt = b;
	t->menu = menu;
	t->cb = NULL;
	t->cmd = -1;
	items.addItem(t);
}

void PopupMenu::addSubMenuCallback(const wchar_t *text, PopupMenuCallback *cb, int param)
{
	ASSERT(text != NULL);
	ASSERT(cb != NULL);
	ButtonWnd *b = new ButtonWnd();
	b->init(this);
	b->setParent(this);
	b->setNotifyId( -1);
	b->setButtonText(translateButtonText(text), 14);
	//  b->setTextJustification(BUTTONJUSTIFY_LEFT);
	b->setTextAlign(TEXTALIGN_LEFT);
	b->setColors("wasabi.popup.text", "wasabi.popup.hiliteText", "wasabi.button.dimmedText");
	ItemT *t = new ItemT;
	t->issep = 0;
	t->butt = b;
	t->menu = 0;
	t->cb = cb;
	t->cbparam = param;
	t->cmd = -1;
	items.addItem(t);
}

void PopupMenu::addSeparator(int addpos)
{
	ButtonWnd *b = new ButtonWnd();
	b->init(this);
	b->setParent(this);
	b->setNotifyId( -1);
	b->setBitmaps("wasabi.popup.menu.seperator");
	b->setBitmapCenter(0);
	b->enableButton(0);
	b->setColors("wasabi.popup.text", "wasabi.popup.hiliteText", "wasabi.button.dimmedText");
	ItemT *t = new ItemT;
	t->issep = 1;
	t->butt = b;
	t->menu = 0;
	t->cb = NULL;
	t->cmd = -1;
	items.addItem(t, addpos);
}

void PopupMenu::addCommandImage(const char *bitmap, const char *pushedbitmap, const char *highlightbitmap, int command, int checked, int disabled, int addpos)
{
	ButtonWnd *b = new ButtonWnd();
	b->init(this);
	b->setParent(this);
	b->setBitmaps(bitmap, pushedbitmap, highlightbitmap);
	b->setBitmapCenter(1);
	b->setNotifyId(command);
	b->enableButton(disabled ? 0 : 1);
	b->setChecked(checked ? 1 : 0);
	b->setAutoDim(1);
	b->setColors("wasabi.popup.text", "wasabi.popup.hiliteText", "wasabi.button.dimmedText");
	ItemT *t = new ItemT;
	t->issep = 0;
	t->butt = b;
	t->menu = 0;
	t->cb = NULL;
	t->cmd = -1;
	items.addItem(t);
}

void PopupMenu::addCommand(const wchar_t *_txt, int command, int checked, int disabled, int addpos)
{
	if (!_txt)
	{
		addSeparator();
		return ;
	}
	String txt = translateButtonText(_txt);
	ButtonWnd *b = new ButtonWnd();
	b->init(this);
	b->setParent(this);
	b->setNotifyId(disabled ? -1 : command);

#ifdef WASABI_COMPILE_LOCALES
	const wchar_t *bind = (command != 0) ? WASABI_API_LOCALE->locales_getBindFromAction(command) : NULL;
	if (bind) txt += StringPrintfW(L"\t%s", bind);
#endif

	b->setButtonText(txt);

	//  b->setTextJustification(BUTTONJUSTIFY_LEFT);
	b->setTextAlign(TEXTALIGN_LEFT);
	b->enableButton(disabled ? 0 : 1);
	b->setChecked(checked);
	if (checked == 2) b->setAlpha(128);
	b->setColors("wasabi.popup.text", "wasabi.popup.hiliteText", "wasabi.button.dimmedText");
	ItemT *t = new ItemT;
	t->issep = 0;
	t->butt = b;
	t->menu = 0;
	t->cmd = command;
	t->cb = NULL;
	ASSERT(PTRLIST_POS_LAST == -1); //BU
	items.addItem(t, addpos);
}

void PopupMenu::disableCommand(int cmd, int disable)
{
	for (int i = 0;i < items.getNumItems();i++)
		if (items.enumItem(i)->cmd == cmd)
		{
			if (items.enumItem(i)->butt)
				items.enumItem(i)->butt->enableButton(!!!disable);
			break;
		}
}

void PopupMenu::checkCommand(int cmd, int check)
{
	for (int i = 0;i < items.getNumItems();i++)
		if (items.enumItem(i)->cmd == cmd)
		{
			if (items.enumItem(i)->butt)
				items.enumItem(i)->butt->setChecked(check);
			break;
		}
}

const wchar_t *PopupMenu::getCommandText(int command)
{
	for (int i = 0;i < items.getNumItems();i++)
		if (items.enumItem(i)->cmd == command && items.enumItem(i)->butt)
			return items.enumItem(i)->butt->getName();
	return NULL;
}

int PopupMenu::popAtXY(int x, int y)
{
	toplevelmenu = 1;
	rcode = -1;
	if (items.getNumItems())
	{
		POINT pt = {x, y};

#ifdef WIN32
		HWND oldcw = GetCapture(); // nonportable
		ifc_window *oldcrw = (ifc_window*)GetWindowLong(oldcw, GWL_USERDATA);
		if (oldcrw != NULL) oldcrw->cancelCapture();
#endif

		WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI*>(this));

		showAtXY(x, y, &rcode);
		beginCapture();

		MSG msg;

#ifdef WIN32
		SetCursor(LoadCursor(NULL, IDC_ARROW)); // NONPORTABLE
#endif
		while (rcode == -1 && GetMessage( &msg, INVALIDOSWINDOWHANDLE, 0, 0 ))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		endCapture();
		hide();

		//    if (hadcapture && under) under->beginCapture();

#ifdef WIN32
		if (rcode == -2 || rcode == -3)
		{
			DWORD p = GetMessagePos();
			POINT pt;
			pt.x = (signed short)LOWORD(p);
			pt.y = (signed short)HIWORD(p);
			HWND w = WindowFromPoint(pt);
			ScreenToClient(w, &pt);
			p = (pt.x & 0xFFFF) | (pt.y << 16);
			//if (under) under->getRootParent()->wndProc(under->getRootParent()->gethWnd(), WM_MOUSEMOVE, 0, p);
			//PostMessage(w, (rcode == -2) ? WM_MOUSEMOVE : WM_RBUTTONDOWN, 0, p);
			//PostMessage(w, (rcode == -2) ? WM_LBUTTONDOWN : WM_RBUTTONDOWN, 0, p);
		}
#else
DebugString("portme: popup.cpp pass on click");
#endif

	}

	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI*>(this));

	//DebugString("appdeactivation_pop_disallow\n");
	WASABI_API_WND->appdeactivation_pop_disallow(this);

	onPostPop(rcode);

#ifdef WASABI_COMPILE_SKIN
	if (!switchskinto.isempty())
	{
		WASABI_API_SKIN->skin_switchSkin(switchskinto.getNonConstVal());
		switchskinto = "";
	}
#endif

	return rcode;
}

void PopupMenu::showAtXY(int x, int y, int *rc, int revside, int parentW)
{
	int i, n;
	int w = 0, h = 0;
	lastitem = -1;
	rcp = rc;
	n = items.getNumItems();
	for (i = 0; i < n; i ++)
	{
		if (items[i]->menu || items[i]->cb)
			items[i]->butt->setRightBitmap("wasabi.popup.menu.submenu");
		if (!items[i]->butt->getChecked()) items[i]->butt->setChecked( -1);
		h += items[i]->butt->getHeight();
		items[i]->butt->setUseBaseTexture(0);
		items[i]->butt->setBorders(0);
		int tw = items[i]->butt->getWidth();
		if (w < tw)w = tw;
	}
	int neww = w + 6 + fl.getWidth() + fr.getWidth();
	int newh = h + 6 + ft.getHeight() + fb.getHeight();

	POINT p = {x, y};
	RECT vp;
	Std::getViewport(&vp, &p);

	// maintain parent's reversal state
	reverse_side = revside;
	int savx = x;
	if (reverse_side) x -= (neww + parentW);
	if (x + neww > vp.right || x < 0)
	{
		reverse_side = !reverse_side;
		x = savx;
		if (reverse_side) x -= (neww + parentW);
	}

	if (y + newh > vp.bottom) y -= newh;
	if (x < vp.left) x = vp.left;
	if (y < vp.top) y = vp.top;

	resize(x, y, neww, newh);

	h = 0;
	for (i = 0; i < n; i ++)
	{
		int lh = h;
		h += items[i]->butt->getHeight();
		items[i]->butt->resize(3 + fl.getWidth(), 3 + lh + ft.getHeight(), w, h - lh);
		items[i]->butt->setHilite(0);
		items[i]->butt->setPushed(0);
	}

	WASABI_API_WND->appdeactivation_push_disallow(this);
#ifdef WIN32
	SetWindowPos(gethWnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#elif defined(LINUX)
Atom NET_STATE = XInternAtom( Linux::getDisplay(), "_NET_WM_STATE", True );
Atom state[2];
state[0] = XInternAtom( Linux::getDisplay(), "_NET_WM_STATE_SKIP_TASKBAR", True );
state[1] = XInternAtom( Linux::getDisplay(), "_NET_WM_STATE_ABOVE", True );

if ( NET_STATE && state[0] && state[1] )
{
	XChangeProperty( Linux::getDisplay(), gethWnd(), NET_STATE, XA_ATOM, 32,
					 PropModeReplace, (unsigned char *)state, 2 );
}
#endif
	WASABI_API_WND->appdeactivation_pop_disallow(this);
	setVisible(1);
}

void PopupMenu::onSetVisible(int v)
{
	POPUPMENU_PARENT::onSetVisible(v);
	if (v && !kbdhooked)
	{
		WASABI_API_WND->hookKeyboard(this);
		kbdhooked = 1;
	}
	else if (!v && kbdhooked)
	{
		WASABI_API_WND->unhookKeyboard(this);
		kbdhooked = 0;
	}

}

void PopupMenu::hide()
{
	if (lastitem >= 0 && items[lastitem]->menu)
	{
		items[lastitem]->menu->hide();
		lastitem = -1;
	}
	setVisible(0);
}

int PopupMenu::popAnchored(int type)
{
	RECT wr;
	if (parentRootWnd != NULL)
	{
		parentRootWnd->getWindowRect(&wr);
	}
	else if (parentWnd != INVALIDOSWINDOWHANDLE)
	{
#ifdef WIN32
		GetWindowRect(parentWnd, &wr);
#else
DebugString("portme PopupMenu::popAnchored\n");
#endif

	}
	else
	{
		ASSERTALWAYS("can't call popAnchored without instantiating with a parent window");
	}
	switch (type)
	{
	case POPUP_ANCHOR_UL: return popAtXY(wr.left, wr.top);
	case POPUP_ANCHOR_LL: return popAtXY(wr.left, wr.bottom);
	case POPUP_ANCHOR_UR: return popAtXY(wr.right, wr.top);
	case POPUP_ANCHOR_LR: return popAtXY(wr.right, wr.bottom);
	}
	return 0;
}

int PopupMenu::popAtMouse()
{
	int x, y;
	Std::getMousePos(&x, &y);
	return popAtXY(x, y);
}

int PopupMenu::childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2)
{
	if (msg == ChildNotify::BUTTON_LEFTPUSH || msg == ChildNotify::BUTTON_RIGHTPUSH)
	{
		for (int i = 0;i < items.getNumItems();i++)
		{
			if (child == items[i]->butt && (items[i]->menu || items[i]->cb))
			{
				if (!items[i]->butt->getEnabled()) continue;
				if (items[i]->cb) initMenuCallback(i);
				if (!items[i]->menu) continue;
				if (openmenuid == i) continue;
				RECT r;
				if (openmenuid >= 0)
				{
					items[openmenuid]->menu->hide();
					openmenuid = -1;
				}
				items[i]->butt->getWindowRect(&r);
				PopupMenu *p = items[i]->menu;
				p->showAtXY(r.right, r.top, rcp, reverse_side, r.right - r.left);
				if (p1)
				{
					p->selectFirst();
				}
				openmenuid = i;
			}
		}
		// mig: changed this to call getNotifyId();
		//    *rcp=p1;
		*rcp = child->getNotifyId();
		return 0;
	}
	if (msg == ChildNotify::POPUP_SUBMENUCLOSE)
	{
		for (int i = 0;i < items.getNumItems();i++)
		{
			if (child == items[i]->menu)
			{
				if (openmenuid != i) continue;
				items[openmenuid]->menu->hide();
				openmenuid = -1;
				return 0;
			}
		}
	}
	return POPUPMENU_PARENT::childNotify(child, msg, p1, p2);
}

void PopupMenu::selectFirst()
{
	ButtonWnd *b = NULL;
	int i;
	for (i = 0; i < items.getNumItems(); i++)
	{
		b = items.enumItem(i)->butt;
		if (b != NULL) break;
	}
	if (b == NULL) return ;
	lastitem = i;
	b->setHilite(1);
	invalidate();
}

int PopupMenu::getWhichItem(POINT &p)
{
	int x, n;
	RECT r2;
	getWindowRect(&r2);
	n = items.getNumItems();
	for (x = 0; x < n; x ++)
	{
		RECT r;
		items[x]->butt->getWindowRect(&r);
		r.right = r2.right;
		r.left = r2.left;
		if (Std::pointInRect(r, p))
		{
			return x;
		}
	}
	return -1;
}

int PopupMenu::isMine(int x, int y)
{
	RECT r;
	getWindowRect(&r);
	POINT p = {x, y};
	if (Std::pointInRect(r, p)) return 1;
	if (lastitem >= 0 && items[lastitem]->menu && items[lastitem]->menu->isMine(x, y)) return 1;
	return 0;
}

void PopupMenu::setFriendlyId(const char *id)
{
	friendid = id;
}

int PopupMenu::onLeftButtonDown(int x, int y)
{
	clientToScreen(&x, &y);
	onButtonDown(1, x, y);
	return 0;
}

int PopupMenu::onRightButtonDown(int x, int y)
{
	clientToScreen(&x, &y);
	onButtonDown(2, x, y);
	return 0;
}

int PopupMenu::onMouseMove(int x, int y)
{
	POPUPMENU_PARENT::onMouseMove(x, y);
	POINT pnt = {x, y};
	clientToScreen(&pnt);

	if (keyctrl && lastxy.x == pnt.x && lastxy.y == pnt.y) return 1;
	keyctrl = 0;
	lastxy = pnt;

	if (lastitem >= 0)
	{
		if (openmenuid >= 0 && items[openmenuid]->menu && items[openmenuid]->menu->isMine(pnt.x, pnt.y))
		{
			if (lastitem != openmenuid)
			{
				items[lastitem]->butt->setHilite(0);
				items[lastitem]->butt->setPushed(0);
				items[openmenuid]->butt->setHilite(1);
				items[openmenuid]->butt->setPushed(1);
				invalidateItem(lastitem);
				lastitem = openmenuid;
				invalidateItem(lastitem);
			}
			resetTimer( -1);
			items[openmenuid]->menu->screenToClient(&pnt);
			items[openmenuid]->menu->onMouseMove(pnt.x, pnt.y);
			return 0;
		}
	}

	int p = getWhichItem(pnt);
	if (p >= 0)
	{
		ItemT *it = items[p];
		if (!it->issep)
		{
			if (p != lastitem)
			{
				if (lastitem >= 0)
				{
					/*        if (items[lastitem]->menu)
					{
					items[lastitem]->menu->hide();
					}*/
					items[lastitem]->butt->setHilite(0);
					items[lastitem]->butt->setPushed(0);
					invalidateItem(lastitem);
				}
				invalidateItem(lastitem);
				lastitem = p;
				invalidateItem(lastitem);
				items[p]->butt->setHilite(1);
				if (bdown) items[p]->butt->setPushed(1);
				/*      if (items[p]->menu) {
				RECT r;
				items[p]->butt->getWindowRect(&r);
				items[p]->menu->showAtXY(r.right,r.top,rcp);
				}*/
				resetTimer(p);
				invalidateItem(lastitem);
			}
		}
		else
		{
			RECT _r;
			getClientRect(&_r);
			int inside = (x >= 0 && y >= 0 && x <= _r.right - _r.left && y <= _r.bottom - _r.top);
			if (lastitem >= 0 && !inside)
			{
				items[lastitem]->butt->setHilite(0);
				items[lastitem]->butt->setPushed(0);
				invalidateItem(lastitem);
				lastitem = -1;
			}
		}
	}
	else
	{
		if (!friendid.isempty())
		{
			ifc_window *w = WASABI_API_WND->rootWndFromPoint(&pnt);
			if (w != NULL)
			{
				MenuButtonSurface *s = static_cast<MenuButtonSurface *>(w->getInterface(menuButtonSurfaceGuid));
				if (s != NULL)
				{
					if (s->getParentWnd() != parentRootWnd)
					{
						const char *str = s->getParentMenuId();
						if (STRCASEEQLSAFE(str, friendid))
						{
							abort();
							rcode = -4;
							chainmenu = s;
						}
					}
				}
			}
		}
	}
	return 0;
}

void PopupMenu::resetTimer(int p)
{
	if (timerset)
	{
		killTimer(POPUP_TIMERID);
		timeritem = -1;
	}
	if (p >= 0 && !disable_autopop)
	{
		setTimer(POPUP_TIMERID, popupdelay);
		timeritem = p;
		timerset = 1;
	}
}

void PopupMenu::timerCallback(int id)
{
	switch (id)
	{
	case POPUP_TIMERID:
		killTimer(POPUP_TIMERID);
		timerset = 0;
		if (timeritem == openmenuid)
		{
			timeritem = -1;
			break;
		}
		if (openmenuid >= 0 && items[openmenuid]->menu)
		{
			items[openmenuid]->menu->hide();
			openmenuid = -1;
		}
		if (timeritem >= 0)
		{
			if (items[timeritem]->cb) initMenuCallback(timeritem);
			if (items[timeritem]->butt->getEnabled() && items[timeritem]->menu)
			{
				RECT r;
				items[timeritem]->butt->getWindowRect(&r);
				items[timeritem]->menu->showAtXY(r.right, r.top, rcp, reverse_side, r.right - r.left);
				openmenuid = timeritem;
			}
			timeritem = -1;
		}
		break;
	default:
		POPUPMENU_PARENT::timerCallback(id);
		break;
	}
}

void PopupMenu::initMenuCallback(int item)
{
	int a = rcode;
	rcode = 0;
	PopupMenu *p = items[item]->cb->popupMenuCallback(this, items[item]->cbparam);
	rcode = a;
	if (p)
	{
		items[item]->cb = NULL;
		items[item]->menu = p;
	}
}

int PopupMenu::onLeftButtonUp(int x, int y)
{
	clientToScreen(&x, &y);
	onButtonUp(1, x, y);
	return 0;
}

int PopupMenu::onRightButtonUp(int x, int y)
{
	clientToScreen(&x, &y);
	onButtonUp(2, x, y);
	return 0;
}

void PopupMenu::onButtonDown(int wb, int x, int y)
{
	POINT pos = {x, y};
	RECT r;
	bdown |= wb;
	if (lastitem >= 0)
	{
		if (items[lastitem]->menu && items[lastitem]->menu->isMine(pos.x, pos.y))
		{
			items[lastitem]->menu->onButtonDown(wb, x, y);
			return ;
		}
	}
	getWindowRect(&r);
	if (!Std::pointInRect(r, pos))
	{
		rcode = (wb == 1) ? -2 : -3;
	}
	else
	{
		int item = getWhichItem(pos);
		if (item >= 0) items[item]->butt->setPushed(1);
	}
}

void PopupMenu::onButtonUp(int wb, int x, int y)
{
	if (lastitem >= 0)
	{
		POINT pos = {x, y};
		if (items[lastitem]->menu && items[lastitem]->menu->isMine(pos.x, pos.y))
		{
			items[lastitem]->menu->onButtonUp(wb, x, y);
			return ;
		}
	}
	if (bdown & wb)
	{
		bdown &= ~wb;
		POINT pnt = {x, y};
		int p = getWhichItem(pnt);
		if (p >= 0)
		{
			items[p]->butt->onLeftPush(x, y);
			if (!bdown)
			{
				items[p]->butt->setPushed(0);
			}
		}
	}
}

int PopupMenu::onKillFocus()
{
#ifndef LINUX
	if (rcode == -1) rcode = -2;
#endif
	return POPUPMENU_PARENT::onKillFocus();
}

// only translates the text, not the optional accelerator
String PopupMenu::translateButtonText(const wchar_t *text)
{
	PathParser pp(text, "\t");
	String ret;
	for (int i = 0; i < pp.getNumStrings(); i++)
	{
		if (i == 0) ret += _(pp.enumString(i));	// translate first
		else ret += pp.enumString(i);
		if (i != pp.getNumStrings() - 1) ret += "\t";
	}
	return ret;
}

int PopupMenu::onPaint(Canvas *canvas)
{
	PaintBltCanvas paintcanvas;
	if (canvas == NULL)
	{
		if (!paintcanvas.beginPaint(this)) return 0;
		canvas = &paintcanvas;
	}
	POPUPMENU_PARENT::onPaint(canvas);

	RECT r, r2;

	getClientRect(&r);
	tex.getBitmap()->blitTile(canvas, &r);

	// left side
	ful.getBitmap()->blitAlpha(canvas, 0, 0);

	r2.left = 0;
	r2.right = fl.getWidth();
	r2.top = ful.getHeight();
	r2.bottom = r.bottom - fll.getHeight();
	fl.getBitmap()->stretchToRectAlpha(canvas, &r2);

	fll.getBitmap()->blitAlpha(canvas, 0, r.bottom - fll.getHeight());

	// right side
	fur.getBitmap()->blitAlpha(canvas, r.right - fur.getWidth(), 0);

	r2.left = r.right - fr.getWidth();
	r2.right = r.right;
	r2.top = fur.getHeight();
	r2.bottom = r.bottom - flr.getHeight();
	fr.getBitmap()->stretchToRectAlpha(canvas, &r2);

	flr.getBitmap()->blitAlpha(canvas, r.right - flr.getWidth(), r.bottom - flr.getHeight());

	// top
	r2.left = ful.getWidth();
	r2.right = r.right - fur.getWidth();
	r2.top = 0;
	r2.bottom = ft.getHeight();
	ft.getBitmap()->stretchToRectAlpha(canvas, &r2);

	// bottom
	r2.left = fll.getWidth();
	r2.right = r.right - flr.getWidth();
	r2.top = r.bottom - fb.getHeight();
	r2.bottom = r.bottom;
	fb.getBitmap()->stretchToRectAlpha(canvas, &r2);

	// selection bar

	if (lastitem != -1)
	{
		ItemT *it = items[lastitem];
		RECT r3, c;

		it->butt->getClientRect(&r3);

		// left
		r2.left = r.left + fl.getWidth() + SELMARGIN;
		r2.top = r.top + r3.top;
		r2.right = r2.left + sl.getWidth();
		r2.bottom = r2.top + (r3.bottom - r3.top);
		sl.getBitmap()->stretchToRectAlpha(canvas, &r2);

		c = r2;
		c.left = c.right - 1;

		// right
		r2.right = r.right - fr.getWidth() - SELMARGIN;
		r2.left = r2.right - sr.getWidth();
		sr.getBitmap()->stretchToRectAlpha(canvas, &r2);

		c.right = r2.left;

		// center
		sc.getBitmap()->stretchToRectAlpha(canvas, &c);

	}

	return 1;
}

void PopupMenu::invalidateItem(int i)
{
	if (i < 0 || i >= items.getNumItems()) return ;
	RECT r, r2, r3;
	getClientRect(&r);

	ItemT *it = items[i];
	it->butt->getClientRect(&r3);

	r2.left = r.left + fl.getWidth();
	r2.top = r.top + r3.top;
	r2.right = r.right - fl.getWidth();
	r2.bottom = r2.top + (r3.bottom - r3.top);
	invalidateRect(&r2);
}

int PopupMenu::getNumCommands()
{
	return items.getNumItems();
}

#ifdef WASABI_COMPILE_SKIN
int PopupMenu::skincb_onCheckPreventSwitch(const char *skinname)
{
	switchskinto = skinname;
	rcode = -2;
	return 1;
}
#endif

int PopupMenu::onSysKeyDown(int code, int d)
{
	int a = POPUPMENU_PARENT::onSysKeyDown(code, d);
	/* if (d & (1<<29)) {
	//ALT key pressed, abort menu (mimics win32 popup behavior)
	abort();
	if(getParent()) SendMessageW(getParent()->gethWnd(),WM_SYSKEYDOWN,code,d);
	return 0;
	}*/
	if (a == 0)
		return onKeyDown(code);
	return 0;
}

int PopupMenu::onKeyDown(int code)
{
	if (POPUPMENU_PARENT::onKeyDown(code)) return 1;
	switch (code)
	{
	case STDKEY_DOWN:
		navigate(1);
		return 1;
	case STDKEY_UP:
		navigate( -1);
		return 1;
	case STDKEY_RETURN:
		navigate(0, 1);
		return 1;
	case STDKEY_RIGHT:
		navigate(0, 0);
		return 1;
	case VK_LEFT:
		if (!toplevelmenu)
			notifyParent(ChildNotify::POPUP_SUBMENUCLOSE);
		return 1;
	case VK_ESCAPE:
		abort();
		return 1;
	}
	return 0;
}

void PopupMenu::abort()
{
	if (toplevelmenu)
		rcode = -2;
	else
		notifyParent(ChildNotify::POPUP_SUBMENUCLOSE);
}

void PopupMenu::navigate(int p, int f)
{
	keyctrl = 1;
	int i = lastitem;

	ItemT *t = NULL;

	if (p == 0)
	{

		if (lastitem >= 0)
		{
			ItemT *t = items.enumItem(lastitem);
			if (f || t->menu || t->cb)
			{
				t->butt->setHilite(1);
				t->butt->setPushed(1);
				childNotify(t->butt, ChildNotify::BUTTON_LEFTPUSH, 1, 0);
			}
		}
		return ;
	}

	while (!t || t->issep)
	{
		i += p;
		i %= items.getNumItems();
		if (i == -1) i = items.getNumItems() - 1;
		if (i >= items.getNumItems()) return ;
		t = items.enumItem(i);
	}

	if (t->butt)
	{
		int i;
		i = items.searchItem(t);
		t->butt->setHilite(1);
		if (lastitem != -1)
		{
			ItemT *s = items.enumItem(lastitem);
			if (s->butt)
			{
				s->butt->setPushed(0);
				s->butt->setHilite(0);
			}
		}
		lastitem = i;
		invalidate();
	}
}




#else

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
// 3rd level popup menu, yay

#include "script/c_script/c_text.h"
#include "script/c_script/c_rootobj.h"
#include "script/c_script/c_button.h"
#include "script/c_script/c_group.h"


PopupMenu::PopupMenu(ifc_window *sourceWnd)
{
	ASSERT(sourceWnd != NULL);
	myInit();
	setParent(sourceWnd);
	sourceWnd->setAllowDeactivation(0);
	init(HINSTANCEfromHWND(getParent()->gethWnd()), sourceWnd->gethWnd(), TRUE);
	sourceWnd->setAllowDeactivation(1);
}

PopupMenu::PopupMenu()
{
	myInit();
	setParent(WASABI_API_WND->main_getRootWnd());
	WASABI_API_WND->main_getRootWnd()->setAllowDeactivation(0);
	init(hInstance, WASABI_API_WND->main_getRootWnd()->gethWnd(), TRUE);
	WASABI_API_WND->main_getRootWnd()->setAllowDeactivation(1);
}

PopupMenu::PopupMenu(PopupMenu *sourceWnd)
{
	myInit();
	setParent(sourceWnd);
	sourceWnd->setAllowDeactivation(0);
	init(GetModuleHandle(NULL), sourceWnd->gethWnd(), TRUE);
	sourceWnd->setAllowDeactivation(1);
}

void PopupMenu::myInit()
{
	setVirtual(0);
	setStartHidden(1);
	setRenderRatio(1.0);
	reverse_side = 0;
	rcode = 0;
	submenus = 0;
	c_grouplist = NULL;
	WASABI_API_WND->popupexit_register(this, this);
}

PopupMenu::~PopupMenu()
{
	WASABI_API_WND->popupexit_deregister(this);
	delete c_grouplist;
}

int PopupMenu::onInit()
{
	POPUPMENU_PARENT::onInit();
#ifdef WASABI_COMPILE_CONFIG
	// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
	const GUID uioptions_guid =
		{ 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };
	setTransparency(_intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid), "Popup menu alpha", 255));
#else
setTransparency(255);
#endif



	setContent("wasabi.popup.main.group");

	return 1;
}

int PopupMenu::popAtXY(int x, int y)
{
	rcode = -1;
	if (1 /*items.getNumItems()*/)
	{
		POINT pt = {x, y};

		//DebugString("appdeactivation_push_disallow\n");
		WASABI_API_WND->appdeactivation_push_disallow(this);

		showAtXY(x, y, &rcode);

		//MSG msg;
		/*while (rcode == -1 && GetMessage( &msg, NULL, 0, 0 )) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}*/

		SetCursor(LoadCursor(NULL, IDC_ARROW)); // NONPORTABLE

		int quit = 0;
		while (!quit)
		{
			rcode = getGuiObject()->guiobject_runModal();
			if (rcode & 0x40000000 && rcode != -2 && rcode != -1)
			{
				int submenuentry = rcode & ~0x40000000;
				ItemT *t = items.enumItem(submenuentry);
				if (t->cb != NULL)
					initMenuCallback(submenuentry);
				if (t->menu != NULL)
				{
					setAllowDeactivation(0);
					t->menu->showAtXY(0, 0, NULL, 0, 0);
					setAllowDeactivation(1);
				}
			}
			else
				quit = 1;
		}

		setVisible(0);
	}

	//DebugString("appdeactivation_pop_disallow\n");
	WASABI_API_WND->appdeactivation_pop_disallow(this);

	onPostPop(rcode);

	return rcode;
}

void PopupMenu::showAtXY(int x, int y, int *rc, int revside /* =0 */, int parentW /* =0 */)
{
	fillContent();
	int neww = getPreferences(SUGGESTED_W);
	int newh = getPreferences(SUGGESTED_H);;

	POINT p = {x, y};
	RECT vp;
	Std::getViewport(&vp, &p);

	// maintain parent's reversal state
	reverse_side = revside;
	int savx = x;
	if (reverse_side) x -= (neww + parentW);
	if (x + neww > vp.right || x < 0)
	{
		reverse_side = !reverse_side;
		x = savx;
		if (reverse_side) x -= (neww + parentW);
	}

	if (y + newh > vp.bottom) y -= newh;
	if (x < vp.left) x = vp.left;
	if (y < vp.top) y = vp.top;

	resize(x, y, neww, newh);

	WASABI_API_WND->appdeactivation_push_disallow(this);
#ifdef WIN32
	SetWindowPos(gethWnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#else
Atom NET_STATE = XInternAtom( Linux::getDisplay(), "_NET_WM_STATE", True );
Atom state[2];
state[0] = XInternAtom( Linux::getDisplay(), "_NET_WM_STATE_SKIP_TASKBAR", True );
state[1] = XInternAtom( Linux::getDisplay(), "_NET_WM_STATE_ABOVE", True );

if ( NET_STATE && state[0] && state[1] )
{
	XChangeProperty( Linux::getDisplay(), gethWnd(), NET_STATE, XA_ATOM, 32,
					 PropModeReplace, (unsigned char *)state, 2 );
}
#endif
	WASABI_API_WND->appdeactivation_pop_disallow(this);
	setVisible(1);
}

int PopupMenu::popAnchored(int type)
{
	RECT wr;
	getParent()->getWindowRect(&wr);
	switch (type)
	{
	case POPUP_ANCHOR_UL: return popAtXY(wr.left, wr.top);
	case POPUP_ANCHOR_LL: return popAtXY(wr.left, wr.bottom);
	case POPUP_ANCHOR_UR: return popAtXY(wr.right, wr.top);
	case POPUP_ANCHOR_LR: return popAtXY(wr.right, wr.bottom);
	}
	return 0;
}

int PopupMenu::popAtMouse()
{
	int x, y;
	Std::getMousePos(&x, &y);
	return popAtXY(x, y);
}

void PopupMenu::onNewContent()
{
	POPUPMENU_PARENT::onNewContent();
	if (isVisible())
		fillContent();
}

void PopupMenu::fillContent()
{
	GuiObject *grouplist = findObject("popup.content");
	if (grouplist != NULL)
	{

		delete c_grouplist;

		c_grouplist = new C_GroupList(*grouplist);
		c_grouplist->removeAll();
		c_grouplist->setRedraw(0);

		for (int i = 0;i < items.getNumItems();i++)
		{
			addItem(items.enumItem(i));
		}

		c_grouplist->setRedraw(1);
	}
}

void PopupMenu::addItem(ItemT *i)
{
	if (c_grouplist == NULL) return ;
	switch (i->type)
	{

	case POPUPITEM_TYPE_TEXT:
		{
			c_grouplist->instantiate("wasabi.popup.text.item", 1);
			ScriptObject *o = c_grouplist->enumItem(c_grouplist->getNumItems() - 1);
			if (o != NULL)
			{
				C_Group grp(o);

				C_RootObject g(o);
				g.notify("arrow", StringPrintfW(L"%d", (submenus) ? 1 : 0), 0, 0);
				g.notify("checkmark", StringPrintfW(L"%d", menuchecks ? 1 : 0), 0, 0);
				g.notify("id", StringPrintfW(L"%d", i->cmd), 0, 0);

				ScriptObject *check = grp.getObject("popup.item.checkmark");
				if (check != NULL)
				{
					C_Button toggle(check);
					toggle.setActivated(i->checked);
				}

				ScriptObject *arrow = grp.getObject("popup.item.submenuarrow");
				if (check != NULL)
				{
					C_Button sub(arrow);
					sub.setActivated((i->menu != NULL || i->cb != NULL));
				}

				ScriptObject *txt = grp.getObject("popup.item.text");
				if (txt != NULL)
				{
					C_Text itemtxt(txt);
					itemtxt.setText(i->txt);
				}
			}
			break;
		}

	case POPUPITEM_TYPE_IMAGE:
		c_grouplist->instantiate("wasabi.popup.image.item", 1);
		break;

	case POPUPITEM_TYPE_SEPARATOR:
		c_grouplist->instantiate("wasabi.popup.separator.item", 1);
		break;

	}
}

String PopupMenu::translateButtonText(const wchar_t *text)
{
	PathParser pp(text, "\t");
	String ret;
	for (int i = 0; i < pp.getNumStrings(); i++)
	{
		if (i == 0) ret += _(pp.enumString(i));	// translate first
		else ret += pp.enumString(i);
		if (i != pp.getNumStrings() - 1) ret += "\t";
	}
	return ret;
}

void PopupMenu::addCommand(const wchar_t *_txt, int command, int checked, int disabled, int addpos)
{
	if (!_txt)
	{
		addSeparator();
		return ;
	}

	String txt = translateButtonText(_txt);
#ifdef WASABI_COMPILE_LOCALES
	const char *bind = WASABI_API_LOCALE->locales_getBindFromAction(command);
	if (bind) txt += StringPrintfW(L"\t%s", bind);
#endif
	ItemT *t = new ItemT;
	t->type = POPUPITEM_TYPE_TEXT;
	t->cmd = command;
	t->txt = txt;
	t->checked = checked;
	t->menu = NULL;
	t->cb = NULL;
	t->cmd = -1;
	ASSERT(PTRLIST_POS_LAST == -1); //BU
	items.addItem(t, addpos);
}

int PopupMenu::popupexitcb_onExitPopup()
{
	getGuiObject()->guiobject_endModal( -2);
	return 1;
}

void PopupMenu::addSubMenu(PopupMenu *menu, const wchar_t *text)
{
	ASSERT(text != NULL);
	submenus = 1;
	ItemT *t = new ItemT;
	t->type = POPUPITEM_TYPE_TEXT;
	t->cmd = items.getNumItems() | 0x40000000;
	t->txt = translateButtonText(text);
	t->checked = 0;
	t->menu = menu;
	t->cb = NULL;
	t->cmd = -1;
	t->cmd = -1;
	items.addItem(t);
}

void PopupMenu::addSubMenuCallback(const wchar_t *text, PopupMenuCallback *cb, int param)
{
	ASSERT(text != NULL);
	ASSERT(cb != NULL);
	ItemT *t = new ItemT;
	submenus = 1;
	t->type = POPUPITEM_TYPE_TEXT;
	t->checked = 0;
	t->cmd = items.getNumItems() | 0x40000000;
	t->menu = NULL;
	t->cb = cb;
	t->cbparam = param;
	t->txt = translateButtonText(text);
	t->cmd = -1;
	items.addItem(t);
}

void PopupMenu::initMenuCallback(int item)
{
	int a = rcode;
	rcode = 0;
	PopupMenu *p = items[item]->cb->popupMenuCallback(this, items[item]->cbparam);
	rcode = a;
	if (p)
	{
		items[item]->cb = NULL;
		items[item]->menu = p;
	}
}

#endif


#endif //WASABI_WANT_FF_POPUP
