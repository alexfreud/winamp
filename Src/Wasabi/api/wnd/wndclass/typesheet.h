#ifndef _TYPESHEET_H
#define _TYPESHEET_H

#include "tabsheet.h"

#define TYPESHEET_PARENT TabSheet
/**
  Class TypeSheet

  @short A Typesheet Control.
  @author Nullsoft
  @ver 1.0
  @see 
*/
class TypeSheet : public TYPESHEET_PARENT {
public:

  /**
    TypeSheet constructor
  
    @param _windowtype
  */
  TypeSheet(const wchar_t *windowtype);
  
  /**
    TypeSheet method onInit .
  
    @ret 1
    @param None
  */
  virtual int onInit();
  
  /**
    TypeSheet method load
  */
  virtual void load();
  
  /**
    TypeSheet method setWindowType .
  
    @param windowtype The type of the window.
  */
  virtual void setWindowType(const wchar_t *windowtype);

private:
  StringW windowtype;
};

#endif
