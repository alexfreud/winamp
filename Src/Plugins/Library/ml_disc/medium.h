#ifndef NULLSOFT_MLDISC_MEDIUM_HEADER
#define NULLSOFT_MLDISC_MEDIUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

LPCWSTR Medium_GetTypeString(DWORD nType);
LPCWSTR Medium_GetPhysicalTypeString(DWORD nType);
LPCWSTR Medium_GetFormatString(DWORD nFormat);
BOOL Medium_IsRecordableType(DWORD nType);
BOOL Medium_IsRecordable(CHAR cLetter);


#endif // NULLSOFT_MLDISC_MEDIUM_HEADER