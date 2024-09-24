#include <precomp.h>

//#define NO_SIMPLEFASTMODE
#include <api/imgldr/api_imgldr.h>
#include <api/wnd/blending.h>

#include "bitmap.h"
#ifndef _NOSTUDIO
#include <api/api.h>
#endif
#include <bfc/std.h>
#include <api/wnd/bltcanvas.h>

#include <api/memmgr/memmgrapi.h>


#if !defined(WIN32) && !defined(LINUX)
#error port me!
#endif

#define ERRORBMP L"wasabi.bitmapnotfound"
#define HARDERRORBMP L"xml/wasabi/window/error.png" 
// do not define NO_MMX in this file. :)

#ifndef NO_MMX

#ifdef WIN32
#define MMX_CONST const
#else
#define MMX_CONST
#endif

static unsigned int MMX_CONST SkinBitmap_mmx_revn2[2]={0x01000100,0x01000100};
static unsigned int MMX_CONST SkinBitmap_mmx_zero[2];
static unsigned int MMX_CONST SkinBitmap_mmx_one[2]={1,0};
#define HAS_MMX Blenders::MMX_AVAILABLE()

#else

//NO_MMX defined
#define HAS_MMX 0

#endif


#ifndef _NOSTUDIO

#ifdef WASABI_COMPILE_IMGLDR
SkinBitmap::SkinBitmap(HINSTANCE hInstance, int id, const wchar_t *forcegroup)
{
  bitmapname = L"";
  subimage_w=-1;
  subimage_h=-1;
  x_offset=-1;
  y_offset=-1;
  fullimage_w=fullimage_h=0;
  has_alpha = 0;
  ASSERT(hInstance != NULL);
  ownbits=1;
  fromskin = 0;
  bits=WASABI_API_IMGLDR->imgldr_makeBmp(hInstance, id,&has_alpha,&fullimage_w,&fullimage_h, forcegroup);
	last_failed = 0;
	#ifdef WASABI_COMPILE_SKIN
  if (bits == NULL) 
	{
		//last_failed = 1;
		//TODO: bits = WASABI_API_IMGLDR->imgldr_requestSkinBitmap(ERRORBMP, &has_alpha, &x_offset, &y_offset, &subimage_w, &subimage_h, &fullimage_w, &fullimage_h,_cached);
	}
	#endif
  if (bits == NULL) 
	{
    last_failed = 1;
    bits = WASABI_API_IMGLDR->imgldr_makeBmp(HARDERRORBMP, &has_alpha, &fullimage_w, &fullimage_h);
  }
}
#endif
#endif

// TODO: benski> make sure this works  :)
SkinBitmap::SkinBitmap(ARGB32 *_bits, int w, int h)
{
	subimage_w=-1;
  subimage_h=-1;
  x_offset=-1;
  y_offset=-1;
  bitmapname = L"";
  fullimage_w=fullimage_h=0;
  has_alpha = 1;
  ownbits=0;
  bits = _bits;
  fromskin = 0;
  last_failed = 0;
}

// TODO: benski> could we be using GetDIBits here?
void SkinBitmap::bmpToBits(HBITMAP hbmp, HDC defaultDC)
{
#ifdef WIN32
  if (hbmp && !bits) 
  {
    BITMAPINFO srcbmi={0,};
    HDC hMemDC, hMemDC2;
    HBITMAP hprev,hprev2=0;
    HBITMAP hsrcdib;
    void *srcdib;
    BITMAP bm;
    int r = GetObject(hbmp, sizeof(BITMAP), &bm);
    ASSERT(r != 0);

    fullimage_w=bm.bmWidth;
    fullimage_h=ABS(bm.bmHeight);

    int bmw=getWidth();
    int bmh=getHeight();
    int xo=getX();
    int yo=getY();

    srcbmi.bmiHeader.biSize=sizeof(srcbmi.bmiHeader);
    srcbmi.bmiHeader.biWidth=bmw;
    srcbmi.bmiHeader.biHeight=-bmh;
    srcbmi.bmiHeader.biPlanes=1;
    srcbmi.bmiHeader.biBitCount=32;
    srcbmi.bmiHeader.biCompression=BI_RGB;
    hMemDC = CreateCompatibleDC(NULL);
    hsrcdib=CreateDIBSection(hMemDC,&srcbmi,DIB_RGB_COLORS,&srcdib,NULL,0);
    ASSERTPR(hsrcdib != 0, "CreateDIBSection() failed #6");
    if (defaultDC) 
      hMemDC2 = defaultDC;
    else {
      hMemDC2 = CreateCompatibleDC(NULL);
      hprev2 = (HBITMAP) SelectObject(hMemDC2, hbmp);
    }
    hprev = (HBITMAP) SelectObject(hMemDC, hsrcdib);
    BitBlt(hMemDC,0,0,bmw,bmh,hMemDC2,xo,yo,SRCCOPY);
    SelectObject(hMemDC, hprev);
    if (!defaultDC) {
      SelectObject(hMemDC2, hprev2);
      DeleteDC(hMemDC2);
    }
    DeleteDC(hMemDC);
    bits=(ARGB32*)MALLOC_(bmw*bmh*4);
    if (getHeight()+getY() > bm.bmHeight || getWidth()+getX() > bm.bmWidth) {
      ASSERTALWAYS(StringPrintf("Subbitmap coordinates outside master bitmap [%d,%d,%d,%d in 0,0,%d,%d]", getX(), getY(), getWidth(), getHeight(), bm.bmWidth, bm.bmHeight));
    }
    MEMCPY32(bits,srcdib,bmw*bmh/**sizeof(ARGB32)*/);
    DeleteObject(hsrcdib);
    x_offset=-1;
    y_offset=-1;
    subimage_w=-1;
    subimage_h=-1;
    fullimage_w=bmw;
    fullimage_h=bmh;
  }
#endif
#ifdef LINUX
  if ( ! bits ) {
    fullimage_w=hbmp.bmWidth;
    fullimage_h=ABS(hbmp.bmHeight);

    bits=(ARGB32*)MALLOC_( fullimage_w * fullimage_h * 4 );
    MEMCPY32( bits, hbmp.shmseginfo->shmaddr, fullimage_w * fullimage_h );
    x_offset=-1;
    y_offset=-1;
    subimage_w=-1;
    subimage_h=-1;
  }
#endif
}

#ifndef _NOSTUDIO
#ifdef WASABI_COMPILE_IMGLDR
SkinBitmap::SkinBitmap(const wchar_t *elementname, int _cached) 
{
  ASSERT(elementname!= NULL);

  bitmapname = elementname;
  x_offset = -1;
  y_offset = -1;
  subimage_w = -1;
  subimage_h = -1;
  fullimage_w=fullimage_h=0;
  ownbits=1;
  bits = NULL;
  fromskin = 0;
  last_failed = 0;
#ifdef WASABI_COMPILE_SKIN
  bits = WASABI_API_IMGLDR->imgldr_requestSkinBitmap(elementname, &has_alpha, &x_offset, &y_offset, &subimage_w, &subimage_h, &fullimage_w, &fullimage_h,_cached);
  fromskin = (bits != NULL);
#endif
  if (bits == NULL) 
		bits = WASABI_API_IMGLDR->imgldr_makeBmp(elementname, &has_alpha, &fullimage_w, &fullimage_h);
	#ifdef WASABI_COMPILE_SKIN
  if (bits == NULL) 
	{
		bits = WASABI_API_IMGLDR->imgldr_requestSkinBitmap(ERRORBMP, &has_alpha, &x_offset, &y_offset, &subimage_w, &subimage_h, &fullimage_w, &fullimage_h,_cached);
		 last_failed = 1;
	}
	#endif
  if (bits == NULL)
	{
    bits = WASABI_API_IMGLDR->imgldr_makeBmp(HARDERRORBMP, &has_alpha, &fullimage_w, &fullimage_h);
    last_failed = 1;
  }
  
  // check that coordinates are correct
  if(x_offset!=-1 && x_offset>fullimage_w) x_offset=fullimage_w-1;
  if(y_offset!=-1 && y_offset>fullimage_h) y_offset=fullimage_h-1;
  if(subimage_w!=-1 && (x_offset+subimage_w)>fullimage_w) subimage_w=fullimage_w-x_offset;
  if(subimage_h!=-1 && (y_offset+subimage_h)>fullimage_h) subimage_h=fullimage_h-y_offset;

  // ASSERTPR(bits != NULL, elementname);
  if (bits == NULL) {
    DebugString("element not found ! %s\n", elementname);
    int n = 10*10;
    bits = (ARGB32 *)WASABI_API_MEMMGR->sysMalloc(n * 4);


    ARGB32 *p = bits;
    while (n--)
      *p++ = 0xFFFF00FF;
  }
}
#endif
#endif

SkinBitmap::SkinBitmap(HBITMAP bitmap) 
{
#ifdef WIN32
  ASSERT(bitmap != NULL);
#endif
  subimage_w=-1;
  subimage_h=-1;
  x_offset=-1;
  y_offset=-1;
  bitmapname = L"";
  fullimage_w=fullimage_h=0;
  has_alpha = 0;
  ownbits=1;
  bits = NULL;
  fromskin = 0;
  last_failed = 0;
  bmpToBits(bitmap,NULL);
}

SkinBitmap::SkinBitmap(HBITMAP bitmap, HDC dc, int _has_alpha, void *_bits) 
{
  subimage_w=-1;
  subimage_h=-1;
  x_offset=-1;
  y_offset=-1;
  fromskin = 0;
  last_failed = 0;
  bitmapname = L"";
  fullimage_w=fullimage_h=0;
#ifdef WIN32
  ASSERT(bitmap != NULL);
#endif
  has_alpha = _has_alpha;
  bits = (ARGB32*)_bits;
  if (!_bits) 
  {
    ownbits=1;
    bmpToBits(bitmap,dc);
  }
  else
  {
#ifdef WIN32
    BITMAP bm;
    ownbits=0;
    int r = GetObject(bitmap, sizeof(BITMAP), &bm);
    ASSERT(r != 0);
    fullimage_w=bm.bmWidth;
    fullimage_h=ABS(bm.bmHeight);
#endif
#ifdef LINUX
    ownbits=0;
    fullimage_w=bitmap.bmWidth;
    fullimage_h=ABS(bitmap.bmHeight);
#endif
//port me
  }
}

SkinBitmap::SkinBitmap(int w, int h, DWORD bgcolor) {
  subimage_w=-1;
  subimage_h=-1;
  x_offset=-1;
  y_offset=-1;
  fullimage_w=w;
  bitmapname = L"";
  fullimage_h=h;
  fromskin = 0;
  last_failed = 0;

  int memsize = w*h*sizeof(ARGB32);
  if (memsize == 0) memsize++; // +1 so no failure when 0x0
  bits = (ARGB32*)MALLOC_(memsize);

  DWORD *dw = (DWORD *)bits;
  MEMFILL<DWORD>(dw, bgcolor, w*h);

  has_alpha = TRUE;
  ownbits=2; // 2 specifies should be FREE()'d
}

SkinBitmap::~SkinBitmap() {
  if (bits) {
    if (ownbits==2) FREE(bits);
#ifndef _NOSTUDIO
#ifdef WASABI_COMPILE_IMGLDR
    else if (ownbits) {
#ifdef WASABI_COMPILE_SKIN
      if (fromskin)
        WASABI_API_IMGLDR->imgldr_releaseSkinBitmap(bits);
      else
#endif
#ifndef _WASABIRUNTIME
        WASABI_API_IMGLDR->imgldr_releaseBmp(bits);
#else
        WASABI_API_IMGLDR->imgldr_releaseSkinBitmap(bits);
#endif
    }
#endif
#endif
  }
  bits=NULL;
}

void SkinBitmap::blit(ifc_canvas *canvas, int x, int y) {
  RECT src, dst;
  src.left=0;
  src.top=0;
  src.bottom=getHeight();
  src.right=getWidth();
  dst.left=x;
  dst.right=x+getWidth();
  dst.top=y;
  dst.bottom=y+getHeight();
  blitToRect(canvas,&src,&dst,255);
}

void SkinBitmap::blitRectToTile(ifc_canvas *canvas, RECT *dest, RECT *src, int xoffs, int yoffs, int alpha) {
  int startx,starty;

  int w,h;

  w = src->right-src->left;
  h = src->bottom-src->top;
  if (w <= 0 || h <= 0) return;	//wtfmf

  RECT c;
  if (canvas->getClipBox(&c) == NULLREGION) {
    c = *dest;
  } else {
    if (dest->left > c.left) c.left = dest->left;
    if (dest->top > c.top) c.top = dest->top;
    if (dest->right < c.right) c.right = dest->right;
    if (dest->bottom < c.bottom) c.bottom = dest->bottom;
  }


  starty = c.top-((c.top - dest->top) % h)- yoffs;
  startx = c.left-((c.left - dest->left) % w) - xoffs;

  for (int j=starty;j<c.bottom;j+=h)
    for (int i=startx;i<c.right;i+=w) {
      int xp=i;
      int yp=j;
      int xo=0;
      int yo=0;
      int _w=getWidth();
      int _h=getHeight();
      if (xp < c.left) {
        xo=c.left-xp;
        _w+=xo;
        xp=c.left;
      }
      if (yp < c.top) {
        yo=c.top-yp;
        _h+=yo;
        yp=c.top;
      }
      if (xp + _w >= c.right) _w=c.right-xp;
      if (yp + _h >= c.bottom) _h=c.bottom-yp;
      RECT _s={xo, yo, xo+_w, yo+_h};
      RECT _d={xp, yp, xp+_w, yp+_h};
      blitToRect(canvas, &_s, &_d, alpha);
    }
}


void SkinBitmap::blitTile(ifc_canvas *canvas, RECT *dest, int xoffs, int yoffs, int alpha) {
  RECT r={0,0,getWidth(),getHeight()};
  blitRectToTile(canvas, dest, &r, xoffs, yoffs, alpha);
}

#ifdef WIN32
#pragma warning(push) 
#pragma warning(disable : 4799) 
#endif


#define DEFAULT_CACHE_WIDTH 64
#define DEFAULT_CACHE_HEIGHT 64
int cacheWidth = DEFAULT_CACHE_WIDTH;
int cacheHeight = DEFAULT_CACHE_HEIGHT;
BltCanvas blitToRectCanvas(DEFAULT_CACHE_WIDTH,DEFAULT_CACHE_HEIGHT);

void SkinBitmap::blitToRect(ifc_canvas *canvas, RECT *src, RECT *dst, int alpha) { // only dst(top,left) are used

  if (alpha <= 0) return;
  if (alpha > 255) alpha = 255;

  HDC hdc = canvas->getHDC();
  if (hdc == NULL) return;
  void *dib=canvas->getBits();
  int cwidth,cheight, pitch;
  BaseCloneCanvas clone(canvas);
	bool usingBlitCanvas = false;
  RECT destrect=*dst;
  destrect.bottom=destrect.top+(src->bottom-src->top);
  destrect.right=destrect.left+(src->right-src->left);

  RECT c;
  int ctype=canvas->getClipBox(&c);

  if (c.top > destrect.top) destrect.top=c.top;
  if (c.left > destrect.left) destrect.left=c.left;
  if (c.bottom < destrect.bottom) destrect.bottom=c.bottom;
  if (c.right < destrect.right) destrect.right=c.right;

#ifdef NO_SIMPLEFASTMODE
  dib=NULL;
#endif

  if (destrect.right <= destrect.left || destrect.bottom <= destrect.top) return;
  int xs,yp,xe,ye;

  if (!dib || canvas->getDim(NULL,&cheight,&cwidth) || !cwidth || cheight < 1 || ctype == COMPLEXREGION)
  {
    cwidth=destrect.right-destrect.left;
    cheight=destrect.bottom-destrect.top;
		if (cwidth > cacheWidth || cheight > cacheHeight)
		{
			cacheWidth=MAX(cacheWidth, cwidth);
			cacheHeight=MAX(cacheHeight, cheight);
			blitToRectCanvas.DestructiveResize(cacheWidth, cacheHeight);
		}

    dib = blitToRectCanvas.getBits();
    if (has_alpha || alpha < 255)
      clone.blit(destrect.left, destrect.top, &blitToRectCanvas, 0, 0, cwidth, cheight);

    xs=0;
    yp=0;
    xe=cwidth;
    ye=cheight;
		pitch=cacheWidth;
		usingBlitCanvas = true;
  }
  else 
  {

    xs=destrect.left;
    xe=destrect.right;
    yp=destrect.top;
    ye=destrect.bottom;

    cwidth/=4;
		pitch=cwidth;
  }
  int xpo=(dst->left-destrect.left+xs)-(getX()+src->left);
  int ypo=(dst->top-destrect.top+yp)-(getY()+src->top);

  if (yp < 0) yp=0;
  if (xs < 0) xs=0;

  if (yp<getY()+ypo) yp=ypo+getY();
  if (xs<getX()+xpo) xs=xpo+getX();

  if (xe > getWidth()+getX()+xpo) xe=getWidth()+getX()+xpo;
  if (ye > getHeight()+getY()+ypo) ye=getHeight()+getY()+ypo;

  // blend bitmap to dib

  if (xs<xe) for (; yp < ye; yp ++) {
    int xp=xe-xs;
    unsigned int *dest=((unsigned int*)dib) + pitch*yp + xs;
    unsigned int *src=((unsigned int*)bits) + (yp-ypo)*fullimage_w + (xs-xpo);
    
    if (!has_alpha && alpha==255) // simple copy
    {
      MEMCPY32(dest,src,xp);
    }   
    else if (!has_alpha) { // no alpha channel info, but just a simple blend
      if (!HAS_MMX)
        while (xp--) 					*dest++ = Blenders::BLEND_ADJ1(*src++, *dest, alpha);
				
      #ifndef NO_MMX
        else
        {
#ifdef WIN32
          if (xp>1) __asm
          {
            movd mm3, [alpha]
            mov ecx, xp

            movq mm4, [SkinBitmap_mmx_revn2]
            packuswb mm3, mm3 // 0000HHVV

            paddusw mm3, [SkinBitmap_mmx_one]
            mov edi, dest

            punpcklwd mm3, mm3 // HHVVHHVV
            mov esi, src
        
            punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV
            shr ecx, 1

            psubw mm4, mm3

            align 16
            _blitAlpha_Loop1:
  
              movd mm0, [edi]

              movd mm1, [esi]
              punpcklbw mm0, [SkinBitmap_mmx_zero]
        
              movd mm7, [edi+4]
              punpcklbw mm1, [SkinBitmap_mmx_zero]

              pmullw mm0, mm4
              pmullw mm1, mm3

              movd mm6, [esi+4]
              punpcklbw mm7, [SkinBitmap_mmx_zero]
            
              punpcklbw mm6, [SkinBitmap_mmx_zero]

              pmullw mm7, mm4      
              pmullw mm6, mm3

              paddw mm0, mm1

              psrlw mm0, 8

              packuswb mm0, mm0
              add esi, 8   

              movd [edi], mm0
              paddw mm7, mm6
           
              psrlw mm7, 8
    
              packuswb mm7, mm7

              movd [edi+4], mm7

              add edi, 8

            dec ecx
            jnz _blitAlpha_Loop1
            mov src, esi
            mov dest, edi
#else
	      if ( xp > 1 ) {
	    __asm__ volatile (
			      "movd %6, %%mm3\n"
			      "mov %2, %%ecx\n"
			      "movq (SkinBitmap_mmx_revn2), %%mm4\n"
			      "packuswb %%mm3, %%mm3\n"
			      "paddusw (SkinBitmap_mmx_one), %%mm3\n"
			      "mov %0, %%edi\n"
			      "punpcklwd %%mm3, %%mm3\n"
			      "mov %1, %%esi\n"
			      "punpckldq %%mm3, %%mm3\n"
			      "shr $1, %%ecx\n"
			      "psubw %%mm3, %%mm4\n"
			      ".align 16\n"
			      "_blitAlpha_Loop1:\n"
			      "movd (%%edi), %%mm0\n"
			      "movd (%%esi), %%mm1\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm0\n"
			      "movd 4(%%edi), %%mm7\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm1\n"
			      "pmullw %%mm3, %%mm0\n"
			      "pmullw %%mm4, %%mm1\n"
			      "movd 4(%%esi), %%mm6\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm7\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm6\n"
			      "pmullw %%mm4, %%mm6\n"
			      "pmullw %%mm3, %%mm7\n"
			      "paddw %%mm1, %%mm0\n"
			      "psrlw $8, %%mm0\n"
			      "packuswb %%mm0, %%mm0\n"
			      "add $8, %%esi\n"
			      "movd %%mm0, (%%edi)\n"
			      "paddw %%mm6, %%mm7\n"
			      "psrlw $8, %%mm7\n"
			      "packuswb %%mm7, %%mm7\n"
			      "movd %%mm7, 4(%%edi)\n"
			      "add $8, %%edi\n"
			      "dec %%ecx\n"
			      "jnz _blitAlpha_Loop1\n"
			      "mov %%esi, %1\n"
			      "mov %%edi, %0\n"

			      : "=m" (dest), "=m" (src), "=m" (xp)
			      : "0" (dest), "1" (src), "2" (xp), "m" (alpha)
			      : "%eax", "%ecx", "%esi", "%edi" );

#endif
          }
          if (xp & 1) *dest++ = Blenders::BLEND_ADJ1_MMX(*src++, *dest, alpha);
        } // mmx available
      #endif // !NO_MMX
    }
    else if (alpha == 255) { // no global alpha, just alpha channel
      if (!HAS_MMX)
        while (xp--)					*dest++ = Blenders::BLEND_ADJ2(*dest, *src++);
				
      #ifndef NO_MMX
        else
        {
#ifdef WIN32
          if (xp > 1) __asm
          {
            mov ecx, xp
            shr ecx, 1
            mov edi, dest
            mov esi, src
            align 16
            _blitAlpha_Loop2:

            movd mm3, [esi]
            movd mm5, [esi+4]

            movq mm2, [SkinBitmap_mmx_revn2]
            psrld mm3, 24

            movq mm4, [SkinBitmap_mmx_revn2]
            psrld mm5, 24
          
            movd mm0, [edi]
            packuswb mm3, mm3 // 0000HHVV

            movd mm1, [esi]
            packuswb mm5, mm5 // 0000HHVV
          
            movd mm6, [esi+4]
            paddusw mm3, [SkinBitmap_mmx_one]

            punpcklwd mm3, mm3 // HHVVHHVV
            paddusw mm5, [SkinBitmap_mmx_one]

            movd mm7, [edi+4]
            punpcklwd mm5, mm5 // HHVVHHVV
        
            punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV
            punpckldq mm5, mm5 // HHVVHHVV HHVVHHVV

            punpcklbw mm6, [SkinBitmap_mmx_zero]
            psubw mm4, mm5

            punpcklbw mm0, [SkinBitmap_mmx_zero]
            psubw mm2, mm3

            punpcklbw mm7, [SkinBitmap_mmx_zero]
            pmullw mm0, mm2
          
            pmullw mm7, mm4      
            punpcklbw mm1, [SkinBitmap_mmx_zero]

            psubw mm2, mm3

            psrlw mm0, 8
            psrlw mm7, 8
            paddw mm0, mm1
   
            paddw mm7, mm6
            packuswb mm0, mm0

            movd [edi], mm0
            packuswb mm7, mm7
                 
            movd [edi+4], mm7

            add esi, 8
            add edi, 8

            dec ecx
            jnz _blitAlpha_Loop2
            mov src, esi
            mov dest, edi
#else
	if( xp > 1 ) {
	    __asm__ volatile (
			      "mov %4, %%ecx\n"
			      "shr $1, %%ecx\n"
			      "mov %0, %%edi\n"
			      "mov %1, %%esi\n"
			      ".align 16\n"
			      "_blitAlpha_Loop2:\n"
			      "movd (%%esi), %%mm3\n"
			      "movd 4(%%esi), %%mm5\n"
			      "movq (SkinBitmap_mmx_revn2), %%mm2\n"
			      "psrld $24, %%mm3\n"
			      "movq (SkinBitmap_mmx_revn2), %%mm4\n"
			      "psrld $24, %%mm5\n"
			      "movd (%%edi), %%mm0\n"
			      "packuswb %%mm3, %%mm3\n"
			      "movd (%%esi), %%mm1\n"
			      "packuswb %%mm5, %%mm5\n"
			      "movd 4(%%esi), %%mm6\n"
			      "paddusw (SkinBitmap_mmx_one), %%mm3\n"
			      "punpcklwd %%mm3, %%mm3\n"
			      "paddusw (SkinBitmap_mmx_one), %%mm5\n"
 			      "movd 4(%%edi), %%mm7\n"
			      "punpcklwd %%mm5, %%mm5\n"
			      "punpckldq %%mm3, %%mm3\n"
			      "punpckldq %%mm5, %%mm5\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm6\n"
			      "psubw %%mm5, %%mm4\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm0\n"
			      "psubw %%mm3, %%mm2\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm7\n"
			      "pmullw %%mm2, %%mm0\n"
			      "pmullw %%mm4, %%mm7\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm1\n"
			      "psubw %%mm3, %%mm2\n"
			      "psrlw $8, %%mm0\n"
			      "psrlw $8, %%mm7\n"
			      "paddw %%mm1, %%mm0\n"
			      "paddw %%mm6, %%mm7\n"
			      "packuswb %%mm0, %%mm0\n"
			      "movd %%mm0, (%%edi)\n"
			      "packuswb %%mm7, %%mm7\n"
			      "movd %%mm7, 4(%%edi)\n"
			      "add $8, %%esi\n"
			      "add $8, %%edi\n"
			      "dec %%ecx\n"
			      "jnz _blitAlpha_Loop2\n"
			      "mov %%esi, %1\n"
			      "mov %%edi, %0\n"

			      : "=m" (dest), "=m" (src)
			      : "0" (dest), "1" (src), "m" (xp)
			      : "%eax", "%ecx", "%esi", "%edi" );
#endif
          }
          if (xp&1) *dest++ = Blenders::BLEND_ADJ2_MMX(*dest, *src++);
        } // HAS_MMX
      #endif // ifndef NO_MMX
    }
    else { // both
      if (!HAS_MMX)
        while (xp--) 					*dest++ = Blenders::BLEND_ADJ3(*dest, *src++, alpha);
      #ifndef NO_MMX
        else
        {
#ifdef WIN32
          if (xp > 1) __asm
          {
            movd mm5, [alpha]
            mov ecx, xp

            packuswb mm5, mm5 
            shr ecx, 1

            paddusw mm5, [SkinBitmap_mmx_one]

            punpcklwd mm5, mm5        
            mov edi, dest

            punpckldq mm5, mm5
            mov esi, src

            align 16
            _blitAlpha_Loop3:

            movd mm3, [esi] // VVVVVVVV
            movd mm4, [esi+4] // VVVVVVVV

            movd mm0, [edi]    
            psrld mm3, 24

            movd mm1, [esi]
            psrld mm4, 24

            paddusw mm3, [SkinBitmap_mmx_one]
            paddusw mm4, [SkinBitmap_mmx_one]

            movd mm7, [edi+4]    
            punpcklwd mm3, mm3

            movd mm6, [esi+4]
            punpcklwd mm4, mm4

            punpckldq mm3, mm3
            punpckldq mm4, mm4
          
            pmullw mm3, mm5
            pmullw mm4, mm5

            punpcklbw mm7, [SkinBitmap_mmx_zero]
            punpcklbw mm6, [SkinBitmap_mmx_zero]

            movq mm2, [SkinBitmap_mmx_revn2]
            psrlw mm3, 8

            psrlw mm4, 8  

            punpcklbw mm0, [SkinBitmap_mmx_zero]
            punpcklbw mm1, [SkinBitmap_mmx_zero]

            psubw mm2, mm3
            pmullw mm0, mm2      

            pmullw mm1, mm5
            add esi, 8

            movq mm2, [SkinBitmap_mmx_revn2]
            pmullw mm6, mm5
    
            paddusw mm0, mm1
            psubw mm2, mm4

            pmullw mm7, mm2      
            psrlw mm0, 8   

            packuswb mm0, mm0
            paddusw mm7, mm6

            movd [edi], mm0
            psrlw mm7, 8   
                 
            packuswb mm7, mm7

            movd [edi+4], mm7

            add edi, 8

            dec ecx
            jnz _blitAlpha_Loop3
            mov src, esi
            mov dest, edi
#else
	  if ( xp > 1 ) {
	    __asm__ volatile (
			      "movd %5, %%mm5\n"
			      "mov %4, %%ecx\n"
			      "packuswb %%mm5, %%mm5 \n"
			      "shr $1, %%ecx\n"
			      "paddusw (SkinBitmap_mmx_one), %%mm5\n"
			      "punpcklwd %%mm5, %%mm5\n"
			      "mov %0, %%edi\n"
			      "punpckldq %%mm5, %%mm5\n"
			      "mov %1, %%esi\n"
			      ".align 16\n"
			      "_blitAlpha_Loop3:\n"
			      "movd (%%esi), %%mm3\n"
			      "movd 4(%%esi), %%mm4\n"
			      "movd (%%edi), %%mm0\n"
			      "psrld $24, %%mm3\n"
			      "movd (%%esi), %%mm1\n"
			      "psrld $24, %%mm4\n"
			      "paddusw (SkinBitmap_mmx_one), %%mm3\n"
			      "paddusw (SkinBitmap_mmx_one), %%mm4\n"
			      "movd 4(%%edi), %%mm7\n"
			      "punpcklwd %%mm3, %%mm3\n"
			      "movd 4(%%esi), %%mm6\n"
			      "punpcklwd %%mm4, %%mm4\n"
			      "punpckldq %%mm3, %%mm3\n"
			      "punpckldq %%mm4, %%mm4\n"
			      "pmullw %%mm5, %%mm3\n"
			      "pmullw %%mm5, %%mm4\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm7\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm6\n"
			      "movq (SkinBitmap_mmx_revn2), %%mm2\n"
			      "psrlw $8, %%mm3\n"
			      "psrlw $8, %%mm4\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm0\n"
			      "punpcklbw (SkinBitmap_mmx_zero), %%mm1\n"
			      "psubw %%mm3, %%mm2\n"
			      "pmullw %%mm2, %%mm0\n"
			      "pmullw %%mm5, %%mm1\n"
			      "add $8, %%esi\n"
			      "movq (SkinBitmap_mmx_revn2), %%mm2\n"
			      "pmullw %%mm5, %%mm6\n"
			      "paddusw %%mm1, %%mm0\n"
			      "psubw %%mm4, %%mm2\n"
			      "pmullw %%mm2, %%mm7\n"
			      "psrlw $8, %%mm0\n"
			      "packuswb %%mm0, %%mm0\n"
			      "paddusw %%mm6, %%mm7\n"
			      "movd %%mm0, (%%edi)\n"
			      "psrlw $8, %%mm7\n"
			      "packuswb %%mm7, %%mm7\n"
			      "movd %%mm7, 4(%%edi)\n"
			      "add $8, %%edi\n"
			      "dec %%ecx\n"
			      "jnz _blitAlpha_Loop3\n"
			      "mov %%esi, %1\n"
			      "mov %%edi, %0\n"

			      : "=m" (dest), "=m" (src)
			      : "0" (dest), "1" (src), "m" (xp), "m" (alpha)
			      : "%eax", "%ecx", "%esi", "%edi" );
#endif
          }
          if (xp&1) *dest++ = Blenders::BLEND_ADJ3_MMX(*dest, *src++, alpha);
        } // HAS_MMX
      #endif // ifndef NO_MMX
    }
  }
#ifndef NO_MMX
  Blenders::BLEND_MMX_END();
#endif
  // write bits back to dib.

  if (usingBlitCanvas) {
    blitToRectCanvas.blit(0, 0, &clone, destrect.left, destrect.top, cwidth, cheight);
  }
}

#ifdef WIN32
#pragma warning(pop) 
#endif

void SkinBitmap::stretch(ifc_canvas *canvas, int x, int y, int w, int h) {
  RECT src, dst;
  src.left=0;
  src.top=0;
  src.right=getWidth();
  src.bottom=getHeight();
  dst.left=x;
  dst.right=x+w;
  dst.top=y;
  dst.bottom=y+h;
  stretchToRectAlpha(canvas,&src,&dst,255);
}

void SkinBitmap::stretchToRect(ifc_canvas *canvas, RECT *r) {
  stretch(canvas, r->left, r->top, r->right - r->left, r->bottom - r->top);
}

void SkinBitmap::stretchRectToRect(ifc_canvas *canvas, RECT *src, RECT *dst) {
  stretchToRectAlpha(canvas,src,dst,255);
}


void SkinBitmap::stretchToRectAlpha(ifc_canvas *canvas, RECT *r, int alpha) {
  RECT re;
  re.left=0; re.top=0;
  re.right=getWidth(); re.bottom=getHeight();
  stretchToRectAlpha(canvas,&re,r,alpha);
}

void SkinBitmap::blitAlpha(ifc_canvas *canvas, int x, int y, int alpha)
{
  RECT dst,src;
  dst.left=x;
  dst.top=y;
  src.left=0;
  src.top=0;
  src.bottom=getHeight();
  src.right=getWidth();
  blitToRect(canvas,&src,&dst,alpha);
}

#ifdef WIN32
#pragma warning(push) 
#pragma warning(disable : 4799) 
#endif

template <class C>
class Stretcher {
public:
  static void _stretchToRectAlpha(SkinBitmap *bitmap, int ys, int ye, int xe, int xs, int xstart, int yv, void *dib, int pitch, int dxv, int dyv, int alpha) {
    int bitmap_x = bitmap->getX();
    int bitmap_y = bitmap->getY();
    int bmpheight = bitmap->getHeight();
    int fullimage_w = bitmap->getFullWidth();
    void *bits = bitmap->getBits();
    int xp=xe-xs;
    for (int yp = ys; yp < ye; yp ++) {
      int t=yv>>16;
      if (t < 0) t=0;
      if (t >= bmpheight) t=bmpheight-1;
      int *psrc=((int*)bits) + (t+bitmap_y)*fullimage_w + bitmap_x;
      int *dest=((int*)dib) + pitch*yp + xs;     

      C::stretch(xp, psrc, dest, xstart, dxv, alpha);

      yv+=dyv;
    }
  }
};

// no alpha, just stretch
class Stretch {
public:
  static void stretch(int xp, int *psrc, int *dest, int xv, int dxv, int alpha) {
    while (xp--) { //JFtodo: assembly optimize - these first two modes aren't used that much anyway
      *dest++ = psrc[xv>>16];
      xv+=dxv;
    }
  }
};

// no alpha channel, just a global alpha val
class StretchGlobal {
public:
  static void stretch(int xp, int *psrc, int *dest, int xv, int dxv, int alpha) {
    while (xp--) { //JFTODO: make MMX optimized version
      *dest++ = Blenders::BLEND_ADJ1(psrc[xv>>16], *dest, alpha);
      xv+=dxv;
    }
  }
};

// alpha channel, no global alpha val
class StretchChannel {
public:
  static void stretch(int xp, int *psrc, int *dest, int xv, int dxv, int alpha) {
    while (xp--) {
      *dest++ = Blenders::BLEND_ADJ2(*dest, psrc[xv>>16]);
      xv+=dxv;
    }
  }
};

class StretchGlobalChannel {
public:
  static void stretch(int xp, int *psrc, int *dest, int xv, int dxv, int alpha) {
    while (xp--) {
      *dest++ = Blenders::BLEND_ADJ3(*dest, psrc[xv>>16], alpha);
      xv+=dxv;
    }
  }
};


#ifndef NO_MMX

// no alpha channel, just a global alpha val
class StretchGlobalMMX {
public:
  static void stretch(int xp, int *psrc, int *dest, int xv, int dxv, int alpha) {
    while (xp--) { //JFTODO: make MMX optimized version
      *dest++ = Blenders::BLEND_ADJ1_MMX(psrc[xv>>16], *dest, alpha);
      xv+=dxv;
    }
  }
};


// alpha channel, no global alpha val
class StretchChannelMMX {
public:
  static void stretch(int xp, int *psrc, int *dest, int xv, int dxv, int alpha) {
#ifdef WIN32
    if (xp>1) __asm
    {
      mov ecx, xp
      mov edi, dest

      shr ecx, 1
      mov esi, psrc

      mov edx, xv
      mov ebx, dxv

      align 16
    _stretchAlpha_Loop2:

      mov eax, edx
      movd mm0, [edi]

      movq mm4, [SkinBitmap_mmx_revn2]
      shr eax, 16

      movq mm2, [SkinBitmap_mmx_revn2]
      punpcklbw mm0, [SkinBitmap_mmx_zero]

      movd mm3, [esi+eax*4]
      movd mm1, [esi+eax*4]
      
      lea eax, [edx+ebx]
      shr eax, 16

      movd mm7, [edi+4]
      psrld mm3, 24

      packuswb mm3, mm3 // 0000HHVV
      movd mm5, [esi+eax*4]

      movd mm6, [esi+eax*4]
      psrld mm5, 24          

      paddusw mm3, [SkinBitmap_mmx_one]
      punpcklbw mm6, [SkinBitmap_mmx_zero]

      packuswb mm5, mm5 // 0000HHVV
      lea edx, [edx+ebx*2]
        
      paddusw mm5, [SkinBitmap_mmx_one]          
      punpcklwd mm3, mm3 // HHVVHHVV

      punpcklwd mm5, mm5 // HHVVHHVV
      add edi, 8
      
      punpckldq mm3, mm3 // HHVVHHVV HHVVHHVV

      punpckldq mm5, mm5 // HHVVHHVV HHVVHHVV

      psubw mm4, mm5

      psubw mm2, mm3

      punpcklbw mm7, [SkinBitmap_mmx_zero]
      pmullw mm0, mm2
        
      pmullw mm7, mm4      
      punpcklbw mm1, [SkinBitmap_mmx_zero]

      psubw mm2, mm3

      psrlw mm0, 8
      psrlw mm7, 8
      paddw mm0, mm1
 
      paddw mm7, mm6
      packuswb mm0, mm0

      movd [edi-8], mm0
      packuswb mm7, mm7
                 
      movd [edi-4], mm7

      dec ecx
      jnz _stretchAlpha_Loop2
      mov dest, edi
      mov xv, edx
    }
#else
    if (xp>1)
    {
      __asm__ volatile (
			"mov %5, %%ecx\n"
			"mov %0, %%edi\n"
			"shr $1, %%ecx\n"
			"mov %1, %%esi\n"
			"mov %2, %%edx\n"
			"mov %7, %%ebx\n"
			".align 16\n"
			"_stretchAlpha_Loop2:\n"
			"mov %%edx, %%eax\n"
			"movd (%%edi), %%mm0\n"
			"movq (SkinBitmap_mmx_revn2), %%mm4\n"
			"shr $16, %%eax\n"
			"movq (SkinBitmap_mmx_revn2), %%mm2\n"
			"punpcklbw (SkinBitmap_mmx_zero), %%mm0\n"
			"movd (%%esi,%%eax,4), %%mm3\n"
			"movd (%%esi,%%eax,4), %%mm1\n"
			"lea (%%edx,%%ebx), %%eax\n"
			"shr $16, %%eax\n"
			"movd 4(%%edi), %%mm7\n"
			"psrld $24, %%mm3\n"
			"packuswb %%mm3, %%mm3\n"
			"movd (%%esi,%%eax,4), %%mm5\n"
			"movd (%%esi,%%eax,4), %%mm6\n"
			"psrld $24, %%mm5\n"
			"paddusw (SkinBitmap_mmx_one), %%mm3\n"
			"punpcklbw (SkinBitmap_mmx_zero), %%mm6\n"
			"packuswb %%mm5, %%mm5\n"
			"lea (%%edx,%%ebx,2), %%edx\n"
			"paddusw (SkinBitmap_mmx_one), %%mm5\n"
			"punpcklwd %%mm3, %%mm3\n"
			"punpcklwd %%mm5, %%mm5\n"
			"add $8, %%edi\n"
			"punpckldq %%mm3, %%mm3\n"
			"punpckldq %%mm5, %%mm5\n"
			"psubw %%mm5, %%mm4\n"
			"psubw %%mm3, %%mm2\n"
			"punpcklbw (SkinBitmap_mmx_zero), %%mm7\n"
			"pmullw %%mm2, %%mm0\n"
			"pmullw %%mm4, %%mm7\n"     
			"punpcklbw (SkinBitmap_mmx_zero), %%mm1\n"
			"psubw %%mm3, %%mm2\n"
			"psrlw $8, %%mm0\n"
			"psrlw $8, %%mm7\n"
			"paddw %%mm1, %%mm0\n"
			"paddw %%mm6, %%mm7\n"
			"packuswb %%mm0, %%mm0\n"
			"movd %%mm0, -8(%%edi)\n"
			"packuswb %%mm7, %%mm7\n"
			"movd %%mm7, -4(%%edi)\n"
			"dec %%ecx\n"
			"jnz _stretchAlpha_Loop2\n"
			"mov %%edi, %0\n"
			"mov %%edx, %2\n"

			: "=m" (dest), "=m" (psrc), "=m" (xv)
			: "0" (dest), "1" (psrc), "m" (xp), 
			  "2" (xv), "m" (dxv), "m" (alpha)
			: "%eax", "%ebx", "%ecx", "%edx",
			  "%esi", "%edi" );

    }
#endif

    if (xp&1) *dest++ = Blenders::BLEND_ADJ2_MMX(*dest, psrc[xv>>16]);
  }
};


class StretchGlobalChannelMMX {
public:
  static void stretch(int xp, int *psrc, int *dest, int xv, int dxv, int alpha) {
#ifdef WIN32
    if (xp>1) __asm
    {
      movd mm5, [alpha]
      mov ecx, xp

      packuswb mm5, mm5 
      shr ecx, 1

      paddusw mm5, [SkinBitmap_mmx_one]

      punpcklwd mm5, mm5        
      mov edi, dest

      punpckldq mm5, mm5
      mov esi, psrc

      mov edx, xv
      mov ebx, dxv

      align 16
    _stretchAlpha_Loop3:
      movd mm0, [edi]    
      mov eax, edx

      movd mm7, [edi+4]    
      shr eax, 16

      movd mm1, [esi+eax*4]
      movd mm3, [esi+eax*4] // VVVVVVVV

      lea eax, [edx+ebx]
      psrld mm3, 24

      paddusw mm3, [SkinBitmap_mmx_one]

      punpcklwd mm3, mm3
      shr eax, 16

      punpckldq mm3, mm3

      pmullw mm3, mm5

      movd mm4, [esi+eax*4] // VVVVVVVV
      movd mm6, [esi+eax*4]

      movq mm2, [SkinBitmap_mmx_revn2]
      psrld mm4, 24

      paddusw mm4, [SkinBitmap_mmx_one]
      punpcklbw mm7, [SkinBitmap_mmx_zero]

      punpcklwd mm4, mm4
      lea edx, [edx+ebx*2]

      punpckldq mm4, mm4
      add edi, 8

      punpcklbw mm6, [SkinBitmap_mmx_zero]         
      pmullw mm4, mm5

      psrlw mm3, 8

      punpcklbw mm0, [SkinBitmap_mmx_zero]

      punpcklbw mm1, [SkinBitmap_mmx_zero]
      psubw mm2, mm3

      pmullw mm0, mm2      
      pmullw mm1, mm5

      pmullw mm6, mm5
      psrlw mm4, 8  

      movq mm2, [SkinBitmap_mmx_revn2]    
      paddusw mm0, mm1
      psubw mm2, mm4

      pmullw mm7, mm2      
      psrlw mm0, 8   

      packuswb mm0, mm0
      paddusw mm7, mm6

      movd [edi-8], mm0
      psrlw mm7, 8   
             
      packuswb mm7, mm7

      movd [edi-4], mm7

      dec ecx
      jnz _stretchAlpha_Loop3
      mov xv, edx
      mov dest, edi
    }
#else
    if (xp>1) 
    {
      __asm__ volatile (
			"movd %8, %%mm5\n"
			"mov %5, %%ecx\n"
			"packuswb %%mm5, %%mm5 \n"
			"shr $1, %%ecx\n"
			"paddusw (SkinBitmap_mmx_one), %%mm5\n"
			"punpcklwd %%mm5, %%mm5\n"
			"mov %0, %%edi\n"
			"punpckldq %%mm5, %%mm5\n"
			"mov %1, %%esi\n"
			"mov %6, %%edx\n"
			"mov %7, %%ebx\n"
			".align 16\n"
			"_stretchAlpha_Loop3:\n"
			"movd (%%edi), %%mm0\n"
			"mov %%edx, %%eax\n"
			"movd 4(%%edi), %%mm7\n"
			"shr $16, %%eax\n"
			"movd (%%esi,%%eax,4), %%mm1\n"
			"movd (%%esi,%%eax,4), %%mm3\n"
			"lea (%%edx,%%ebx), %%eax\n"
			"psrld $24, %%mm3\n"
			"paddusw (SkinBitmap_mmx_one), %%mm3\n"
			"punpcklwd %%mm3, %%mm3\n"
			"shr $16, %%eax\n"
			"punpckldq %%mm3, %%mm3\n"
			"pmullw %%mm5, %%mm3\n"
			"movd (%%esi,%%eax,4), %%mm4\n"
			"movd (%%esi,%%eax,4), %%mm6\n"
			"movq (SkinBitmap_mmx_revn2), %%mm2\n"
			"psrld $24, %%mm4\n"
			"paddusw (SkinBitmap_mmx_one), %%mm4\n"
			"punpcklbw (SkinBitmap_mmx_zero), %%mm7\n"
			"punpcklwd %%mm4, %%mm4\n"
			"lea (%%edx,%%ebx,2), %%edx\n"
			"punpckldq %%mm4, %%mm4\n"
			"add $8, %%edi\n"
			"punpcklbw (SkinBitmap_mmx_zero), %%mm6\n"
			"pmullw %%mm5, %%mm4\n"
			"psrlw $8, %%mm3\n"
			"punpcklbw (SkinBitmap_mmx_zero), %%mm0\n"
			"punpcklbw (SkinBitmap_mmx_zero), %%mm1\n"
			"psubw %%mm3, %%mm2\n"
			"pmullw %%mm2, %%mm0\n"      
			"pmullw %%mm5, %%mm1\n"
			"pmullw %%mm5, %%mm6\n"
			"psrlw $8, %%mm4\n"
			"movq (SkinBitmap_mmx_revn2), %%mm2\n"
			"paddusw %%mm1, %%mm0\n"
			"psubw %%mm4, %%mm2\n"
			"pmullw %%mm2, %%mm7\n"      
			"psrlw $8, %%mm0\n"
			"packuswb %%mm0, %%mm0\n"
			"paddusw %%mm6, %%mm7\n"
			"movd %%mm0, -8(%%edi)\n"
			"psrlw $8, %%mm7\n"
			"packuswb %%mm7, %%mm7\n"
			"movd %%mm7, -4(%%edi)\n"
			"dec %%ecx\n"
			"jnz _stretchAlpha_Loop3\n"
			"mov %%edi, %0\n"
			"mov %%edx, %2\n"

			: "=m" (dest), "=m" (psrc), "=m" (xv)
			: "0" (dest), "1" (psrc), "m" (xp), 
			  "m" (xv), "m" (dxv), "m" (alpha)
			: "%eax", "%ebx", "%ecx", "%edx",
			  "%esi", "%edi" );

    }
#endif

    if (xp&1) *dest++ = Blenders::BLEND_ADJ3_MMX(*dest, psrc[xv>>16], alpha);
  }
};
#endif


class __Stretch : public Stretcher<Stretch> {};
class __StretchGlobal : public Stretcher<StretchGlobal> {};
class __StretchChannel : public Stretcher<StretchChannel> {};
class __StretchGlobalChannel : public Stretcher<StretchGlobalChannel> {};

#ifndef NO_MMX
class __StretchGlobalMMX : public Stretcher<StretchGlobalMMX> {};
class __StretchChannelMMX : public Stretcher<StretchChannelMMX> {};
class __StretchGlobalChannelMMX : public Stretcher<StretchGlobalChannelMMX> {};
#endif

#ifdef WIN32
#pragma warning(pop) 
#endif


void SkinBitmap::stretchToRectAlpha(ifc_canvas *canvas, RECT *_src, RECT *_dst, int alpha)
{
  if (alpha <= 0) return;
  if (alpha > 255) alpha = 255;

  RECT src=*_src;
  RECT dst=*_dst;

  if ((src.right-src.left) == (dst.right-dst.left) &&
      (src.bottom-src.top) == (dst.bottom-dst.top))
  {
    blitToRect(canvas,_src,_dst,alpha);
    return;
  }
  //FG> this is a hack, we should support subpixels instead
  if (src.left == src.right) {
    if (src.right < getWidth()) 
      src.right++;
    else
      src.left--;
  } 
  if (src.top== src.bottom) {
    if (src.bottom < getHeight()) 
      src.bottom++;
    else
      src.top--;
  } 

  if (src.left >= src.right || src.top >= src.bottom) return;
  if (dst.left >= dst.right || dst.top >= dst.bottom) return;

  void *dib=canvas->getBits();
  HDC hdc=canvas->getHDC();
  bool usingBlitCanvas = false;
  BaseCloneCanvas clone(canvas);
  int cwidth, cheight, pitch;

  int dyv=((src.bottom-src.top)<<16)/(dst.bottom-dst.top);
  int dxv=((src.right-src.left)<<16)/(dst.right-dst.left);  
  int yv=(src.top<<16);
  int xstart=(src.left<<16);

  RECT c;
  int ctype=canvas->getClipBox(&c);
  if (c.top > dst.top) 
  {
    yv+=(c.top-dst.top)*dyv;
    dst.top=c.top;
  }
  if (c.left > dst.left)
  {
    xstart+=(c.left-dst.left)*dxv;
    dst.left=c.left;
  }
  if (c.bottom < dst.bottom) dst.bottom=c.bottom;
  if (c.right < dst.right) dst.right=c.right;

  if (dst.right <= dst.left || dst.bottom <= dst.top) return;

  int xs,xe,ys,ye;

#ifdef NO_SIMPLEFASTMODE
  dib=NULL;
#endif
  if (!dib || canvas->getDim(NULL,&cheight,&cwidth) || !cwidth || cheight < 1 || ctype == COMPLEXREGION)
  {
    cwidth=dst.right-dst.left;
    cheight=dst.bottom-dst.top;
		if (cwidth > cacheWidth || cheight > cacheHeight)
		{
			cacheWidth=MAX(cacheWidth, cwidth);
			cacheHeight=MAX(cacheHeight, cheight);
			blitToRectCanvas.DestructiveResize(cacheWidth, cacheHeight);
		}
    
    dib = blitToRectCanvas.getBits();
    if ( has_alpha || alpha < 255 )
      clone.blit( dst.left, dst.top, &blitToRectCanvas, 0, 0, cwidth, cheight );

    xs=0;
    ys=0;
    xe=cwidth;
    ye=cheight;
		pitch=cacheWidth;
		usingBlitCanvas=true;
  }
  else 
  {
    xs=dst.left;
    xe=dst.right;
    ys=dst.top;
    ye=dst.bottom;
    cwidth/=4;
		pitch=cwidth;
  }

  // stretch and blend bitmap to dib

  if (xstart < 0) xstart=0;

  if (xs<xe) {
    if (!has_alpha) {	// doesn't have alpha channel
      if (alpha == 255) {	// no global alpha
        __Stretch::_stretchToRectAlpha(this, ys, ye, xe, xs, xstart, yv, dib, pitch, dxv, dyv, alpha);
      } else {	// has global alpha
#ifndef NO_MMX
        if (HAS_MMX) {
          __StretchGlobalMMX::_stretchToRectAlpha(this, ys, ye, xe, xs, xstart, yv, dib, pitch, dxv, dyv, alpha);
        } else
#endif
        {
          __StretchGlobal::_stretchToRectAlpha(this, ys, ye, xe, xs, xstart, yv, dib, pitch, dxv, dyv, alpha);
        }
      }
    } else {	// has alpha channel
      // FUCKO: JF> BRENNAN FIX THESE BITCHES :)
      if (alpha == 255) {	// no global alpha
#ifndef NO_MMX
        if (HAS_MMX) {
          __StretchChannelMMX::_stretchToRectAlpha(this, ys, ye, xe, xs, xstart, yv, dib, pitch, dxv, dyv, alpha);
        } else 
#endif
        {
          __StretchChannel::_stretchToRectAlpha(this, ys, ye, xe, xs, xstart, yv, dib, pitch, dxv, dyv, alpha);
        }
      } else {	// has global alpha
#ifndef NO_MMX
        if (HAS_MMX) {
          __StretchGlobalChannelMMX::_stretchToRectAlpha(this, ys, ye, xe, xs, xstart, yv, dib, pitch, dxv, dyv, alpha);
        } else 
#endif
        {
          __StretchGlobalChannel::_stretchToRectAlpha(this, ys, ye, xe, xs, xstart, yv, dib, pitch, dxv, dyv, alpha);
        }
      }
    }
  }

#ifndef NO_MMX
  Blenders::BLEND_MMX_END();
#endif
  // write bits back to dib.

  if (usingBlitCanvas) {
    blitToRectCanvas.blit(0, 0, &clone, dst.left, dst.top, cwidth, cheight);
  }
}

COLORREF SkinBitmap::getPixel(int x, int y) {
  ASSERT(bits != NULL);
  if (x < 0 || y < 0 || x >= getFullWidth()-getX() || y>= getFullHeight()-getY()) return (COLORREF)0;
  return (COLORREF)(((int*)bits)[x+getX()+(y+getY())*getFullWidth()]);
}

void *SkinBitmap::getBits() {
  return bits;
}

int SkinBitmap::isInvalid() {
  return last_failed;
}

void SkinBitmap::setHasAlpha(int ha) { 
  has_alpha=ha; 
}

const wchar_t *SkinBitmap::getBitmapName() {
  return bitmapname;
}

