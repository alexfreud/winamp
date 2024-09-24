#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <windows.h>
#include "resource.h"

#include "..\settings.h"

extern Settings settings;

static int ParseCommandLine(wchar_t *cmdline, wchar_t **argv);

BOOL CALLBACK SilentDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL ZipData(void);
BOOL SendData(HWND hwnd);
BOOL Restart(void);

#endif FEEDBACK_H