#ifndef __BBWND_H
#define __BBWND_H

#include <api/wnd/wndclass/abstractwndhold.h>

#ifdef WASABI_COMPILE_SKIN
#define BBWND_PARENT AbstractWndHolder
#else
#define BBWND_PARENT ServiceWndHolder
#endif

/**
  class BackBufferWnd
  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class BackBufferWnd : public BBWND_PARENT {
  
  public:

    BackBufferWnd();
    virtual ~BackBufferWnd();

    virtual int onPaint(Canvas *c); 

    /**
      BackBufferWnd method wantBackBuffer .
      
      @ret 0
      @param None
    */
    virtual int wantBackBuffer() { return 0; }
    virtual BltCanvas *getBackBuffer();
    virtual int onSiblingInvalidateRgn(api_region *r, ifc_window *who, int who_idx, int my_idx);
    
    /**
      BackBufferWnd method wantSiblingInvalidations .
    
      @ret 0
      @param None
    */
    virtual int wantSiblingInvalidations() { return wantBackBuffer(); }

  private:

    int backbuffer;
    BltCanvas *back_buffer;
    int canvas_w, canvas_h;
};

#endif
