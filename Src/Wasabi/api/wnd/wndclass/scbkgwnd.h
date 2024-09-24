#ifndef __SCRLBKGWND_H
#define __SCRLBKGWND_H

#include <tataki/canvas/canvas.h>
#include <tataki/bitmap/autobitmap.h>
#include <api/wnd/wndclass/labelwnd.h>
#include <api/wnd/wndclass/scrollbar.h>
#include <api/wnd/wndclass/sepwnd.h>

#define SCRLBKGWND_PARENT LabelWnd


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class ScrlBkgWnd : public SCRLBKGWND_PARENT {
protected:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  ScrlBkgWnd();
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual ~ScrlBkgWnd();
  
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onInit();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onPaint(Canvas *c);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void drawBackground(Canvas *canvas);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onEraseBkgnd(HDC dc);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onResize();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void getClientRect(RECT *r);
//  virtual void getNonClientRect(RECT *r);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int getHeaderHeight();
  virtual void timerCallback (int id);

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void onHScrollToggle(int set);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void onVScrollToggle(int set);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void onSetVisible(int show);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int wantHScroll() { return 1; }
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int wantVScroll() { return 1; }
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void makeWindowOverlayMask(api_region *r);

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  SkinBitmap *getBgBitmap(void);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setBgBitmap(const wchar_t *b);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setBgColor(ARGB32 rgb);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual ARGB32 getBgColor(void);

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int getContentsWidth();	// not safe to call getclientrect!
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int getContentsHeight();	// not safe to call getclientrect!

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setLineHeight(int h);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getLinesPerPage();

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getScrollX();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getScrollY();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getScrollbarWidth();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void scrollToY(int y, int signal=TRUE);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void scrollToX(int x, int signal=TRUE);

protected:

  virtual void onScrollY(int y) { }
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setSlidersPosition();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int needDoubleBuffer();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  bool needHScroll();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  bool needVScroll();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getMaxScrollY();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getMaxScrollX();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void updateScrollY(bool smooth=false);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void updateScrollX(bool smooth=false);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void smoothScrollToY(int y);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void smoothScrollToX(int x);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void updateVScroll(int y);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void updateHScroll(int x);

  AutoSkinBitmap bmp;
  
  int dbbuffer;
  bool inDestroy;

  ScrollBar hScroll;
  ScrollBar vScroll;
	SepWnd hSep;
	SepWnd vSep;

	ARGB32 bgColor;

	int scrollX;
	int scrollY;

	bool needSetSliders;
	bool wantsep;
	bool wantTileBg;

	int lineHeight;

	float smoothScrollYInc, smoothScrollXInc;
	float smoothScrollYCur, smoothScrollXCur;
	int smoothScrollYTimerCount, smoothScrollXTimerCount;
	int smoothYTimer, smoothXTimer;
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void killSmoothYTimer();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void killSmoothXTimer();
  double lastratio;
  RECT smsqr;
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void _setSlidersPosition();
  int in_set_slider_position;
};

#endif
