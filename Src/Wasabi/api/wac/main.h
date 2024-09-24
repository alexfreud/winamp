// THIS FILE SHOULD FUCKING DISAPPEAR DAMMIT

//NONPORTABLE	-- HWND and HINSTANCE references
#ifndef _MAIN_H
#define _MAIN_H

#warning don't include me

#include <bfc/std.h>
#include <bfc/string/string.h>
#include <bfc/ptrlist.h>

#define WM_SYSTRAY WM_USER+1102 //FG> Arbitrary value. Would probably be better with a registered message
#define WM_SETSKIN WM_USER+0x1000

class Systray;
class MetricsCallback;
class ScriptObjectManager;
class CfgItem;
class api_window;
class GenWnd;
class WasabiKernelController;
class CoreHandle;
class CompCommandEntry;
class Layout;

class Main {
public:
  friend class WasabiKernel;
  static HINSTANCE gethInstance();
  static HWND gethWnd();
  static DWORD getThreadId();
  static WasabiKernelController *getKernelController();
  static int isMaximized();
  static int isMinimized();
  static int minimizeWnd();
  static int restoreWnd();
  static int invalidate();

  static void outputDebugString(int severity, const char *string);
  static void outputDebugString(const char *string) {
    outputDebugString(0, string);
  }

  // sets the ownerwnd title
  static void setWindowTitle(const wchar_t *text);

  // status
  static void setOverlayText(const wchar_t *text, int interval);//displays then reverts
  static void setTrayTipText(const wchar_t *text); // set systrem tray icon tooltip text

  // ontop status
  static void setOnTop(BOOL set);
  static BOOL getOnTop();

  // systray
  // 0 = nothing
  // 1 = taskbar
  // 2 = systray
  // 3 = both :)
  static void setIconMode(int mode);

  // skin
  static void setSkinDelayed(const wchar_t *skinName);

  // path to wasabi.dll
  static const wchar_t *getWasabiPath();
  // path to main EXE
  static const wchar_t *getMainAppPath();

  static GUID getGuid();

  static void shutdown();
  static void cancelShutdown();

  static void savePosition();

  static void navigateUrl(const wchar_t *url); // displays in minibrowser if present, otherwise launch external

  static int appContextMenu(api_window *parent, BOOL canScale, BOOL canAlpha);
  static int thingerContextMenu(api_window *parent);
  
  static void doAction(int action, int param=0);
  static void doMenu(const wchar_t *which);

  static void processCommandLine(const wchar_t *cmdLine);

  static void setSkinsPath(const wchar_t *path);
  static const wchar_t *getSkinsPath();

  static GenWnd *getGenericWnd();
  static int isShutingDown();

  static HICON smallicon;
  static HICON bigicon;
  static bool ontop;
  static Systray *systray;
  static GenWnd *genericwnd;
  static int shuting_down;

  // maintains a stack of modal windows so basewnds can discard messages in WM_MOUSEACTIVATE, should only be used by MsgboxWnd & ModalWnd when we write it ;)
  static api_window *getModalWnd();
  static void pushModalWnd(api_window *wnd);
  static int popModalWnd(api_window *wnd);

  static void metrics_addCallback(MetricsCallback *);
  static void metrics_delCallback(MetricsCallback *);
  static int metrics_getDelta();
  static int metrics_setDelta(int newdelta);

  static int isInetAvailable(); //return 1 if connected, 0 if not available

  static String lastwindowtitle;
  static int taskbaractive;

  static CoreHandle *getMainCoreHandle();
  static CoreHandle *mainCoreHandle;

  static const wchar_t *getCommandLine();
  static String commandLine;
  static String skinspath;

  static int revert_on_error;
  static int cancel_shutdown;
  static PtrList<api_window> ontoplist;
  static void saveTopMosts();
  static void restoreTopMosts();
  static int onMouseWheel(int l, int a);

private:
  static int isRASActive();
};

#endif
