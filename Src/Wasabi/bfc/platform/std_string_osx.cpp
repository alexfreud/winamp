#include <bfc/std.h>

// not perfect but it should work
int WCSICMP(const wchar_t *str1, const wchar_t *str2)
{
	while (*str1 && *str2) 
	{
		if (towlower(*str1) < towlower(*str2))
			return -1;
		if (towlower(*str2) < towlower(*str1)) 
			return 1;
		str1++;
		str2++;
	}
	
	if (!*str1 && !*str2) return 0;
	if (*str1) return 1;
	if (*str2) return -1;

	return -1; // shouldn't get here but we'll make the compiler shut up
}

int WCSNICMP(const wchar_t *str1, const wchar_t *str2, size_t len)
{
	while (*str1 && *str2 && len) 
	{
		if (towlower(*str1) < towlower(*str2)) 
			return -1;
		if (towlower(*str2) < towlower(*str1)) 
			return 1;
		str1++;
		str2++;
		len--;
	}

	if (!len) return 0;
	if (!*str1 && !*str2) return 0;
	if (*str1) return 1;
	if (*str2) return -1;

	return -1; // shouldn't get here but we'll make the compiler shut up
}


/* these are super slow because of memory allocation, but will tide us over until apple adds wcscasecmp and family to their BSD API */

#if 0 // remember this if we ever decide to use -fshort-wchar
int WCSICMP(const wchar_t *str1, const wchar_t *str2)
{
	CFStringRef cfstr1 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str1, wcslen(str1)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
	CFStringRef cfstr2 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str2, wcslen(str2)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
	int result = CFStringCompare(cfstr1, cfstr2, kCFCompareCaseInsensitive);
	CFRelease(cfstr1);
	CFRelease(cfstr2);
	return result;
}

int WCSNICMP(const wchar_t *str1, const wchar_t *str2, size_t len)
{
	CFStringRef cfstr1 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str1, wcslen(str1)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
	CFStringRef cfstr2 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str2, wcslen(str2)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
	int result =   CFStringCompareWithOptions(cfstr1, cfstr2, CFRangeMake(0, len), kCFCompareCaseInsensitive);
	CFRelease(cfstr1);
	CFRelease(cfstr2);
	return result;
}
#endif

int WCSICOLL(const wchar_t *str1, const wchar_t *str2)
{
	CFStringRef cfstr1 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str1, wcslen(str1)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
	CFStringRef cfstr2 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str2, wcslen(str2)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
	int result = CFStringCompare(cfstr1, cfstr2, kCFCompareCaseInsensitive|kCFCompareNonliteral);
	CFRelease(cfstr1);
	CFRelease(cfstr2);
	return result;
}

bool WCSIPREFIX(const wchar_t *str1, const wchar_t *prefix)
{
	CFStringRef cfstr1 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)str1, wcslen(str1)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
	CFStringRef cfstr2 =  CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8 *)prefix, wcslen(prefix)*sizeof(wchar_t), kCFStringEncodingUTF32, false);
	bool result = CFStringHasPrefix(cfstr1, cfstr2);
	CFRelease(cfstr1);
	CFRelease(cfstr2);
	return result;
}

wchar_t *DO_WCSDUP(const wchar_t *ptr EXTRA_INFO) 
{
	if (ptr == NULL) return NULL;
	int size = wcslen(ptr);
	wchar_t *ret = (wchar_t *)MALLOC((size + 1) * sizeof(wchar_t));
	if (ret != NULL) 
	{
		WCSCPYN(ret, ptr, size+1);
	}
	return ret;
}

void WCSCPYN(wchar_t *dest, const wchar_t *src, int maxchar)
{
	ASSERT(dest != NULL);
	ASSERT(src != NULL);
	wcsncpy(dest, src, maxchar-1); // TODO: switch to a less brain dead function
  dest[maxchar-1]=0;
}

void STRTOUPPER(char *str) 
{
if (str)
{
  while (*str)
  {
    *str = toupper(*str);
  }
}
}
void STRTOLOWER(char *str) 
{
  if (str)
  {
    while (*str)
    {
      *str = towlower(*str);
    }
  }
}

COMEXP void WCSTOUPPER(wchar_t *str)
{
  if (str)
  {
    while (*str)
    {
      *str = towupper(*str);
    }
  }
}

COMEXP void WCSTOLOWER(wchar_t *str)
{
  if (str)
  {
    while (*str)
    {
      *str = towlower(*str);
    }
  }
}

int WCSICMPSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1, const wchar_t *defval2) 
{
  if (str1 == NULL) str1 = defval1;
  if (str2 == NULL) str2 = defval2;
  return WCSICMP(str1, str2);
}

wchar_t *WCSTOK(wchar_t *str, const wchar_t *sep, wchar_t **last)
{
	return wcstok(str, sep, last);
}

int WCSCASEEQLSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1, const wchar_t *defval2) 
{
  return !WCSICMPSAFE(str1, str2, defval1, defval2);
}

int STRLEN(const char *str) 
{
  ASSERT(str != NULL);
  return strlen(str);
}

bool ISALPHA(wchar_t alpha) 
{
	return iswalpha(alpha); 
}

bool ISDIGIT(wchar_t digit)
{
  return iswdigit(digit);
}

bool ISSPACE(wchar_t space)
{
  return iswspace(space);
}

bool ISPUNCT(wchar_t punct)
{
  return iswpunct(punct);
}

int STRCMP(const char *str1, const char *str2) 
{
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
  return strcmp(str1, str2);
}

int STRCMPSAFE(const char *str1, const char *str2, const char *defval1, const char *defval2) 
{
  if (str1 == NULL) str1 = defval1;
  if (str2 == NULL) str2 = defval2;
  return STRCMP(str1, str2);
}

void STRNCPY(char *dest, const char *src, int maxchar) 
{
  ASSERT(dest != NULL);
  ASSERT(src != NULL);
  strncpy(dest, src, maxchar);
  //INLINE
}

int STRICMPSAFE(const char *str1, const char *str2, const char *defval1, const char *defval2) 
{
  if (str1 == NULL) str1 = defval1;
  if (str2 == NULL) str2 = defval2;
  return STRICMP(str1, str2);
}

int STRICMP(const char *str1, const char *str2) 
{
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
  return strcasecmp(str1, str2);
}

void STRCPY(char *dest, const char *src) 
{
  ASSERT(dest != NULL);
  ASSERT(src != NULL);
  ASSERT(dest != src);
  strcpy(dest, src);
  //INLINE
}

char *STRSTR(const char *str1, const char *str2) 
{
  ASSERT(str1!=NULL);
  ASSERT(str2!=NULL);
  ASSERT(str1 != str2);
  return strstr(str1, str2);
}
