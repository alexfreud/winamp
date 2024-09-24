#ifndef NULLSOFT_BFC_PLATFORM_MINMAX_H
#define NULLSOFT_BFC_PLATFORM_MINMAX_H

#if defined(_WIN32) && !defined(MIN)
#define MIN( a, b ) ((a>b)?b:a)
#define MAX( a, b ) ((a>b)?a:b)
#endif

#if defined(__APPLE__) && !defined(MIN)
#define MIN( a, b ) ((a>b)?b:a)
#define MAX( a, b ) ((a>b)?a:b)
#endif

#if defined(__linux__) && !defined(MIN)
#define MIN( a, b ) ((a>b)?b:a)
#define MAX( a, b ) ((a>b)?a:b)
#endif

#endif