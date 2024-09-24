#include <precomp.h>
#include <wasabicfg.h>
#include "animate.h"

#include <api/config/items/cfgitem.h>

//---------------------------------------------------------------------------
void AnimatedRects::draw(const RECT *source, const RECT *dest, int steps)
{
#ifdef WASABI_COMPILE_CONFIG
  // {280876CF-48C0-40bc-8E86-73CE6BB462E5}
  const GUID options_guid = 
  { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
  if (!_intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid), L"Animated rects")) return;
#else
  if (!WASABI_WNDMGR_ANIMATEDRECTS) return;
#endif

     //FG> Not anymore, old code, old bugs
     //BU you're so cool, Francis
     //FG> thank you, you're not too bad either :)
  int sizex=source->right-source->left-(dest->right-dest->left);
  int sizey=source->bottom-source->top-(dest->bottom-dest->top);
  int diffx=source->left-dest->left;
  int diffy=(source->top)-dest->top;

#ifdef WIN32
  HDC dc;
  dc=GetDC(0);
  HBRUSH brush = CreateSolidBrush(0xFFFFFF);
	HPEN pen = CreatePen(PS_SOLID,0,0xFFFFFF);
	HBRUSH obrush = (HBRUSH)SelectObject(dc, brush);
  HPEN open = (HPEN)SelectObject(dc, pen);
  int oldrop = SetROP2(dc,R2_XORPEN);
#endif
#ifdef LINUX
  HDC dc = (HDC)MALLOC( sizeof( hdc_typ ) );
  XGCValues gcv;
  gcv.foreground = 0xffffff;
  gcv.function = GXxor;
  gcv.subwindow_mode = IncludeInferiors;
  dc->gc = XCreateGC( Linux::getDisplay(), Linux::RootWin(), GCForeground | GCFunction | GCSubwindowMode, &gcv );
#endif
//PORTME

  for(int i=0;i<steps;i++) {
    int x=dest->left+diffx-((diffx*i)/steps);
    int y=dest->top+diffy-((diffy*i)/steps);
    int maxx=(source->right-source->left)-((sizex*i)/steps);
    int maxy=(source->bottom-source->top)-((sizey*i)/steps);

#ifdef WIN32
    int p1x=x,p1y=y;
    int p2x=x+maxx,p2y=y;
    int p3x=x+maxx,p3y=y+maxy;
    int p4x=x,p4y=y+maxy;
    MoveToEx(dc,p1x,p1y,NULL);
    LineTo(dc,p2x,p2y);
    LineTo(dc,p3x,p3y);
    LineTo(dc,p4x,p4y);
    LineTo(dc,p1x,p1y);
#endif
#ifdef LINUX
    XDrawRectangle( Linux::getDisplay(), Linux::RootWin(), dc->gc,
                    x, y, maxx, maxy );
#endif
//PORTME
    Wasabi::Std::usleep(5);
#ifdef WIN32
    MoveToEx(dc,p1x,p1y,NULL);
    LineTo(dc,p2x,p2y);
    LineTo(dc,p3x,p3y);
    LineTo(dc,p4x,p4y);
    LineTo(dc,p1x,p1y);
#endif
#ifdef LINUX
    XDrawRectangle( Linux::getDisplay(), Linux::RootWin(), dc->gc,
                    x, y, maxx, maxy );
#endif
//PORTME
  }
#ifdef WIN32
  SetROP2(dc, oldrop);
	SelectObject(dc, open);
	SelectObject(dc, obrush);
	DeleteObject(brush);
	DeleteObject(pen);
  ReleaseDC(0,dc);
#endif
#ifdef LINUX
  XFreeGC( Linux::getDisplay(), dc->gc );
  FREE( dc );
#endif
//PORTME
}

