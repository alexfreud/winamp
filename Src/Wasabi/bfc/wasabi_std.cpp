#include "wasabi_std.h"
//#include <wasabicfg.h>
#include <bfc/parse/pathparse.h>
#ifdef WIN32

#define WIN32_LEAN_AND_MEAN

#include <shlobj.h>
#include <multimon.h>
#if !defined(HMONITOR_DECLARED) && (WINVER < 0x500)
DECLARE_HANDLE(HMONITOR);
#define HMONITOR_DECLARED
#endif

#include <objbase.h>
#include <shellapi.h>  // for ShellExecute
#include <math.h>
#include <mbctype.h>

#include <sys/stat.h>

#ifndef SPI_GETWHEELSCROLLLINES
#  define SPI_GETWHEELSCROLLLINES  104
#endif

#ifndef SPI_GETLISTBOXSMOOTHSCROLLING
#  define SPI_GETLISTBOXSMOOTHSCROLLING  0x1006
#endif

#endif  // WIN32

#include <bfc/bfc_assert.h>

#include <bfc/ptrlist.h>

#include <time.h>  // CUT if possible... maybe make bfc/platform/time.h
#ifdef __APPLE__
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <malloc.h>
#include <shlwapi.h>
#endif

static HANDLE wasabiHeap = NULL;

void *MALLOC(size_t size) 
{
	//ASSERT(wasabiHeap != NULL);
      ASSERT(size > 0);
	//return HeapAlloc(wasabiHeap, 0, size);
    return malloc(size);
}

wchar_t *WMALLOC(size_t size) 
{
//	ASSERT(wasabiHeap != NULL);
      ASSERT(size > 0);
	size *= sizeof(wchar_t);
//	return (wchar_t *)HeapAlloc(wasabiHeap, 0, size);
    return (wchar_t*)malloc(size);
}

void *CALLOC(size_t records, size_t recordsize) {
//	ASSERT(wasabiHeap != NULL);
  ASSERT(records > 0 && recordsize > 0);
 // return HeapAlloc(wasabiHeap, HEAP_ZERO_MEMORY, records * recordsize);
  return calloc(records, recordsize);
}

void FREE(void *ptr) {
  if (ptr == NULL) 
      return;
	//HeapFree(wasabiHeap, 0, ptr);
  free(ptr);
  ptr = NULL;
}

// Note: MEMCPY allows dest and src to overlap
void MEMCPY(void *dest, const void *src, size_t n) {
  ASSERT(dest != NULL);
  ASSERT(src != NULL);
  ASSERT(n >= 0);
  memmove(dest, src, n);
}

void *MEMDUP(const void *src, size_t n) {
  void *ret;
  ASSERT(n >= 0);
  if (src == NULL || n == 0) return NULL;
  ret = MALLOC(n);
  if (ret == NULL) return NULL;
  MEMCPY(ret, src, n);
  return ret;
}

void *REALLOC(void *ptr, size_t size) {
	//ASSERT(wasabiHeap != NULL);
	ASSERT(size >= 0);
	if (ptr == NULL) {
        return realloc(0, size);
        //return HeapAlloc(wasabiHeap, 0, size);
	} else {
        return realloc(ptr, size);
		//return HeapReAlloc(wasabiHeap, 0, ptr, size);
	}
}

static wchar_t TOUPPERANDSLASH(wchar_t a)
{
  if (a=='\\')
    a = '/';
  else
    a = TOUPPERW(a);
  return a;
}

int PATHEQL(const wchar_t *str1, const wchar_t *str2) {
  if (str1 == NULL) {
    if (str2 == NULL) return TRUE;
    return FALSE;
  }
  while (TOUPPERANDSLASH(*str1) == TOUPPERANDSLASH(*str2) && *str1 != 0 && *str2 != 0) str1++, str2++;
  return *str1 == *str2;
}


static int rand_inited;

static double divider=0.;


void Wasabi::Std::Initialize() 
{
	//if (wasabiHeap == NULL) {
	//	wasabiHeap = HeapCreate(0, 0, 0);
	//	srandom();
	//	LARGE_INTEGER ll;
	//	QueryPerformanceFrequency(&ll);
	//	divider = (double)ll.QuadPart; 
	//}

    if (0 == divider)
    {
        srandom();
        LARGE_INTEGER ll;
        QueryPerformanceFrequency(&ll);
        divider = (double)ll.QuadPart;
    }
}

void Wasabi::Std::getMousePos(POINT *p) {
  ASSERT(p != NULL);
#ifdef WIN32
  GetCursorPos(p);
#elif defined(__APPLE__)
  Point pt;
  GetMouse(&pt);
  p->x = pt.v;
  p->y = pt.h;
#else
  Window w1, w2;
  int a, b;
  unsigned int c;
  int x, y;
  
  XQueryPointer( Linux::getDisplay(), Linux::RootWin(), &w1, &w2,
                 &x, &y, &a, &b, &c );
  
  p->x = x;
  p->y = y;
#endif
}

void Wasabi::Std::getMousePos(int *x, int *y) {
  POINT p;
  getMousePos(&p);
  if (x != NULL) *x = p.x;
  if (y != NULL) *y = p.y;
}

void Wasabi::Std::getMousePos(long *x, long *y) {
  getMousePos((int *)x, (int *)y);
}

void Wasabi::Std::setMousePos(POINT *p) {
  ASSERT(p != NULL);
#ifdef WIN32
  SetCursorPos(p->x, p->y);
#elif defined(__APPLE__)
  CGWarpMouseCursorPosition(HIPointFromPOINT(p));
#else
  POINT p2;
  getMousePos( &p2 );
  
  XWarpPointer( Linux::getDisplay(), None, None,
                0, 0, 1, 1, p->x - p2.x, p->y - p2.y );
#endif
}

void Wasabi::Std::setMousePos(int x, int y) {
  POINT p={x,y};
  setMousePos(&p);
}

void Wasabi::Std::getViewport(RECT *r, POINT *p, int full)
{
#ifdef _WIN32
  getViewport(r, p, NULL, (HWND)0, full);
#elif defined(__APPLE__)
  CGDirectDisplayID display;
  CGDisplayCount count;
  if (CGGetDisplaysWithPoint(HIPointFromPOINT(p), 1, &display, &count) == kCGErrorSuccess)
  {
    HIRect rect = CGDisplayBounds(display);
    *r = RECTFromHIRect(&rect);
  // TODO: cut out dock if full == 0   maybe GetAvailableWindowPositioningBounds if we can get the GDHandle
  }
#endif
}

void Wasabi::Std::getViewport(RECT *r, RECT *sr, int full)
{
#ifdef _WIN32
  getViewport(r, NULL, sr, (HWND)0, full);
#elif defined(__APPLE__)
  CGDirectDisplayID display;
  CGDisplayCount count;
  if (CGGetDisplaysWithRect(HIRectFromRECT(sr), 1, &display, &count) == kCGErrorSuccess)
  {
    HIRect rect = CGDisplayBounds(display);
    *r = RECTFromHIRect(&rect);
  // TODO: cut out dock if full == 0   maybe GetAvailableWindowPositioningBounds if we can get the GDHandle
  }
#endif
}

void Wasabi::Std::getViewport(RECT *r, OSWINDOWHANDLE wnd, int full)
{
#ifdef _WIN32
	getViewport(r, NULL, NULL, wnd, full);
#elif defined(__APPLE__)
	GDHandle gd;
	Rect gdr;
	GetWindowGreatestAreaDevice(wnd, kWindowStructureRgn, &gd, &gdr);
	  
	if (full)
	{
		CGDirectDisplayID display = QDGetCGDirectDisplayID(gd);
		HIRect rect = CGDisplayBounds(display);
		*r = RECTFromHIRect(&rect);
	}
	else
	{
		// TODO: maybe use GetAvailableWindowPositioningBounds instead
		r->left = gdr.left;
		r->top = gdr.top;
		r->right = gdr.right;
		r->bottom = gdr.bottom;
	}
#endif
}
#ifdef _WIN32
static HINSTANCE user32_instance = 0;
BOOL (WINAPI *Gmi)(HMONITOR mon, LPMONITORINFO lpmi) = 0;
#endif
void Wasabi::Std::getViewport(RECT *r, POINT *p, RECT *sr, OSWINDOWHANDLE wnd, int full) {
	ASSERT(r != NULL);
#ifdef WIN32
	if (p || sr || wnd) 
	{
		if (!user32_instance)
			user32_instance=LoadLibraryW(L"user32.dll");

		if (user32_instance)
		{
			HMONITOR (WINAPI *Mfp)(POINT pt, DWORD dwFlags) = (HMONITOR (WINAPI *)(POINT,DWORD)) GetProcAddress(user32_instance,"MonitorFromPoint");
			HMONITOR (WINAPI *Mfr)(LPCRECT lpcr, DWORD dwFlags) = (HMONITOR (WINAPI *)(LPCRECT, DWORD)) GetProcAddress(user32_instance, "MonitorFromRect");
			HMONITOR (WINAPI *Mfw)(HWND wnd, DWORD dwFlags)=(HMONITOR (WINAPI *)(HWND, DWORD)) GetProcAddress(user32_instance, "MonitorFromWindow");

			if (!Gmi)
				Gmi = (BOOL (WINAPI *)(HMONITOR,LPMONITORINFO)) GetProcAddress(user32_instance,"GetMonitorInfoW");

			if (Mfp && Mfr && Mfw && Gmi) 
			{
				HMONITOR hm;
                if ( p )
                    hm = Mfp( *p, MONITOR_DEFAULTTONEAREST );
                else if ( sr )
                    hm = Mfr( sr, MONITOR_DEFAULTTONEAREST );
                else if ( wnd )
                    hm = Mfw( wnd, MONITOR_DEFAULTTONEAREST );

				if (hm)
                {
					MONITORINFOEXW mi;
					ZERO(mi);
					mi.cbSize=sizeof(mi);

					if (Gmi(hm,&mi)) 
					{
						if(!full)
                            *r=mi.rcWork;
						else
                            *r=mi.rcMonitor;

						return;
					}
				}
			}
		}
	}
	SystemParametersInfoW(SPI_GETWORKAREA,0,r,0);
#endif
#ifdef LINUX
	r->left = r->top = 0;
	r->right = Wasabi::Std::getScreenWidth();
	r->bottom = Wasabi::Std::getScreenHeight();
#endif
}

#ifdef WIN32
class ViewportEnumerator {
  public:
    HMONITOR hm;
#ifdef WIN32
    int monitor_n;
#endif
    int counter;
};

static BOOL CALLBACK enumViewportsProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	ViewportEnumerator *ve = reinterpret_cast<ViewportEnumerator *>(dwData);
	ASSERT(ve != NULL);
	if (ve->counter == ve->monitor_n) {
		ve->hm = hMonitor;
		return FALSE;
	}
	ve->counter++;
	return TRUE;
}
#endif

void Wasabi::Std::srandom(unsigned int key) {
	if (key == 0) key = (unsigned int)Std::getTimeStamp();
	::srand(key);
}

int Wasabi::Std::random(int max) {
//  ASSERT(max <= RAND_MAX);
  int ret = ::rand();
  if (max != RAND_MAX+1) ret %= max;
  return ret;
}

unsigned int Wasabi::Std::random32(unsigned int max) {
  if (!rand_inited++) Wasabi::Std::srandom();
  unsigned int val=0;
  for (int i = 0; i < sizeof(unsigned int); i++) {
    val <<= 8;
    val |= ((::rand()>>2) & 0xff);
  }
  if (max != 0xffffffff) val %= max;
  return val;
}

#ifdef _WIN32 // PORT ME
void Wasabi::Std::usleep(int ms) {
  Sleep(ms);
//INLINE
}
#endif

__time64_t Wasabi::Std::getTimeStamp() {
  __time64_t time64;
  _time64(&time64);
  return time64;
}

stdtimevalms Wasabi::Std::getTimeStampMS() {
#ifdef WIN32
  LARGE_INTEGER ll;
  if (!QueryPerformanceCounter(&ll)) return getTickCount() / 1000.f;
  stdtimevalms ret = (stdtimevalms)ll.QuadPart;
  return ret /= divider;
#else
  return getTickCount() / 1000.f;
#endif
}

uint32_t Wasabi::Std::getTickCount()
{
#ifdef _WIN32
  return GetTickCount();
#elif defined(__APPLE__)
  struct timeval newtime;
  
	gettimeofday(&newtime, 0);
  return newtime.tv_sec*1000 + newtime.tv_usec/1000;
#endif
//INLINE
}

#ifdef _WIN32 // PORT ME
void Wasabi::Std::ensureVisible(RECT *r) {
  RECT sr;
  POINT p={(r->right+r->left)/2,(r->bottom+r->top)/2};
  Wasabi::Std::getViewport(&sr,&p);
  int w = r->right-r->left;
  int h = r->bottom-r->top;
  if (r->bottom > sr.bottom) {
    r->bottom = sr.bottom;
    r->top = r->bottom-h;
  }
  if (r->right > sr.right) {
    r->right = sr.right;
    r->left = r->right-w;
  }
  if (r->left < sr.left) {
    r->left = sr.left;
    r->right = r->left+w;
  }
  if (r->top < sr.top) {
    r->top = sr.top;
    r->bottom = r->top+h;
  }
}
#endif

int Wasabi::Std::getScreenWidth()
{
#ifdef WIN32
  RECT r;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.right-r.left;
#elif defined(__APPLE__)
  CGDirectDisplayID  mainID = CGMainDisplayID();
  return CGDisplayPixelsWide(mainID);
#elif defined(LINUX)
  return DisplayWidth(Linux::getDisplay(), Linux::getScreenNum());
#endif
}

int Wasabi::Std::getScreenHeight() 
{
#ifdef WIN32
  RECT r;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
  return r.bottom-r.top;
#elif defined(__APPLE__)
  CGDirectDisplayID  mainID = CGMainDisplayID();
  return CGDisplayPixelsHigh(mainID);  
#else
  return DisplayHeight( Linux::getDisplay(), Linux::getScreenNum() );
#endif
}
#ifndef __APPLE__ // PORT ME
int Wasabi::Std::messageBox(const wchar_t *txt, const wchar_t *title, int flags) {
#ifdef WIN32
  return MessageBoxW(NULL, txt, title, flags);
#else
 OutputDebugString(txt);
#endif
}
#endif

int Wasabi::Std::getDoubleClickDelay() {
#ifdef WIN32
  return GetDoubleClickTime();
#elif defined(__APPLE__)
  return GetDblTime();
#endif
}

#undef GetSystemMetrics //FG> DUH!
int Wasabi::Std::getDoubleClickX() {
#ifdef WIN32
  return GetSystemMetrics(SM_CYDOUBLECLK);
#else
  return 5;
#endif
}

int Wasabi::Std::getDoubleClickY() {
#ifdef WIN32
  return GetSystemMetrics(SM_CXDOUBLECLK);
#else
  return 5;
#endif
}

int Wasabi::Std::osparam_getScrollLines() {
  int ret = 3;
#ifdef WIN32
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ret, 0);
#endif
  return ret;
}

int Wasabi::Std::osparam_getSmoothScroll() {
  int ret = 1;
#ifdef WIN32
  SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &ret, 0);
#endif
  return ret;
}



const wchar_t Wasabi::Std::dirChar() {
#ifdef WIN32
  return '\\';
#else
  return '/';
#endif
}

const char * Wasabi::Std::dirCharStr() {
#ifdef WIN32
  return "\\";
#else
  return "/";
#endif
};

const wchar_t * Wasabi::Std::dirCharStrW()
{
#ifdef WIN32
  return L"\\";
#else
  return L"/";
#endif
};

int Wasabi::Std::isDirChar(int thechar, int allow_multiple_platforms) {
  if (thechar == DIRCHAR) return 1;
  if (allow_multiple_platforms) {
#ifdef WIN32
    if (thechar == '/') return 1;
#else
    if (thechar == '\\') return 1;
#endif
  }
  return 0;
}

const wchar_t * Wasabi::Std::matchAllFiles() {
#ifdef WIN32
  return L"*.*";
#else
  return L"*";
#endif
}

const wchar_t * Wasabi::Std::dotDir() {
  return L".";
}

const wchar_t * Wasabi::Std::dotDotDir()
{
  return L"..";
}
#ifdef _WIN32 // PORT ME
bool Wasabi::Std::isRootPath(const wchar_t *path) 
{
	return PathIsRootW(path)==TRUE;
}
#endif

int Wasabi::Std::switchChar() {
#ifdef WIN32
  return '/';
#else
  return '-';
#endif
}

int Wasabi::Std::enumViewports( int monitor_n, RECT *r, int full )
{
    ASSERT( r != NULL );
#ifdef WIN32
    if ( !user32_instance )
    {
        user32_instance = LoadLibraryW( L"user32.dll" );
    }
    if ( user32_instance )
    {
        if ( !Gmi )
        {
            Gmi = ( BOOL( WINAPI * )( HMONITOR, LPMONITORINFO ) ) GetProcAddress( user32_instance, "GetMonitorInfoW" );
        }
        BOOL( WINAPI * Edm )( HDC hdc, LPCRECT clip, MONITORENUMPROC proc, LPARAM dwdata ) = ( BOOL( WINAPI * )( HDC, LPCRECT, MONITORENUMPROC, LPARAM ) ) GetProcAddress( user32_instance, "EnumDisplayMonitors" );
        if ( Gmi && Edm )
        {
            ViewportEnumerator ve;
            ve.monitor_n = monitor_n;
            ve.hm = NULL;
            ve.counter = 0;
            Edm( NULL, NULL, enumViewportsProc, reinterpret_cast<LPARAM>( &ve ) );
            HMONITOR hm = ve.hm;
            if ( hm )
            {
                MONITORINFOEXW mi;
                ZERO( mi );
                mi.cbSize = sizeof( mi );
                if ( Gmi( hm, &mi ) )
                {
                    if ( !full )
                        *r = mi.rcWork;
                    else
                        *r = mi.rcMonitor;

                    return 1;
                }
            }
        }
    }
    SystemParametersInfoW( SPI_GETWORKAREA, 0, r, 0 );
    return 0;
#elif defined(__APPLE__)
    CGDirectDisplayID monitors[ 256 ]; // TODO: allocate dynamically
    CGDisplayCount count;
    if ( monitor_n >= 256 )
        return 0;

    CGGetActiveDisplayList( 256, monitors, &count );
    if ( count <= monitor_n )
        return 0;

    HIRect rect = CGDisplayBounds( monitors[ monitor_n ] );
    *r = RECTFromHIRect( &rect );
    // TODO: cut out dock if full == 0   maybe GetAvailableWindowPositioningBounds if we can get the GDHandle
#elif defined(LINUX)
    if ( monitor_n > 0 )
        return 0;
    r->left = r->top = 0;
    r->right = Wasabi::Std::getScreenWidth();
    r->bottom = Wasabi::Std::getScreenHeight();
    return 1;
#endif
}

const char * Wasabi::Std::scanstr_back(const char *str, const char *toscan, const char *defval) {
  int strl = STRLEN(str);
  const char *s=str+strl-1;
  if (strl < 1) return defval;
  if (STRLEN(toscan) < 1) return defval;
  while (1) {
    const char *t=toscan;
    while (t && *t) if (*t++ == *s) return s;
#ifdef _WIN32
    t=CharPrevA(str,s);
#else
    t = s-1;
#endif
    if (t==s) return defval;
    s=t;
  }
}

const wchar_t * Wasabi::Std::extension(const wchar_t *fn)
{
#ifdef _WIN32 // PORT ME
	wchar_t *x = PathFindExtensionW(fn);
	if (x && *x)
		return CharNextW(x);
	else
		return x;
#else
#warning port me
  return 0;
#endif
}

const wchar_t * Wasabi::Std::filename(const wchar_t *fn)
{
#ifdef _WIN32
	return PathFindFileNameW(fn);
#else
#warning port me
  return 0;
#endif
}

bool Wasabi::Std::isMatchPattern(const wchar_t *p) 
{
  while (p && *p) {
    switch (*p++) {
      case '?':
      case '*':
      case '[':
      case '\\':
        return TRUE;
    }
  }
  return FALSE;
}

bool Wasabi::Std::isValidMatchPattern(const wchar_t *p, int *error_type) 
{
  /* init error_type */
  *error_type = PATTERN_VALID;
  /* loop through pattern to EOS */
  while (p && *p) {
    /* determine pattern type */
    switch (*p) {
      /* check literal escape, it cannot be at end of pattern */
      case '\\':
        if (!*++p) {
          *error_type = PATTERN_ESC;
          return FALSE;
        }
        p++;
        break;
      /* the [..] construct must be well formed */
      case '[':
        p++;
        /* if the next character is ']' then bad pattern */
        if (*p == ']') {
          *error_type = PATTERN_EMPTY;
          return FALSE;
        }
        /* if end of pattern here then bad pattern */
        if (!*p) {
          *error_type = PATTERN_CLOSE;
          return FALSE;
        }
        /* loop to end of [..] construct */
        while (p && *p != ']') {
          /* check for literal escape */
          if (*p == '\\') {
            p++;
            /* if end of pattern here then bad pattern */
            if (!*p++) {
              *error_type = PATTERN_ESC;
              return FALSE;
            }
          } else p++;
        /* if end of pattern here then bad pattern */
        if (!*p) {
          *error_type = PATTERN_CLOSE;
          return FALSE;
        }
        /* if this a range */
        if (*p == '-') {
          /* we must have an end of range */
          if (!*++p || *p == ']') {
            *error_type = PATTERN_RANGE;
            return FALSE;
          } else {
            /* check for literal escape */
            if (*p == '\\') p++;
            /* if end of pattern here then bad pattern */
            if (!*p++) {
              *error_type = PATTERN_ESC;
              return FALSE;
            }
          }
        }
      }
      break;
      /* all other characters are valid pattern elements */
      case '*':
      case '?':
      default:
        p++; /* "normal" character */
        break;
    }
  }
  return TRUE;
}

#ifdef _WIN32 // PORT ME
int Wasabi::Std::matche(register const wchar_t *p, register const wchar_t *t) 
{
  register wchar_t range_start, range_end;  /* start and end in range */

  BOOLEAN invert;             /* is this [..] or [!..] */
  BOOLEAN member_match;       /* have I matched the [..] construct? */
  BOOLEAN loop;               /* should I terminate? */

  for ( ; *p; p++, t++) 
	{
    /* if this is the end of the text then this is the end of the match */
    if (!*t) {
      return ( *p == '*' && *++p == '\0' ) ? MATCH_VALID : MATCH_ABORT;
    }
    /* determine and react to pattern type */
    switch (*p) {
      case '?': /* single any character match */
        break;
      case '*': /* multiple any character match */
        return matche_after_star (p, t);

      /* [..] construct, single member/exclusion character match */
      case '[': {
        /* move to beginning of range */
        p++;
        /* check if this is a member match or exclusion match */
        invert = FALSE;
        if (*p == '!' || *p == '^') {
          invert = TRUE;
          p++;
        }
        /* if closing bracket here or at range start then we have a malformed pattern */
        if (*p == ']') return MATCH_PATTERN;

        member_match = FALSE;
        loop = TRUE;
        while (loop) {
          /* if end of construct then loop is done */
          if (*p == ']') {
            loop = FALSE;
            continue;
          }
          /* matching a '!', '^', '-', '\' or a ']' */
          if (*p == '\\') range_start = range_end = *++p;
          else  range_start = range_end = *p;
          /* if end of pattern then bad pattern (Missing ']') */
          if (!*p) return MATCH_PATTERN;
          /* check for range bar */
          if (*++p == '-') {
            /* get the range end */
            range_end = *++p;
            /* if end of pattern or construct then bad pattern */
            if (range_end == '\0' || range_end == ']') return MATCH_PATTERN;
            /* special character range end */
            if (range_end == '\\') {
              range_end = *++p;
              /* if end of text then we have a bad pattern */
              if (!range_end) return MATCH_PATTERN;
            }
            /* move just beyond this range */
            p++;
          }
          /* if the text character is in range then match found.
             make sure the range letters have the proper
             relationship to one another before comparison */
          if (range_start < range_end) {
            if (*t >= range_start && *t <= range_end) {
              member_match = TRUE;
              loop = FALSE;
            }
          } else {
            if (*t >= range_end && *t <= range_start) {
              member_match = TRUE;
              loop = FALSE;
            }
          }
        }
        /* if there was a match in an exclusion set then no match */
        /* if there was no match in a member set then no match */
        if ((invert && member_match) || !(invert || member_match)) return MATCH_RANGE;
        /* if this is not an exclusion then skip the rest of the [...] construct that already matched. */
        if (member_match) {
          while (p && *p != ']') {
            /* bad pattern (Missing ']') */
            if (!*p) return MATCH_PATTERN;
            /* skip exact match */
            if (*p == '\\') {
              p++;
              /* if end of text then we have a bad pattern */
              if (!*p) return MATCH_PATTERN;
            }
            /* move to next pattern char */
            p++;
          }
        }
        break;
      }
      case '\\':  /* next character is quoted and must match exactly */
        /* move pattern pointer to quoted char and fall through */
        p++;
        /* if end of text then we have a bad pattern */
        if (!*p) return MATCH_PATTERN;
        /* must match this character exactly */
      default:
        if (TOUPPERW(*p) != TOUPPERW(*t)) return MATCH_LITERAL;
    }
  }
  /* if end of text not reached then the pattern fails */
  if (*t) return MATCH_END;
  else return MATCH_VALID;
}

int Wasabi::Std::matche_after_star(register const wchar_t *p, register const wchar_t *t) 
{
  register int match = 0;
  register wchar_t nextp;
  /* pass over existing ? and * in pattern */
  while ( *p == '?' || *p == '*' ) 
	{
    /* take one char for each ? and + */
    if (*p == '?') {
      /* if end of text then no match */
      if (!*t++) return MATCH_ABORT;
    }
    /* move to next char in pattern */
    p++;
  }
  /* if end of pattern we have matched regardless of text left */
  if (!*p) return MATCH_VALID;
  /* get the next character to match which must be a literal or '[' */
  nextp = *p;
  if (nextp == '\\') {
    nextp = p[1];
    /* if end of text then we have a bad pattern */
    if (!nextp) return MATCH_PATTERN;
  }
  /* Continue until we run out of text or definite result seen */
  do {
    /* a precondition for matching is that the next character
       in the pattern match the next character in the text or that
       the next pattern char is the beginning of a range.  Increment
       text pointer as we go here */
    if (TOUPPERW(nextp) == TOUPPERW(*t) || nextp == '[') match = matche(p, t);
    /* if the end of text is reached then no match */
    if (!*t++) match = MATCH_ABORT;
  } while ( match != MATCH_VALID && match != MATCH_ABORT && match != MATCH_PATTERN);
  /* return result */
  return match;
}

bool Wasabi::Std::match(const wchar_t *p, const wchar_t *t)
{
  int error_type;
  error_type = matche(p,t);
  return (error_type == MATCH_VALID ) ? TRUE : FALSE;
}
#endif
#ifndef _NOSTUDIO
int Wasabi::Std::getCurDir(wchar_t *str, int maxlen) {
  ASSERT(str != NULL);
#ifdef WIN32
  int retval = 0;

  retval = GetCurrentDirectoryW(maxlen, str);

  return retval;

#else
#warning port me
  return 0;
  //return getcwd( str, maxlen ) != NULL;
#endif
}

int Wasabi::Std::setCurDir(const wchar_t *str) {
  ASSERT(str != NULL);
#ifdef WIN32
  int retval = 0;

  retval = SetCurrentDirectoryW(str);

  return retval;

#else
#warning port me
  return 0;
//  return chdir( str );
#endif
}


int Wasabi::Std::getNumCPUs() {
#ifdef WIN32
  SYSTEM_INFO si;
  ZERO(si);
  GetSystemInfo(&si);
  return si.dwNumberOfProcessors;
#else
#ifdef PLATFORM_WARNINGS
#warning Wasabi::Std::getNumCPUs not implemented on LINUX
#endif
  return 1;
#endif
}

THREADID Wasabi::Std::getCurrentThreadId() {
#ifdef WIN32
  return GetCurrentThreadId();
#else
  return pthread_self();
#endif
}

#ifdef WIN32
void Wasabi::Std::setThreadPriority(int delta, HANDLE thread_handle) 
{
  if (thread_handle == NULL) thread_handle = GetCurrentThread();
  int v = THREAD_PRIORITY_NORMAL;
  switch (delta) {
    case -32767: v = THREAD_PRIORITY_IDLE; break;
    case -2: v = THREAD_PRIORITY_LOWEST; break;
    case -1: v = THREAD_PRIORITY_BELOW_NORMAL; break;
    case 1: v = THREAD_PRIORITY_ABOVE_NORMAL; break;
    case 2: v = THREAD_PRIORITY_HIGHEST; break;
    case 32767: v = THREAD_PRIORITY_TIME_CRITICAL; break;
  }
  SetThreadPriority(thread_handle, v);
}
#else
#ifdef PLATFORM_WARNINGS
#warning Wasabi::Std::setThreadPriority not implemented on LINUX
#endif
#endif


String Wasabi::Std::getLastErrorString(int _err) {
  String ret;
  if (_err == -1) {
#ifdef WIN32
    _err = GetLastError();
#else
#ifdef PLATFORM_WARNINGS
#warning Wasabi::Std::getLastErrorString not implemented on LINUX
#endif
#endif
  }
#ifdef WIN32
  char *sysbuf;
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, _err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *)&sysbuf, 0, NULL);
  ret = sysbuf;
  LocalFree(sysbuf);
#endif
  return ret;
}

// gets the system font path
void Wasabi::Std::getFontPath(int bufsize, wchar_t *dst) 
{
  ASSERT(dst != NULL);
#ifdef WIN32
  
	wchar_t path[MAX_PATH]=L"";
  SHGetSpecialFolderPathW(NULL, path, CSIDL_FONTS, FALSE);
  WCSCPYN(dst, path, bufsize);


#else
#ifdef PLATFORM_WARNINGS
#warning Wasabi::Std::getFontPath not implemented on LINUX
#endif
#endif

}

const wchar_t wasabi_default_fontnameW[] = WASABI_DEFAULT_FONTNAMEW;
int default_font_scale = 100;
wchar_t default_font[256] =
#ifdef WIN32
WASABI_DEFAULT_FONTNAMEW L".ttf";
#elif defined(__APPLE__)
WASABI_DEFAULT_FONTNAMEW;
#elif defined(LINUX)
  // even tho this isn't the way we'll port this, the style is fun.
"-*-arial-medium-r-*--10-*-*-*-*-*-*-*";
#endif

// gets the filename of a font file guaranteed to be in the system font path.
void Wasabi::Std::getDefaultFont(int bufsize, wchar_t *dst)
{
  ASSERT(dst != NULL);
  WCSCPYN(dst, default_font, bufsize);
}

void Wasabi::Std::setDefaultFont(const wchar_t *newdefaultfont) 
{
  WCSCPYN(default_font, newdefaultfont, 256);
}

int Wasabi::Std::getDefaultFontScale() {
  return default_font_scale;
}

void Wasabi::Std::setDefaultFontScale(int scale) {
  default_font_scale = scale;
}

#ifndef __APPLE__ // PORT ME
int Wasabi::Std::createDirectory(const wchar_t *dirname) 
{
#ifdef WIN32
  if(!CreateDirectoryW(dirname,NULL))
#else
  if(mkdir(dirname, 0755))
#endif
  {
    // create all the path
    PathParserW pp(dirname);
    int l = pp.getNumStrings();
    for(int i=2;i<=l;i++) 
		{
      StringW dir;
      for(int j=0;j<i;j++)
				dir.AppendFolder(pp.enumString(j));
#ifdef WIN32
      CreateDirectoryW(dir,NULL);
#else
      mkdir(dir, 0755);
#endif
    }
  }
  return 1;
}
#endif

int Wasabi::Std::getFileInfos(const char *filename, fileInfoStruct *infos) {
#ifdef WIN32
	HANDLE h;
	WIN32_FIND_DATAA fd = {0};
	if((h=FindFirstFileA(filename, &fd))==INVALID_HANDLE_VALUE) return 0;

	infos->fileSizeHigh=fd.nFileSizeHigh;
	infos->fileSizeLow=fd.nFileSizeLow;
	struct _stati64 statbuf;
	if (_stati64(filename, &statbuf) == -1) return 0;
	infos->lastWriteTime = statbuf.st_mtime;
	infos->readonly = fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
	FindClose(h);

	return 1;
#else
	struct stat st;

	int ret = stat( filename, &st );
	if ( ret != 0 )
		return 0;

	infos->fileSizeHigh = 0;
	infos->fileSizeLow = st.st_size;
	infos->lastWriteTime = st.st_mtime;
	return 1;
#endif
}

void Wasabi::Std::shellExec(const wchar_t *cmd, const wchar_t *params)
{
#ifdef WIN32
  ShellExecuteW(NULL, NULL, cmd, params, L".", SW_SHOWNORMAL);
#else
	system(StringPrintf("%S %S", cmd, params));
#endif
}


#endif // nostudio

void MEMFILL32(void *lptr, unsigned long val, unsigned int n) {
  if (n == 0) return;
#if defined(WIN32) && !defined(_WIN64)
__asm {
  mov eax, val
  mov edi, lptr
  mov ecx, n
  rep stosd
};
#elif defined(GCC)
//http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html ;)
asm ("cld\n\t"
     "rep\n\t"
     "stosl"
     : /* no output registers */
     : "c" (n), "a" (val), "D" (lptr)
     : "%ecx", "%edi" );
#else
  uint32_t *ptr32 = (uint32_t *)lptr;
  for (unsigned int i=0;i!=n;i++)
  {
	 ptr32[i] = val;
  }
#endif
}

void MEMFILL(unsigned short *ptr, unsigned short val, unsigned int n) {
  if (n == 0) return;
  unsigned long v = (unsigned long)val | ((unsigned long)val << 16);
  int r = n & 1;
  MEMFILL32(ptr, v, n/2);
  if (r) ptr[n-1] = val;
}

void MEMCPY32(void *dest, const void *src, size_t words)
{
	// TODO: write fast asm version of this
	memcpy(dest, src, words*4);
}

void MEMCPY_(void *dest, const void *src, size_t n)
{
	memcpy(dest, src, n);
}
