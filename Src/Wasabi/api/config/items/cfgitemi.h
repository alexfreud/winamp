#ifndef _CFGITEMI_H
#define _CFGITEMI_H

#include "cfgitemx.h"

#include <bfc/named.h>
#include <bfc/ptrlist.h>
#include <bfc/depend.h>

#include <map>
#include <string>

class AttrCallback;
class Attribute;

// this is the one you inherit from/use
/**
  
  @short Configuration Item
  @ver 1.0
  @author Nullsoft
  @see Attribute
  @see _bool
  @see _int
  @see _float
  @see _string
*/
class CfgItemI : public CfgItemX, public DependentI, private NamedW
{
public:
  /**
    Optionally sets the name and the GUID of the
    configuration item if they are specified
    upon creation of the object.
    
    @param name Name of the configuration item.
    @param guid GUID of the configuration item.
  */
  CfgItemI(const wchar_t *name=NULL, GUID guid=INVALID_GUID);
  
  /**
    Does nothing.
  */
  virtual ~CfgItemI();

  /**
    Get the name of the configuration item.
    
    @ret Name of the configuration item.
  */
  const wchar_t *cfgitem_getName();
  
  /**
    Get the GUID of the configuration item.
    
    @ret GUID of the configuration item.
  */
  GUID cfgitem_getGuid();

  /**
    Sets the prefix to be prepended in the config file for all attributes
    of this item.

    @see cfgitem_getPrefix
    @param prefix The prefix.
  */
  void cfgitem_setPrefix(const wchar_t *prefix);
/**
  Gets the config prefix, if any was set.

  @see cfgitem_setPrefix
  @ret Pointer to the config prefix.
*/
  const wchar_t *cfgitem_getPrefix();
  
  /**
    Get the number of attributes registered
    to this configuration item.
    
    @ret Number of attributes.
  */
  int cfgitem_getNumAttributes();
  
  /**
    Enumerate the attributes registered
    with this configuration item.
    
    @ret 
  */
  const wchar_t *cfgitem_enumAttribute(int n);

  const wchar_t *cfgitem_getConfigXML();
  virtual void cfgitem_onCfgGroupCreate(ifc_window *cfggroup, const wchar_t *attrname) {}
  virtual void cfgitem_onCfgGroupDelete(ifc_window *cfggroup) {}

  virtual int cfgitem_getNumChildren();
  virtual CfgItem *cfgitem_enumChild(int n);
  virtual GUID cfgitem_getParentGuid();

  virtual void cfgitem_onRegister();
  virtual void cfgitem_onDeregister();

  int cfgitem_getAttributeType(const wchar_t *name);
  const wchar_t *cfgitem_getAttributeConfigGroup(const wchar_t *name);
  int cfgitem_getDataLen(const wchar_t *name);
  int cfgitem_getData(const wchar_t *name, wchar_t *data, int data_len);
  int cfgitem_setData(const wchar_t *name, const wchar_t *data);

  // override these to catch notifications from attribs, call down
  virtual int cfgitem_onAttribSetValue(Attribute *attr);

  virtual int cfgitem_usePrivateStorage() { return 0; } //override and return 1 to keep stuff out of system settings

protected:
  void cfgitem_setGUID(GUID guid);

public:
  int setName(const wchar_t *name);
  int registerAttribute(Attribute *attr, AttrCallback *acb=NULL);
  // does not call delete on the attribute
  int deregisterAttribute(Attribute *attr);
  void deregisterAll();

  void addCallback(Attribute *attr, AttrCallback *acb);

  int cfgitem_addAttribute(const wchar_t *name, const wchar_t *defval);
  int cfgitem_delAttribute(const wchar_t *name);

protected:
  
  // derived classes can override this to catch name changes
  virtual void cfgitem_onSetName() { }

  Attribute *getAttributeByName(const wchar_t *name);
 
  void addChildItem(CfgItemI *child);

  void setCfgXml(const wchar_t *groupname);

  void setParentGuid(GUID guid);

private:
  api_dependent *cfgitem_getDependencyPtr() { return this; };
  virtual void *dependent_getInterface(const GUID *classguid);

  // from Named
  virtual void onSetName() { cfgitem_onSetName(); }

  std::wstring prefix;
  PtrList<Attribute> attributes;
  std::multimap<Attribute*, AttrCallback*> callbacks;	//CUT
  PtrList<CfgItemI> children;
  std::wstring cfgxml;
  GUID myguid, parent_guid;
  PtrList<Attribute> newattribs;
};

#endif
