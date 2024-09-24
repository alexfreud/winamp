#include <precomp.h>
#include "wa2wndembed.h"
#include "wa2frontend.h"
#include "wa2buckitems.h"
#include "embedwndguid.h"
#include "main.h"
#include <api/wnd/bucketitem.h>
#include "resource.h"
#include <api/wnd/wndclass/wndholder.h>
#include <api/wndmgr/layout.h>
#include "wa2cfgitems.h"
#include "gen.h"
#include "../Agave/Language/api_language.h"

extern TList<HWND> forcedoffwnds;

#define BUCKETITEM_WNDTYPE L"buck"
#define WINAMP_OPTIONS_WINDOWSHADE_PL   40266
ReentryFilterObject wndMsgFilter;

int embedTable[] = {
                       IPC_GETWND_PE,
#ifdef MINIBROWSER_SUPPORT
                       IPC_GETWND_MB,
#endif

                       IPC_GETWND_VIDEO};

extern int switching_skin;
extern int going_fixedform;
extern int going_freeform;
extern HINSTANCE hInstance;
//-----------------------------------------------------------------------------------------------
void WaOsWndHost::onBeforeReparent(int host)
{
#if defined(_WIN64)
	embedWindowState *ws = (embedWindowState *)GetWindowLong(getHWND(), GWLP_USERDATA);
#else
	embedWindowState* ws = (embedWindowState*)GetWindowLong(getHWND(), GWL_USERDATA);
#endif
	// 0x49474541 is related to keeping windows shown on litestep desktops
	if (ws == NULL || (int)ws == 0x49474541)
	{
		HWND w = getHWND();
		if (w == wa2.getWnd(IPC_GETWND_VIDEO))
		{
			// this tells the video to not trigger its callback on windowposchanged, otherwise it will generate a new IPC_ONSHOW
			SendMessageW(w, WM_USER + 0x2, 0, 1);
		}
		return ;
	}
	ws->extra_data[EMBED_STATE_EXTRA_REPARENTING] = 1; // tell winamp to ignore show/hide events
	if (!host)
	{
		ShowWindow(getHWND(), SW_HIDE);
		if (!transfer && ((switching_skin && !Wa2WndEmbed::hadRememberedWndVisible(getHWND())) || !switching_skin))
		{
			PostMessage(getHWND(), WM_USER + 101, 0, 0);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void WaOsWndHost::onAfterReparent(int host)
{
#if defined(_WIN64)
	embedWindowState *ws = (embedWindowState *)GetWindowLong(getHWND(), GWLP_USERDATA);
#else
	embedWindowState* ws = (embedWindowState*)GetWindowLong(getHWND(), GWL_USERDATA);
#endif
	// 0x49474541 is related to keeping windows shown on litestep desktops
	if (ws == NULL || (int)ws == 0x49474541)
	{
		HWND w = getHWND();
		if (w == wa2.getWnd(IPC_GETWND_VIDEO))
		{
			// stop preventing handling of video windowposchanged
			SendMessageW(w, WM_USER + 0x2, 0, 0);
		}
		return ;
	}
	ws->extra_data[EMBED_STATE_EXTRA_REPARENTING] = 0; // tell winamp NOT to ignore show/hide events anymore
}

//-----------------------------------------------------------------------------------------------
int WaOsWndHost::onGetFocus()
{
	XuiOSWndHost::onGetFocus();
	ifc_window *z = findWindowByInterface(windowHolderGuid);
	if (z)
	{
		WindowHolder *wh = static_cast<WindowHolder*>(z->getInterface(windowHolderGuid));
		if (wh && wh->wndholder_wantAutoFocus())
		{
			HWND w = getHWND();
			if (IsWindow(w)) SetFocus(w);
		}
	}
	return 1;
}

//-----------------------------------------------------------------------------------------------
int WaOsWndHost::wantFocus()
{
	ifc_window *w = findWindowByInterface(windowHolderGuid);
	if (w)
	{
		WindowHolder *wh = static_cast<WindowHolder*>(w->getInterface(windowHolderGuid));
		if (wh)
		{
			return wh->wndholder_wantAutoFocus();
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------
int WaOsWndHost::onMouseWheelUp(int click, int lines)
{
	return 1;
}

//-----------------------------------------------------------------------------------------------
int WaOsWndHost::onMouseWheelDown(int click, int lines)
{
	return 1;
}

//-----------------------------------------------------------------------------------------------
void VideoLayoutMonitor::hook_onResize(int x, int y, int w, int h)
{
	SendMessageW(wa2.getWnd(IPC_GETWND_VIDEO), WM_TIMER, 12345, 0);
}

//-----------------------------------------------------------------------------------------------
void VideoLayoutMonitor::hook_onMove()
{
	SendMessageW(wa2.getWnd(IPC_GETWND_VIDEO), WM_TIMER, 12345, 0);
}

//-----------------------------------------------------------------------------------------------
Wa2WndEmbed::Wa2WndEmbed()
{
	WASABI_API_SYSCB->syscb_registerCallback(static_cast<WndCallbackI*>(this));
}

//-----------------------------------------------------------------------------------------------
Wa2WndEmbed::~Wa2WndEmbed()
{
	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<WndCallbackI*>(this));
	wa2wndstatus.deleteAll();
}

extern int we_have_ml;

//-----------------------------------------------------------------------------------------------
int Wa2WndEmbed::testGuid(GUID g)
{
	if (embedWndGuidMgr.testGuid(g)) return 1;

	/*  if (embedWndGuids.Data2 == g.Data2 &&  // embed wnd check :)
	      embedWndGuids.Data3 == g.Data3 && 
	      !memcmp(embedWndGuids.Data4,g.Data4,8)) return 1;*/

	return (g == pleditWndGuid || g == videoWndGuid
#ifdef MINIBROWSER_SUPPORT
	        || g == minibrowserWndGuid
#endif
	        || (we_have_ml && g == library_guid)
	       );
}

int make_sure_library_is_here_at_startup = 0;
extern int m_loading_at_startup;

//-----------------------------------------------------------------------------------------------
ifc_window *Wa2WndEmbed::createWindowByGuid(GUID g, ifc_window *parent)
{
	if (m_loading_at_startup)
		if (g == library_guid)
			make_sure_library_is_here_at_startup = 1;
	WaOsWndHost *oldhost = NULL;
	if (embedWndGuidMgr.testGuid(g) && !embedWndGuidMgr.getEmbedWindowState(g))
		return NULL;
	// first check if this window is already open in a host, and if so, remove it from the wndholder
	foreach(wndhosts)
	if (wndhosts.getfor()->g == g)
	{
		WaOsWndHost *host = wndhosts.getfor()->host;
		oldhost = host;
		host->setTransfering(1);
		host->oswndhost_unhost();
		Layout *l = static_cast<Layout *>(host->getDesktopParent());
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
					ifc_window *wnd = host->findWindowByInterface(windowHolderGuid);
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
	endfor;
	// now host the wnd in a new host
	WaOsWndHost *host = new WaOsWndHost();
	viewer_addViewItem(host);
	EmbedEntry *ee = new EmbedEntry();
	wndhosts.addItem(ee);
	ee->g = g;
	ee->host = host;
	ee->monitor = NULL;
	ee->dep = host->getDependencyPtr();
	ee->cmds = NULL;
	if (g == pleditWndGuid)
	{
		RECT r = {10, 20, 5, 38};
		host->oswndhost_setRegionOffsets(&r);
		host->oswndhost_host(wa2.getWnd(IPC_GETWND_PE));
		ee->whichwnd = IPC_GETWND_PE;
		host->setName((L"Playlist Editor")/*(WASABI_API_LNGSTRINGW(IDS_PLAYLIST_EDITOR)*/);
		GuiObject *go = parent->getGuiObject();
		PlaylistAppCmds *plEditAppCmds = new PlaylistAppCmds();
		ee->cmds = plEditAppCmds;
		go->guiobject_addAppCmds(plEditAppCmds);
		plWnd = parent; //parent->getDesktopParent();
		//ShowWindow(host->getHWND(), SW_NORMAL);
	}
	else if (g == videoWndGuid)
	{
		RECT r = {11, 20, 8, 38};
		host->oswndhost_setRegionOffsets(&r);
#ifdef VIDDEBUG
		DebugString("Video : Window service creates the host\n");
#endif
		HWND vid = wa2.getWnd(IPC_GETWND_VIDEO);
		host->oswndhost_host(vid);
		((WaOsWndHost *)host)->setNoTransparency();
		ee->whichwnd = IPC_GETWND_VIDEO;
		host->setName(WASABI_API_LNGSTRINGW(IDS_VIDEO));
		ifc_window *lw = parent->getDesktopParent();
		if (lw)
		{
			GuiObject *o = lw->getGuiObject();
			if (o)
			{
				ee->monitor = new VideoLayoutMonitor(o->guiobject_getScriptObject());
			}
		}
		SetTimer(vid, 12345, 250, NULL);
		GuiObject *go = parent->getGuiObject();
		VideoAppCmds *videoAppCmds = new VideoAppCmds();
		ee->cmds = videoAppCmds;
		go->guiobject_addAppCmds(videoAppCmds);
		vidWnd = parent; //parent->getDesktopParent();
		//ShowWindow(host->getHWND(), SW_NORMAL);
#ifdef MINIBROWSER_SUPPORT

	}
	else if (g == minibrowserWndGuid)
	{
		RECT r = {10, 20, 5, 38};
		host->oswndhost_setRegionOffsets(&r);
		host->oswndhost_host(wa2.getWnd(IPC_GETWND_MB));
		ee->whichwnd = IPC_GETWND_MB;
		host->setName("Minibrowser");
		GuiObject *go = parent->getGuiObject();
		mbWnd = parent; //parent->getDesktopParent();
		MinibrowserAppCmds *mbAppCmds = new MinibrowserAppCmds();
		ee->cmds = mbAppCmds;
		go->guiobject_addAppCmds(mbAppCmds);
		//ShowWindow(host->getHWND(), SW_NORMAL);
#endif

	}
	else if (embedWndGuidMgr.testGuid(g)) /*(embedWndGuids.Data2 == g.Data2 &&
		           embedWndGuids.Data3 == g.Data3 && 
		           !memcmp(embedWndGuids.Data4,g.Data4,8))*/
	{
		embedWindowState *ws = embedWndGuidMgr.getEmbedWindowState(g);
		ASSERT(ws != NULL);

		if (0 == (WS_BORDER & GetWindowLongPtrW(ws->me, GWL_STYLE)))
		{
			RECT r = {11, 20, 8, 14};
			host->oswndhost_setRegionOffsets(&r);
		}
		else
			host->oswndhost_setRegionOffsets(NULL);
				
		ws->extra_data[EMBED_STATE_EXTRA_HOSTCOUNT]++;
		ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = (intptr_t)parent; //parent->getDesktopParent();
		ee->whichwnd = (intptr_t)ws;
		if (ws->flags & EMBED_FLAGS_NOTRANSPARENCY) host->setNoTransparency();
		host->oswndhost_host(ws->me);
		wchar_t buf[512] = {0};
		GetWindowTextW(ws->me, buf, 512);
		host->setName(buf);
	}
	else
	{
		wndhosts.removeItem(ee);
		delete host;
		delete ee;
		return NULL;
	}
	wa2.setOnTop(cfg_options_alwaysontop.getValueAsInt());
	return host;
}

//-----------------------------------------------------------------------------------------------
int Wa2WndEmbed::testType(const wchar_t *windowtype)
{
	return !_wcsicmp(windowtype, BUCKETITEM_WNDTYPE) || !_wcsicmp(windowtype, L"plsc");
}

//-----------------------------------------------------------------------------------------------
ifc_window *Wa2WndEmbed::createWindowOfType(const wchar_t *windowtype, ifc_window *parent, int n)
{
	if (!_wcsicmp(windowtype, BUCKETITEM_WNDTYPE))
	{
		switch (n)
		{
		case 0:
			{
				PlBucketItem *bi = new PlBucketItem();
				bi->setBitmaps(L"winamp.thinger.pledit", NULL, L"winamp.thinger.pledit.hilited", L"winamp.thinger.pledit.selected");
				bucketitems.addItem(bi);
				return bi;
			}
		case 1:
			{
				MlBucketItem *bi = new MlBucketItem();
				bi->setBitmaps(L"winamp.thinger.library", NULL, L"winamp.thinger.library.hilited", L"winamp.thinger.library.selected");
				bucketitems.addItem(bi);
				return bi;
			}
		case 2:
			{
				VidBucketItem *bi = new VidBucketItem();
				bi->setBitmaps(L"winamp.thinger.video", NULL, L"winamp.thinger.video.hilited", L"winamp.thinger.video.selected");
				bucketitems.addItem(bi);
				return bi;
			}
		case 3:
			{
				VisBucketItem *bi = new VisBucketItem();
				bi->setBitmaps(L"winamp.thinger.vis", NULL, L"winamp.thinger.vis.hilited", L"winamp.thinger.vis.selected");
				bucketitems.addItem(bi);
				return bi;
			}
			// cases must be contiguous, enumerator stops at first NULL
#ifdef MINIBROWSER_SUPPORT
		case 4:
			{
				MbBucketItem *bi = new MbBucketItem();
				bi->setBitmaps(hInstance, IDB_MB_TAB_NORMAL, NULL, IDB_MB_TAB_HILITED, IDB_MB_TAB_SELECTED);
				bucketitems.addItem(bi);
				return bi;
			}
#endif 
			// n is enumertor, not whichwnd
			// we also need some way for the embeddedwnd to expose at least one bitmap (ideally 3) so we can make a nice bucketitem here (this code uses a pledit icon)
			/*      default:
			        if (n > 1024)
			        {
			          EmbedBucketItem *bi = new EmbedBucketItem();
			          bi->setBitmaps(hInstance, IDB_PLEDIT_TAB_NORMAL, NULL, IDB_PLEDIT_TAB_HILITED, IDB_PLEDIT_TAB_SELECTED);
			          bucketitems.addItem(bi);
			          return bi;
			        }
			      break;*/
		}
	}
	else if (!_wcsicmp(windowtype, L"plsc"))
	{
		switch (n)
		{
		case 0:
			pldirs.addItem(new PlDirObject);
			return pldirs.getLast();
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------------------------
int Wa2WndEmbed::destroyWindow(ifc_window *w)
{
	foreach(bucketitems)
	Wa2BucketItem *i = bucketitems.getfor();
	ifc_window *rw = i;
	if (rw == w)
	{
		delete i;
		return 1;
	}
	endfor;
	foreach(wndhosts)
	EmbedEntry *ee = wndhosts.getfor();
	WaOsWndHost *x = ee->host;
	if (WASABI_API_WND->rootwndIsValid(x))
	{
		ifc_window *rw = x;
		if (rw == w)
		{
			ReentryFilter f(&wndMsgFilter, ee->whichwnd);
			if (!f.mustLeave())
			{
				// this would hide the winamp window, which is probably not what we want to do (it should remain visible if it
				// was visible, no?

				// well, no, because we don't only run this in skin unloading, but also when a window gets destroyed (this is the wndcreation
				// service being called to free what it created) -- this won't happen for mmd3/pledit because mmd3 has a static window for
				// everything, which means that when you click close on it, it doesn't destroy it but hides it, so this code isn't called. but
				// if you load another skin (ie: NonStep), and you close the pledit, it immediately reappears with the wa2 look since oswndhost_unhost
				// reset the flags, region and parent to what they were before the window was embedded

				// i think that what we need is to save which windows were visible (and their location) before switching to freeform
				// and to restore them when we go back to wa2 mode. this will also be more consistant with the freeform behavior of
				// remembering visible status and coordinates on a per skin basis (since otherwise freeform dockings get screwed)
				// it also makes sense when we consider that we are going to need to remove all windowshade modes from the embedded
				// windows when going freeform.

				// see new functions: rememberVisibleWindows() and restoreVisibleWindows()

				// in any case, we need to hide the window here, at least temporarily in the case of skin unloading
				{
					if (ee->whichwnd > 1024)
					{
						embedWindowState *ws = NULL;
						//embedWindowState *ws = (embedWindowState *)ee->whichwnd;
						HWND hHost, hContent;
						hHost = (NULL != x) ? x->getHWND() : NULL;
						hContent = (NULL != hHost) ? (HWND)SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)hHost, IPC_FF_GETCONTENTWND) : NULL;
						
						if (NULL != hContent)
						{
							ws = (embedWindowState *)GetWindowLongPtrW(hContent, GWLP_USERDATA);
						}
						else
						{
							embedWndGuidMgr.retireEmbedWindowState((embedWindowState *)ee->whichwnd);
						}
					
						if (NULL != ws &&
							!(wa2.isValidEmbedWndState(ws) && --ws->hostcount != 0))
						{	
							if (0 != (EMBED_FLAGS_FFCALLBACK & ws->flags) &&
								NULL != ws->callback)
							{
								ws->callback(ws, FFC_DESTROYEMBED, (LPARAM)w);
							}

							x->oswndhost_unhost();
							if (wa2.isValidEmbedWndState(ws))
								ws->wasabi_window = NULL;

							if (!x->isTransfering() && wa2.isValidEmbedWndState(ws))
							{
								if (IsWindow(x->getHWND()))
								{
									SendMessageW(ws->me, WM_USER + 101, 0, 0);
								}
								embedWndGuidMgr.retireEmbedWindowState(ws);
							}
							
						}
					}
					else
					{
						if (ee->whichwnd == IPC_GETWND_VIDEO) KillTimer(wa2.getWnd(ee->whichwnd), 12345);
						x->oswndhost_unhost();
						if (!x->isTransfering())
							wa2.setWindowVisible(ee->whichwnd, 0);
#ifdef VIDDEBUG
						if (ee->whichwnd == IPC_GETWND_VIDEO) DebugString("Video : Window service asks WA2 to close the window\n");
#endif

					}
				}
			}
			wndhosts.removeItem(ee);

			embedWindowState *ws = NULL;
			HWND thiswnd = NULL;
			if (ee->whichwnd > 1024)
			{
				if (IsWindow(x->getHWND()))
					thiswnd = x->getHWND();
				//ws=(embedWindowState *)ee->whichwnd;
				//thiswnd=ws->me;
			}
			else thiswnd = wa2.getWnd(ee->whichwnd);
			//moved to xuioswndhost
			//SetWindowLong(thiswnd,GWL_STYLE,GetWindowLong(thiswnd,GWL_STYLE)&~(WS_CHILDWINDOW));
			switch (ee->whichwnd)
			{
			case IPC_GETWND_PE: plWnd = NULL; break;
#ifdef MINIBROWSER_SUPPORT
			case IPC_GETWND_MB: mbWnd = NULL; break;
#endif
			case IPC_GETWND_VIDEO:
#ifdef VIDDEBUG
				DebugString("Video : Window service destroys host\n");
#endif
				vidWnd = NULL;
				break;
			default:
				if (ee->whichwnd > 1024 && ws && thiswnd != NULL)
				{
					ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = 0;
				}
				break;
			}
			if (ee->cmds)
			{
				GuiObject *o = w->getParent()->getGuiObject();
				o->guiobject_removeAppCmds(ee->cmds);
			}
			x->oswndhost_unhost(); // ignored if already done by reentryfiltered code
			delete ee->monitor;
			delete ee->cmds;
			delete x;


			if (ee->whichwnd > 1024 && ws)
			{
				if (forcedoffwnds.haveItem(ws->me))
				{
					RECT r;
					GetWindowRect(ws->me, &r);
					SetWindowPos(ws->me, NULL, r.left + 20000, r.top + 20000, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOZORDER);
					forcedoffwnds.delItem(ws->me);
				}
			}
			delete ee;
			SetFocus(wa2.getMainWindow());

			return 1;
		}
		endfor;
		foreach (pldirs)
		PlDirObject *pldir = pldirs.getfor();
		if (pldir == w)
		{
			delete pldir;
			return 1;
		}
		endfor;
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------
int Wa2WndEmbed::viewer_onEvent(ifc_window *item, int event, intptr_t param, void *ptr, size_t ptrlen)
{
	if (event == ifc_window::Event_SETVISIBLE)
	{
		/*    if (!param) {
		      // the wnd may be going away, but then again, it might just be hidden to show an alternate layout of the same
		      // container, so before continuing, we need to check if it's actually going away. There is of course an exception
		      // in that if the window is hosted by a wndholder with autoclose="1", we should mirror the hiding state regardless
		      // of the container state
		 
		      api_window *whr = item->getParent();
		      int except = 0;
		      if (whr) {
		        GuiObject *go = whr->getGuiObject();
		        if (go) {
		          const char *par = go->guiobject_getXmlParam("autoclose");
		          if (!par || (par && ATOI(par) == 1)) except = 1;
		        }
		      }
		      if (!except) {
		        api_window *lr = item->getDesktopParent();
		        if (lr) {
		          Layout *l = static_cast<Layout *>(lr->getInterface(layoutGuid));
		          if (l) {
		            Container *c = l->getParentContainer();
		            if (c) {
		              if (c->isVisible()) return 1;
		            }
		          }
		        }
		      }
		    }*/
		foreach(wndhosts)
		EmbedEntry *ee = wndhosts.getfor();
		XuiOSWndHost *x = ee->host;
		ifc_window *rw = x;
		if (rw == item)
		{
			{
				ReentryFilter f(&wndMsgFilter, ee->whichwnd);
				if (f.mustLeave()) continue;
			}
			if (ee->whichwnd > 1024)
			{
				embedWindowState *ws = (embedWindowState *)ee->whichwnd;
				if (!param && wa2.isValidEmbedWndState(ws))
				{
					if (IsWindow(ws->me))
						SendMessageW(ws->me, WM_USER + 101, 0, 0);
					ifc_window *rwh = x->findWindowByInterface(windowHolderGuid);
					if (rwh != NULL)
					{
						WindowHolder *wh = static_cast<WindowHolder *>(rwh->getInterface(windowHolderGuid));
						if (wh != NULL)
						{
							wh->onRemoveWindow(1);
						}
					}
					if (wa2.isValidEmbedWndState(ws)) ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = NULL;
				}
				else if (wa2.isValidEmbedWndState(ws))
				{
					ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = (intptr_t)item->getParent();
					ShowWindow(ws->me, SW_NORMAL);
				}
			}
			else
			{
				ReentryFilter f(&wndMsgFilter, ee->whichwnd);
#ifdef VIDDEBUG
				if (ee->whichwnd == IPC_GETWND_VIDEO && param != wa2.isWindowVisible(ee->whichwnd)) DebugString("Video : Detected that the host is %s, syncing\n", param ? "shown" : "hidden");
#endif
				wa2.setWindowVisible(ee->whichwnd, param);
			}
		}
		endfor;
	}
	return 1;
}

int Wa2WndEmbed::onShowWindow(Container *c, GUID guid, const wchar_t *groupid)
{
	foreach(wndhosts)
	EmbedEntry *ee = wndhosts.getfor();
	if (ee->g == guid)
	{
		ReentryFilter f(&wndMsgFilter, ee->whichwnd);
		if (f.mustLeave()) return 1;
		if (guid == videoWndGuid) wa2.setWindowVisible(IPC_GETWND_VIDEO, 1);
#ifdef MINIBROWSER_SUPPORT
		else if (guid == minibrowserWndGuid) wa2.setWindowVisible(IPC_GETWND_MB, 1);
#endif
		else if (guid == pleditWndGuid) wa2.setWindowVisible(IPC_GETWND_PE, 1);
	}
	endfor;
	return 1;
}

int Wa2WndEmbed::onHideWindow(Container *c, GUID guid, const wchar_t *groupid)
{
	/*  if (guid == INVALID_GUID) return 1;
	  embedWindowState *ws = embedWndGuidMgr.getEmbedWindowState(guid);
	  if (ws != NULL && wa2.isValidEmbedWndState(ws)) {
	    if (IsWindow(ws->me))
	      SendMessageW(ws->me,WM_USER+101,0,0);
	    api_window *x = (api_window*)ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND];
	    if (x && WASABI_API_WND->rootwndIsValid(x)) {
	      api_window *rwh = x->findWindowByInterface(windowHolderGuid);
	      if (rwh != NULL) {
	        WindowHolder *wh = static_cast<WindowHolder *>(rwh->getInterface(windowHolderGuid));
	        if (wh != NULL) {
	          wh->onRemoveWindow(1);
	        }
	      }
	    }
	    ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = NULL;
	  }
	*/
	foreach(wndhosts)
	EmbedEntry *ee = wndhosts.getfor();
	if (ee->g == guid)
	{
		ReentryFilter f(&wndMsgFilter, ee->whichwnd);
		if (f.mustLeave()) return 1;
		if (ee->host->isTransfering()) return 1;
		ifc_window *dp = ee->host->getDesktopParent();
		if (dp)
		{
			Layout *l = static_cast<Layout*>(dp->getInterface(layoutGuid));
			if (l)
			{
				if (l->getParentContainer() != c) return 1;
			}
		}
		if (guid == videoWndGuid) wa2.setWindowVisible(IPC_GETWND_VIDEO, 0);
#ifdef MINIBROWSER_SUPPORT
		else if (guid == minibrowserWndGuid) wa2.setWindowVisible(IPC_GETWND_MB, 0);
#endif
		else if (guid == pleditWndGuid) wa2.setWindowVisible(IPC_GETWND_PE, 0);
	}
	endfor;

	return 1;
}

extern wchar_t *INI_FILE;

int Wa2WndEmbed::embedRememberProc(embedWindowState *p, embedEnumStruct *parms)
{
	WndStatus *ws = new WndStatus;
	ws->wndcode = -1; // if you insert a wnd that is not in embedTable, put -1 as wndcode
	ws->wnd = p->me;
	ws->visible = IsWindowVisible(p->me);
	GetWindowRect(p->me, &ws->position);
	// ws->position=p->r;
	wa2wndstatus.addItem(ws);

	// only store the ml window position if not loading on startup
	if(going_freeform && !m_loading_at_startup)
	{
		HWND mlwnd = wa2.getMediaLibrary();
		if(GetWindow(p->me, GW_CHILD) == mlwnd)
		{
			WritePrivateProfileStringW(L"gen_ff", L"classicmlwidth", StringPrintfW(L"%d", ws->position.right - ws->position.left), INI_FILE);
			WritePrivateProfileStringW(L"gen_ff", L"classicmlheight", StringPrintfW(L"%d", ws->position.bottom - ws->position.top), INI_FILE);
		}
	}
	return 0;
}

extern int m_loading_at_startup;

//-----------------------------------------------------------------------------------------------
// todo: remember and restore windowshade modes
void Wa2WndEmbed::rememberVisibleWindows()
{
	wa2wndstatus.deleteAll();
	for (int i = 0;i < sizeof(embedTable) / sizeof(embedTable[0]);i++)
	{
		HWND w = wa2.getWnd(embedTable[i]);
		WndStatus *ws = new WndStatus;
		ws->wndcode = embedTable[i]; // if you insert a wnd that is not in embedTable, put -1 as wndcode
		ws->wnd = w;
		ws->visible = wa2.isWindowVisible(embedTable[i]);
		GetWindowRect(w, &ws->position);
		if (going_freeform)
		{
			if (embedTable[i] == IPC_GETWND_PE)
			{
				int peheight = ws->position.bottom - ws->position.top;
				int pewidth = ws->position.right - ws->position.left;
				if (!m_loading_at_startup)
				{
					WritePrivateProfileStringW(L"gen_ff", L"classicplwidth", StringPrintfW(L"%d", pewidth), INI_FILE);
					WritePrivateProfileStringW(L"gen_ff", L"classicplheight", StringPrintfW(L"%d", peheight), INI_FILE);
				}
				int classicpews = wa2.isWindowShade(IPC_GETWND_PE);
				if (!m_loading_at_startup || GetPrivateProfileIntW(L"gen_ff", L"classicplws", -1, INI_FILE) == -1)
					WritePrivateProfileStringW(L"gen_ff", L"classicplws", classicpews ? L"1" : L"0", INI_FILE);
				if (classicpews)
					SendMessageW(wa2.getMainWindow(), WM_COMMAND, WINAMP_OPTIONS_WINDOWSHADE_PL, 0);
				GetWindowRect(w, &ws->position);
			}
		}
		wa2wndstatus.addItem(ws);
	}
	embedEnumStruct cs = { embedRememberProc, 0};
	SendMessageW(wa2.getMainWindow(), WM_WA_IPC, (WPARAM)&cs, IPC_EMBED_ENUM);
}

int Wa2WndEmbed::hadRememberedWndVisible(HWND w)
{
	int n = wa2wndstatus.getNumItems();
	for (int i = 0;i < n;i++)
	{
		WndStatus *ws = wa2wndstatus.enumItem(i);
		if (ws->wnd == w && ws->visible)
			return 1;
	}
	return 0;
}

void Wa2WndEmbed::restoreVisibleWindows()
{
	int n = wa2wndstatus.getNumItems();
	HWND mlwnd = wa2.getMediaLibrary();
	for (int i = 0;i < n;i++)
	{
		WndStatus *ws = wa2wndstatus.enumItem(i);
		if (going_fixedform && !m_loading_at_startup)
		{
			if (embedTable[i] == IPC_GETWND_PE)
			{
				int classicpews = GetPrivateProfileIntW(L"gen_ff", L"classicplws", 0, INI_FILE);
				if (classicpews)
				{
					SendMessageW(wa2.getMainWindow(), WM_COMMAND, WINAMP_OPTIONS_WINDOWSHADE_PL, 0);
				}
				int classicwidth = GetPrivateProfileIntW(L"gen_ff", L"classicplwidth", 275, INI_FILE);
				int classicheight = GetPrivateProfileIntW(L"gen_ff", L"classicplheight", 145, INI_FILE);
				wa2.setPlEditWidthHeight(classicwidth, classicheight);
			}
			
			if(GetWindow(ws->wnd, GW_CHILD) == mlwnd)
			{
				// only restore the ml window size if we were able to read in saved values
				int mlwidth = GetPrivateProfileIntW(L"gen_ff", L"classicmlwidth", -1, INI_FILE);
				int mlheight = GetPrivateProfileIntW(L"gen_ff", L"classicmlheight", -1, INI_FILE);
				if(mlwidth != -1 && mlheight != -1)
					SetWindowPos(ws->wnd, 0, 0, 0, mlwidth, mlheight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
			}
		}
		// FG> as of oct19, this function only restores state for windows that WERE visible
		// because there is no reason to hide one, since this function is designed to bring
		// back those windows that were here in one mode, but aren't so anymore in another
		if (ws->visible)
		{
			if (ws->wndcode != -1)
			{
				wa2.setWindowVisible(ws->wndcode, ws->visible);
			}
			else
			{
				ShowWindow(ws->wnd, ws->visible ? SW_SHOWNA : SW_HIDE);
			}
		}
	}
}

PtrList<WndStatus> Wa2WndEmbed::wa2wndstatus;

//-----------------------------------------------------------------------------------------------
PlaylistAppCmds::PlaylistAppCmds()
: addCmd(L"Add", PL_ADD, AppCmds::SIDE_LEFT, 0),
	remCmd(L"Rem", PL_REM, AppCmds::SIDE_LEFT, 0),
	selCmd(L"Sel", PL_SEL, AppCmds::SIDE_LEFT, 0),
	miscCmd(L"Misc", PL_MISC, AppCmds::SIDE_LEFT, 0),
	listCmd(L"List", PL_LIST, AppCmds::SIDE_RIGHT, 0)
{
	appcmds_addCmd(&addCmd);
	appcmds_addCmd(&remCmd);
	appcmds_addCmd(&selCmd);
	appcmds_addCmd(&miscCmd);
	appcmds_addCmd(&listCmd);
}

void PlaylistAppCmds::appcmds_onCommand(int id, const RECT *buttonRect, int which_button)
{
	switch (id)
	{
	case PL_ADD:
		wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_ADD, buttonRect->left, buttonRect->top, TPM_BOTTOMALIGN | TPM_LEFTALIGN);
		break;
	case PL_REM:
		wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_REM, buttonRect->left, buttonRect->top, TPM_BOTTOMALIGN | TPM_LEFTALIGN);
		break;
	case PL_SEL:
		wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_SEL, buttonRect->left, buttonRect->top, TPM_BOTTOMALIGN | TPM_LEFTALIGN);
		break;
	case PL_MISC:
		wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_MISC, buttonRect->left, buttonRect->top, TPM_BOTTOMALIGN | TPM_LEFTALIGN);
		break;
	case PL_LIST:
		wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_LIST, buttonRect->right, buttonRect->top, TPM_BOTTOMALIGN | TPM_RIGHTALIGN);
		break;
	}
}


#ifdef MINIBROWSER_SUPPORT 
//-----------------------------------------------------------------------------------------------
MinibrowserAppCmds::MinibrowserAppCmds()
{
	appcmds_addCmd(new CmdRec("Back", MB_BACK, AppCmds::SIDE_LEFT, 1));
	appcmds_addCmd(new CmdRec("Forward", MB_FORWARD, AppCmds::SIDE_LEFT, 1));
	appcmds_addCmd(new CmdRec("Stop", MB_STOP, AppCmds::SIDE_LEFT, 1));
	appcmds_addCmd(new CmdRec("Reload", MB_RELOAD, AppCmds::SIDE_LEFT, 1));
	appcmds_addCmd(new CmdRec("Misc", MB_MISC, AppCmds::SIDE_RIGHT, 1));
}

void MinibrowserAppCmds::appcmds_onCommand(int id, const RECT *buttonRect, int which_button)
{
	switch (id)
	{
	case MB_BACK:
		wa2.sendMbCmd(Winamp2FrontEnd::WA2_MBCMD_BACK);
		break;
	case MB_FORWARD:
		wa2.sendMbCmd(Winamp2FrontEnd::WA2_MBCMD_FORWARD);
		break;
	case MB_STOP:
		wa2.sendMbCmd(Winamp2FrontEnd::WA2_MBCMD_STOP);
		break;
	case MB_RELOAD:
		wa2.sendMbCmd(Winamp2FrontEnd::WA2_MBCMD_RELOAD);
		break;
	case MB_MISC:
		wa2.sendMbCmd(Winamp2FrontEnd::WA2_MBPOPUP_MISC, buttonRect->right, buttonRect->top, TPM_BOTTOMALIGN | TPM_RIGHTALIGN);
		break;
	}
}
#endif

//-----------------------------------------------------------------------------------------------
VideoAppCmds::VideoAppCmds()
{
	appcmds_addCmd(new CmdRec(L"Fullscreen", VID_FULLSCREEN, AppCmds::SIDE_LEFT, 1));
	appcmds_addCmd(new CmdRec(L"1x", VID_1X, AppCmds::SIDE_LEFT, 1));
	appcmds_addCmd(new CmdRec(L"2x", VID_2X, AppCmds::SIDE_LEFT, 1));
	appcmds_addCmd(new CmdRec(L"TV", VID_LIB, AppCmds::SIDE_LEFT, 1));
	appcmds_addCmd(new CmdRec(L"Misc", VID_MISC, AppCmds::SIDE_RIGHT, 1));
}

void VideoAppCmds::appcmds_onCommand(int id, const RECT *buttonRect, int which_button)
{
	switch (id)
	{
	case VID_FULLSCREEN:
		wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDCMD_FULLSCREEN);
		break;
	case VID_1X:
		wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDCMD_1X);
		break;
	case VID_2X:
		wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDCMD_2X);
		break;
	case VID_LIB:
		wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDCMD_LIB);
		break;
	case VID_MISC:
		wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDPOPUP_MISC, buttonRect->right, buttonRect->top, TPM_BOTTOMALIGN | TPM_RIGHTALIGN);
		break;
	}
}

