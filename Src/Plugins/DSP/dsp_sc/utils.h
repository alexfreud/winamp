#ifndef __UTILS_H
#define __UTILS_H

#include <vector>
#include <string>

#ifdef WIN32
#define snprintf _snprintf
#define snwprintf _snwprintf
#endif

extern char IniName[MAX_PATH],
			IniEncName[MAX_PATH],
			IniDir[MAX_PATH];

wchar_t* GetSharedDirectoryW(HWND winamp);
wchar_t* GetPluginDirectoryW(HWND winamp);
char* GetIniDirectory(HWND winamp);
char* GetSCIniFile(HWND winamp);
void GetDefaultNextTracksLogFile(HWND winamp, int bufferLen, wchar_t* buffer, int index);

#define NUM_OUTPUTS 5
extern HANDLE logFiles[NUM_OUTPUTS];
wchar_t* GetSCLogFile(HWND winamp, int bufferLen, wchar_t* logFile, int index);
char* CreateLogFileMessage(char* buffer, wchar_t* message, int* len);
void StartLogging(int index, int clearOnStart);
void StopLogging(int index);

void StartNextTracks(int index, wchar_t* file);
void WriteNextTracks(int index, HWND winamp, std::vector<int> nextListIdx, std::vector<std::wstring> nextList, bool xml);
void StopNextTracks(int index);

void StartSaveEncoded(int index, wchar_t* file);
void WriteSaveEncoded(int index, LPCVOID buffer, int bufferLen);
void StopSaveEncoded(int index);

INT_PTR LocalisedDialogBox(HINSTANCE hDllInstance, UINT dialog_id, HWND hWndParent, DLGPROC lpDialogFunc);
HWND LocalisedCreateDialog(HINSTANCE instance, UINT dialog_id, HWND hWndParent, DLGPROC DlgProc, LPARAM user_id);
char* LocalisedStringA(UINT uID, char *str, size_t maxlen);
wchar_t* LocalisedString(UINT uID, wchar_t *str, size_t maxlen);

UINT GetWinampVersion(HWND winamp);

bool IsVistaUp();
HINSTANCE GetMyInstance();
HWND GetWinampHWND(HWND winamp);

extern int isthemethere;
BOOL link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void link_startsubclass(HWND hwndDlg, UINT id);
BOOL DirectMouseWheel_ProcessDialogMessage(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);

char* escapeXML(const char* s);
char* ConvertToUTF8Escaped(const wchar_t *str);
char* ConvertToUTF8(const wchar_t *str);
int ConvertFromUTF8(const char *src, wchar_t *dest, int destlen);
// reads a unicode string stored in utf8 from an ini file
DWORD GetPrivateProfileStringUTF8(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName);

void WritePrivateProfileInt(LPCSTR lpKeyName, int value, LPCSTR lpAppName, LPCSTR lpFileName);

void ShowWindowDlgItem(HWND hDlg, int nIDDlgItem, int nCmdShow);
void EnableWindowDlgItem(HWND hDlg, int nIDDlgItem, BOOL bEnable);

bool CompareVersions(char *verStr);

#endif