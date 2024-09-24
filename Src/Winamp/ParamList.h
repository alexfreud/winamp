#pragma once
#include "../xml/ifc_xmlreaderparams.h"
#include <vector>
#include <bfc/string/stringw.h>
#include <bfc/wasabi_std.h>

class ParamList : public ifc_xmlreaderparams
{
public:
  ParamList() {}
  ~ParamList();

  const wchar_t *getItemName(int i);
  const wchar_t *getItemValueIndex(int i);
  const wchar_t *getItemValue(const wchar_t *name);
  const wchar_t *enumItemValues(const wchar_t *name, int nb);
  int getItemValueInt(const wchar_t *name, int def = 0);
  size_t getNbItems();

  void addItem(const wchar_t *parm, const wchar_t *value);
  void removeItem(const wchar_t *parm);
  void replaceItem(const wchar_t *parm, const wchar_t *value);
  int findItem(const wchar_t *parm);

protected:
	RECVS_DISPATCH;
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
	std::vector<parms_struct*> parms_list;
};