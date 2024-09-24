#ifndef _ATTRIBUTE_H
#define _ATTRIBUTE_H

#include <bfc/depend.h>
#include <bfc/named.h>
#include <bfc/common.h>
#include <bfc/string/StringW.h>

class CfgItemI;

// lowercase types are reserved for official Nullsoft use
// uppercase are 3rd-party defined
namespace AttributeType {
  /**
    Attribute types.
  */
  enum {
    NONE = 0,
    INT = MK3CC('i','n','t'),		// attrint.h
    STRING = MK3CC('s','t','r'),	// attrstr.h
    BOOL = MK4CC('b','o','o','l'),	// attrbool.h
    FLOAT = MK4CC('f','l','o','t'),	// attrfloat.h
    FILENAME = MK2CC('f','n'),		// attrfn.h
  };
};

/**
  Generic configuration attribute. 
  
  Configuration attributes enable you to store 
  uniquely identifiable values that get pushed 
  to a configuration file automatically upon shutdown 
  of any Wasabi application.
  
  You shouldn't normally use this on 
  it's own, look at the CfgItemI class 
  instead.

  @short Generic configuration attribute.
  @ver 1.0
  @author Nullsoft
  @see _float
  @see _int
  @see _bool
  @see _string
  @see CfgItemI
*/
class NOVTABLE Attribute : public DependentI, private NamedW 
{
public:
  static const GUID *depend_getClassGuid() {
    // {5AB601D4-1628-4604-808A-7ED899849BEB}
    static const GUID ret = 
    { 0x5ab601d4, 0x1628, 0x4604, { 0x80, 0x8a, 0x7e, 0xd8, 0x99, 0x84, 0x9b, 0xeb } };
    return &ret;
  }
protected:
  
  /**
    Optionally set the name and default value of 
    your configuration attribute during construction.
    
    @param name Name of the configuration attribute.
    @param default_val Default value.
  */
  Attribute(const wchar_t *name=NULL, const wchar_t *desc=NULL);
  
public:
  virtual ~Attribute();

  /**
    Set the name of the configuration
    attribute.
    
    @param newname Name of the attribute.
  */
  void setName(const wchar_t *newname);
  
  /**
    Get the name of the configuration
    attribute.
    
    @ret Name of the attribute.
  */
  const wchar_t *getAttributeName();

  /**
    Get the attribute's description.
    
    @ret Attribute's description.
  */
  const wchar_t *getAttributeDesc();

  /**
    Get the attribute type. Override
    this for your custom attribute type.
    
    @ret Attribute type.
  */
  virtual int getAttributeType()=0;	// override me
  
  /**
    Get the configuration group to be used to represent
    this attribute in the registry. 
    
    This is only called if the kernel doesn't have a default 
    config group set for your type already.
    
    @ret Config group to be used.
  */
  virtual const wchar_t *getConfigGroup() { return NULL; }	// override me

  /**
    Get the attribute's value as signed integer.
    
    @ret Attribute value, as a signed integer.
  */
  int getValueAsInt();
  
  /**
    Set the attribute's value with a signed integer while
    also being able to replace the default value previously
    set.
    
    @param newval Attribute's new value.
    @param def true, replace the current default value; false, leave the default value unchanged;
  */
  int setValueAsInt(int newval, bool def=false);

  /**
    Get the attribute's value as signed double.
    
    @ret Attribute value, as a signed double.
  */
  double getValueAsDouble();
  
  /**
    Set the attribute's value with a signed double while
    also being able to replace the default value previously
    set.
    
    @param newval Attribute's new value.
    @param def true, replace the current default value; false, leave the default value unchanged;
  */
  double setValueAsDouble(double newval, bool def=false);

  /**
    Get the length of the attribute's value (data)
    in bytes.
    
    @ret Attribute value (data) length, in bytes.
  */
  int getDataLen();

  /**
    Get the attribute's raw data. 
    
    This will return the data the attribute is storing
    in a char buffer you hand to it.
    
    @ret Attribute value, as a signed double.
    @param data Pointer to a char buffer.
    @param data_len The maximum amount of bytes the char buffer can hold.
  */
  int getData(wchar_t *data, int data_len);
  
  /**
    Set the attribute's value with a zero terminated string. Also 
    enables you to replace the default value previously
    set.
    
    @param newval Attribute's new value.
    @param def true, replace the current default value; false, leave the default value unchanged;
  */
  int setData(const wchar_t *data, bool def=false);

  void disconnect();

  enum {
    Event_DATACHANGE=100,
  };
protected:
  friend class CfgItemI;
  
  /**
    Set the attribute's value without causing 
    a callback.

    @ret 1.
    @param data Attribute's new value.
  */
  int setDataNoCB(const wchar_t *data);
  
  /**
    Set the configuration item associated with this
    attribute.
  */
  void setCfgItem(CfgItemI *item);

  StringW mkTag();

private:
  StringW desc;
	StringW default_val, *private_storage;
  CfgItemI *cfgitemi;
};

#define ATTR_PERM_READ	1
#define ATTR_PERM_WRITE	2

#define ATTR_PERM_ALL	(~0)

// render hints for getRenderHint
enum {
  ATTR_RENDER_HINT_INT_CHECKMARK
};

#endif
