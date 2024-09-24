#ifndef TATAKI_EXPORT_H
#define TATAKI_EXPORT_H

#include <bfc/platform/types.h>
#include <api/service/api_service.h>

#ifdef _WIN32
#ifdef TATAKI_STATIC
#define TATAKIAPI
#else
#ifdef TATAKI_EXPORTS
#define TATAKIAPI __declspec(dllexport)
#else
#define TATAKIAPI __declspec(dllimport)
#endif
#endif
#elif defined(__GNUC__)
#ifdef TATAKI_EXPORTS
#define TATAKIAPI __attribute__ ((visibility("default")))
#else
#define TATAKIAPI
#endif
#else
#error port me
#endif

namespace Tataki
{
extern "C" TATAKIAPI size_t Init(api_service *_serviceApi);
extern "C" TATAKIAPI size_t Quit();
}

#endif