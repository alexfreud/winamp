#ifndef _ATTRSTR_H
#define _ATTRSTR_H

#include "attribute.h"

#include <bfc/string/bfcstring.h>
#include <bfc/common.h>

/**
  String configuration attributes, can have any string value 
  of any length. They can be used like any other config item.
  
  @short String configuration attribute.
  @ver 1.0
  @author Nullsoft
  @see _int
  @see _bool
  @see _float
*/
class _string : public Attribute {
public:
  /**
    Optionally set the name and default value of 
    your configuration attribute during construction.
      
    @param name
    @param default_val
  */
  _string(const wchar_t *name=NULL, const wchar_t *default_val=NULL)
    : Attribute(name) {
    setData(default_val, true);
  }

  /**
    Get the attribute type. This will return 
    a constant representing the attribute type.
    
    These constants can be: BOOL, FLOAT, STRING and INT.
    
    @see AttributeType
    @ret The attribute type.
  */
  virtual int getAttributeType() { return AttributeType::STRING; }
  
  /**
    Get the configuration group to be used to represent
    this attribute in the registry.
    
    @ret Config group to be used.
  */
  virtual const wchar_t *getConfigGroup() { return L"studio.configgroup.string"; }

//CUT  virtual int getPermissions() { return ATTR_PERM_ALL; }

  /**
    Get the value of the attribute.
    
    @ret The value of the attribute
  */
  const wchar_t *getValue();
  
  /**
    Set the value of the attribute.
    
    @param val The value you want to set.
    @ret 1, success; 0, failure;
  */
  int setValue(const wchar_t *val) { return setData(val); }

  // convenience operators
  /**
    Get the value of the attribute.
  */
  operator const wchar_t *() { return getValue(); }

  /**
    Set the value of the attribute.
  */
  const wchar_t *operator =(const wchar_t *newval) { return setValue(newval) ? newval : getValue(); }

private:
  StringW returnval;
};

#endif
