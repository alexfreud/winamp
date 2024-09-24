#include "functions.h"
#include "string.h"
#include "varlist.h"
#include <windows.h>
#include <stddef.h>
#include <shlwapi.h>
#include "api__tagz.h"
#include "resource.h"
#include <strsafe.h>
#define MAKEFUNC(X) static void X(size_t n_src, wchar_t **src,size_t *found_src, tagz_::string *out, VarList *vars)

static bool StringMatch(const wchar_t *string1, const wchar_t *string2)
{
	int comp = CompareStringW(LOCALE_USER_DEFAULT,  NORM_IGNORECASE | /*NORM_IGNOREKANATYPE |*/ NORM_IGNOREWIDTH, string1, -1, string2, -1);
	return comp == CSTR_EQUAL;
}

/* ------ Logic ------ */
#if 0
MAKEFUNC(And)
{
	if (n_src != 2)
		out->AddString("[INVALID $and SYNTAX]");
	else
		if (found_src[0] && found_src[1])
			out->AddString("true");

}

MAKEFUNC(Greater)
{
	if (n_src != 2)
		out->AddString("[INVALID $greater SYNTAX]");
	else
	{
		if (_wtoi(src[0])>_wtoi(src[1]))
			out->AddString("true");
	}

}
#endif
#if 0
MAKEFUNC(_StrCmp)
{
	if (n_src != 2)
		out->AddString("[INVALID $STRCMP SYNTAX]");
	else
	{
		int comp = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | /*NORM_IGNOREKANATYPE |*/ NORM_IGNOREWIDTH, string1, -1, string2, -1);
		return comp == CSTR_EQUAL;
	}
}
#endif

MAKEFUNC(IfStrEqual)
{
	if (n_src != 3)
		out->AddString(WASABI_API_LNGSTRINGW(IDS_INVALID_IF_SYNTAX));
	else
	{
		if (StringMatch(src[0], src[1]))
			out->AddString(src[2]);
	}
}

MAKEFUNC(IfStrNotEqual)
{
	if (n_src != 3)
		out->AddString(WASABI_API_LNGSTRINGW(IDS_INVALID_IF_SYNTAX));
	else
	{
		if (!StringMatch(src[0], src[1]))
			out->AddString(src[2]);
	}
}

MAKEFUNC(IfStrEqual2)
{
	if (n_src != 4)
		out->AddString(WASABI_API_LNGSTRINGW(IDS_INVALID_IF_SYNTAX));
	else
	{
		if (StringMatch(src[0], src[1]))
			out->AddString(src[2]);
		else
			out->AddString(src[3]);
	}
}

MAKEFUNC(If)
{
	if (n_src != 3)
		out->AddString(WASABI_API_LNGSTRINGW(IDS_INVALID_IF_SYNTAX));
	else
		out->AddString(src[found_src[0] ? 1 : 2]);
}

MAKEFUNC(If2)
{
	if (n_src != 2)
		out->AddString(WASABI_API_LNGSTRINGW(IDS_INVALID_IF_SYNTAX));
	else
		out->AddString(src[found_src[0] ? 0 : 1]);
}

MAKEFUNC(If3)
{
	if (!n_src)
		return;
	for (size_t i = 0; i != (n_src-1); i++)
	{
		if (found_src[i])
		{
			out->AddString(src[i]);
			return;
		}
	}
	out->AddString(src[n_src-1]);
}

MAKEFUNC(Decode)
{
	if (n_src == 0)
		return;

	for (size_t s=1;s!=n_src;s+=2)
	{
		if ((s + 1) == n_src) // last (default) parameter
		{
			out->AddString(src[s]);
			return;
		}

		if (StringMatch(src[0], src[s]))
		{
			out->AddString(src[s+1]); // last parameter check (see above) ensures that this is safe
			return;
		}
	}
}

MAKEFUNC(Select)
{
	size_t select = (src[0] ? _wtoi(src[0]) : 0);

	if (select<n_src)
	{
		out->AddString(src[select]);
	}
}

/* ------ Length ------ */
static int NumChars(const wchar_t *x)
{
	int count = 0;
	while (x && *x)
	{
		x = CharNext(x);
		count++;
	}
	return count;
}

MAKEFUNC(Iflonger)
{
	if (n_src != 4) out->AddString(WASABI_API_LNGSTRINGW(IDS_INVALID_IFLONGER_SYNTAX));
	else
	{
		if (NumChars(src[0]) > _wtoi(src[1]))
			out->AddString(src[2]);
		else
			out->AddString(src[3]);
	}
}

MAKEFUNC(PadLeft)
{
	if (n_src >= 2)
	{
		LPTSTR fill = L" ";
		if (n_src >= 3 && src[2][0])
			fill = src[2];
		int num = (src[1] ? _wtoi(src[1]) : 0);
		wchar_t *p = src[0];
		int strChars = NumChars(p);
		if (strChars<num)
		{
			size_t fl = lstrlen(fill);
			while (strChars != num)
			{
				out->AddChar(fill[(--num) % fl]);
			}
		}
		while (p && *p)
		{
			out->AddDBChar(p);
			p = CharNext(p);
		}
	}
}

MAKEFUNC(Pad)
{
	if (n_src >= 2)
	{
		LPTSTR fill = L" ";
		if (n_src >= 3 && src[2][0])
			fill = src[2];
		int num = (src[1] ? _wtoi(src[1]) : 0);
		LPTSTR p = src[0];
		while (p && *p)
		{
			out->AddDBChar(p);
			p = CharNext(p);
			num--;
		}
		size_t fl = lstrlen(fill);
		while (num > 0)
		{
			out->AddChar(fill[(--num) % fl]);
		}
	}
}

MAKEFUNC(Cut)
{
	if (n_src >= 2)
	{
		size_t num = (src[1] ? _wtoi(src[1]) : 0);
		LPTSTR p = src[0];
		while (p && *p && num)
		{
			out->AddDBChar(p);
			p = CharNext(p);
			num--;
		}
	}
}

// todo: benski> there's no way this works (fixed by DrO Mar 2009)
MAKEFUNC(Right)
{
	if (n_src >= 2)
	{
		size_t num = (src[1] ? _wtoi(src[1]) : 0);
		int offset = NumChars(src[0]) - (int)num;
		LPTSTR p = src[0];
		while (p && *p && offset--)
		{
			p = CharNext(p);
		}
		while (p && *p && num)
		{
			out->AddDBChar(p);
			p = CharNext(p);
			num--;
		}
	}
}

MAKEFUNC(PadCut)
{
	if (n_src >= 2)
	{
		LPTSTR fill = L" ";
		if (n_src >= 3 && src[2][0]) fill = src[3];
		size_t num = (src[1] ? _wtoi(src[1]) : 0);
		LPTSTR p = src[0];
		while (p && *p && num > 0)
		{
			out->AddDBChar(p);
			p = CharNext(p);
			num--;
		}
		size_t fl = lstrlen(fill);
		while (num > 0)
		{
			out->AddChar(fill[(--num) % fl]);
		}
	}
}

MAKEFUNC(Longest)
{
	LPTSTR ptr = 0;
	size_t m = 0;
	for (size_t n = 0;n != n_src;n++)
	{
		size_t l = NumChars(src[n]);
		if (l > m)
		{
			m = l;
			ptr = src[n];
		}
	}
	if (ptr)
		out->AddString(ptr);
}

MAKEFUNC(Shortest)
{
	LPTSTR ptr = 0;
	size_t m = (size_t)(-1);
	for (size_t n = 0;n != n_src;n++)
	{
		size_t l = NumChars(src[n]);
		if (l < m)
		{
			m = l;
			ptr = src[n];
		}
	}
	if (ptr)
		out->AddString(ptr);
}

MAKEFUNC(Len)
{
	if (n_src >= 1)
		out->AddInt(NumChars(src[0]));
}

/* ------ Case ------ */
static void DBUpper(LPTSTR x)
{
	LPTSTR end = CharNext(x);
	DWORD count = (DWORD)(end - x);
	CharUpperBuff(x, count);
}

static bool IsSpace(LPTSTR x)
{
	LPTSTR end = CharNext(x);
	int count = (int)(end - x);
	WORD charType = 0;
	GetStringTypeEx(LOCALE_USER_DEFAULT,
	                CT_CTYPE1,
	                x,
	                count,
	                &charType);

	return !!(charType & C1_SPACE);
}

static bool IsWordEnd(LPTSTR x)
{
	/* TODO: decide if we want to do this
	if (x == '\'')
		return false; // keeps words like "isn't" from capitalizing incorrectly, although it does break single quoted strings
		*/

	LPTSTR end = CharNext(x);
	int count = (int)(end - x);
	WORD charType = 0;
	GetStringTypeEx(LOCALE_USER_DEFAULT,
	                CT_CTYPE1,
	                x,
	                count,
	                &charType);

	return !!(charType & C1_SPACE) || !!(charType & C1_PUNCT);
}

static int separator(TCHAR x)
{
	if (!x
	    || x == L' '
	    || x == L'\t') return 1;
	if (x == L'\'' || x == L'_') return 0;
	return 0;
}

MAKEFUNC(Upper)
{
	if (n_src >= 1)
	{
		CharUpper(src[0]);
		out->AddString(src[0]);
	}
}

MAKEFUNC(Lower)
{
	if (n_src >= 1)
	{
		CharLower(src[0]);
		out->AddString(src[0]);
	}
}

MAKEFUNC(Caps)
{
	if (n_src < 1) return ;

	int firstLetter = 1;
	CharLower(src[0]);
	LPTSTR sp = src[0];
	while (sp && *sp)
	{
		bool isSep = IsWordEnd(sp);
		if (!isSep && firstLetter)
			DBUpper(sp);

		sp = CharNext(sp);
		firstLetter = isSep;
	}
	out->AddString(src[0]);
}

MAKEFUNC(Caps2)
{
	if (n_src < 1) return ;

	int firstLetter = 1;
	LPTSTR sp = src[0];
	while (sp && *sp)
	{
		int isSep = IsWordEnd(sp);
		if (!isSep && firstLetter)
			DBUpper(sp);

		sp = CharNext(sp);
		firstLetter = isSep;

	}
	out->AddString(src[0]);
}

/* ------ Numbers ------ */

MAKEFUNC(Num)
{
	if (n_src == 2)
	{
		wchar_t tmp[16] = {0};
		StringCbPrintfW(tmp, sizeof(tmp), L"%0*u", (src[1] ? _wtoi(src[1]) : 0), (src[0] ? _wtoi(src[0]) : 0));
		out->AddString(tmp);
	}
}

MAKEFUNC(Hex)
{
	if (n_src == 2)
	{
		wchar_t tmp[16] = {0};
		StringCbPrintfW(tmp, sizeof(tmp), L"%0*x", (src[1] ? _wtoi(src[1]) : 0), (src[0] ? _wtoi(src[0]) : 0));
		out->AddString(tmp);
	}
}

/* ------ Time ------ */


MAKEFUNC(SysTime_year)
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	wchar_t tmp[16] = {0};
	StringCbPrintfW(tmp, sizeof(tmp), L"%u",st.wYear);
	out->AddString(tmp);
}

MAKEFUNC(SysTime_month) //these are for stream saving, time in filenames
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	wchar_t tmp[16] = {0};
	StringCbPrintfW(tmp, sizeof(tmp), L"%02u",st.wMonth);
	out->AddString(tmp);
}

MAKEFUNC(SysTime_day)
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	wchar_t tmp[16] = {0};
	StringCbPrintfW(tmp, sizeof(tmp), L"%02u",st.wDay);
	out->AddString(tmp);
}

MAKEFUNC(SysTime_hour)
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	wchar_t tmp[16] = {0};
	StringCbPrintfW(tmp, sizeof(tmp), L"%02u",st.wHour);
	out->AddString(tmp);
}

MAKEFUNC(SysTime_minute)
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	wchar_t tmp[16] = {0};
	StringCbPrintfW(tmp, sizeof(tmp), L"%02u",st.wMinute);
	out->AddString(tmp);
}

MAKEFUNC(SysTime_second)
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	wchar_t tmp[16] = {0};
	StringCbPrintfW(tmp, sizeof(tmp), L"%02u",st.wSecond);
	out->AddString(tmp);
}

/* ------ Math ------ */
MAKEFUNC(Ifgreater)
{
	if (n_src!=4)
		out->AddString(WASABI_API_LNGSTRINGW(IDS_INVALID_IFGREATER_SYNTAX));
	else
	{
		if ((src[0] ? _wtoi(src[0]) : 0) > (src[1] ? _wtoi(src[1]) : 0))
			out->AddString(src[2]);
		else
			out->AddString(src[3]);
	}
}

MAKEFUNC(Add)
{
	int s=0;
	for (size_t n=0;n!=n_src;n++)
	{
		s += (src[n] ? _wtoi(src[n]) : 0);
	}
	out->AddInt(s);
}

MAKEFUNC(Sub)
{
	if (n_src>=1)
	{
		int s = (src[0] ? _wtoi(src[0]) : 0);
		for (size_t n=1;n!=n_src;n++)
		{
			s -= (src[n] ? _wtoi(src[n]) : 0);
		}
		out->AddInt(s);
	}
}

MAKEFUNC(Mul)
{
	int s=1;
	for (size_t n=0;n!=n_src;n++)
	{
		s *= (src[n] ? _wtoi(src[n]) : 1);
	}
	out->AddInt(s);
}

MAKEFUNC(Div)
{
	if (n_src>=1)
	{
		int s = (src[0] ? _wtoi(src[0]) : 0);
		for (size_t n = 1; n != n_src; n++)
		{
			int t = (src[n] ? _wtoi(src[n]) : 0);
			if (t) s /= t;
			else t = 0;
		}
		out->AddInt(s);
	}
}

MAKEFUNC(Mod)
{
	if (n_src>=1)
	{
		int s = (src[0] ? _wtoi(src[0]) : 0);
		for (size_t n = 1; n != n_src; n++)
		{
			int t = (src[n] ? _wtoi(src[n]) : 0);
			if (t) s %= t;
			else t = 0;
		}
		out->AddInt(s);
	}
}

MAKEFUNC(_MulDiv)
{
	if (n_src == 3)
	{
		out->AddInt(MulDiv(_wtoi(src[0]), _wtoi(src[1]), _wtoi(src[2])));
	}
}

MAKEFUNC(Max)
{
	if (!n_src) return;
	int m = (src[0] ? _wtoi(src[0]) : 0);
	for (size_t n = 1; n < n_src; n++)
	{
		int t = (src[n] ? _wtoi(src[n]) : 0);
		if (t > m) m = t;
	}
	out->AddInt(m);
}

MAKEFUNC(Min)
{
	if (!n_src) return;
	int m = (src[0] ? _wtoi(src[0]) : 0);
	for (size_t n = 1; n < n_src; n++)
	{
		int t = (src[n] ? _wtoi(src[n]) : 0);
		if (t < m) m = t;
	}
	out->AddInt(m);
}

/* ------ Path ------ */
MAKEFUNC(PathSafe)
{
	LPTSTR p=src[0];
	while (p && *p)
	{
		if (*p <= 31 && *p >= 0)
		{
			// we'll just skip these characters
		}
		else switch (*p)
		{
			case L'?':
			case L'*':
			case  L'|':
				out->AddDBChar(L"_");
				break;
			case '/':
			case L'\\':
			case L':':
				out->AddDBChar(L"-");
				break;
			case L'\"': 
				out->AddDBChar(L"\'");
				break;
			case L'<':
				out->AddDBChar(L"(");
				break;
			case L'>': 
				out->AddDBChar(L")");
				break;
			default:
				out->AddDBChar(p);
		}
		p = CharNext(p);
	}
}

MAKEFUNC(FileName)
{
	if (n_src < 1) return ;
	out->AddString(PathFindFileName(src[0]));
}

MAKEFUNC(FilePart)
{
	if (n_src < 1) return;
	LPTSTR beg = PathFindFileName(src[0]);
	LPTSTR end = PathFindExtension(beg);
	while (beg != end)
	{
		out->AddChar(*beg++);
	}
}

MAKEFUNC(PathRTrim)
{
	if (n_src < 1) return;
	int cut=1;
	if (n_src>=2)
		cut = (src[1] ? _wtoi(src[1]) : 0);
	if (cut<0)
		cut=0;
	wchar_t folder[MAX_PATH] = {0};
	lstrcpyn(folder, src[0], MAX_PATH);
	while (cut--)
	{
		PathRemoveFileSpec(folder);
		PathRemoveBackslash(folder);
	}

	out->AddString(folder);
}

MAKEFUNC(PathLPart)
{
	if (n_src < 1) return;
	int cut=1;
	if (n_src>=2)
		cut = (src[1] ? _wtoi(src[1]) : 0);
	if (cut<0)
		cut=0;
	wchar_t *p = src[0];
	while (cut-- && p)
	{
		p = PathFindNextComponent(p);
	}
	if (p)
		*p = 0;
	PathRemoveBackslash(src[0]);
	out->AddString(src[0]);
}

MAKEFUNC(PathRPart)
{
	if (n_src < 1) return;
	int cut=1;
	if (n_src>=2)
		cut = (src[1] ? _wtoi(src[1]) : 0);
	if (cut<0)
		cut=0;
	wchar_t temp[MAX_PATH] = {0};
	lstrcpyn(temp, src[0], MAX_PATH);
	while (cut--)
	{
		PathRemoveBackslash(temp);
		PathRemoveFileSpec(temp);
		PathAddBackslash(temp);
	}

	out->AddString(&(src[0][lstrlenW(temp)]));
}

MAKEFUNC(PathLTrim)
{
	if (n_src < 1) return;
	int cut=1;
	if (n_src>=2)
		cut = (src[1] ? _wtoi(src[1]) : 0);
	if (cut<0)
		cut=0;
	wchar_t *p = src[0];
	while (cut-- && p)
	{
		p = PathFindNextComponent(p);
	}
	if (p)
		out->AddString(p);
}

MAKEFUNC(FileExt)
{
	if (n_src < 1) return ;
	LPTSTR ext = PathFindExtension(src[0]);
	if (ext) ext = CharNext(ext);
	out->AddString(ext);
}

/* ------ Strings ------ */

static wchar_t roman_num[]=
{
	L'I',L'V',L'X',L'L',L'C',L'D',L'M'
};

static bool IsRoman(LPTSTR ptr)//could be more smart i think
{
	if (ptr[0]==']' && ptr[1]=='[' && IsSpace(ptr+2))
		return true;
	while (!IsSpace(ptr))
	{
		bool found = 0;
		for (size_t n = 0; n < TABSIZE(roman_num); n++)
		{
			if (*ptr==roman_num[n])
			{
				found=1;
				break;
			}
		}
		if (!found)
			return false;
		ptr=CharNext(ptr);
	}
	return true;
}

static bool IsDigit(LPTSTR x)
{
	LPTSTR end = CharNext(x);
	int count = (int)(end - x);
	WORD charType = 0;
	GetStringTypeEx(LOCALE_USER_DEFAULT,
	                CT_CTYPE1,
	                x,
	                count,
	                &charType);

	return !!(charType & C1_DIGIT);
}

// TODO: what's this do exactly???
static bool sepcmp(LPTSTR src, LPTSTR val)
{
	int l=lstrlen(val);
	return !StrCmpNI(src,val,l) && IsSpace(&src[l]);
}

static bool need_full(LPTSTR ptr)
{
	if (IsRoman(ptr))
		return true;
	if (sepcmp(ptr, L"RPG"))
		return true;
	while (ptr && !IsSpace(ptr))
	{
		if (!ptr || !*ptr || !IsDigit(ptr))
			return false;
		ptr = CharNext(ptr);
	}
	return true;
}

static bool CharMatch(LPTSTR string1, LPTSTR string2)
{
	LPTSTR end1 = CharNext(string1);
	LPTSTR end2 = CharNext(string2);

	int count1 = (int)(end1 - string1);
	int count2 = (int)(end2 - string2);
	if (count1 == count2)
	{
		int comp = CompareString(LOCALE_USER_DEFAULT,  /*NORM_IGNORECASE |*/ /*NORM_IGNOREKANATYPE |*/ NORM_IGNOREWIDTH, string1, count1, string2, count2);
		return comp == CSTR_EQUAL;
	}
	else
		return false;
}

MAKEFUNC(_StrChr)
{
	if (n_src == 2)
	{
		LPTSTR p = src[0];
		LPTSTR s = src[1];
		while (p && *p)
		{
			if (CharMatch(p, s))
			{
				out->AddInt(1 + (int)(p - src[0]));
				return ;
			}
			p = CharNext(p);
		}

		out->AddChar('0');
	}
}

MAKEFUNC(_StrRChr)
{
	if (n_src == 2)
	{
		LPTSTR p = src[0], p1 = p;
		LPTSTR s = src[1];

		while (p && *p)
		{
			p1 = p;
			p = CharNext(p);
		}

		while (1)
		{
			if (CharMatch(p1, s))
			{
				out->AddInt(1 + (int)(p1 - src[0]));
				return ;
			}
			if (src[0] == p1)
				break;
			p1 = CharPrev(src[0], p1);
		}
		out->AddChar(L'0');
	}
}

MAKEFUNC(_StrStr)
{
	// TODO: not multi-byte or unicode correct, but we'll deal for now
	if (n_src == 2)
	{
		LPTSTR p = StrStr(src[0], src[1]);
		if (p)
			out->AddInt(1 + (int)(p - src[0]));
		else
			out->AddChar(L'0');
	}
}

MAKEFUNC(SubStr)
{
	int n1, n2;
	if (n_src < 2)
		return ;
	n1 = (src[1] ? _wtoi(src[1]) : 0);
	if (n_src >= 3)
	{
		n2 = (src[2] ? _wtoi(src[2]) : 0);
	}
	else
		n2 = n1;
	if (n1 < 1)
		n1 = 1;
	if (n2 >= n1)
	{
		n1--;
		n2--;
		LPTSTR p=src[0];
		while (n1 <= n2 && p && *p)
		{
			out->AddDBChar(p);
			p = CharNext(p);
		}
	}
}

MAKEFUNC(Split)
{
	if (n_src!=3 || !src[0])
		return;

	LPTSTR p=src[0];
	LPTSTR split=src[1];
	int which = (src[2] ? _wtoi(src[2]) : 0);
	if (which)
	{
		LPCTSTR where = wcsstr(p, split);
		if (where)
		{
			where += wcslen(split);
			out->AddString(where);
		}
	}
	else
	{
		LPCTSTR where = wcsstr(p, split);
		if (where)
		{
			while (p != where)
				out->AddChar(*p++);
		}
		else
			out->AddString(p);
	}
}

MAKEFUNC(Replace)
{
	if (n_src<3 || !src[0])
		return;

	LPTSTR p=src[0];
	while (p && *p)
	{
		LPTSTR p2 = p;
		LPTSTR src1 = src[1];

		while (src1 && *src1 && CharMatch(src1, p2))
		{
			p2 = CharNext(p2);
			src1 = CharNext(src1);
		}

		if (!*src1)
		{
			out->AddString(src[2]);
			p = p2;
		}
		else
		{
			out->AddDBChar(p);
			p = CharNext(p);
		}
	}
}

MAKEFUNC(Trim)
{
	LPTSTR stringStart = src[0];

	// trim from beginning
	while (IsSpace(stringStart))
		stringStart = CharNext(stringStart);

	// trim from end
	LPTSTR stringLast = stringStart+lstrlen(stringStart)+1;
	stringLast=CharPrev(stringStart, stringLast); // have to go back from the null terminator in case the last char is multibyte

	while (IsSpace(stringLast))
	{
		stringLast= CharPrev(stringStart, stringLast);
	}
	stringLast = CharNext(stringLast);
	while (stringStart != stringLast)
	{
		out->AddChar(*stringStart++);
	}
}

/* ------ Generators ------ */

MAKEFUNC(Tab)
{
	int num=1;
	if (n_src == 1)
		num = (src[0] ? _wtoi(src[0]) : 0);
	while (num--)
		out->AddChar(L'\t');
}

MAKEFUNC(Crlf)
{
	out->AddString(L"\r\n");
}

MAKEFUNC(Char)
{
	if (n_src != 1)
		return;

	wchar_t wide[2]={ (wchar_t)(src[0] ? _wtoi(src[0]) : 0),(wchar_t)0};
	out->AddString(wide);
	/* below doesn't seem to work.  not sure why
	wchar_t wide[5]={0,}; // 5 words just in case we go outside the BMP
	int ucs4[2] = {0,0}; // incoming value is encoded in UCS-4 (little endian)
	ucs4[0] = _wtoi(src[0]);
	int x = MultiByteToWideChar(12000, 0, (char*)ucs4, 4, wide, 5); // convert from UCS-4 to UTF-16
	int error = GetLastError();
	*/
}

MAKEFUNC(Repeat)
{
	if (n_src!=2)
		return;

	LPTSTR fillChar=src[0];
	int fillCount=(src[1] ? _wtoi(src[1]) : 0);
	if (fillCount<0)
		return;
	while (fillCount--)
	{
		out->AddString(fillChar);
	}
}

/* ------ Misc ------ */

MAKEFUNC(Abbr)
{
	//abbr(string,len)
	if (n_src == 0 || n_src > 2)
		return;

	if (n_src==2 && NumChars(src[0]) < (src[1] ? _wtoi(src[1]) : 0))
	{
		out->AddString(src[0]);
		return;
	}
	LPTSTR meta=src[0];
	bool w=0,r=0;
	while (meta && *meta)
	{
		bool an=!IsSpace(meta) || *meta==']' || *meta=='[';
		if (w && !an)
		{
			w=0;
		}
		else if (!w && an)
		{
			w=1;
			r=need_full(meta)?1:0;
			out->AddDBChar(meta);
		}
		else if (w && r)
		{
			out->AddDBChar(meta);
		}
		meta=CharNext(meta);
	}
}

MAKEFUNC(Get)
{
	if (n_src >= 1)
	{
		wchar_t *p = vars->Get(src[0]);
		if (p)
			out->AddString(p);
	}
}

MAKEFUNC(Put)
{
	if (n_src >= 2)
	{
		vars->Put(src[0], src[1]);
		out->AddString(src[1]);
	}
}

MAKEFUNC(PutQ)
{
	if (n_src >= 2)
	{
		vars->Put(src[0], src[1]);
	}
}

MAKEFUNC(Null)
{
}

MAKEFUNC(Directory)
{
	int numLevels=1;
	if (n_src == 2)
		numLevels = (src[1] ? _wtoi(src[1]) : 0);

	wchar_t folder[MAX_PATH] = {0};
	lstrcpyn(folder, (src[0] ? src[0] : L""), MAX_PATH);
	while (numLevels--)
	{
		PathRemoveFileSpec(folder);
		PathRemoveBackslash(folder);
	}
	PathStripPath(folder);
	out->AddString(folder);
}

// TODO: order these or sort these, and use a binary search
TextFunction FUNCS[]=
{
	//	Blah,"blah",
	//	Nop,"nop",
	If,L"if",
	If2,L"if2",
	If3, L"if3",
	Upper,L"upper",
	Lower,L"lower",
	Pad,L"pad",
	Cut,L"cut",
	Cut,L"left",
	PadCut,L"padcut",
	Abbr,L"abbr",
	FilePart,L"filepart",
	FilePart,L"filename",
	FileExt,L"fileext",
	FileExt,L"ext",
	PathLTrim,L"PathLTrim",
	PathLPart,L"PathLPart",
	PathRTrim,L"PathRTrim",
	PathRPart,L"PathRPart",
	Caps,L"caps",
	Caps2,L"caps2",
	Longest,L"longest",
	Shortest,L"shortest",
	Iflonger,L"iflonger",
	Ifgreater,L"ifgreater",
	Num,L"num",Num,L"dec",
	Hex,L"hex",
	_StrChr,L"strchr",
	_StrChr,L"strlchr",
	_StrRChr,L"strrchr",
	_StrStr,L"strstr",
	SubStr,L"substr",
	Len,L"len",
	Add,L"add",
	Sub,L"sub",
	Mul,L"mul",
	Div,L"div",
	Mod,L"mod",
	_MulDiv, L"muldiv",
	FileName,L"filename",
	Min,L"min",
	Max,L"max",
	Get,L"get",
	Put,L"put",
	PutQ,L"puts",
	Null,L"null",
	SysTime_year,L"systime_year",
	SysTime_month,L"systime_month",
	SysTime_day,L"systime_day",
	SysTime_hour,L"systime_hour",
	SysTime_minute,L"systime_minute",
	SysTime_second,L"systime_second",
	Replace,L"replace",
	Repeat, L"repeat",
	PadLeft, L"lpad",
	IfStrEqual, L"IfStrEqual",
	IfStrEqual2, L"IfStrEqual2",
	IfStrNotEqual, L"IfStrNotEqual",
	Decode, L"decode",
	//And, L"and",
	//Greater,L"greater",
	Char, L"char",
	Crlf, L"crlf",
	Directory, L"directory",
	Right, L"right",
	Tab, L"tab",
	Trim, L"trim",
	Select, L"select",
	PathSafe, L"pathsafe",
	Split, L"split",
	0,0,
};