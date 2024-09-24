#ifndef _ATTRCB_H
#define _ATTRCB_H

#include "attribute.h"

/**
  Enables you to register callbacks on
  a specific attribute to monitor if it's
  value has been changed by the user or other.

  This class is not meant to be used on it's own.
  Please derive from it instead.
  
  @short Attribute callback
  @ver 1.0
  @author Nullsoft
  @see Attribute
  @see int_attrCB
  @see _int
  @see _float
  @see _string
  @see _bool   
*/
class AttrCallback {
public:
  /**
    Does nothing.
  */
  virtual ~AttrCallback() {}
  
  /**
    Event triggered when the value of the attribute,
    for which this callback has been registered, changes.
    
    This is a pure virtual, please override to implement
    your custom behavior.
    
    @param attr Attribute for which the value has changed.
  */
  virtual void onValueChange(Attribute *attr)=0;
};

/**
  Enables you to register callbacks on a specific
  integer or boolean attribute to monitor if the
  value has been changed by the user or other.
  
  @short Integer or Boolean attribute Callback.
  @ver 1.0
  @author Nullsoft
  @see Attribute
  @see _int
  @see _bool   
*/
class int_attrCB : public AttrCallback {
  typedef void (*fnPtrType)(int);
public:
  /**
    Upon construction, you must specify which
    function will be called when the value of
    the attribute has indeed changed.
    
    This is done using a pointer to the function.
    The function must accept one parameter of type
    int, like so: void myfunc(int val);
    
    @param _fn Pointer to the function to use on value change.
  */
  int_attrCB(fnPtrType _fn) { fnptr = _fn; }
  
  /**
    Event triggered when the value of the attribute,
    for which this callback has been registered, changes.
    
    Override this to implement your own behavior.
    The default is to send the new value of the attribute
    to a function which you specify upon construction
    of this object.
        
    @param attr Attribute for which the value has changed.
  */
  virtual void onValueChange(Attribute *attr) {
    ASSERT(attr->getAttributeType() == AttributeType::INT ||
           attr->getAttributeType() == AttributeType::BOOL);
    (*fnptr)(attr->getValueAsInt());
  }
private:
  fnPtrType fnptr;
};

class string_attrCB : public AttrCallback {
  typedef void (*fnPtrType)(const wchar_t *);
public:
  /**
    Upon construction, you must specify which
    function will be called when the value of
    the attribute has indeed changed.
    
    This is done using a pointer to the function.
    The function must accept one parameter of type
    int, like so: void myfunc(const char *val);
    
    @param _fn Pointer to the function to use on value change.
  */
  string_attrCB(fnPtrType _fn) { fnptr = _fn; }
  
  /**
    Event triggered when the value of the attribute,
    for which this callback has been registered, changes.
    
    Override this to implement your own behavior.
    The default is to send the name of the attribute
    to a function which you specify upon construction
    of this object.
        
    @param attr Attribute for which the value has changed.
  */
  virtual void onValueChange(Attribute *attr) 
	{
    ASSERT(attr->getAttributeType() == AttributeType::STRING);
    (*fnptr)(attr->getAttributeName());
  }
private:
  fnPtrType fnptr;
};

#endif
