#include <precomp.h>
#include "xmlparamsi.h"
#include <bfc/wasabi_std.h>

XmlReaderParamsI::~XmlReaderParamsI()
{
  parms_list.deleteAll();
}

const wchar_t *XmlReaderParamsI::getItemName(int i) 
{
  if(i>getNbItems()) 
		return L"";
  return 
		parms_list[i]->parm;
}


const wchar_t *XmlReaderParamsI::getItemValue(int i) 
{
  if(i>getNbItems()) 
		return L"";
  return 
		parms_list[i]->value;
}

const wchar_t *XmlReaderParamsI::getItemValue(const wchar_t *name) 
{
  for(int i=0;i<getNbItems();i++)
    if(!WCSICMP(parms_list[i]->parm, name))
      return parms_list[i]->value;
  return NULL;
}

const wchar_t *XmlReaderParamsI::enumItemValues(const wchar_t *name, int nb)
{
  int f=0;
  for(int i=0;i<getNbItems();i++)
    if(!WCSICMP(parms_list[i]->parm, name))
      if(f==nb)
        return parms_list[i]->value;
      else f++;
  return NULL;
}

int XmlReaderParamsI::getItemValueInt(const wchar_t *name, int def) 
{
  for(int i=0;i<getNbItems();i++)
    if(!WCSICMP(parms_list[i]->parm, name))
		{
			// TODO: benski> do we want to return "def" when there's an empty value?
      return WTOI(parms_list[i]->value);
		}
  return def;
}


int XmlReaderParamsI::getNbItems() 
{
  return parms_list.getNumItems();
}

void XmlReaderParamsI::addItem(const wchar_t *parm, const wchar_t *value) 
{
  parms_struct *p= new parms_struct;
  p->parm = WCSDUP(parm);
	p->ownValue = true;
  p->value = value;
  parms_list.addItem(p);
}

void XmlReaderParamsI::removeItem(const wchar_t *parm) 
{
  for (int i=0;i<parms_list.getNumItems();i++) 
	{
    parms_struct *s = parms_list.enumItem(i);
    if (!WCSICMP(parm, s->parm)) 
		{
      delete s;
      parms_list.removeByPos(i);
      i--;
    }
  }
}

void XmlReaderParamsI::replaceItem(const wchar_t *parm, const wchar_t *value) 
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

int XmlReaderParamsI::findItem(const wchar_t *parm) 
{
  for(int i=0;i<getNbItems();i++)
    if(!WCSICMP(parms_list[i]->parm, parm))
      return i;
  return -1;
}

 // calling this will destroy your String... here for optimization ...
void XmlReaderParamsI::addItemSwapValue(const wchar_t *parm, StringW &value)
{
	parms_struct *p= new parms_struct;
  p->parm = parm;
  p->value.swap(&value);
  parms_list.addItem(p);
}
