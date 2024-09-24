#ifndef NULLSOFT_WASABI_CFGITEMX_H
#define NULLSOFT_WASABI_CFGITEMX_H

#include "cfgitem.h"

class CfgItemX : public CfgItem
{
public:
	virtual ~CfgItemX() {}
  virtual const wchar_t *cfgitem_getName()=0;
  virtual GUID cfgitem_getGuid()=0;
  virtual int cfgitem_getNumAttributes()=0;
  virtual const wchar_t *cfgitem_enumAttribute(int n)=0;
  virtual const wchar_t *cfgitem_getConfigXML()=0;
  virtual void cfgitem_onCfgGroupCreate(ifc_window *cfggroup, const wchar_t *attrname)=0;
  virtual void cfgitem_onCfgGroupDelete(ifc_window *cfggroup)=0;

  virtual int cfgitem_getNumChildren()=0;
  virtual CfgItem *cfgitem_enumChild(int n)=0;
  virtual GUID cfgitem_getParentGuid()=0;

  virtual void cfgitem_onRegister()=0;
  virtual void cfgitem_onDeregister()=0;

  virtual int cfgitem_getAttributeType(const wchar_t *name)=0;
  virtual const wchar_t *cfgitem_getAttributeConfigGroup(const wchar_t *name)=0;

  virtual int cfgitem_getDataLen(const wchar_t *name)=0;
  virtual int cfgitem_getData(const wchar_t *name, wchar_t *data, int data_len)=0;
  virtual int cfgitem_setData(const wchar_t *name, const wchar_t *data)=0;
	virtual ifc_dependent *cfgitem_getDependencyPtr()=0;
	  virtual int cfgitem_delAttribute(const wchar_t *name)=0;
		virtual int cfgitem_addAttribute(const wchar_t *name, const wchar_t *defval)=0;

protected:
  RECVS_DISPATCH;

   
};


#endif