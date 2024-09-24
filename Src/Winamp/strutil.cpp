/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/

#include <windows.h>
#include <Shlwapi.h>
#include "strutil.h"

char *SkipX(char *str, int count)
{
	while (count--)
	{
		str = CharNextA(str);
	}

	return str;
}

wchar_t *SkipXW(wchar_t *str, int count)
{
	while (count--)
	{
		str = CharNextW(str);
	}

	return str;
}

void CopyChar(char *dest, const char *src)
{
	char *end = CharNextA(src);
	ptrdiff_t count = end-src;
	while (count--)
	{
		*dest++=*src++;
	}
}

ptrdiff_t CopyCharW(wchar_t *dest, const wchar_t *src)
{
	wchar_t *end = CharNextW(src);
	ptrdiff_t count = end-src;
	for (ptrdiff_t i=0;i<count;i++)
	{
		*dest++=*src++;
	}
	return count;
}

void MakeRelativePathName(const wchar_t *filename, wchar_t *outFile, const wchar_t *path)
{
	wchar_t outPath[MAX_PATH] = {0};

	int common = PathCommonPrefixW(path, filename, outPath);
	if (common && common == lstrlenW(path))
	{
		PathAddBackslashW(outPath);
		const wchar_t *p = filename + lstrlenW(outPath);
		lstrcpynW(outFile, p, FILENAME_SIZE);
	}
	else if (!PathIsUNCW(filename) && PathIsSameRootW(filename, path))
	{
		lstrcpynW(outFile, filename+2, FILENAME_SIZE);
	}
}

static int CharacterCompareW(const wchar_t *ch1, const wchar_t *ch2)
{
	wchar_t str1[3]={0,0,0}, str2[3]={0,0,0};

	CopyCharW(str1, ch1);
	CharUpperW(str1);

	CopyCharW(str2, ch2);
	CharUpperW(str2);

	return memcmp(str1, str2, 3*sizeof(wchar_t));
}

static void IncHelperW(LPCWSTR *src, ptrdiff_t *size)
{
	wchar_t *end = CharNextW(*src);
	ptrdiff_t count = end-*src;
	*size-=count;
	*src=end;
}

int IsCharDigit(char digit)
{
	WORD type=0;
	GetStringTypeExA(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return type&C1_DIGIT;
}

int IsCharDigitW(wchar_t digit)
{
	WORD type=0;
	GetStringTypeExW(LOCALE_USER_DEFAULT, CT_CTYPE1, &digit, 1, &type);
	return type&C1_DIGIT;
}

int FileCompareLogical(const wchar_t *str1, const wchar_t *str2)
{
	if (str1 && str2)
	{
		while (str1 && *str1)
		{
			if (!*str2)
				return 1;
			else if (IsCharDigitW(*str1))
			{
				int iStr, iComp;
 
				if (!IsCharDigitW(*str2))
					return -1;

				/* Compare the numbers */
				StrToIntExW(str1, 0, &iStr);
				StrToIntExW(str2, 0, &iComp);

				if (iStr < iComp)
					return -1;
				else if (iStr > iComp)
					return 1;

				/* Skip */
				while (IsCharDigitW(*str1))
					str1=CharNextW(str1);
				while (IsCharDigitW(*str2))
					str2=CharNextW(str2);
			}
			else if (IsCharDigitW(*str2))
				return 1;
			else
			{
				int diff = CharacterCompareW(str1, str2);
				if (diff > 0)
					return 1;
				else if (diff < 0)
					return -1;

				str1=CharNextW(str1);
				str2=CharNextW(str2);
			}
		}
		if (*str2)
			return -1;
	}
	return 0;
}

static int StringLengthNoDigits(LPCWSTR str, LPCWSTR *end)
{
	ptrdiff_t length=0;
	while (str && *str && !IsCharDigitW(*str))
	{
		IncHelperW(&str, &length);
	}
	if (end) *end = str;
	return (int)(-length); // IncHelper decrements so we need to negate
}

int CompareStringLogical(LPCWSTR str1, LPCWSTR str2)
{
	if (str1 && str2)
	{
		while (str1 && *str1)
		{
			if (!*str2)
				return 1;
			else if (IsCharDigitW(*str1))
			{
				int iStr, iComp;
 
				if (!IsCharDigitW(*str2))
					return -1;
 
				/* Compare the numbers */
				StrToIntExW(str1, 0, &iStr);
				StrToIntExW(str2, 0, &iComp);

				if (iStr < iComp)
					return -1;
				else if (iStr > iComp)
					return 1;

				/* Skip */
				while (IsCharDigitW(*str1))
					str1=CharNextW(str1);
				while (IsCharDigitW(*str2))
					str2=CharNextW(str2);
			}
			else if (IsCharDigitW(*str2))
				return 1;
			else
			{
				LPCWSTR next1, next2;
				int len1 = StringLengthNoDigits(str1, &next1);
				int len2 = StringLengthNoDigits(str2, &next2);

				int comp = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE|NORM_IGNOREWIDTH, str1, len1, str2, len2);
				if (comp == CSTR_LESS_THAN)
					return -1;
				else if (comp == CSTR_GREATER_THAN)
					return 1;

				str1 = next1;
				str2 = next2;
			}
		}
		if (*str2)
			return -1;
	}
	return 0;
}

int FileCompareLogicalN(LPCWSTR str1, ptrdiff_t str1size, LPCWSTR str2,  ptrdiff_t str2size)
{
	if (str1 && str2)
	{
		while (str1 && *str1 && str1size)
		{
			if (!*str2 || !str2size)
				return 1;
			else if (IsCharDigitW(*str1))
			{
				int iStr, iComp;
 
				if (!IsCharDigitW(*str2))
					return -1;

				/* Compare the numbers */
				StrToIntExW(str1, 0, &iStr);
				StrToIntExW(str2, 0, &iComp);

				if (iStr < iComp)
					return -1;
				else if (iStr > iComp)
					return 1;

				/* Skip */
				while (IsCharDigitW(*str1))
					IncHelperW(&str1, &str1size);
				while (IsCharDigitW(*str2))
					IncHelperW(&str2, &str2size);
			}
			else if (IsCharDigitW(*str2))
				return 1;
			else
			{
				int diff = CharacterCompareW(str1, str2);
				if (diff > 0)
					return 1;
				else if (diff < 0)
					return -1;

				IncHelperW(&str1, &str1size);
				IncHelperW(&str2, &str2size);
			}
		}

		if (!str1size && !str2size)
			return 0;
		if (*str2 || str2size < str1size)
			return -1;
		if (*str1 || str1size < str2size)
			return 1;
	}
	return 0;
}

char *GetLastCharacter(char *string)
{
	if (!string || !*string)
		return string;

	return CharPrevA(string, string+lstrlenA(string));
}

wchar_t *GetLastCharacterW(wchar_t *string)
{
	if (!string || !*string)
		return string;

	return CharPrevW(string, string+lstrlenW(string));
}

const char *GetLastCharacterc(const char *string)
{
	if (!string || !*string)
		return string;

	for (;;)
	{
		const char *next = CharNextA(string);
		if (!*next)
			return string;
		string = next;
	}
}

const wchar_t *GetLastCharactercW(const wchar_t *string)
{
	if (!string || !*string)
		return string;

	return CharPrevW(string, string+lstrlenW(string));
}

wchar_t *scanstr_backW(wchar_t *str, wchar_t *toscan, wchar_t *defval)
{
	wchar_t *s = GetLastCharacterW(str);
	if (!s || !str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	for (;;)
	{
		wchar_t *t = toscan;
		while (t && *t)
		{
			if (*t == *s) return s;
			t = CharNextW(t);
		}
		t = CharPrevW(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

const wchar_t *scanstr_backcW(const wchar_t *str, const wchar_t *toscan, const wchar_t *defval)
{
	const wchar_t *s = GetLastCharactercW(str);
	if (!s || !str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	for (;;)
	{
		const wchar_t *t = toscan;
		while (t && *t)
		{
			if (*t == *s) return s;
			t = CharNextW(t);
		}
		t = CharPrevW(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

char *scanstr_back(char *str, char *toscan, char *defval)
{
	char *s = GetLastCharacter(str);
	if (!s || !str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	for (;;)
	{
		char *t = toscan;
		while (t && *t)
		{
			if (*t == *s) return s;
			t = CharNextA(t);
		}
		t = CharPrevA(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

const char *scanstr_backc(const char *str, const char *toscan, const char *defval)
{
	const char *s = GetLastCharacterc(str);
	if (!s || !str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	for (;;)
	{
		const char *t = toscan;
		while (t && *t)
		{
			if (*t == *s) return s;
			t = CharNextA(t);
		}
		t = CharPrevA(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

char *extension(const char *fn)
{
	// TODO: deal with making sure that URLs don't return .com, etc.
	// e.g. http://www.winamp.com  should return nothing
	char *end = scanstr_back((char*)fn, "./\\", 0);
	if (!end)
		return (char*)(fn+lstrlenA(fn));

	if (*end == '.')
		return CharNextA(end);

	return (char*)(fn+lstrlenA(fn));
}

wchar_t *extensionW(const wchar_t *fn)
{
	// TODO: deal with making sure that URLs don't return .com, etc.
	// e.g. http://www.winamp.com  should return nothing
	wchar_t *end = scanstr_backW((wchar_t*)fn, L"./\\", 0);
	if (!end)
		return (wchar_t *)(fn+lstrlenW(fn));

	if (*end == L'.')
		return CharNextW(end);

	return (wchar_t*)(fn+lstrlenW(fn));
}

const char *extensionc(const char *fn)
{
	return extension(fn);
}

const wchar_t *extensioncW(const wchar_t *fn)
{
	return extensionW(fn);
}

void extension_ex(const char *fn, char *buf, int buflen)
{ 
	const char *s = extensionc(fn);
	if (!PathIsURLA(fn) 
		|| (!strstr(s, "?") && !strstr(s, "&") && !strstr(s, "=") && *s))
	{
		lstrcpynA(buf, s, buflen);
		return ;
	}
	// s is not a terribly good extension, let's try again
	{
		char *copy = _strdup(fn);
		s = "";
	again:
		{
			char *p = scanstr_back(copy, "?", copy);
			if (p != copy)
			{
				*p = 0;
				s = extension(copy);
				if (!*s) goto again;
			}
			lstrcpynA(buf, s, buflen);
		}
		free(copy);
	}
}

void extension_exW(const wchar_t *fn, wchar_t *buf, int buflen)
{ 
	const wchar_t *s = extensioncW(fn);
	if (!PathIsURLW(fn) 
		|| (!wcsstr(s, L"?") && !wcsstr(s, L"&") && !wcsstr(s, L"=") && *s))
	{
		lstrcpynW(buf, s, buflen);
		return ;
	}
	// s is not a terribly good extension, let's try again
	{
		wchar_t *copy = _wcsdup(fn);
		s = L"";
	again:
		{
			wchar_t *p = scanstr_backW(copy, L"?", copy);
			if (p != copy)
			{
				*p = 0;
				s = extensionW(copy);
				if (!*s) goto again;
			}
			lstrcpynW(buf, s, buflen);
		}
		free(copy);
	}
}