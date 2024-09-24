#ifndef ___HOTAMP3_H___
#define ___HOTAMP3_H___

#include <windows.h>
#include <commctrl.h>
#include "winamp.h"
#include "wa_hotkeys.h"
#include "resource.h"
#include "hotkey.h"
#include "WACommands.h"
#include "HotKeyCtl.h"
#include "../winamp/gen.h"
#include "api__gen_hotkeys.h"
#include <strsafe.h>

///////////////////////////////////////////////////////////
//  Hot keys initializer function
///////////////////////////////////////////////////////////
void hotkeysInit();
int hotkeysLoad(HOTKEY_DATA *hkds, DWORD num, int do_register, int verbose=1);
void hotkeysSave(HOTKEY_DATA *hkds, DWORD num);
void hotkeysClear();

///////////////////////////////////////////////////////////
//  Plugin function headers
///////////////////////////////////////////////////////////
int pluginInit();
void pluginConfig();
void pluginQuit();

///////////////////////////////////////////////////////////
//  DLL Windows message handling procedure header
///////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////////
//  Globals
///////////////////////////////////////////////////////////
extern winampGeneralPurposePlugin psPlugin;
extern HOTKEY *g_hotkeys;
extern DWORD g_dwHotkeys;
extern wchar_t *g_iniFile;
extern HOTKEY_DATA g_defhkds[];
#define DEFHKDS_NUM 15

void writePrivateProfileInt(wchar_t *section, int val);

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#endif