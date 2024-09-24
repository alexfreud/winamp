#include "std_string.h"
#include <bfc/assert.h>

int WCSICOLL(const wchar_t *str1, const wchar_t *str2)
{
	return lstrcmpiW(str1, str2);
}

int WCSICMP(const wchar_t *str1, const wchar_t *str2)
{
	// WCSICMP is supposed to be used for string-lookup kinds of code, so we need to make sure it's done in a non-locale-aware way
	return CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, str1, -1, str2, -1)-2;
	//return _wcsicmp(str1, str2);
}

int WCSNICMP(const wchar_t *str1, const wchar_t *str2, size_t len)
{
	return _wcsnicmp(str1, str2, len);
}

wchar_t *WCSTOK(wchar_t *str, const wchar_t *sep, wchar_t **last)
{
	return wcstok(str, sep);
}

bool ISALPHA(wchar_t alpha) 
{
	return IsCharAlphaW(alpha)==TRUE; 
}

bool ISDIGIT(wchar_t digit)
{
	WORD type=0;
	GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return !!(type&C1_DIGIT);
}

bool ISSPACE(wchar_t space)
{
	WORD type=0;
	GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, &space, 1, &type);
	return !!(type&C1_SPACE);
}

bool ISPUNCT(wchar_t punct)
{
	WORD type=0;
	GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, &punct, 1, &type);
	return !!(type&C1_PUNCT);	
}

char *STRSTR(const char *str1, const char *str2) {
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
  ASSERT(str1 != str2);
  return const_cast<char *>(strstr(str1, str2));
}

wchar_t *WCSCASESTR(const wchar_t *str1, const wchar_t *str2) 
{
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
  wchar_t *p;
  size_t len = wcslen(str2);
  for (p = (wchar_t *)str1; *p; p++) 
	{
    if (!_wcsnicmp(p, str2, len)) 
			return p;
  }
  return NULL;
}

void STRCPY(char *dest, const char *src) {
  ASSERT(dest != NULL);
  ASSERT(src != NULL);
  ASSERT(dest != src);
  strcpy(dest, src);
//INLINE
}

void STRNCPY(char *dest, const char *src, int maxchar) {
  ASSERT(dest != NULL);
  ASSERT(src != NULL);
  strncpy(dest, src, maxchar);
//INLINE
}

void WCSCPYN(wchar_t *dest, const wchar_t *src, size_t maxchar)
{
	ASSERT(dest != NULL);
  ASSERT(src != NULL);
	StringCchCopyW(dest, maxchar, src);
}

char *STRCHR(const char *str, int c) {
  ASSERT(str != NULL);
  return const_cast<char *>(strchr(str, c));
//INLINE
}

void STRCAT(char *dest, const char *append) {
  ASSERT(dest != NULL);
  ASSERT(append != NULL);
  ASSERT(dest != append);
  strcat(dest, append);
}

unsigned long STRTOUL(const char *s, char **p, int rx) {
  ASSERT(s != NULL);
  ASSERT(p != NULL);
  return strtoul(s,p,rx);
}

wchar_t *WCSDUP(const wchar_t *ptr) 
{
  if (ptr == NULL) return NULL;
  size_t size = wcslen(ptr);
  wchar_t *ret = (wchar_t *)MALLOC((size + 1) * sizeof(wchar_t));
  if (ret != NULL) 
	{
		WCSCPYN(ret, ptr, size+1);
  }
  return ret;
}

int STRLEN(const char *str) {
  ASSERT(str != NULL);
  return (int)strlen(str);
}

int STRCMP(const char *str1, const char *str2) {
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
  return strcmp(str1, str2);
}

int STRCMPSAFE(const char *str1, const char *str2, const char *defval1, const char *defval2) {
  if (str1 == NULL) str1 = defval1;
  if (str2 == NULL) str2 = defval2;
  return STRCMP(str1, str2);
}

int WCSCMPSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1, const wchar_t *defval2) {
  if (str1 == NULL) str1 = defval1;
  if (str2 == NULL) str2 = defval2;
  return wcscmp(str1, str2);
}
int STRICMP(const char *str1, const char *str2) {
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
#if defined(WIN32)
  return _stricmp(str1, str2);
#elif defined(LINUX)
  return strcasecmp(str1, str2);
#endif
}

int STRICMPSAFE(const char *str1, const char *str2, const char *defval1, const char *defval2) {
  if (str1 == NULL) str1 = defval1;
  if (str2 == NULL) str2 = defval2;
  return STRICMP(str1, str2);
}

int WCSICMPSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1, const wchar_t *defval2) 
{
  if (str1 == NULL) str1 = defval1;
  if (str2 == NULL) str2 = defval2;
  return WCSICMP(str1, str2);
}


int WCSEQLSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1, const wchar_t *defval2) {
  return !WCSCMPSAFE(str1, str2, defval1, defval2);
}

int STRCASEEQLSAFE(const char *str1, const char *str2, const char *defval1, const char *defval2) {
  return !STRICMPSAFE(str1, str2, defval1, defval2);
}

int WCSCASEEQLSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1, const wchar_t *defval2) 
{
  return !WCSICMPSAFE(str1, str2, defval1, defval2);
}



//FG> sorry brennan, this need to not be a const :)
void STRTOUPPER(char *p) 
{
	CharUpperA(p);
}

void STRTOLOWER(char *p) 
{
	CharLowerA(p);
}

void WCSTOUPPER(wchar_t *p) 
{
	CharUpperW(p);
}

void KEYWORDUPPER(wchar_t *p) 
{/*
	if (p)
	{
		while (p && *p)
		{
			*p = towupper(*p);
			p++;
		}
	}*/
	int l = (int)wcslen(p);
	// from MSDN - If LCMAP_UPPERCASE or LCMAP_LOWERCASE is set, the lpSrcStr and lpDestStr pointers can be the same. 
	LCMapStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), LCMAP_UPPERCASE, p, l, p, l);
}

void WCSTOLOWER(wchar_t *p) 
{
	CharLowerW(p);
}

int STRNICMP(const char *str1, const char *str2, int l) {
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);

  ASSERT(l>=0);
  while (TOUPPER(*str1) == TOUPPER(*str2) && *str1 != 0 && *str2 != 0 && l--)
    str1++, str2++;
  if (l == 0) return 0;
  return (*str2 - *str1);
}

int STRNCASEEQL(const char *str1, const char *str2, int l) {
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
  return STRNICMP(str1, str2, l) == 0;
}

int STREQL(const char *str1, const char *str2) {
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
  return (strcmp(str1, str2) == 0);
}

int STRCASEEQL(const char *str1, const char *str2) {
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
#ifdef WIN32
  return (_stricmp(str1, str2)==0);
#else
  return (strcasecmp(str1, str2)==0);
#endif
}