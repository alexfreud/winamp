#ifndef _ATTRFLOAT_H
#define _ATTRFLOAT_H

#include "attribute.h"

// actually it's a double :)

class _float : public Attribute {
public:
  /**
    Optionally set the name and default value of 
    your configuration attribute during construction.
    
    @param name Name of the configuration attribute.
    @param default_val Default value.
  */
  _float(const wchar_t *name=NULL, double default_val=0.f) : Attribute(name) {
    setValueAsDouble(default_val, true);
  }
  
  /**
    Get the attribute type. This will return 
    a constant representing the attribute type.
    
    These constants can be: BOOL, FLOAT, STRING and INT.
    
    @see AttributeType
    @ret The attribute type.
  */
  virtual int getAttributeType() { return AttributeType::FLOAT; }
  
  /**
    Get the configuration group to be used to represent
    this attribute in the registry.
    
    @ret Config group to be used.
  */
  virtual const wchar_t *getConfigGroup() { return L"studio.configgroup.float"; }

  // convenience operators
  /**
    Get the value of the attribute.
  */
  operator double() { return getValueAsDouble(); }
  
  /**
    Set the value of the attribute.
  */
  double operator =(double newval) { return setValueAsDouble(newval) ? newval : getValueAsDouble(); }
};

#endif
