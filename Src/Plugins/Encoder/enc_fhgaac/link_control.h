#pragma once
#include <windows.h>

extern "C" {
void link_startsubclass(HWND hwndDlg, UINT id);
void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
}