#include <precomp.h>
#include "skinembed.h"
#include <api/wndmgr/layout.h>
#include <api/wnd/wndclass/wndholder.h>
#include <api/skin/skinparse.h>
#include <api/wnd/wndtrack.h>
#include <api/config/items/cfgitemi.h>
#include <api/config/items/attrint.h>
#include <bfc/named.h>
#include <api/wndmgr/autopopup.h>
#include <api/syscb/callbacks/wndcb.h>
#include <api/wndmgr/skinwnd.h>
#include <api/script/scriptmgr.h>
#include <bfc/critsec.h>

CriticalSection skinembed_cs;

SkinEmbedder *skinEmbedder = NULL;

SkinEmbedder::SkinEmbedder()
{
	ASSERTPR(skinEmbedder == NULL, "only one skin embedder please!");
	skinEmbedder = this;
}

SkinEmbedder::~SkinEmbedder()
{
	inserted.deleteAll();
	allofthem.deleteAll();
}

int SkinEmbedder::toggle(GUID g, const wchar_t *prefered_container, int container_flag, RECT *r, int transcient)
{
	ifc_window *w = enumItem(g, 0);
	if (w != NULL)
	{
		destroy(w, r);
		return 0;
	}
	ifc_window *wnd = create(g, NULL, prefered_container, container_flag, r, transcient);
	if (wnd == NULL)
	{
#ifdef ON_CREATE_EXTERNAL_WINDOW_GUID
		int y;
		ON_CREATE_EXTERNAL_WINDOW_GUID(g, y);
#endif

	}
	return 1;
}

int SkinEmbedder::toggle(const wchar_t *groupid, const wchar_t *prefered_container, int container_flag, RECT *r, int transcient)
{
	ifc_window *w = enumItem(groupid, 0);
	if (w != NULL)
	{
		destroy(w, r);
		return 0;
	}
	create(INVALID_GUID, groupid, prefered_container, container_flag, r, transcient);
	return 1;
}

ifc_window *SkinEmbedder::create(GUID g, const wchar_t *prefered_container, int container_flag, RECT *r, int transcient, int starthidden, int *isnew)
{
	return create(g, NULL, prefered_container, container_flag, r, transcient, starthidden, isnew);
}

ifc_window *SkinEmbedder::create(const wchar_t *groupid, const wchar_t *prefered_container, int container_flag, RECT *r, int transcient, int starthidden, int *isnew)
{
	return create(INVALID_GUID, groupid, prefered_container, container_flag, r, transcient, starthidden, isnew);
}

WindowHolder *SkinEmbedder::getSuitableWindowHolder(GUID g, const wchar_t *group_id, Container *cont, Layout *lay, int _visible, int _dynamic, int _empty, int _hasself, int _autoavail)
{
	foreach(wndholders)
	WindowHolder *h = wndholders.getfor();

	ifc_window *w = h->getRootWndPtr()->getDesktopParent();
	int dyn = 1;
	int visible = 0;
	int hasself = 0;
	int empty = 0;
	int autoavail = h->wndholder_isAutoAvailable();

	Layout *l = (NULL != w) ? static_cast<Layout *>(w->getInterface(layoutGuid)) : NULL;
	Container *c = NULL;

	if (l)
	{
		c = l->getParentContainer();
		if (c)
		{
			dyn = c->isDynamic();
		}
	}

	if (NULL == w || !w->isInited()) visible = -1;
	else
	{
		if (_visible == 1)
		{
			if (h->getRootWndPtr()->isVisible()) visible = 1;
		}
		else if (_visible == 0)
		{
			if (w->isVisible()) visible = 1;
		}
	}
	if (!h->getCurRootWnd()) empty = 1;

	if (g != INVALID_GUID)
	{
		if (g == h->getCurGuid())
			hasself = 1;
	}
	else if (group_id && h->getCurGroupId())
	{
		if (!WCSICMP(group_id, h->getCurGroupId()))
			hasself = 1;
	}

	if (_visible != -1)
	{
		if (visible != _visible) continue;
	}
	if (_dynamic != -1)
	{
		if (_dynamic != dyn) continue;
	}
	if (_empty != -1)
	{
		if (empty != _empty) continue;
	}
	if (_hasself != -1)
	{
		if (hasself != _hasself) continue;
	}
	if (_autoavail != -1)
	{
		if (autoavail != _autoavail) continue;
	}

	if (cont != NULL)
	{
		if (c != cont) continue;
	}

	if (lay != NULL)
	{
		if (l != lay) continue;
	}

	if (g != INVALID_GUID)
	{
		if (h->wantGuid(g)) return h;
	}
	else if (group_id)
	{
		if (h->wantGroup(group_id)) return h;
	}
	endfor;

	return NULL;
}

ifc_window *SkinEmbedder::create(GUID g, const wchar_t *groupid, const wchar_t *prefered_container, int container_flag, RECT *r, int transcient, int starthidden, int *isnew)
{
	//  InCriticalSection in_cs(&skinembed_cs);

	foreach(in_deferred_callback)
	SkinEmbedEntry *e = in_deferred_callback.getfor();
	if ((e->guid != INVALID_GUID && e->guid == g) || WCSCASEEQLSAFE(groupid, e->groupid))
	{
#ifdef _DEBUG
		DebugStringW(L"trying to show a window that is being destroyed, hm, try again later\n");
#endif
		// too soon! try later dude
		return NULL;
	}
	endfor;

	RECT destrect = {0};

	if (isnew) *isnew = 0;

	WindowHolder *wh = NULL;
	// todo: get from c++ callback

	if (SOM::checkAbortShowHideWindow(g, 1))
		return NULL;

	if (g != INVALID_GUID)
		wh = SOM::getSuitableWindowHolderFromScript(g);

	if (!wh) wh = getSuitableWindowHolder(g, groupid, NULL, NULL, 1 /*visible*/, 0 /*static*/, 1 /*empty*/, -1 /*donttest*/, -1 /*donttest*/);
	if (!wh) wh = getSuitableWindowHolder(g, groupid, NULL, NULL, 0 /*hidden*/, 0 /*static*/, 0 /*notempty*/, 1 /*hasself*/, -1 /*donttest*/);
	if (!wh) wh = getSuitableWindowHolder(g, groupid, NULL, NULL, 0 /*hidden*/, 0 /*static*/, 1 /*empty*/, -1 /*donttest*/, 1 /*autoavail*/);
	if (!wh) wh = getSuitableWindowHolder(g, groupid, NULL, NULL, 1 /*visible*/, 1 /*dynamic*/, 1 /*empty*/, -1 /*donttest*/, -1 /*donttest*/);
	ifc_window *whrw = wh ? wh->getRootWndPtr() : NULL;
	Container *cont = NULL;
	Layout *lay = NULL;
	ifc_window *wnd = NULL;
	int newcont = 0;

	if (!wh)
	{ // no hidden static container, so lets create a dynamic one
		if (isnew) *isnew = 1;
		if (container_flag == 0 && (prefered_container == NULL || !*prefered_container))
		{
			prefered_container = AutoPopup::getDefaultContainerParams(groupid, g, &container_flag);
		}
		if (container_flag == 0 && (prefered_container == NULL || !*prefered_container))
		{
			prefered_container = WASABI_DEFAULT_STDCONTAINER;
		}
		cont = SkinParser::loadContainerForWindowHolder(groupid, g, 0, transcient, prefered_container, container_flag);
		if (cont)
		{
			cont->setTranscient(transcient);
			newcont = 1;
			lay = cont->enumLayout(0);
			/*      if (prefered_layout) { // find its target layout
			        cont->setDefaultLayout(prefered_layout);
			        lay = cont->getLayout(prefered_layout);
			        if (!lay) {
			          if (!layout_flag) // see if we can fallback
			            lay = cont->enumLayout(0);
			          else {
			            destroyContainer(cont); // deferred
			            api->console_outputString(9, StringPrintf("could not find the requested layout (%s) to host the window", prefered_container));
			            return NULL;
			          }
			        }*/
		}
		else
		{
			DebugStringW(L"no container found to hold the window... not even a default one :/\n");
		}
		wh = getSuitableWindowHolder(g, groupid, cont, lay, -1 /*donttest*/, -1 /*donttest*/, 0 /*notempty*/, 1 /*hasself*/, -1 /*donttest*/);
		if (!wh) wh = getSuitableWindowHolder(g, groupid, cont, lay, -1 /*donttest*/, -1 /*donttest*/, 1 /*empty*/, -1 /*donttest*/, -1 /*donttest*/);
		whrw = wh ? wh->getRootWndPtr() : NULL;
	}

	if (wh)
	{
		GuiObject *o = static_cast<GuiObject *>(whrw->getInterface(guiObjectGuid));
		ASSERT(o != NULL);
		if (o)
		{
			lay = o->guiobject_getParentLayout();
			if (lay)
				cont = lay->getParentContainer();
		}
		if ((g != INVALID_GUID && (g == wh->getCurGuid())) || (groupid && (wh->getCurGroupId() == groupid)))
			wnd = wh->getCurRootWnd();
		else
		{
			if (wh->getCurRootWnd())
				wh->onRemoveWindow();
			wnd = wh->onInsertWindow(g, groupid);
		}
	}

	if (!wnd)
	{
		if (cont)
		{
			if (!WCSCASEEQLSAFE(cont->getId(), L"main"))
				cont->close();
		}
		return NULL;
	}

	//int anim = 1;
	//if (wh && !whrw->getAnimatedRects()) anim = 0; // FIXME!!

	if (lay && r)
	{
		lay->getWindowRect(&destrect);
	}

	if (cont /*&& cont->isDynamic()*/)
	{
		const wchar_t *text = wnd->getRootWndName();
		cont->setTranscient(transcient);
		if (text != NULL)
			if (cont->isDynamic()) cont->setName(text);
		if (cont->isDynamic() && !transcient) cont->resetLayouts();
		if (newcont) cont->onInit(1);
	}

#ifdef WASABI_COMPILE_CONFIG
	// {280876CF-48C0-40bc-8E86-73CE6BB462E5}
	const GUID options_guid =
	    { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
	CfgItem *cfgitem = WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
	int findopenrect = _int_getValue(cfgitem, L"Find open rect");
#else
	int findopenrect = WASABI_WNDMGR_FINDOPENRECT;
#endif

	if (wh && findopenrect)
	{
		if (lay)
		{
			RECT rl;
			lay->getWindowRect(&rl);
			RECT nr = windowTracker->findOpenRect(rl, lay);
			destrect = nr;	// rewrite destrect
			int w = rl.right - rl.left;
			int h = rl.bottom - rl.top;
			lay->divRatio(&w, &h);
			nr.right = nr.left + w;
			nr.bottom = nr.top + h;
			lay->resizeToRect(&nr);
		}
	}

	if (wh)
	{
		if (r)
			/*if (anim)*/ WASABI_API_WNDMGR->drawAnimatedRects(r, &destrect);
			/*else
			{
				RECT r;
				r.left = destrect.left + (destrect.right - destrect.left) / 2 - 1;
				r.top = destrect.top + (destrect.bottom - destrect.top) / 2 + 1;
				r.right = r.left + 2;
				r.bottom = r.top + 2;
				if (anim) WASABI_API_WNDMGR->drawAnimatedRects(&r, &destrect);
			}*/
	}

	if (!starthidden)
	{
		if (cont && cont->isVisible())
		{ // we were already shown, duh
			//      cont->setVisible(0);
		}
		else
		{
			if (cont) cont->setVisible(1);
			else if (lay) lay->setVisible(1);
		}
	}

	if (wnd && newcont && !transcient)
	{
		inserted.addItem(new SkinEmbedEntry(wnd->getDependencyPtr(), wnd, g, groupid, prefered_container, container_flag, cont, wh));
		viewer_addViewItem(wnd->getDependencyPtr());
	}

	if (!transcient && wnd)
		allofthem.addItem(new SkinEmbedEntry(wnd->getDependencyPtr(), wnd, g, groupid, prefered_container, container_flag, cont, wh));

	return wnd;
}

void SkinEmbedder::destroy(ifc_window *w, RECT *r)
{
	int donecheck = 0;
	SkinEmbedEntry *e = NULL;
	for (int i = 0; i < allofthem.getNumItems();i++)
	{
		e = allofthem.enumItem(i);
		if (e->wnd == w)
		{
			donecheck = 1;
			if (SOM::checkAbortShowHideWindow(e->guid, 0))
				return ;
			break;
		}
	}
	if (WASABI_API_WND->rootwndIsValid(w))
	{
		if (!donecheck)
		{
			ifc_window *ww = w->findWindowByInterface(windowHolderGuid);
			if (ww)
			{
				WindowHolder *wh = static_cast<WindowHolder*>(ww->getInterface(windowHolderGuid));
				if (wh)
				{
					GUID g = wh->getCurGuid();
					if (g != INVALID_GUID)
					{
						donecheck = 1;
						if (SOM::checkAbortShowHideWindow(g, 0))
							return ;
					}
				}
			}
		}
	}
	// checkAbort can render w invalid !!! check again
	if (WASABI_API_WND->rootwndIsValid(w))
	{
		ifc_window *wnd = w->getDesktopParent();
		GuiObject *go = static_cast<GuiObject *>(wnd->getInterface(guiObjectGuid));
		if (go)
		{
			Layout *l = go->guiobject_getParentLayout();
			if (l)
			{
				Container *c = l->getParentContainer();
				if (c)
				{
					if (!WCSCASEEQLSAFE(c->getId(), L"main"))
					{
						c->close(); // deferred if needed
					}
					else
					{
					softclose:
						ifc_window *wnd = w->findWindowByInterface(windowHolderGuid);
						if (wnd != NULL)
						{
							WindowHolder *wh = static_cast<WindowHolder *>(wnd->getInterface(windowHolderGuid));
							if (wh != NULL)
							{
								wh->onRemoveWindow(1);
							}
						}
					}
				}
				else goto softclose;
			}
		}
	}
}

int SkinEmbedder::getNumItems(GUID g)
{
	int n = 0;
	for (int i = 0;i < wndholders.getNumItems();i++)
		if (wndholders.enumItem(i)->getRootWndPtr()->isVisible() && wndholders.enumItem(i)->getCurGuid() == g) n++;
	return n;
}

int SkinEmbedder::getNumItems(const wchar_t *groupid)
{
	int n = 0;
	for (int i = 0;i < wndholders.getNumItems();i++)
	{
		WindowHolder *wh = wndholders.enumItem(i);
		if (wh->getRootWndPtr()->isVisible()
		        && WCSCASEEQLSAFE(wh->getCurGroupId(), groupid))
			n++;
	}
	return n;
}

ifc_window *SkinEmbedder::enumItem(GUID g, int n)
{
	ASSERT(n >= 0);
	for (int i = 0;i < wndholders.getNumItems();i++)
	{
		WindowHolder *wh = wndholders.enumItem(i);
		if (wh->getCurGuid() == g)
		{
			ifc_window *w = wh->getRootWndPtr();
			Container *c = NULL;
			if (w)
			{
				w = w->getDesktopParent();
				if (w)
				{
					Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
					if (l)
					{
						c = l->getParentContainer();
					}
				}
			}
			if (c && c->isVisible() || (!c && wndholders.enumItem(i)->getRootWndPtr()->isVisible()))
			{
				if (n == 0)
					return wndholders.enumItem(i)->getCurRootWnd();
				n--;
			}
		}
	}
	return NULL;
}

ifc_window *SkinEmbedder::enumItem(const wchar_t *groupid, int n)
{
	ASSERT(n >= 0);
	for (int i = 0;i < wndholders.getNumItems();i++)
	{
		WindowHolder *wh = wndholders.enumItem(i);
		const wchar_t *curgroupid = wndholders[i]->getCurGroupId();
		if (WCSCASEEQLSAFE(curgroupid, groupid))
		{
			ifc_window *w = wh->getRootWndPtr();
			Container *c = NULL;
			if (w)
			{
				w = w->getDesktopParent();
				if (w)
				{
					Layout *l = static_cast<Layout *>(w->getInterface(layoutGuid));
					if (l)
					{
						c = l->getParentContainer();
					}
				}
			}
			if (c && c->isVisible() || (!c && wndholders.enumItem(i)->getRootWndPtr()->isVisible()))
			{
				if (n == 0)
					return wndholders.enumItem(i)->getCurRootWnd();
				n--;
			}
		}
	}
	return NULL;
}

void SkinEmbedder::registerWindowHolder(WindowHolder *wh)
{
	if (wndholders.haveItem(wh)) return ;
	wndholders.addItem(wh);
}

void SkinEmbedder::unregisterWindowHolder(WindowHolder *wh)
{
	if (!wndholders.haveItem(wh)) return ;
	wndholders.removeItem(wh);
}

int SkinEmbedder::viewer_onItemDeleted(api_dependent *item)
{
	foreach(in_deferred_callback)
	if (in_deferred_callback.getfor()->dep == item)
		in_deferred_callback.removeByPos(foreach_index);
	endfor
	foreach(inserted)
	SkinEmbedEntry *e = inserted.getfor();
	if (e->dep == item)
	{
		inserted.removeItem(e);
		allofthem.removeItem(e);
		delete e;
		break;
	}
	endfor;
	foreach(allofthem)
	SkinEmbedEntry *e = allofthem.getfor();
	if (e->dep == item)
	{
		allofthem.removeItem(e);
		delete e;
		break;
	}
	endfor;
	return 1;
}

void SkinEmbedder::cancelDestroyContainer(Container *c)
{
	if (!cancel_deferred_destroy.haveItem(c)) cancel_deferred_destroy.addItem(c);
}

void SkinEmbedder::destroyContainer(Container *o)
{
	ASSERT(o);
	//fg>disabled as of 11/4/2003, side effect of cancelling fullscreen video (ie, notifier while playing video fullscreen)
	//delt with the problem in basewnd directly
	//#ifdef WIN32
	//SetFocus(NULL); // FG> this avoids Win32 calling directly WM_KILLFOCUS into a child's wndproc (couldn't they just have posted the damn msg ?), who would then call some of his parent's virtual functions while it is being deleted
	// perhaps it would be good to add a generic way to disable most child wnd messages while the parent is being deleted
	//#endif

	cancel_deferred_destroy.removeItem(o);

	foreach(inserted)
	SkinEmbedEntry *e = inserted.getfor();
	if (e->container == o)
	{
		if (!in_deferred_callback.haveItem(e))
		in_deferred_callback.addItem(e);
		
		break;
	}
	endfor
	deferred_destroy.addItem(o);
	timerclient_setTimer(CB_DESTROYCONTAINER, 20);
	//o->setVisible(0);
	//timerclient_postDeferredCallback(CB_DESTROYCONTAINER, (intptr_t)o);
}

void SkinEmbedder::timerclient_timerCallback(int id)
{
	if (id == CB_DESTROYCONTAINER)
	{
		foreach(deferred_destroy)
		Container *c = deferred_destroy.getfor();
		foreach(in_deferred_callback)
		if (in_deferred_callback.getfor()->container == c)
		{
			in_deferred_callback.removeByPos(foreach_index);
		}
		endfor;
		if (cancel_deferred_destroy.haveItem(c))
		{
			continue;
		}
		if (SkinParser::isContainer(c)) // otherwise i'ts already gone while we were waiting for the callback, duh!
			delete c;
		endfor
		cancel_deferred_destroy.removeAll();
		deferred_destroy.removeAll();
		timerclient_killTimer(CB_DESTROYCONTAINER);
		return ;
	}
	TimerClientDI::timerclient_timerCallback(id);
}

int SkinEmbedder::timerclient_onDeferredCallback(intptr_t p1, intptr_t p2)
{
	return TimerClientDI::timerclient_onDeferredCallback(p1, p2);
}

#ifdef WASABI_COMPILE_CONFIG
void SkinEmbedder::saveState()
{
	int i = 0;
	wchar_t t[1024] = {0};
	foreach(inserted)
	SkinEmbedEntry *e = inserted.getfor();
	if (e->guid != INVALID_GUID)
	{
		nsGUID::toCharW(e->guid, t);
	}
	else
	{
		WCSCPYN(t, e->groupid, 1024);
	}
	WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"wndstatus/id/%d", i), t);
	WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"wndstatus/layout/%d", i), !e->layout.isempty() ? e->layout.getValue() : L"");
	WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"wndstatus/flag/%d", i), e->required);
	i++;
	endfor;
	WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"wndstatus/id/%d", i), L"");
	WASABI_API_CONFIG->setStringPrivate(StringPrintfW(L"wndstatus/layout/%d", i), L"");
	WASABI_API_CONFIG->setIntPrivate(StringPrintfW(L"wndstatus/flag/%d", i), 0);
}

void SkinEmbedder::restoreSavedState()
{
	int i = 0;
	wchar_t t[1024] = {0};
	wchar_t l[256] = {0};
	while (1)
	{
		WASABI_API_CONFIG->getStringPrivate(StringPrintfW(L"wndstatus/id/%d", i), t, 1024, L"");
		if (!*t) break;
		WASABI_API_CONFIG->getStringPrivate(StringPrintfW(L"wndstatus/layout/%d", i), l, 256, L"");
		int flag = WASABI_API_CONFIG->getIntPrivate(StringPrintfW(L"wndstatus/flag/%d", i), 0);
		GUID g = nsGUID::fromCharW(t);
		//ifc_window *created = NULL;
		//int tried = 0;
		if (g != INVALID_GUID)
		{
			ifc_window *w = enumItem(g, 0);
			if (w == NULL)
			{
				//tried = 1;
				/*created = */create(g, l, flag);
			}
		}
		else
		{
			ifc_window *w = enumItem(t, 0);
			if (w == NULL)
			{
				//tried = 1;
				/*created = */create(t, l, flag);
			}
		}
		/*    if (tried && !created) {
		      for (int j=0;j<inserted.getNumItems();j++) {
		        SkinEmbedEntry *e = inserted.enumItem(j);
		        int yes = 0;
		        if (g != INVALID_GUID) {
		          if (e->guid == g && e->required == flag && STRCASEEQLSAFE(e->layout, l))
		            yes = 1;
		        } else {
		          if (STRCASEEQLSAFE(e->groupid, t) && e->required == flag && STRCASEEQLSAFE(e->layout, l))
		            yes = 1;
		        }
		        if (yes) {
		          inserted.removeByPos(j);
		          j--;
		          delete e;
		        }
		      }
		    }*/
		i++;
	}
}
#endif

void SkinEmbedder::attachToSkin(ifc_window *w, int side, int size)
{
	RECT r;
	if (w == NULL) return ;
	ifc_window *_w = w->getDesktopParent();
	if (_w == NULL) _w = w;
	SkinParser::getSkinRect(&r, _w);

	switch (side)
	{
	case SKINWND_ATTACH_RIGHT:
		_w->resize(r.right, r.top, size, r.bottom - r.top);
		break;
	case SKINWND_ATTACH_TOP:
		_w->resize(r.left, r.top - size, r.right - r.left, size);
		break;
	case SKINWND_ATTACH_LEFT:
		_w->resize(r.left - size, r.top, size, r.bottom - r.top);
		break;
	case SKINWND_ATTACH_BOTTOM:
		_w->resize(r.left, r.bottom, r.right - r.left, size);
		break;
	}
}

PtrList<SkinEmbedEntry> SkinEmbedder::in_deferred_callback;
PtrList<Container> SkinEmbedder::cancel_deferred_destroy;
PtrList<Container> SkinEmbedder::deferred_destroy;
