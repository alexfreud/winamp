#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../General/gen_ml/ml.h"
#include "./common.h"

#include <string>
#include <vector>
#include "../nu/AutoLock.h"

#define PLUGIN_VERSION_MAJOR		2
#define PLUGIN_VERSION_MINOR		03

HINSTANCE Plugin_GetInstance(void);
HWND Plugin_GetWinamp(void);
HWND Plugin_GetLibrary(void);

class Navigation;
HRESULT Plugin_GetNavigation(Navigation **instance);

typedef void (CALLBACK *PLUGINUNLOADCALLBACK)(void);
void Plugin_RegisterUnloadCallback(PLUGINUNLOADCALLBACK callback);


extern int winampVersion;

#define MUTEX_T CRITICAL_SECTION
#define MUTEX_ENTER(n) EnterCriticalSection(&(n))
#define MUTEX_LEAVE(n) LeaveCriticalSection(&(n))
#define MUTEX_INIT(n) InitializeCriticalSection(&(n))
#define MUTEX_DEL(n) DeleteCriticalSection(&(n))

#define FILECACHEVERSION 0x00000001
typedef struct FileCacheType {
  INT64 version;
  INT64 expires;
  INT64 urllen;
  INT64 datalen;
  INT64 resv1;  // Future use, older versions MUST ignore
  INT64 resv2;  // Future use, older versions MUST ignore
  INT64 resv3;  // Future use, older versions MUST ignore
  INT64 resv4;  // Future use, older versions MUST ignore
} FileCacheType;
#define FILECACHETYPE FileCacheType

struct url_info
{
	std::wstring url;
	size_t url_wcslen;
	std::wstring title;
	int length;
} ;

struct metadata_info
{
	std::wstring url;
	std::wstring tag;
	std::wstring metadata;
} ;

typedef std::vector<url_info> URLMap; // just to save some typing & template code ugliness
typedef std::vector<metadata_info> MetadataMap;


extern URLMap urlMap;
extern MetadataMap metadataMap;

extern Nullsoft::Utility::LockGuard urlMapGuard;

typedef void (CALLBACK *PLUGINTIMERPROC)(UINT_PTR /*eventId*/, DWORD /*elapsedMs*/, ULONG_PTR /*data*/);
UINT_PTR Plugin_SetTimer(UINT elapseMs, PLUGINTIMERPROC callback, ULONG_PTR data);
void Plugin_KillTimer(UINT_PTR eventId);

#endif