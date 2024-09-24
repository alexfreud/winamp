// ============================================================================================================================================================
// Font abstract class + statics to install TT fonts and Bitmap fonts
// ============================================================================================================================================================

// what other linux headers need be made?

#include "truetypefont_linux.h"

#include <tataki/canvas/ifc_canvas.h>
#include "../../bitmap.h"

// ============================================================================================================================================================
// TrueTypeFont_Linux implementation. 
// ============================================================================================================================================================

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
TrueTypeFont_Linux::TrueTypeFont_Linux() {
  font = NULL;
  antialias_canvas = NULL;
  DColdstate = NULL;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
TrueTypeFont_Linux::~TrueTypeFont_Linux() {
  ASSERT(fontstack.isempty());
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Linux::isBitmap() {
  return 0;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::addFontResource(FILE *in){
  ASSERT(in != NULL);
  OutputDebugString( "portme -- TrueTypeFont_Linux::addFontResource\n" );
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::setFontFace(const char *face) {
  face_name = face;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
const char *TrueTypeFont_Linux::getFaceName() {
  return face_name;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::prepareCanvas(ifc_canvas *c, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor) {
  String fontname = StringPrintf( "-*-%s-%s-%s-*--%d-*-*-*-*-*-*-*",
				  (const char *)face_name, 
				  bold?"bold":"medium",
				  italic?"i":"r", size * 3/4 );
  font = XLoadQueryFont( Linux::getDisplay(), (const char *)fontname );
  if ( font == NULL ) {
    fontname = StringPrintf( "-*-arial-%s-%s-*--%d-*-*-*-*-*-*-*", 
			     bold?"bold":"medium",
			     italic?"i":"r", size * 3/4 );
    font = XLoadQueryFont( Linux::getDisplay(), (const char *)fontname );

    if ( font == NULL ) {
      fontname = StringPrintf( "-*-courier-%s-%s-*--%d-*-*-*-*-*-*-*", 
			       bold?"bold":"medium",
			       italic?"i":"r", size * 3/4 );
      font = XLoadQueryFont( Linux::getDisplay(), (const char *)fontname );
    } 
  } 
  ASSERTPR( font != NULL, fontname );
  XSetFont( Linux::getDisplay(), c->getHDC()->gc, font->fid );
  XSetForeground( Linux::getDisplay(), c->getHDC()->gc, color );
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::restoreCanvas(ifc_canvas *c) { 
  if ( font != NULL ) {
    XFreeFont( Linux::getDisplay(), font );
    font = NULL;
  }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
ifc_canvas *TrueTypeFont_Linux::prepareAntialias(ifc_canvas *c, int x, int y, const char *txt, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int w, int h) {
  ASSERT(antialias_canvas == NULL);
  BaseCloneCanvas canvas(c);
  prepareCanvas(&canvas, size, bold, opaque, underline, italic, color, bkcolor);
  al_w = MAX(2,canvas.getTextWidth(txt) * 2 + xoffset*2);
  al_h = MAX(2,canvas.getTextHeight()*2 + yoffset*2);
  restoreCanvas(&canvas);
  if (w != -1) {
    al_w = w * 2;
    al_dw = w;
  } else al_dw = w;
  if (h != -1) {
    al_h = h * 2;
    al_dh = h;
  } else al_dh = h;
  al_mask=RGB(0,0,0);
  if (color == al_mask) al_mask=RGB(255,255,255);
  antialias_canvas = new BltCanvas(al_w, al_h);
  antialias_canvas->fillBits(0);
  prepareCanvas(antialias_canvas, size*2, bold, opaque, underline, italic, color, bkcolor);
  if (al_mask != 0) antialias_canvas->fillBits(al_mask);
  al_x = x; al_y = y; al_xo = xoffset; al_yo = yoffset;
  return antialias_canvas;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::completeAntialias(ifc_canvas *c) {
  BaseCloneCanvas canvas(c);
  antialias_canvas->maskColor(al_mask, RGB(0,0,0));
  BltCanvas *ac = new BltCanvas(al_w/2, al_h/2);
  antialias_canvas->antiAliasTo(ac, al_w/2, al_h/2, 2);
  SkinBitmap *b = ac->getSkinBitmap();
  RECT src={0,0,al_w/2,al_h/2};
  RECT dst={al_x+al_xo,al_y+al_yo,al_x+al_xo+al_dw,al_y+al_yo+al_dh};
  b->blitToRect(&canvas, &src, &dst);
  delete ac;
  restoreCanvas(antialias_canvas);
  delete antialias_canvas;
  antialias_canvas = NULL;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::textOut(ifc_canvas *c, int x, int y, const char *txt, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased) {
  if (antialiased) {
    ifc_canvas *canvas = prepareAntialias(c, x, y, txt, size, bold, opaque, underline, italic, color, bkcolor, xoffset, yoffset, -1, -1);

    HDC hdc = c->getHDC();
    XDrawString( Linux::getDisplay(), hdc->d, hdc->gc, 0, 0, txt, STRLEN(txt) );

    completeAntialias(c);
  } else {
    prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor);

    int dir, ascent, descent;
    XCharStruct overall;
    XTextExtents( font, txt, STRLEN( txt ), &dir, &ascent, &descent, &overall );

    HDC hdc = c->getHDC();
    XDrawString( Linux::getDisplay(), hdc->d, hdc->gc, x+xoffset, y+yoffset+ascent, txt, STRLEN(txt) );


    restoreCanvas(c);
  }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::textOut(ifc_canvas *c, int x, int y, int w, int h, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased) {
  if (antialiased) {
    ifc_canvas *canvas = prepareAntialias(c, x, y, txt, size, bold, opaque, underline, italic, color, bkcolor, xoffset, yoffset, w, h);
    RECT al_r={0,0,w*2,h*2};

    HDC hdc = c->getHDC();
    XDrawString( Linux::getDisplay(), hdc->d, hdc->gc, al_r.left, al_r.top, txt, STRLEN(txt) );

    completeAntialias(c);
  } else {
    RECT r;
    r.left = x+xoffset;
    r.top = y+yoffset;
    r.right = r.left + w;
    r.bottom = r.top + h;
    prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor);

    int dir, ascent, descent;
    XCharStruct overall;
    XTextExtents( font, txt, STRLEN( txt ), &dir, &ascent, &descent, &overall );
    int xstart = r.left;

    if ( align == DT_RIGHT ) {
      int width = XTextWidth( font, txt, STRLEN( txt ) );
      xstart = r.right - width;

    } else if ( align == DT_CENTER ) {
      int width = XTextWidth( font, txt, STRLEN( txt ) );
      xstart = (r.right + r.left - width) / 2;
    }

    HDC hdc = c->getHDC();
    XDrawString( Linux::getDisplay(), hdc->d, hdc->gc, xstart, r.top + ascent, txt, STRLEN(txt) );

    restoreCanvas(c);
  }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased) {
  if (antialiased) {
    ifc_canvas *canvas = prepareAntialias(c, x, y, txt, size, bold, opaque, underline, italic, color, bkcolor, xoffset, yoffset, w, h);
    RECT al_r={0,0,w*2,h*2};

    OutputDebugString( "portme -- TrueTypeFont_Linux::textoutEllipsed (antialiased)\n" );

    completeAntialias(c);
  } else {
    RECT r;
    r.left = x+xoffset;
    r.top = y+yoffset;
    r.right = r.left + w;
    r.bottom = r.top + h;
    prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor);

    if ( txt == NULL )
      return;

    int dir, ascent, descent;
    XCharStruct overall;
    XTextExtents( font, txt, STRLEN( txt ), &dir, &ascent, &descent, &overall );

    char *tmp = (char *)MALLOC( STRLEN( txt ) + 3 );
    STRCPY( tmp, txt );

    if ( XTextWidth( font, tmp, STRLEN( tmp ) ) > r.right - r.left ) {
      int len = STRLEN( tmp );
      char *p = tmp + len;
      int width = r.right - r.left - XTextWidth( font, "...", 3 );
      while( XTextWidth( font, tmp, len ) > width ) {
	      *p-- = '\0';
	      len--;
      }
     STRCPY( p, "..." );
    }

    HDC hdc = c->getHDC();
    XDrawString( Linux::getDisplay(), hdc->d, hdc->gc, r.left, r.top + ascent, tmp, STRLEN(tmp) );

    FREE( tmp );

    restoreCanvas(c);
  }
}

const char *find_break( int (*width_func)(void *, const char *, int ), 
			void *f, const char *str, int width ) {
  const char *softret, *lastsoft, *hardret;

  if ( width_func( f, str, STRLEN( str ) ) <= width )
    return str + STRLEN( str );

  for( hardret = str; *hardret; hardret ++ )
    if ( *hardret == '\r' || *hardret == '\n' )
      break;

  if ( hardret && width_func( f, str, hardret - str ) <= width ) {
    return hardret;
  }
  for( softret = str; *softret && !isspace( *softret ); softret++ )
    ;
  
  if ( width_func( f, str, softret - str ) <= width ) {
    do {
      lastsoft = softret;

      for( softret = lastsoft+1; *softret && !isspace( *softret ); softret++ )
	;
      
    } while ( *lastsoft && width_func( f, str, softret - str ) <= width );

    softret = lastsoft;
  } else {
    for( softret = str; *softret; softret++ )
      if ( width_func( f, str, softret - str ) > width )
	break;

    softret--;
  }

  return softret;
}

int xlib_width( void *data, const char *str, int len ) {
  return XTextWidth( (XFontStruct *)data, str, len );
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased) {
  if (antialiased) {
    ifc_canvas *canvas = prepareAntialias(c, x, y, txt, size, bold, opaque, underline, italic, color, bkcolor, xoffset, yoffset, w, h);
    RECT al_r={0,0,w*2,h*2};

    OutputDebugString( "portme -- TrueTypeFont_Linux::textoutWrapped (antialiased)\n" );
    
    completeAntialias(c);
  } else {
    RECT r;
    r.left = x+xoffset;
    r.top = y+yoffset;
    r.right = r.left + w;
    r.bottom = r.top + h;
    prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor);

    
    int dir, ascent, descent;
    XCharStruct overall;
    XTextExtents( font, txt, STRLEN( txt ), &dir, &ascent, &descent, &overall );
    HDC hdc = c->getHDC();

    int yoff = r.top + ascent;
    const char *cur = txt, *next;
    int length = STRLEN( txt );
    for (int yoff = r.top + ascent; yoff < r.bottom - descent; yoff += ascent + descent) {
      next = find_break(xlib_width, font, cur, r.right - r.left);
      XDrawString( Linux::getDisplay(), hdc->d, hdc->gc, r.left, yoff, cur, next - cur );
      for ( cur = next; *cur && isspace( *cur ); cur++ )
        ;
      if ( cur >= txt + length )
        break;
    }

    
    restoreCanvas(c);
  }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased) {
  prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor);
  RECT r;
  char *ptr, *d;
  const char *s;
  ptr = (char *)MALLOC(STRLEN(txt)+1+4);
  for (s = txt, d = ptr; *s; s++, d++) {
    if (*s == '/') *d = '\\';
    else *d = *s;
  }
  r.left = x+xoffset;
  r.top = y+yoffset;
  r.right = r.left + w;
  r.bottom = r.top + getTextHeight(c, size, bold, underline, italic, antialiased);
  
  OutputDebugString( "portme -- TrueTypeFont_Linux::textoutWrappedPathed\n" );
  
  for (d = ptr; *d; d++) {
    if (*d == '\\') *d = '/';
  }

  if (antialiased) {
    restoreCanvas(c);

    ifc_canvas *canvas = prepareAntialias(c, x, y, txt, size, bold, opaque, underline, italic, color, bkcolor, xoffset, yoffset, w, -1);
    RECT al_r={0,0,w*2,al_h};
    
    OutputDebugString( "portme -- TrueTypeFont_Linux::textoutWrappedPathed (antialised)\n" );
    
    completeAntialias(c);
  } else {    
    int dir, ascent, descent;
    XCharStruct overall;
    XTextExtents( font, txt, STRLEN( txt ), &dir, &ascent, &descent, &overall );

    HDC hdc = c->getHDC();
    XDrawString( Linux::getDisplay(), hdc->d, hdc->gc, r.left, r.top + ascent, txt, STRLEN(txt) );
    OutputDebugString( "portme -- TrueTypeFont_Linux::textoutWrappedPathed\n" );
    
    restoreCanvas(c);
  }
  
  FREE(ptr);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::textOutCentered(ifc_canvas *c, RECT *r, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialiased) {
  ASSERT(r != NULL);
  ASSERT(txt != NULL);
  RECT rr=*r;
  rr.left += xoffset;
  rr.right += xoffset;
  rr.top += yoffset;
  rr.bottom += yoffset;

  if (antialiased) {
    ifc_canvas *canvas = prepareAntialias(c, r->left, r->top, txt, size, bold, opaque, underline, italic, color, bkcolor, xoffset, yoffset, r->right-r->left, r->bottom-r->top);
    RECT al_r={0,0,(r->right-r->left)*2,(r->bottom-r->top)*2};

    OutputDebugString( "portme -- TrueTypeFont_Linux::textoutCentered (antialiased)\n" );
    
    completeAntialias(c);
  } else { 
    prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor);
    
    int dir, ascent, descent, width;
    XCharStruct overall;
    XTextExtents( font, txt, STRLEN( txt ), &dir, &ascent, &descent, &overall );
    width = XTextWidth( font, txt, STRLEN( txt ) );

    HDC hdc = c->getHDC();
    XDrawString( Linux::getDisplay(), hdc->d, hdc->gc, (rr.right + rr.left - width) / 2, rr.top + ascent, txt, STRLEN(txt) );
    
    restoreCanvas(c);
  }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Linux::getTextWidth(ifc_canvas *c, const char *text, int size, int bold, int underline, int italic, int antialiased) {
  int w;
  getTextExtent(c, text, &w, NULL, size, bold, underline, italic, antialiased);
  return w;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Linux::getTextHeight(ifc_canvas *c, const char *text, int size, int bold, int underline, int italic, int antialiased) {
  int h;
  getTextExtent(c, text, NULL, &h, size, bold, underline, italic, antialiased);
  {
    // calcul for multiline text
    const char *p=text;
    int n=0;
    while(*p!=0) if(*p++=='\n') n++;
    if(n) h*=(n+1);
  }
  return h;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void TrueTypeFont_Linux::getTextExtent(ifc_canvas *c, const char *txt, int *w, int *h, int size, int bold, int underline, int italic, int antialiased) {
  SIZE rsize={0,0};
  ASSERT(txt != NULL);
  if (*txt == 0) {
    if (w != NULL) *w = 0;
    if (h != NULL) *h = 0;
    return;
  }

  if (antialiased) 
    prepareCanvas(c, size*2, bold, 0, underline, italic, 0, 0);
  else
    prepareCanvas(c, size, bold, 0, underline, italic, 0, 0);

  int dir, ascent, descent;
  XCharStruct overall;
  XTextExtents( font, txt, STRLEN( txt ), &dir, &ascent, &descent, &overall );
  rsize.cy = ascent + descent;
  rsize.cx = XTextWidth( font, txt, STRLEN( txt ) );
  
  if (w != NULL) *w = rsize.cx;
  if (h != NULL) *h = rsize.cy;

  if (antialiased) {
    if (w != NULL) *w /= 2;
    if (h != NULL) *h /= 2;
  }

  restoreCanvas(c);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int TrueTypeFont_Linux::getTextHeight(ifc_canvas *c, int size, int bold, int underline, int italic, int antialiased) {
  return getTextHeight(c, "My", size, bold, underline, italic, antialiased);
}


// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// code from ftp.microsoft.com/Softlib/MSLFILES/FONTINST.EXE
// retrieves the friendly font name from its filename
char *TrueTypeFont_Linux::filenameToFontFace(const char *pszFile) {
  static char					lpszLongName[256];
  unsigned            i;
  char								namebuf[255];
  int									fp;
  unsigned short      numNames;
  long								curseek;
  unsigned            cTables;
  sfnt_OffsetTable    OffsetTable;
  sfnt_DirectoryEntry Table;
  sfnt_NamingTable    NamingTable;
  sfnt_NameRecord			NameRecord;

  OutputDebugString( "portme -- TrueTypeFont_Linux::filenameToFontFace\n" );

  return FALSE;
}
