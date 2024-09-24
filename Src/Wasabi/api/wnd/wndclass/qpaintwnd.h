#ifndef __QPAINTWND_H
#define __QPAINTWND_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/service/svcs/svc_skinfilter.h>

#define QUICKPAINTWND_PARENT GuiObjectWnd 

/**
  class QuickPaintWnd .

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
  @cat BFC
*/
class QuickPaintContext;
class QuickPaintWnd : public QUICKPAINTWND_PARENT {
  
  public:
    /**
      QuickPaintWnd constructor .

      @see ~QuickPaintWnd()
    */
    QuickPaintWnd();

    /**
      Destructor for QuickPaintWnd .

      @see QuickPaintWnd()
    */
    virtual ~QuickPaintWnd();

    /**
      QuickPaintWnd method onInit .

      @ret 1
    */
    virtual int onInit(); 
    virtual int onPaint(Canvas *c); 
  
    /**
      QuickPaintWnd method timerCallback .

      @param id Identifies requested action
    */
    virtual void timerCallback(int id);
    virtual void onSetVisible(int show);

    /**
      QuickPaintWnd method setRealtime .

      @see getRealtime()
      @param rt 
    */
    virtual void setRealtime(int rt);
		int getRealtime() const;

    /**
      QuickPaintWnd method setSpeed sets the timer interval in milliseconds. 

      @see getSpeed()
      @param ms The timer interval in milliseconds.
    */
    virtual void setSpeed(int ms);
    
    /**
      QuickPaintWnd method getSpeed gets the timer interval in milliseconds. 

      @see setSpeed()
      @param ms The timer interval in milliseconds.
    */
    virtual int getSpeed();

    /**
      QuickPaintWnd method startQuickPaint .
    */
    virtual void startQuickPaint();

    /**
      QuickPaintWnd method stopQuickPaint .
    */
    virtual void stopQuickPaint();
    
    /**
      QuickPaintWnd method isQuickPainting .
    */
    virtual int isQuickPainting();

    virtual int onQuickPaint(BltCanvas *c, int w, int h, int newone) { return 0; } // return 1 if your content has changed, or 0 to cancel update of your buffer to the window
    virtual int wantEvenAlignment() { return 0; } // if you need even coordinates for your framebuffer, return 1 here
    
    /**
      QuickPaintWnd method getQuickPaintSize gets the client area width and 
      height.

      @param w A pointer to the width to fill.
      @param h A pointer to the height to fill.
    */
    virtual void getQuickPaintSize(int *w, int *h); // by default returns client width/height

    /**
      QuickPaintWnd method getQuickPaintSource .

      @see getQuickPaintSize()
      @assert r exists.
      @ret None
      @except 
      @param r 
    */
    virtual void getQuickPaintSource(RECT *r); // by default returns the size of the quickpaint canvas

    /**
      QuickPaintWnd method getQuickPaintDest .

      @see getQuickPaintSource()
      @assert r exists.
      @param r
    */
    virtual void getQuickPaintDest(RECT *r); // by default returns the size of client area
    virtual int wantNegativeHeight() { return 0; }
    virtual int wantFilters() { return 0; }
    virtual const wchar_t *getFiltersGroup() { return L"Vis/Eq"; }
 
		protected:
int invalidated;
  private:
    /**
      QuickPaintWnd method quickPaint .
    */
		friend class QuickPaintContext;
    int quickPaint();
		void KillThread();
		void CreateRenderThread();
    int realtime;
		volatile LONG invalidates_required;
    BltCanvas *render_canvas1, *render_canvas2, *paint_canvas;
		void SetPaintingCanvas(BltCanvas *c);
		BltCanvas *&GetDrawingConvas();
    int canvas_w, canvas_h;
    int speed;
    int timerset;
    int enabled;

    
    PtrList<svc_skinFilter>filters;
    SkinFilterEnum sfe;
		QuickPaintContext *thread_context;
};


#endif
