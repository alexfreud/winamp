#include <precomp.h>
#include "keyboard.h"
#include <api/locales/localesmgr.h>
//#include <api/wac/main.h> // CUT !!!
#include <api/script/objects/systemobj.h>
#include <api/wnd/wndtrack.h>

#ifdef WASABI_COMPILE_SCRIPT
#include <api/script/vcpu.h>
#endif

#if !defined(WIN32) && !defined(LINUX)
#error port me
#endif

Keyboard::vkEntry Keyboard::vkEntries[]={
#ifdef WIN32
  //1, "lbutton", // fg> we don't want mouse messages in keyboard events, no.
  //2, "rbutton",
  3, L"cancel",
  //4, L"mbutton",
  8, L"backspace",
  9, L"tab",
  0xc, L"clear",
  0xd, L"return",
  0x10, L"shift",
  0x11, L"ctrl",
  0x12, L"alt",
  0x13, L"pause",
  0x14, L"capslock",
  0x1b, L"esc",
  0x20, L"space",
  0x21, L"pgup",
  0x22, L"pgdn",
  0x23, L"end",
  0x24, L"home",
  0x25, L"left",
  0x26, L"up",
  0x27, L"right",
  0x28, L"down",
  0x29, L"select",
  0x2b, L"execute",
  0x2c, L"prtscr",
  0x2d, L"insert",
  0x2e, L"del",
  0x2f, L"help",
  0x5b, L"lwin",
  0x5c, L"rwin",
  0x5d, L"mwin",
  0x60, L"n0",
  0x61, L"n1",
  0x62, L"n2",
  0x63, L"n3",
  0x64, L"n4",
  0x65, L"n5",
  0x66, L"n6",
  0x67, L"n7",
  0x68, L"n8",
  0x69, L"n9",
  0x6a, L"numpad_multiply",
  0x6b, L"numpad_add",
  0x6c, L"separator",
  0x6d, L"numpad_substract",
  0x6e, L"numpad_point",
  0x6f, L"numpad_divide",
  0x70, L"f1",
  0x71, L"f2",
  0x72, L"f3",
  0x73, L"f4",
  0x74, L"f5",
  0x75, L"f6",
  0x76, L"f7",
  0x77, L"f8",
  0x78, L"f9",
  0x79, L"f10",
  0x7a, L"f11",
  0x7b, L"f12",
  0x90, L"numlock",
  0x91, L"scroll",
  0xbb, L"equal",
  0xbd, L"minus",
  0xbf, L"slash",
//  0xdb, L"minus",	// seems to be the french code? --BU
  0xf6, L"attn",
  0xfe, L"clear",
#endif
#ifdef LINUX
  {XK_space, L"space"},
  {XK_Cancel, L"cancel"},
  {XK_BackSpace, L"backspace"},  
  {XK_Tab, L"tab"},
  {XK_Clear, L"clear"},
  {XK_Return, L"return"},
  {XK_Shift_L, L"shift"},
  {XK_Shift_R, L"shift"},
  {XK_Control_L, L"ctrl"},
  {XK_Control_R, L"ctrl"},
  {XK_Alt_L, L"alt"},
  {XK_Alt_R, L"alt"},
  {XK_Pause, L"pause"},
  {XK_Caps_Lock, L"capslock"},
  {XK_Escape, L"esc"},
  {XK_Page_Up, L"pgup"},
  {XK_KP_Page_Up, L"pgup"},
  {XK_Page_Down, L"pgdn"},
  {XK_KP_Page_Down, L"pgdn"},
  {XK_End, L"end"},
  {XK_KP_End, L"end"},
  {XK_Home, L"home"},
  {XK_KP_Home, L"home"},
  {XK_Left, L"left"},
  {XK_KP_Left, L"left"},
  {XK_Up, L"up"},
  {XK_KP_Up, L"up"},
  {XK_Right, L"right"},
  {XK_KP_Right, L"right"},
  {XK_Down, L"down"},
  {XK_KP_Down, L"down"},
  {XK_Select, L"select"},
  {XK_Execute, L"execute"},
  {XK_Print, L"prtscr"},
  {XK_Insert, L"insert"},
  {XK_KP_Insert, L"insert"},
  {XK_Delete, L"del"},
  {XK_KP_Delete, L"del"},
  {XK_Help, L"help"},
  {XK_KP_0, L"n0"},
  {XK_KP_1, L"n1"},
  {XK_KP_2, L"n2"},
  {XK_KP_3, L"n3"},
  {XK_KP_4, L"n4"},
  {XK_KP_5, L"n5"},
  {XK_KP_6, L"n6"},
  {XK_KP_7, L"n7"},
  {XK_KP_8, L"n8"},
  {XK_KP_9, L"n9"},
  {XK_KP_Multiply, L"numpad_multiply"},
  {XK_KP_Add, L"numpad_add"},
  {XK_KP_Separator, L"separator"},
  {XK_KP_Subtract, L"numpad_substract"},
  {XK_KP_Decimal, L"numpad_point"},
  {XK_KP_Divide, L"numpad_divide"},
  {XK_F1, L"f1"},
  {XK_F2, L"f2"},
  {XK_F3, L"f3"},
  {XK_F4, L"f4"},
  {XK_F5, L"f5"},
  {XK_F6, L"f6"},
  {XK_F7, L"f7"},
  {XK_F8, L"f8"},
  {XK_F9, L"f9"},
  {XK_F10, L"f10"},
  {XK_F11, L"f11"},
  {XK_F12, L"f12"},
  {XK_Scroll_Lock, L"scroll"},
#ifdef XK_3270
  {XK_3270_Attn, L"attn"}, // I don't know what this is...
#endif
  {XK_Clear, L"clear"},
#endif
};
//PORTME

wchar_t *Keyboard::getVkName(int vkey) 
{
  if(vkey>=0x41 && vkey<=0x5A) { // letters
    static wchar_t key[2];
    key[0]=vkey+0x20;
    key[1]=0;
    return key;
  }
  if(vkey>=0x30 && vkey<=0x39) { // numbers
    static wchar_t key[2];
    key[0]=vkey;
    key[1]=0;
    return key;
  }
  for(int i=0;i<(sizeof(vkEntries)/sizeof(vkEntry));i++) {
    if(vkEntries[i].vk==vkey) return vkEntries[i].trans;
  }
#ifdef _DEBUG
  //DebugString("undefined vk key pressed! (0x%x) :(\n",vkey);
#endif
  return NULL;
}


int Keyboard::forwardKbdMessage(ifc_window *from, int msg, int wp, int lp) 
{
  OSWINDOWHANDLE wnd_to = WASABI_API_WND->main_getRootWnd()->gethWnd();
  // note to self:
  // this is necessary for winamp2's TranslateAccelerator call to work, it seems that this function will
  // use the mouse capture wnd to determine the keyboard focus, oh thank you microsoft, because of you
  // a script has to use "complete;" to be able to detect shift+ctrl+alt + click on a toggle button, 
  // otherwise pressing a key will throw the capture away, and the button will think the mouse is gone.
  // this means that we can't be stealth doing that, we have to prevent anybody else getting shift+ctrl+alt
  // isn't that nice ?
  // we still avoid doing that if a mouse button is down, this will allow key+drags with capture
  #ifdef WIN32
  if (!(GetAsyncKeyState(VK_LBUTTON)&(1 << 31)) && !(GetAsyncKeyState(VK_RBUTTON)&(1 << 31)) && !(GetAsyncKeyState(VK_MBUTTON)&(1 << 31))) {
    SetCapture(NULL);
  }

   
  #endif

  #ifdef GET_KBDFORWARD_WND
  ifc_window *dp = from->getDesktopParent();
  if (dp) {
    Layout *l = static_cast<Layout *>(dp->getInterface(layoutGuid));
    if (l) {
      Container *c = l->getParentContainer();
      if (c) {
        GUID g = c->getDefaultContent();
        GET_KBDFORWARD_WND(g, wnd_to);
      }
    }
  }
  #endif

  if (infw) return 0;
 // if (from && from->gethWnd() == wnd_to) return 1;
  infw = 1;
 
 /* MSG winmsg;
  winmsg.message = msg;
  winmsg.hwnd = from->gethWnd();
  winmsg.wParam = wp;
  winmsg.lParam = lp;*/

  int r = 0;
 // int r = WASABI_API_APP->app_translateAccelerators(&winmsg);
  infw = 0;

  return r;
}

int Keyboard::onForwardOnChar(ifc_window *from, unsigned int c, int kd) 
{
  if (WASABI_API_WND->isKeyboardLocked()) return 1;
  return forwardKbdMessage(from, WM_CHAR, (WPARAM)c, (LPARAM)kd);
}

int MEMCMPC(void *m, char c, int size) {
  char *p = (char*)m;
  for (int i=0;i<size;i++) {
    if (*p != c) return 1;
  }
  return 0;
}

int Keyboard::onForwardOnKeyDown(ifc_window *from, int k, int kd, int nomsg)
{
  if (WASABI_API_WND->isKeyboardLocked()) return 1;
  if (infw) return 0;
  if (k >= MAX_KEY) return 0;
  lastwasreset = 0;
  pressedKeys[k]=1;
  syncKeyTable();
  wchar_t s[64]={0,};
  int first=1;
#ifdef LINUX
  for (int i=MAX_KEY-1; i >= 0; i--) {
#else
  for (int i=0;i<MAX_KEY;i++) {
#endif
    if (pressedKeys[i]) {
      wchar_t *n = getVkName(i);
      if (n) {
        if (!first) wcscat(s, L"+");
        else first=0;
        wcscat(s,n);
      }
    }
  }
  ifc_window *wnd = from;
  if(s[0]) {
#ifdef _DEBUG
    DebugString("keyboard: key pressed: %s\n",s);
#endif
#ifdef WASABI_COMPILE_LOCALES
    const wchar_t *action;
#endif
    int found=0;
    while(wnd!=NULL) {
      for(int i=0;i<accSecEntries.getNumItems();i++) {
        AccSec *ase = accSecEntries[i];
        if(ase->global || ase->wnd==wnd) {
#ifdef WASABI_COMPILE_LOCALES
          if (action=LocalesManager::translateAccelerator(ase->name, s)) 
					{
            if(ase->wnd==wnd) found = 1;
            wnd->onAcceleratorEvent(action);
#ifdef _DEBUG
            DebugString("keyboard: accelerator found\n");
#endif
            continue;
          }
#else
        wnd->onAcceleratorEvent(s);
#endif
        }
      }
      wnd=wnd->getParent();
    }
    if (found) return 1;
	
	if (NULL != from)
	{
		const wchar_t *accelSec = from->getId();
		if (accelSec && *accelSec)
		{
			#ifdef WASABI_COMPILE_LOCALES
			if(action=LocalesManager::translateAccelerator(accelSec, s)) 
			{
				int r = 0;
				#ifdef WASABI_COMPILE_SCRIPT
				r = SystemObject::onAccelerator(action, accelSec, s);
				#endif
				#ifdef WASABI_COMPILE_ACTIONS
				if (r == 0)
				{
					int act=SkinParser::getAction(action);
					if(act) Main::doAction(act);
				}
				#endif
				#ifdef _DEBUG
				DebugString("keyboard: accelerator found\n");
				#endif
				return 1;
			}
			#endif
		}

	}
#ifdef WASABI_COMPILE_LOCALES
		if(action=LocalesManager::translateAccelerator(L"general", s)) 
		{
      int r = 0;
#ifdef WASABI_COMPILE_SCRIPT
			r = SystemObject::onAccelerator(action, L"general", s);
#endif
#ifdef WASABI_COMPILE_ACTIONS
      if (r == 0) {
        int act=SkinParser::getAction(action);
        if(act) Main::doAction(act);
      }
#endif
#ifdef _DEBUG
      DebugString("keyboard: accelerator found\n");
#endif
      return 1;
    }
#endif
#ifdef _DEBUG
    DebugString("keyboard: accelerator not found\n");
#endif
#ifdef WASABI_COMPILE_SCRIPT
    DebugStringW(L"keyboard: sending \"%s\" to script\n", s);
    SystemObject::onKeyDown(s);
    if (VCPU::getComplete()) {
      DebugStringW(L"keyboard: %s trapped by script\n", s);
      return 1;
    }
#endif
  if (pressedKeys[VK_CONTROL] && pressedKeys[VK_TAB]) 
  {
    int next = pressedKeys[VK_SHIFT] ? -1 : 1;
    HWND w = GetForegroundWindow();
    if (w == NULL) {
      WASABI_API_WND->main_getRootWnd()->setFocus();
      return 1;
    }
    ifc_window *cur = windowTracker->rootWndFromHwnd(w); // TODO: API_WNDMGR->
    if (cur != NULL) {
      ifc_window *nextwnd = windowTracker->getNextDesktopWindow(cur, next);
      if (nextwnd) nextwnd->setFocus();
      return 1;
    }
    WASABI_API_WND->main_getRootWnd()->setFocus();
    return 1;
  }
  if (from && pressedKeys[VK_CONTROL] && pressedKeys[VK_F4]) {
    ifc_window *dp = from->getDesktopParent();
    if (dp) {
      Layout *l = static_cast<Layout *>(dp->getInterface(layoutGuid));
      if (l) {
        Container *c = l->getParentContainer();
        if (c) {
          if (c->isMainContainer()) 
            c->setVisible(!c->isVisible());
          else
            c->close();
          return 1;
        }
      }
    }
  }
  if (pressedKeys[0x5D]) {
#if defined(WA3COMPATIBILITY)
    Main::appContextMenu(from, TRUE, 0);
#elif defined(WASABI_CUSTOM_CONTEXTMENUS)
    extern void appContextMenu(ifc_window *wnd);
    appContextMenu(windowTracker->rootWndFromHwnd(GetForegroundWindow()));
#endif
  }
  if (s[0] && from) return forwardKbdMessage(from, WM_KEYDOWN, (WPARAM)k, (LPARAM)kd);
  }
  return 0;
}

void Keyboard::syncKeyTable() {
  for (int i=0;i<MAX_KEY;i++) {
    //if (pressedKeys[i] && !(GetAsyncKeyState(i) & (1 << 31))) pressedKeys[i] = 0;
    if (pressedKeys[i] && !Std::keyDown(i)) pressedKeys[i] = 0;
  }
}

int Keyboard::onForwardOnKeyUp(ifc_window *from, int k, int kd) {
  if (WASABI_API_WND->isKeyboardLocked()) return 1;
  if (infw) return 0;
  if (k >= MAX_KEY) return 0;
  /*int hadkey = */MEMCMPC(pressedKeys, 0, sizeof(pressedKeys));
  pressedKeys[k]=0;
  syncKeyTable();
  wchar_t s[64]={0,};
  int first=1;
#ifdef LINUX
  for (int i=MAX_KEY-1; i >= 0; i--) {
#else
  for (int i=0;i<MAX_KEY;i++) {
#endif
    if (pressedKeys[i]) {
      wchar_t *n = getVkName(i);
      if (n) {
        if (!first) wcscat(s, L"+");
        else first=0;
        wcscat(s,n);
      }
    }
  }
  if (!*s) {
    if (!lastwasreset) 
		{
      lastwasreset = 1;
#ifdef WASABI_COMPILE_SCRIPT
			DebugStringW(L"keyboard: sending \"%s\" to script\n", s);
      SystemObject::onKeyDown(s);
      if (VCPU::getComplete()) {
        DebugStringW(L"keyboard: %s trapped by script\n", s);
        return 1;
      }
#endif
    }
  }
  return forwardKbdMessage(from, WM_KEYUP, (WPARAM)k, (LPARAM)kd);
}

int Keyboard::onForwardOnSysKeyDown(ifc_window *from, int k, int kd) {
  if (WASABI_API_WND->isKeyboardLocked()) return 1;
  if (infw) return 0;
  if(kd&(1<<29)) pressedKeys[0x12]=1;
  int r = onForwardOnKeyDown(from, k, 1);
  if (r == 0) {
    if (from && forwardKbdMessage(from, WM_SYSKEYDOWN, (WPARAM)k, (LPARAM)kd)) return 1;
  }
  return r;
}

int Keyboard::onForwardOnSysKeyUp(ifc_window *from, int k, int kd) 
{
  if (WASABI_API_WND->isKeyboardLocked()) return 1;
  if (infw) return 0;
  if(kd&(1<<29)) pressedKeys[0x12]=0;
  pressedKeys[k]=0;
  int r = onForwardOnKeyUp(from, k, 1);
  if (r == 0) {
    if (forwardKbdMessage(from, WM_SYSKEYUP, (WPARAM)k, (WPARAM)kd)) return 1;
  }
  return r;
}


int Keyboard::onForwardOnKillFocus() {
  // FG> I don't think this is necessary anymore because onkeydown always resyncs the pressedKeys table
  // and supressing this allows scripts to trap ctrl/alt/shit + clicks (otherwise the click would reset
  // the modifiers by way of an automatic focus)
  //MEMSET(pressedKeys,0,sizeof(pressedKeys));
  return 0;
}

void Keyboard::registerAcceleratorSection(const wchar_t *name, ifc_window *wnd, int global) 
{
  accSecEntries.addItem(new AccSec(name,wnd,global));
  viewer.viewItem(wnd);
}

int Keyboard::interceptOnChar(unsigned int c) {
  if (hookers.getNumItems() > 0) {
    return hookers.getLast()->onChar(c);
  }
  return 0;
}

int Keyboard::interceptOnKeyDown(int k){
  if (hookers.getNumItems() > 0) {
    return hookers.getLast()->onKeyDown(k);
  }
  return 0;
}

int Keyboard::interceptOnKeyUp(int k){
  if (hookers.getNumItems() > 0) {
    return hookers.getLast()->onKeyUp(k);
  }
  return 0;
}

int Keyboard::interceptOnSysKeyDown(int k, int kd){
  if (hookers.getNumItems() > 0) {
    return hookers.getLast()->onSysKeyDown(k, kd);
  }
  return 0;
}

int Keyboard::interceptOnSysKeyUp(int k, int kd){
  if (hookers.getNumItems() > 0) {
    return hookers.getLast()->onSysKeyUp(k, kd);
  }
  return 0;
}

void Keyboard::hookKeyboard(ifc_window *hooker) {
  hookers.addItem(hooker);
  DebugString("hookKeyboard = %d\n", hookers.getNumItems());
}

void Keyboard::unhookKeyboard(ifc_window *hooker) {
  hookers.removeItem(hooker);
  DebugString("unhookKeyboard = %d\n", hookers.getNumItems());
}

void Keyboard::reset() {
  if (lastwasreset) return;
  DebugString("keyboard reset\n");
  MEMZERO(pressedKeys, sizeof(pressedKeys));
  if (!lastwasreset) {
    lastwasreset = 1;
#ifdef WASABI_COMPILE_SCRIPT
    DebugString("keyboard: sending \"\" to script\n");
    SystemObject::onKeyDown(L"");
#endif
  }
}

int AccSecViewer::viewer_onItemDeleted(ifc_window *item) { 
  for(int i=0;i<Keyboard::accSecEntries.getNumItems();i++)
    if(Keyboard::accSecEntries[i]->wnd==item) {
      Keyboard::accSecEntries.removeByPos(i);
      i--;
    }
  return 1; 
}

wchar_t Keyboard::pressedKeys[MAX_KEY]={0,};
PtrList<AccSec> Keyboard::accSecEntries;
AccSecViewer Keyboard::viewer;
PtrList<ifc_window> Keyboard::hookers;
int Keyboard::infw = 0;
int Keyboard::lastwasreset = 0;
