#ifndef NULLSOFT_XML_IFC_XMLREADERPARAMS_H
#define NULLSOFT_XML_IFC_XMLREADERPARAMS_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
// ----------------------------------------------------------------------------

class NOVTABLE ifc_xmlreaderparams : public Dispatchable 
{
  protected:
    ifc_xmlreaderparams() {}
    virtual ~ifc_xmlreaderparams() {}
  public:
    const wchar_t *getItemName(size_t i);
    const wchar_t *getItemValue(size_t i);
    const wchar_t *getItemValue(const wchar_t *name);
    const wchar_t *enumItemValues(const wchar_t *name, size_t nb);
		int getItemValueInt(const wchar_t *name, int def = 0);
    size_t getNbItems();
  
  protected:
    DISPATCH_CODES
		{
      XMLREADERPARAMS_GETITEMNAME = 100,
      XMLREADERPARAMS_GETITEMVALUE = 200,
      XMLREADERPARAMS_GETITEMVALUE2 = 201,
      XMLREADERPARAMS_ENUMITEMVALUES = 202,
			XMLREADERPARAMS_GETITEMVALUEINT = 300,
      XMLREADERPARAMS_GETNBITEMS = 400,
    };
};

// ----------------------------------------------------------------------------

inline const wchar_t *ifc_xmlreaderparams::getItemName(size_t i) 
{
  return _call(XMLREADERPARAMS_GETITEMNAME, (const wchar_t *)0, i);
}

inline const wchar_t *ifc_xmlreaderparams::getItemValue(size_t i) 
{
  return _call(XMLREADERPARAMS_GETITEMVALUE, (const wchar_t *)0, i);
}

inline const wchar_t *ifc_xmlreaderparams::getItemValue(const wchar_t *name) 
{
  return _call(XMLREADERPARAMS_GETITEMVALUE2, (const wchar_t *)0, name);  
}

inline const wchar_t *ifc_xmlreaderparams::enumItemValues(const wchar_t *name, size_t nb) 
{
  return _call(XMLREADERPARAMS_ENUMITEMVALUES, (const wchar_t *)0, name, nb);
 }

inline int ifc_xmlreaderparams::getItemValueInt(const wchar_t *name, int def) 
{
 return _call(XMLREADERPARAMS_GETITEMVALUEINT, (int)0, name, def);
}

inline size_t ifc_xmlreaderparams::getNbItems() 
{
  return _call(XMLREADERPARAMS_GETNBITEMS, (size_t)0);
}

#endif