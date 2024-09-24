#ifndef __PATHPICKER_H
#define __PATHPICKER_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/objects/c_script/h_guiobject.h>
#include <api/script/objects/c_script/h_button.h>

#define  PATHPICKER_PARENT GuiObjectWnd

class PPClicksCallback;
extern int __id;


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class PathPicker : public PATHPICKER_PARENT {
  
  public:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    PathPicker();
    

    virtual ~PathPicker();

    
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
    void clickCallback();

    
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
    virtual void abstract_onNewContent();
    void setPath(const wchar_t *newpath);
    const wchar_t *getPath() { return curpath; }

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void onPathChanged(const wchar_t *newpath);
   
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void setDefault();

  private:
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void updatePathInControl();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
#ifdef WASABI_COMPILE_CONFIG
    void updatePathFromConfig();
#endif    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void trapControls();

    PPClicksCallback *clicks_button;
    StringW curpath;
    int disable_cfg_event;
};


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class PPClicksCallback : public H_GuiObject {
  public:
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    PPClicksCallback(ScriptObject *trap, PathPicker *_callback) :
        
        /**
          Method
        
          @see 
          @ret 
          @param 
        */
        callback(_callback), H_GuiObject(trap) {
    }

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual void hook_onLeftButtonDown(int x, int y) {
      callback->clickCallback();
    }
  private:
    PathPicker *callback;
};

#endif
