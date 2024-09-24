#ifndef __SKINITEM_H
#define __SKINITEM_H

#include <bfc/dispatch.h>
#include "../xml/ifc_xmlreaderparams.h"

class skin_xmlreaderparams;

class SkinItem : public Dispatchable 
{
  public:
    const wchar_t *getXmlRootPath();
    const wchar_t *getName();
    ifc_xmlreaderparams *getParams();
    int getSkinPartId();
    SkinItem *getAncestor();

  enum 
	{
    SKINITEM_GETXMLROOTPATH = 0,
    SKINITEM_GETNAME = 10,
    SKINITEM_GETPARAMS = 20,
    SKINITEM_GETSKINPARTID = 30,
    SKINITEM_GETANCESTOR = 40,
  };
};

inline const wchar_t *SkinItem::getXmlRootPath() 
{
  return _call(SKINITEM_GETXMLROOTPATH, (const wchar_t *)0);
}

inline const wchar_t *SkinItem::getName() 
{
  return _call(SKINITEM_GETNAME, (const wchar_t *)0);
}

inline ifc_xmlreaderparams *SkinItem::getParams() 
{
  return _call(SKINITEM_GETPARAMS, (ifc_xmlreaderparams *)NULL);
}

inline int SkinItem::getSkinPartId() 
{
  return _call(SKINITEM_GETSKINPARTID, (int)0);
}

inline SkinItem *SkinItem::getAncestor() 
{
  return _call(SKINITEM_GETANCESTOR, (SkinItem *)NULL);
}

class SkinItemI : public SkinItem 
{
  public:
    virtual const wchar_t *getXmlRootPath()=0;
    virtual const wchar_t *getName()=0;
    virtual ifc_xmlreaderparams *getParams()=0;
    virtual int getSkinPartId()=0;
    virtual SkinItem *getAncestor()=0;

  protected:
    RECVS_DISPATCH;
};

#endif
