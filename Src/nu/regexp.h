#ifndef NULLSOFT_UTILITY_REGEXP_H
#define NULLSOFT_UTILITY_REGEXP_H
#pragma once

#ifdef _WIN32
#include <wchar.h>
typedef wchar_t regchar_t;
#else
typedef char regchar_t;
#endif
// strings must be in ALL CAPS. sorry.
bool Match(const regchar_t *match, const regchar_t *string);

#endif