#ifndef _ASSERT_H
#define _ASSERT_H

#include <bfc/wasabi_std.h>

#ifdef CPLUSPLUS
extern "C" {
#endif

COMEXP void _assert_handler(const char *reason, const char *file, int line);
COMEXP void _assert_handler_str(const char *string, const char *reason, const char *file, int line);

#ifdef CPLUSPLUS
}
#endif

#ifdef ASSERT
#undef ASSERT
#endif

// benski> added june 7 2007.. somewhat of a hack - don't want to enable assert's for final builds
#include "../../Winamp/buildtype.h"

#if defined(_DEBUG) /*|| defined(INTERNAL) */|| defined(BETA)
#define ASSERTS_ENABLED
#endif

#ifdef ASSERTS_ENABLED

  #define ASSERT(x) ((x) ? void() : _assert_handler(#x, __FILE__, __LINE__))
  #define ASSERTPR(x, str) ((x) ? void() : _assert_handler_str((str), #x, __FILE__, __LINE__))
  #define ASSERTALWAYS(str) _assert_handler_str((str), 0, __FILE__, __LINE__)
#else
  #define ASSERT(x) ;
  #define ASSERTPR(x,y) ;
  #define ASSERTALWAYS ;
#endif

#endif
