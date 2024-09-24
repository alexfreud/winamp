#ifndef _FF_PREFS_H
#define _FF_PREFS_H

#include <api/config/items/cfgitemi.h>

extern INT_PTR CALLBACK ffPrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern INT_PTR CALLBACK ffPrefsProc1(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern INT_PTR CALLBACK ffPrefsProc2(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern INT_PTR CALLBACK ffPrefsProc3(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern INT_PTR CALLBACK ffPrefsProc4(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern INT_PTR CALLBACK ffPrefsProc5(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

void _dosetsel(HWND hwndDlg);
HWND 
ActiveChildWindowFromPoint(HWND hwnd, POINTS cursor_s, const int *controls, size_t controlsCount);

#define WA2FFOPTIONS_PARENT CfgItemI

// {68A2EFD7-0FBB-4ef9-9D3A-590F943C2A73}
static const GUID Wa2FFOptionsGuid = 
{ 0x68a2efd7, 0xfbb, 0x4ef9, { 0x9d, 0x3a, 0x59, 0xf, 0x94, 0x3c, 0x2a, 0x73 } };

class Wa2FFOptions : public WA2FFOPTIONS_PARENT {
public:
  Wa2FFOptions ();
};

extern Wa2FFOptions *ffoptions;

#endif