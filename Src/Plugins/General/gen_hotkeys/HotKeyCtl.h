#ifndef ____HOTKEY_CTL___H____
#define ____HOTKEY_CTL___H____

#include <windows.h>

#define HOTKEYF_WIN 0x10

int SubclassEditBox(HWND hwEdit);

extern UINT wmHKCtlSet;
extern UINT wmHKCtlGet;

#endif//____HOTKEY_CTL___H____