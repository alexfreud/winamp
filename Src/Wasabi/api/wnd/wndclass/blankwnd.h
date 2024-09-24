#ifndef _BLANKWND_H
#define _BLANKWND_H

#include <bfc/common.h>
#include <api/wnd/virtualwnd.h>

#define BLANKWND_PARENT VirtualWnd

/**
  Class BlankWnd provides blank windows. The initial color can be set in the 
	constructor, with a default of black. There is a method for painting the window from a Canvas.

  @short Blank Window with background color.
  @author Nullsoft
  @ver 1.0
  @see VirtualWnd
*/
class BlankWnd : public BLANKWND_PARENT {
public:
  /**
    You can set the background color for the window via an RGB value.
    The RGB value is contructed using RGB(), like so RGB(Red, Green, Blue);
    
    @param color The RGB value of the background color to use.
  */
  BlankWnd(RGB32 color=RGB(0,0,0));
  
  /**
    This event is triggered when the window needs to be repainted.
    Override it to implement your own handling of this event.
    
    @ret 1, If you handle the event;
    @param canvas A pointer to the canvas on which will we paint.
  */
  virtual int onPaint(Canvas *canvas);

private:
  RGB32 color;
};

#endif
