// --------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//
//     rapi.h
//
// Purpose:
//
//    Master include file for Windows CE Remote API
//
// --------------------------------------------------------------------------

#ifndef RAPI_H
#define RAPI_H

#include <windows.h>

#include "rapitypes.h"
#include "irapistream.h"

#ifndef UNDER_CE

STDAPI CeRapiInitEx(RAPIINIT*);
STDAPI CeRapiInit();
STDAPI CeRapiUninit();
STDAPI CeRapiGetError(void);
STDAPI CeRapiFreeBuffer(LPVOID);
STDAPI_( HRESULT ) CeRapiInvoke(LPCWSTR, LPCWSTR,DWORD,BYTE *, DWORD *,BYTE **, IRAPIStream **,DWORD);

STDAPI_(CEOID)  CeCreateDatabase     (LPWSTR, DWORD, WORD, SORTORDERSPEC*);
STDAPI_(BOOL  ) CeDeleteDatabase     (CEOID);
STDAPI_(BOOL  ) CeDeleteRecord       (HANDLE, CEOID);
STDAPI_(HANDLE) CeFindFirstDatabase  (DWORD);
STDAPI_(CEOID)  CeFindNextDatabase    (HANDLE);
STDAPI_(BOOL  ) CeOidGetInfo         (CEOID, CEOIDINFO*);
STDAPI_(HANDLE) CeOpenDatabase       (PCEOID, LPWSTR, CEPROPID, DWORD, HWND);
STDAPI_(CEOID)  CeReadRecordProps    (HANDLE, DWORD, LPWORD, CEPROPID*, LPBYTE*, LPDWORD);
STDAPI_(CEOID)  CeSeekDatabase       (HANDLE, DWORD, DWORD, LPDWORD);
STDAPI_(BOOL  ) CeSetDatabaseInfo    (CEOID, CEDBASEINFO*);
STDAPI_(HANDLE) CeFindFirstFile      (LPCWSTR, LPCE_FIND_DATA);
STDAPI_(BOOL  ) CeFindNextFile       (HANDLE, LPCE_FIND_DATA);
STDAPI_(BOOL  ) CeFindClose          (HANDLE);
STDAPI_(DWORD ) CeGetFileAttributes  (LPCWSTR);
STDAPI_(BOOL  ) CeSetFileAttributes  (LPCWSTR, DWORD);
STDAPI_(HANDLE) CeCreateFile         (LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
STDAPI_(BOOL  ) CeReadFile           (HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
STDAPI_(BOOL  ) CeWriteFile          (HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
STDAPI_(BOOL  ) CeCloseHandle        (HANDLE);
STDAPI_(BOOL  ) CeFindAllFiles       (LPCWSTR, DWORD, LPDWORD, LPLPCE_FIND_DATA);
STDAPI_(BOOL  ) CeFindAllDatabases   (DWORD, WORD, LPWORD, LPLPCEDB_FIND_DATA);
STDAPI_(DWORD ) CeGetLastError       (void);
STDAPI_(DWORD ) CeSetFilePointer     (HANDLE, LONG, PLONG, DWORD);
STDAPI_(BOOL  ) CeSetEndOfFile       (HANDLE);
STDAPI_(BOOL  ) CeCreateDirectory    (LPCWSTR, LPSECURITY_ATTRIBUTES);
STDAPI_(BOOL  ) CeRemoveDirectory    (LPCWSTR);
STDAPI_(BOOL  ) CeCreateProcess      (LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPWSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION);
STDAPI_(BOOL  ) CeMoveFile           (LPCWSTR, LPCWSTR);
STDAPI_(BOOL  ) CeCopyFile           (LPCWSTR, LPCWSTR, BOOL);
STDAPI_(BOOL  ) CeDeleteFile         (LPCWSTR);
STDAPI_(DWORD ) CeGetFileSize        (HANDLE, LPDWORD);
STDAPI_(LONG  ) CeRegOpenKeyEx       (HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);
STDAPI_(LONG  ) CeRegEnumKeyEx       (HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPWSTR, LPDWORD, PFILETIME);
STDAPI_(LONG  ) CeRegCreateKeyEx     (HKEY, LPCWSTR, DWORD, LPWSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
STDAPI_(LONG  ) CeRegCloseKey        (HKEY);
STDAPI_(LONG  ) CeRegDeleteKey       (HKEY, LPCWSTR);
STDAPI_(LONG  ) CeRegEnumValue       (HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
STDAPI_(LONG  ) CeRegDeleteValue     (HKEY, LPCWSTR);
STDAPI_(LONG  ) CeRegQueryInfoKey    (HKEY, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, PFILETIME);
STDAPI_(LONG  ) CeRegQueryValueEx    (HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
STDAPI_(LONG  ) CeRegSetValueEx      (HKEY, LPCWSTR, DWORD, DWORD, LPBYTE, DWORD);
STDAPI_(BOOL  ) CeGetStoreInformation(LPSTORE_INFORMATION);
STDAPI_(INT   ) CeGetSystemMetrics   (INT);
STDAPI_(INT   ) CeGetDesktopDeviceCaps(INT);
STDAPI_(VOID  ) CeGetSystemInfo      (LPSYSTEM_INFO);
STDAPI_(DWORD ) CeSHCreateShortcut   (LPWSTR, LPWSTR);
STDAPI_(BOOL  ) CeSHGetShortcutTarget(LPWSTR, LPWSTR, INT);
STDAPI_(BOOL  ) CeCheckPassword      (LPWSTR);
STDAPI_(BOOL  ) CeGetFileTime        (HANDLE, LPFILETIME, LPFILETIME, LPFILETIME);
STDAPI_(BOOL  ) CeSetFileTime        (HANDLE, LPFILETIME, LPFILETIME, LPFILETIME);
STDAPI_(BOOL  ) CeGetVersionEx       (LPCEOSVERSIONINFO);
STDAPI_(HWND  ) CeGetWindow          (HWND, UINT);
STDAPI_(LONG  ) CeGetWindowLong      (HWND, int);
STDAPI_(int   ) CeGetWindowText      (HWND, LPWSTR, int);
STDAPI_(int   ) CeGetClassName       (HWND, LPWSTR, int);
STDAPI_(VOID  ) CeGlobalMemoryStatus (LPMEMORYSTATUS);
STDAPI_(BOOL  ) CeGetSystemPowerStatusEx(PSYSTEM_POWER_STATUS_EX, BOOL);
STDAPI_(DWORD ) CeGetTempPath        (DWORD, LPWSTR);
STDAPI_(DWORD ) CeGetSpecialFolderPath(int, DWORD, LPWSTR);
STDAPI_(HANDLE) CeFindFirstDatabaseEx (PCEGUID, DWORD);
STDAPI_(CEOID ) CeFindNextDatabaseEx (HANDLE, PCEGUID);
STDAPI_(CEOID ) CeCreateDatabaseEx   (PCEGUID, CEDBASEINFO*);
STDAPI_(BOOL  ) CeSetDatabaseInfoEx  (PCEGUID, CEOID, CEDBASEINFO*);
STDAPI_(HANDLE) CeOpenDatabaseEx     (PCEGUID, PCEOID, LPWSTR, CEPROPID, DWORD, CENOTIFYREQUEST *);
STDAPI_(BOOL  ) CeDeleteDatabaseEx   (PCEGUID, CEOID);
STDAPI_(CEOID ) CeReadRecordPropsEx  (HANDLE, DWORD, LPWORD, CEPROPID*, LPBYTE*, LPDWORD, HANDLE);
STDAPI_(CEOID ) CeWriteRecordProps   (HANDLE, CEOID, WORD, CEPROPVAL*);
STDAPI_(BOOL  ) CeMountDBVol         (PCEGUID, LPWSTR, DWORD);
STDAPI_(BOOL  ) CeUnmountDBVol       (PCEGUID);
STDAPI_(BOOL  ) CeFlushDBVol         (PCEGUID);
STDAPI_(BOOL  ) CeEnumDBVolumes      (PCEGUID, LPWSTR, DWORD);
STDAPI_(BOOL  ) CeOidGetInfoEx       (PCEGUID, CEOID, CEOIDINFO*);
STDAPI CeSyncStart                   (LPCWSTR);
STDAPI CeSyncStop                    ();
STDAPI_(BOOL  ) CeQueryInstructionSet (DWORD, LPDWORD);
STDAPI_(BOOL  ) CeGetDiskFreeSpaceEx (LPCWSTR, ULARGE_INTEGER *, ULARGE_INTEGER *, ULARGE_INTEGER *);
#endif // #ifndef UNDER_CE

#ifndef NO_APIMAP
#include <ceapimap.h>
#endif

#ifdef CONN_INTERNAL
#include <prapi.h>  // internal defines
#endif

#endif // #ifndef RAPI_H
