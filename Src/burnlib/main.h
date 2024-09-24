#pragma once

#include <windows.h>

#ifdef BURNLIB_EXPORTS
 #define BURNLIB_API //__declspec(dllexport)
#else
 #define BURNLIB_API //__declspec(dllimport)
#endif

extern HINSTANCE	dllInstance;
extern HMODULE		hResource;
extern HWND			winampWnd;

#include <api/service/api_service.h>
#include <api/service/waServiceFactory.h>
#include "../Agave/Language/api_language.h"
#include "./primosdk.h"
//library Initialization
BURNLIB_API void InitializeBurningLibrary(api_service *wasabiServiceManager, HINSTANCE, HWND);

// text values (englsih)
BURNLIB_API wchar_t* GetMediumText(wchar_t *buffer, unsigned int cchBuffer, DWORD medium);
BURNLIB_API wchar_t* GetMediumTypeText(wchar_t *buffer, unsigned int cchBuffer, DWORD type);
BURNLIB_API wchar_t* GetMediumFormatText(wchar_t *buffer, unsigned int cchBuffer, DWORD format);
BURNLIB_API wchar_t* GetUnitStatusText(wchar_t *buffer, unsigned int cchBuffer, DWORD sense, DWORD asc, DWORD ascq);
BURNLIB_API wchar_t* GetTrackTypeText(wchar_t *buffer, unsigned int cchBuffer, DWORD trackType);
BURNLIB_API wchar_t* GetPrimoCodeText(wchar_t *buffer, unsigned int cchBuffer, DWORD primoCode);
BURNLIB_API wchar_t* GetBussText(wchar_t *buffer, unsigned int cchBuffer, DWORD bussType);
wchar_t* GetTimeString(wchar_t *string, unsigned int cchLen, unsigned int timesec);