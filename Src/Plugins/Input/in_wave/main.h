#ifndef NULLSOFT_IN_WAVE_MAINH
#define NULLSOFT_IN_WAVE_MAINH

extern volatile int currentSongLength;
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES
#include "sndfile.h"
extern SNDFILE *sndFile;
#include "../Winamp/in2.h"
extern In_Module plugin;

extern int pan, volume;

#include <windows.h>
BOOL CALLBACK PreferencesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int ExtensionExists(const char *ext, const char *extensionList);

void BuildDefaultExtensions();

extern char defaultExtensions[1024];
void SetFileExtensions(const char *extList);
int CalcBitRate(const SF_INFO *info);
BOOL GetExtensionName(LPCWSTR pszExt, LPWSTR pszDest, INT cchDest);

#endif