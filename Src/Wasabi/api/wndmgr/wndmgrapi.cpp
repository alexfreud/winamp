#include <precomp.h>
#include "wndmgrapi.h"
#include <api/wndmgr/autopopup.h>
#include <api/wndmgr/skinembed.h>
#include <api/wndmgr/animate.h>
#include <api/wnd/wndtrack.h>
#include <api/skin/skinparse.h>
#include <api/wndmgr/msgbox.h>
#include <api/util/varmgr.h>

wndmgr_api *wndManagerApi = NULL;

WndMgrApi::WndMgrApi()
{
	skinEmbedder = new SkinEmbedder();
	windowTracker = new WindowTracker();
}

WndMgrApi::~WndMgrApi()
{
	delete skinEmbedder;
	skinEmbedder = NULL;
	delete windowTracker;
	windowTracker = NULL;
}

void WndMgrApi::wndTrackAdd(ifc_window *wnd)
{
	if (wnd == NULL)
	{
		DebugString("wndTrackAdd, illegal param : wnd == NULL\n");
		return ;
	}
	windowTracker->addWindow(wnd);
}

void WndMgrApi::wndTrackRemove(ifc_window *wnd)
{
	if (wnd == NULL)
	{
		DebugString("wndTrackAdd, illegal param : wnd == NULL\n");
		return ;
	}
	windowTracker->removeWindow(wnd);
}

bool WndMgrApi::wndTrackDock(ifc_window *wnd, RECT *r, int mask)
{
	return windowTracker->autoDock(wnd, r, mask);
}

bool WndMgrApi::wndTrackDock2(ifc_window *wnd, RECT *r, RECT *orig_r, int mask)
{
	return windowTracker->autoDock(wnd, r, orig_r, mask);
}

void WndMgrApi::wndTrackStartCooperative(ifc_window *wnd)
{
	windowTracker->startCooperativeMove(wnd);
}

void WndMgrApi::wndTrackEndCooperative(void)
{
	windowTracker->endCooperativeMove();
}

int WndMgrApi::wndTrackWasCooperative(void)
{
	return windowTracker->wasCooperativeMove();
}

void WndMgrApi::wndTrackInvalidateAll()
{
	windowTracker->invalidateAllWindows();
}

int WndMgrApi::skinwnd_toggleByGuid(GUID g, const wchar_t *prefered_container, int container_flag, RECT *sourceanimrect, int transcient)
{
	return skinEmbedder->toggle(g, prefered_container, container_flag, sourceanimrect, transcient);
}

int WndMgrApi::skinwnd_toggleByGroupId(const wchar_t *groupid, const wchar_t *prefered_container, int container_flag, RECT *sourceanimrect, int transcient)
{
	return skinEmbedder->toggle(groupid, prefered_container, container_flag, sourceanimrect, transcient);
}

ifc_window *WndMgrApi::skinwnd_createByGuid(GUID g, const wchar_t *prefered_container, int container_flag, RECT *sourceanimrect, int transcient, int starthidden, int *isnew)
{
	return skinEmbedder->create(g, prefered_container, container_flag, sourceanimrect, transcient, starthidden, isnew);
}

ifc_window *WndMgrApi::skinwnd_createByGroupId(const wchar_t *groupid, const wchar_t *prefered_container, int container_flag, RECT *sourceanimrect, int transcient, int starthidden, int *isnew)
{
	return skinEmbedder->create(groupid, prefered_container, container_flag, sourceanimrect, transcient, starthidden, isnew);
}

void WndMgrApi::skinwnd_destroy(ifc_window *w, RECT *destanimrect)
{
	skinEmbedder->destroy(w, destanimrect);
}

int WndMgrApi::skinwnd_getNumByGuid(GUID g)
{
	return skinEmbedder->getNumItems(g);
}

ifc_window *WndMgrApi::skinwnd_enumByGuid(GUID g, int n)
{
	return skinEmbedder->enumItem(g, n);
}

int WndMgrApi::skinwnd_getNumByGroupId(const wchar_t *groupid)
{
	return skinEmbedder->getNumItems(groupid);
}

ifc_window *WndMgrApi::skinwnd_enumByGroupId(const wchar_t *groupid, int n)
{
	return skinEmbedder->enumItem(groupid, n);
}

void WndMgrApi::skinwnd_attachToSkin(ifc_window *w, int side, int size)
{
	skinEmbedder->attachToSkin(w, side, size);
}

ScriptObject *WndMgrApi::skin_getContainer(const wchar_t *container_name)
{
	Container *c = SkinParser::getContainer(container_name);
	if (c != NULL) return c->getScriptObject();
	return NULL;
}

ScriptObject *WndMgrApi::skin_getLayout(ScriptObject *container, const wchar_t *layout_name)
{
	ASSERT(container != NULL);
	Container *c = static_cast<Container *>(container->vcpu_getInterface(containerGuid));
	if (c != NULL)
	{
		Layout *l = c->getLayout(layout_name);
		if (l != NULL) return l->getScriptObject();
	}
	return NULL;
}

void WndMgrApi::wndholder_register(WindowHolder *wh)
{
	skinEmbedder->registerWindowHolder(wh);
}

void WndMgrApi::wndholder_unregister(WindowHolder *wh)
{
	skinEmbedder->unregisterWindowHolder(wh);
}

int WndMgrApi::messageBox(const wchar_t *txt, const wchar_t *title, int flags, const wchar_t *not_anymore_identifier, ifc_window *parentwnd)
{
#ifdef WASABI_WNDMGR_OSMSGBOX
 #ifdef WIN32

	int f = 0;
	if (flags & MSGBOX_OK) f |= MB_OK;
	if (flags & MSGBOX_CANCEL) f |= MB_OKCANCEL;
	if (flags & MSGBOX_YES) f |= MB_YESNO;
	if ((flags & MSGBOX_YES) && (flags & MSGBOX_CANCEL)) f |= MB_YESNOCANCEL;

	int r = MessageBoxW(parentwnd ? parentwnd->gethWnd() : NULL, txt, title, f);

	if (r == IDOK) r = MSGBOX_OK;
	if (r == IDCANCEL) r = MSGBOX_CANCEL;
	if (r == IDYES) r = MSGBOX_YES;
	if (r == IDNO) r = MSGBOX_NO;

	return r;

#else
	// port me!
#endif
#else
	MsgBox _mb(txt, title, flags, not_anymore_identifier);
	return _mb.run();
#endif
}

void WndMgrApi::drawAnimatedRects(const RECT *r1, const RECT *r2)
{
	if (r1 == NULL)
	{
		DebugString("drawAnimatedRects, illegal param : r1 == NULL\n");
		return ;
	}
	if (r2 == NULL)
	{
		DebugString("drawAnimatedRects, illegal param : r2 == NULL\n");
		return ;
	}
	AnimatedRects::draw(r1, r2);
}

int WndMgrApi::autopopup_registerGuid(GUID g, const wchar_t *desc, const wchar_t *prefered_container, int container_flag)
{
	return AutoPopup::registerGuid(SKINPARTID_NONE, g, desc, prefered_container, container_flag);
}

int WndMgrApi::autopopup_registerGroupId(const wchar_t *groupid, const wchar_t *desc, const wchar_t *prefered_container, int container_flag)
{
	return AutoPopup::registerGroupId(SKINPARTID_NONE, groupid, desc, prefered_container, container_flag);
}

void WndMgrApi::autopopup_unregister(int id)
{
	AutoPopup::unregister(id);
}

int WndMgrApi::autopopup_getNumGuids()
{
	return AutoPopup::getNumGuids();
}

GUID WndMgrApi::autopopup_enumGuid(int n)
{
	return AutoPopup::enumGuid(n);
}

int WndMgrApi::autopopup_getNumGroups()
{
	return AutoPopup::getNumGroups();
}

const wchar_t *WndMgrApi::autopopup_enumGroup(int n)
{
	return AutoPopup::enumGroup(n);
}

const wchar_t *WndMgrApi::autopopup_enumGuidDescription(int n)
{
	return AutoPopup::enumGuidDescription(n);
}

const wchar_t *WndMgrApi::autopopup_enumGroupDescription(int n)
{
	return AutoPopup::enumGroupDescription(n);
}

const wchar_t *WndMgrApi::varmgr_translate(const wchar_t *str)
{
	StringW *s = PublicVarManager::translate_nocontext(str);
	if (s)
	{
		ret.swap(s);
		delete s;
		return ret.getValueSafe();
	}
	return str;
}

Stack<ifc_window*> WndMgrApi::modal_wnd_stack;

ifc_window *WndMgrApi::getModalWnd()
{
	if (!modal_wnd_stack.peek()) return NULL;
	return modal_wnd_stack.top();
}

void WndMgrApi::pushModalWnd(ifc_window *wnd)
{
	modal_wnd_stack.push(wnd);
}

void WndMgrApi::popModalWnd(ifc_window *wnd)
{
	if (getModalWnd() != wnd) return ;
	modal_wnd_stack.pop();
}

Container *WndMgrApi::newDynamicContainer(const wchar_t *name, int transcient)
{
	return SkinParser::newDynamicContainer(name, transcient);
}
