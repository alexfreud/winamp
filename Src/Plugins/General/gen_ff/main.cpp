#include "precomp__gen_ff.h"
#include <windows.h>
#include <commctrl.h>

#include "main.h"
#include "resource.h"
#include "prefs.h"
#include "wa2frontend.h"
#include "wa2groupdefs.h"
#include "wa2wndembed.h"
#include "wa2cfgitems.h"
#include "wa2coreactions.h"
#include "wa2pledit.h"
#include "embedwndguid.h"
#include "gen_ff_ipc.h"
#include "fsmonitor.h"
#include <api/wnd/wndclass/foreignwnd.h>
#include "../winamp/wa_ipc.h"
#include "../gen_hotkeys/wa_hotkeys.h"
#include <api/script/objects/c_script/c_group.h>
#include <api/script/objects/c_script/c_text.h>

#include <api/apiinit.h>
#include <api/wnd/rootwnd.h>
#include <api/script/objects/guiobject.h>
#include <api/core/coreactions.h>
#include "menuactions.h"
#include <api/wnd/wndclass/oswndhost.h>
#include <api/script/scriptobj.h>
#include <api/service/svcs/svc_wndcreate.h>
#include <bfc/reentryfilter.h>
#include <api/skin/skinparse.h>
#include <api/wndmgr/skinembed.h>
#include <api/skin/widgets/xuiwndholder.h>
#include <api/skin/widgets/text.h>
#include <api/script/scriptmgr.h>
#include <api/wac/compon.h>
#include <api/application/version.h>
#include <bfc/parse/pathparse.h>
#include <tataki/blending/blending.h>
#include "skininfo.h"
#include <api/skin/guitree.h>

#include <bfc/wasabi_std_wnd.h>

#include "wa2core.h"
#include <api/locales/xlatstr.h>

#include <api/wndmgr/gc.h>
#include <api/syscb/callbacks/gccb.h>
#include <api/script/vcpu.h>
#include <tataki/canvas/bltcanvas.h>

#include "../nu/AutoWide.h"
#include <shlwapi.h>
#include <windowsx.h>
//wtf?
#define _WAFE_H_
#define ___HOTAMP3_H___
#include "../gen_hotkeys/hotkey.h"

int embedCreateProc(embedWindowState *p, embedEnumStruct *parms);

DECLARE_MODULE_SVCMGR;

#define VERSION L"1.55"
//#define VIDDEBUG
//#define DEBUG_CAPTURES

#include "../gen_ml/ml_ipc.h"
librarySendToMenuStruct mainSendTo;

#include "../Agave/Language/api_language.h"
// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST;
static wchar_t szDescription[256];

//-----------------------------------------------------------------------------------------------
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

#define ID_FILE_SHOWLIBRARY     40379
#define WINAMP_OPTIONS_PREFS    40012
#define WINAMP_HELP_ABOUT       40041
#define WINAMP_LIGHTNING_CLICK  40339
#define WINAMP
#define UPDATE_EGG 0xC0DE+2
#define VIEWPORT 0xC0DE+3
static int DEFERREDCALLBACKMSG;
static int UPDATEDIALOGBOXPARENTMSG;

//-----------------------------------------------------------------------------------------------
void config();
void quit();
int init();
void quit_inst();
void init_inst();
void ToggleLayout(const wchar_t *containerName);
void RestoreClassicWinamp(int was_loaded);
extern "C" winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER_U,
	"nullsoft(gen_ff.dll)",
	init,
	config,
	quit,
};

//-----------------------------------------------------------------------------------------------

int wantContainerInMenu(Container *cont)
{
	wchar_t tmpBuf[96] = {0};
	GUID g = cont->getGUID();
	if (WCSCASEEQLSAFE(cont->getId(), L"main")) return 0;
	if (cont->getNoMenu()) return 0;
	if (g == pleditWndGuid) return 0;
	if (g == videoWndGuid) return 0;
	if (g == avs_guid) return 0;
	if (g == library_guid) return 0;
	if (WCSCASEEQLSAFE(cont->getName(), L":componenttitle")) return 0;
	if (WCSCASEEQLSAFE(cont->getName(), L"Playlist Editor") || WCSCASEEQLSAFE(cont->getName(), L"Playlist") || WCSCASEEQLSAFE(cont->getId(), L"pledit")) return 0;
	if (WCSCASEEQLSAFE(cont->getName(), L"Video Window") || WCSCASEEQLSAFE(cont->getName(), L"Video") || WCSCASEEQLSAFE(cont->getId(), L"Video")) return 0;
	if (WCSCASEEQLSAFE(cont->getName(), WASABI_API_LNG->GetStringFromGUIDW(GenMlLangGUID,plugin.hDllInstance,18)) ||
	    WCSCASEEQLSAFE(cont->getName(), L"Media Library") ||
	    WCSCASEEQLSAFE(cont->getId(), L"Library") ||
	    WCSCASEEQLSAFE(cont->getName(),WASABI_API_LNGSTRINGW_BUF(IDS_MEDIA_LIBRARY,tmpBuf,96))) return 0;
	if (WCSCASEEQLSAFE(cont->getName(), L"AVS") || WCSCASEEQLSAFE(cont->getName(), WASABI_API_LNGSTRINGW(IDS_VISUALIZATIONS)) || WCSCASEEQLSAFE(cont->getName(), L"Vis") || WCSCASEEQLSAFE(cont->getId(), L"Avs") || WCSCASEEQLSAFE(cont->getId(), L"Vis")) return 0;
	return 1;
}


//-----------------------------------------------------------------------------------------------

api_playlists       *AGAVE_API_PLAYLISTS       = 0;
api_playlistmanager *AGAVE_API_PLAYLISTMANAGER = 0;
api_albumart        *AGAVE_API_ALBUMART        = 0;
api_downloadManager *WAC_API_DOWNLOADMANAGER = 0;
api_colorthemes     *WASABI_API_COLORTHEMES    = 0;
api_palette         *WASABI_API_PALETTE        = 0;
api_threadpool      *WASABI_API_THREADPOOL     = 0;

StringW m_lastskin_nam, m_lastskin_dir;

inline int lumidiff(int a, int b) {
	int r1 = (a & 0xFF0000) >> 16;
	int r2 = (b & 0xFF0000) >> 16;
	int g1 = (a & 0xFF00) >> 8;
	int g2 = (b & 0xFF00) >> 8;
	int b1 = a & 0xFF;
	int b2 = b & 0xFF;
	return MIN((ABS(r1 - r2), ABS(g1 - g2)), ABS(b1 - b2));
}

static void doplaylistcolors()
{
	int interpolate = 0;
	if (SkinParser::getSkinVersion()*10 < 10) interpolate = 1;

	// update colors
	int buf[6] = {0};
	waSetPlColorsStruct s = {0, };
	extern COLORREF getWindowBackground(COLORREF *);
	COLORREF windowbackground;
	buf[5] = getWindowBackground(&windowbackground); // auto inverted

	int need_cols = 1, need_bms = 1;

	if (!m_lastskin_dir.isempty())
	{
		StringPathCombine bitmapfilename(m_lastskin_dir, L"pledit.bmp");
		HANDLE h = CreateFileW(bitmapfilename, 0, 0, 0, OPEN_EXISTING, 0, 0);
		if (h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
			need_bms = 0;
		}

		bitmapfilename = StringPathCombine(m_lastskin_dir, L"pledit.txt");
		h = CreateFileW(bitmapfilename, 0, 0, 0, OPEN_EXISTING, 0, 0);
		if (h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
			need_cols = 0;
		}
	}

	if (need_cols)
	{
		s.numElems = 6;
		s.elems = buf;

		// list background
		if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.background"))
			buf[2] = SkinColor(L"wasabi.list.background") & 0xFFFFFF;
		else
			buf[2] = RGBTOBGR(WASABI_API_SKIN->skin_getBitmapColor(L"wasabi.list.background")) & 0xFFFFFF;  // inverted coz coming from bitmap

		// normal text
		if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text"))
			buf[0] = SkinColor(L"wasabi.list.text") & 0xFFFFFF;
		else
			buf[0] = SkinColor(L"wasabi.edit.text") & 0xFFFFFF;

		if (interpolate)
		{
			int c = buf[0];
			c = lumidiff(c, buf[2]) < 0x1F ? Blenders::BLEND_AVG(buf[0], 0xFF7F7F7F) : c;
			c = lumidiff(c, buf[2]) < 0x1F ? Blenders::BLEND_AVG(buf[0], 0xFF101010) : c;
			buf[0] = c & 0xFFFFFF;
		}

		COLORREF selected;

		// selected items background
		if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text.selected.background"))
		{
			selected = SkinColor(L"wasabi.list.text.selected.background");
			buf[3] = selected;
		}
		else
		{
			// inverted twice, see bellow
			if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.item.selected"))
				selected = RGBTOBGR(SkinColor(L"wasabi.list.item.selected"));
			else
				selected = RGBTOBGR(SkinColor(SKINCOLOR_TREE_SELITEMBKG));

			COLORREF col = RGBTOBGR(SkinColor(SKINCOLOR_LIST_COLUMNBKG));
			int a = MAX((col & 0xFF0000) >> 16, MAX((col & 0xFF00) >> 8, col & 0xFF));
			col = a < 0x1F ? Blenders::BLEND_AVG(windowbackground, 0xFF000000) : col;
			int listbkg = RGBTOBGR(buf[2]);
			selected = lumidiff(selected, listbkg) < 0x2F ? Blenders::BLEND_AVG(windowbackground, 0xFF7F7F7F) : selected;
			selected = lumidiff(selected, listbkg) < 0x2F ? Blenders::BLEND_AVG(listbkg, 0xFF7F7F7F) : selected;
			selected = lumidiff(selected, listbkg) < 0x2F ? Blenders::BLEND_AVG(windowbackground, 0xFFF0F0F0) : selected;
			selected = lumidiff(selected, listbkg) < 0x2F ? Blenders::BLEND_AVG(listbkg, 0xFFF0F0F0) : selected;
			selected = lumidiff(selected, listbkg) < 0x2F ? Blenders::BLEND_AVG(col, 0xFFF0F0F0) : selected;
			selected = lumidiff(selected, listbkg) < 0x2F ? Blenders::BLEND_AVG(col, 0xFF101010) : selected;
			selected = lumidiff(selected, RGBTOBGR(buf[0])) < 0x1F ? Blenders::BLEND_AVG(selected, 0xFF101010) : selected;
			selected = lumidiff(selected, RGBTOBGR(buf[0])) < 0x1F ? Blenders::BLEND_AVG(selected, 0xFFF0F0F0) : selected;
			selected &= 0xFFFFFF;
			buf[3] = RGBTOBGR(selected);
		}

		// active text
		if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text.current"))
			buf[1] = SkinColor(L"wasabi.list.text.current");
		else
		{
			COLORREF act = Blenders::BLEND_AVG(selected, buf[0]);
			act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(buf[2], 0xFFFFFFFF) : act;
			act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(buf[2], 0xFF101010) : act;
			act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(selected, buf[0]) : act;
			act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(selected, buf[2]) : act;
			act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(buf[0], buf[2]) : act;
			buf[1] = act & 0xFFFFFF;
		}
	}

	if (need_bms)
	{
		BltCanvas c(280, 186);

		c.fillBits(windowbackground); // already inverted

		void blitButtonToCanvas(int w, int h, int state, const wchar_t *overlayelement, int xpos, int ypos, BltCanvas *c);

		blitButtonToCanvas(8, 18, 0, L"wasabi.scrollbar.vertical.grip", 52, 53, &c);
		blitButtonToCanvas(8, 18, 1, L"wasabi.scrollbar.vertical.grip", 61, 53, &c);

		int xpos, ypos;
		//w=8,h=29
		int cols[2];
		int *fb = (int*)c.getBits() + 280 * 42;

		// inverted colors because going into bitmap

		if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.scrollbar.background.inverted"))
			cols[0] = RGBTOBGR(SkinColor(L"wasabi.scrollbar.background.inverted"));
		else
		{
			if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.column.empty"))
				cols[0] = RGBTOBGR(SkinColor(L"wasabi.list.column.empty"));
			else
			{
				COLORREF col = SkinColor(SKINCOLOR_LIST_COLUMNBKG);
				int a = MAX((col & 0xFF0000) >> 16, MAX((col & 0xFF00) >> 8, col & 0xFF));
				col = a < 0x1F ? Blenders::BLEND_AVG(windowbackground, 0xFF000000) : col;
				cols[0] = RGBTOBGR(Blenders::BLEND_AVG(col, 0xFF000000));
			}
		}

		if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.border.sunken"))
			cols[1] = RGBTOBGR(SkinColor(L"wasabi.border.sunken"));
		else
		{
			int a = MAX((windowbackground & 0xFF0000) >> 16, MAX((windowbackground & 0xFF00) >> 8, windowbackground & 0xFF));
			cols[1] = Blenders::BLEND_AVG(windowbackground, a > 0xE0 ? 0xFF000000 : 0xFFFFFFFF);
		}

		for (ypos = 42;ypos < 42 + 29; ypos ++)
		{
			for (xpos = 36; xpos < 36 + 8; xpos ++)
			{
				fb[xpos] = cols[(xpos == 36 || xpos == 36 + 8 - 1)];
			}
			fb += 280;
		}

		extern HBITMAP CreateBitmapDIB(int w, int h, int planes, int bpp, void *data);
		s.bm = CreateBitmapDIB(280, 186, 1, 32, c.getBits());
	}
	if (s.bm || s.elems) SendMessageW(wa2.getMainWindow(), WM_WA_IPC, (WPARAM)&s, IPC_SETPLEDITCOLORS);
}

static prefsDlgRecW ffPrefsItem;

//-----------------------------------------------------------------------------------------------

// a guid for our app : {4BE592C7-6937-426a-A388-ACF0EBC88E93}
static const GUID GEN_FREEFORM =
  { 0x4be592c7, 0x6937, 0x426a, { 0xa3, 0x88, 0xac, 0xf0, 0xeb, 0xc8, 0x8e, 0x93 } };

Wa2Groupdefs *groups;
static WNDPROC wa_oldWndProc;

Wa2CfgItems *cfgitems=0;

HINSTANCE hInstance = NULL;
HWND last_dlg_parent = NULL;
void populateWindowsMenus();
void unpopulateWindowsMenus();
void addWindowOptionsToContextMenu(ifc_window *w);
void removeWindowOptionsFromContextMenu();
void controlOpacity(int v);
void controlScaling(double v);
void customScaling();
void customOpacity();
void autoOpacifyHover();
void autoOpacifyFocus();
ifc_window *g_controlMenuTarget = NULL;
HMENU controlmenu = NULL;
void lockScaling(int lock);
int ffwindowsitempos = -1;
int ffoptionstop = -1;
int ffwoptionstop = -1;
int ffwindowstop = -1;
int ffwindowsitempos2 = -1;
int eqremoved = 0;
void removeEq();
void restoreEq();
int g_timedisplaymode = 0;
ifc_window *lastFocused = NULL;
int before_startup_callback = 0;
void loadExtraColorThemes();
extern _int last_page;
int gothrueqmsg = 0;
void unhookOutputIPC();
int we_have_ml = 0;
void checkMlPresent();
char eggstr[9] = {0};
int eggstat = 0;
int eggfallout = 0;
void initEgg();
void toggleEgg();
Layout *lastlayoutegg = NULL;
int disable_send_visrandom = 0;
void registerGlobalHotkeys();
int processGenericHotkey(const char *hk);
const char *getSkinInfo();
const wchar_t *getSkinInfoW();
void controlAppBar(int side);
void controlAppBarAOT();
void controlAppBarAH();
void updateAppBarMenu(ifc_window *w);
void startFSMonitor();
void stopFSMonitor();
void onGoFullscreen();
void onCancelFullscreen();
FullScreenMonitor *g_fsmonitor = NULL;
class Wa5FSCallback : public FSCallback
{
public:
	virtual void onGoFullscreen()
	{
		::onGoFullscreen();
	}
	virtual void onCancelFullscreen()
	{
		::onCancelFullscreen();
	}
};
Wa5FSCallback *g_fscallback = NULL;
int getAOTTempDisable()
{
	return g_fsmonitor->isFullScreen();
}

LRESULT CallWinampWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProcW(wa_oldWndProc, hwnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------------------------
BOOL WINAPI dll_main_raw_fn(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	Wasabi::Std::Initialize();
    return TRUE;
}

extern "C"  
{
    void *_pRawDllMain = &dll_main_raw_fn;
};

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	return TRUE;
}

//-----------------------------------------------------------------------------------------------
// core actions implement "play", "stop", etc. wndEmbedded embed wa2 windows into GUID-based wnds
//-----------------------------------------------------------------------------------------------
BEGIN_SERVICES(Winamp2_Svcs);
DECLARE_SERVICETSINGLE(svc_action, CoreActions);
DECLARE_SERVICETSINGLE(svc_action, MenuActions);
DECLARE_SERVICETSINGLE(svc_windowCreate, Wa2WndEmbed);
END_SERVICES(Winamp2_Svcs, _Winamp2_Svcs);

//-----------------------------------------------------------------------------------------------
// functions called by the overriden windowproc to create and destroy wndembedders
//-----------------------------------------------------------------------------------------------
ifc_window *plWnd = NULL;
#ifdef MINIBROWSER_SUPPORT
ifc_window *mbWnd = NULL;
#endif
ifc_window *vidWnd = NULL;
TList<HWND> forcedoffwnds;

int going_freeform = 0;
int going_fixedform = 0;

wchar_t *INI_FILE = 0, *INI_DIR = 0;

void ToggleLayout(const wchar_t *containerName)
{
	Container *c = SkinParser::getContainer(containerName);
	if (c)
	{
		int numLayouts = c->getNumLayouts();

		for (int i = 0;i < numLayouts;i++)
		{
			if (c->enumLayout(i) == c->getCurrentLayout())
			{
				int nextLayout = (i + 1) % numLayouts;
				Layout *layout = c->enumLayout(nextLayout);
				c->switchToLayout(layout->getId());
				return ;
			}
		}
	}
}

int updatePl()
{
	// find out if this GUID is already shown.
	WindowHolder *wh = skinEmbedder->getSuitableWindowHolder(pleditWndGuid, NULL, NULL, NULL, 1, -1, 0, 1, -1);
	if (wh != NULL)
	{
		wh->cancelDeferredRemove();
		GuiObject *go = wh->getRootWndPtr()->getGuiObject();
		if (go)
		{
			Layout *l = go->guiobject_getParentLayout();
			if (l)
			{
				Container *c = l->getParentContainer();
				if (c)
				{
					DebugStringW(L"Container is %s\n", c->getId());
					SkinEmbedder::cancelDestroyContainer(c);
				}
			}
		}
		plWnd = wh->getRootWndPtr(); //->getDesktopParent();
		return 1;
	}
	return 0;
}

int createPl()
{
	if (updatePl())
	{
		ShowWindow(wa2.getWnd(IPC_GETWND_PE), SW_SHOWNA);
		return 1;
	}
	ReentryFilter f(&wndMsgFilter, IPC_GETWND_PE);
	if (f.mustLeave()) return 1;
	plWnd = WASABI_API_WNDMGR->skinwnd_createByGuid(pleditWndGuid, L"resizable_status");
	if (plWnd != NULL)
	{
		plWnd->setVisible(1);
	}
	return plWnd != NULL;
}

static ifc_window *updateEmb(GUID thisguid, embedWindowState *ws)
{
	WindowHolder *wh = skinEmbedder->getSuitableWindowHolder(thisguid, NULL, NULL, NULL, 1, -1, 0, 1, -1);
	if (wh != NULL && !(ws->flags & EMBED_FLAGS_LEGACY_WND))
	{
		ifc_window *p = wh->getRootWndPtr(); //->getDesktopParent();

		//    p->getGuiScriptObject()->vcpu_setInterface(embeddedWndStateGuid, ws, INTERFACE_GENERICVOIDPTR);
		ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = (intptr_t)p;
		return p;
	}
	return 0;
}

static ifc_window *createEmb(embedWindowState *ws, bool startHidden)
{
	GUID thisguid = EmbedWndGuid(ws).getGuid();
	if (thisguid == INVALID_GUID || (ws->flags & EMBED_FLAGS_LEGACY_WND))
		return 0;

	//thisguid.Data1 = (int)ws->me;
	ifc_window *update = updateEmb(thisguid, ws);
	if (update)
		return update;

	ReentryFilter f(&wndMsgFilter, (intptr_t)ws);
	if (f.mustLeave())
		return 0;

	RECT r;
	if (!GetWindowRect(ws->me, &r))
		SetRectEmpty(&r);

	ifc_window *p = NULL;
	if (ws->flags & EMBED_FLAGS_NORESIZE)
		p = WASABI_API_WNDMGR->skinwnd_createByGuid(thisguid, L"static", 0, NULL, 0, startHidden, NULL);
	else
		p = WASABI_API_WNDMGR->skinwnd_createByGuid(thisguid, L"resizable_nostatus", 0, NULL, 0, startHidden, NULL);
	
	ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = (intptr_t) p;
	if (p != NULL)
	{
		//    p->getGuiScriptObject()->vcpu_setInterface(embeddedWndStateGuid, ws, INTERFACE_GENERICVOIDPTR);
		if (NULL != WASABI_API_APP)
			WASABI_API_APP->app_unregisterGlobalWindow(p->getDesktopParent()->gethWnd());
		
		if (!startHidden)
			p->setVisible(1);

		GuiObject *go = p->getGuiObject();
		if (go)
		{			
			Layout *l = go->guiobject_getParentLayout();
			if (l)
			{
				Container *c = l->getParentContainer();
				if (c)
				{
					if (ws->flags & EMBED_FLAGS_NOWINDOWMENU)
						c->setXmlParam(L"nomenu", L"1");
					else
						c->setXmlParam(L"nomenu", L"0");
				}
			}
		}
	}
	if (p == NULL)
	{		
		RECT r;
		GetWindowRect(ws->me, &r);
		//////SetWindowPos( ws->me, NULL, r.left - 20000, r.top - 20000, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOCOPYBITS );
		SetWindowPos( ws->me, NULL, r.left, r.top, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOCOPYBITS );
		forcedoffwnds.addItem(ws->me);
		//if (0 != (WS_VISIBLE & windowStyle))
		//RedrawWindow(GetDesktopWindow(), &r, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_NOERASE);
	}

	RedrawWindow(GetDesktopWindow(), &r, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_NOERASE);
	return p;
}

#ifdef MINIBROWSER_SUPPORT
int updateMb()
{
	WindowHolder *wh = skinEmbedder->getSuitableWindowHolder(minibrowserWndGuid, NULL, NULL, NULL, 1, -1, 0, 1, -1);
	if (wh != NULL)
	{
		mbWnd = wh->getRootWndPtr(); //->getDesktopParent();
		ShowWindow(wa2.getWnd(IPC_GETWND_MB), SW_SHOWNA);
		return 1;
	}
	return 0;
}

int createMb()
{
	// see cratePl()
	if (updateMb()) return 1;
	ReentryFilter f(&wndMsgFilter, IPC_GETWND_MB);
	if (f.mustLeave()) return 1;
	mbWnd = WASABI_API_WNDMGR->skinwnd_createByGuid(minibrowserWndGuid, "resizable_status");
	if (mbWnd != NULL) mbWnd->setVisible(1);
	return mbWnd != NULL;
}
#endif

static HWND oldVideoWnd;
static WNDPROC oldVideoWndProc;
static DWORD WINAPI newVideoWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_USER + 0x100 && wParam == 1 && lParam)
		return 0;
	
	return CallWindowProc(oldVideoWndProc, hwnd, uMsg, wParam, lParam);
}

int updateVid()
{
	WindowHolder *wh = skinEmbedder->getSuitableWindowHolder(videoWndGuid, NULL, NULL, NULL, 1, -1, 0, 1, -1);
	if (wh != NULL)
	{
#ifdef VIDDEBUG
		DebugString("Video : Host already exists and window already shown in it\n");
#endif
		wh->cancelDeferredRemove();
		GuiObject *go = wh->getRootWndPtr()->getGuiObject();
		if (go)
		{
			Layout *l = go->guiobject_getParentLayout();
			if (l)
			{
				Container *c = l->getParentContainer();
				if (c)
				{
					DebugStringW(L"Container is %s\n", c->getId());
					SkinEmbedder::cancelDestroyContainer(c);
				}
			}
		}
		vidWnd = wh->getRootWndPtr(); //->getDesktopParent();
		return 1;
	}
	return 0;
}

int createVid()
{
	ShowWindow(WASABI_API_WND->main_getRootWnd()->gethWnd(), SW_RESTORE);
	SetForegroundWindow(WASABI_API_WND->main_getRootWnd()->gethWnd());

	if (updateVid())
	{
		ShowWindow(wa2.getWnd(IPC_GETWND_VIDEO), SW_SHOWNA);
		return 1;
	}

	if (vidWnd == NULL)
	{
		ReentryFilter f(&wndMsgFilter, IPC_GETWND_VIDEO);
		if (f.mustLeave()) return 1;
#ifdef VIDDEBUG
		DebugString("Video : Trying to find a host or creating it if needed\n");
#endif
		vidWnd = WASABI_API_WNDMGR->skinwnd_createByGuid(videoWndGuid, L"resizable_status");
	}

	if (vidWnd != NULL)
	{
		vidWnd->setVisible(1);
	}
	return vidWnd != NULL;
}

int destroyPl(HWND hwndDlg, int uMsg, WPARAM wParam, LPARAM lParam)
{
	ReentryFilter f(&wndMsgFilter, IPC_GETWND_PE);
	int r = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
	if (f.mustLeave()) return r;
	if (plWnd != NULL && WASABI_API_WND->rootwndIsValid(plWnd))
	{
		WASABI_API_WNDMGR->skinwnd_destroy(plWnd);
		r = !WASABI_API_WND->rootwndIsValid(plWnd);
	}
	else
		r = !SOM::checkAbortShowHideWindow(pleditWndGuid, 0);
	if (r)
		plWnd = NULL;
	return r;
}

int destroyEmb(HWND hwndDlg, int uMsg, WPARAM wParam, LPARAM lParam, embedWindowState *ws)
{
	ReentryFilter f(&wndMsgFilter, (intptr_t)ws);
	int r = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
	if (f.mustLeave() || (ws->flags & EMBED_FLAGS_LEGACY_WND)) return r;
	if (ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] == NULL) return r;
	if (ws->extra_data[EMBED_STATE_EXTRA_REPARENTING] == 1) return r;
	ifc_window *w = (ifc_window*)ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND];
#if 1
	if (WASABI_API_WND->rootwndIsValid(w))
	{
		WASABI_API_WNDMGR->skinwnd_destroy(w);
		r = !WASABI_API_WND->rootwndIsValid(w);
	}
	else
	{
		r =	!SOM::checkAbortShowHideWindow(embedWndGuidMgr.getGuid(ws), 0);
	}
	if (r)
		ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = NULL;
#else
	if (WASABI_API_WND->rootwndIsValid(w)) WASABI_API_WNDMGR->skinwnd_destroy(w);
	else SOM::checkAbortShowHideWindow(embedWndGuidMgr.getGuid(ws), 0);
	ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND] = NULL;
#endif
	return r;
}

#ifdef MINIBROWSER_SUPPORT
int destroyMb(HWND hwndDlg, int uMsg, WPARAM wParam, LPARAM lParam)
{
	ReentryFilter f(&wndMsgFilter, IPC_GETWND_MB);
	int r = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
	if (f.mustLeave()) return r;

	if (mbWnd != NULL && WASABI_API_WND->rootwndIsValid(mbWnd)) WASABI_API_WNDMGR->skinwnd_destroy(mbWnd);
	else SOM::checkAbortShowHideWindow(minibrowserWndGuid, 0);
	mbWnd = NULL;
	return r;
}
#endif

int destroyVid(HWND hwndDlg, int uMsg, WPARAM wParam, LPARAM lParam)
{
	ReentryFilter f(&wndMsgFilter, IPC_GETWND_VIDEO);
	int r = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);

	if (f.mustLeave()) return r;

	if (vidWnd != NULL && WASABI_API_WND->rootwndIsValid(vidWnd))
	{
		WASABI_API_WNDMGR->skinwnd_destroy(vidWnd);
		r = !WASABI_API_WND->rootwndIsValid(vidWnd);
	}
	else
		r = !SOM::checkAbortShowHideWindow(videoWndGuid, 0);

	if (r)
		vidWnd = NULL;

	return r;
}


int m_loading_at_startup;
int m_are_we_loaded;

#define WINAMP_REFRESHSKIN              40291
#define WINAMP_OPTIONS_EQ               40036
#define WINAMP_OPTIONS_DSIZE            40165
#define WINAMP_OPTIONS_ELAPSED          40037
#define WINAMP_OPTIONS_REMAINING        40038

//-----------------------------------------------------------------------------------------------
class MainLayoutMonitor : public H_Layout
{
  public:
    MainLayoutMonitor(HWND _w, ScriptObject *o) : H_Layout(o) { hwnd= _w; }
    void hook_onResize(int x, int y, int w, int h);
		HWND hwnd;
};

void MainLayoutMonitor::hook_onResize(int x, int y, int w, int h)
{
	PostMessage(wa2.getMainWindow(), WM_WA_IPC, (WPARAM)hwnd, UPDATEDIALOGBOXPARENTMSG); 
}

//-----------------------------------------------------------------------------------------------
// must be called after switching to a new freeform skin
// should also be called when the main container changes its visible layout
//-----------------------------------------------------------------------------------------------
Layout *GetMainLayout()
{
	Container *container = SkinParser::getContainer(L"main");
	if (container != NULL)
	{
		return container->getCurrentLayout();
	}
	return 0;
}

HWND GetMainContainerHWND()
{
	Layout *l = GetMainLayout();
	if (l != NULL)
		return l->gethWnd();

	return 0;
}
static MainLayoutMonitor *mainLayoutMonitor = 0;
void setDialogBoxesParent()
{
	Layout *l = GetMainLayout();
	if (l != NULL)
	{
		HWND hwnd = l->gethWnd();
		delete mainLayoutMonitor;
		mainLayoutMonitor = new MainLayoutMonitor(hwnd, l->getScriptObject());
		wa2.setDialogBoxParent(hwnd);
		PostMessage(wa2.getMainWindow(), WM_WA_IPC, (WPARAM)hwnd, UPDATEDIALOGBOXPARENTMSG); 
		lastFocused = l;
	}
}

static void removeSkinExtension(StringW &skinname)
{
	int x = skinname.len() - 4;
	if (x > 0)
	{
		const wchar_t *p = ((const wchar_t *)skinname) + x;
		if (!_wcsicmp(p, L".zip") || !_wcsicmp(p, L".wal") || !_wcsicmp(p, L".wsz"))
			skinname.trunc(x);
	}
}

extern HWND subWnd, tabwnd;
extern int subWndId;
extern int toggle_from_wa2;
int switching_skin = 0;
void shutdownFFApi();


/*-----------------------------------------------------------------------------------------------
*	Winamp2 message processor
*/

static void RerouteMessage(MSG *pMsg, ifc_window *wnd)
{
	if (NULL == wnd)
		return;

	HWND hReroute = NULL;
	if (pMsg->hwnd == GetMainContainerHWND()) 
	{
		hReroute = WASABI_API_WND->main_getRootWnd()->gethWnd();
		if (NULL != hReroute)
		{
			pMsg->hwnd = hReroute;
		}
		return;
	}
	if (wnd->gethWnd() != pMsg->hwnd)
		return;

	ifc_window *parent = wnd->getRootParent();
	if (NULL == parent)
		return;
		
	Layout *l = static_cast<Layout *>(parent->getInterface(layoutGuid));
	if (NULL == l) 
		return;
			
	Container *c = l->getParentContainer();
	if (NULL == c) 
		return;
	
	GUID g = c->getDefaultContent();
	if (INVALID_GUID == g)
		return;
					
	if (g == playerWndGuid)
		hReroute = wa2.getMainWindow();
	else if (g == pleditWndGuid) 
		hReroute = wa2.getWnd(IPC_GETWND_PE);
	else if (g == videoWndGuid) 
		hReroute = wa2.getWnd(IPC_GETWND_VIDEO);
	else 
	{
		embedWindowState *ews = embedWndGuidMgr.getEmbedWindowState(g);
		if (ews)
			hReroute = ews->me;
	}

	if (NULL != hReroute)
		pMsg->hwnd = hReroute;
}

class WaMessageProccessor : public ifc_messageprocessor
{
public: 	
	WaMessageProccessor() {} 
	~WaMessageProccessor(void)  {} 

public:
	bool ProcessMessage(MSG *pMsg)
	{	
		if (WM_KEYDOWN == pMsg->message ||
			WM_KEYUP == pMsg->message ||
			WM_SYSKEYDOWN == pMsg->message ||
			WM_SYSKEYUP == pMsg->message)
		{

			ifc_window *wnd;
			HWND h = pMsg->hwnd;
			do
			{
				wnd = WASABI_API_WND->rootWndFromOSHandle(h);
				
			} while (!wnd && NULL!= (h = GetAncestor(h, GA_PARENT)));
						
			if (wnd)
			{
				if (!wnd->isVirtual())
				{
					while (wnd)
					{
						const wchar_t *pid = wnd->getId();
						if (pid && *pid) break;
						wnd = wnd->getParent();
					}
				}
				if (wnd && wnd->getCurVirtualChildFocus())
					wnd = wnd->getCurVirtualChildFocus();
			}
			
			if (wnd) 
				DebugStringW(L"target wnd 0x%08X, id: %s", wnd, wnd->getId());
						
			INT keyMsg = -1;
			switch(pMsg->message)
			{
				case WM_KEYDOWN:
					static int pos;
					if (toupper((int)pMsg->wParam) == eggstr[pos])
					{
						if (!eggstr[++pos])
						{
							toggleEgg();
							pos = 0;
						}
					}
					else pos = 0;
					keyMsg = (0 != WASABI_API_WND->forwardOnKeyDown(wnd, (int)pMsg->wParam, pMsg->lParam));
					break;					
				case WM_KEYUP:		keyMsg = (wnd && 0 != WASABI_API_WND->forwardOnKeyUp(wnd, (int)pMsg->wParam, pMsg->lParam)); break;
				case WM_SYSKEYDOWN:	keyMsg = ( 0 != WASABI_API_WND->forwardOnSysKeyDown(wnd, (int)pMsg->wParam, pMsg->lParam)); break;
				case WM_SYSKEYUP:	keyMsg = (wnd && 0 != WASABI_API_WND->forwardOnSysKeyUp(wnd, (int)pMsg->wParam, pMsg->lParam)); break;

			}
			if (-1 != keyMsg)
			{
				if (keyMsg > 0) 
					return true;

				RerouteMessage(pMsg, wnd);
			}
		}
		return false;
	}

protected:
	RECVS_DISPATCH;
};

#define CBCLASS WaMessageProccessor
START_DISPATCH;
CB(IFC_MESSAGEPROCESSOR_PROCESS_MESSAGE, ProcessMessage)
END_DISPATCH;
#undef CBCLASS

static WaMessageProccessor waMessageProcessor;

#define TabCtrl_InsertItemW(hwnd, iItem, pitem)   \
    (int)SNDMSG((hwnd), TCM_INSERTITEMW, (WPARAM)(int)(iItem), (LPARAM)(const TC_ITEMW *)(pitem))

//-----------------------------------------------------------------------------------------------
void onSkinSwitch()
{
	switching_skin = 1;
	int lastloaded = m_are_we_loaded;

	Wa2WndEmbed::rememberVisibleWindows();

	// get skin name
	wchar_t buf[MAX_PATH] = {0};
	wchar_t *p = (wchar_t *) SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)buf, IPC_GETSKINW);
	{
		m_lastskin_nam.setValue(p);
		m_lastskin_dir.setValue(buf);

		StringPathCombine t(m_lastskin_dir, L"skin.xml");

		if (!m_lastskin_nam.isempty()
		    && _waccess(t.getValue(), 0))
		{
			t.setValue(m_lastskin_dir);
			StringW n = m_lastskin_nam;
			removeSkinExtension(n);
			t.AppendPath(n);
			t.AppendPath(L"skin.xml");
			if (!_waccess(t.getValue(), 0))
				m_lastskin_dir.AppendPath(n);
		}

		if (!_waccess(t.getValue(), 0))
		{
			if (!m_are_we_loaded) init_inst();
			else
			{
				// load new skin
				StringW skinname = m_lastskin_nam;

				removeSkinExtension(skinname);

				before_startup_callback = 1;
				WASABI_API_SKIN->skin_switchSkin(skinname, m_lastskin_dir);

				if (DEFERREDCALLBACKMSG > 65536)
					PostMessage(wa2.getMainWindow(), WM_WA_IPC, 2, DEFERREDCALLBACKMSG);
				//embedEnumStruct cs = { embedCreateProc, 0 };
				//SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&cs, IPC_EMBED_ENUM);

				doplaylistcolors();
				setDialogBoxesParent();
				populateWindowsMenus();
			}
		}
		else
		{
			going_fixedform = 1;
			quit_inst();
		}
	}
	if (subWnd != NULL && IsWindow(subWnd)) SendMessageW(subWnd, WM_INITDIALOG, 0, 0);

	if (IsWindow(tabwnd))
	{
		if (!m_are_we_loaded)
		{
			if (subWndId == 5)
			{
				DestroyWindow(subWnd);
				TabCtrl_SetCurSel(tabwnd,0);
				_dosetsel(GetParent(tabwnd));
			}

			if (TabCtrl_GetItemCount(tabwnd) == 5)
			{
				TabCtrl_DeleteItem(tabwnd,4);
				TabCtrl_DeleteItem(tabwnd,3);
			}
		}
		else
		{
			if (TabCtrl_GetItemCount(tabwnd) == 3)
			{
				TCITEMW item = {0};
				item.mask=TCIF_TEXT;
				item.pszText=WASABI_API_LNGSTRINGW(IDS_COLOR_THEMES);
				TabCtrl_InsertItemW(tabwnd,3,&item);
				item.pszText=WASABI_API_LNGSTRINGW(IDS_CURRENT_SKIN);
				TabCtrl_InsertItemW(tabwnd,4,&item);
			}
		}
	}

	if (m_are_we_loaded || lastloaded) // dont call this if going from classic->classic
		Wa2WndEmbed::restoreVisibleWindows();

	if (m_are_we_loaded && WASABI_API_SYSCB)
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::GC, GarbageCollectCallback::GARBAGECOLLECT);

	if (WASABI_API_APP)
	{
		if (m_are_we_loaded) WASABI_API_APP->app_addMessageProcessor(&waMessageProcessor);
		else WASABI_API_APP->app_removeMessageProcessor(&waMessageProcessor);
	}

	/* TODO: benski> want to add this, but it's causing a weird crash
	if (going_fixedform)
		shutdownFFApi();
		*/
	switching_skin = 0;
	going_fixedform = 0;
	before_startup_callback = 0;
}

static int embedUpdateColorProc(embedWindowState *p, embedEnumStruct *parms)
{
	SendMessageW(p->me, WM_DISPLAYCHANGE, 0, 0);
	return 0;
}

int embedCreateProc(embedWindowState *ws, embedEnumStruct *parms)
{
	ifc_window *rw = reinterpret_cast<ifc_window*>(ws->extra_data[EMBED_STATE_EXTRA_FFROOTWND]);

	if (!rw && IsWindow(ws->me) && IsWindowVisible(ws->me) && !(ws->flags & EMBED_FLAGS_LEGACY_WND))
	{
		ifc_window *windowParent = createEmb(ws, true);
		if (NULL != windowParent)
		{
			GuiObject *uiObject = windowParent->getGuiObject();
			if (NULL != uiObject)
			{
				if (0 != (EMBED_FLAGS_FFCALLBACK & ws->flags) &&
					NULL != ws->callback)
				{
					ws->callback(ws, FFC_CREATEEMBED, (LPARAM)windowParent);
				}

				Layout *layout = uiObject->guiobject_getParentLayout();
				if (NULL != layout)
				{
					Container *container = layout->getParentContainer();
					if (NULL != container) container->setVisible(1);
					else layout->setVisible(1);
					windowParent->setVisible(1);
				}
			}	
		}

		if (NULL == windowParent)
			ShowWindow(ws->me, SW_HIDE);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------
void syncDoubleSize(int loading = 0)
{
	if (!cfg_uioptions_uselocks.getValueAsInt()) return ;
	int isdsize = wa2.isDoubleSize();
	int i;
	for (i = 0;i < SkinParser::getNumContainers();i++)
	{
		Container *c = SkinParser::enumContainer(i);
		if (c != NULL)
		{
			int j;
			for (j = 0;j < c->getNumLayouts();j++)
			{
				Layout *l = c->enumLayout(j);
				if (l != NULL)
				{
					if (l->isScaleLocked()) continue;
					l->setRenderRatio(isdsize ? 2.0f : 1.0f);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
class syncDisplayModeEnumerator : public FindObjectCallbackI
{
public:
	int findobjectcb_matchObject(ifc_window *object)
	{
		ScriptObject *s = NULL;
		GuiObject *g = object->getGuiObject();
		if (g)
		{
			ScriptObject *_s = g->guiobject_getScriptObject();
			if (_s)
			{
				void *v = _s->vcpu_getInterfaceObject(textGuid, &s);
				if (v && !s)
				{
					(static_cast<Text*>(v))->setTimeDisplayMode(g_timedisplaymode);
				}
			}
		}
		return 0;
	}
};

//-----------------------------------------------------------------------------------------------
void syncDisplayMode()
{
	if (switching_skin) return ;
	g_timedisplaymode = wa2.getTimeDisplayMode();
	int i;
	syncDisplayModeEnumerator enumerator;
	for (i = 0;i < SkinParser::getNumContainers();i++)
	{
		Container *c = SkinParser::enumContainer(i);
		if (c != NULL)
		{
			int j;
			for (j = 0;j < c->getNumLayouts();j++)
			{
				Layout *l = c->enumLayout(j);
				if (!WASABI_API_WND->rootwndIsValid(l)) continue;
				l->findWindowByCallback(&enumerator);
			}
		}
	}
}

static POINT viewPort;
VOID CALLBACK ViewPortChanged(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	if (viewPort.x && viewPort.y)
	{
		SystemObject::onViewPortChanged(viewPort.x, viewPort.y);
	}
}

//-----------------------------------------------------------------------------------------------
// Winamp2 main window subclass
//-----------------------------------------------------------------------------------------------
static LRESULT WINAPI wa_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int m_in_skinrefresh;

	if (uMsg == WM_COMMAND && LOWORD(wParam) == WINAMP_REFRESHSKIN)
	{
		HWND prefs = wa2.getPreferencesWindow();
		RECT prefrect = {0};
		if (prefs)
		{
			GetWindowRect(prefs, &prefrect);
			HWND hOwner = (HWND)(LONG_PTR)GetWindowLongPtrW(prefs, GWLP_HWNDPARENT);
			if (NULL != hOwner && hOwner != hwndDlg)
				SetWindowLongPtrW(prefs, GWLP_HWNDPARENT, (LONG_PTR)hwndDlg);
		}
		m_in_skinrefresh++;
		DWORD b = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
		onSkinSwitch();
		if (m_are_we_loaded) doplaylistcolors();
		m_in_skinrefresh--;
		if (prefs && !wa2.getPreferencesWindow())
		{
			SendMessageW(wa2.getMainWindow(), WM_COMMAND, WINAMP_OPTIONS_PREFS, 0);
			SetWindowPos(wa2.getPreferencesWindow(), NULL, prefrect.left, prefrect.top, prefrect.right - prefrect.left, prefrect.bottom - prefrect.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_DEFERERASE);
		}

		return b;
	}
	if (g_Core)
	{
		if (uMsg == WM_WA_IPC && lParam == IPC_CB_MISC) g_Core->gotCallback(wParam);
#define WINAMP_FILE_REPEAT              40022
#define WINAMP_FILE_SHUFFLE             40023
#define WINAMP_FILE_MANUALPLADVANCE     40395
#define UPDATE_DISPLAY_TIMER			38
		if (uMsg == WM_TIMER)
		{
			if (LOWORD(wParam) == UPDATE_EGG)
			{
				for (int i = 0;i < SkinParser::getNumContainers();i++)
				{
					Container *cont = SkinParser::enumContainer(i);
					if (cont->isMainContainer())
					{
						Layout *l = cont->getCurrentLayout();
						if (lastlayoutegg && l != lastlayoutegg)
						{
							if (WASABI_API_WND->rootwndIsValid(lastlayoutegg))
							{
								lastlayoutegg->updateTransparency();
								lastlayoutegg->setTransparencyOverride(-1);
							}
						}
						lastlayoutegg = l;
						int v = (WASABI_API_MEDIACORE->core_getLeftVuMeter(0) + WASABI_API_MEDIACORE->core_getRightVuMeter(0)) / 4;
						eggfallout -= 4;
						if (v > eggfallout) eggfallout = v;
						lastlayoutegg->setTransparencyOverride(255 - eggfallout);
						break;
					}
				}
				return 0;
			}
			if (LOWORD(wParam) == UPDATE_DISPLAY_TIMER + 4)
			{
				syncDisplayMode();
			}
			if (LOWORD(wParam) == 0xC0DE)
			{
				if (!wa2.isVisRunning())
				{
					KillTimer(wa2.getMainWindow(), 0xC0DE);
					KillTimer(wa2.getMainWindow(), 0xC0DE + 1);
					wa2.toggleVis();
				}
			}
			if (LOWORD(wParam) == 0xC0DE + 1)
			{
				KillTimer(wa2.getMainWindow(), 0xC0DE);
				KillTimer(wa2.getMainWindow(), 0xC0DE + 1);
			}
		}
		if (!my_set && (uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && LOWORD(wParam) == WINAMP_FILE_SHUFFLE)
		{
			DWORD v = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
			int shuf = wa2.getShuffle();
			if (!shuffle.getValueAsInt() != !shuf) shuffle.setValueAsInt(shuf);
			return v;
		}
		if (!my_set && (uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) &&
		    (LOWORD(wParam) == WINAMP_FILE_REPEAT) ||
		    (LOWORD(wParam) == WINAMP_FILE_MANUALPLADVANCE))
		{
			DWORD v = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
			int rep = wa2.getRepeat();
			int manadv = wa2.getManualPlaylistAdvance();
			int _v = (rep && manadv) ? -1 : rep;
			if (repeat.getValueAsInt() != _v) repeat.setValueAsInt(_v);
			return v;
		}
		if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && LOWORD(wParam) == WINAMP_OPTIONS_DSIZE)
		{
			int v = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
			syncDoubleSize();
			return v;
		}
		if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && (LOWORD(wParam) == WINAMP_OPTIONS_ELAPSED || LOWORD(wParam) == WINAMP_OPTIONS_REMAINING))
		{
			int v = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
			syncDisplayMode();
			return v;
		}
	}
	if (m_are_we_loaded)
	{
		if (uMsg == WM_COMMAND && LOWORD(wParam) == 40266)
		{
			ToggleLayout(L"pledit");
			return 0;
		}
		else if (uMsg == WM_COMMAND && LOWORD(wParam) == 40064)
		{
			ToggleLayout(L"main");
			return 0;
		}
		else if (uMsg == WM_CLOSE && wParam == 0xDEADBEEF && lParam == 0xDEADF00D)
		{
			//if (/*todo: configurable */ 1)
			{
				uMsg = WM_COMMAND;
				wParam = WINAMP_MAIN_WINDOW;
			}
		}
		else if (uMsg == WM_SETTINGCHANGE)
		{
			// used to track viewport changes so we can update
			// e.g. when removing Win8's side-by-side app mode
			if (wParam == SPI_SETWORKAREA)
			{
				KillTimer(wa2.getMainWindow(), VIEWPORT);
				viewPort.x = -1;
				viewPort.y = -1;
				SetTimer(wa2.getMainWindow(), VIEWPORT, 100, ViewPortChanged);
			}
		}
		else if (uMsg == WM_DISPLAYCHANGE)
		{
			DWORD b = CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
			if (!m_in_skinrefresh) doplaylistcolors();

			// filter out our internal skin updates
			if (wParam && lParam)
			{
				KillTimer(wa2.getMainWindow(), VIEWPORT);
				viewPort.x = LOWORD(lParam);
				viewPort.y = HIWORD(lParam);
				SetTimer(wa2.getMainWindow(), VIEWPORT, 100, ViewPortChanged);
			}

			return b;
		}
		else if (uMsg == WM_INITMENUPOPUP)
		{
			HMENU hmenuPopup = (HMENU) wParam;

			if (hmenuPopup && hmenuPopup == mainSendTo.build_hMenu && mainSendTo.mode == 1)
			{
				int IPC_LIBRARY_SENDTOMENU = SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
				if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
					mainSendTo.mode = 2;
			}

			if(hmenuPopup == wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_PLAY))
			{
				CheckMenuItem((HMENU)wParam, WINAMP_FILE_SHUFFLE, wa2.getShuffle() ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem((HMENU)wParam, WINAMP_FILE_REPEAT, wa2.getRepeat() ? MF_CHECKED : MF_UNCHECKED);
			}

			if (hmenuPopup == wa2.getPopupMenu() ||
			    hmenuPopup == wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_OPTIONS) ||
			    hmenuPopup == wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_WINDOWS))
			{
				if (hmenuPopup == wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_OPTIONS)) {
					EnableMenuItem(hmenuPopup, WINAMP_OPTIONS_DSIZE,
								   MF_BYCOMMAND | (cfg_uioptions_uselocks.getValueAsInt() ? MF_ENABLED : MF_GRAYED));
				}
				if (hmenuPopup == wa2.getPopupMenu()){
					EnableMenuItem(GetSubMenu(hmenuPopup, 11 + wa2.adjustOptionsPopupMenu(0)), WINAMP_OPTIONS_DSIZE,
								   MF_BYCOMMAND | (cfg_uioptions_uselocks.getValueAsInt() ? MF_ENABLED : MF_GRAYED));
				}

				populateWindowsMenus();

				if ((HMENU)wParam == wa2.getPopupMenu())
				{
					for (int i = 0;i < SkinParser::getNumContainers();i++)
					{
						Container *cont = SkinParser::enumContainer(i);
						if (!_wcsicmp(cont->getId(), L"main"))
						{
							CheckMenuItem((HMENU)wParam, WINAMP_MAIN_WINDOW, cont->isVisible() ? MF_CHECKED : MF_UNCHECKED);
							break;
						}
					}
				}
			}
		}
		else if (uMsg == WM_SETFOCUS)
		{
			if (m_are_we_loaded && !switching_skin)
			{
				Container *c = SkinParser::getContainer(L"main");
				if (c)
				{
					Layout *l = c->enumLayout(0);
					if (l) l->setFocus();
				}
				return 1;
			}
		}
		else if (uMsg == WM_WA_IPC)
		{
			if (lParam == DEFERREDCALLBACKMSG && DEFERREDCALLBACKMSG > 65536)
			{
				if (wParam == 1)
				{
					setDialogBoxesParent();
					return 1;
				}
				else if (wParam == 2)
				{
					// embed the windows that are currently visible because at the time the window were originally shown,
					// we hadn't been loaded yet
					// but then again, we might if it's only a switch skin and not a app load, so let's check anyway

					populateWindowsMenus();

					doplaylistcolors();

					if (plWnd == NULL && wa2.isWindowVisible(IPC_GETWND_PE))
						if (!createPl()) wa2.setWindowVisible(IPC_GETWND_PE, 0);

#ifdef MINIBROWSER_SUPPORT
					if (mbWnd == NULL && wa2.isWindowVisible(IPC_GETWND_MB))
						if (!createMb()) wa2.setWindowVisible(IPC_GETWND_MB, 0);
#endif
					if (vidWnd == NULL && wa2.isWindowVisible(IPC_GETWND_VIDEO))
						if (!createVid()) wa2.setWindowVisible(IPC_GETWND_VIDEO, 0);

					embedEnumStruct cs = { embedCreateProc, 0 };
					SendMessageW(hwndDlg, WM_WA_IPC, (WPARAM)&cs, IPC_EMBED_ENUM);

					for (int i = 0;i < SkinParser::getNumContainers();i++)
					{
						Container *cont = SkinParser::enumContainer(i);
						if (!_wcsicmp(cont->getId(), L"main"))
						{
							for (int j = 0;j < cont->getNumLayouts();j++)
							{
								Layout *l = cont->enumLayout(j);
								if (l && l->isVisible())
								{
									SetFocus(l->gethWnd());
									break;
								}
							}
						}
					}

					Crossfader::onOutputChanged();

					extern int make_sure_library_is_here_at_startup;
					if (make_sure_library_is_here_at_startup)
						SendMessageW(wa2.getMainWindow(), WM_COMMAND, ID_FILE_SHOWLIBRARY, 0);
					make_sure_library_is_here_at_startup = 0;

					before_startup_callback = 0;

					registerGlobalHotkeys();

					// force a title update since it's not always correct
					// if gen_ml hasn't loaded fully when we're loading
					if (wa2.isPlaying())
						g_Core->gotCallback(IPC_CB_MISC_TITLE, 2);
					return 1;
				}
			}
			else if (lParam == UPDATEDIALOGBOXPARENTMSG && UPDATEDIALOGBOXPARENTMSG > 65536)
			{
				wa2.updateDialogBoxParent((HWND)wParam);
			}
			else switch (lParam)
				{
				case IPC_GETSKININFO:
					return (LRESULT)getSkinInfo();
				case IPC_GETSKININFOW:
					return (LRESULT)getSkinInfoW();
				case IPC_SHOW_NOTIFICATION:
					return SystemObject::onShowNotification();
				case IPC_CB_VISRANDOM:
				{
					int v = wParam;
					extern _bool visrandom;
					if (!!v == !!visrandom.getValueAsInt()) break;
					disable_send_visrandom = 1;
					visrandom.setValueAsInt(v);
					disable_send_visrandom = 0;
					break;
				}
				case IPC_CB_OUTPUTCHANGED:
					Crossfader::onOutputChanged();
					break;

				case IPC_CB_ONTOGGLEAOT:
				{
					if ((WPARAM)cfg_options_alwaysontop.getValueAsInt() != wParam)
						cfg_options_alwaysontop.setValueAsInt(wParam);
					break;
				}

				case IPC_SETIDEALVIDEOSIZE:
				{
					wa2.setIdealVideoSize(HIWORD(wParam), LOWORD(wParam));
					break;
				}
				// this is where we detect that wa2 wants to open one of its windows (thru popup menu, button, whatever)
				// when this happens, we create a freeform wndembedder if one doesn't already exist. that embedder will
				// reparent and resize the wa2 window on its own. when we return, winamp then shows the HWND inside our frame
				// as it would show the HWND as a popup normally.
				case IPC_CB_ONSHOWWND:
					switch (wParam)
					{
					case IPC_CB_WND_PE:
						if (!createPl()) return 0;
						break;
#ifdef MINIBROWSER_SUPPORT
					case IPC_CB_WND_MB:
						if (!createMb()) return 0;
						break;
#endif
					case IPC_CB_WND_VIDEO:
#ifdef VIDDEBUG
						DebugString("Video : Got IPC_ONSHOW\n");
#endif
						if (!createVid())
						{
#ifdef VIDDEBUG
							DebugString("Video : SHOW was cancelled by script\n");
#endif
							return 0;
						}
						break;
					default:
						DebugStringW(L"embedWnd : Got IPC_ONSHOW\n");
						if (IsWindow((HWND)wParam))
						{
							HWND hTarget = (HWND)wParam;
							embedWindowState *ws = (embedWindowState*)GetWindowLongPtrW(hTarget, GWLP_USERDATA);
							if (ws && ws->me == hTarget)
							{
								GUID thisguid = EmbedWndGuid(ws).getGuid();
								if (INVALID_GUID == thisguid || NULL == updateEmb(thisguid, ws))
								{
									ifc_window *windowParent = createEmb(ws, true);
									if (NULL != windowParent)
									{										
										GuiObject *uiObject = windowParent->getGuiObject();
										if (NULL != uiObject)
										{			
											if (0 != (EMBED_FLAGS_FFCALLBACK & ws->flags) &&
												NULL != ws->callback)
											{
												ws->callback(ws, FFC_CREATEEMBED, (LPARAM)windowParent);
											}

											Layout *layout = uiObject->guiobject_getParentLayout();
											if (NULL != layout)
											{
												Container *container = layout->getParentContainer();
												if (NULL != container) container->setVisible(1);
												else layout->setVisible(1);
												windowParent->setVisible(1);
											}
										}

									}
									return (NULL != windowParent);
								}
							}
						}
						break;
					}
					break;
					// here we do the reverse, we detect that wa2 wants to close one of its windows, so we destroy our window
					// embedder (it will reparent the wa2 window back to its former parent and resize it back to where it was
					// on its own). when we return, winamp then hides the window.

					// NOTE! because of this, there might be a split second where the window is seen on the screen as a popup
					// after you closed the window (this won't happen for static containers [ie: pledit/video in mmd3] since
					// they are hidden rather than destroyed). this can be fixed in the future
				case IPC_CB_ONHIDEWND:
					switch (wParam)
					{
					case IPC_CB_WND_PE:
						destroyPl(hwndDlg, uMsg, wParam, lParam);
						break;
#ifdef MINIBROWSER_SUPPORT
					case IPC_CB_WND_MB:
						return destroyMb(hwndDlg, uMsg, wParam, lParam);
#endif
					case IPC_CB_WND_VIDEO:
#ifdef VIDDEBUG
						DebugString("Video : Got IPC_ONHIDE\n");
#endif
						return destroyVid(hwndDlg, uMsg, wParam, lParam);
					default:
						DebugStringW(L"embedWnd : Got IPC_ONHIDE\n");
						if (IsWindow((HWND)wParam))
						{
							HWND h = (HWND)wParam;
							embedWindowState *ws = (embedWindowState *)GetWindowLongPtrW(h, GWLP_USERDATA);
							if (ws) return destroyEmb(hwndDlg, uMsg, wParam, lParam, ws);
						}
						break;
					}
					break;
				case IPC_FF_ISMAINWND:
				{
					for (int i = 0;i < SkinParser::getNumContainers();i++)
					{
						Container *cont = SkinParser::enumContainer(i);
						if (!_wcsicmp(cont->getId(), L"main"))
						{
							for (int j = 0;j < cont->getNumLayouts();j++)
							{
								if (cont->enumLayout(j)->gethWnd() == (HWND)wParam) return 1;
							}
							break;
						}
					}
					return 0;
				}
				case IPC_FF_GETCONTENTWND:
					return (LRESULT)ff_ipc_getContentWnd((HWND)wParam);
				case IPC_FF_GETSKINCOLOR:
					ff_ipc_getSkinColor((ff_skincolor*)wParam);
					return 1;
				case IPC_FF_GENSKINBITMAP:
					ff_ipc_genSkinBitmap((ff_skinbitmap*)wParam);
					return 1;
				case IPC_FF_NOTIFYHOTKEY:
					if (processGenericHotkey((const char *)wParam)) return 0; // prevent gen_hotkey from processing the hotkey
					return 1; // let gen_hotkey process the hotkey
				case IPC_GET_GENSKINBITMAP:
					if (m_are_we_loaded)
					{
						if (wParam == 0)
						{
							if (m_lastskin_dir[0])
							{
								HBITMAP bm;
								wchar_t bitmapfilename[MAX_PATH] = {0};
								PathCombineW(bitmapfilename, m_lastskin_dir, L"genex.bmp");
								bm = (HBITMAP)LoadImageW(NULL, bitmapfilename, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
								if (bm) return (LRESULT)bm;
							}
							return (LRESULT)ff_genwa2skinbitmap();
						}
						else if (wParam == 4)
						{
							// TODO need to make sure we're covering everything needed for all of this!!
							int buf[6] = {0};
							// active text
							if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text.current"))
								buf[1] = SkinColor(L"wasabi.list.text.current");
							/*else
							{
								COLORREF act = Blenders::BLEND_AVG(selected, buf[0]);
								act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(buf[2], 0xFFFFFFFF) : act;
								act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(buf[2], 0xFF101010) : act;
								act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(selected, buf[0]) : act;
								act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(selected, buf[2]) : act;
								act = (lumidiff(act, buf[0]) < 0x3F || lumidiff(act, buf[2]) < 0x2F) ? Blenders::BLEND_AVG(buf[0], buf[2]) : act;
								buf[1] = act & 0xFFFFFF;
							}*/
							return buf[1];
						}
					}
					break;
				case IPC_FF_ONCOLORTHEMECHANGED:
				{
					embedEnumStruct cs = { embedUpdateColorProc, 0 };
					SendMessageW(hwndDlg, WM_WA_IPC, (WPARAM)&cs, IPC_EMBED_ENUM);
					if (wParam != 0xf00d) doplaylistcolors();
				}
				break;
				case IPC_PLAYLIST_MODIFIED:
				{
					Wa2PlaylistEditor::_onPlaylistModified();
				}
				break;
			}
		}
		else if (uMsg == WM_COMMAND && (LOWORD(wParam) == 40144 || LOWORD(wParam) == 40148))
		{
			// seek left/right, we need to disable them if we are in a menu, because this is a
			// plugin (ie: milkdrop) sending us the command manually as a forward rather than
			// sending us the WM_KEY* directly
			if (WASABI_API_WND->isKeyboardLocked()) return 0;
		}
		else if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && LOWORD(wParam) >= 43000 && LOWORD(wParam) < ffwindowstop)
		{
			int id = LOWORD(wParam) - 43000;

			int n = SkinParser::getNumContainers();
			if (id < n)
			{
				Container *cont = SkinParser::enumContainer(id);
				if (cont) cont->toggle();
				return 1;
			}
			id -= n;

			n = WASABI_API_WNDMGR->autopopup_getNumGuids();
			if (id < n)
			{
				GUID guid = WASABI_API_WNDMGR->autopopup_enumGuid(id);
				if (guid != INVALID_GUID)
					WASABI_API_WNDMGR->skinwnd_toggleByGuid(guid);
				return 1;
			}
			id -= n;

			n = WASABI_API_WNDMGR->autopopup_getNumGroups();
			if (id < n)
			{
				const wchar_t *gid = WASABI_API_WNDMGR->autopopup_enumGroup(id);
				if (gid && *gid)
					WASABI_API_WNDMGR->skinwnd_toggleByGroupId(gid);
				return 1;
			}

			/*      for (int c=0;c<SkinParser::getNumContainers();c++) {
			        Container *cont = SkinParser::enumContainer(c);
			        if (cont && wantContainerInMenu(cont)) {
				        if (cont->getName() == NULL) continue;
			          if (id == 0) {
			            if (cont)
			              cont->toggle();
			            return 1;
			          }
			          id--;
			        }
			      }
			      int n = WASABI_API_WNDMGR->autopopup_getNumGuids();
			      for (c=0;c<n;c++) {
			        GUID guid = WASABI_API_WNDMGR->autopopup_enumGuid(c);
			        const char *groupdesc = WASABI_API_WNDMGR->autopopup_enumGuidDescription(c);
			        if (guid != INVALID_GUID  && groupdesc && *groupdesc) {
			          if (id == 0) {
			            WASABI_API_WNDMGR->skinwnd_toggleByGuid(guid);
			            return 1;
			          }
			          id--;
			        }
			      }
			      n = WASABI_API_WNDMGR->autopopup_getNumGroups();
			      for (c=0;c<n;c++) {
			        const char *gid = WASABI_API_WNDMGR->autopopup_enumGroup(c);
			        const char *groupdesc = WASABI_API_WNDMGR->autopopup_enumGroupDescription(c);
			        if (id && groupdesc && *gid && *groupdesc) {
			          if (id == 0) {
			            WASABI_API_WNDMGR->skinwnd_toggleByGroupId(gid);
			            return 1;
			          }
			          id--;
			        }
			      }*/
			return 0;
		}
		else if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && LOWORD(wParam) >= 44000 && LOWORD(wParam) < ffoptionstop)
		{
			int id = LOWORD(wParam);
			MenuActions::toggleOption(id - 44000);
		}
		else if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && LOWORD(wParam) >= 44500 && LOWORD(wParam) < 45000)
		{
			int id = LOWORD(wParam) - 44500;
			WASABI_API_SKIN->colortheme_setColorSet(WASABI_API_SKIN->colortheme_enumColorSet(id));
		}
		else if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && LOWORD(wParam) >= 42000 && LOWORD(wParam) < ffwoptionstop)
		{
			int id = LOWORD(wParam);
			MenuActions::toggleWindowOption(id - 42000);
		}
		else if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && LOWORD(wParam) == WINAMP_MAIN_WINDOW)
		{
			for (int i = 0;i < SkinParser::getNumContainers();i++)
			{
				Container *cont = SkinParser::enumContainer(i);
				if (!_wcsicmp(cont->getId(), L"main"))
				{
					cont->toggle();
					return 1;
				}
			}
		}
		else if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND) && LOWORD(wParam) == WINAMP_OPTIONS_EQ)
		{
			if (!gothrueqmsg)
				return 0;
		}
		else if ((uMsg == WM_COMMAND || uMsg == WM_SYSCOMMAND))
		{
			switch (LOWORD(wParam))
			{
				case ID_CONTROLMENU_OPACITY_10: controlOpacity(10); return 0;
				case ID_CONTROLMENU_OPACITY_20: controlOpacity(20); return 0;
				case ID_CONTROLMENU_OPACITY_30: controlOpacity(30); return 0;
				case ID_CONTROLMENU_OPACITY_40: controlOpacity(40); return 0;
				case ID_CONTROLMENU_OPACITY_50: controlOpacity(50); return 0;
				case ID_CONTROLMENU_OPACITY_60: controlOpacity(60); return 0;
				case ID_CONTROLMENU_OPACITY_70: controlOpacity(70); return 0;
				case ID_CONTROLMENU_OPACITY_80: controlOpacity(80); return 0;
				case ID_CONTROLMENU_OPACITY_90: controlOpacity(90); return 0;
				case ID_CONTROLMENU_OPACITY_100: controlOpacity(100); return 0;
				case ID_CONTROLMENU_SCALING_50: controlScaling(0.5); return 0;
				case ID_CONTROLMENU_SCALING_75: controlScaling(0.75); return 0;
				case ID_CONTROLMENU_SCALING_100: controlScaling(1.0); return 0;
				case ID_CONTROLMENU_SCALING_125: controlScaling(1.25); return 0;
				case ID_CONTROLMENU_SCALING_150: controlScaling(1.5); return 0;
				case ID_CONTROLMENU_SCALING_200: controlScaling(2.0); return 0;
				case ID_CONTROLMENU_SCALING_250: controlScaling(2.5); return 0;
				case ID_CONTROLMENU_SCALING_300: controlScaling(3.0); return 0;
				case ID_CONTROLMENU_SCALING_LOCKED: lockScaling(1); return 0;
				case ID_CONTROLMENU_SCALING_FOLLOWDOUBLESIZE: lockScaling(0); return 0;
				case ID_CONTROLMENU_SCALING_CUSTOM: customScaling(); return 0;
				case ID_CONTROLMENU_OPACITY_CUSTOM: customOpacity(); return 0;
				case ID_CONTROLMENU_OPACITY_AUTO100_HOVER: autoOpacifyHover(); return 0;
				case ID_CONTROLMENU_OPACITY_AUTO100_FOCUS: autoOpacifyFocus(); return 0;
				case SC_MOVE:
				{
					Layout *l = SkinParser::getMainLayout();
					if (l != NULL)
					{
						Container *c = l->getParentContainer();
						if (c)
						{
							Layout *ll = c->getCurrentLayout();
							if (ll) l = ll;
							if (l)
							{
								if (!l->isVisible()) l->setVisible(1);
								return SendMessageW(l->gethWnd(), WM_SYSCOMMAND, wParam, lParam);
							}
						}
					}
					break;
				}
				case ID_CONTROLMENU_TOOLBAR_DISABLED: controlAppBar(APPBAR_NOTDOCKED); break;
				case ID_CONTROLMENU_TOOLBAR_TOP: controlAppBar(APPBAR_TOP); break;
				case ID_CONTROLMENU_TOOLBAR_LEFT: controlAppBar(APPBAR_LEFT); break;
				case ID_CONTROLMENU_TOOLBAR_RIGHT: controlAppBar(APPBAR_RIGHT); break;
				case ID_CONTROLMENU_TOOLBAR_BOTTOM: controlAppBar(APPBAR_BOTTOM); break;
				case ID_CONTROLMENU_TOOLBAR_ALWAYSONTOP: controlAppBarAOT(); break;
				case ID_CONTROLMENU_TOOLBAR_AUTOHIDE: controlAppBarAH(); break;
				case ID_CONTROLMENU_TOOLBAR_AUTODOCKONDRAG: cfg_options_appbarondrag = !cfg_options_appbarondrag; break;
			}
		}
	}

	return CallWinampWndProc(hwndDlg, uMsg, wParam, lParam);
}

void onLayoutChanged()
{
	if (DEFERREDCALLBACKMSG > 65536)
		PostMessage(wa2.getMainWindow(), WM_WA_IPC, 1, DEFERREDCALLBACKMSG);
}

#ifdef DEBUG_CAPTURES
int tid = 0;

VOID TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD time)
{
	DebugString("Capture belongs to %x (foregroundwnd = %x)\n", GetCapture(), GetForegroundWindow());
}
#endif

template <class api_T>
static void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
static void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC && api_t)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

//-----------------------------------------------------------------------------------------------
// initializes freeform library
//-----------------------------------------------------------------------------------------------
int     m_loaded_at_all = 0;
StringW g_resourcepath;

void initFFApi()
{
	if ( !m_loaded_at_all )
	{
		m_loaded_at_all = 1;
		api_service *svc = (api_service *)SendMessageW( wa2.getMainWindow(), WM_WA_IPC, 0, IPC_GET_API_SERVICE );

		ServiceBuild( AGAVE_API_PLAYLISTS,       api_playlistsGUID );
		ServiceBuild( AGAVE_API_ALBUMART,        albumArtGUID );
		ServiceBuild( AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID );
		ServiceBuild( WAC_API_DOWNLOADMANAGER, DownloadManagerGUID );
		ServiceBuild( WASABI_API_COLORTHEMES,    ColorThemesAPIGUID );
		ServiceBuild( WASABI_API_PALETTE,        PaletteManagerGUID );
		ServiceBuild( WASABI_API_THREADPOOL,     ThreadPoolGUID );

		ApiInit::init( hInstance, wa2.getMainWindow(), svc );
		cfgitems = new Wa2CfgItems();

#ifdef DEBUG_CAPTURES
		tid = SetTimer( NULL, 0x159, 50, (TIMERPROC)TimerProc );
#endif

		wchar_t filename[ WA_MAX_PATH ] = { 0 };
		GetModuleFileNameW( hInstance, filename, WA_MAX_PATH );
		PathParserW pp( filename );
		StringW path;
		for ( int i = 0; i < pp.getNumStrings() - 1; i++ )
		{
			path.AppendPath( pp.enumString( i ) );
		}
		path.AppendPath( L"freeform" );
		path.AppendFolder( L"wacs" );

		// we can load a somewhat restricted version of the wac format

		ComponentManager::loadAll( path );
		ComponentManager::postLoad();

		ApiInit::widgets->loadResources();

		startFSMonitor();
	}
	cfg_options_alwaysontop.setValueAsInt( wa2.isOnTop() );
}

//-----------------------------------------------------------------------------------------------
// shutdown ff lib
//-----------------------------------------------------------------------------------------------
void shutdownFFApi()
{
	if (m_loaded_at_all)
	{
		ServiceRelease( AGAVE_API_PLAYLISTS,       api_playlistsGUID);
		ServiceRelease( AGAVE_API_ALBUMART,        albumArtGUID);
		ServiceRelease( AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
		ServiceRelease( WAC_API_DOWNLOADMANAGER, DownloadManagerGUID);
		ServiceRelease( WASABI_API_COLORTHEMES,    ColorThemesAPIGUID);
		ServiceRelease( WASABI_API_PALETTE,        PaletteManagerGUID);
		ServiceRelease( WASABI_API_THREADPOOL,     ThreadPoolGUID);

		stopFSMonitor();

		// shutdown the library
#ifdef DEBUG_CAPTURES
		KillTimer(NULL, tid);
#endif

		delete cfgitems; cfgitems = NULL;
		ApiInit::shutdown();
ComponentManager::unloadAll();

		m_loaded_at_all = 0;
	}
}

static void MakeControlMenu()
{
	if (controlmenu)
		return ;

	controlmenu = WASABI_API_LOADMENU(IDR_CONTROLMENU);

	if (!Wasabi::Std::Wnd::isDesktopAlphaAvailable())
		EnableMenuItem(controlmenu, 0, MF_BYPOSITION | MF_GRAYED);
}

//-----------------------------------------------------------------------------------------------
// go freeform
//-----------------------------------------------------------------------------------------------
void init_inst()
{
	if ( m_are_we_loaded )
		return;

	// set the classic main window to be transparent
	// so we can move it around the screen to allow
	// certain Windows 8 modes as well as improving
	// where it is placed for some multi-mon setups
	Wasabi::Std::Wnd::setLayeredWnd( wa2.getMainWindow(), 1 );
	Wasabi::Std::Wnd::setLayeredAlpha( wa2.getMainWindow(), 0 );

	initEgg();

	wa2.setDrawBorders( 0 );
	wa2.disableSkinnedCursors( 1 );

	int wasminimized = IsIconic( wa2.getMainWindow() );
	if ( wasminimized )
		ShowWindow( wa2.getMainWindow(), SW_RESTORE );

	MakeControlMenu();

	going_freeform = 1;

	Wa2WndEmbed::rememberVisibleWindows();

	if ( wa2.isWindowVisible( IPC_GETWND_PE ) )
	{
		if ( !m_loading_at_startup ) WritePrivateProfileStringW( L"gen_ff", L"classicpe", L"1", INI_FILE );
	}
	else
		if ( !m_loading_at_startup ) WritePrivateProfileStringW( L"gen_ff", L"classicpe", L"0", INI_FILE );

	if ( wa2.isWindowVisible( IPC_GETWND_EQ ) )
	{
		gothrueqmsg = 1;
		SendMessageW( plugin.hwndParent, WM_COMMAND, WINAMP_OPTIONS_EQ, 0 );
		gothrueqmsg = 0;
		if ( !m_loading_at_startup ) WritePrivateProfileStringW( L"gen_ff", L"classiceq", L"1", INI_FILE );
	}
	else
		if ( !m_loading_at_startup ) WritePrivateProfileStringW( L"gen_ff", L"classiceq", L"0", INI_FILE );

	removeEq();

	if ( SendMessageW( plugin.hwndParent, WM_WA_IPC, 0, IPC_ISMAINWNDVISIBLE ) )
	{
		SendMessageW( plugin.hwndParent, WM_COMMAND, WINAMP_MAIN_WINDOW, 0 );
		if ( !m_loading_at_startup ) WritePrivateProfileStringW( L"gen_ff", L"classicmw", L"1", INI_FILE );
	}
	else
		if ( !m_loading_at_startup ) WritePrivateProfileStringW( L"gen_ff", L"classicmw", L"0", INI_FILE );

	m_are_we_loaded = 1;

	initFFApi();

	// redirect drag and drop to winamp2 by default
	WASABI_API_WND->setDefaultDropTarget( (void *)wa2.getDropTarget() );

	// this installs a bunch of predefined groups to map wa3 to wa2 functionnality
	groups = new Wa2Groupdefs();

	// now load a skin ! the path to the skin is extracted and temporarilly becomes the official skins directory

	StringW skinname = m_lastskin_nam;

	removeSkinExtension( skinname );

	before_startup_callback = 1;
	WASABI_API_SKIN->skin_switchSkin( skinname, m_lastskin_dir );

	/*  // if we wanted to have drag&drop support for just the main window, we'd do it this way :-)
	  Container *maincontainer = SkinParser::getContainer("main");
	  if (maincontainer != NULL) {
		for (int i=0;i<maincontainer->getNumLayouts();i++) {
		  Layout *layout = maincontainer->enumLayout(i);
		  if (layout != NULL) {
			layout->setDropTarget((void *)wa2.getDropTarget());
		  }
		}
	  }*/

	  // send event for dialog parent
	onLayoutChanged();

	if ( wa2.export_sa_setreq )
		wa2.export_sa_setreq( 1 );
	
	shuffle.setValueAsInt( wa2.getShuffle() );
	
	int rep    = wa2.getRepeat();
	int manadv = wa2.getManualPlaylistAdvance();
	
	disable_set_wa2_repeat = 1;
	repeat.setValueAsInt( ( rep && manadv ) ? -1 : rep );
	disable_set_wa2_repeat = 0;

	if ( DEFERREDCALLBACKMSG > 65536 )
		PostMessage( wa2.getMainWindow(), WM_WA_IPC, 2, DEFERREDCALLBACKMSG );

	// so if some embedwindows are already visible, they update their look
	PostMessage( wa2.getMainWindow(), WM_WA_IPC, 0xf00d, IPC_FF_ONCOLORTHEMECHANGED );

	// monitor color theme
	colorThemeMonitor = new ColorThemeMonitor();

	syncDoubleSize( 1 );
	syncDisplayMode();

	going_freeform = 0;

	if ( wa2.getWnd( IPC_GETWND_VIDEO ) )
	{
		oldVideoWnd = wa2.getWnd( IPC_GETWND_VIDEO );
		if ( !oldVideoWndProc )
			oldVideoWndProc = (WNDPROC)SetWindowLongPtrW( oldVideoWnd, GWLP_WNDPROC, (LONG_PTR)newVideoWndProc );
	}

	if ( wasminimized )
		ShowWindow( wa2.getMainWindow(), SW_MINIMIZE );

	if ( eggstat )
		SetTimer( wa2.getMainWindow(), UPDATE_EGG, 25, NULL );
}

void quit_inst()
{
	if (!m_are_we_loaded) return ;

	KillTimer(wa2.getMainWindow(), UPDATE_EGG);

	wa2.setDrawBorders(1);
	wa2.disableSkinnedCursors(0);

	KillTimer(wa2.getMainWindow(), 0xC0DE);
	KillTimer(wa2.getMainWindow(), 0xC0DE + 1);

	EnableMenuItem(wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_OPTIONS), WINAMP_OPTIONS_DSIZE, MF_ENABLED);
	EnableMenuItem(GetSubMenu(wa2.getPopupMenu(), 11 + wa2.adjustOptionsPopupMenu(0)), WINAMP_OPTIONS_DSIZE, MF_ENABLED);

	if (oldVideoWnd && oldVideoWndProc)
		SetWindowLongPtrW(oldVideoWnd, GWLP_WNDPROC, (LONG_PTR)oldVideoWndProc);
	oldVideoWndProc = 0;
	oldVideoWnd = 0;

	removeWindowOptionsFromContextMenu();

	delete mainLayoutMonitor;
	mainLayoutMonitor=0;

	wa2.setDialogBoxParent(NULL);
	unpopulateWindowsMenus();
	restoreEq();

	// unload the skin -- skinspath is restored to default
	WASABI_API_SKIN->skin_unloadSkin();

	// delete predefined groups service
	delete groups; groups = NULL;

	// delete options
	delete ffoptions; ffoptions = NULL;

	// stop monitoring color theme
	delete colorThemeMonitor; colorThemeMonitor = NULL;

	if (wa2.export_sa_setreq) wa2.export_sa_setreq(0);

	m_are_we_loaded = 0;

	int classicmw = GetPrivateProfileIntW(L"gen_ff", L"classicmw", 1, INI_FILE);
	if (classicmw && !SendMessageW(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISMAINWNDVISIBLE))
		SendMessageW(plugin.hwndParent, WM_COMMAND, WINAMP_MAIN_WINDOW, 0);

	CheckMenuItem(wa2.getPopupMenu(), WINAMP_MAIN_WINDOW, classicmw ? MF_CHECKED : MF_UNCHECKED);

	if (GetPrivateProfileIntW(L"gen_ff", L"classiceq", 1, INI_FILE) && !wa2.isWindowVisible(IPC_GETWND_EQ))
	{
		gothrueqmsg = 1;
		SendMessageW(plugin.hwndParent, WM_COMMAND, WINAMP_OPTIONS_EQ, 0);
		gothrueqmsg = 0;
	}

	if (GetPrivateProfileIntW(L"gen_ff", L"classicpe", 1, INI_FILE) && !wa2.isWindowVisible(IPC_GETWND_PE))
	{
		SendMessageW(plugin.hwndParent, WM_COMMAND, WINAMP_OPTIONS_PLEDIT, 0);
	}

	// restore the classic main window to be solid
	Wasabi::Std::Wnd::setLayeredWnd(wa2.getMainWindow(), 0);
}

//-----------------------------------------------------------------------------------------------
// init (from Winamp2)
//-----------------------------------------------------------------------------------------------
int init()
{
	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*) SendMessageW(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1) WASABI_API_SVC = NULL;
	if (!WASABI_API_SVC || WASABI_API_SVC == (api_service *)1)
		return GEN_INIT_FAILURE;

	static wchar_t modskin[128];
	INI_FILE = (wchar_t*) SendMessageW(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIFILEW);
	INI_DIR = (wchar_t*) SendMessageW(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORYW);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,GenFFLangGUID);

	swprintf(szDescription, ARRAYSIZE(szDescription),
			 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_MODERN_SKINS), VERSION);
	plugin.description = (char*)szDescription;

	wa2.init(plugin.hwndParent);

	DEFERREDCALLBACKMSG = SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)"gen_ff_deferred", IPC_REGISTER_WINAMP_IPCMESSAGE);
	UPDATEDIALOGBOXPARENTMSG = SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)"gen_ff_update", IPC_REGISTER_WINAMP_IPCMESSAGE);

	// subclass the Winamp2 main window to receive our callbacks
	wa_oldWndProc = (WNDPROC) SetWindowLongPtrW(wa2.getMainWindow(), GWLP_WNDPROC, (LONG_PTR)wa_newWndProc);
	
	ffPrefsItem.dlgID = IDD_PREFS;
	ffPrefsItem.name  = WASABI_API_LNGSTRINGW_BUF(IDS_MODERN_SKINS,modskin,128);
	ffPrefsItem.proc  = (void*)ffPrefsProc;
	ffPrefsItem.hInst = WASABI_API_LNG_HINST;
	ffPrefsItem.where = 2; //skins subtreeitem
	SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&ffPrefsItem, IPC_ADD_PREFS_DLGW);

	checkMlPresent();

	m_loading_at_startup = 1;
	onSkinSwitch();
	m_loading_at_startup = 0;
	return 0;
}

void RestoreClassicWinamp(int was_loaded)
{
	// JF> my proposed fix to the problems :)
	// investigating doing this from winamp.exe on startup, gotta figure it out
	if (INI_FILE && was_loaded) // restore winamp.ini to about what it shoulda been
	{
		int classicpews   = GetPrivateProfileIntW(L"gen_ff", L"classicplws",       0, INI_FILE);
		int classicwidth  = GetPrivateProfileIntW(L"gen_ff", L"classicplwidth",  275, INI_FILE);
		int classicheight = GetPrivateProfileIntW(L"gen_ff", L"classicplheight", 145, INI_FILE);
		int classicmw     = GetPrivateProfileIntW(L"gen_ff", L"classicmw",         1, INI_FILE);
		int classiceq     = GetPrivateProfileIntW(L"gen_ff", L"classiceq",         1, INI_FILE);
		wchar_t buf[64] = {0};

		wsprintfW(buf, L"%d", classicheight);
		if (classicpews)
		{
			WritePrivateProfileStringW(L"winamp", L"pe_height", L"14", INI_FILE);
			WritePrivateProfileStringW(L"winamp", L"pe_height_ws", buf, INI_FILE);
		}
		else
		{
			WritePrivateProfileStringW(L"winamp", L"pe_height", buf, INI_FILE);
			WritePrivateProfileStringW(L"winamp", L"pe_height_ws", L"", INI_FILE);
		}
		wsprintfW(buf, L"%d", classicwidth);
		WritePrivateProfileStringW(L"winamp", L"pe_width", buf, INI_FILE);

		WritePrivateProfileStringW(L"winamp", L"eq_open", classiceq ? L"1" : L"0", INI_FILE);
		WritePrivateProfileStringW(L"winamp", L"mw_open", classicmw ? L"1" : L"0", INI_FILE);
	}
}

//-----------------------------------------------------------------------------------------------
// quit (from Winamp 2)
//-----------------------------------------------------------------------------------------------
void quit()
{
	int was_loaded = m_are_we_loaded;
	quit_inst();

	RestoreClassicWinamp(was_loaded);
	// restore wa2's windowproc
	//SetWindowLong(wa2.getMainWindow(), GWL_WNDPROC, (LONG)wa_oldWndProc);
	shutdownFFApi();
}

//-----------------------------------------------------------------------------------------------
// About box wndproc
//-----------------------------------------------------------------------------------------------
ifc_window *about_group = NULL;
StringW oldrenderer;
BOOL CALLBACK aboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		oldrenderer = cfg_options_fontrenderer.getValue();
		if (!WCSCASEEQLSAFE(cfg_options_fontrenderer.getValue(), L"FreeType"))
			cfg_options_fontrenderer.setValue(L"FreeType");
		about_group = WASABI_API_SKIN->group_create(L"wasabi.gen_ff.about");
		about_group->setVirtual(0);
		HWND w = GetDlgItem(hwndDlg, IDC_STATIC_GROUP);
		about_group->setStartHidden(1);
		about_group->init(WASABI_API_WND->main_getRootWnd(), 1);
		SetWindowLong(about_group->gethWnd(), GWL_STYLE, GetWindowLong(about_group->gethWnd(), GWL_STYLE) | WS_CHILD);
		SetParent(about_group->gethWnd(), w);
		SetWindowLong(w, GWL_STYLE, GetWindowLong(w, GWL_STYLE) | WS_CLIPCHILDREN);
		RECT r;
		GetClientRect(w, &r);
		about_group->resize(r.left + 1, r.top + 1, r.right - r.left - 2, r.bottom - r.top - 2);
		about_group->setVisible(1);
		C_Group g(about_group->getGuiObject()->guiobject_getScriptObject());
		ScriptObject *ver = g.findObject(L"version");
		if (ver)
		{
			C_Text t(ver);
			t.setText(StringPrintfW(L". © 2003-2023 Winamp SA %s", VERSION));
		}
		ShowWindow(about_group->gethWnd(), SW_NORMAL);
		return 1;
	}
	case WM_DESTROY:
		WASABI_API_SKIN->group_destroy(about_group);
		about_group = NULL;
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
	case IDOK: case IDCANCEL:
			if (!WCSCASEEQLSAFE(cfg_options_fontrenderer.getValue(), oldrenderer))
				cfg_options_fontrenderer.setValue(oldrenderer);
			EndDialog(hwndDlg, 0);
			return 0;
		}
		return 0;
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------
// configure plugin (from Winamp 2)
//-----------------------------------------------------------------------------------------------
void config()
{
	if (m_loaded_at_all)
	{
		StringW skin = WASABI_API_SKIN->getSkinName();
		wchar_t _skName[64] = {0};
		WASABI_API_LNGSTRINGW_BUF(IDS_NO_SKIN_LOADED_,_skName,64);

		if ((!skin.iscaseequal(_skName)) || guiTree->getNumObject() > 0)
		{
			WASABI_API_DIALOGBOXW(IDD_ABOUT, wa2.getPreferencesWindow(), aboutProc);
			return ;
		}
	}

	MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMS),0};
	msgbx.lpszText = WASABI_API_LNGSTRINGW(IDS_MODERN_SKIN_SUPPORT_CLASSIC);
	msgbx.lpszCaption = szDescription;
	msgbx.lpszIcon = MAKEINTRESOURCEW(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	MessageBoxIndirectW(&msgbx);
}

//-----------------------------------------------------------------------------------------------
// expose the genpurp plugin interface to dll
//-----------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() { return &plugin; }

//-----------------------------------------------------------------------------------------------
// a window was right clicked where there was no custom context menu available, spawn Wa2's menu
//-----------------------------------------------------------------------------------------------
void appContextMenu(ifc_window *w)
{
	if (!WASABI_API_WND->rootwndIsValid(w)) return ;
	WASABI_API_WND->appdeactivation_setbypass(1);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	w->setFocus();
	DWORD p = GetMessagePos();
	int x = GET_X_LPARAM(p);
	int y = GET_Y_LPARAM(p);
	addWindowOptionsToContextMenu(w);
	wa2.triggerPopupMenu(x, y);
	WASABI_API_WND->appdeactivation_setbypass(0);
}

//-----------------------------------------------------------------------------------------------
void updateControlMenu(ifc_window *w)
{
	int curalpha = 255;
	double curratio = 1.;
	int opacitysafe = 0;
	int scalelocked = 0;
	int auto100_hover = 0;
	int auto100_focus = 0;

	if (g_controlMenuTarget == NULL) return ;

	if (w)
	{
		ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
		if (rootparent && rootparent->getInterface(layoutGuid))
		{
			Layout *l = static_cast<Layout*>(rootparent);
			opacitysafe = l->isTransparencySafe();
			scalelocked = l->isScaleLocked();
			curratio = w->getRenderRatio();
			if (!cfg_uioptions_linkallalpha.getValueAsInt())
			{
				auto100_hover = l->getAutoOpacify() == 1;
				auto100_focus = l->getAutoOpacify() == 2;
				curalpha = l->getAlpha();
			}
			else
			{
				auto100_hover = l->getAlphaMgr()->getAutoOpacify() == 1;
				auto100_focus = l->getAlphaMgr()->getAutoOpacify() == 2;
				curalpha = l->getAlphaMgr()->getGlobalAlpha();
			}
		}
	}

	HMENU ctrlmenu = GetSubMenu(controlmenu, 0);
	EnableMenuItem(ctrlmenu, 0, MF_BYPOSITION | (opacitysafe ? MF_ENABLED : MF_GRAYED));
	HMENU scalemenu = GetSubMenu(ctrlmenu, 1);
	HMENU alphamenu = GetSubMenu(ctrlmenu, 0);
	int uselocks = cfg_uioptions_uselocks.getValueAsInt();
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_LOCKED, MF_BYCOMMAND | ((scalelocked || !uselocks) ? MF_CHECKED : MF_UNCHECKED));
	EnableMenuItem(scalemenu, ID_CONTROLMENU_SCALING_LOCKED, MF_BYCOMMAND | (uselocks ? MF_ENABLED : MF_GRAYED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_FOLLOWDOUBLESIZE, MF_BYCOMMAND | (scalelocked || !uselocks ? MF_UNCHECKED : MF_CHECKED));
	EnableMenuItem(scalemenu, ID_CONTROLMENU_SCALING_FOLLOWDOUBLESIZE, MF_BYCOMMAND | (uselocks ? MF_ENABLED : MF_GRAYED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_AUTO100_HOVER, MF_BYCOMMAND | (auto100_hover ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_AUTO100_FOCUS, MF_BYCOMMAND | (auto100_focus ? MF_CHECKED : MF_UNCHECKED));

	int v = (int)((curalpha / 255.0f * 100.0f) + 0.5f);
	int u = (int)((curratio * 100.0f) + 0.5f);

	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_10, MF_BYCOMMAND | (v == 10 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_20, MF_BYCOMMAND | (v == 20 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_30, MF_BYCOMMAND | (v == 30 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_40, MF_BYCOMMAND | (v == 40 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_50, MF_BYCOMMAND | (v == 50 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_60, MF_BYCOMMAND | (v == 60 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_70, MF_BYCOMMAND | (v == 70 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_80, MF_BYCOMMAND | (v == 80 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_90, MF_BYCOMMAND | (v == 90 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_100, MF_BYCOMMAND | (v == 100 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_50, MF_BYCOMMAND | (u == 50 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_75, MF_BYCOMMAND | (u == 75 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_100, MF_BYCOMMAND | (u == 100 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_125, MF_BYCOMMAND | (u == 125 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_150, MF_BYCOMMAND | (u == 150 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_200, MF_BYCOMMAND | (u == 200 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_250, MF_BYCOMMAND | (u == 250 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_300, MF_BYCOMMAND | (u == 300 ? MF_CHECKED : MF_UNCHECKED));

	if (u != 50 && u != 75 && u != 100 && u != 125 && u != 150 && u != 200 && u != 250 && u != 300)
	{
		ModifyMenuW(scalemenu, ID_CONTROLMENU_SCALING_CUSTOM, MF_BYCOMMAND | MF_STRING, ID_CONTROLMENU_SCALING_CUSTOM, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_CUSTOM_X_PERCENT), u));
		CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_CUSTOM, MF_BYCOMMAND | MF_CHECKED);
	}
	else
	{
		ModifyMenuW(scalemenu, ID_CONTROLMENU_SCALING_CUSTOM, MF_BYCOMMAND | MF_STRING, ID_CONTROLMENU_SCALING_CUSTOM, WASABI_API_LNGSTRINGW(IDS_CUSTOM));
		CheckMenuItem(scalemenu, ID_CONTROLMENU_SCALING_CUSTOM, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (((float)v / 10.0) - (v / 10) != 0.0)
	{
		ModifyMenuW(alphamenu, ID_CONTROLMENU_OPACITY_CUSTOM, MF_BYCOMMAND | MF_STRING, ID_CONTROLMENU_OPACITY_CUSTOM, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_CUSTOM_X_PERCENT), v));
		CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_CUSTOM, MF_BYCOMMAND | MF_CHECKED);
	}
	else
	{
		ModifyMenuW(alphamenu, ID_CONTROLMENU_OPACITY_CUSTOM, MF_BYCOMMAND | MF_STRING, ID_CONTROLMENU_OPACITY_CUSTOM, WASABI_API_LNGSTRINGW(IDS_CUSTOM));
		CheckMenuItem(alphamenu, ID_CONTROLMENU_OPACITY_CUSTOM, MF_BYCOMMAND | MF_UNCHECKED);
	}
}

//-----------------------------------------------------------------------------------------------
// a window has triggered the control menu (control scale & alpha)
//-----------------------------------------------------------------------------------------------
void appControlMenu(ifc_window *w)
{
	WASABI_API_WND->appdeactivation_setbypass(1);
	int x, y;
	if (w)
	{
		Wasabi::Std::getMousePos(&x, &y);
		g_controlMenuTarget = w;
		updateControlMenu(w);
		updateAppBarMenu(w);
		HMENU ctrlmenu = GetSubMenu(controlmenu, 0);
		DoTrackPopup(ctrlmenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, x, y, wa2.getMainWindow());
	}
	WASABI_API_WND->appdeactivation_setbypass(0);
}

//-----------------------------------------------------------------------------------------------
// a close application button was clicked
//-----------------------------------------------------------------------------------------------
void appQuit()
{
	if (!wa2.isExitEnabled()) return ;
	wa2.quit(); // this is in fact *posting* a quit message
}


//-----------------------------------------------------------------------------------------------
// a fatal skin error occured, we should revert to the wa2 skin (deferred)
//-----------------------------------------------------------------------------------------------
#define DCB_UNLOADSKIN 64

class deferredUnloadSkin : public TimerClientDI
{
public:
	deferredUnloadSkin()
	{
		timerclient_postDeferredCallback(DCB_UNLOADSKIN, 0);
	}

	virtual int timerclient_onDeferredCallback(intptr_t param1, intptr_t param2)
	{
		if (param1 == DCB_UNLOADSKIN)
		{
			quit_inst();
			delete this;
			return 1;
		}
		else return TimerClientDI::timerclient_onDeferredCallback(param1, param2);
	}
};

//-----------------------------------------------------------------------------------------------
// this is the actual event for skin fata errors, it is called when a try/except block failed
// upon calling a skin operation which should not be able to fail (ie: calling a script function pointer)
//-----------------------------------------------------------------------------------------------
void onFatalSkinError()
{
	new deferredUnloadSkin();
}


//-----------------------------------------------------------------------------------------------
// map a GUID to the toggling of a window for which there is no permanent wndCreationService,
// ie: library, prefs, avs
//-----------------------------------------------------------------------------------------------

// called by a skinwnd_toggle call when the guid isn't found in the wndcreation services
int onCreateExternalWindowGuid(GUID g)
{
	if (g == library_guid)
	{
		SendMessageW(wa2.getMainWindow(), WM_COMMAND, ID_FILE_SHOWLIBRARY, 0);
		return 1;
	}
	else if (g == preferences_guid)
	{
		SendMessageW(wa2.getMainWindow(), WM_COMMAND, WINAMP_OPTIONS_PREFS, 0);
		return 1;
	}
	else if (g == about_guid)
	{
		SendMessageW(wa2.getMainWindow(), WM_COMMAND, WINAMP_HELP_ABOUT, 0);
		return 1;
	}
	else if (g == lightning_bolt_guid)
	{
		SendMessageW(wa2.getMainWindow(), WM_COMMAND, WINAMP_LIGHTNING_CLICK, 0);
		return 1;
	}
	else if (g == colorthemes_guid)
	{
		last_page.setData(L"3");
		SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&ffPrefsItem, IPC_OPENPREFSTOPAGE);
		return 1;
	}
	else if (g == avs_guid)
	{
		if (wa2.isVisRunning())
		{
			SetTimer(wa2.getMainWindow(), 0xC0DE, 50, NULL);
			SetTimer(wa2.getMainWindow(), 0xC0DE + 1, 5000, NULL);
		}
		else
			wa2.toggleVis();
		return 1;
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------
void onToggleDesktopAlpha(int v)
{
	if (!toggle_from_wa2 && subWndId == 0 && IsWindow(subWnd)) SendMessageW(subWnd, WM_INITDIALOG, 0, 0);
}


//-----------------------------------------------------------------------------------------------
// changes the name of a container so that static containers with the wrong name don't load the config
// values off the wrong name. this should match the WindowText of the embededwindow. note: this is only
// necessary for static containers holding an internal embedwnd (ie: library, avs, but not third party wnds
// as they can't be refered to by GUID anyway [if they can't be refered to by GUID, you can't make a static
// container for them])
//-----------------------------------------------------------------------------------------------

const wchar_t *onTweakContainerNameW(const wchar_t *name)
{
	static wchar_t tweaked[96];
	ZERO(tweaked);
	if (!_wcsicmp(name, WASABI_API_LNG->GetStringFromGUIDW(GenMlLangGUID, plugin.hDllInstance, 18, tweaked, sizeof(tweaked)/sizeof(wchar_t))) ||
	    !_wcsicmp(name, L"Media Library") ||
	    !_wcsicmp(name, L"Winamp Library") ||
	    !_wcsicmp(name, L"Library"))
	{
		return WASABI_API_LNGSTRINGW_BUF(IDS_MEDIA_LIBRARY,tweaked, sizeof(tweaked)/sizeof(wchar_t));
	}
	if (!_wcsicmp(name, L"Avs")) return WASABI_API_LNGSTRINGW_BUF(IDS_VISUALIZATIONS,tweaked, sizeof(tweaked)/sizeof(wchar_t));
	return name;
}

const wchar_t *GetMenuItemString(HMENU menu, int id, int bypos)
{
	static StringW rtn;
	rtn.trunc(0);
	MENUITEMINFOW info = {sizeof(info), MIIM_DATA | MIIM_TYPE | MIIM_STATE | MIIM_ID, MFT_STRING, };
	GetMenuItemInfoW(menu, id, bypos, &info);
	if (info.cch > 0)
	{
		info.dwTypeData = WMALLOC(++info.cch + 1);
		GetMenuItemInfoW(menu, id, bypos, &info);
		info.dwTypeData[info.cch] = 0;
		rtn = info.dwTypeData;
		FREE(info.dwTypeData);
	}
	return rtn;
}

StringW eqmenustring;

//-----------------------------------------------------------------------------------------------
void removeEq()
{
	if (eqremoved) return ;
	eqremoved = 1;
	eqmenustring = GetMenuItemString(wa2.getPopupMenu(), WINAMP_OPTIONS_EQ, FALSE);
	RemoveMenu(wa2.getPopupMenu(), WINAMP_OPTIONS_EQ, MF_BYCOMMAND);
	wa2.adjustOptionsPopupMenu(-1);
}

//-----------------------------------------------------------------------------------------------
void restoreEq()
{
	if (!eqremoved) return ;
	MENUITEMINFOW i = {sizeof(i), MIIM_ID | MIIM_STATE | MIIM_TYPE, MFT_STRING, wa2.isWindowVisible(IPC_GETWND_EQ) ? MFS_CHECKED : 0, WINAMP_OPTIONS_EQ};
	i.dwTypeData = eqmenustring.getNonConstVal();
	InsertMenuItemW(wa2.getPopupMenu(), 8, TRUE, &i);
	wa2.adjustOptionsPopupMenu(1);
	eqremoved = 0;
}

//-----------------------------------------------------------------------------------------------
void unpopulateWindowsMenus()
{
	if (ffwindowsitempos == -1) return ;

	HMENU menuBarMenu = wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_WINDOWS);
	for(int i = GetMenuItemCount(menuBarMenu)-1; i >= 0; i--)
	{
		MENUITEMINFOW info = {sizeof(info), MIIM_DATA, 0, };
		if(GetMenuItemInfoW(menuBarMenu,i,TRUE,&info))
		{
			if(info.dwItemData == 0xD01){
				RemoveMenu(menuBarMenu,i,MF_BYPOSITION);
				wa2.adjustFFWindowsMenu(-1);
			}
		}
	}
	
	HMENU menuPopupMenu = wa2.getPopupMenu();
	for(int i = GetMenuItemCount(menuPopupMenu)-1; i >= 0; i--)
	{
		MENUITEMINFOW info = {sizeof(info), MIIM_DATA, 0, };
		if(GetMenuItemInfoW(menuPopupMenu,i,TRUE,&info))
		{
			if(info.dwItemData == 0xD01){
				RemoveMenu(menuPopupMenu,i,MF_BYPOSITION);
				wa2.adjustOptionsPopupMenu(-1);
			}
		}
	}

	ffwindowsitempos = -1;
	ffwindowsitempos2 = -1;

	MenuActions::removeSkinWindowOptions();
	MenuActions::removeSkinOptions();
}

//-----------------------------------------------------------------------------------------------
void populateWindowsMenus()
{
	if (ffwindowsitempos != -1) unpopulateWindowsMenus();
	MenuActions::installSkinOptions();
	MenuActions::installSkinWindowOptions();

	ffwindowsitempos = wa2.adjustFFWindowsMenu(0) + NUMSTATICWINDOWS;
	ffwindowsitempos2 = wa2.adjustOptionsPopupMenu(0) + 6 + NUMSTATICWINDOWS + 1;

	MENUITEMINFOW i = {sizeof(i), };
	i.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID | MIIM_STATE;
	i.fType = MFT_STRING;
	i.wID = 43000;
	i.dwItemData = 0xD01;	// use this as a check so we're only removing the correct items!!
	int pos = ffwindowsitempos;
	int pos2 = ffwindowsitempos2;
	PtrListQuickSorted<StringW, StringWComparator> items;

	HMENU hMenu = wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_WINDOWS);
	HMENU hMenu2 = wa2.getPopupMenu();
	for (int c = 0;c < SkinParser::getNumContainers();c++)
	{
		Container *cont = SkinParser::enumContainer(c);
		if (cont && wantContainerInMenu(cont))
		{
			i.dwTypeData = const_cast<wchar_t *>(cont->getName());
			if (i.dwTypeData != NULL)
			{
				if (!items.findItem(i.dwTypeData))
				{
					items.addItem(new StringW(i.dwTypeData));
					i.cch = wcslen(i.dwTypeData);
					i.fState = cont->isVisible() ? MFS_CHECKED : 0;
					InsertMenuItemW(hMenu, pos++, TRUE, &i);
					wa2.adjustFFWindowsMenu(1);
					InsertMenuItemW(hMenu2, pos2++, TRUE, &i);
					wa2.adjustOptionsPopupMenu(1);
				}
			}
		}
		i.wID++;
	}

	int n = WASABI_API_WNDMGR->autopopup_getNumGuids();
	for (int c = 0;c < n;c++)
	{
		GUID guid = WASABI_API_WNDMGR->autopopup_enumGuid(c);
		const wchar_t *groupdesc = WASABI_API_WNDMGR->autopopup_enumGuidDescription(c);
		if (guid != INVALID_GUID && groupdesc && *groupdesc)
		{
			i.dwTypeData = const_cast<wchar_t *>(groupdesc);
			if (!items.findItem(i.dwTypeData))
			{
				items.addItem(new StringW(i.dwTypeData));
				i.cch = wcslen(i.dwTypeData);
				i.fState = WASABI_API_WNDMGR->skinwnd_getNumByGuid(guid) ? MFS_CHECKED : 0;
				InsertMenuItemW(hMenu, pos++, TRUE, &i);
				wa2.adjustFFWindowsMenu(1);
				InsertMenuItemW(hMenu2, pos2++, TRUE, &i);
				wa2.adjustOptionsPopupMenu(1);
			}
		}
		i.wID++;
	}

	n = WASABI_API_WNDMGR->autopopup_getNumGroups();
	for (int c = 0;c < n;c++)
	{
		const wchar_t *id = WASABI_API_WNDMGR->autopopup_enumGroup(c);
		const wchar_t *groupdesc = WASABI_API_WNDMGR->autopopup_enumGroupDescription(c);
		if (id && groupdesc && *id && *groupdesc)
		{
			i.dwTypeData = const_cast<wchar_t *>(groupdesc);
			// allow localisation of the color editor menu item
			i.dwTypeData = const_cast<wchar_t *>(MenuActions::localizeSkinWindowName(i.dwTypeData));
			if (!items.findItem(i.dwTypeData))
			{
				items.addItem(new StringW(i.dwTypeData));
				i.cch = wcslen(i.dwTypeData);
				i.fState = WASABI_API_WNDMGR->skinwnd_getNumByGroupId(id) ? MFS_CHECKED : 0;
				InsertMenuItemW(hMenu, pos++, TRUE, &i);
				wa2.adjustFFWindowsMenu(1);
				InsertMenuItemW(hMenu2, pos2++, TRUE, &i);
				wa2.adjustOptionsPopupMenu(1);
			}
		}
		i.wID++;
	}

	items.deleteAll();
	ffwindowstop = i.wID;
}

//-----------------------------------------------------------------------------------------------
void switchSkin(const wchar_t *skinname)
{
	wa2.switchSkin(skinname);
}

//-----------------------------------------------------------------------------------------------
void addWindowOptionsToContextMenu(ifc_window *w)
{
	if (g_controlMenuTarget != NULL)
		removeWindowOptionsFromContextMenu();
	if (w == NULL && WASABI_API_WND->rootwndIsValid(lastFocused))
		w = lastFocused;
	g_controlMenuTarget = w;

	if (g_controlMenuTarget == NULL) return ;

	/*int opacitysafe = 1;
	int scalelocked = 0;

	ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
	if (rootparent && rootparent->getInterface(layoutGuid))
	{
		Layout *l = static_cast<Layout*>(rootparent);
		opacitysafe = l->isTransparencySafe();
		scalelocked = l->isScaleLocked();
	}*/

	updateControlMenu(w);
	updateAppBarMenu(w);

	HMENU menu = wa2.getPopupMenu();
	HMENU ctrlmenu = GetSubMenu(controlmenu, 0);

	// JF> Francis, made this use command IDs (inserts it right before the help ID in the main menu).
	// does this look OK? if you want, revert back to the old one. I just thought this seemed a tiny
	// bit cleaner... :)
#define ID_HELP_HELPTOPICS              40347
	InsertMenuW(menu, ID_HELP_HELPTOPICS, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)ctrlmenu, WASABI_API_LNGSTRINGW(IDS_WINDOW_SETTINGS));
	InsertMenu(menu, ID_HELP_HELPTOPICS, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);

}

//-----------------------------------------------------------------------------------------------
void removeWindowOptionsFromContextMenu()
{
	if (g_controlMenuTarget == NULL) return ;
	g_controlMenuTarget = NULL;
	HMENU menu = wa2.getPopupMenu();
	HMENU ctrlmenu = GetSubMenu(controlmenu, 0);
	if (ctrlmenu)
	{
		int l = GetMenuItemCount(menu);
		while (l-- > 0 && GetSubMenu(menu, l) != ctrlmenu);
		if (l >= 0)
		{
			RemoveMenu(menu, l, MF_BYPOSITION);
			RemoveMenu(menu, l, MF_BYPOSITION); // remove sep
		}
	}
}

//-----------------------------------------------------------------------------------------------
void controlOpacity(int v)
{
	if (!g_controlMenuTarget) return ;
	if (!WASABI_API_WND->rootwndIsValid(g_controlMenuTarget)) return ;
	v = (int)(((float)v / 100.0f) * 255.0f + 0.5f);
	ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
	if (rootparent && rootparent->getInterface(layoutGuid))
	{
		Layout *l = static_cast<Layout*>(rootparent);
		if (!cfg_uioptions_linkallalpha.getValueAsInt())
		{
			l->setAlpha(v);
		}
		else
		{
			cfg_uioptions_linkedalpha.setValueAsInt(v);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void controlScaling(double v)
{
	if (!g_controlMenuTarget) return ;
	if (!WASABI_API_WND->rootwndIsValid(g_controlMenuTarget)) return ;
	ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
	if (rootparent) rootparent->setRenderRatio(v);
}

//-----------------------------------------------------------------------------------------------
void lockScaling(int lock)
{
	ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
	if (!WASABI_API_WND->rootwndIsValid(g_controlMenuTarget)) return ;
	if (rootparent && rootparent->getInterface(layoutGuid))
	{
		Layout *l = static_cast<Layout*>(rootparent);
		l->lockScale(lock);
	}
}

//-----------------------------------------------------------------------------------------------
void controlAppBar(int side)
{
	ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
	if (rootparent)
	{
		AppBar *ab = (AppBar *)rootparent->getInterface(appBarGuid);
		if (ab)
		{
			ab->appbar_dock(side);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void controlAppBarAOT()
{
	ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
	if (rootparent)
	{
		Layout *l = (Layout *)rootparent->getInterface(layoutGuid);
		if (l)
		{
			int curaot = l->getAppBarAlwaysOnTop();
			l->setAppBarAlwaysOnTop(!curaot);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void controlAppBarAH()
{
	ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
	if (rootparent)
	{
		Layout *l = (Layout *)rootparent->getInterface(layoutGuid);
		if (l)
		{
			int curah = l->getAppBarAutoHide();
			l->setAppBarAutoHide(!curah);
		}
	}
}

//-----------------------------------------------------------------------------------------------
void updateAppBarMenu(ifc_window *w)
{
	if (g_controlMenuTarget == NULL) return ;
	ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
	if (rootparent)
	{
		Layout *l = (Layout *)rootparent->getInterface(layoutGuid);
		if (l)
		{
			HMENU ctrlmenu = GetSubMenu(controlmenu, 0);
			HMENU appbarmenu = GetSubMenu(ctrlmenu, 2);
			if (l->appbar_getEnabledSides() == 0)
			{
				EnableMenuItem(ctrlmenu, 2, MF_BYPOSITION | MF_GRAYED);
				return ;
			}
			else
				EnableMenuItem(ctrlmenu, 2, MF_BYPOSITION | MF_ENABLED);
			int docked = l->appbar_isDocked();
			int side = l->appbar_getSide();
			CheckMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_ALWAYSONTOP, MF_BYCOMMAND | (l->appbar_wantAlwaysOnTop() ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_AUTOHIDE, MF_BYCOMMAND | (l->appbar_wantAutoHide() ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_LEFT, MF_BYCOMMAND | ((docked && side == APPBAR_LEFT) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_TOP, MF_BYCOMMAND | ((docked && side == APPBAR_TOP) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_RIGHT, MF_BYCOMMAND | ((docked && side == APPBAR_RIGHT) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_BOTTOM, MF_BYCOMMAND | ((docked && side == APPBAR_BOTTOM) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_DISABLED, MF_BYCOMMAND | (!docked ? MF_CHECKED : MF_UNCHECKED));
			EnableMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_AUTOHIDE, MF_BYCOMMAND | ((!docked || l->appbar_isSideAutoHideSafe(side)) ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_LEFT, MF_BYCOMMAND | (l->appbar_isSideEnabled(APPBAR_LEFT) ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_TOP, MF_BYCOMMAND | (l->appbar_isSideEnabled(APPBAR_TOP) ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_RIGHT, MF_BYCOMMAND | (l->appbar_isSideEnabled(APPBAR_RIGHT) ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_BOTTOM, MF_BYCOMMAND | (l->appbar_isSideEnabled(APPBAR_BOTTOM) ? MF_ENABLED : MF_GRAYED));
			CheckMenuItem(appbarmenu, ID_CONTROLMENU_TOOLBAR_AUTODOCKONDRAG, MF_BYCOMMAND | (cfg_options_appbarondrag ? MF_CHECKED : MF_UNCHECKED));
		}
	}
}

//-----------------------------------------------------------------------------------------------
double onTweakRenderRatio(double v)
{
	if (!cfg_uioptions_uselocks.getValueAsInt())
		return v;
	return wa2.isDoubleSize() ? 2.0 : 1.0;
}

//-----------------------------------------------------------------------------------------------
void onCustomAltF4()
{
	SendMessageW(wa2.getMainWindow(), WM_CLOSE, 0, 0);
}

int isSkinStillLoading()
{
	return before_startup_callback;
}

//-----------------------------------------------------------------------------------------------
void loadExtraColorThemes()
{
	wchar_t filename[WA_MAX_PATH] = {0};
	GetModuleFileNameW(hInstance, filename, WA_MAX_PATH);
	PathParserW pp(filename);
	StringW path;
	for (int i = 0;i < pp.getNumStrings() - 1;i++)
	{
		path.AppendPath(pp.enumString(i));
	}
	StringW file = path;
	file.AppendPath(L"ColorThemes");
	file.AppendPath(WASABI_API_SKIN->getSkinName());
	file.AppendPath(L"*.xml");
	WASABI_API_SKIN->loadSkinFile(file);
}

double oldscale = 1.;

//-----------------------------------------------------------------------------------------------
static BOOL CALLBACK customScaleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		//CUT: double curratio=1.;
		ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
		if (!rootparent || !WASABI_API_WND->rootwndIsValid(rootparent))
		{
			g_controlMenuTarget = NULL; return TRUE;
		}

		oldscale = rootparent->getRenderRatio();

		int u = (int)((oldscale * 100.0f) + 0.5f);

		SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETRANGEMAX, 0, 300);
		SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETRANGEMIN, 0, 10);
		SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETPOS, 1, u);
		SetDlgItemTextW(hwndDlg, IDC_STATIC_SCALE, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_SCALE_X_PERCENT), u));
		return TRUE;
	}
	case WM_HSCROLL:
	{
		int t = (int)SendMessageW((HWND) lParam, TBM_GETPOS, 0, 0);
		if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE))
		{
			SetDlgItemTextW(hwndDlg, IDC_STATIC_SCALE, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_SCALE_X_PERCENT), t));
			controlScaling((double)t / 100.0);
			if (g_controlMenuTarget)
			{
				ifc_window *w = g_controlMenuTarget->getDesktopParent();
				UpdateWindow(w->gethWnd());
			}
		}
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, IDOK);
			return 0;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			return 0;
		}
		break;
	}
	return FALSE;
}

double oldalpha = 255.;

//-----------------------------------------------------------------------------------------------
static BOOL CALLBACK customAlphaProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		ifc_window *rootparent = g_controlMenuTarget->getDesktopParent();
		if (!rootparent || !WASABI_API_WND->rootwndIsValid(rootparent))
		{
			g_controlMenuTarget = NULL; return TRUE;
		}

		int v = 100;
		if (!cfg_uioptions_linkallalpha.getValueAsInt())
		{
			Layout *l = static_cast<Layout *>(rootparent);
			if (l != NULL)
				oldalpha = static_cast<double>(l->getAlpha());
		}
		else
		{
			oldalpha = static_cast<double>(cfg_uioptions_linkedalpha.getValueAsInt());
		}
		v = (int)((oldalpha / 255.0f * 100.0f) + 0.5f);
		SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMALPHA), TBM_SETRANGEMAX, 0, 100);
		SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMALPHA), TBM_SETRANGEMIN, 0, 10);
		SendMessageW(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMALPHA), TBM_SETPOS, 1, v);
		SetDlgItemTextW(hwndDlg, IDC_STATIC_ALPHA, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_OPACITY_X_PERCENT), v));
		return TRUE;
	}
	case WM_HSCROLL:
	{
		int t = (int)SendMessageW((HWND) lParam, TBM_GETPOS, 0, 0);
		if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMALPHA))
		{
			SetDlgItemTextW(hwndDlg, IDC_STATIC_ALPHA, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_OPACITY_X_PERCENT), t));
			controlOpacity(t);
		}
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, IDOK);
			return 0;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			return 0;
		}
		break;
	}
	return FALSE;
}

void customScaling()
{
	if (!g_controlMenuTarget) return ;
	if (!WASABI_API_WND->rootwndIsValid(g_controlMenuTarget)) return ;
	ifc_window *w = g_controlMenuTarget->getDesktopParent();
	if (!w || !WASABI_API_WND->rootwndIsValid(w)) return ;
	int r = WASABI_API_DIALOGBOXW(IDD_CUSTOMSCALE, w->gethWnd(), customScaleProc);
	if (r != IDOK)
		controlScaling(oldscale);
}

void customOpacity()
{
	if (!g_controlMenuTarget) return ;
	if (!WASABI_API_WND->rootwndIsValid(g_controlMenuTarget)) return ;
	ifc_window *w = g_controlMenuTarget->getDesktopParent();
	if (!w || !WASABI_API_WND->rootwndIsValid(w)) return ;
	int r = WASABI_API_DIALOGBOXW(IDD_CUSTOMALPHA, w->gethWnd(), customAlphaProc);
	if (r != IDOK)
		controlOpacity((int)((oldalpha / 255.0f * 100.0f) + 0.5f));
}

void autoOpacifyHover()
{
	if (!g_controlMenuTarget) return ;
	if (!WASABI_API_WND->rootwndIsValid(g_controlMenuTarget)) return ;
	ifc_window *w = g_controlMenuTarget->getDesktopParent();
	Layout *l = static_cast<Layout*>(w->getInterface(layoutGuid));
	if (l)
	{
		if (!cfg_uioptions_linkallalpha.getValueAsInt())
		{
			int a = l->getAutoOpacify();
			if (a == 2) a = 1;
			else a = !a;
			l->setAutoOpacify(a);
		}
		else
		{
			int a = cfg_uioptions_autoopacitylinked.getValueAsInt();
			if (a == 2) a = 1;
			else a = !a;
			cfg_uioptions_autoopacitylinked.setValueAsInt(a);
		}
	}
}

void autoOpacifyFocus()
{
	if (!g_controlMenuTarget) return ;
	if (!WASABI_API_WND->rootwndIsValid(g_controlMenuTarget)) return ;
	ifc_window *w = g_controlMenuTarget->getDesktopParent();
	Layout *l = static_cast<Layout*>(w->getInterface(layoutGuid));
	if (l)
	{
		if (!cfg_uioptions_linkallalpha.getValueAsInt())
		{
			int a = l->getAutoOpacify();
			if (a == 1) a = 2;
			else if (a == 0) a = 2;
			else if (a == 2) a = 0;
			l->setAutoOpacify(a);
		}
		else
		{
			int a = cfg_uioptions_autoopacitylinked.getValueAsInt();
			if (a == 1) a = 2;
			else if (a == 0) a = 2;
			else if (a == 2) a = 0;
			cfg_uioptions_autoopacitylinked.setValueAsInt(a);
		}
	}
}

StringW langpackfilename;

const wchar_t *localesCustomGetFile()
{
	const wchar_t *langDir = WASABI_API_LNG->GetLanguageFolder();
	if (!langDir || !*langDir)
		return NULL;

	langpackfilename = StringPathCombine(langDir, L"freeform");
	return langpackfilename;
#if 0 // old code
	wchar_t buf[256] = L"";
	GetPrivateProfileStringW(L"Winamp", L"langpack", L"", buf, 256, AutoWide(INI_FILE));  // TODO: maybe we should change all ini file stuff to W versions
	if (*buf == 0)
		return NULL;
	wchar_t *p = wcschr(buf, '.');
	if (p)
		*p = 0;

	wchar_t filename[WA_MAX_PATH] = {0};
	GetModuleFileNameW(hInstance, filename, WA_MAX_PATH);
	PathParserW pp(filename);
	langpackfilename = L"";

	for (int i = 0;i < pp.getNumStrings() - 1;i++)
	{
		langpackfilename.AppendPath(pp.enumString(i));
	}
	langpackfilename.AppendPath(L"freeform");
	langpackfilename.AppendPath(L"langpacks");
	langpackfilename.AppendPath(StringPrintfW(L"%s.xml", buf));

	return langpackfilename;
#endif
}

const wchar_t *getCustomVar(const wchar_t *var)
{
	static StringW ret;
	if (WCSCASEEQLSAFE(var, L"@HAVE_LIBRARY@"))
	{
		ret = StringPrintfW(L"%d", we_have_ml);
		return ret;
	}
	return NULL;
}

void checkMlPresent()
{
	wchar_t filename[WA_MAX_PATH] = {0};
	GetModuleFileNameW(hInstance, filename, WA_MAX_PATH);
	PathParserW pp(filename);
	StringW path;
	for (int i = 0;i < pp.getNumStrings() - 1;i++)
	{
		path.AppendPath(pp.enumString(i));
	}
	path.AppendPath(L"gen_ml.dll");
	we_have_ml = !WACCESS(path, 0);
}

void initEgg()
{
	eggstr[0] = ~'N';
	eggstr[1] = ~'U';
	eggstr[2] = ~'L';
	eggstr[3] = ~'L';
	eggstr[4] = ~'S';
	eggstr[5] = ~'O';
	eggstr[6] = ~'F';
	eggstr[7] = ~'T';
	eggstr[8] = 0;

	{
		int x;
		for (x = 0; x < 8; x ++) eggstr[x] ^= 255;
	}
}

void toggleEgg()
{
	eggstat = !eggstat;
	if (!eggstat)
	{
		KillTimer(wa2.getMainWindow(), UPDATE_EGG);
		if (lastlayoutegg && WASABI_API_WND->rootwndIsValid(lastlayoutegg)) lastlayoutegg->setTransparencyOverride(-1);
		lastlayoutegg = NULL;
	}
	else SetTimer(wa2.getMainWindow(), UPDATE_EGG, 25, NULL);
}

void getCustomMetaData(const wchar_t *field, wchar_t *buf, int len)
{
	StringW curfile = WASABI_API_MEDIACORE->core_getCurrent(0);
	if (curfile.isempty())
	{
		buf[0] = 0;
		return ;
	}
	if (!_wcsnicmp(curfile, L"file://", 7))
		curfile = StringW(curfile.getValue() + 7);
	buf[0] = 0;

	if (WCSCASEEQLSAFE(field, L"filename"))
	{
		WCSCPYN(buf, curfile, len);
	}
	else
	{
		wa2.getMetaData(curfile, field, buf, len);
	}
}

void registerGlobalHotkeys()
{
	static int registered = 0;
	if (!registered)
	{
		static wchar_t ghkStr[96];
		wa2.registerGlobalHotkey((char*)WASABI_API_LNGSTRINGW_BUF(IDS_GHK_SHOW_NOTIFICATION,ghkStr,96), WM_WA_IPC, 0, IPC_SHOW_NOTIFICATION, HKF_UNICODE_NAME, "genff shn");
		registered = 1;
	}
}

const wchar_t *getSongInfoText()
{
	return Core::getSongInfoText();
}

const wchar_t *getSongInfoTextTranslated()
{
	return Core::getSongInfoTextTranslated();
}

OSWINDOWHANDLE getKeyboardForwardWnd(GUID g)
{
	if (g != INVALID_GUID)
	{
		if (g == playerWndGuid)
			return wa2.getMainWindow();
		else if (g == pleditWndGuid)
			return wa2.getWnd(IPC_GETWND_PE);
		else if (g == videoWndGuid)
			return wa2.getWnd(IPC_GETWND_VIDEO);
		else
		{
			embedWindowState *ews = embedWndGuidMgr.getEmbedWindowState(g);
			if (ews && wa2.isValidEmbedWndState(ews)) return ews->me;
		}
	}
	return WASABI_API_WND->main_getRootWnd()->gethWnd();
}

void onAppBarDockChanged(ifc_window *w)
{
	setDialogBoxesParent();
}

void onMainLayoutMove(HWND w)
{
	// for Winamp to appear on the correct taskbar on Windows 8
	// its necessary to set the classic main window to appear on
	// that monitor and to be on-screen (but hidden) otherwise
	// Windows will ignore it and it then makes us look buggy.
	RECT r;
	Wasabi::Std::getViewport(&r, w, 1);
	SetWindowPos(wa2.getMainWindow(), NULL, r.left, r.bottom - 1, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
}

void updateAll()
{
	updatePl();
#ifdef MINIBROWSER_SUPPORT
	updateMb();
#endif
	updateVid();
	int n = embedWndGuidMgr.getNumWindowStates();
	for (int i = 0;i < n;i++)
	{
		embedWindowState *ws = NULL;
		GUID g = embedWndGuidMgr.enumWindowState(i, &ws);
		if (g != INVALID_GUID)
		{
			updateEmb(g, ws);
		}
	}
}

void onReParent(HWND wnd)
{
	updateAll();
}

void onReInit(HWND wnd)
{
	updateAll();
}

void startFSMonitor()
{
	g_fsmonitor = new FullScreenMonitor();
	g_fscallback = new Wa5FSCallback();
	g_fsmonitor->registerCallback(g_fscallback);
}

void stopFSMonitor()
{
	delete g_fsmonitor;
	g_fsmonitor = NULL;
	delete g_fscallback;
	g_fscallback = NULL;
}

void updateParentlessOnTop()
{
	int i;
	for (i = 0;i < SkinParser::getNumContainers();i++)
	{
		Container *c = SkinParser::enumContainer(i);
		if (c != NULL)
		{
			int j;
			for (j = 0;j < c->getNumLayouts();j++)
			{
				Layout *l = c->enumLayout(j);
				if (l != NULL)
				{
					// skip windows owned by winamp
					// skip appbars, they take care of themselves
					if (l->getNoParent() && !l->appbar_isDocked())
					{
						l->updateOnTop();
					}
				}
			}
		}
	}
}

void onGoFullscreen()
{
	// hidden windows will not receive APPBAR_CALLBACK, so forward it to winamp's main
	SendMessageW(wa2.getMainWindow(), APPBAR_CALLBACK, ABN_FULLSCREENAPP, 1);
	// update ontop flag for windows that are not parented to winamp
	updateParentlessOnTop();
}

void onCancelFullscreen()
{
	// hidden windows will not receive APPBAR_CALLBACK, so forward it to winamp's main
	SendMessageW(wa2.getMainWindow(), APPBAR_CALLBACK, ABN_FULLSCREENAPP, 0);
	// update ontop flag for windows that are not owned by winamp
	updateParentlessOnTop();
}

int processGenericHotkey(const char *hk)
{
	if (!m_are_we_loaded) return 0;

	SystemObject::onKeyDown(AutoWide(StringPrintf("HOTKEY: %s", hk)));
	if (VCPU::getComplete())
	{
		DebugStringW(L"HOTKEY: %s trapped by script\n", hk);
		return 1;
	}
	return 0;
}

int canExitWinamp()
{
	return wa2.isExitEnabled();
}

int fsMonitorIsFS()
{
	return g_fsmonitor->isFullScreen();
}

void modalPush()
{
	wa2.pushExitDisabled();
}

void modalPop()
{
	wa2.popExitDisabled();
}