#include <bfc/wasabi_std.h>
#include <bfc/std_mem.h>
#include <bfc/nsguid.h>
#include "StringW.h"
#ifdef _WIN32
#include <shlwapi.h>
#endif

StringW::StringW(const wchar_t *initial_val)
		: val(NULL)
{
	setValue(initial_val);
}

StringW::StringW(const StringW &s)
		: val(NULL)
{
	if (s == NULL) setValue(NULL);
	else setValue(s.getValue());
}

StringW::StringW(const StringW *s)
		: val(NULL)
{
	if (s == NULL) setValue(NULL);
	else setValue(s->getValue());
}

StringW::~StringW()
{
	FREE(val);
	val = NULL;
}

int StringW::replaceNumericField(int value, wchar_t fieldchar)
{
	if (val == NULL || *val == '\0') return 0;
	int nrep = 0;
	for (const wchar_t *p = val; *p; p++)
	{
		if (*p == fieldchar) nrep++;
		else if (nrep) break;
	}
	if (nrep == 0) return 0;	// no field found
	StringW rc;
	wchar_t fc[2] = { 0, 0 };
	fc[0] = fieldchar;
	for (int i = 0; i < nrep; i++) rc.cat(fc);
	StringPrintfW fmt(L"%%0%0dd", nrep);
	StringPrintfW replacement(fmt.getValue(), value);
	return replace(rc, replacement);
}

// Returns index of first found, -1 if not found.
size_t StringW::lFindChar(wchar_t findval)
{
	size_t length = len();
	for (size_t i = 0; i != length; i++)
	{
		if (val[i] == findval)
		{
			return i;
		}
	}
	return -1;
}

const wchar_t *StringW::setValue(const wchar_t *newval)
{
	if (newval != val)
	{
		if ((unsigned long long)newval == NULL || ((unsigned long long)newval <= 65535))
		{
			FREE(val);
			val = NULL;
		}
		else
		{
			if (val)
			{
				size_t len = wcslen(newval);
				if (val != NULL)
#ifdef STRING_REALLOC_OPTIMS
				{
					size_t oldlen = wcslen(val);
					// if smaller but greater than half previous size, don't realloc
					if (len > oldlen || len < oldlen / 2)
						val = (wchar_t *)REALLOC(val, sizeof(wchar_t) * (len + 1));
				}
#else
					val = (wchar_t *)REALLOC(val, sizeof(wchar_t) * (len + 1));
#endif
				else
					val = WMALLOC(len + 1);
				ASSERT(newval != NULL);
				MEMCPY_(val, newval, sizeof(wchar_t)*(len + 1));
			}
			else
				val = WCSDUP(newval);
		}
	}
	return getValue();
}

int StringW::isequal(const wchar_t *otherval) const
{
	// TODO: benski> move to WCSCMPSAFE
	if (!otherval)
		otherval = L"";
	if (!getValue())
		return !wcscmp(L"", otherval);
	else
		return !wcscmp(getValue(), otherval);
}

int StringW::iscaseequal(const wchar_t *otherval) const
{
	return !WCSICMPSAFE(getValue(), otherval);
}

int StringW::isempty() const
{
	return (!val || !*val);
}

void StringW::toupper()
{
	if (!isempty())
	{
#ifdef _WIN32
		CharUpperW(val);
#else
		wchar_t *itr = val;
		while (itr && *itr)
		{
			*itr = ::towupper(*itr);
			itr++;
		}
#endif
	}
}

void StringW::tolower()
{
	if (!isempty())
	{
#ifdef _WIN32
		CharLowerW(val);
#else
		wchar_t *itr = val;
		while (itr && *itr)
		{
			*itr = ::towlower(*itr);
			itr++;
		}
#endif
	}
}

const wchar_t *StringW::getValueSafe(const wchar_t *def_val) const
{
	if (val == NULL)
		return def_val;
	else
		return val;
}

const wchar_t *StringW::cat(const wchar_t *value)
{
	if (value == NULL || *value == 0)
		return getValue();
	if (val == NULL)
		return setValue(value);
	return catn(value, wcslen(value));
}

const wchar_t *StringW::catn(const wchar_t *value, size_t len)
{
	if (len == 0) return val;
	if (value == NULL || *value == 0) return getValue();
	if (val == NULL) return ncpy(value, len);
	size_t ol = wcslen(val);
	val = (wchar_t *)REALLOC(val, sizeof(wchar_t) * (ol + len + 1));
	val[ol + len] = 0;
	wcsncpy(val + ol, value, len);
	return val;
}

// replaces string with n chars of val or length of val, whichever is less.
const wchar_t *StringW::ncpy(const wchar_t *newstr, size_t numchars)
{
	val = (wchar_t *)REALLOC(val, sizeof(wchar_t) * (numchars + 1));
	val[numchars] = 0;
	wcsncpy(val, newstr, numchars);
	return getValue();
}

// swaps buffers with another string
void StringW::swap(StringW *swapper)
{
	wchar_t *tempChar = swapper->val;
	swapper->val = val;
	val = tempChar;
}

void StringW::swap(StringW &swapper) // swaps buffers with another string
{
	wchar_t *tempChar = swapper.val;
	swapper.val = val;
	val = tempChar;
}

// take ownership of a buffer
void StringW::own(wchar_t *swapper)
{
	if (val)
		FREE(val);
	val = swapper;
}

const wchar_t *StringW::catPostSeparator(const wchar_t *value, const wchar_t separator)
{
	if (value == NULL || *value == 0 || separator == 0) return getValue();
	size_t oldLen = val ? wcslen(val) : 0;
	size_t newLen = wcslen(value);
	val = (wchar_t *)REALLOC(val, sizeof(wchar_t) * (oldLen + newLen + 1 + 1)); // +1 for separator, +1 for null character
	wcsncpy(val + oldLen, value, newLen + 1);
	val[oldLen + newLen] = separator; // add the separator
	val[oldLen + newLen + 1] = 0; // null terminate
	return val;
}

size_t StringW::va_sprintf(const wchar_t *format, va_list args)
{
	if (!format) return 0;

	va_list saveargs = args;
	// roughly evaluate size of dest string
	const wchar_t *p = format;
	size_t length = 0;
	while (p && *p)
	{
		if (*(p++) != '%') length++;
		else
		{
			void *arg = va_arg(args, void *);
			for (;;)
			{
				const wchar_t f = *p++;
				if (f == 'c') length++;
				else if (f == 'i') length += 16;
				else if (f == 'u') length += 16;
				else if (f == 'f') length += 64;
				else if (f == 'd' || f == 'f') length += 64;
				else if (f == 'x') length += 32; // Hex with LC Alphas: 0x0009a64c
				else if (f == 'X') length += 32; // Hex with UC Alphas: 0x0009A64C
				else if (f == 's')
				{ // ::vsrintf can properly handle null.
					if (arg == NULL)
					{
						length += wcslen(L"(null)"); // Just to be explicit.
					}
					else
					{
						length += wcslen((const wchar_t *)arg);
					}
				}
				else if (f == 'S') // uppercase S mean narrow string
				{ // ::vsrintf can properly handle null.
					if (arg == NULL)
					{
						length += STRLEN("(null)"); // Just to be explicit.
					}
					else
					{
						length += STRLEN((const char *)arg);
					}
				}
				else if (ISDIGIT(f)) continue;
				else if (f == '.') continue;
				else if (f == '%') length++;
				else ASSERTPR(0, "undefined format passed to stringprintf!");
				break;
			}
		}
	}
	if (val)
	{
		if (len() < length)
			val = (wchar_t *)REALLOC(val, sizeof(wchar_t) * (length + 1));
	}
	else
		val = WMALLOC(length + 1);

	// now write the string in val
#ifdef _WIN32
	size_t remain;
	StringCchVPrintfExW(val, length+1, 0, &remain, 0, format, saveargs);
	return length-remain;
#elif defined(__APPLE__)
  int real_len = ::vswprintf(val, length+1, format, saveargs);
  ASSERTPR(real_len <= (int)length, "String.printf overflow");
  return real_len;
#endif

}

size_t StringW::len() const
{
	return (val == NULL) ? 0 : wcslen(val);
}

void StringW::trunc(int newlen)
{
	if (val)
	{
		int oldlen = (int)wcslen(val);
		if (newlen < 0) newlen = MAX(oldlen + newlen, 0);
		int trimLen = MIN(oldlen, newlen);
		val[trimLen] = 0;
	}
}

int StringW::lastChar()
{
	if (isempty()) return -1;
	return val[len() - 1];
}

const wchar_t *StringW::prepend(const wchar_t *value)
{
	if (value == NULL || *value == 0) return getValue();
	if (val == NULL) return setValue(value);
	StringPrintfW temp(L"%s%s", value, getValue());
	swap(&temp);
	return getValue();
}

int StringW::replace(const wchar_t *find, const wchar_t *replace)
{
	if (len() == 0 || find == NULL || replace == NULL) return 0;

	int find_count = 0;

	wchar_t *p, *p2;
	size_t rep_len = wcslen(replace);
	size_t find_len = wcslen(find);
	ptrdiff_t size_diff = rep_len - find_len;

	if ( size_diff == 0 )
	{
		p = val;
		while ( p = wcsstr( p, find ) )
		{
			wcsncpy( p, replace, rep_len );
			p += find_len;
			find_count++;
		}
	}
	else
	{
		wchar_t *new_buf, *in;

		p = val;
		while ( p = wcsstr( p, find ) )
		{
			find_count++;
			p += find_len;
		}

		int length = (int)( len() + find_count * size_diff + 1 );
		new_buf = (wchar_t *)MALLOC(sizeof(wchar_t) * length);

		p = val;
		in = new_buf;
		while ( p2 = wcsstr( p, find ) )
		{
			wcsncpy( in, p, p2 - p );
			in += p2 - p;
			wcsncpy( in, replace, rep_len );
			in += rep_len;
			p = p2 + find_len;
		}
		wcscpy( in, p );
		new_buf[ len() + find_count * size_diff ] = 0;

		// just swap buffers

		FREE(val);
		val = new_buf;
	}
	return find_count;
}

const wchar_t *StringW::printf(const wchar_t *format, ...)
{
	va_list args;
	va_start (args, format);
	va_sprintf(format, args);
	va_end(args);
	return getValue();
}

void StringW::AppendPath(const wchar_t *path)
{
	FixSlashes();
	if (val)
	{
#ifdef _WIN32
		wchar_t temp[MAX_PATH] = {0};
		PathCombineW(temp, val, path);
		setValue(temp);
#else
#warning find a better way
	    wchar_t temp[PATH_MAX] = {0};
		swprintf(temp, PATH_MAX, L"%s%s", val, path);
		setValue(temp);
#endif
	}
	else
		setValue(path);
}

void StringW::AppendFolder(const wchar_t *path)
{
#ifdef _WIN32
	FixSlashes();
	wchar_t temp[MAX_PATH] = {0};
	if (val)
		PathCombineW(temp, val, path);
	else
		WCSCPYN(temp, path, MAX_PATH);

	PathAddBackslashW(temp);
	setValue(temp);
#else
#warning find a better way
	wchar_t temp[PATH_MAX] = {0};
	swprintf(temp, PATH_MAX, L"%s%s/", val, path);
	setValue(temp);
#endif
}

void StringW::AddBackslash()
{
	FixSlashes();
	if (val)
	{
#ifdef _WIN32
		wchar_t temp[MAX_PATH] = {0};
		WCSCPYN(temp, val, MAX_PATH);
		PathAddBackslashW(temp);
#else
		wchar_t temp[PATH_MAX] = {0};
		WCSCPYN(temp, val, PATH_MAX);
		wcscat(temp, L"/");
#warning port me
#endif
		setValue(temp);
	}
}

void StringW::changeChar(wchar_t from, wchar_t to)
{
	if (val == NULL) return ;
	size_t length = len();
	for (size_t i = 0; i != length; i++)
		if (val[i] == from) val[i] = to;
}

void StringW::RemovePath()
{
#ifdef _WIN32
	// benski> the OS-level function fucks up if there are forward slashes, so we'll fix it
	// TODO: remove eventually
	FixSlashes();

	if (val)
		PathRemoveFileSpecW(val);
#else
#warning port me
#endif
}

void StringW::purge()
{
	FREE(val);
	val = NULL;
}

StringW StringW::rSplitChar(const wchar_t *findval)
{
	if (val == NULL) return StringW();
	// The index of the found character
	size_t idxval = rFindChar(findval);
	return rSplit(idxval);
}

// Same as above, save the "findval" is a string where it searches
// for any of the characters in the string.
size_t StringW::rFindChar(const wchar_t *findval)
{
	size_t length = len();
	size_t numchars = wcslen(findval);
	for (size_t i = length - 1; i > 0; i--)
	{
		for (size_t j = 0; j != numchars; j++)
		{
			if (val[i] == findval[j])
			{
				return i;
			}
		}
	}
	return -1;
}

StringW StringW::lSplitChar(const wchar_t *findval)
{
	if (val == NULL) return StringW();
	// The index of the found character
	size_t idxval = lFindChar(findval);
	return lSplit(idxval);
}

// Same as above, save the "findval" is a string where it searches
// for any of the characters in the string.
size_t StringW::lFindChar(const wchar_t *findval)
{
	size_t length = len();
	size_t numchars = wcslen(findval);
	for (size_t i = 0; i != length; i++)
	{
		for (size_t j = 0; j != numchars; j++)
		{
			if (val[i] == findval[j])
			{
				return i;
			}
		}
	}
	return -1;
}

StringW StringW::rSplit(size_t idxval)
{
	if (val == NULL) return StringW();
	if (idxval == -1)
	{	// Not Found
		// Copy our contents to return on the stack
		StringW retval(val);
		// And zero the string.
		val[0] = 0;
		return retval;
	}
	else
	{
		// Copy from the found index downwards to the retval
		StringW retval(val + idxval);
		// Terminate the found char index
		val[idxval] = 0;
		// That was easier, wasn't it?
		return retval;
	}
}

// Splits string at findval.  Characters passed by search, including the
// found character, are MOVED to the returned string.  If there is no char
// to be found, the entire string is returnef and the called instance is
// left empty.  (Makes looped splits very easy).
StringW StringW::lSplit(size_t idxval)
{
	if (val == NULL) return StringW();
	if (idxval == -1)
	{	// Not Found
		// Copy our contents to return on the stack
		StringW retval(val);
		// And zero the string.
		if (val)
		{
			val[0] = 0;
		}
		return retval;
	}
	else
	{
		StringW retval;
		// Copy into retval the number of characters to the found char index.
		retval.ncpy(val, idxval + 1);
		{
			StringW testscope;
			// Copy into retval the number of characters to the found char index.
			testscope.ncpy(val, idxval + 1);
		}
#if USE == FAST_METHODS
		size_t len = wcslen(val + idxval + 1);
		MEMCPY(val, val + idxval + 1, sizeof(wchar_t)*(len + 1));
#elif USE == SAFE_METHODS
		// Copy from the found index downwards to save for this object
		StringW temp(val + idxval + 1);
		// And then copy into ourselves the tempspace.
		*this = temp;
#endif
		return retval;
	}
}

// Same as split, except the find char is cut completely.
StringW StringW::lSpliceChar(const wchar_t *findval)
{
	if (val == NULL) return StringW();
	//CUT  // Auto-scope reference allows us to avoid a copy.
	//CUT  String & retval = lSplitChar(findval);
	//BU gcc doesn't agree with you and neither do I :/
	StringW retval = lSplitChar(findval);
	// We need to strip the findval char, which is the end char.
	size_t end = retval.len();
	size_t num = wcslen(findval);
	if (end)
	{
		for (size_t i = 0; i != num; i++)
		{
			if (retval.val[end - 1] == findval[i])
			{
				retval.val[end - 1] = 0;
			}
		}
	}
	return retval;
}

StringW StringW::rSpliceChar(const wchar_t *findval)
{
	if (val == NULL) return StringW();
	//CUT  // Auto-scope reference allows us to avoid a copy.
	//CUT  String & retval = rSplitChar(findval);
	//BU gcc doesn't agree with you and neither do I :/
	StringW retval = rSplitChar(findval);
	// We need to strip the findval char, which is the first char.
	// (But we still check for empty string:)
	size_t end = retval.len();
	size_t num = wcslen(findval);
	if (end)
	{
		for (size_t i = 0; i != num; i++)
		{
			if (retval.val[0] == findval[i])
			{
#if USE == FAST_METHODS
				size_t len = wcslen(retval.val + 1);
				MEMCPY(retval.val, retval.val + 1, sizeof(wchar_t)*(len + 1));
#elif USE == SAFE_METHODS
				StringW temp(retval.val + 1);
				retval = temp;
#endif
				return retval;
			}
		}
	}
	return retval;
}

void StringW::FixSlashes()
{
	if (val)
	{
		wchar_t *itr = val;
		while (itr && *itr)
		{
			if (*itr == '\\' || *itr == '/')
				*itr = Wasabi::Std::dirChar();
      #ifdef _WIN32
			itr = CharNextW(itr);
#else
      itr++;
#endif
		}
	}
}

/* ------------ */
StringPrintfW::StringPrintfW(const wchar_t *format, ...)
{
	va_list args;
	va_start (args, format);
	va_sprintf(format, args);
	va_end(args);
}

StringPrintfW::StringPrintfW(int value)
{
	*this += value;
}

StringPrintfW::StringPrintfW(double value)
{
	// TODO: review to use locale variant...
	wchar_t* locale = _wcsdup(_wsetlocale(LC_NUMERIC, NULL));
	_wsetlocale(LC_NUMERIC, L"C");
	*this += StringPrintfW(L"%f", value);
	if (locale)
	{
		_wsetlocale(LC_NUMERIC, locale);
		free(locale);
	}
}

StringPrintfW::StringPrintfW(GUID g)
{
	wchar_t splab[nsGUID::GUID_STRLEN + 1] = {0};
	nsGUID::toCharW(g, splab);
	cat(splab);
}

/* ------------ */
int StringWComparator::compareItem(StringW *p1, StringW* p2)
{
	return wcscmp(p1->getValue(), p2->getValue());
}

int StringWComparator::compareAttrib(const wchar_t *attrib, StringW *item)
{
	return wcscmp(attrib, item->getValue());
}

/* ------------ */
_DebugStringW::_DebugStringW(const wchar_t *format, ...)
{
	va_list args;
	va_start (args, format);
	va_sprintf(format, args);
	va_end(args);
	debugPrint();
}

void _DebugStringW::debugPrint()
{
#ifdef _WIN32
	OutputDebugStringW(getValue());
	if (lastChar() != L'\n') OutputDebugStringW(L"\n");
#else
#warning port me
#endif
}

StringPathCombine::StringPathCombine(const wchar_t *path, const wchar_t *filename)
{
#ifdef _WIN32
	wchar_t temp[MAX_PATH] = {0};
	PathCombineW(temp, path, filename);
	setValue(temp);
#else
	setValue(path);
	AppendPath(filename);
#endif
}

// StringW operators using StringPrintf
const wchar_t *StringW::operator +=(wchar_t value)
{
	wchar_t add[2]={value, 0};
	return cat(add);
	// the "Fast" methods and the Printf be
	// built off of that?
}

const wchar_t *StringW::operator +=(int value)
{
	wchar_t num[64] = {0};
#ifdef _WIN32
	_itow(value, num, 10);
	#else
	WCSNPRINTF(num, 64, L"%d", value);
	#endif
	return cat(num);
}

const wchar_t *StringW::operator +=(GUID guid)
{
	wchar_t guidstr[64] = {0};
	nsGUID::toCharW(guid, guidstr);
	return cat(guidstr);
}