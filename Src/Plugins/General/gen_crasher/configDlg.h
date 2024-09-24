#pragma once
#include <windows.h>

int SelectComboBoxItem(const HWND hwCombo, int data);
BOOL OpenFolderDialog(HWND parent, LPWSTR pathBuffer);
BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void UpdateSendType(HWND hwndDlg, BOOL enabled);
void UpdateSend(HWND hwndDlg, BOOL enabled);
void UpdateCreateDmp(HWND hwndDlg, BOOL enabled);
void UpdateCreateLog(HWND hwndDlg, BOOL enabled);
void UpdateZip(HWND hwndDlg, BOOL enabled);

void CreatePathFromFullName(wchar_t **path, const wchar_t *fullname);
const wchar_t* GetFileName(const wchar_t *fullname);