#include "ParamList.h"

ParamList::~ParamList()
{
    //parms_list.deleteAll();
    for (auto obj : parms_list)
    {
        delete obj;
    }
    parms_list.clear();
}

const wchar_t *ParamList::getItemName(int i) 
{
  if(((size_t)i)>=getNbItems())
		return L"";
  return 
		parms_list[i]->parm;
}


const wchar_t *ParamList::getItemValueIndex(int i) 
{
  if(((size_t)i)>=getNbItems()) 
		return L"";
  return 
		parms_list[i]->value;
}

const wchar_t *ParamList::getItemValue(const wchar_t *name) 
{
  for(size_t i=0;i!=getNbItems();i++)
    if(!WCSICMP(parms_list[i]->parm, name))
      return parms_list[i]->value;
  return NULL;
}

const wchar_t *ParamList::enumItemValues(const wchar_t *name, int nb)
{
  int f=0;
  for(size_t i=0;i!=getNbItems();i++)
    if(!WCSICMP(parms_list[i]->parm, name))
      if(f==nb)
        return parms_list[i]->value;
      else f++;
  return NULL;
}

int ParamList::getItemValueInt(const wchar_t *name, int def) 
{
  for(size_t i=0;i!=getNbItems();i++)
    if(!WCSICMP(parms_list[i]->parm, name))
		{
			// TODO: benski> do we want to return "def" when there's an empty value?
      return WTOI(parms_list[i]->value);
		}
  return def;
}


size_t ParamList::getNbItems() 
{
  return parms_list.size();
}

void ParamList::addItem(const wchar_t *parm, const wchar_t *value) 
{
  parms_struct *p= new parms_struct;
  p->parm = WCSDUP(parm);
	p->ownValue = true;
  p->value = value;
  parms_list.push_back(p);
}

void ParamList::removeItem(const wchar_t *parm) 
{
    for (size_t i=0; i!=parms_list.size(); i++) 
    {
        parms_struct *s = parms_list[i];
        if (!WCSICMP(parm, s->parm)) 
	    {
            delete s;
            parms_list.erase(parms_list.begin() + i);
            i--;
        }
    }
}

void ParamList::replaceItem(const wchar_t *parm, const wchar_t *value) 
{
  if (!value) 
	{
    removeItem(parm);
    return;
  }

  StringW s = value; // if we were passed our current value's pointer ... 

  const wchar_t *curval = getItemValue(parm);
  if (s.isequal(value) && curval) return; // (hey, if we're replacing with same value, forget about it, but only if we did have that value, because isequal will return true if curval is NULL and we pass it ("") )

  removeItem(parm); // ... then this call would make the value param invalid ...

  addItem(parm, s); // ... so we're sending a saved buffer instead
}

int ParamList::findItem(const wchar_t *parm) 
{
  for(size_t i=0;i!=getNbItems();i++)
    if(!WCSICMP(parms_list[i]->parm, parm))
      return (int)i;
  return -1;
}

#define CBCLASS ParamList
START_DISPATCH;
CB(XMLREADERPARAMS_GETITEMNAME, getItemName)
CB(XMLREADERPARAMS_GETITEMVALUE, getItemValueIndex)
CB(XMLREADERPARAMS_GETITEMVALUE2, getItemValue)
CB(XMLREADERPARAMS_ENUMITEMVALUES, enumItemValues)
CB(XMLREADERPARAMS_GETITEMVALUEINT, getItemValueInt)
CB(XMLREADERPARAMS_GETNBITEMS, getNbItems)
END_DISPATCH;
#undef CBCLASS