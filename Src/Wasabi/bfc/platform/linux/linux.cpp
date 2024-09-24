#include <precomp.h>

#include <api/api.h>
  #include <api/linux/api_linux.h>

#include <bfc/ptrlist.h>
#include <bfc/string/string.h>
#include <bfc/critsec.h>
#include <bfc/thread.h>
#include <api/application/ipcs.h>

#ifdef WASABI_COMPILE_WND
Display *Linux::display = NULL;

int linux_atoms_loaded = 0;

Atom winamp_msg;
Atom dnd_enter, dnd_position, dnd_status, dnd_leave, dnd_drop, dnd_finished;
Atom dnd_selection, dnd_wa3drop, dnd_private, dnd_typelist;
Atom dnd_urilist, dnd_textplain, dnd_mozurl;
#endif

#ifndef _NOSTUDIO

#ifdef WASABI_COMPILE_WND
void LoadAtoms() {
  if ( !linux_atoms_loaded ) {
    linux_atoms_loaded = 1;
    winamp_msg = XInternAtom( Linux::getDisplay(), "Winamp3", False );
    dnd_wa3drop = XInternAtom( Linux::getDisplay(), "Winamp3_drop", False );
    dnd_enter = XInternAtom( Linux::getDisplay(), "XdndEnter", True );
    dnd_position = XInternAtom( Linux::getDisplay(), "XdndPosition", True );
    dnd_status = XInternAtom( Linux::getDisplay(), "XdndStatus", True );
    dnd_leave = XInternAtom( Linux::getDisplay(), "XdndLeave", True );
    dnd_drop = XInternAtom( Linux::getDisplay(), "XdndDrop", True );
    dnd_finished = XInternAtom( Linux::getDisplay(), "XdndFinished", True );
    dnd_selection = XInternAtom( Linux::getDisplay(), "XdndSelection", True );
    dnd_private = XInternAtom( Linux::getDisplay(), "XdndActionPrivate", True );
    dnd_typelist = XInternAtom( Linux::getDisplay(), "XdndTypeList", True );
    dnd_urilist = XInternAtom( Linux::getDisplay(), "text/uri-list", True );
    dnd_textplain = XInternAtom( Linux::getDisplay(), "text/plain", True );
    dnd_mozurl = XInternAtom( Linux::getDisplay(), "text/x-moz-url", True );
  }
}
#endif

#endif

void OutputDebugString( const char *s ) {
#ifdef _DEBUG
  fprintf( stderr, "%s", s );
#endif
  char *file = getenv( "WASABI_LOG_FILE" );
  if ( file ) {
    if ( !STRCMP( file, "-" ) ) {
      fprintf( stdout, "%s", s );
    } else {
      FILE *f = fopen( file, "a" );
      if ( f ) {
        fprintf( f, "%s", s );
        fclose( f );
      }
    }
  }
}

DWORD GetTickCount() {
  static int starttime = -1;

  if ( starttime == -1 )
    starttime = time( NULL );

  struct timeb tb;
  ftime( &tb );
  tb.time -= starttime;
  return tb.time * 1000 + tb.millitm;
}
void Sleep( int ms ) {
  if ( ms != 0 ) {
    struct timespec ts = { 0, 0 };
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep( &ts, NULL);
    //    usleep(ms * 1000);
  }
}

#ifndef _NOSTUDIO

#ifdef WASABI_COMPILE_WND

Display *Linux::getDisplay() {
  if ( ! display )
    display = WASABI_API_LINUX->linux_getDisplay();

  return display;
}

XContext Linux::getContext() {
  static XContext context = 0;

  if ( context == 0 )
    context = WASABI_API_LINUX->linux_getContext();

  return context;
}

int Linux::getScreenNum() { return DefaultScreen( getDisplay() ); }

Window Linux::RootWin() {
  return RootWindow( getDisplay(), getScreenNum() );
}
Visual *Linux::DefaultVis() {
  return DefaultVisual( getDisplay(), getScreenNum() );
}

void Linux::setCursor( HWND h, int cursor ) {
  Cursor c = XCreateFontCursor( Linux::getDisplay(), cursor );

  if ( cursor == None )
    XUndefineCursor( Linux::getDisplay(), h );
  else
    XDefineCursor( Linux::getDisplay(), h, c );

  XFreeCursor( Linux::getDisplay(), c );
}

int Linux::convertEvent( MSG *m, XEvent *e ) {
  m->hwnd = e->xany.window;

  if ( m->hwnd ) {
    api_window *rw =(api_window *)GetWindowLong( m->hwnd, GWL_USERDATA );
    if ( !rw ) {
      // This is to fix messages for dead windows...
      return 0;
    }
  }

  switch ( e->type ) {
  case ButtonPress:
    switch( e->xbutton.button ) {
    case 1:
      m->message = WM_LBUTTONDOWN;
      m->lParam = (e->xbutton.x & 0xffff) | (e->xbutton.y << 16);
      break;

    case 2:

    case 3:
      m->message = WM_RBUTTONDOWN;
      m->lParam = (e->xbutton.x & 0xffff) | (e->xbutton.y << 16);
      break;

    case 4:
      m->message = WM_MOUSEWHEEL;
      m->wParam = 120 << 16 | 0; // 1 tick, no modifiers
      m->lParam = (e->xbutton.x & 0xffff) | (e->xbutton.y << 16);
      break;

    case 5:
      m->message = WM_MOUSEWHEEL;
      m->wParam = (-120) << 16 | 0; // 1 tick, no modifiers
      m->lParam = (e->xbutton.x & 0xffff) | (e->xbutton.y << 16);
      break;
    }
    break;
  case ButtonRelease:
    switch( e->xbutton.button ) {
    case 1:
      m->message = WM_LBUTTONUP;
      m->lParam = (e->xbutton.x & 0xffff) | (e->xbutton.y << 16);
      break;

    case 2:

    case 3:
      m->message = WM_RBUTTONUP;
      m->lParam = (e->xbutton.x & 0xffff)  | (e->xbutton.y << 16);
      break;
    }
    break;
  case MotionNotify:
    {
      m->message = WM_MOUSEMOVE;
      do {
  // Spin...
      } while( XCheckTypedWindowEvent( Linux::getDisplay(), m->hwnd, MotionNotify, e ) );

      RECT r;
      POINT offset = {0, 0};
      HWND hwnd = m->hwnd;

      GetWindowRect( hwnd, &r );

      m->lParam = ((e->xmotion.x_root - r.left) & 0xffff) |
                 ((e->xmotion.y_root - r.top) << 16);

      if ( ! (e->xmotion.state & ( Button1Mask | Button2Mask | Button3Mask )) )
  PostMessage( m->hwnd, WM_SETCURSOR, m->hwnd, 0 );
    }
    break;

  case KeyPress:
    m->message = WM_KEYDOWN;
    m->wParam = e->xkey.keycode;
    break;

  case KeyRelease:
    m->message = WM_KEYUP;
    m->wParam = e->xkey.keycode;
    break;

  case Expose:
    {
      RECT r;
      m->message = WM_PAINT;
      do {
  r.left = e->xexpose.x;
  r.top = e->xexpose.y;
  r.right = r.left + e->xexpose.width;
  r.bottom = r.top + e->xexpose.height;
  InvalidateRect( m->hwnd, &r, FALSE );
      } while( XCheckTypedWindowEvent( Linux::getDisplay(), m->hwnd, Expose, e ) );
    }
    break;

  case ClientMessage: {
    static int coord = -1;
    static Atom supported = None;
    XClientMessageEvent cme;

    LoadAtoms();

    int message = e->xclient.message_type;
    if ( message == dnd_enter ) {
      if ( e->xclient.data.l[1] & 1 ) {
  Atom actual;
  int format;
  long unsigned int nitems, bytes;
  unsigned char *data = NULL;

  XGetWindowProperty( Linux::getDisplay(), e->xclient.data.l[0],
          dnd_typelist, 0, 65536, True, XA_ATOM,
          &actual, &format, &nitems, &bytes, &data );

  Atom *atomdata = (Atom *)data;

  supported = None;
  for( int i = 0; i < nitems; i++ ) {
    if ( atomdata[i] == dnd_urilist ) {
      supported = dnd_urilist;
    }
  }
  if ( supported == None ) {
    for( int i = 0; i < nitems; i++ ) {
      if ( atomdata[i] == dnd_textplain ) {
        OutputDebugString( "text/plain found\n" );
        supported = dnd_textplain;
      }
    }
  }
  if ( supported == None ) {
    for( int i = 0; i < nitems; i++ ) {
      if ( atomdata[i] == dnd_mozurl ) {
        supported = dnd_mozurl;
      }
    }
  }

  XFree( data );
      } else {
  if ( e->xclient.data.l[2] == dnd_urilist ||
       e->xclient.data.l[3] == dnd_urilist ||
       e->xclient.data.l[4] == dnd_urilist ) {
    supported = dnd_urilist;
  } else if ( e->xclient.data.l[2] == dnd_mozurl ||
        e->xclient.data.l[3] == dnd_mozurl ||
        e->xclient.data.l[4] == dnd_mozurl ) {
    supported = dnd_mozurl;
  }
      }


      // DnD Enter
      return 0;

    } else if ( message == dnd_position ) {
      // DnD Position Notify

      cme.type = ClientMessage;
      cme.message_type = dnd_status;
      cme.format = 32;
      cme.window = e->xclient.data.l[0];
      cme.data.l[0] = e->xclient.window;
      cme.data.l[1] = 1; // Can Accept
      cme.data.l[2] = cme.data.l[3] = 0; // Empty rectangle - give us moves
      cme.data.l[4] = dnd_private; // We're doing our own thing

      if ( coord == -1 && supported != None ) {
  XConvertSelection( Linux::getDisplay(), dnd_selection, supported,
         dnd_wa3drop, cme.window, CurrentTime );
      }

      coord = e->xclient.data.l[2];

      XSendEvent( Linux::getDisplay(), e->xclient.data.l[0], False,
      NoEventMask, (XEvent *)&cme );

      return 0;

    } else if ( message == dnd_leave ) {
      // DnD Leave
      coord = -1;
      supported = None;

      return 0;

    } else if ( message == dnd_drop ) {
      // DnD Drop

      Window win = e->xclient.data.l[0];

      cme.type = ClientMessage;
      cme.message_type = dnd_finished;
      cme.format = 32;
      cme.window = e->xclient.data.l[0];
      cme.data.l[0] = e->xclient.window;
      cme.data.l[1] = cme.data.l[2] = cme.data.l[3] = cme.data.l[4] = 0;

      XSendEvent( Linux::getDisplay(), e->xclient.data.l[0], False,
      NoEventMask, (XEvent *)&cme );

      if ( supported != None ) {
  Atom actual;
  int format;
  long unsigned int nitems, bytes;
  unsigned char *data = NULL;

  XGetWindowProperty( Linux::getDisplay(), cme.window, dnd_wa3drop,
          0, 65536, True, supported, &actual,
          &format, &nitems, &bytes,
          &data );

  OutputDebugString( StringPrintf( "Drop data (%d):\n%s\n", nitems, data ) );

  m->message = WM_DROPFILES;
  m->wParam = coord;
  m->lParam = (LPARAM)data;

  coord = -1;
  supported = None;

      } else {
  coord = -1;
  supported = None;
  return 0;
      }

      break;

    } else if ( message == winamp_msg ) {
      // Internal Message ...

      m->message = e->xclient.data.l[0];
      m->wParam = e->xclient.data.l[1];
      m->lParam = e->xclient.data.l[2];
      break;

    } else {
      return 0;
    }
    break;
  }

  case LeaveNotify:
  case EnterNotify:
    m->message = WM_MOUSEMOVE;
    m->lParam = (e->xcrossing.x & 0xffff) | (e->xcrossing.y << 16);

    if ( ! (e->xcrossing.state & ( Button1Mask | Button2Mask | Button3Mask )) )
      PostMessage( m->hwnd, WM_SETCURSOR, m->hwnd, 0 );
    break;

  case FocusIn:
    m->message = WM_SETFOCUS;
    break;

  case FocusOut:
    m->message = WM_KILLFOCUS;
    break;

  default:
    return 0;
  }

  return 1;
}

static HWND activeWindow;

HWND GetActiveWindow() {
  return activeWindow;
}

int IntersectRect( RECT *out, const RECT *i1, const RECT *i2 ) {
  return Std::rectIntersect(i1, i2, out);
}

void TranslateMessage( MSG *m ) {
  if ( m->message != WM_CHAR && m->message != WM_KEYDOWN &&
       m->message != WM_KEYUP )
    return;

  int index = !!( Std::keyDown( VK_SHIFT ));

  m->wParam = XKeycodeToKeysym( Linux::getDisplay(), m->wParam, index );
}

void PostMessage( HWND win, UINT msg, WPARAM wParam, LPARAM lParam ) {
  XEvent e;

  LoadAtoms();

  e.type = ClientMessage;
  e.xclient.window = win;
  e.xclient.message_type = winamp_msg;
  e.xclient.format = 32;
  e.xclient.data.l[0] = msg;
  e.xclient.data.l[1] = wParam;
  e.xclient.data.l[2] = lParam;

  XSendEvent( Linux::getDisplay(), win, FALSE, NoEventMask, &e );
}

void PostQuitMessage( int i ) {
  PostMessage( None, WM_QUIT, i, 0 );
}

#endif // wnd

#if defined(WASABI_API_TIMER) | defined(WASABI_API_WND)

struct TimerElem {
  HWND win;
  int id;
  int nexttime;
  int delta;
  TIMERPROC tproc;

  TimerElem( HWND _win, int _id, int ms, TIMERPROC _tproc ) {
    win = _win;
    id = _id;
    delta = ms;
    tproc = _tproc;
    nexttime = Std::getTickCount() + delta;
  }
};

int timer_id = 0;
CriticalSection timer_cs;
PtrList<TimerElem> timer_elems;

int SetTimer( HWND win, int id, int ms, TIMERPROC tproc ) {
  KillTimer(win, id);

  if ( win == (HWND)0 ) {
    id = timer_id++;
  }

  TimerElem *te = new TimerElem( win, id, ms, tproc );
  timer_cs.enter();
  timer_elems.addItem( te, PTRLIST_POS_LAST );
  timer_cs.leave();

  return id;
}

void KillTimer( HWND win, int id ) {
  timer_cs.enter();
  for( int i = 0; i < timer_elems.getNumItems(); i++ )
    if ( timer_elems[i]->win == win && timer_elems[i]->id == id ) {
      delete timer_elems[i];
      timer_elems.delByPos( i );
      i--;
    }
  timer_cs.leave();
}

CriticalSection send_cs;
MSG *send_msg;
int sending = 0;
int send_ret;
pthread_t message_thread = (pthread_t)-1;

int _GetMessage( MSG *m, HWND, UINT, UINT, int block=1) {
  MEMSET( m, 0, sizeof( MSG ) );

  message_thread = pthread_self();

#ifdef WASABI_COMPILE_WND
  XEvent e;
#endif // wnd
  int curtime;
  int done = 0;
  int first = 1;
  static wa_msgbuf ipcm;
  static int qid = -1;
  int size;

  if ( qid == -1 ) { qid = WASABI_API_LINUX->linux_getIPCId(); }

  if ( sending ) {
    *m = *send_msg;
    done = 1;
  }

  while( !done && (block || first)) {
    if ( qid != -1 ) {
      if ( (size = msgrcv( qid, &ipcm, IPC_MSGMAX , 0, IPC_NOWAIT )) != -1 ) {
        m->hwnd = None;
        m->message = WM_WA_IPC;
        m->wParam = (WPARAM)&ipcm;
        break;
      }
    }

    curtime = GetTickCount();

    timer_cs.enter();
    for( int i = 0; i < timer_elems.getNumItems(); i++ ) {
      if ( timer_elems[i]->nexttime < curtime ) {
        if (block)
          while( timer_elems[i]->nexttime < curtime )
            timer_elems[i]->nexttime += timer_elems[i]->delta;

        m->hwnd    = timer_elems[i]->win;
        m->message = WM_TIMER;
        m->wParam  = (WPARAM)timer_elems[i]->id;
        m->lParam  = (LPARAM)timer_elems[i]->tproc;

        done = 1;
      }
    }
    timer_cs.leave();

    if ( !done && ! first )
      Sleep( 1 );
    else
      first = 0;

#ifdef WASABI_API_WND
    if ( !done && XPending( Linux::getDisplay() ) ) {
      int n = XEventsQueued( Linux::getDisplay(), QueuedAlready );

      for ( int i = 0; !done && i < n; i++ ) {
        XNextEvent( Linux::getDisplay(), &e );
        if ( Linux::convertEvent( m, &e ) )
          done = 1;
      }
      if ( done )
        break;
    }
#endif // wnd
  }

#ifdef WASABI_API_WND
  activeWindow = m->hwnd;
#endif // wnd

  return m->message != WM_QUIT;
}

int GetMessage( MSG *m, HWND w, UINT f, UINT l) {
  return _GetMessage(m, w, f, l, 1);
}

// on linux, we don't really simply peek when PM_NOREMOVE is used,
// we just don't block, which is the only thing we want to accomplish here
int PeekMessage( MSG *m, HWND w, UINT f, UINT l, UINT remove) {
  if (remove == PM_NOREMOVE) return _GetMessage(m, w, f, l, 0);
  else _GetMessage(m, w, f, l, 1);
}


int DispatchMessage( MSG *m ) {
  if ( m->message == WM_TIMER && m->hwnd == None ) {
    TIMERPROC tproc = (TIMERPROC)m->lParam;
    tproc( m->hwnd, m->message, m->wParam, 0 );
    return 1;
  }

  int ret = 0;

#ifdef WASABI_COMPILE_WND
  api_window *rootwnd = (api_window *)GetWindowLong( m->hwnd, GWL_USERDATA );

  if ( rootwnd ) {
    ret = rootwnd->wndProc( m->hwnd, m->message, m->wParam, m->lParam );
    rootwnd->performBatchProcesses();
  }
#endif // wnd

  if ( sending ) {
    send_ret = ret;
    sending = 0;
  }

  return ret;
}

int SendMessage( HWND win, UINT msg, WPARAM wParam, LPARAM lParam ) {
  MSG m;
  m.hwnd = win;
  m.message = msg;
  m.wParam = wParam;
  m.lParam = lParam;

  int ret;

  if ( pthread_equal( message_thread, pthread_self() ) ) {
    return DispatchMessage( &m );

  } else {
    send_cs.enter();
    sending = 1;
    send_msg = &m;
    while( sending ) { Sleep( 1 ); }
    ret = send_ret;
    send_cs.leave();

    return ret;
  }
}

#endif // timer | wnd

int MulDiv( int m1, int m2, int d ) {
  __asm__ volatile (
        "mov %0, %%eax\n"
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mul %%ebx\n"
        "div %%ecx\n"
        : : "m" (m1), "m" (m2), "m" (d)
        : "%eax", "%ebx", "%ecx", "%edx" );
}

void ExitProcess( int ret ) {
  exit( ret );
}

#ifdef WASABI_COMPILE_WND

void Linux::initContextData( HWND h ) {
  int *data;
  XPointer xp;

  ASSERT( XFindContext( Linux::getDisplay(), h, Linux::getContext(), &xp ));

  data = (int *)MALLOC( GWL_ENUM_SIZE * sizeof( int ) );

  data[GWL_HWND] = h;

  XSaveContext( Linux::getDisplay(), h, Linux::getContext(), (char *)data );
}

void Linux::nukeContextData( HWND h ) {
  int *data;
  XPointer xp;

  if ( XFindContext( Linux::getDisplay(), h, Linux::getContext(), &xp ) )
    return;

  data = (int *)xp;

  ASSERT( data[GWL_HWND] == h );

  if ( data[GWL_INVALIDREGION] ) {
    XDestroyRegion( (HRGN)data[GWL_INVALIDREGION] );
  }

  XDeleteContext( Linux::getDisplay(), h, Linux::getContext() );

  FREE( data );
}

void SetWindowLong( HWND h, contextdata type, LONG value ) {
  XPointer data;

  if ( XFindContext( Linux::getDisplay(), h, Linux::getContext(), &data ) )
    return;

  ASSERT( ((int *)data)[GWL_HWND] == h );

  ((int*)data)[type] = value;
}

LONG GetWindowLong( HWND h, contextdata type ) {
  XPointer data;

  if ( XFindContext( Linux::getDisplay(), h, Linux::getContext(), &data ) )
    return 0;

  ASSERT( ((int *)data)[GWL_HWND] == h );

  return ((int*)data)[type];
}

void MoveWindowRect( HWND h, int x, int y ) {
  XPointer xp;
  int *data;

  if ( XFindContext( Linux::getDisplay(), h, Linux::getContext(), &xp ) )
    return;

  data = (int *)xp;

  ASSERT( data[GWL_HWND] == h );

  data[GWL_RECT_RIGHT]  -= data[GWL_RECT_LEFT] - x;
  data[GWL_RECT_BOTTOM] -= data[GWL_RECT_TOP] - y;
  data[GWL_RECT_LEFT]   = x;
  data[GWL_RECT_TOP]    = y;
}

void SetWindowRect( HWND h, RECT *r ) {
  int *data;
  XPointer xp;

  if ( XFindContext( Linux::getDisplay(), h, Linux::getContext(), &xp ) )
    return;

  data = (int *)xp;

  ASSERT( data[GWL_HWND] == h );

  data[GWL_RECT_LEFT]   = r->left;
  data[GWL_RECT_TOP]    = r->top;
  data[GWL_RECT_RIGHT]  = r->right;
  data[GWL_RECT_BOTTOM] = r->bottom;
}

int GetWindowRect( HWND h, RECT *r ) {
  int *data;
  XPointer xp;

  if ( XFindContext( Linux::getDisplay(), h, Linux::getContext(), &xp ) )
    return 0;

  data = (int *)xp;

  ASSERT( data[GWL_HWND] == h );

  r->left   = data[GWL_RECT_LEFT];
  r->top    = data[GWL_RECT_TOP];
  r->right  = data[GWL_RECT_RIGHT];
  r->bottom = data[GWL_RECT_BOTTOM];

  POINT offset = { 0, 0};
  while( (h = data[GWL_PARENT]) != Linux::RootWin() ) {
    if ( XFindContext( Linux::getDisplay(), h, Linux::getContext(), &xp ) )
      return 0;

    data = (int *)xp;

    ASSERT( data[GWL_HWND] == h );

    offset.x += data[GWL_RECT_LEFT];
    offset.y += data[GWL_RECT_TOP];
  }
  r->left += offset.x;
  r->top += offset.y;
  r->right += offset.x;
  r->bottom += offset.y;

  return 1;
}

int GetUpdateRect( HWND h, RECT *ret, BOOL ) {
  HRGN invalid = (HRGN)GetWindowLong( h, GWL_INVALIDREGION );
  if ( ! invalid || XEmptyRegion( invalid ) )
    return 0;

  XRectangle xr;
  XClipBox( invalid, &xr );
  ret->left = xr.x;
  ret->top = xr.y;
  ret->right = xr.x + xr.width;
  ret->bottom = xr.y + xr.height;

  return 1;
}

void GetUpdateRgn( HWND h, HRGN r, BOOL ) {
  XSubtractRegion( r, r, r );

  HRGN invalid = (HRGN)GetWindowLong( h, GWL_INVALIDREGION );
  if ( ! invalid ) return;


  XUnionRegion( r, invalid, r );

  XRectangle xr;

  RECT rct;
  GetWindowRect( h, &rct );
  xr.x = 0;
  xr.y = 0;
  xr.width = rct.right - rct.left;
  xr.height = rct.bottom - rct.top;

  HRGN tmp = XCreateRegion();

  XUnionRectWithRegion( &xr, tmp, tmp );
  XIntersectRegion( r, tmp, r );
  XDestroyRegion( tmp );
}

void InvalidateRect( HWND h, const RECT *r, BOOL ) {
  HRGN invalid = (HRGN)GetWindowLong( h, GWL_INVALIDREGION );
  if ( ! invalid ) {
    invalid = XCreateRegion();
    SetWindowLong( h, GWL_INVALIDREGION, (LONG)invalid );
  }

  XRectangle xr;
  if ( r == NULL ) {
    RECT rct;
    GetWindowRect( h, &rct );
    xr.x = 0;
    xr.y = 0;
    xr.width = rct.right - rct.left;
    xr.height = rct.bottom - rct.top;
  } else {
    xr.x = r->left;
    xr.y = r->top;
    xr.width = r->right - r->left;
    xr.height = r->bottom - r->top;
  }

  XUnionRectWithRegion( &xr, invalid, invalid );

  PostMessage( h, WM_PAINT, 0, 0 );
}

void InvalidateRgn( HWND h, HRGN r, BOOL ) {
  HRGN invalid = (HRGN)GetWindowLong( h, GWL_INVALIDREGION );

  if ( ! invalid ) {
    invalid = XCreateRegion();
    SetWindowLong( h, GWL_INVALIDREGION, (LONG)invalid );
  }

  ASSERT( r != invalid );
  XUnionRegion( invalid, r, invalid );

  PostMessage( h, WM_PAINT, 0, 0 );
}

void ValidateRect( HWND h, const RECT *r ) {
  HRGN invalid = (HRGN)GetWindowLong( h, GWL_INVALIDREGION );
  if ( ! invalid ) return;

  XRectangle xr;
  if ( r == NULL ) {
    XDestroyRegion( invalid );
    SetWindowLong( h, GWL_INVALIDREGION, 0 );
    return;
  }

  xr.x = r->left;
  xr.y = r->top;
  xr.width = r->right - r->left;
  xr.height = r->bottom - r->top;

  HRGN tmp = XCreateRegion();
  XUnionRectWithRegion( &xr, tmp, tmp );
  XSubtractRegion( invalid, tmp, invalid );
  XDestroyRegion( tmp );
}

void ValidateRgn( HWND h, HRGN r ) {
  HRGN invalid = (HRGN)GetWindowLong( h, GWL_INVALIDREGION );
  if ( ! invalid ) return;

  ASSERT( r != invalid );
  XSubtractRegion( invalid, r, invalid );
}
#endif // wnd

int SubtractRect( RECT *out, RECT *in1, RECT *in2 ) {
  int ret;
  if ( in1->left >= in2->left && in1->right <= in2->right ) {
    out->left = in1->left; out->right = in1->right;

    if ( in1->top >= in2->top && in2->bottom >= in2->top && in2->bottom <= in2->bottom ) {
      out->top = in1->bottom; out->bottom = in2->bottom;

      ret = 1;
    } else if ( in1->top <= in2->top && in1->bottom >= in2->top && in1->bottom <= in2->bottom ) {
      out->top = in1->top; out->bottom = in2->top;

      ret = 1;
    } else {
      ret = 0;
    }

  } else if ( in1->top >= in2->top && in1->bottom <= in2->bottom ) {
    out->top = in1->top; out->bottom = in1->bottom;

    if ( in1->left >= in2->left && in2->right >= in2->left && in2->right <= in2->right ) {
      out->left = in1->right; out->right = in2->right;

      ret = 1;
    } else if ( in1->left <= in2->left && in1->right >= in2->left && in1->right <= in2->right ) {
      out->left = in1->left; out->right = in2->left;

      ret = 1;
    } else {
      ret = 0;
    }


  } else {
    ret = 0;
  }
  return ret;
}

int EqualRect( RECT *a, RECT *b ) {
  return ( a->top == b->top && a->bottom == b->bottom &&
     a->left == b->left && a->right == b->right );
}

#ifdef WASABI_COMPILE_WND

HWND WindowFromPoint( POINT p ) {
  int x, y;
  Window child;

  XTranslateCoordinates( Linux::getDisplay(), Linux::RootWin(), Linux::RootWin(), p.x, p.y, &x, &y, &child );
  return child;
}
#endif // wnd

void CopyFile( const char *f1, const char *f2, BOOL b ) {
  COPYFILE( f1, f2 );
}

DWORD GetModuleFileName(void *pid, const char *filename, int bufsize) {
  char procbuffer[512];
  sprintf(procbuffer, "/proc/%d/exe", (int)pid);
  return readlink(procbuffer, (char *)filename, bufsize);
}

const char *CharPrev(const char *lpszStart, const char *lpszCurrent) {
  if (lpszCurrent-1 >= lpszStart) return lpszCurrent-1;
  return lpszStart;
}

#endif
