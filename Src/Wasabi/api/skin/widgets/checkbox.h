#ifndef __CHECKBOX_H
#define __CHECKBOX_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/objects/c_script/h_guiobject.h>
#include <api/script/objects/c_script/h_button.h>

#define  CHECKBOX_PARENT GuiObjectWnd

class TextClicks;
class ToggleClicks;


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class CheckBox : public CHECKBOX_PARENT {
  
  public:

    CheckBox(const wchar_t *_text = L"ERROR", const wchar_t *_radioid = NULL);
    virtual ~CheckBox();

    
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
    virtual int getPreferences(int what);

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void toggle(int self_switch);  // this is called by the click catchers.

    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void setActivated(int activated, int writetocfg=1);
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    int isActivated();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void setText(const wchar_t *_text);
    const wchar_t *getText();
    void setRadioid(const wchar_t *_radioid);
    const wchar_t *getRadioid() { return radioid; }
    void setRadioVal(const wchar_t *val, int use_radioval=TRUE);
    const wchar_t *getRadioVal() { return radioval; }

    
#ifdef WASABI_COMPILE_CONFIG
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual int onReloadConfig();
#endif
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void onNewContent();

    virtual int wantFocus() { return 1; }
    virtual int onChar(unsigned int c);

    virtual void setAction(const wchar_t *str);
    virtual void setActionTarget(const wchar_t *target);
    virtual void setActionParam(const wchar_t *param);
    virtual const wchar_t *getActionParam();

  protected:
    virtual void onToggle(); //override to catch toggles

  private:

    void doAction();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void updateText();

    TextClicks *textclicks;
    ToggleClicks *toggleclicks;
    StringW text;
    StringW radioid;
    GuiObject *buttonGuiObj;
    StringW radioval;
    int use_radioval;
    StringW action, action_target, action_param;
};


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class TextClicks : public H_GuiObject {
  public:
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    TextClicks(ScriptObject *textobj, CheckBox *_callback) :
        
        /**
          Method
        
          @see 
          @ret 
          @param 
        */
        callback(_callback), H_GuiObject(textobj) {
    }

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onLeftButtonDown(int x, int y) {
      callback->toggle(0);
    }
  private:
    CheckBox *callback;
};


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class ToggleClicks : public H_Button {
  public:
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    ToggleClicks(ScriptObject *button, CheckBox *_callback) :
        
        /**
          Method
        
          @see 
          @ret 
          @param 
        */
        callback(_callback), H_Button(button) {
        inhere=0;
    }

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onActivate(int activate) {
      if (inhere) return;
      inhere=1;
      callback->toggle(1);
      inhere=0;
    }

  private:
    CheckBox *callback;
    int inhere;
};


#endif
