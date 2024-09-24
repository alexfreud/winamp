#ifndef NULLSOFT_XML_XMLPARAMETERS_H
#define NULLSOFT_XML_XMLPARAMETERS_H

#include "ifc_xmlreaderparams.h"

class XMLParameters : public ifc_xmlreaderparams
{
public:
	XMLParameters(const wchar_t **_parameters);

  const wchar_t *GetItemName(int i);
  const wchar_t *GetItemValueIndex(int i);
  const wchar_t *GetItemValue(const wchar_t *name);
	int GetItemValueInt(const wchar_t *name, int def = 0);
  const wchar_t *EnumItemValues(const wchar_t *name, int nb);
  int GetNumItems();

private:
	const wchar_t **parameters;
	int numParameters;
	bool numParametersCalculated;

	void CountTo(int x);
	void Count();
	
protected:
	RECVS_DISPATCH;
};

#endif