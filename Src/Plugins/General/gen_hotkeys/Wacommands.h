#ifndef ___WINAMP_COMMANDS___H___
#define ___WINAMP_COMMANDS___H___

#include "../winamp/wa_ipc.h"
#include "wa_hotkeys.h"

// calls SetForegroundWindow before sending the message
#define HKF_BRING_TO_FRONT 0x1
// sets wParam with Winamp's window handle
#define HKF_WPARAM_HWND 0x2
// copies returned text to the clipboard (CF_TEXT)
#define HKF_COPY_RET 0x4
// sets wParam with current pledit position
#define HKF_WPARAM_PLPOS 0x8
// sets wParam to genHotkeysAddStruct's wParam if playing, lParam if not
// uses IPC_ISPLAYING to check if playing
#define HKF_WPARAM_ISPLAYING_WL 0x10
// brings Winamp to front or minimizes Winamp if already at front
#define HKF_SHOWHIDE 0x20
#define HKF_CUSTOM_FUNC 0x40
// copies returned text to the clipboard (CF_UNICODETEXT)
#define HKF_COPYW_RET 0x80
#define HKF_UNICODE_NAME 0x100
// set this when the 'name' is passed as a unicode string

typedef void (*pfnWAC)();

struct WACommand
{
  wchar_t *id;
  char *name;
  DWORD dwFlags;
  UINT uMsg;
  WPARAM wParam;
  LPARAM lParam;
  BOOL bEnabled;
  HWND wnd;
};

extern WACommand *WACommands;

extern inline unsigned int GetCommandsNum();
void InitCommands();
int AddCommand(genHotkeysAddStruct *ghas);
extern inline char *GetCommandName(unsigned int i, bool *unicode);
extern inline wchar_t *GetCommandId(unsigned int i);
int GetCommandIdx(wchar_t *id);
int DoCommand(unsigned int i);

#endif