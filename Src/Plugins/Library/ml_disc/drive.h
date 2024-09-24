#ifndef NULLSOFT_MLDISC_DRIVE_HEADER
#define NULLSOFT_MLDISC_DRIVE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

LPCWSTR Drive_GetTypeString(DWORD nType);
LPCWSTR Drive_GetBusTypeString(DWORD nBusType);
BOOL Drive_IsRecorderType(DWORD nType);
BOOL Drive_IsRecorder(CHAR cLetter);

#endif // NULLSOFT_MLDISC_DRIVE_HEADER

