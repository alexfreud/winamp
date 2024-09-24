#ifndef __GUIRADIOGROUP_H
#define __GUIRADIOGROUP_H

#include <api/skin/nakedobject.h>
#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>
                              
#define  GUIRADIOGROUP_PARENT NakedObject



/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class GuiRadioGroup : public GUIRADIOGROUP_PARENT {
  
  public:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void toggleChild(GuiObject *who);
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void registerChild(GuiObject *who);

    // From BaseWnd
    

    virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);

  private:

    PtrList<GuiObject> children;
};

#endif
