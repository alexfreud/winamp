#ifndef __TITLEBOX_H
#define __TITLEBOX_H

#include <api/wnd/wndclass/guiobjwnd.h>

#define TITLEBOX_PARENT GuiObjectWnd 


/**
  Titlebox  

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class TitleBox : public TITLEBOX_PARENT {

  public:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    TitleBox();
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual ~TitleBox();

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual int onInit();
    
    virtual int onResize();
    virtual int getCentered() { return centered; }
    virtual void setCentered(int _centered);
    virtual void setTitle(const wchar_t *t);
    virtual const wchar_t *getTitle() { return title; }
    virtual void setSuffix(const wchar_t *suffix);
    const wchar_t *getSuffix() { return suffix; }

    virtual void onNewContent();
    
    
    virtual void setChildGroup(const wchar_t *grp);
    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    virtual int getPreferences(int what);

  private:

    
    /**
      Method
    
      @see 
      @ret 
      @param 
    */
    void setSubContent(int insertcontent=1);

    GuiObjectWnd *titleleft;
    GuiObjectWnd *titleright;
    GuiObjectWnd *titlecenter;
    GuiObjectWnd *content;
    int centered;
    StringW title;
    StringW content_id;
    StringW suffix;
};


#endif
