#ifndef NULLOSFT_MLDISC_PRIMOSDK_HELPER_HEADER
#define NULLOSFT_MLDISC_PRIMOSDK_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
//#include <primosdk.h>
#define PRIMOSDK_CMDSEQUENCE ((DWORD)0xFFFFFFFF)
#define PRIMOSDK_OK 0
#define PRIMOSDK_PACKETWRITTEN 0
// !!!! Not thread safe  !!!!

#define DEFAULT_HANDLE		((DWORD)0xFFFFFFFF)

// Initialization
LONG PrimoSDKHelper_Initialize(void);
LONG PrimoSDKHelper_Uninitialize(void);
BOOL PrimoSDKHelper_IsInitialized(void);
BOOL PrimoSDKHelper_IsLoaded(void);

// Drive Info (You can use DEFAULT_HANDLE)
DWORD PrimoSDKHelper_UnitInfo(PDWORD pdwUnit, PDWORD pdwType, PBYTE szDescr, PDWORD pdwReady);
DWORD PrimoSDKHelper_UnitInfo2(PDWORD pdwUnit, PDWORD pdwTypes, PDWORD pdwClass, PDWORD pdwBusType, PDWORD pdwRFU);

// Medium Info (You can use DEFAULT_HANDLE)
DWORD PrimoSDKHelper_DiscInfoEx(PDWORD pdwUnit, DWORD dwFlags, PDWORD pdwMediumType, PDWORD pdwMediumFormat, PDWORD pdwErasable, PDWORD pdwTracks, PDWORD pdwUsed, PDWORD pdwFree);
DWORD PrimoSDKHelper_DiscInfo2(PDWORD pdwUnit, PDWORD pdwMedium, PDWORD pdwProtectedDVD, PDWORD pdwFlags, PDWORD pdwMediumEx, PDWORD pdwRFU3);

#endif // NULLOSFT_MLDISC_PRIMOSDK_HELPER_HEADER
