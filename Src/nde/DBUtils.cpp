/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 All Purposes Functions

--------------------------------------------------------------------------- */

#include "nde.h"
#include "BinaryField.h"
#include "Binary32Field.h"
#include "vfs.h"
#include "ColumnField.h"
#include "IndexField.h"
#include "StringField.h"
#include "FilenameField.h"
#include "IntegerField.h"
#include "Int64Field.h"
#include "Int128Field.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
int (WINAPI *findNLSString)(LCID Locale, DWORD dwFindNLSStringFlags, LPCWSTR lpStringSource, int cchSource, LPCWSTR lpStringValue, int cchValue, LPINT pcchFound) = NDE_FindNLSString;
#endif

//---------------------------------------------------------------------------
bool CompatibleFields(unsigned char oldType, unsigned char newType)
{
	if (oldType == newType) // duh :)
		return true;
	// going from an int field to another int equivalent field is OK
	if ((oldType == FIELD_INTEGER || oldType == FIELD_BOOLEAN || oldType == FIELD_DATETIME || oldType == FIELD_LENGTH || oldType == FIELD_INT64) &&
		(newType == FIELD_INTEGER || newType == FIELD_BOOLEAN || newType == FIELD_DATETIME || newType == FIELD_LENGTH || newType == FIELD_INT64)) {
			return true;
	}

	// going from string to filename or filename to string is OK
	if ((oldType == FIELD_FILENAME && newType == FIELD_STRING)
		|| (oldType == FIELD_STRING && newType == FIELD_FILENAME))
	{
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------
uint32_t AllocNewPos(VFILE *Handle)
{
	Vfseek(Handle, 0, SEEK_END);
	return Vftell(Handle);
}

//---------------------------------------------------------------------------
Field *TranslateObject(unsigned char Type, Table *tbl)
{
	switch (Type)
	{
		case FIELD_COLUMN: //0
			return new ColumnField();
		case FIELD_INDEX: //1
			return new IndexField();
		case FIELD_STRING: // 3
			return new StringField();
		case FIELD_INTEGER: // 4
			return new IntegerField();
		case FIELD_BINARY: // 6
			return new BinaryField();
		case FIELD_DATETIME: // 10
			return new DateTimeField();
		case FIELD_LENGTH: // 11
			return new LengthField();
		case FIELD_FILENAME: // 12
			return new FilenameField();
		case FIELD_INT64: // 13
			return new Int64Field();
		case FIELD_BINARY32: // 14
			return new Binary32Field();
		case FIELD_INT128: // 15
			return new Int128Field();
		default:
#ifdef WIN32
			if (!tbl->HasErrors())
			{
				//MessageBox(plugin.hwndParent, "Your database has been corrupted!\n\nWinamp will try to continue, but some of the library metadata may be lost :(", "Database Error", 0);
			}
#else
			printf("NDE Error: unknown field type encountered\n");
#endif
			tbl->IncErrorCount();
			return new Field();
	}
}


//---------------------------------------------------------------------------
#ifndef __ANDROID__
const void *memmem(const void *a, const void *b, size_t s, size_t l)
{
	size_t n = s - l;
	while (n--)
	{
		if (!memcmp(a, b, l))
			return a;
		a = (const uint8_t *)a + 1;
	}
	return NULL;
}
#endif

#ifdef _WIN32
// a faster way of doing min(wcslen(str), _len)
static size_t nde_wcsnlen(const wchar_t *str, size_t _len)
{
	size_t len = 0;
	while (str && *str++)
	{
		if (_len == len)
			return len;
		len++;
	}
	return len;
}

// len must be <= wcslen(b)
static int nde_wcsnicmp(const wchar_t *a, const wchar_t *b, size_t len)
{
	return CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, a, (int)nde_wcsnlen(a, len), b, (int)len) - 2;
}

/* this is a VERY LIMITED emulation of the vista only function.  it ONLY supports the ways we're currently call it.  it's also slow.  */
int WINAPI NDE_FindNLSString(LCID Locale, DWORD dwFindNLSStringFlags, LPCWSTR lpStringSource, int cchSource, LPCWSTR lpStringValue, int cchValue, LPINT pcchFound)
{
	dwFindNLSStringFlags &= ~NORM_LINGUISTIC_CASING; // remove on XP and below, not supported
	if (dwFindNLSStringFlags & FIND_STARTSWITH)
	{
		dwFindNLSStringFlags &= ~FIND_STARTSWITH; // clear flag
		size_t len = wcslen(lpStringValue);
		if (CompareStringW(Locale, dwFindNLSStringFlags, lpStringSource, (int)nde_wcsnlen(lpStringSource, len), lpStringValue, (int)len) ==  CSTR_EQUAL)
			return 0;
		else
			return -1;
	}
	else if (dwFindNLSStringFlags & FIND_ENDSWITH)
	{
		dwFindNLSStringFlags &= ~FIND_ENDSWITH; // clear flag
		int lenp = (int)wcslen(lpStringValue), lend = (int)wcslen(lpStringSource);
		if (lend < lenp) return -1;  // too short
		if (CompareStringW(Locale, dwFindNLSStringFlags, lpStringSource+lend-lenp, -1, lpStringValue, -1) == CSTR_EQUAL)
			return 0;
		else
			return -1;
	}
	else if (dwFindNLSStringFlags & FIND_FROMSTART)
	{
		dwFindNLSStringFlags &= ~FIND_FROMSTART; // clear flag
		int s2len = (int)wcslen(lpStringValue);
		int s1len = (int)wcslen(lpStringSource);
		const wchar_t *p;
		for (p = lpStringSource;*p && s1len >= s2len;p++,s1len--)
			if (CompareStringW(Locale, dwFindNLSStringFlags, p, min(s1len, s2len), lpStringValue, (int)s2len) == CSTR_EQUAL)
				return 0;
		return -1;
	}

	return -1;
}

int nde_wcsicmp(const wchar_t *a, const wchar_t *b)
{
	return CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE|NORM_IGNORENONSPACE, a, -1, b, -1) - 2;
}

bool nde_wcsbegins(const wchar_t *a, const wchar_t *b)
{
	int index = findNLSString(LOCALE_USER_DEFAULT, FIND_STARTSWITH|NORM_LINGUISTIC_CASING|NORM_IGNORECASE|NORM_IGNORENONSPACE, a, -1, b, -1, 0);
	return (index != -1);
}

bool nde_wcsends(const wchar_t *a, const wchar_t *b)
{
	int index = findNLSString(LOCALE_USER_DEFAULT, FIND_ENDSWITH|NORM_LINGUISTIC_CASING|NORM_IGNORECASE|NORM_IGNORENONSPACE, a, -1, b, -1, 0);
	return (index != -1);
}

bool nde_wcscontains(const wchar_t *a, const wchar_t *b)
{
	int index = findNLSString(LOCALE_USER_DEFAULT, FIND_FROMSTART|NORM_LINGUISTIC_CASING|NORM_IGNORECASE|NORM_IGNORENONSPACE, a, -1, b, -1, 0);
	return index != -1;
}

//---------------------------------------------------------------------------

int mywcsicmp(const wchar_t *a, const wchar_t *b)
{
	if (!a && !b) return 0;
	if (!a && b) return 1;
	if (!b) return -1;
	int r = nde_wcsicmp(a, b);
	return min(max(r, -1), 1);
}

int mywcsicmp_fn(const wchar_t *a, const wchar_t *b)
{
	if (!a && !b) return 0;
	if (!a && b) return 1;
	if (!b) return -1;
	int r = nde_wcsicmp_fn(a, b);
	return min(max(r, -1), 1);
}

int nde_wcsicmp_fn(const wchar_t *a, const wchar_t *b)
{
	return CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, a, -1, b, -1) - 2;
}

bool nde_fnbegins(const wchar_t *a, const wchar_t *b)
{
	int index = findNLSString(LOCALE_USER_DEFAULT, FIND_STARTSWITH|NORM_IGNORECASE, a, -1, b, -1, 0);
	return (index != -1);
}

bool nde_fnends(const wchar_t *a, const wchar_t *b)
{
	int index = findNLSString(LOCALE_USER_DEFAULT, FIND_ENDSWITH|NORM_IGNORECASE, a, -1, b, -1, 0);
	return (index != -1);
}

bool nde_fncontains(const wchar_t *a, const wchar_t *b)
{
	int index = findNLSString(LOCALE_USER_DEFAULT, FIND_FROMSTART|NORM_IGNORECASE, a, -1, b, -1, 0);
	return index != -1;
}

#endif

#ifdef __ANDROID__

//---------------------------------------------------------------------------
// a faster way of doing min(wcslen(str), _len)
size_t nde_strnlen(const char *str, size_t _len)
{
	size_t len = 0;
	while (str && *str++)
	{
		if (_len == len)
			return len;
		len++;
	}
	return len;
}

// len must be <= strlen(b)
int nde_strnicmp(const char *a, const char *b, size_t len)
{
	return strncasecmp(a,b,len);
}

int nde_strnicmp_ignore(const char *a, const char *b, size_t len)
{
	return strncasecmp(a,b,len);
}

char *stristr(const char *s1, const char *s2)
{
	size_t s2len = strlen(s2);
	const char *p;
	for (p = s1;*p;p++)
		if (!nde_strnicmp(p, s2, s2len))
			return (char *)p;
	return NULL;
}

char *stristr_ignore(const char *s1, const char *s2)
{
	size_t s2len = strlen(s2);
	const char *p;
	for (p = s1;*p;p++)
		if (!nde_strnicmp_ignore(p, s2, s2len))
			return (char *)p;
	return NULL;
}

//---------------------------------------------------------------------------

int nde_stricmp(const char *a, const char *b)
{
	return strcasecmp(a,b);
}

int nde_stricmp_ignore(const char *a, const char *b)
{
	return strcasecmp(a,b); // TODO: maybe strcoll?
}

int mystricmp(const char *a, const char *b)
{
	if (!a && !b) return 0;
	if (!a && b) return 1;
	if (!b) return -1;
	int r = nde_stricmp(a, b);
	return min(max(r, -1), 1);
}

char* mystristr(const char *a, const char *b)
{
	return (!a || !b) ? NULL : stristr(a, b);
}

char* mystristr_fn(const char *a, const char *b)
{
	return (!a || !b) ? NULL : stristr_fn(a, b);
}

int mystricmp_fn(const char *a, const char *b)
{
	if (!a && !b) return 0;
	if (!a && b) return 1;
	if (!b) return -1;
	int r = nde_stricmp_fn(a, b);
	return min(max(r, -1), 1);
}

char *stristr_fn(const char *s1, const char *s2)
{
	size_t s2len = strlen(s2);
	const char *p;
	for (p = s1;*p;p++)
		if (!nde_strnicmp_fn(p, s2, s2len))
			return (char *)p;
	return NULL;
}

int nde_stricmp_fn(const char *a, const char *b)
{
	return strcasecmp(a,b);
}

int nde_strnicmp_fn(const char *a, const char *b, size_t len)
{
	return strncasecmp(a,b, len);

}

static uint16_t swap_utf16LE(uint16_t value)
{
#ifdef BIG_ENDIAN
	return (value >> 8) | (value << 8);
#else
	return value;
#endif
}

static uint16_t swap_utf16BE(uint16_t value)
{
#ifdef LITTLE_ENDIAN
	return (value >> 8) | (value << 8);
#else
	return value;
#endif
}

static size_t utf16LE_to_ucs4_character(const uint16_t *utf16_string, size_t len, uint32_t *codepoint)
{
	uint16_t lead = swap_utf16LE(utf16_string[0]);
	if (lead < 0xD800 || lead >= 0xE000)
	{
		return lead;
	}

	if (lead < 0xDC00)
	{
		if (len >= 2)
		{
			uint16_t trail = swap_utf16LE(utf16_string[1]);
			if (trail >= 0xDC00 && trail < 0xE000)
			{
				*codepoint = 0x10000 + ((lead - 0xD800) << 10) + (trail - 0xDC00);
				return 2;
			}
		}
	}

	*codepoint=0xFFFD; // invalid
	return 1;
}

static size_t utf16BE_to_ucs4_character(const uint16_t *utf16_string, size_t len, uint32_t *codepoint)
{
	uint16_t lead = swap_utf16LE(utf16_string[0]);
	if (lead < 0xD800 || lead >= 0xE000)
	{
		return lead;
	}

	if (lead < 0xDC00)
	{
		if (len >= 2)
		{
			uint16_t trail = swap_utf16LE(utf16_string[1]);
			if (trail >= 0xDC00 && trail < 0xE000)
			{
				*codepoint = 0x10000 + ((lead - 0xD800) << 10) + (trail - 0xDC00);
				return 2;
			}
		}
	}

	*codepoint=0xFFFD; // invalid
	return 1;
}

static size_t ucs4count(uint32_t codepoint)
{
	if (codepoint < 0x80)
		return 1;
	else if (codepoint < 0x800)
		return 2;
	else if (codepoint < 0x10000)
		return  3;
	else if (codepoint < 0x200000)
		return  4;
	else if (codepoint < 0x4000000)
		return  5;
	else if (codepoint <= 0x7FFFFFFF)
		return  6;
	else
		return 0;
}

static size_t ucs4_to_utf8_character(char *target, uint32_t codepoint, size_t max)
{
	size_t count = ucs4count(codepoint);

	if (!count)
		return 0;

	if (count>max) return 0;

	if (target == 0)
		return count;

	switch (count)
	{
		case 6:
			target[5] = 0x80 | (codepoint & 0x3F);
			codepoint = codepoint >> 6;
			codepoint |= 0x4000000;
		case 5:
			target[4] = 0x80 | (codepoint & 0x3F);
			codepoint = codepoint >> 6;
			codepoint |= 0x200000;
		case 4:
			target[3] = 0x80 | (codepoint & 0x3F);
			codepoint = codepoint >> 6;
			codepoint |= 0x10000;
		case 3:
			target[2] = 0x80 | (codepoint & 0x3F);
			codepoint = codepoint >> 6;
			codepoint |= 0x800;
		case 2:
			target[1] = 0x80 | (codepoint & 0x3F);
			codepoint = codepoint >> 6;
			codepoint |= 0xC0;
		case 1:
			target[0] = codepoint;
	}

	return count;
}

size_t utf16LE_to_utf8(const uint16_t *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint=0xFFFD;
	size_t position=0;
	size_t characters_processed=0;

	if (!dst) // they just want the size
	{
		while (source_len)
		{
			characters_processed = utf16LE_to_ucs4_character(src, source_len, &codepoint);
			if (codepoint == 0xFFFD)
				break;

			if (!codepoint)
				break;

			source_len -= characters_processed;

			characters_processed = ucs4count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}

	while(source_len && position<out_len)
	{
		characters_processed = utf16LE_to_ucs4_character(src, source_len, &codepoint);
		if (codepoint == 0xFFFD)
			break;

		if (!codepoint)
			break;

		source_len -= characters_processed;

		characters_processed=ucs4_to_utf8_character(&dst[position], codepoint, out_len-position);
		if (!characters_processed)
			break;
		position+=characters_processed;
	}
	if (position<out_len) 
		dst[position]=0;
	return position;
}

size_t utf16BE_to_utf8(const uint16_t *src, size_t source_len, char *dst, size_t out_len)
{
	uint32_t codepoint=0xFFFD;
	size_t position=0;
	size_t characters_processed=0;

	if (!dst) // they just want the size
	{
		while (source_len)
		{
			characters_processed = utf16BE_to_ucs4_character(src, source_len, &codepoint);
			if (codepoint == 0xFFFD)
				break;

			if (!codepoint)
				break;

			source_len -= characters_processed;

			characters_processed = ucs4count(codepoint);
			if (!characters_processed)
				break;

			position+=characters_processed;
		}
		return position;
	}

	while(source_len && position<out_len)
	{
		characters_processed = utf16BE_to_ucs4_character(src, source_len, &codepoint);
		if (codepoint == 0xFFFD)
			break;

		if (!codepoint)
			break;

		source_len -= characters_processed;

		characters_processed=ucs4_to_utf8_character(&dst[position], codepoint, out_len-position);
		if (!characters_processed)
			break;
		position+=characters_processed;
	}
	if (position<out_len) 
		dst[position]=0;
	return position;
}
#endif