#ifndef _SLIDER_H
#define _SLIDER_H

#include <bfc/common.h>
#include <tataki/bitmap/autobitmap.h>
#include <api/wnd/wndclass/guiobjwnd.h>

#define SLIDERWND_PARENT GuiObjectWnd
/**
  Slider style control.
  
  @short Slider style control.
  @author Nullsoft
  @ver 1.0
*/
class SliderWnd : public SLIDERWND_PARENT 
{
public:
  /**
    Sets the defaults for the slider. Defaults to a horizontal 
    slider with the thumb in the center and is enabled.
  */
  SliderWnd();
  
  /**
    Nothing is handled by the destructor.
  */
  virtual ~SliderWnd();

  /**
    Event is triggered when the window requires a repaint.
    Override this to implement your own behavior.
    
    Paints the slider on canvas according to current 
    state of the slider.

    @ret 0, Failed; 1, Success;
    @param canvas The canvas on which to paint.
  */
  virtual int onPaint(Canvas *canvas);

  /**
    Event is triggered when the left mouse button is pressed while 
    the slider has focus. Override this to implement your
    own behavior.
    
    @ret 
    @param x X coordinate of the mouse pointer.
    @param y Y coordinate of the mouse pointer.
  */
  virtual int onLeftButtonDown(int x, int y);
  
  /**
    Event is triggered when the mouse has capture on the slider
    and is being moved. Override this to implement your own
    behavior.
    
    @ret 0, Failed; 1, Success;
    @param x The X position of the mouse.
    @param y The Y position of the mouse.
  */
  virtual int onMouseMove(int x, int y); // only called when mouse captured
  
  /**
    Event is triggered when the left mouse button is released.
    Note that the mouse button must have been previously pressed
    for this event to happen. Override this to implement your 
    own behavior.
    
    @ret 1, If you handle the event; 0, If you don't handle the event;
    @param x The X position of the mouse.
    @param y The Y position of the mouse.
  */
  virtual int onLeftButtonUp(int x, int y);
  
  /**
    Event is triggered when the right mouse button is pressed.
    Override this to implement your own behavior.
  */
  virtual int onRightButtonDown(int x, int y);
  
  /**
    Event is triggered when a key is pressed and the slider
    has focus. Override this to implement your own behavior.
    
    @ret 1, If you handle the event; 0, If you don't handle the event;
    @param c The key that was pressed.
  */
  virtual int onChar(unsigned int c);

  /**
    Event is triggered when the mouse enters the region 
    of the slider. Override this to implement your 
    own behavior.
  */
  virtual void onEnterArea();
  
  /**
    Event is triggered when the mouse leaves the region 
    of the slider. Override this to implement your 
    own behavior.
  */
  virtual void onLeaveArea();

  /**
    Event is triggered then the slider is about to be initialized.
    Override this event to implement your own behavior.
    
    By default this will render the slider according the it's current settings
    and position of the thumb.
    
    @ret 1, Success; 0, Failure;
  */
  virtual int onInit();

  /**
    Constants for positioning of the thumb.
  */
  enum {
    START = 0,
    END = 65535,
    FULL = END
  };
  
  /**
    Set the sliders position.
    
    @param newpos The sliders new position.
    @param wantcb !0, Generate a callback after the position has been set; 0, No callback;
  */
  virtual void setPosition(int newpos, int wantcb=1);
  
  /**
    Get the sliders current position. The range is from
    START (0) to END (65535).
    
    @ret The sliders position (ranges from 0 to 65535).
  */
  int getSliderPosition();

  //void cancelSeek();
  
  /**
    Use a base texture when rendering the slider.
    
    @see setBaseTexture()
    @param useit 0, Do not use; 1, Use base texture;
  */
  void setUseBaseTexture(int useit);
  
  /**
    Set the base texture of the slider.
    
    @see setUseBaseTexture()
    @see SkinBitmap
    @param bmp The bitmap to use as a texture.
    @param x  The X position of the base texture.
    @param y  The Y position of the base texture.
  */
  void setBaseTexture(SkinBitmap *bmp, int x, int y);
  
  /**
    Set the draw area to include the edge borders.
    
    @param draw 0, Do not include the edges; 1, Include the edges;
  */
  void setDrawOnBorders(int draw);
  
  /**
    Do not use the default background provided 
    by the current skin?
    
    If you set this to 1, you MUST specify your bitmaps.
    
    @param no 0, Use default background; 1, Do not use default;
  */
  void setNoDefaultBackground(int no);

  /**
    Set the bitmaps to be used to render the slider.
    These include bitmaps for the left, middle, right of
    the slider. For the thumb, we have bitmaps for the
    normal, hilited and pushed thumb. 
    
    The bitmaps are set using their xml id or "name".
    The name should resemble something like this: 
    "studio.seekbar.left".
    
    @see setLeftBmp()
    @see setMiddleBmp()
    @see setRightBmp()
    @see setThumbBmp()
    @see setThumbDownBmp()
    @see setThumbHiliteBmp()
    @param thumbbmp The normal thumb bitmap name.
    @param thumbdownbmp The thumb down bitmap name.
    @param thumbhighbmp The hilited thumb bitmap name.
    @param leftbmp The left bitmap of the slider name.
    @param middlebmp The middle bitmap of the slider name.
    @param rightbmp The right bitmap of the slider name.
  */
  void setBitmaps(const wchar_t *thumbbmp, const wchar_t *thumbdownbmp, const wchar_t *thumbhighbmp, const wchar_t *leftbmp, const wchar_t *middlebmp, const wchar_t *rightbmp);

  /**
    Set the left bitmap of the slider.
    
    @param name The left bitmap name.
  */
  void setLeftBmp(const wchar_t *name);
  
  /**
    Set the middle bitmap of the slider.
    
    @param name The middle bitmap name.
  */
  void setMiddleBmp(const wchar_t *name);
  
  /**
    Set the right bitmap of the slider.
    
    @param name The right bitmap name.
  */
  void setRightBmp(const wchar_t *name);
  
  /**
    Set the normal thumb bitmap of the slider.
    
    @param name The normal thumb bitmap name.
  */
  void setThumbBmp(const wchar_t *name);
  
  /**
    Set the thumb down bitmap of the slider.
    
    @param name The thumb down bitmap name.
  */
  void setThumbDownBmp(const wchar_t *name);
  
  /**
    Set the hilited thumb bitmap of the slider.
    
    @param name The hilited thumb bitmap name.
  */
  void setThumbHiliteBmp(const wchar_t *name);

  /**
    Get the height of the slider in pixels.
    
    @ret The height of the slider (in pixels).
  */
  virtual int getHeight();
  
  /**
    Get the width of the slider in pixels.
    
    @ret The width of the slider (in pixels).
  */
  virtual int getWidth();

  /**
    Get the left bitmap of the slider.
    
    @see SkinBitmap
    @ret The left SkinBitmap.
  */
  SkinBitmap *getLeftBitmap();
  
  /**
    Get the right bitmap of the slider.
    
    @see SkinBitmap
    @ret The right SkinBitmap.
  */
  SkinBitmap *getRightBitmap();

  /**
    Get the middle bitmap of the slider.
    
    @see SkinBitmap
    @ret The middle SkinBitmap.
  */
  SkinBitmap *getMiddleBitmap();

  /**
    Get the thumb bitmap of the slider.
    
    @see SkinBitmap
    @ret The thumb SkinBitmap.
  */
  SkinBitmap *getThumbBitmap();

  /**
    Get the thumb down bitmap of the slider.
    
    @see SkinBitmap
    @ret The thumb down SkinBitmap.
  */
  SkinBitmap *getThumbDownBitmap();

  /**
    Get the thumb hilite bitmap of the slider.
    
    @see SkinBitmap
    @ret The thumb hilite SkinBitmap.
  */
  SkinBitmap *getThumbHiliteBitmap();

  /**
    Set the sliders enable state.
    
    @param en 1, Enabled; 0, Disabled;
  */
    
  virtual void setEnable(int en);
  
  /**
    Get the sliders enable state.
    
    @ret 1, Enabled; 0, Disabled;
  */
  virtual int getEnable(void);

  /**
    Set the orientation of the slider 
    (horizontal or vertical).
    
    @param o 0, Horizontal; 1, Vertical;
  */
  virtual void setOrientation(int o); 
  
  /**
    This will set a "jump-to" position (like "center" for a balance slider).
    The parameter is in thumb coordinates (0 to 65535).
    
    @param h The jump-to position (ranges from 0 to 65535, or START to END).
  */
  virtual void setHotPosition(int h);
  virtual int getHotPosRange() { return hotposrange; }
  virtual void setHotPosRange(int range) { hotposrange = range; }

  /**
    Set the thumb center flag. If on, this flag will
    cause the thumb of the slider to be centered
    automatically.
    
    @param c 1, Centered; 0, No centering;
  */
  virtual void setThumbCentered(int c);
  virtual void setThumbStretched(int c);

  /**
    Set the thumb offset (from the left hand side).
    This offset will be added to the zero position of the thumb.
    Note, if you're using centering also, this will cause the slider
    thumb to be passed the middle of the slider.
    
    @param o The offset of the thumb (in pixels).
  */
  virtual void setThumbOffset(int o);

  /**
    Set the minimum and maximum limit for the slider.
    
    @param minlimit The minimum value.
    @param maxlimit The maximum value.
  */
  virtual void setLimits(int minlimit, int maxlimit);

  virtual int getMaxLimit() { return maxlimit; }
  virtual int getMinLimit() { return minlimit; }
  virtual int getRange() { return maxlimit-minlimit; }

  virtual int onKeyDown(int vkcode);
  
	virtual void onCancelCapture();
protected:
  /**
    Abort the current seek and end capture.
  */
  void abort();

  // override this to get position change notification
  /**
    Event is triggered when the mouse is moving the thumb
    is being moved. Override this to implment your own behavior.
    
    @ret The thumb's position (ranges from 0 to 65535 or START to END).
  */
  virtual int onSetPosition();		// called constantly as mouse moves
  
  /**
    Event is triggered when the thumb is released and the final position
    is about to be set.
    
    @ret The thumb's position (ranges from 0 to 65535 or START to END).
  */
  virtual int onSetFinalPosition();	// called once after move done

  /**
    Get the seeking status.
    
    @ret 1, User is seeking; 0, User is not seeking;
  */
  int getSeekStatus();	// returns 1 if user is sliding tab

  int vertical;	// set to 1 for up-n-down instead

  /**
    Get the width of the thumb bitmap, in pixels.
    
    @ret The thumb's width (in pixels).
  */
  int thumbWidth();
  
  /**
    Get the height of the thumb bitmap, in pixels.
    
    @ret The thumb's width (in pixels).
  */
  int thumbHeight();
  // keyboard
  void move_left(int bigstep);
  void move_right(int bigstep);
  void move_start();
  void move_end();

  int minlimit, maxlimit, length;

private:
  int seeking;
  int enabled;
  int hilite;
  int pos;
  int oldpos;
  int thumbwidth;
  int captured;
  int xShift, yShift;
  SkinBitmap *base_texture;
  int use_base_texture;
  int no_default_background;
  int drawOnBorders;
  int hotPosition;
  int origPos;
  int thumbCentered, thumbOffset, thumbStretched;
  int hotposrange;

  AutoSkinBitmap left, middle, right;
  AutoSkinBitmap thumb, thumbdown, thumbhilite;
};

#endif
