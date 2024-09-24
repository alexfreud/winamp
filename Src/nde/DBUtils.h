/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 All Purposes Functions Prototypes

--------------------------------------------------------------------------- */

#ifndef __DBUTILS_H
#define __DBUTILS_H

#include <stdio.h>
#include "vfs.h"
#include "field.h"

bool CompatibleFields(unsigned char oldType, unsigned char newType);
uint32_t AllocNewPos(VFILE *Handle);
Field *TranslateObject(unsigned char Type, Table *tbl);

#ifndef __ANDROID__
const void *memmem(const void *a, const void *b, size_t s, size_t l);
#endif

#ifdef __ANDROID__
char *stristr(const char *s1, const char *s2);
int mystricmp(const char *a, const char *b);
char* mystristr(const char *a, const char *b);
int  nde_stricmp(const char *a, const char *b);  
int nde_stricmp_ignore(const char *a, const char *b); // ignores accents
int nde_strnicmp(const char *a, const char *b, size_t len); // len must be <= strlen(b)
int nde_strnicmp_ignore(const char *a, const char *b, size_t len); // ignores accents
char *stristr_ignore(const char *s1, const char *s2);
// filesystem safe versions
char* mystristr_fn(const char *a, const char *b);
int mystricmp_fn(const char *a, const char *b);
char *stristr_fn(const char *s1, const char *s2);
extern "C" int NDE_API nde_stricmp_fn(const char *a, const char *b);  
int nde_strnicmp_fn(const char *a, const char *b, size_t len); // len must be <= strlen(b)

size_t utf16LE_to_utf8(const uint16_t *src, size_t source_len, char *dst, size_t out_len);
size_t utf16BE_to_utf8(const uint16_t *src, size_t source_len, char *dst, size_t out_len);
#endif

#ifdef _WIN32
#include <windows.h>
int WINAPI NDE_FindNLSString(LCID Locale, DWORD dwFindNLSStringFlags, LPCWSTR lpStringSource, int cchSource, LPCWSTR lpStringValue, int cchValue, LPINT pcchFound);
extern int (WINAPI *findNLSString)(LCID Locale, DWORD dwFindNLSStringFlags, LPCWSTR lpStringSource, int cchSource, LPCWSTR lpStringValue, int cchValue, LPINT pcchFound);
bool nde_wcsbegins(const wchar_t *a, const wchar_t *b);
bool nde_wcsends(const wchar_t *a, const wchar_t *b);
bool nde_wcscontains(const wchar_t *a, const wchar_t *b);

int mywcsicmp(const wchar_t *a, const wchar_t *b);
int  nde_wcsicmp(const wchar_t *a, const wchar_t *b);  

// filesystem safe versions
bool nde_fnbegins(const wchar_t *a, const wchar_t *b);
bool nde_fnends(const wchar_t *a, const wchar_t *b);
bool nde_fncontains(const wchar_t *a, const wchar_t *b);

const wchar_t* mywcsistr_fn(const wchar_t *a, const wchar_t *b);
int mywcsicmp_fn(const wchar_t *a, const wchar_t *b);
extern "C" int NDE_API nde_wcsicmp_fn(const wchar_t *a, const wchar_t *b);  
#endif

#ifdef __APPLE__
wchar_t *_wcsdup(const wchar_t *val);
#endif

#endif