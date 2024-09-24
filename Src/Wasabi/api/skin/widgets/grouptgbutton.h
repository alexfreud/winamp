#ifndef __GROUPTGBUTTON_H
#define __GROUPTGBUTTON_H

#include <api/skin/widgets/groupclickwnd.h>
#include <api/wnd/wndclass/guiobjwnd.h>

#define GROUPTOGGLEBUTTON_PARENT GuiObjectWnd

#define STATUS_OFF 0
#define STATUS_ON  1


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class GroupToggleButton : public GROUPTOGGLEBUTTON_PARENT {

  public:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    GroupToggleButton();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual ~GroupToggleButton();

    
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
    virtual int childNotify(ifc_window *child, int msg, intptr_t param1=0, intptr_t param2=0);

   
    virtual void setGroups(const wchar_t *on, const wchar_t *off);


    virtual void toggle();
 
    virtual int wantFullClick();

   
    virtual void grouptoggle_onLeftPush();
 
    virtual void grouptoggle_onRightPush();


    virtual void setStatus(int s);

    virtual int getStatus() { return status; }
  
    virtual int wantAutoToggle() { return 1; }

    virtual GroupClickWnd *enumGroups(int n);
    

    virtual int getNumGroups();

  private:

 
    void initGroups();
    
    GroupClickWnd on;
    GroupClickWnd off;

    StringW on_id, off_id;

    int status;
};

#endif
