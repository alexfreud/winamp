#ifndef NULLSOFT_BFC_STRCMP_H
#define NULLSOFT_BFC_STRCMP_H

#ifdef _WIN32
#include <shlwapi.h>
#define strcasestr StrStrIA
#endif

#ifdef __APPLE__
#include <string.h>
#define _strnicmp strncasecmp
#define _stricmp strcasecmp
#endif

#endif