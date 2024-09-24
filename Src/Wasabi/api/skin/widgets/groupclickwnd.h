/* This class simulates a very basic button thru the use of a group. the group needs to have a guiobject (ie: a transparent layer) 
 with id "mousetrap" on top of the rest of its content. */

#ifndef __GROUPCLICKWND_H
#define __GROUPCLICKWND_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/objects/c_script/h_guiobject.h>

class MouseTrap;


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class GroupClickWnd : public GuiObjectWnd {

  public:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    GroupClickWnd();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual ~GroupClickWnd();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void content_onLeftButtonDown();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void content_onLeftButtonUp();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void content_onRightButtonDown();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void content_onRightButtonUp();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void content_onEnterArea();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void content_onLeaveArea();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void groupclick_onLeftPush();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void groupclick_onRightPush();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void abstract_onNewContent();

  private:
    
    int inarea;  
    MouseTrap *trap;
};


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class MouseTrap : public H_GuiObject {
  public:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    MouseTrap(GroupClickWnd *w, ScriptObject *obj) : H_GuiObject(obj), window(w) { }
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual ~MouseTrap() {}

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onLeftButtonDown(int x, int y);
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onLeftButtonUp(int x, int y);
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onRightButtonDown(int x, int y);
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onRightButtonUp(int x, int y);
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onEnterArea();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onLeaveArea();
  
  private:

    GroupClickWnd *window;
};


#endif
