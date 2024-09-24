#ifndef __XMLPARAMSI_H
#define __XMLPARAMSI_H

#include <bfc/dispatch.h>
#include <bfc/string/bfcstring.h>
#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>

/*<?<autoheader/>*/
#include <api/xml/xmlparams.h>
#include <api/xml/xmlparamsx.h>
/*?>*/


class XmlReaderParamsI : public XmlReaderParamsX
{
public:
  XmlReaderParamsI() {}
  virtual ~XmlReaderParamsI();

  DISPATCH(100) const wchar_t *getItemName(int i);
  DISPATCH(200) const wchar_t *getItemValue(int i);
  DISPATCH(201) const wchar_t *getItemValue(const wchar_t *name);
  DISPATCH(202) const wchar_t *enumItemValues(const wchar_t *name, int nb);
  DISPATCH(300) int getItemValueInt(const wchar_t *name, int def = 0);
  DISPATCH(400) int getNbItems();

  DISPATCH(500) void addItem(const wchar_t *parm, const wchar_t *value);
  DISPATCH(600) void removeItem(const wchar_t *parm);
  DISPATCH(700) void replaceItem(const wchar_t *parm, const wchar_t *value);
  DISPATCH(800) int findItem(const wchar_t *parm);

	NODISPATCH void addItemSwapValue(const wchar_t *parm, StringW &value); // calling this will destroy your String... here for optimization ...
private:
  struct parms_struct 
	{
		parms_struct() : parm(0), ownValue(false)
		{}
		~parms_struct()
		{
			if (ownValue)
				FREE((wchar_t *)parm);
		}
    const wchar_t *parm;
    StringW value;
		bool ownValue;
  };
  PtrList<parms_struct> parms_list;
};


#endif
