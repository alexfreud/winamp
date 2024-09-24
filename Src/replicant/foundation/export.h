#pragma once

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define DLLEXPORT __attribute__ ((visibility("default")))
#else
#error port me!
#endif

#ifdef _MSC_VER
#define DLLIMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#define DLLIMPORT 
#else
#error port me!
#endif

