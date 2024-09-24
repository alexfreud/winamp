#ifndef _STD_H
#define _STD_H

#include <string.h>
#include <bfc/platform/platform.h>
#include <bfc/common.h>
#include <bfc/bfc_assert.h>
//#include <bfc/string/string.h>

//#define WANT_UTF8_WARNINGS

// to use this, do #pragma WARNING("your message") <- note the ABSENCE of ';'
#ifndef WARNING
 #define WARNING_TOSTR(text) #text
 #define WARNING_TOSTR1(text) WARNING_TOSTR(text)
 #define WARNING(text) message(__FILE__ "(" WARNING_TOSTR1(__LINE__) ") : " text)
#endif 
// and for this one, do #pragma COMPILATION_CHAT("your_nick", "nick_you're_talking_to", "your message")
#define CHAT(from, to, text) message(__FILE__ "(" WARNING_TOSTR1(__LINE__) ") : <" from "> " to ": " text)
#define SELF(from, text) message(__FILE__ "(" WARNING_TOSTR1(__LINE__) ") : * " from"/#wasabi " text)

//#ifndef WASABIDLLEXPORT
//#error go define WASABIDLLEXPORT in your platform .h
//#endif

#ifndef NOVTABLE
#define NOVTABLE
#endif

#ifndef NOVTABLE
#error go define NOVTABLE in your platform .h
#endif

#include "std_mem.h"
#include "std_math.h"
#include "std_string.h"
#include "std_file.h"
#include "wasabi_std_rect.h"
#include "std_keyboard.h"
#include <stdlib.h>
#include <locale.h>
#define WASABI_DEFAULT_FONTNAME "Arial"
#define WASABI_DEFAULT_FONTNAMEW L"Arial"
extern const wchar_t wasabi_default_fontnameW[];
static _locale_t C_locale;

#include <wchar.h>
static __inline char TOUPPER(char c)
{
#ifdef _WIN32
	char tmp = c;
	CharUpperBuffA(&tmp, 1);
	return tmp;
#else
  return toupper(c);
#endif
}

static __inline wchar_t TOUPPERW(wchar_t c)
{
	return towupper(c);
}

static __inline wchar_t TOLOWERW(wchar_t c)
{
	return towlower(c);
}

__inline bool WTOB(const wchar_t *str) {
#ifdef WIN64
	if (!str || (unsigned long long)str < WCHAR_MAX) return 0;
#else
	if (!str || (unsigned long)str < WCHAR_MAX) return 0;
#endif
		
	if(!C_locale) C_locale = _create_locale(LC_NUMERIC,"C");
	return !!_wtoi_l(str,C_locale);
}

 __inline int WTOI(const wchar_t *str) {
#ifdef WIN64
	 if (!str || (unsigned long long)str < WCHAR_MAX) return 0;
#else
	 if (!str || (unsigned long)str < WCHAR_MAX) return 0;
#endif
	if(!C_locale) C_locale = _create_locale(LC_NUMERIC,"C");
	return _wtoi_l(str,C_locale);
 }

 //__inline int ATOI(const char *str) { if (!str) return 0; return atoi(str); }
 //__inline double ATOF(const char *str) { if (!str) return 0.0; return atof(str); }

 __inline double WTOF(const wchar_t *str) {
#ifdef WIN64
	 if (!str || (unsigned long long)str < WCHAR_MAX) return 0.0;
#else
	 if (!str || (unsigned long)str < WCHAR_MAX) return 0.0;
#endif

	if(!C_locale) C_locale = _create_locale(LC_NUMERIC,"C");
	return _wtof_l(str,C_locale);
 }

__inline int STRTOL(const char *str, char **stopstr, int base)
{
	if(!C_locale) C_locale = _create_locale(LC_NUMERIC,"C");
	return _strtol_l(str, stopstr, base, C_locale);
}

wchar_t *WCSDUP(const wchar_t *ptr);
COMEXP int STRLEN(const char *str);
COMEXP int STRCMP(const char *str1, const char *str2);
COMEXP int STRICMP(const char *str1, const char *str2);

int WCSICMP(const wchar_t *str1, const wchar_t *str2);
int WCSNICMP(const wchar_t *str1, const wchar_t *str2, size_t len);
int WCSCOLL(const wchar_t *str1, const wchar_t *str2);
int WCSICOLL(const wchar_t *str1, const wchar_t *str2);
bool WCSIPREFIX(const wchar_t *str1, const wchar_t *prefix);
wchar_t *WCSTOK(wchar_t *str, const wchar_t *sep, wchar_t **last);

COMEXP int STREQL(const char *str1, const char *str2);
COMEXP int STRCASEEQL(const char *str1, const char *str2);
wchar_t *WCSCASESTR(const wchar_t *str1, const wchar_t *str2);
COMEXP char *STRSTR(const char *str1, const char *str2);
COMEXP void STRCPY(char *dest, const char *src);
COMEXP void STRNCPY(char *dest, const char *src, int maxchar);
COMEXP void WCSCPYN(wchar_t *dest, const wchar_t *src, size_t maxchar);
COMEXP char *STRCHR(const char *str, int c);
COMEXP void STRCAT(char *dest, const char *append);
COMEXP unsigned long STRTOUL(const char *str, char **p, int radix);

#ifdef __cplusplus
COMEXP int STRCMPSAFE(const char *str1, const char *str2, const char *defval1 = "", const char *defval2 = "");
COMEXP int STRICMPSAFE(const char *str1, const char *str2, const char *defval1 = "", const char *defval2 = "");
COMEXP int WCSICMPSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1 = L"", const wchar_t *defval2 = L"");
COMEXP int WCSEQLSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1 = L"", const wchar_t *defval2 = L"");
COMEXP int STRCASEEQLSAFE(const char *str1, const char *str2, const char *defval1 = "", const char *defval2 = "");
COMEXP int WCSCASEEQLSAFE(const wchar_t *str1, const wchar_t *str2, const wchar_t *defval1 = L"", const wchar_t *defval2 = L"");
#endif

COMEXP int PATHEQL(const wchar_t *str1, const wchar_t *str2);
COMEXP void STRTOUPPER(char *str);
COMEXP void STRTOLOWER(char *str);
COMEXP void WCSTOUPPER(wchar_t *str);
COMEXP void KEYWORDUPPER(wchar_t *p);
COMEXP void WCSTOLOWER(wchar_t *str);

#ifdef _WIN32
#define STRSAFE_NO_DEPRECATE
//#include <tchar.h>
#include <strsafe.h>
#undef STRSAFE_NO_DEPRECATE
#define WCSNPRINTF StringCchPrintfW
#elif defined(__APPLE__)
#define WCSNPRINTF swprintf
#endif

#ifdef WIN32
#define SPRINTF wsprintfA
#else
#define SPRINTF sprintf
#endif

#define SSCANF sscanf

#define WA_MAX_PATH (8*1024)

// seconds since 1970
typedef unsigned int stdtimeval;
// milliseconds since...??
typedef double stdtimevalms;

#ifdef _WIN32
typedef unsigned long THREADID;
#else
typedef pthread_t THREADID;
#endif

#ifdef __cplusplus

#include <bfc/string/bfcstring.h>

namespace Wasabi
{
	namespace Std
	{
		void Initialize();
		void getMousePos(POINT* p);
		void getMousePos(int* x, int* y);
		void getMousePos(long* x, long* y);
		void setMousePos(POINT* p);
		void setMousePos(int x, int y);

		void srandom(unsigned int key = 0); //if key==0, uses time()
		int random(int max = RAND_MAX + 1); // from 0 to max-1 (RAND_MAX = 0x7fff)
		unsigned int random32(unsigned int max = 0xffffffff);  // full 32-bits of randomness

		// time functions
		void usleep(int ms);
		// get time in seconds since 1970
		time_t getTimeStamp();
		// get time in seconds as double, but not since any specific time
		// useful for relative timestamps only
		stdtimevalms getTimeStampMS();
		// get milliseconds since system started. usefull for relative only
		uint32_t getTickCount();

		void tolowerString(char* str);
		void ensureVisible(RECT* r);
		int getScreenWidth();
		int getScreenHeight();
		// THREADS/PROCESSES/CPUs
		int getNumCPUs();
		// a unique # returned by the OS
		THREADID getCurrentThreadId();
		// attempts to adjust thread priority compared to main thread
		// normal range is from -2 .. +2, and -32767 for idle, 32767 for time critical
		// use the constants from bfc/thread.h
#ifdef _WIN32
		void setThreadPriority(int delta, HANDLE thread_handle = NULL);
#endif

		String getLastErrorString(int force_errno = -1);


		int messageBox(const wchar_t* txt, const wchar_t* title, int flags);

		int getDoubleClickDelay();
		int getDoubleClickX();
		int getDoubleClickY();

		// returns how many lines to scroll for wheelie mice (from the OS)
		int osparam_getScrollLines();
		int osparam_getSmoothScroll();


		const wchar_t dirChar();  // \ for win32, / for all else
#define DIRCHAR (Wasabi::Std::dirChar())
		const char* dirCharStr(); // "\\" for win32, "/" for all else
#define DIRCHARSTR (Wasabi::Std::dirCharStr())
		const wchar_t* dirCharStrW(); // "\\" for win32, "/" for all else
#define DIRCHARSTRW (Wasabi::Std::dirCharStrW())
		int isDirChar(int thechar, int allow_multiple_platforms = TRUE);
		const wchar_t* matchAllFiles(); // "*.*" on win32, "*" on else
#define MATCHALLFILES (Wasabi::Std::matchAllFiles())
		const wchar_t* dotDir();  // usually "."
#define DOTDIR (Wasabi::Std::dotDir())
		const wchar_t* dotDotDir();  // usually ".."
#define DOTDOTDIR (Wasabi::Std::dotDotDir())

		bool isRootPath(const wchar_t* path); // "c:\" or "\" on win32, "/" on linux

		int switchChar();   // '/' on win32, '-' for all else
#define SWITCHCHAR (Wasabi::Std::switchChar())


		void getViewport(RECT* r, POINT* p, int full = 0);
		void getViewport(RECT* r, RECT* sr, int full = 0);
		void getViewport(RECT* r, OSWINDOWHANDLE wnd, int full = 0);
		void getViewport(RECT* r, POINT* p, RECT* sr, OSWINDOWHANDLE wnd, int full = 0);
		int enumViewports(int monitor_n, RECT* r, int full = 0);

		// returns the address of the last occurence of any of the characters of toscan in str string
		const char* scanstr_back(const char* str, const char* toscan, const char* defval);

		// retrieves extension of a given filename
		const wchar_t* extension(const wchar_t* fn);

		// retrieves filename from a given path+filename
		const wchar_t* filename(const wchar_t* fn);

		int getCurDir(wchar_t* str, int maxlen);
		int setCurDir(const wchar_t* str);

		// regexp match functions

		// A match means the entire string TEXT is used up in matching.
		// In the pattern string:
		//      `*' matches any sequence of characters (zero or more)
		//      `?' matches any character
		//      [SET] matches any character in the specified set,
		//      [!SET] or [^SET] matches any character not in the specified set.

		// A set is composed of characters or ranges; a range looks like
		// character hyphen character (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the
		// minimal set of characters allowed in the [..] pattern construct.
		// Other characters are allowed (ie. 8 bit characters) if your system
		// will support them.

		// To suppress the special syntactic significance of any of `[]*?!^-\',
		// and match the character exactly, precede it with a `\'.

		enum {
			MATCH_VALID = 1,   /* valid match */
			MATCH_END,       /* premature end of pattern string */
			MATCH_ABORT,     /* premature end of text string */
			MATCH_RANGE,     /* match failure on [..] construct */
			MATCH_LITERAL,   /* match failure on literal match */
			MATCH_PATTERN,   /* bad pattern */
		};

		enum {
			PATTERN_VALID = 0,   /* valid pattern */
			PATTERN_ESC = -1,    /* literal escape at end of pattern */
			PATTERN_RANGE = -2,  /* malformed range in [..] construct */
			PATTERN_CLOSE = -3,  /* no end bracket in [..] construct */
			PATTERN_EMPTY = -4,  /* [..] contstruct is empty */
		};

		// return TRUE if PATTERN has any special wildcard characters
		bool isMatchPattern(const wchar_t* p);

		// return TRUE if PATTERN has is a well formed regular expression
		bool isValidMatchPattern(const wchar_t* p, int* error_type);

		// return MATCH_VALID if pattern matches, or an errorcode otherwise
		int matche(register const wchar_t* p, register const wchar_t* t);
		int matche_after_star(register const wchar_t* p, register const wchar_t* t);

		// return TRUE if pattern matches, FALSE otherwise.
		bool match(const wchar_t* p, const wchar_t* t);

		// gets the system font path.
		void getFontPath(int bufsize, wchar_t* dst);

		// gets the filename of a font file guaranteed to be in the system font path.
		void getDefaultFont(int bufsize, wchar_t* dst);

		// sets a new default font, ie "blah.ttf"
		void setDefaultFont(const wchar_t* newdefaultfont);

		// gets the default font scale
		int getDefaultFontScale();

		// sets the new default font scale
		void setDefaultFontScale(int scale);

		// creates a directory. returns 0 on error, nonzero on success
		int createDirectory(const wchar_t* dirname);

		// gets informations about a given filename. returns 0 if file not found
		typedef struct
		{
			unsigned int fileSizeHigh;
			unsigned int fileSizeLow;
			time_t lastWriteTime;
			int readonly;
		}
		fileInfoStruct;

		int getFileInfos(const char* filename, fileInfoStruct* infos);

		// removes a directory
		void removeDirectory(const char* buf, int recurse);


		void shellExec(const wchar_t* cmd, const wchar_t* params = NULL);

		int isLittleEndian();

	}
}
#endif

#endif
