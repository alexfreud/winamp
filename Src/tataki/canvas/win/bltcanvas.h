#ifndef _BLTCANVAS_H
#define _BLTCANVAS_H

#include "canvas.h"
#include <tataki/export.h>
#include <bfc/ptrlist.h>
class TATAKIAPI BltCanvas : public Canvas 
{
public:
	BltCanvas();
  BltCanvas(int w, int h, HWND wnd=NULL, int nb_bpp=32/*, unsigned __int8 *pal=NULL,int palsize=0*/);
  BltCanvas(HBITMAP bmp);
  virtual ~BltCanvas();
  void *getBits();
  HBITMAP getBitmap();
  SkinBitmap *makeSkinBitmap(); // this one makes a new, with own bits
  SkinBitmap *getSkinBitmap(); // this one gives a skinbitmap envoloppe of this bltcanvas
  void disposeSkinBitmap(SkinBitmap *b); // call only after makeSkinBitmap

  void fillBits(COLORREF color);
	
  void vflip(int vert_cells=1);
  void hflip(int hor_cells=1);
  void maskColor(COLORREF from, COLORREF to);
  void makeAlpha(int newalpha=-1); // -1 = premultiply using current alpha

	void DestructiveResize(int w, int h, int nb_bpp = 32); // resizes the bitmap, destroying the contents
private:	// NONPORTABLE
	
	void AllocBitmap(int w, int h, int nb_bpp);
  HBITMAP hbmp, prevbmp;
  PtrList<SkinBitmap> *skinbmps;
  SkinBitmap *envelope;
  BITMAP bm;
  bool ourbmp;
  int bpp;

  //void premultiply(ARGB32 *m_pBits, int nwords, int newalpha=-1);
};

#endif
