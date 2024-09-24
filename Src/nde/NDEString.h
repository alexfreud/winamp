/*
 Ben Allison benski@winamp.com Nov 14 2007
 Simple reference counted string, to avoid a whole bunch of _wcsdup's in NDE and ml_local
 */

#pragma once
#include "foundation/types.h"
#include "nx/nxstring.h"

enum
{
	STRING_IS_WCHAR=0,
	STRING_IS_NDESTRING=1,
};

#include "nde_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

	NDE_API wchar_t *ndestring_wcsdup(const wchar_t *str);
	NDE_API wchar_t *ndestring_wcsndup(const wchar_t *str, size_t n);
	NDE_API wchar_t *ndestring_malloc(size_t str_size);
	NDE_API void ndestring_release(wchar_t *str);
	NDE_API void ndestring_retain(wchar_t *str);
	NDE_API nx_string_t ndestring_get_string(wchar_t *str);
	
#ifdef __cplusplus
}
#endif