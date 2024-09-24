#ifndef __LINUX_H_WASABI
#define __LINUX_H_WASABI

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <sys/shm.h>

#ifndef NOVTABLE
#define NOVTABLE
#endif

#ifndef __USE_GNU
#define __USE_GNU
#include <pthread.h>
#undef __USE_GNU
#else
#include <pthread.h>
#endif

#ifdef WASABI_COMPILE_WND

// Fucking api_region and Font namespace conflicts
#define _XRegion _XRegion
#define api_region HRGN
#define Font HFONT
#define Cursor _Cursor
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XShm.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#undef _XRegion
#undef api_region
#undef Font
#undef Cursor

#ifdef WASABI_COMPILE_FONTS

#ifdef __INCLUDE_FREETYPE
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#endif // freetype

//typedef FT_Face HFONT;
#endif // fonts

#ifdef __INCLUDE_GTK
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#endif // gtk

typedef Window HWND;

#else // wnd
typedef void * HWND;
#endif // wnd

// I know about XRectangle, but it's easier to recreate this than it is
// to merge it with the rest of the code
typedef struct { int left, top, right, bottom; } RECT;

typedef unsigned long COLORREF;
typedef struct {
  char rgbBlue;
  char rgbGreen;
  char rgbRed;
  char filler;
} RGBQUAD;
#define RGB( r, g, b ) ((((r)&0xFF)<<16)|(((g)&0xFF)<<8)|((b)&0xFF))
#define min( a, b ) ((a>b)?b:a)
#define max( a, b ) ((a>b)?a:b)
#define CLR_NONE    0

#ifdef __cplusplus
#ifndef XMD_H
typedef int BOOL; // It's int not bool because of some inheritance conflicts
#endif
#ifndef TRUE
#define TRUE  true
#endif
#ifndef FALSE
#define FALSE false
#endif
#else
#ifndef XMD_H
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif
typedef BOOL BOOLEAN;
typedef struct { int x, y; } POINT;

#ifdef WASABI_COMPILE_WND

typedef void *HPEN;

// The HDC knows what it's drawing on
typedef struct hdc_typ {
  GC gc;
  Drawable d;
  HRGN clip;
} *HDC;

// Pixmaps don't have associated width and height, at least not that I know of
typedef struct {
  Pixmap p;
  int bmWidth;
  int bmHeight;
  XShmSegmentInfo *shmseginfo;

} BITMAP, HBITMAP;

typedef Pixmap HICON;

#endif // wnd

typedef int LRESULT;
typedef int LPARAM;
typedef int WPARAM;
typedef int RPARAM;
typedef unsigned int TCHAR;
typedef long long __int64;
typedef long long LARGE_INTEGER;
typedef unsigned long long ULARGE_INTEGER;
#define OSPIPE int
#define OSPROCESSID int
#define LOWORD(a) ((a)&0xffff)
#define HIWORD(a) (((a)>>16)&0xffff)

#define MAX_PATH 8192

#define COMEXP
#define EXTC extern "C"
#define __cdecl
#define CALLBACK
#define WINAPI
#define HRESULT void*
#define WINUSERAPI
#define APIENTRY
#define __declspec(a)
typedef char * LPSTR;
typedef unsigned long DWORD;
typedef short int WORD;
#ifndef XMD_H
typedef unsigned char BYTE;
#endif
typedef void* LPVOID;
typedef struct {
  long cx, cy;
} SIZE;
typedef long LONG;
#define VOID void

#ifdef WASABI_COMPILE_WND
// Fix this for canvas
typedef void * PAINTSTRUCT;
#endif

#ifndef WASABI_COMPILE_WND
#ifndef None
#define None (HWND)0
#endif
#endif

typedef void* THREAD_RET;

#ifdef WASABI_COMPILE_WND
// Fix this with editwnd!
typedef void * WNDPROC;
typedef void * DRAWITEMSTRUCT;

#define VK_MENU    (XK_Alt_L | (XK_Alt_R << 16))
#define VK_MBUTTON (XK_Meta_L | (XK_Meta_R << 16))
#define VK_SHIFT   (XK_Shift_L | (XK_Shift_R << 16))
#define MK_RBUTTON ((XK_VoidSymbol << 16) | 1)
#define MK_LBUTTON ((XK_VoidSymbol << 16) | 2)
#define VK_CONTROL (XK_Control_L | (XK_Control_R << 16))
#define VK_DELETE  (XK_Delete | (XK_KP_Delete << 16))
#define VK_RETURN  (XK_Return)
#define VK_ESCAPE  (XK_Escape)
#define VK_DOWN    (XK_Down)
#define VK_UP      (XK_Up)
#define VK_LEFT    (XK_Left)
#define VK_RIGHT   (XK_Right)
#define VK_HOME    (XK_Home)
#define VK_END     (XK_End)
#define VK_PRIOR   (XK_Prior)
#define VK_NEXT    (XK_Next)
#define VK_BACK    (XK_BackSpace)
#define VK_F1      (XK_F1)
#define VK_SPACE   (XK_space)

#define INVALID_HANDLE_VALUE NULL
#endif

// Come back here later
typedef struct {
  pthread_mutex_t mutex;
  pthread_t id;
  int count;
} CRITICAL_SECTION;
typedef pthread_t HANDLE;

typedef char OLECHAR;

//#define NO_MMX
//CUT? #define NULLREGION    0

#define MAIN_MINX 0
#define MAIN_MINY 0

typedef void (*TIMERPROC)(HWND, UINT, UINT, DWORD);

typedef struct {
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD time;
  POINT pt;
} MSG;

#ifdef __cplusplus

#define _TRY try
#define _EXCEPT(x) catch(...)

#include <new>

#ifdef WASABI_COMPILE_WND

#define NULLREGION    1
#define SIMPLEREGION  2
#define COMPLEXREGION 3

#endif

// some of these are used even without wnd support

enum {
  WM_CREATE, // MULL
  WM_CLOSE, // NULL
  WM_PAINT, // NULL
  WM_NCPAINT, // NULL
  WM_SYNCPAINT, // NULL
  WM_SETCURSOR, // NULL
  WM_TIMER, // timerid
  WM_SETFOCUS, // NULL
  WM_KILLFOCUS, // NULL
  WM_LBUTTONDOWN, // xPos | yPos << 16
  WM_RBUTTONDOWN, // "
  WM_MOUSEMOVE, // "
  WM_LBUTTONUP, // "
  WM_RBUTTONUP, // "
  WM_CONTEXTMENU, // "
  WM_ERASEBKGND, // NULL
  WM_MOUSEWHEEL, // a << 16 | t (l=a/120)
  WM_CHAR, // char
  WM_KEYDOWN, // keypress
  WM_KEYUP, // "
  WM_SYSKEYDOWN, // look at OnSysKeyDown
  WM_SYSKEYUP, // "
  WM_SYSCOMMAND, // Hunh?
  WM_MOUSEACTIVATE, // Hunh?
  WM_ACTIVATEAPP, // Hunh?
  WM_ACTIVATE, // WA_ACTIVE || WA_CLICKACTIVE
  WM_NCACTIVATE, // NULL
  WM_WINDOWPOSCHANGED, // NULL, WINDOWPOS *
  WM_DROPFILES, // HDROP
  WM_CAPTURECHANGED, // NULL
  WM_COMMAND, // Commands?..
  WM_SETTINGCHANGE,
  WM_QUIT,
  WM_DESTROY,
  WM_USER = 1000, // wParam, lParam -> make sure this is last
};

#define PM_NOREMOVE         0x0000
#define PM_REMOVE           0x0001
#define PM_NOYIELD          0x0002 // ignored

#ifdef WASABI_COMPILE_WND

enum {
  WA_ACTIVE, WA_CLICKACTIVE,
};

enum {
  SC_CLOSE,
};

enum {
  DT_LEFT,
  DT_CENTER,
  DT_RIGHT,
  DT_VCENTER,
  DT_WORDBREAK,
  DT_SINGLELINE,
  DT_NOPREFIX,
  DT_PATH_ELLIPSIS,
  DT_END_ELLIPSIS,
  DT_MODIFYSTRING,
  DT_CALCRECT,

};

#define ALL_EVENTS       ExposureMask | StructureNotifyMask | \
             KeyPressMask | KeyReleaseMask | \
             ButtonPressMask | ButtonReleaseMask | \
                         PointerMotionMask | FocusChangeMask | \
                         EnterWindowMask | LeaveWindowMask


#define NO_INPUT_EVENTS  ExposureMask | StructureNotifyMask | \
                         PointerMotionMask | FocusChangeMask | \
                         EnterWindowMask | LeaveWindowMask

class Linux {
 private:
  static Display *display;
  static XContext context;

 public:
  static Display *getDisplay();
  static int getScreenNum();
  static Window RootWin();
  static Visual *DefaultVis();

  static int convertEvent( MSG *, XEvent * );
  static void initContextData( HWND h );
  static void nukeContextData( HWND h );
  static XContext getContext();
  static void setCursor( HWND h, int cursor );
};

#endif

#endif

typedef void* HINSTANCE; // Useless, just a placeholder
typedef void* HMONITOR;
typedef void* WIN32_FIND_DATA;
typedef void* WIN32_FIND_DATAW;
typedef void* BLENDFUNCTION;
typedef void* ATOM;
typedef void* HGLOBAL;
typedef void* HKEY;
typedef char* LPTSTR;
typedef char* LPCTSTR;
typedef DWORD* LPDWORD;

#if defined(WASABI_API_TIMER) || defined(WASABI_API_WND)
int GetMessage( MSG *, HWND, UINT, UINT );
int PeekMessage( MSG *, HWND, UINT, UINT, UINT );
int DispatchMessage( MSG * );
int SendMessage( HWND, UINT, WPARAM, LPARAM );
int SetTimer( HWND, int id, int ms, TIMERPROC );
void KillTimer( HWND, int id );
#endif

#ifdef WASABI_COMPILE_WND

void TranslateMessage( MSG * );
void PostMessage( HWND, UINT, WPARAM, LPARAM );
void PostQuitMessage( int );

enum contextdata {
  GWL_HINSTANCE = 0,
  GWL_USERDATA,
  GWL_INVALIDREGION,
  GWL_RECT_LEFT,
  GWL_RECT_TOP,
  GWL_RECT_RIGHT,
  GWL_RECT_BOTTOM,
  GWL_HWND,
  GWL_PARENT,

  GWL_ENUM_SIZE
};
void MoveWindowRect( HWND, int, int );
void SetWindowRect( HWND, RECT * );
int GetWindowRect( HWND, RECT * );
void SetWindowLong( HWND, enum contextdata, LONG );
LONG GetWindowLong( HWND, enum contextdata );
int GetUpdateRect( HWND, RECT *, BOOL );
void GetUpdateRgn( HWND, HRGN, BOOL );
void InvalidateRgn( HWND, HRGN, BOOL );
void InvalidateRect( HWND, const RECT *, BOOL );
void ValidateRgn( HWND, HRGN );
void ValidateRect( HWND, const RECT * );
HWND GetActiveWindow();
HWND WindowFromPoint( POINT p );
#endif

void OutputDebugString( const char *s );
int MulDiv( int m1, int m2, int d );
DWORD GetTickCount();
void Sleep( int ms );
int IntersectRect(RECT *, const RECT *, const RECT *);
void ExitProcess( int ret );
DWORD GetModuleFileName(void *pid, const char *filename, int bufsize);
const char *CharPrev(const char *lpszStart, const char *lpszCurrent);

int SubtractRect( RECT *out, RECT *in1, RECT *in2 );
int EqualRect( RECT *a, RECT *b );


void CopyFile( const char *f1, const char *f2, BOOL b );

#endif

