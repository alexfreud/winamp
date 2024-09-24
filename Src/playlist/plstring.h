#pragma once
#include <bfc/platform/types.h>
extern wchar_t *(*plstring_wcsdup)(const wchar_t *str);
extern wchar_t *(*plstring_malloc)(size_t str_size);
extern void (*plstring_release)(wchar_t *str);
extern void (*plstring_retain)(wchar_t *str);
void plstring_init();