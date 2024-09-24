#ifndef _STATUS_H
#define _STATUS_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <bfc/string/StringW.h>
#include <api/wndmgr/guistatuscb.h>
#include <bfc/depend.h>

class ButtBar;
class AppCmds;

#define STATUSBAR_PARENT GuiObjectWnd

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class StatusBar : public STATUSBAR_PARENT, public GuiStatusCallbackI {
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  StatusBar();
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual ~StatusBar();

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onInit();

  // completeness indicator
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void pushCompleted(int max);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void incCompleted(int add);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void setCompleted(int pos);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void popCompleted();

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void timerCallback(int id);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual int onResize();

  virtual api_dependent *status_getDependencyPtr() { return this; }

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void onSetStatusText(const wchar_t *text, int overlay);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void onAddAppCmds(AppCmds *commands);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void onRemoveAppCmds(AppCmds *commands);
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void fakeButtonPush(const wchar_t *name);

protected:
  
  int fakeButtonPush(ButtBar *bb, const wchar_t *name);
  void setExclude(const wchar_t *val);
  
  void setIncludeOnly(const wchar_t *val);
  StringW exclude_list, include_only;

protected:
  void regenerate();

private:
  StringW contentgroupname;

  StringW status_text;
  int overtimer;

  // completeness
  int max;
  int completed;
  int progress_width;

  GuiObjectWnd bg;

  ButtBar *bbleft, *bbright;

  PtrList<AppCmds> appcmds;
};

#endif
