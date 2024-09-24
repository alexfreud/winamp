#ifndef _WIN32_H
#define _WIN32_H

#ifndef WIN32
#error this file is only for win32
#endif

#ifndef _PLATFORM_H
#error this file should only be included from platform.h
#endif

// this should be the *only* place windows.h gets included!
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifndef _WIN32_WCE
#include <io.h>
#endif

#if defined(_MSC_VER)	// msvc
# define WASABIDLLEXPORT __declspec(dllexport)
# if _MSC_VER >= 1100
#  define NOVTABLE __declspec(novtable)
# endif
#endif

#define _TRY __try
#define _EXCEPT(x) __except(x)

#define OSPIPE HANDLE


typedef WCHAR OSFNCHAR;
typedef LPWSTR OSFNSTR;
typedef LPCWSTR OSFNCSTR;

#endif
