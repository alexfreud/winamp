#ifndef __SCROLLBAR_H
#define __SCROLLBAR_H

#include <api/wnd/virtualwnd.h>
#include <tataki/region/region.h>
#include <api/wnd/usermsg.h>
#include <tataki/bitmap/autobitmap.h>

#define SCROLLBAR_FULL 65535

#define POS_NONE    0
#define POS_LEFT    1
#define POS_BUTTON  2
#define POS_RIGHT   3

#define PAGE_NONE   0
#define PAGE_DOWN   1
#define PAGE_UP     2

#define DEFAULT_HEIGHT 16

#define SCROLLBAR_PARENT VirtualWnd

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class ScrollBar : public SCROLLBAR_PARENT {
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  ScrollBar();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual ~ScrollBar();

  virtual int onMouseMove (int x, int y);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onLeftButtonDown(int x, int y);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onLeftButtonUp(int x, int y);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onRightButtonDown(int x, int y);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onRightButtonUp(int x, int y);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onMouseWheelUp(int clicked, int lines);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onMouseWheelDown(int clicked, int lines);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onPaint(Canvas *canvas);
  
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
  virtual int onInit();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void timerCallback(int id);

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int wantDoubleClicks() { return 0; };

 
  virtual int onSetPosition(bool smooth=false);

  virtual int onSetFinalPosition();

  void setBitmaps(wchar_t *left, wchar_t *lpressed, wchar_t *lhilite,
                  wchar_t *right, wchar_t *rpressed, wchar_t *rhilite, 
                  wchar_t *button, wchar_t *bpressed, wchar_t *bhilite);
  
  void setBackgroundBitmaps(const wchar_t *left, const wchar_t *middle, const wchar_t *right);

  void setPosition(int pos);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getPosition();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getHeight();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setHeight(int newheight);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setNPages(int n);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void gotoPage(int n);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setUpDownValue(int newupdown);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setVertical(bool isvertical);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getWidth();

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void freeResources();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void reloadResources();

private:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void deleteResources();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int getMousePosition();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void calcOverlapping();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void calcXPosition();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void calcPosition();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void handlePageUpDown();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int checkPageUpDown();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void handleUpDown();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int checkUpDown();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int pageUp();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int pageDown();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  int upDown(int which);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void setPrivatePosition(int pos, bool signal=true, bool smooth=false);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void loadBmps();

  AutoSkinBitmap bmpleft, bmplpressed, bmplhilite,
             bmpright, bmprpressed, bmprhilite,
             bmpbutton, bmpbpressed, bmpbhilite,
             bmpbackgroundleft, bmpbackgroundmiddle, bmpbackgroundright;

  RegionI *leftrgn, *rightrgn, *buttonrgn;
  int position;

  int moving;
  int lefting;
  int righting;
  int clicked;

  int buttonx;

  int curmouseposition;
  int clickmouseposition;
  int height;

  int shiftleft, shiftright;
  POINT clickpos;
  int clickbuttonx;
  int pageing;
  int firstdelay;
  int timer;
  int npages;
  int pageway;
  int updown;
  int timer2;
  int insetpos;

  int vertical;
  int lastx, lasty;
};


#endif
