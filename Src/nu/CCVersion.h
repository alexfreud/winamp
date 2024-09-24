#ifndef NULLSOFT_CCVERSIONH
#define NULLSOFT_CCVERSIONH
#include <windows.h>
DWORD GetCommCtrlDllVersion(LPCTSTR);
#define PACKVERSION(major,minor) MAKELONG(minor,major)
#endif