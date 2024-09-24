#ifndef _MAIN_H
#define _MAIN_H

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "../winamp/wa_dlg.h"
#include "../winamp/wa_ipc.h"


#include "./itemlist.h"
#include "./config.h"
#include "../winamp/gen.h"
#include "../Agave/Config/ifc_configitem.h"

#define PLUGIN_VERSION		0x0378

#include "./ml.h"
#include "./skinning.h"
#include "../nu/trace.h"

#define WA_MENUITEM_ID 23123
#define WINAMP_VIDEO_TVBUTTON           40338 // we hook this =)
#define WINAMP_LIGHTNING_CLICK          40339 // this three
#define WINAMP_NEXT_WINDOW              40063
#define WINAMP_SHOWLIBRARY              40379
#define WINAMP_CLOSELIBRARY             40380


#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

BOOL FlickerFixWindow(HWND hwnd, INT mode);

extern "C" winampGeneralPurposePlugin plugin;
extern "C" int getFileInfo(const char *filename, const char *metadata, char *dest, int len);

extern LARGE_INTEGER freq;
extern int profile;
extern HCURSOR hDragNDropCursor;
extern wchar_t WINAMP_INI[MAX_PATH], WINAMP_INI_DIR[MAX_PATH];
extern wchar_t pluginPath[];
extern C_Config *g_config;

extern HMENU g_context_menus;
extern HWND g_PEWindow, g_hwnd, prefsWnd;
extern int g_safeMode, sneak;
extern int config_use_ff_scrollbars;
extern int config_use_alternate_colors;
extern C_Config *g_view_metaconf;

void main_playItemRecordList (itemRecordList *obj, int enqueue, int startplaying=1);
void main_playItemRecordListW(itemRecordListW *obj, int enqueue, int startplaying=1);

int handleDragDropMove(HWND hwndDlg, int type, POINT p, int do_cursors);
void MLVisibleChanged(BOOL fVisible);

INT MediaLibrary_TrackPopupEx(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, HMLIMGLST hmlil, 
							INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);

INT MediaLibrary_TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd);

HANDLE MediaLibrary_InitSkinnedPopupHook(HWND hwnd, HMLIMGLST hmlil, INT width, UINT skinStyle, 
										MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);

BOOL 
MediaLibrary_OpenUrl(HWND ownerWindow, const wchar_t *url, BOOL forceExternal);

BOOL
MediaLibrary_OpenHelpUrl(const wchar_t *helpUrl);

/*
//gracenote.cpp
void gracenoteInit();
int gracenoteQueryFile(const char *filename);
void gracenoteCancelRequest();
int gracenoteDoTimerStuff();
void gracenoteSetValues(char *artist, char *album, char *title);
char *gracenoteGetTuid();
int gracenoteIsWorking();
*/
//listheader.cpp
INT_PTR handleListViewHeaderMsgs(HWND hwndDlg,
							 HWND headerWnd,
							 HWND listWnd,
							 UINT uMsg,
							 WPARAM wParam,
							 LPARAM lParam,
							 BOOL sortShow,
							 BOOL sortAscending,
							 int sortIndex);

INT_PTR handleListViewHeaderPaintMsgs(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//plugin.cpp
void loadMlPlugins();
void unloadMlPlugins();
INT_PTR pluginHandleIpcMessage(HWND hwndML, int msg, INT_PTR param);
INT_PTR plugin_SendMessage(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);
INT_PTR CALLBACK PluginsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);


//prefs.cpp
INT_PTR CALLBACK PrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

void refreshPrefs(INT_PTR screen);
void openPrefs(INT_PTR screen);	//-1 for default

void FixAmps(char *str, size_t len);
LPWSTR FixAmpsW(LPWSTR pszText, INT cchMaxText);

//view_devices.cpp
INT_PTR CALLBACK view_devicesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
// webinfo_dlg
HWND CreateWebInfoWindow(HWND hwndParent, UINT uMsgQuery, INT x, INT y, INT cx, INT cy, INT ctrlId);

extern int winampVersion;

void toggleVisible(int closecb = 0);
bool IsVisible();
void myOpenURLWithFallback(HWND hwnd, wchar_t *loc, wchar_t *fallbackLoc);

bool IsVista();

void SkinnedScrollWnd_Init();
void SkinnedScrollWnd_Quit();

class ifc_configitem;

extern ifc_configitem *ieDisableSEH;

#endif 