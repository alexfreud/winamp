//PORTABLE
#ifndef _TEXTBAR_H
#define _TEXTBAR_H

#include <bfc/virtualwnd.h>
#include <bfc/autobitmap.h>
#include <bfc/textalign.h>

class CheckWnd;

#define TEXTBAR_PARENT VirtualWnd
/**
  TextBar uses the BaseWnd name field of the object as the text 
  to be displayed.

  @short TextBar control.
  @author Nullsoft
  @ver 1.0
  @see LabelWnd
*/
class TextBar : public VirtualWnd {
public:
  /**
    Sets the default flags of the TextBar. Defaults to 16px fonts, 
    no background texture, left justified text, shadowed text in 
    the bgcolor, no outline, not box around the text.
  */
  TextBar();

  /**
    Event is triggered when the window requires a repaint.
    Override this to implement your own behavior.
    
    Paints the bitmap on canvas according to current 
    options (centering, tiling, stretching, title).

    @ret 0 for failure, 1 for success
    @param canvas The canvas on which to paint.
  */
  virtual int onPaint(Canvas *canvas);

  /**
    Event is triggered when the name of the window is changed.
    Override this to implement your own behavior.
    
    @see BaseWnd::setName()
  */
  virtual void onSetName();
  
  /**
    Set the text to be displayed to an ascii representation of a numeric value.
    
    @ret 1.
    @param i The numeric value to be displayed.
  */
  int setInt(int i);

  /**
    Set the size of the text for the textbar.
    
    @ret 1, success; 0, failure.
    @param newsize The new text size, range is from 1 to 72 pixels.
  */
  int setTextSize(int newsize);

  /**
    Get the width of the text displayed, in pixels.
    
    @ret Width of the displayed text (in pixels).
  */
  int getTextWidth();
  
  /**
    Get the height of the text displayed, in pixels.
    
    @ret Height of the displayed text.
  */
  int getTextHeight();
  
  /**
    Use the base texture when rendering the TextBar?
    If the base texture is used, it will be rendered as 
    the background of the textbar.
    
    @param u !0, Use base texture; 0, Do not use base texture;
  */
  void setUseBaseTexture(int u);
  
  /**
    Event is triggered when the left mouse button is pressed while 
    the textbar has focus. Override this to implement your
    own behavior.
    
    @param x X coordinate of the mouse pointer.
    @param y Y coordinate of the mouse pointer.
  */
  virtual int onLeftButtonDown(int x, int y);

  /**
    Center the text in the textbar? If not, 
    it will be left justified by default.
    
    @param center !0, Center text; 0, Do not center text;
  */
//  void setCenter(int center);  //old code

  /**
    Get the center text flag.
    
    @see setCenter()    
    @ret TRUE, Text is being centered; FALSE, No centering (left justified);
  */
//  bool getCentered();

  /**
    Sets the alignment of the text to left, center, or right aligned
    (possibly more later on, not too sure yet)
  */
  void setAlign(TextAlign alignment);

  /**
    @ret returns the alignment of the text
  */
  TextAlign getAlign();
  

  // The following three options have ascending overriding priority --

  /**
    Sets the shadowed text flag. If enabled, the text will be shadowed
    with the "bgcolor" value.
    
    @see getTextShadowed()
    @param settextshadowed !0, Shadow the text; 0, Do not shadow the text;
  */
  void setTextShadowed(int settextshadowed) { 
    textshadowed = !!settextshadowed;
  }
  
  /**
    Get the shadowed text flag. If enabled, the text will be shadowed
    with the "bgcolor" value.
    
    @see setTextShadowed()
    @ret !0, Shadow the text; 0, Do not shadow the text;
  */
  int getTextShadowed() {
    return textshadowed;
  }
  
  /**
    Sets the outline text flag. If enabled, the text will be
    outlined with the "bgcolor" value.
    
    @param settextoutlined !0, Outline the text; 0, Do not outline the text;
  */
  void setTextOutlined(int settextoutlined) { 
    textoutlined = !!settextoutlined;
  }
  
  /**
    Get the outline text flag. If enabled, the text will be
    outlined with the "bgcolor" value.
    
    @ret !0, Outline the text; 0, Do not outline the text;
  */
  int getTextOutlined() {
    return textoutlined;
  }
 
  /**
    Set the drawbox flag. If true, the drawbox flag will cause
    a box to be drawn around the text in the textbar.
    
    @param setdrawbox !0, Drawbox around the text; 0, No drawbox;
  */
  void setDrawBox(int setdrawbox) {
    drawbox = !!setdrawbox;
  }
  
  /**
    Get the drawbox flag. If true, the drawbox flag will cause
    a box to be drawn around the text in the textbar.
    
    @ret !0, Drawbox around the text; 0, No drawbox;
  */
  int getDrawBox() {
    return drawbox;
  }
  
  /**
    Associate a checkbox with the textbar. When a textbar is linked
    to a checkbox, it will toggle the checkbox when it receives
    left clicks.
    
    @param target A pointer to the CheckWnd to link.
  */
  void setAutoToggleCheckWnd(CheckWnd *target) {
    checkwndtarget = target;
  }


private:
  int size;
  int usebt;
  TextAlign alignment;  //i changed this from centered, to a set text alignment thingie


  int textshadowed; // display a shadow of the text in bgcolor.  default: on
  int textoutlined; // draw an outline of the text in bgcolor.  default: off
  int drawbox;      // draw a box of bgcolor the size of the boundsrect.  default: off

  AutoSkinBitmap bgbitmap;
  CheckWnd *checkwndtarget;
};

const int TEXTBAR_LEFTMARGIN = 2;

#endif
