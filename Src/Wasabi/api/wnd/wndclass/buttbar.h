#ifndef _BUTTBAR_H
#define _BUTTBAR_H

#include <bfc/common.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <bfc/ptrlist.h>

class ButtonWnd;
class ButtHooker;

#define BUTTBAR_PARENT GuiObjectWnd
/**
  A resizable button bar control.

  @short A resizable button bar
  @author Nullsoft
  @ver 1.0
  @see ButtonWnd
*/
class ButtBar : public BUTTBAR_PARENT {
public:
  // resize modes
  /**
    Resize modes for the button bar.
  */
  enum { NORMAL, STRETCH, STACK };

  /**
    You can set the resize mode for the button bar by specifying it
    via the contructor.

    @see ~ButtBar() setResizeMode()
    @param resizemode The default resize mode.
  */
  ButtBar(int resizemode=NORMAL);
  
  /**
    Deletes all the buttons present in the button bar.

    @see ButtBar()
  */
  virtual ~ButtBar();

  /**
    This event is triggered when the button bar is being initialized.
    If you override this, please call up the parent chain first, then 
    do your initialization.

    @ret 1
  */
  virtual int onInit();
  
  /**
    This event is triggered when the button bar is being resized.
    If you override this, please call up the parent chain first, then 
    do your own resize handling.
  
    @ret 1
  */
  virtual int onResize();
  
  /**
    This event is triggered when the button bar is being painted.
    If you override this, please call up the parent chain first, then 
    do your painting.

    @ret 1
    @param canvas The canvas upon which we will paint ourself.
  */
  virtual int onPaint(Canvas *canvas);
  
  /**
    Sets the resize mode for the button bar.

    @param resizemode NORMAL, Normal Resize; STRETCH, Stretch the button bar to window width; STACK, ?;
  */
  virtual void setResizeMode(int resizemode);

  /**
    Enables you to add a child window to your button bar.
    Since this is a button bar, the windows you can add must be
    derived or be ButtonWnd's.

    @see removeChild()
    @see getNumChildren()
    @see enumChild()
    @see ButtonWnd
    @ret 1
    @param child A pointer to the child window to add.
  */
  int addChild(ButtonWnd *child);
  
  /**
  */
  int removeChild(ButtonWnd *child);	// does not delete, just removes
  
  /**
    Get the number of children (buttons) that the button bar has.
    
    @ret The number of children (buttons).
  */
  int getNumChildren();
  
  /**
    Get a pointer to a child (button) in the button bar, by button index number.
    The first button added is at index 0.
    
    @ret !NULL, a pointer the requested button; NULL, The button does not exist;
  */
  ButtonWnd *enumChild(int n);
  
  /**
    Get the width of the button bar (in pixels).
    
    @see getHeight()
    @ret Width of the button bar (in pixels).
  */
  int getWidth();
  
  /**
    Get the height of the button bar (in pixels).
  
    @see getWidth()
    @ret Height of the button bar (in pixels).
  */
  int getHeight();

  /**
    Event is triggered when the left mouse button is used to click on the
    button bar. Override this to implement your own handling of the event.
    If you override this method, call up the parent chain.

    @param x The x coordinate of the mouse cursor in the button bar.
    @param y The y coordinate of the mouse cursor in the button bar.
  */
  virtual void onLeftPush(int x, int y);

  /**
    Notify a child window via a generic message system.

    @see addChild()
    @ret 
    @param child A pointer to the child window which will receive the notify.
    @param msg The message you want to send to the child.
    @param p1 A user parameter.
    @param p2 A user parameter.
  */
  virtual int childNotify(ifc_window *child, int msg,
                          intptr_t param1=0, intptr_t param2=0);

  // GuiObjectWnd
  virtual void onNewContent();

  void setSpacer(int sp) { spacer = sp; }

  void setGroupLabel(const wchar_t *l);

protected:
  virtual int onLeftPush(int id) { return 0; }

  PtrList<ButtonWnd> buttons;
private:
  int resize_mode;
  int spacer;
  ButtHooker *hooker;
};

#endif
