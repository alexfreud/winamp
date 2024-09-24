#ifndef WA_HOTKEYS
#define WA_HOTKEYS

/*
** #define IPC_GEN_HOTKEYS_ADD xxxx // pass a genHotkeysAddStruct * struct in data
**
** To get the IPC_GEN_HOTKEYS_ADD IPC number, you need to do this:
**
** genhotkeys_add_ipc=SendMessage(winampWindow,WM_WA_IPC,(WPARAM)&"GenHotkeysAdd",IPC_REGISTER_WINAMP_IPCMESSAGE);
**
** Then you can use:
**
** PostMessage(winampWindow,WM_WA_IPC,(WPARAM)&myGenHotkeysAddStruct,genhotkeys_add_ipc);
*/

//flags for the genHotkeysAddStruct struct
#define HKF_BRING_TO_FRONT 0x1  // calls SetForegroundWindow before sending the message
#define HKF_HWND_WPARAM 0x2     // sets wParam with Winamp's window handle
#define HKF_COPY 0x4            // copies returned text to the clipboard (using CF_TEXT)
#define HKF_PLPOS_WPARAM 0x8    // sets wParam with current pledit position
#define HKF_ISPLAYING_WL 0x10   // sets wParam to genHotkeysAddStruct's wParam if playing, lParam if not
                                // uses IPC_ISPLAYING to check if playing
#define HKF_SHOWHIDE 0x20       // brings Winamp to front or minimizes Winamp if already at front
#define HKF_NOSENDMSG 0x40      // don't send any message to the winamp window

#define HKF_COPYW 0x80          // copies returned text to the clipboard (using CF_UNICODETEXT)
#define HKF_UNICODE_NAME 0x100  // set this when the 'name' is passed as a unicode string

#define HKF_DISABLED 0x80000000

#include "WinDef.h"

typedef struct {
  char *name;     //name that will appear in the Global Hotkeys preferences panel
  DWORD flags;    //one or more flags from above
  UINT uMsg;      //message that will be sent to winamp's main window (must always be !=NULL)
  WPARAM wParam;  //wParam that will be sent to winamp's main window
  LPARAM lParam;  //lParam that will be sent to winamp's main window
  char *id;       //unique string to identify this command - case insensitive
  HWND wnd;       //set the HWND to send message (or 0 for main winamp window)
  
  int extended[6]; //for future extension - always set to zero!
} genHotkeysAddStruct;

/*
** Before calling this function you need to setup the genHotkeysAddStruct struct
**
** genHotkeysAddStruct test_key = {0};   // this will make the compiler zero the struct for you
**                                       // though you can do it manually if you want to as well
**
** UINT test_ipc = 0;                    // this will hold the uMsg value to look for in the assigned window proc
**
** // then call the function to register the hotkey
** AddGlobalHotkey(&test_key,"Playback: Test key","Test_key",&test_ipc);
**
** void AddGlobalHotkey(genHotkeysAddStruct *hotkey, char* name, char* id, int* ipc){
** UINT genhotkeys_add_ipc = 0;
**   hotkey->wnd = 0;
**   hotkey->flags = 0;
**   hotkey->name = name;
**   hotkey->id = id;
**   *ipc = hotkey->uMsg = SendMessage(winampWindow,WM_WA_IPC,(WPARAM)&hotkey->id,IPC_REGISTER_WINAMP_IPCMESSAGE);
**
**   genhotkeys_add_ipc=SendMessage(winampWindow,WM_WA_IPC,(WPARAM)&"GenHotkeysAdd",IPC_REGISTER_WINAMP_IPCMESSAGE);
**   PostMessage(winampWindow,WM_WA_IPC,(WPARAM)hotkey,genhotkeys_add_ipc);
** }
*/

/*
** This shows how to detect the registered hotkey message
** then it's upto you what you do
**
** LRESULT CALLBACK WinampWnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
** int ret = 0;
**
**   // handles the hotkey for the option :o)
**   if(uMsg == test_ipc){
**     // here you can do things needed for the hotkey you registered
**   }
**
**   // do stuff before the winamp proc has been called
** 
**   ret = CallWindowProc(WinampProc,hwnd,uMsg,wParam,lParam);
**
**   // do other stuff after the winamp proc has been called
**   // then return the value from CallWindowProc(..)
**   return ret;
** }
*/

#endif