#ifndef __BPAINTWND_H
#define __BPAINTWND_H

#include <api/wnd/wndclass/guiobjwnd.h>

#define BUFFERPAINTWND_PARENT GuiObjectWnd 

class BufferPaintWnd : public BUFFERPAINTWND_PARENT {
  
  public:
    BufferPaintWnd();
    virtual ~BufferPaintWnd();

    virtual int onInit(); 
    virtual int onPaint(Canvas *c); 
  
    virtual int onBufferPaint(BltCanvas *c, int w, int h) { return 1; } 
    virtual int wantEvenAlignment() { return 0; } // if you need even coordinates for your framebuffer, return 1 here
    virtual void getBufferPaintSize(int *w, int *h); // by default returns client width/height
    virtual void getBufferPaintSource(RECT *r); // by default returns the size of the quickpaint canvas
    virtual void getBufferPaintDest(RECT *r); // by default returns the size of client area
    virtual int wantNegativeHeight() { return 0; }
    virtual void invalidateBuffer();
    virtual int onResize();
    virtual void onNewBuffer(int w, int h) {}
 
  protected:
    BltCanvas *render_canvas;

  private:
    void bufferPaint();
    int updateCanvas();

    int canvas_w, canvas_h;

    int invalidated;
};


#endif
