#ifndef _BUTTWND_H
#define _BUTTWND_H

#include <wasabicfg.h>

#include <bfc/common.h>
#include <tataki/canvas/canvas.h>
#include <tataki/bitmap/autobitmap.h>
#include <api/wnd/wndclass/guiobjwnd.h>	
#include <tataki/color/skinclr.h>
#include <api/wnd/accessible.h>
#include <api/wnd/textalign.h>

class api_region;

#define DEFAULT_BUTTON_TEXT_SIZE 14
/**
  Button Text Alignment
	Darkain:  this was changed to use TextAlign
*/
/*
typedef enum {
  BUTTONJUSTIFY_LEFT,
  BUTTONJUSTIFY_CENTER
} ButtonJustify;
*/

#define DEFEREDCB_DOWN 0x450
#define DEFEREDCB_UP   0x451

#define BUTTONWND_PARENT GuiObjectWnd

/**
  A fully skinnable button. Has images for normal, hilited, activated states.
  Plus images for a checked state. It may also be used to draw OS style buttons.
  See setBorderStyle() for more details.
  
  @short Button control.
  @author Nullsoft
  @ver 1.0
  @see ButtBar
*/
class ButtonWnd : public BUTTONWND_PARENT {
public:
  /**
    Sets defaults for ButtonWnd objects.
    
    @see ~ButtonWnd()
    @param button_text The button's caption.
  */
  ButtonWnd(const wchar_t *button_text=NULL);
  
  /**
    Deletes components of ButtonWnd.
    
    @see ButtonWnd()
  */
  virtual ~ButtonWnd();

  /**
    Paints the bitmap on canvas according 
	  to current options (centering, tiling, stretching, title).

    @ret 0 for failure, 1 for success
    @param canvas The canvas on which to paint.
  */
  virtual int onPaint(Canvas *canvas);

  /**
    Sets the bitmaps that will be used to render the button.
    This includes bitmaps for various button states. Also enables
    you to set the colorgroup (gammagroup) for the bitmaps.
    
    @ret 1
    @param _normal Bitmap for normal state.
    @param _pushed Bitmap for pushed state.
    @param _hilited Bitmap for hilited state.
    @param _activated Bitmap for activated state.
    @param colorgroup The colorgroup for the bitmaps (gammagroup).
  */
  int setBitmaps(const wchar_t *normal, const wchar_t *pushed=NULL, const wchar_t *hilited=NULL, const wchar_t *activated=NULL);

  SkinBitmap *getNormalBitmap();
  
  /**
    Sets the bitmaps that will be used to render the button.
    This includes bitmaps for various button states. Also enables
    you to set the colorgroup (gammagroup) for the bitmaps.
    
    @ret 1
    @param hInst The parent window's instance handle.
    @param _normal Bitmap for normal state.
    @param _pushed Bitmap for pushed state.
    @param _hilited Bitmap for hilited state.
    @param _activated Bitmap for activated state.
    @param colorgroup The colorgroup for the bitmaps (gammagroup).
  */
  int setBitmaps(OSMODULEHANDLE hInst, int normal, int pushed, int hilited, int activated, const wchar_t *colorgroup=NULL);
  
  /**
    Set the right bitmap to be used.

    @see setBitmaps()
    @ret 1
    @param bitmap The name of the bitmap to use.
  */
  int setRightBitmap(const wchar_t *bitmap);
  
  /**
    Center the bitmap?

    @see setBitmaps()
    @ret Normalized flag
    @param centerit A non zero value will center the bitmap.
  */
  int setBitmapCenter(int centerit);
  
  /**
    Sets base texture and causes rerendering.

    @see setBaseTexture()
    @param useit A non zero value will use the base texture.
  */
  void setUseBaseTexture(int useit);
  
  /**
    Sets bitmap for button, sets position for button, flags whether to tile the bitmap

    @see setUseBaseTexture()
    @param bmp Skin bitmap for button
    @param x Button position on x-coordinate
    @param yButton position on y-coordinate
    @param tile Flag
  */
  void setBaseTexture(SkinBitmap *bmp, int x, int y, int tile=0);
  
  /**
    Sets the colorgroup (gammagroup) for all the bitmaps associated with
    this button.
    
    @param _colorgroup The colorgroup for the bitmaps.
  */
  void setHInstanceColorGroup(const wchar_t *_colorgroup) { colorgroup = _colorgroup; }

  /**
    Writes given text to button in given size and triggers rendering.
	  
	  @see getButtonText()
    @assert Text string is not empty
    @ret 1
    @param text Label text
    @param size Size to render label text
  */
  int setButtonText(const wchar_t *text, int size=DEFAULT_BUTTON_TEXT_SIZE);

  /**
    Gets text from button.
  
    @see setButtonText()
    @ret Button text string
  */
  const wchar_t * getButtonText();

  /**
    Sets text to render at left, in center, or at right.

    @see setButtonText()
    @see getButtonText()
    @see ButtonJustify
    @param jus BUTTONJUSTIFY_LEFT, left justified; BUTTONJUSTIFY_CENTER, centered;
  */
//  void setTextJustification(ButtonJustify jus);
	void setTextAlign(TextAlign align);

	TextAlign getTextAlign() { return alignment; }
  
  /**
    Enables and disables wantfocus for the button. When disabled, the button can
    never receive focus.
    
    @param want !0, enable focus; 0, disable focus;
  */
  void setWantFocus(int want) { iwantfocus = !!want; }
  
  /**
    Return the wantfocus
  */
  virtual int wantFocus() const { return iwantfocus; }
  
  /**
    Event is triggered when the mouse leaves the button's region.
    Override this event to implement your own behavior.
  */
  virtual void onLeaveArea();
  virtual void onEnterArea();

  /**
    Gets width of button, allowing for length of text plus button margin, if any.

    @see getHeight()
    @ret Button width (in pixels).
  */
  int getWidth();	// our preferred width and height (from bitmaps)
  
  /**
    Gets height of button, allowing for height of text plus button margin, if any.

    @see getWidth()
    @ret Button height (in pixels).
  */
  int getHeight();

  /**
    Event is triggered when focus is given to the button.
    Override this event to implement your own behavior.

    @see onKillFocus()
    @ret 1
  */
  virtual int onGetFocus();
  
  /**
    Event is triggered when the button focus is lost.
    Override this event to implement your own behavior.

    @see onGetFocus()
    @ret 1
  */
  virtual int onKillFocus();
  
  /**
    Event is triggered when a key is pressed and the button
    has focus.

    @ret 1, if you handle the event;
    @param c The value of the key that was pressed.
  */
  virtual int onChar(unsigned int c);

  /**
    Saves new status and rerenders, if button enabled status changes.
    
    @see getEnabled()
    @see onEnable()
    @param _enabled 0, disabled; !0 enabled;
  */
  void enableButton(int enabled);	// can be pushed

  /**
    Tells parent to handle left button click.
	  
	  @see onRightPush()
    @param x Mouse click x-coordinate
    @param y Mouse click y-coordinate
  */
  virtual void onLeftPush(int x, int y);
  
  /**
    Passes right mouse clicks to the parent.
  
    @see onLeftPush()
    @param x Mouse click x-coordinate
    @param y Mouse click y-coordinate
  */
  virtual void onRightPush(int x, int y);
  
  /**
    Passes left double click to parent.

    @see onRightDoubleClick()
    @param x Mouse click x-coordinate
    @param y Mouse click y-coordinate
  */
  virtual void onLeftDoubleClick(int x, int y);
  
  /**
    Passes right double click to parent

    @see onLeftDoubleClick()
    @param x Mouse click x-coordinate
    @param y Mouse click y-coordinate
  */
  virtual void onRightDoubleClick(int x, int y);

  /**
    Event is triggered when the button will be resized.
    Override this event to implement your own behavior.
    
    The default behavior is to cause a repaint.

    @ret 1
  */
  virtual int onResize();

  /**
    Sets the region pointed at after each mouse move. 
    If the region has changed, it invalidate the region 
    so that it will be updated on the screen.
	  
    @ret Status from parent class
    @param x New x-coordinate of mouse cursor
    @param y New y-coordinate of mouse cursor
  */
  virtual int onMouseMove(int x, int y);	// need to catch region changes

  /**
    Event is triggered when the button is enabled or disabled.
    Override this event to implement your own behavior.

    @see getEnabled()
    @ret 1
    @param is The enable state (nonzero is enabled).
  */
  virtual int onEnable(int is);

  /**
    Returns the value of the enabled flag.
  
    @see enableButton()
    @see onEnable()
    @ret enabled
  */
  virtual int getEnabled() const;
  
  /**
    Get the preferences for this button.
    This will enable you to read the suggested width and height
    for the button.

    @ret Width or height of the normal bitmap, as requested, or a property from the parent class.
    @param what SUGGESTED_W, will return the width; SUGGESTED_H, will return the height;
  */
  virtual int getPreferences(int what);

  /**
    Get the button state. This is the state caused by user interaction.
    
    @ret !0, pushed; 0, not pushed;
  */
  virtual int userDown() { return userdown; }
  
  /**
    
  */
  virtual int wantClicks() { return getEnabled(); }

  /**
    Set the bitmap to use when the button will be "checked".
    This enables you to have checked buttons and menu items.

    @see setChecked()
    @see getChecked()
    @param checkbm The name of the bitmap to use.
  */
  void setCheckBitmap(const wchar_t *checkbm);
  
  /**
    Set the checked state of the button.
    
    @param c <0, not checked; 0, none, >0 checked;
  */
  void setChecked(int c) { checked=c; }; // <0=nocheck, 0=none, >0=checked
  
  /**
    Get the checked state of the button.
    
    @ret <0, not checked; 0, none; >0 checked;
  */
  int  getChecked() const { return checked; }
  
  /**
    Triggers rerendering in the opposite
	  highlight state if the hilighting flag is changed.

    @see getHilite()
    @param h
  */
  void setHilite(int h);
  
  /**
    

    @see setHilite()
    @ret Is either highlighting flag set?
  */
  int getHilite();
  
  /**
    Simulate a button push. You can use this method to simulate
    menu pushing also.
    
    @see getPushed()
    @param p A nonzero value will simulate a push.
  */
  void setPushed(int p); // used by menus to simulate pushing

  /**
    Get the pushed state of a button. 

    @see setPushed()
    @ret 0, not pushed; !0, pushed;
  */
  int getPushed() const; // used by menus to simulate pushing
  
  /**
    Sets the auto dim state. Autodim will dim the normal 
    bitmap if no hilite bitmap is provided.
    
    @param ad !0, autodim on; 0, autodim off;
  */
  void setAutoDim(int ad) { autodim=!!ad; } // nonzero makes it dim if there's no hilite bitmap
  
  /**
    Get the autodim state.
    
    @see setAutoDim()
    @ret 0, autodim off; !0 autodim on;
  */
  int getAutoDim() const { return autodim; } // nonzero makes it dim if there's no hilite bitmap
  
  /**
    Set the active state of the button. 

    @see getActivatedButton()
    @see setActivatedNoCallback()
    @param a !0, activate the button; 0, deactivate the button;
  */
  virtual void setActivatedButton(int a);
  
  /**
    Set the active state of the button, without generating a callback.
    This means that the onActivated event will not fire for this button.
    
    @see getActivatedButton()
    @see setActivatedButton()
    @param a !0, activate the button; 0, deactivate the button;
  */
  virtual void setActivatedNoCallback(int a);
  
  /**
    Get the active state of the button.

    @see setActivatedButton()
    @ret activated !0, active; 0, inactive;
  */
  virtual int getActivatedButton();
  
  /**
    Render borders around the button?
    
    @param b !0, borders; 0, no borders;
  */
  void setBorders(int b);
  
  /**
    Sets the border style for the button. This 
    has no effect if no borders are being drawn.
    
    "button_normal"       A normal button.
    "osbutton_normal"     A normal OS button (if in Windows, will show a std win32 button). 
    "osbutton_close"      An OS close button.
    "osbutton_minimize"   An OS minimize button.
    "osbutton_maximize"   An OS maximize button.
    
    @see getBorderStyle()
    @param style The style of button you want.
  */
  void setBorderStyle(const wchar_t *style);
  
  /**
    Get the border style of the button (if there is one).
    If no border is drawn, this method always returns NULL.
    
    @see setBorderStyle()
    @ret The border style.
  */
  const wchar_t *getBorderStyle();
  
  /**
    Set the inactive alpha blending value. This is the alpha blending
    value that will be used for blending when the button does NOT have focus.
    
    @param a The alpha value, range is from 0 (fully transparent) to 255 (fully opaque).
  */
  void setInactiveAlpha(int a);
  
  /**
    Set the active alpha blending value. This is the alpha blending value 
    that will be used for blending when the button HAS focus.
    
    @param a The alpha value, range is from 0 (fully transparent) to 255 (fully opaque).
  */
  void setActiveAlpha(int a);

  /**
    Sets the colors for various states of our button. This is 
    done via element id's which are in the skin xml or registered
    as seperate xml.

    @param text Normal text color (window has focus but button is not active).
    @param hilite Hilited text color (button has focus).
    @param dimmed Dimmed text color (parent window doesn't even have focus).
  */
  void setColors(const wchar_t *text=L"studio.button.text", const wchar_t *hilite=L"studio.button.hiliteText", const wchar_t *dimmed=L"studio.button.dimmedText");
  
  /**
    Deletes the regions and resets them to NULL.

    @see reloadResources()
  */
  virtual void freeResources();
  
  /**
    Reinitializes regions for which there are bitmaps available.
	  
	  @see freeResources()
  */
  virtual void reloadResources();

  /**
    Event is triggered when the is being activated.
    Override this event to implement your own behavior.

    @see setActivatedButton()
    @ret 1
    @param active The button's state (nonzero is active).
  */
  virtual int onActivateButton(int active);
  
  /**
    Returns the current region of the button.
    
    @see api_region
    @ret The region of the button.
  */
  virtual api_region *getRegion();
  
  /**
    Set the modal return. This is what will be returned
    when the window is closed and the window is set to modal.
    
    @param r The return code you wish to set.
  */
  virtual void setModalRetCode(int r);
  
  /**
    Get the modal return code for the window.
    
    @ret The modal return code.
  */
  virtual int getModalRetCode() const;
  
  /**
    Event is triggered when the button is about to be initialized.
    Override this event to implement your own behavior.
  
    @ret 1
  */
  virtual int onInit();
  virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

  virtual void setTextColor(const wchar_t *text);
  virtual void setTextHoverColor(const wchar_t *text);
  virtual void setTextDimmedColor(const wchar_t *text);

  virtual void checkState(POINT *pt=NULL);
  virtual void onCancelCapture();

private:
  AutoSkinBitmap normalbmp, pushedbmp, hilitebmp, checkbmp, rightbmp, activatedbmp;
  SkinBitmap *base_texture;
  RegionI *normalrgn, *pushedrgn, *hirgn, *currgn, *activatedrgn;
  int textsize;
  TextAlign alignment;
  SkinColor color_text, color_hilite, color_dimmed;
  int retcode;

  StringW normalBmpStr, pushedBmpStr, hilitedBmpStr, activatedBmpStr;

  int folderstyle;
  int autodim;
  int userhilite;
  int userdown;
  int activated;
  int enabled;
  int borders;
  const wchar_t *borderstyle;
  int dsoNormal, dsoPushed, dsoDisabled;

  int iwantfocus;
  int center_bitmap;
  int use_base_texture;

  int checked;
  int xShift, yShift, tile_base_texture;

  int inactivealpha, activealpha;
  StringW colorgroup;
  int forcedown;
};

#endif
