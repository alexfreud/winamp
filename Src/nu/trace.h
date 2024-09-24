#ifndef NULLOSFT_MEDIALIBRARY_TRACE_HEADER
#define NULLOSFT_MEDIALIBRARY_TRACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef _DEBUG

#include <wtypes.h>

#ifdef __cplusplus
extern "C" {
#endif 

void DebugPrintfA(LPCSTR format, ...);
void DebugPrintfW(LPCWSTR format, ...);

#ifdef __cplusplus
}
#endif 

#ifdef UNICODE
#define DebugPrintf  DebugPrintfW
#else
#define DebugPrintf  DebugPrintfA
#endif // !UNICODE

#define aTRACE				OutputDebugStringA
#define aTRACE_FMT			DebugPrintfA
#define aTRACE_LINE(x)		aTRACE_FMT("%s\n", (x))

#define wTRACE				OutputDebugStringW
#define wTRACE_FMT			DebugPrintfW
#define wTRACE_LINE(x)		wTRACE_FMT(L"%s\n", (x))

#define TRACE				OutputDebugString
#define TRACE_FMT			DebugPrintf
#define TRACE_LINE(x)		TRACE_FMT(TEXT("%s\n"), (x))

#else // _DEBUG

#define aTRACE				
#define aTRACE_FMT			
#define aTRACE_LINE	

#define wTRACE				
#define wTRACE_FMT			
#define wTRACE_LINE

#define TRACE				
#define TRACE_FMT			
#define TRACE_LINE

#endif // _DEBUG
#endif // NULLOSFT_MEDIALIBRARY_TRACE_HEADER