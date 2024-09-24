// ***************************************************************************
//  rapitypes2.h
//
//  Copyright 2003 Microsoft Corporation, All Rights Reserved
//
//  Types needed for RAPI2.
//
// ***************************************************************************

#pragma once

#ifdef MIDL_ONLY

#include <basetsd.h>

typedef struct OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union UNION_OFFSET_POINTER{
        struct OFFSET{
            DWORD Offset;
            DWORD OffsetHigh;
        } OFFSET;

        PVOID Pointer;
    } UNION_OFFSET_POINTER;

    HANDLE  hEvent;
} OVERLAPPED;

typedef OVERLAPPED * LPOVERLAPPED;

#define CONST const
typedef CONST void far      *LPCVOID;
typedef LONG *PLONG;    

typedef struct STARTUPINFOA {
    DWORD   cb;
    LPSTR   lpReserved;
    LPSTR   lpDesktop;
    LPSTR   lpTitle;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwXCountChars;
    DWORD   dwYCountChars;
    DWORD   dwFillAttribute;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
    LPBYTE  lpReserved2;
    HANDLE  hStdInput;
    HANDLE  hStdOutput;
    HANDLE  hStdError;
} STARTUPINFOA;

typedef STARTUPINFOA * LPSTARTUPINFOA;

typedef struct STARTUPINFOW {
    DWORD   cb;
    LPWSTR  lpReserved;
    LPWSTR  lpDesktop;
    LPWSTR  lpTitle;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwXCountChars;
    DWORD   dwYCountChars;
    DWORD   dwFillAttribute;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
    LPBYTE  lpReserved2;
    HANDLE  hStdInput;
    HANDLE  hStdOutput;
    HANDLE  hStdError;
} STARTUPINFOW;

typedef STARTUPINFOW * LPSTARTUPINFOW;

#ifdef UNICODE
typedef STARTUPINFOW STARTUPINFO;
typedef LPSTARTUPINFOW LPSTARTUPINFO;
#else
typedef STARTUPINFOA STARTUPINFO;
typedef LPSTARTUPINFOA LPSTARTUPINFO;
#endif // UNICODE

typedef struct PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION;

typedef PROCESS_INFORMATION * PPROCESS_INFORMATION;
typedef PROCESS_INFORMATION * LPPROCESS_INFORMATION;

typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK REGSAM;
typedef HKEY *PHKEY;

typedef struct SYSTEM_INFO {
    union UNION_OEMID_PROCESSOR_INFO{
        DWORD dwOemId;          // Obsolete field...do not use
        struct PROCESSOR_INFO{
            WORD wProcessorArchitecture;
            WORD wReserved;
        } PROCESSOR_INFO;
    } UNION_OEMID_PROCESSOR_INFO;
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO;
typedef SYSTEM_INFO * LPSYSTEM_INFO;


typedef struct MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
} MEMORYSTATUS;

typedef MEMORYSTATUS * LPMEMORYSTATUS;

#define _SS_MAXSIZE 128                  // Maximum size.
#define _SS_ALIGNSIZE (sizeof(__int64))  // Desired alignment.
#define _SS_PAD1SIZE (_SS_ALIGNSIZE - sizeof (short))
#define _SS_PAD2SIZE (_SS_MAXSIZE - (sizeof (short) + _SS_PAD1SIZE \
                                                    + _SS_ALIGNSIZE))
/*
typedef struct sockaddr_storage {
    short ss_family;               // Address family.
    char __ss_pad1[_SS_PAD1SIZE];  // 6 byte pad, this is to make
                                   // implementation specific pad up to
                                   // alignment field that follows explicit
                                   // in the data structure.
    __int64 __ss_align;            // Field to force desired structure.
    char __ss_pad2[_SS_PAD2SIZE];  // 112 byte pad to achieve desired size;
                                   // _SS_MAXSIZE value minus size of
                                   // ss_family, __ss_pad1, and
                                   // __ss_align fields is 112.
} sockaddr_storage ;

typedef struct sockaddr_storage SOCKADDR_STORAGE;
*/
#endif // MIDL_ONLY

// end gross struct copy hack (everything after this is previously hacked from CE stuff that doesn't exist on desktop, so leave it as is).

//
// DEVICEID structure
//
typedef GUID RAPIDEVICEID;
#define RAPIDEVICEID_BACKCOMPAT  GUID_NULL  // device id for older devices

#ifndef UNDER_CE

// this is defined in CE's pwinuser.h, so we only want it for desktop
typedef struct {
    DWORD dwMajor;
    DWORD dwMinor;
} PLATFORMVERSION;

#endif

//
//  struct to hold additional platforms and their versions.
//
#define MAX_SUB_PLATFORMS   6

typedef struct RAPI_DEVICE_PLATFORM
{
    BSTR     bstrName;
    DWORD    dwMajor;
    DWORD    dwMinor;
} RAPI_DEVICE_PLATFORM;

//
// RAPI_DEVICEINFO
//
typedef struct RAPI_DEVICEINFO 
{
    RAPIDEVICEID    DeviceId;                       
    DWORD           dwOsVersionMajor;               
    DWORD           dwOsVersionMinor;               
    BSTR            bstrName;       
    BSTR            bstrPlatform;
} RAPI_DEVICEINFO;                     

#define FreeDeviceInfoData(pDevInfo) \
    do \
    { \
        SysFreeString((pDevInfo)->bstrName); \
        SysFreeString((pDevInfo)->bstrPlatform); \
    } while (0,0)

#ifndef MIDL_ONLY
#include "winsock2.h"
#endif

//
// RAPI_DEVICESTATUS
//
typedef enum RAPI_DEVICESTATUS
{
    RAPI_DEVICE_DISCONNECTED = 0,
    RAPI_DEVICE_CONNECTED    = 1,
} RAPI_DEVICESTATUS;

//
// op codes for IRAPIDesktop funcs
//
typedef enum RAPI_GETDEVICEOPCODE
{
    RAPI_GETDEVICE_NONBLOCKING,
    RAPI_GETDEVICE_BLOCKING
} RAPI_GETDEVICEOPCODE;

//
// Connected type codes
//
typedef enum RAPI_CONNECTIONTYPE
{
    RAPI_CONNECTION_USB         = 0,
    RAPI_CONNECTION_IR          = 1,
    RAPI_CONNECTION_SERIAL      = 2,
    RAPI_CONNECTION_NETWORK     = 3,
} RAPI_CONNECTIONTYPE;

typedef struct RAPI_CONNECTIONINFO {
    SOCKADDR_STORAGE ipaddr;            
    SOCKADDR_STORAGE hostIpaddr;   
    RAPI_CONNECTIONTYPE   connectionType;
} RAPI_CONNECTIONINFO;
